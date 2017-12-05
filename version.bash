#!/bin/bash
# 
# File:   version.bash
# Author: seongjinoh
#
# Created on Sep 7, 2017, 10:46:47 AM
#

export BUILD_DIR="./Build"
export ORIGIN_BIN="main.bin"
export TARGET_BIN=""
export MODEL_INFO=""
export VERSION_INFO=""
VERSION_DELIMITER="-"

export MODEL_FILE=./Src/modelInfo.c
export VERSION_FILE=./Src/versionInfo.c

export MODEL_TOKEN="MODEL_NAME"
export VERSION_TOKEN="REVISION_INFO"

MODEL_INFO=`grep ${MODEL_TOKEN} ${MODEL_FILE} | awk '{print $3}' | sed 's/"//g' | sed -n '1p' `
VERSION_INFO=`grep ${VERSION_TOKEN} ${VERSION_FILE} | awk '{print $3}' | sed 's/"//g' | sed -n '1p' `

TARGET_BIN="${MODEL_INFO}${VERSION_DELIMITER}${VERSION_INFO}.bin"

cp $BUILD_DIR/${ORIGIN_BIN} $BUILD_DIR/${TARGET_BIN}
echo "[ ${TARGET_BIN} ] is copied."

# unset variables
unset BUILD_DIR
unset ORIGIN_BIN
unset TARGET_BIN
unset MODEL_INFO
unset VERSION_INFO
unset VERSION_DELIMITER
unset MODEL_FILE
unset VERSION_FILE

echo "Done."