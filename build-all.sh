#!/bin/bash
# Build script for rebuilding everything
set echo on

echo "Building everything..."

make -f Makefile.Engine.linux.mak all
ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then
echo "Error:"$ERRORLEVEL && exit
fi

make -f Makefile.Testbed.linux.mak all
ERRORLEVEL=$?
if [ $ERRORLEVEL -ne 0 ]
then
echo "Error:"$ERRORLEVEL && exit
fi

pushd Shaders
source post-build.sh
popd

echo "All assemblies built successfully."