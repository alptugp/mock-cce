cmake_minimum_required(VERSION 3.10)

# Set the project name
project(mock-exchange-rest-server)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(mock-exchange-rest-server mock-exchange-rest-server.cpp)

# Find Boost and OpenSSL
find_package(Boost 1.66 REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)

# Include directories for Boost and OpenSSL
include_directories(${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})

# Link the necessary libraries
target_link_libraries(mock-exchange-rest-server ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)
