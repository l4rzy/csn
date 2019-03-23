#!/bin/bash

export LD_LIBRARY_PATH=./build
echo "=> Testing buf_t functions"
./buf
echo "=> Testing queue functions"
./queue
echo "=> Testing xpath functions"
./xpath
