#!/bin/sh

set -x

SOURCE_DIR=`pwd`
BUILD_DIR=${BUILD_DIR:-./build}
EXECUTE_DIR=${BUILD_DIR:-./bin}
BUILD_TYPE=${BUILD_TYPE:-Debug}
rm -rf $BUILD_DIR 
rm -rf $EXECUTE_DIR
mkdir -p $BUILD_DIR/$BUILD_TYPE \
    && cd $BUILD_DIR/$BUILD_TYPE \
    && cmake \
            -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
            $SOURCE_DIR \
    && make $*