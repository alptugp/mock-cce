#include <libwebsockets.h>
#include <string.h>
#include <signal.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <ctime>
#include <chrono>
#include <nlohmann/json.hpp> // Include the nlohmann::json library
#include <regex>

#define WEBSOCKET_SERVER_TX_BUFFER_SIZE 16378
#define MESSAGE_RATE 80

#if defined(USE_BITMEX_MOCK_EXCHANGE)  
	#define NUMBER_OF_CONNECTIONS 3
	const std::vector<std::string> currencyPairs = {"XBTETH", "XBTUSDT", "ETHUSDT"};
#elif defined(USE_KRAKEN_MOCK_EXCHANGE)  
	#if defined(USE_PORTFOLIO_122)
		static const std::vector<std::string> currencyPairs = { "KSM/EUR","KSM/BTC","KSM/DOT","KSM/GBP","KSM/ETH","KSM/USD","GBP/USD","BTC/CAD","BTC/EUR","BTC/AUD","BTC/JPY","BTC/GBP","BTC/CHF","BTC/USDT","BTC/USD","BTC/USDC","LTC/EUR","LTC/BTC","LTC/AUD","LTC/JPY","LTC/GBP","LTC/ETH","LTC/USDT","LTC/USD","SOL/EUR","SOL/BTC","SOL/GBP","SOL/ETH","SOL/USDT","SOL/USD","DOT/EUR","DOT/BTC","DOT/JPY","DOT/GBP","DOT/ETH","DOT/USDT","DOT/USD","ETH/CAD","ETH/EUR","ETH/BTC","ETH/AUD","ETH/JPY","ETH/GBP","ETH/CHF","ETH/USDT","ETH/USD","ETH/USDC","LINK/EUR","LINK/BTC","LINK/AUD","LINK/JPY","LINK/GBP","LINK/ETH","LINK/USDT","LINK/USD","USDC/CAD","USDC/EUR","USDC/AUD","USDC/GBP","USDC/CHF","USDC/USDT","USDC/USD","ADA/EUR","ADA/BTC","ADA/AUD","ADA/GBP","ADA/ETH","ADA/USDT","ADA/USD","ATOM/EUR","ATOM/BTC","ATOM/GBP","ATOM/ETH","ATOM/USDT","ATOM/USD","USDT/EUR","USDT/AUD","USDT/JPY","USDT/GBP","USDT/CHF","USDT/USD","USDT/CAD","AUD/JPY","AUD/USD","XRP/CAD","XRP/EUR","XRP/BTC","XRP/AUD","XRP/GBP","XRP/ETH","XRP/USDT","XRP/USD","EUR/CAD","EUR/AUD","EUR/JPY","EUR/GBP","EUR/CHF","EUR/USD","BCH/EUR","BCH/BTC","BCH/AUD","BCH/JPY","BCH/GBP","BCH/ETH","BCH/USDT","BCH/USD","USD/CHF","USD/JPY","USD/CAD","ALGO/EUR","ALGO/BTC","ALGO/GBP","ALGO/ETH","ALGO/USDT","ALGO/USD", };
	#elif defined(USE_PORTFOLIO_92)
		static const std::vector<std::string> currencyPairs = { "BCH/USD","BCH/BTC","BCH/EUR","BCH/AUD","BCH/GBP","BCH/ETH","BCH/USDT","BCH/JPY","BTC/USD","BTC/EUR","BTC/USDC","BTC/AUD","BTC/GBP","BTC/CAD","BTC/USDT","BTC/JPY","USD/CAD","USD/JPY","XRP/USD","XRP/BTC","XRP/EUR","XRP/AUD","XRP/GBP","XRP/ETH","XRP/CAD","XRP/USDT","EUR/USD","EUR/AUD","EUR/GBP","EUR/CAD","EUR/JPY","LTC/USD","LTC/EUR","LTC/BTC","LTC/AUD","LTC/GBP","LTC/ETH","LTC/USDT","LTC/JPY","ETH/USD","ETH/EUR","ETH/BTC","ETH/USDC","ETH/AUD","ETH/GBP","ETH/CAD","ETH/USDT","ETH/JPY","LINK/USD","LINK/BTC","LINK/EUR","LINK/AUD","LINK/GBP","LINK/ETH","LINK/USDT","LINK/JPY","ADA/USD","ADA/BTC","ADA/EUR","ADA/AUD","ADA/GBP","ADA/ETH","ADA/USDT","USDC/USD","USDC/EUR","USDC/AUD","USDC/GBP","USDC/CAD","USDC/USDT","GBP/USD","DOT/USD","DOT/BTC","DOT/EUR","DOT/GBP","DOT/ETH","DOT/USDT","DOT/JPY","USDT/USD","USDT/EUR","USDT/AUD","USDT/GBP","USDT/CAD","USDT/JPY","AUD/USD","AUD/JPY", };
	#elif defined(USE_PORTFOLIO_50)
		static const std::vector<std::string> currencyPairs = { "BCH/JPY","BCH/ETH","BCH/GBP","BCH/AUD","BCH/BTC","BCH/USDT","BCH/EUR","BCH/USD","USDT/JPY","USDT/GBP","USDT/AUD","USDT/EUR","USDT/USD","BTC/JPY","BTC/GBP","BTC/AUD","BTC/USDT","BTC/EUR","BTC/USD","EUR/GBP","EUR/JPY","EUR/AUD","EUR/USD","ETH/JPY","ETH/EUR","ETH/AUD","ETH/BTC","ETH/USDT","ETH/GBP","ETH/USD","USD/JPY","LINK/JPY","LINK/ETH","LINK/EUR","LINK/AUD","LINK/BTC","LINK/USDT","LINK/GBP","LINK/USD","LTC/JPY","LTC/ETH","LTC/GBP","LTC/AUD","LTC/BTC","LTC/USDT","LTC/EUR","LTC/USD","GBP/USD","AUD/JPY","AUD/USD", };
	#elif defined(USE_PORTFOLIO_3)
		static const std::vector<std::string> currencyPairs = {"USDT/USD", "SOL/USDT", "SOL/USD"};
	#endif
