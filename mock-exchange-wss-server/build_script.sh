#!/bin/bash

USE_PORTFOLIO=""
USE_EXCHANGE="USE_KRAKEN_MOCK_EXCHANGE"

# Parse command-line arguments
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -p|--portfolio)
    USE_PORTFOLIO="$2"
    shift # past argument
    shift # past value
    ;;
    -e|--exchange)
    USE_EXCHANGE="$2"
    shift # past argument
    shift # past value
    ;;
    *)    # unknown option
    echo "Unknown option: $key"
    exit 1
    ;;
esac
done

# Check if portfolio option is set
if [[ -z "$USE_PORTFOLIO" ]]; then
    echo "Error: An option must be specified for portfolio using -p or --portfolio."
    exit 1
fi

# Remove the build directory
rm -r build

# Recreate the build directory
mkdir build

# Change to the build directory
cd build

# Run cmake with the specified options
cmake -D$USE_PORTFOLIO=ON -D$USE_EXCHANGE=ON ..

# Run make
make
