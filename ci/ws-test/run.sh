#!/bin/bash -x

set -e

# Build rules to index everything
git clone https://github.com/juruen/cavalieri-rules.git
cd cavalieri-rules
git checkout -b index-everything origin/index-everything
mkdir build && cd build && cmake .. && make

# Run cavalieri in background
cavalieri 2>/dev/null &

# Run integration test
cd ../../
if ! timeout 30 ./test-ws.py; then
  echo "websocket test failed"
  pkill cavalieri
  exit 1
fi

# Kill cavalieri
pkill cavalieri
