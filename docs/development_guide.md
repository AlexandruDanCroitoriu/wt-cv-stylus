# Development Guide

## Introduction
This guide provides detailed instructions on how to further develop this WT (C++) web application, including setting up the development environment, understanding the project structure, and following best practices.

## Project Structure

## Setup Instructions

### Debug Build
```sh
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build/debug && make -j$(nproc)
```
### Release Build
```sh
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build/release && make -j$(nproc)
```
### running the application
To run the application, use the following command from the desierd build directory:
```sh
./app --docroot ../../ -c ../../wt_config.xml --http-address 0.0.0.0 --http-port 9020
```

## Development Workflow

## Coding Standards

## Troubleshooting

