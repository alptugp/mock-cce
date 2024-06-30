# Mock Centralized Cryptocurrency Exchange (CCE)

## Overview
The Mock CCE is designed to emulate the exchanges Kraken and Bitmex. It enables consistent and reproducible latency measurement experiments for PublicHFT by running both a WebSocket (WSS) server and a REST server. This controlled environment ensures the exact same market conditions for each experiment, mitigating the impact of random and unpredictable latency from live exchanges and allowing for precise performance evaluation of the trading system.

### WebSocket Server
The WebSocket server simulates the OrderBookL2 market updates published by the exchange.

#### Functionality
The mock exchange’s WSS server handles one secure connection per desired order book, transmitting historical OrderBookL2 market data for each order book in JSON format. This historical data includes various triangular arbitrage opportunities and was recorded from the real-time OrderBookL2 messages sent by Kraken’s WSS API during a 10-minute interval after subscribing to the maximum number of order books/currency pairs possible. Each order book’s or currency pair’s historical data is stored in a separate file.

When sending a market message, the WSS server replaces Kraken’s timestamp field value in the JSON object with the current time in microsecond resolution. After the server sends a market update to one of PublicHFT’s clients, it uses a timer callback to send the next JSON message after the time difference between the previously sent Kraken timestamp and the next one in the file has elapsed. This setup allows us to replicate the message rate for each order book in the recorded data. The WSS server was developed using the libwebsockets library, chosen for its robust timer callback functionality, which is separately managed for each client within its built-in event loop.

#### Dependencies
- **libwebsockets**
- **pthread**
- **crypto**
- **ssl**
- **nlohmann** (JSON library)

#### Build the WebSocket Server
Set up the build environment using the provided build script. You need to specify the portfolio and exchange options:

    ```bash
    cd mock-exchange-wss-server
    ./build_script.sh -p USE_PORTFOLIO_3 -e USE_KRAKEN_MOCK_EXCHANGE
    ```

#### Exchange Options:
- **To emulate Kraken**: `USE_KRAKEN_MOCK_EXCHANGE`
- **To emulate Bitmex**: `USE_BITMEX_MOCK_EXCHANGE`

#### Portfolio Options:
These portfolios are the optimized portfolios used by [PublicHFT](https://github.com/alptugp/PublicHFT):
- **122 Order Books**: `USE_PORTFOLIO_122`
- **92 Order Books**: `USE_PORTFOLIO_92`
- **50 Order Books**: `USE_PORTFOLIO_50`
- **3 Order Books**: `USE_PORTFOLIO_3`

### REST Server
The REST server simulates the order handling of the exchange.

#### Functionality
The REST API server of the mock exchange was implemented using Boost.Asio and OpenSSL to handle secure HTTP requests for the orders. For each experiment, it simultaneously handles three client connections in total for the arbitrage legs. Whenever it receives a POST request to add an order, it responds with a JSON object containing the timestamp of when the request was received and processed, representing the exchange execution timestamp.

#### Dependencies
- **Boost** (system, thread)
- **OpenSSL**

#### Build the REST Server
Set up the build environment:

    ```bash
    cd mock-exchange-rest-server
    mkdir build
    cd build
    cmake ..
    make
    ```

### Run the Mock CCE

1. Start the WebSocket server:

    ```bash
    cd mock-exchange-wss-server/build
    ./mock-exchange-wss-server 
    ```

2. Start the REST server:

    ```bash
    cd mock-exchange-rest-server/build
    ./mock-exchange-rest-server <port> ../mock-exchange.cert ../mock-exchange.key
    ```

By following these steps, you will have the Mock CCE running on your local machine, providing a consistent environment for your HFT system experiments.
