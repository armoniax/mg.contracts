#!/usr/bin/env bash
set -eo pipefail

# Source helper functions and variables.
. ./scripts/.environment
. ./scripts/helper.sh

# Prompt user for location of amax.cdt.
cdt-directory-prompt

# Include CDT_INSTALL_DIR in CMAKE_FRAMEWORK_PATH
echo "Using EOSIO.CDT installation at: $CDT_INSTALL_DIR"
export CMAKE_FRAMEWORK_PATH="${CDT_INSTALL_DIR}:${CMAKE_FRAMEWORK_PATH}"


printf "\t=========== Building amax.contracts ===========\n\n"
RED='\033[0;31m'
NC='\033[0m'
CPU_CORES=$(getconf _NPROCESSORS_ONLN)
mkdir -p build
pushd build &> /dev/null
cmake -DBUILD_TESTS=${BUILD_TESTS} ../

# cmake .. \
#   -DCMAKE_C_COMPILER=${CDT_INSTALL_DIR}/bin/amax-cc \
#   -DCMAKE_CXX_COMPILER=${CDT_INSTALL_DIR}/bin/amax-cpp \
#   -DBUILD_TESTS=${BUILD_TESTS}
make -j $CPU_CORES
popd &> /dev/null
