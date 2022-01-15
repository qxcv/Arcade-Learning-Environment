#!/bin/env bash

set -euo pipefail

echo Building ALE library
cd ../build/
cmake --build . --target install

echo Building fuzzable simulator
cd ../fuzzable-sim/build
cmake --build .
