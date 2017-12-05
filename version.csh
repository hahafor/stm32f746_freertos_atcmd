#!/bin/csh
# 
# File:   version.csh
# Author: seongjinoh
#
# Created on Sep 7, 2017, 11:43:48 AM
#

set BUILD_DIR="./Build"
set ORIGIN_BIN="main.bin"
set VERSION_DELIMITER="-"

set MODEL_FILE="./Src/modelInfo.c"
set VERSION_FILE="./Src/versionInfo.c"

set MODEL_TOKEN="MODEL_NAME"
set VERSION_TOKEN="REVISION_INFO"

set MODEL_INFO = `grep ${MODEL_TOKEN} ${MODEL_FILE} | awk '{print $3}' | sed 's/"//g' | sed -n '1p' `
set VERSION_INFO = `grep ${VERSION_TOKEN} ${VERSION_FILE} | awk '{print $3}' | sed 's/"//g' | sed -n '1p' `

set TARGET_BIN = "$MODEL_INFO""$VERSION_DELIMITER""$VERSION_INFO.bin"

echo "MODEL_INFO = $MODEL_INFO" 
echo "VERSION_INFO = $VERSION_INFO"
echo "TARGET_BIN = $TARGET_BIN"

cp $BUILD_DIR/$ORIGIN_BIN $BUILD_DIR/$TARGET_BIN
echo "[ $TARGET_BIN ] is copied."

# unset variables
unsetenv BUILD_DIR
unsetenv ORIGIN_BIN
unsetenv TARGET_BIN
unsetenv MODEL_INFO
unsetenv VERSION_INFO
unsetenv VERSION_DELIMITER
unsetenv MODEL_FILE
unsetenv VERSION_FILE

echo "Done."