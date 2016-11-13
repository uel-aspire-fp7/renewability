#!/bin/bash

#set -o errexit
set -o pipefail

BASE_DIRECTORY=$(dirname $0)
MANAGER_PLATFORM=serverlinux

cd ${BASE_DIRECTORY}

echo "Copying headers files to headers/..."

cp src/client/*.h headers/client/
cp src/server/*.h headers/server/

echo "Building client-side support object files..."

for PLATFORM in linux android serverlinux; do
    cd src/client
    make -f Makefile.${PLATFORM} clean && make -f Makefile.${PLATFORM} >> ../../logs/client-support.log 2>> ../../logs/client-support.err

    echo "Copying client-side object file to obj/${PLATFORM}"

    cd ../..
    cp src/client/renewability.{a,o} obj/${PLATFORM}
done

echo "Building server-side support..."
RN_PID=$(pidof renewability_manager)

if [ "${RN_PID}" != "" ]; then
    kill -9 ${RN_PID}
fi

pidof renewability_manager

cd src/server
make -f Makefile.${MANAGER_PLATFORM} clean && make -f Makefile.${MANAGER_PLATFORM} > ../../logs/server-support.log 2> ../../logs/server-support.err

echo "Copying renewability_manager to obj/..."

cd ../..
cp src/server/renewability_manager obj/${MANAGER_PLATFORM}

echo "Done. For additional info, please refer to {client,server}-support.{log,err}."
