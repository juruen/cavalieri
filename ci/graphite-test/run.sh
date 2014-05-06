#!/bin/bash -x

set -e

cleanup() {
  pkill cavalieri || true
  pkill graphite-read-events.sh || true
  pkill tcpserver || true
  pkill send-events.py || true

  rm -rf *.out || true

  rm -rf rules-forward rules-graphite || true
}

git clone https://github.com/juruen/cavalieri-rules.git

cp -a cavalieri-rules rules-forward
cp -a cavalieri-rules rules-graphite

cp rules-forward.cpp rules-forward/rules.cpp
cd rules-forward && mkdir build && cd build && cmake .. && make

cavalieri -v 0 -rules_directory . &

cd ../..

cp rules-graphite.cpp rules-graphite/rules.cpp
cd rules-graphite && mkdir build && cd build && cmake .. && make

cavalieri -v 0 -rules_directory . -events_port 15555 -ws_port 15556 &

cd ../..

./graphite-read-events.sh &

touch start.out

sleep 5

./send-events.py &

for i in $(seq 0 60); do

  LINES=$(wc -l *.out | tail -n 1)

  echo "LINES: ${LINES}"

  if [ "$LINES" == " 1000 total" ]; then
    cleanup
    exit 0
  fi

  sleep 1

done

echo "graphite-forward test failed"

cleanup

exit 1
