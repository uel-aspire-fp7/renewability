#!/bin/bash

#set -o errexit
set -o pipefail

BASE_DIRECTORY=$(dirname $0)
MANAGER_PLATFORM=serverlinux

cd ${BASE_DIRECTORY}
TARGET_DIRECTORY=$1

echo "Copying headers files to headers/..."

mkdir -p ${TARGET_DIRECTORY}/headers/{client,server}
cp src/client/*.h ${TARGET_DIRECTORY}/headers/client/
cp src/server/*.h ${TARGET_DIRECTORY}/headers/server/

echo "Building client-side support object files..."

mkdir -p ${TARGET_DIRECTORY}/logs
for PLATFORM in linux android serverlinux; do
    cd src/client
    make -f Makefile.${PLATFORM} clean && make -f Makefile.${PLATFORM} >> ${TARGET_DIRECTORY}/logs/client-support.log 2>> ${TARGET_DIRECTORY}/logs/client-support.err

    echo "Copying client-side object file to obj/${PLATFORM}"

    cd ../..
    mkdir -p ${TARGET_DIRECTORY}/obj/${PLATFORM}
    cp src/client/renewability.{a,o} ${TARGET_DIRECTORY}/obj/${PLATFORM}
done

echo "Building server-side support..."
RN_PID=$(pidof renewability_manager)

if [ "${RN_PID}" != "" ]; then
    kill -9 ${RN_PID}
fi

pidof renewability_manager

cd src/server
make -f Makefile.${MANAGER_PLATFORM} clean && make -f Makefile.${MANAGER_PLATFORM} > ${TARGET_DIRECTORY}/logs/server-support.log 2> ${TARGET_DIRECTORY}/logs/server-support.err

echo "Copying renewability_manager to obj/..."

cd ../..
mkdir -p ${TARGET_DIRECTORY}/obj/${MANAGER_PLATFORM}
cp src/server/renewability_manager ${TARGET_DIRECTORY}/obj/${MANAGER_PLATFORM}

echo "Done. For additional info, please refer to {client,server}-support.{log,err}."