#endif

struct per_session_data__minimal {
	struct per_session_data__minimal *pss_list;
	struct lws *wsi;
	int connection_index;
	int last; /* the last message number we sent */
};

// File handling variables
static std::unordered_map<std::string, std::ifstream> infiles;

std::unordered_map<std::string, std::vector<std::string>> linesMap;
// size_t connection_to_current_line[NUMBER_OF_CONNECTIONS];
int next_connection_index = 0;

// Function to read all lines from the file
void readLinesFromFiles() {
	for (const std::string& currencyPair : currencyPairs) { 
        // Replace '/' with '_' in currencyPair to create a valid filename
        std::string sanitizedPair = currencyPair;
        std::replace(sanitizedPair.begin(), sanitizedPair.end(), '/', '_');

        std::string filePath = std::string("../kraken-market-data/21-115-historical-data/") + sanitizedPair + "-data.txt";
        infiles[currencyPair].open(filePath, std::ios_base::in);
        if (!infiles[currencyPair].is_open()) {
            std::cerr << "Error: Unable to open file for " << currencyPair << std::endl;
            return;
        }
		std::string line;
		while (std::getline(infiles[currencyPair], line)) {
			linesMap[currencyPair].push_back(line);
		}
		infiles[currencyPair].close();
    }

    
}

// Function to get the current timestamp in the required format
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;

    std::stringstream ss;
    ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dT%H:%M:%S");
    ss << '.' << std::setw(6) << std::setfill('0') << us.count();
    ss << 'Z';
    return ss.str();
}

// Function to parse a timestamp string into a std::chrono::time_point
std::chrono::system_clock::time_point parseTimestamp(const std::string& timestamp) {
    std::tm tm = {};
    std::istringstream ss(timestamp);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    auto us = std::stoi(timestamp.substr(20, 6));
    return tp + std::chrono::microseconds(us);
}

