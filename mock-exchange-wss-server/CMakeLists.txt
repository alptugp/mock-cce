project(mock-exchange-wss-server)
cmake_minimum_required(VERSION 3.5)

option(USE_PORTFOLIO_122 "Use the optimized portfolio with 122 currency pairs" OFF)
option(USE_PORTFOLIO_92 "Use the optimized portfolio with 92 currency pairs" OFF)
option(USE_PORTFOLIO_50 "Use the optimized portfolio with 50 currency pairs" OFF)
option(USE_PORTFOLIO_3 "Use the optimized portfolio with 3 currency pairs" OFF)

option(USE_KRAKEN_MOCK_EXCHANGE "Use Kraken Mock Exchange" OFF)
option(USE_BITMEX_MOCK_EXCHANGE "Use BitMEX Mock Exchange" OFF)

if (USE_PORTFOLIO_122)
    add_definitions(-DUSE_PORTFOLIO_122)
elseif (USE_PORTFOLIO_92)
    add_definitions(-DUSE_PORTFOLIO_92)
elseif (USE_PORTFOLIO_50)
    add_definitions(-DUSE_PORTFOLIO_50)
elseif (USE_PORTFOLIO_3)
    add_definitions(-DUSE_PORTFOLIO_3)
else()
    message(FATAL_ERROR "You must specify one of the portfolio options.")
endif()

if (USE_KRAKEN_MOCK_EXCHANGE)
    add_definitions(-DUSE_KRAKEN_MOCK_EXCHANGE)
elseif (USE_BITMEX_MOCK_EXCHANGE)
    add_definitions(-DUSE_BITMEX_MOCK_EXCHANGE)
else()
    message(FATAL_ERROR "You must specify one of the exchange options.")
endif()

find_package(libwebsockets CONFIG REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${LWS_CMAKE_DIR})
include(CheckCSourceCompiles)
include(LwsCheckRequirements)

set(requirements 1)
require_lws_config(LWS_ROLE_WS 1 requirements)
require_lws_config(LWS_WITH_SERVER 1 requirements)

# Source Files
set(SOURCES
    mock-exchange-wss-server.cpp
)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Compiler Features and Options
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wno-unused-function -Wno-unused-parameter)

# Linked Libraries
target_link_libraries(${PROJECT_NAME} PRIVATE pthread crypto ssl websockets ${Boost_LIBRARIES})
