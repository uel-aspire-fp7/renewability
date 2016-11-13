#!/bin/bash

set -o errexit

cd $(dirname $0)/src
make clean all 2>/dev/null

echo -e "\n###################################"
echo "SAMPLE TEST OUTPUT (ctrl+c) to stop"
echo -e "###################################\n"
./rntest