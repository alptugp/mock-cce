# Mock Centralized Cryptocurrency Exchange (CCE)

## Overview
The Mock CCE is designed to emulate the exchanges Kraken and Bitmex. It enables consistent and reproducible latency measurement experiments for PublicHFT by running both a WebSocket (WSS) server and a REST server. This controlled environment ensures the exact same market conditions for each experiment, mitigating the impact of random and unpredictable latency from live exchanges and allowing for precise performance evaluation of the trading system.


## Getting Started
Follow these instructions to set up and run the Mock CCE on your local machine or a VM.

### Installation

1. Clone the repository:

    ```bash
    git clone https://github.com/yourusername/mock-cce.git
    ```

2. Navigate to the project directory:

    ```bash
    cd mock-cce
    ```
   
## WebSocket Server
The WebSocket server simulates the market updates from the exchange.

#Build the WebSocket Server
Set up the build environment using the provided build script. You need to specify the portfolio and exchange options:
    ```bash
    cd mock-exchange-wss-server
    ./build_script.sh -p USE_PORTFOLIO_3 -e USE_KRAKEN_MOCK_EXCHANGE
    ```

## REST Server
The REST server simulates the order handling of the exchange.

#Build the REST Server
Set up the build environment:
    ```bash
    cd mock-exchange-rest-server
    mkdir build
    cd build
    cmake ..
    make
    ```
## Run the Mock CCE
1. Start the WebSocket server:

    ```bash
    cd mock-exchange-wss-server
    ./build/mock-exchange-wss-server
    ```

2. Start the REST server:

    ```bash
    cd mock-exchange-rest-server
    ./build/mock-exchange-rest-server
    ```