static int
callback_protocol(struct lws *wsi, enum lws_callback_reasons reason,
			  void *user, void *in, size_t len)
{
	unsigned char buf[LWS_PRE + WEBSOCKET_SERVER_TX_BUFFER_SIZE];
	struct per_session_data__minimal *pss =
			(struct per_session_data__minimal *)user;

	switch (reason) {

	case LWS_CALLBACK_ESTABLISHED:
		lwsl_user("LWS_CALLBACK_ESTABLISHED\n");
		pss->wsi = wsi;
		pss->last = 0;
		pss->connection_index = next_connection_index;
		next_connection_index++;
		
		lws_set_timer_usecs(wsi, (LWS_USEC_PER_SEC / 0.3));
		// lws_set_timeout(wsi, (pending_timeout)1, 60);
		break;

	case LWS_CALLBACK_TIMER:
		lwsl_user("LWS_CALLBACK_TIMER\n");
		lws_callback_on_writable(wsi);
        break;

	case LWS_CALLBACK_CLOSED:
		lwsl_user("LWS_CALLBACK_CLOSED\n");
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE: {
		// Check if there are more lines to send
		int connection_indx = pss->connection_index;
		if (pss->last < linesMap[currencyPairs[connection_indx]].size() - 1) {
			std::cout << pss->last << " " << currencyPairs[connection_indx] << " " << linesMap[currencyPairs[connection_indx]].size() << " " << std::endl;
			int current_line_for_connection = pss->last;
			std::string line_to_send = linesMap[currencyPairs[connection_indx]][current_line_for_connection];

            // Extract the timestamp of the current and next lines
            std::regex timestamp_regex("\"timestamp\":\"([^\"]+)\"");
            std::smatch match;
            std::chrono::system_clock::time_point current_time, next_time;

            bool current_has_timestamp = false;
            if (std::regex_search(line_to_send, match, timestamp_regex)) {
                std::cout << "Matched current timestamp: " << match[1].str() << std::endl;
                current_time = parseTimestamp(match[1].str());
                current_has_timestamp = true;
            } else {
                std::cout << "Failed to match current timestamp in line: " << line_to_send << std::endl;
            }

            if (pss->last + 1 < linesMap[currencyPairs[connection_indx]].size() && 
                std::regex_search(linesMap[currencyPairs[connection_indx]][pss->last + 1], match, timestamp_regex)) {
                std::cout << "Matched next timestamp: " << match[1].str() << std::endl;
                next_time = parseTimestamp(match[1].str());
            } else {
                std::cout << "Failed to match next timestamp in line: " << linesMap[currencyPairs[connection_indx]][pss->last + 1] << std::endl;
            }

            long long time_diff = 0;
            if (current_has_timestamp) {
                // Calculate the difference between the current and next timestamp
                time_diff = std::chrono::duration_cast<std::chrono::microseconds>(next_time - current_time).count();
                if (time_diff < 0) {
                    time_diff = 0;
                }
            }

            // Print current and next time
            std::cout << "CURRENT_TIME: " << (current_has_timestamp ? std::chrono::duration_cast<std::chrono::microseconds>(current_time.time_since_epoch()).count() : 0) << std::endl;
            std::cout << "NEXT_TIME: " << std::chrono::duration_cast<std::chrono::microseconds>(next_time.time_since_epoch()).count() << std::endl;
            std::cout << "TIMEDIFF: " << time_diff << std::endl;

            // Update the timestamp field with the current timestamp using regex
            std::string new_timestamp = "\"timestamp\":\"" + getCurrentTimestamp() + "\"";
            line_to_send = std::regex_replace(line_to_send, timestamp_regex, new_timestamp);
			std::cout << "LINE_TO_SEND: " << line_to_send << std::endl;

            size_t message_length = line_to_send.size();
            memcpy(&buf[LWS_PRE], line_to_send.c_str(), message_length);

            // Send data using lws_write
            int written = lws_write(wsi, &buf[LWS_PRE], message_length, LWS_WRITE_TEXT);

            // Increment the line counter for connection
			pss->last = pss->last + 1;

            // Set the timer for the next line
            lws_set_timer_usecs(wsi, time_diff); 
        } else {
            lwsl_user("All lines sent\n");
        }
		break;
	}
	default:
		break;
	}

	return 0;
}

static struct lws_protocols protocols[] = {
	{ "timer", callback_protocol, sizeof(struct per_session_data__minimal), 128, 0, NULL, WEBSOCKET_SERVER_TX_BUFFER_SIZE},
	LWS_PROTOCOL_LIST_TERM
};

static const lws_retry_bo_t retry = {
	.secs_since_valid_ping = 3,
	.secs_since_valid_hangup = 10,
};

static int interrupted;

void sigint_handler(int sig)
{
	interrupted = 1;
}

int main(int argc, const char **argv)
{	
	readLinesFromFiles();
	struct lws_context_creation_info info;
	struct lws_context *context;
	const char *p;
	int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
			/* for LLL_ verbosity above NOTICE to be built into lws,
			 * lws must have been configured and built with
			 * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE */
			/* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
			/* | LLL_EXT */ /* | LLL_CLIENT */ /* | LLL_LATENCY */
			/* | LLL_DEBUG */;

	signal(SIGINT, sigint_handler);

	lws_set_log_level(logs, NULL);
	lwsl_user("LWS minimal ws server\n");

	memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
	info.fd_limit_per_thread = 1024; // Or any appropriate value

	info.port = 7681;
	info.protocols = protocols;
	info.vhost_name = "localhost";
	info.options =
		LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;


	lwsl_user("Server using TLS\n");
	info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
	info.ssl_cert_filepath = "../../mock-exchange.cert";
	info.ssl_private_key_filepath = "../../mock-exchange.key";

	context = lws_create_context(&info);
	if (!context) {
		lwsl_err("lws init failed\n");
		return 1;
	}

	

	while (n >= 0 && !interrupted)
		n = lws_service(context, 0);

	lws_context_destroy(context);

	return 0;
}
