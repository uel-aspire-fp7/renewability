#!/bin/bash
set -o errexit
set -o pipefail
set -o nounset
#set -o xtrace

# Get the repo and build directories, go to the build directory
repo_dir=$(dirname $0)
build_dir=$1
mkdir -p $build_dir
cd $build_dir

# Create extra symlinks
ln -s $repo_dir/scripts/ $build_dir
ln -s $repo_dir/setup/ $build_dir

echo "Copying headers files to headers/..."

mkdir -p ${build_dir}/headers/{client,server}
ln -s ${repo_dir}/src/client/*.h ${build_dir}/headers/client
ln -s ${repo_dir}/src/server/*.h ${build_dir}/headers/server

echo "Building client-side support object files..."

mkdir -p ${build_dir}/logs
cd ${repo_dir}/src/client
for PLATFORM in linux android serverlinux; do
    make -f Makefile.${PLATFORM} clean && make -f Makefile.${PLATFORM} >> ${build_dir}/logs/client-support.log 2>> ${build_dir}/logs/client-support.err

    echo "Copying client-side object file to obj/${PLATFORM}"

    mkdir -p ${build_dir}/obj/${PLATFORM}
    mv ${repo_dir}/src/client/renewability.{a,o} ${build_dir}/obj/${PLATFORM}
done

echo "Building server-side support..."
MANAGER_PLATFORM=serverlinux

cd ${repo_dir}/src/server
make -f Makefile.${MANAGER_PLATFORM} clean && make -f Makefile.${MANAGER_PLATFORM} > ${build_dir}/logs/server-support.log 2> ${build_dir}/logs/server-support.err

echo "Copying renewability_manager to obj/..."

mkdir -p ${build_dir}/obj/${MANAGER_PLATFORM}
mv ${repo_dir}/src/server/renewability_manager ${build_dir}/obj/${MANAGER_PLATFORM}

echo "Done. For additional info, please refer to {client,server}-support.{log,err}."
