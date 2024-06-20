#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>

using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

class ClientSession : public std::enable_shared_from_this<ClientSession> {
public:
    ClientSession(tcp::socket socket, ssl::context& context)
        : socket_(std::move(socket), context) {}

    void start() {
        auto self(shared_from_this());
        socket_.async_handshake(ssl::stream_base::server,
            [this, self](const boost::system::error_code& error) {
                if (!error) {
                    std::cout << "Client connected from " << socket_.lowest_layer().remote_endpoint() << std::endl;
                    do_read();
                } else {
                    std::cerr << "Handshake failed: " << error.message() << std::endl;
                }
            });
    }

private:
    void do_read() {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::cout << "########################################" << std::endl;
                    std::cout << "\nReceived data from client: \n" << std::string(data_, length) << std::endl;
                    send_timestamp();
                } else if (ec != boost::asio::error::eof) {
                    std::cerr << "Read error: " << ec.message() << std::endl;
                }
            });
    }

    void send_timestamp() {
        auto self(shared_from_this());
        auto now = std::chrono::high_resolution_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() % 1000000;

        std::ostringstream timestamp_stream;
        timestamp_stream << std::put_time(gmtime(&now_time_t), "%Y-%m-%dT%H:%M:%S");
        timestamp_stream << "." << std::setw(6) << std::setfill('0') << microseconds << "Z";
        std::string timestamp = timestamp_stream.str();

        std::string message = "{\"transactTime\": \"" + timestamp + "\"}";
        boost::asio::async_write(socket_, boost::asio::buffer(message),
            [this, self, message](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    std::cout << "\nSent timestamp to client: \n" << message << "\n";
                    // After writing, read again to handle multiple requests from the same client
                    do_read();
                    std::cout << "########################################" << std::endl;
                } else {
                    std::cerr << "Write error: " << ec.message() << std::endl;
                }
            });
    }

    ssl::stream<tcp::socket> socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class Server {
public:
    Server(boost::asio::io_service& io_service, short port, ssl::context& context)
        : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), socket_(io_service), context_(context) {
        std::cout << "Server running on port " << port << std::endl;
        start_accept();
    }

private:
    void start_accept() {
        acceptor_.async_accept(socket_,
            boost::bind(&Server::handle_accept, this, boost::asio::placeholders::error));
    }

    void handle_accept(const boost::system::error_code& error) {
        if (!error) {
            std::cout << "Accepted new connection" << std::endl;
            std::make_shared<ClientSession>(std::move(socket_), context_)->start();
        } else {
            std::cerr << "Accept error: " << error.message() << std::endl;
        }
        start_accept(); // Accept the next connection
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    ssl::context& context_;
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 4) {
            std::cerr << "Usage: server <port> <cert_file> <key_file>\n";
            return 1;
        }

        boost::asio::io_service io_service;
        ssl::context context(ssl::context::sslv23);

        context.use_certificate_chain_file(argv[2]);
        context.use_private_key_file(argv[3], ssl::context::pem);

        Server s(io_service, std::atoi(argv[1]), context);
        io_service.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
