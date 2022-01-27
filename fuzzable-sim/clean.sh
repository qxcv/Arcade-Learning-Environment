#!/usr/bin/env bash

set -xeuo pipefail

make -C ../AFL clean
rm -fr ./build ../build
