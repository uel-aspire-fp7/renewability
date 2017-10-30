#!/bin/bash
set -o errexit
set -o pipefail
set -o nounset
#set -o xtrace

# Get the repo and build directories
repo_dir=$(dirname $0)
build_dir=$1
mkdir -p $build_dir

echo "Building server-side support..."
cd ${repo_dir}/src/server
make -f Makefile ASCL=/opt/ASCL SRCDIR=$repo_dir/src

mv ${repo_dir}/src/server/renewability_manager $build_dir
