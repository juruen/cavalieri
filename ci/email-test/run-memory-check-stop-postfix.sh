#!/bin/bash -x

set -e

cleanup() {
  pkill send-events.py || true

  rm -rf cavalieri-rules || true

  sudo sh -c "cat > /var/mail/ubuntu"

}

git clone https://github.com/juruen/cavalieri-rules.git

cp rules-mail.cpp cavalieri-rules/rules.cpp
cd cavalieri-rules && mkdir build && cd build && cmake .. && make

sudo /etc/init.d/postfix stop || true

export LD_LIBRARY_PATH=$(pwd)

valgrind --error-exitcode=1 --show-possibly-lost=no \
          --workaround-gcc296-bugs=yes  --log-file=../../valgrind.out \
          cavalieri -v 0 -rules_directory .  -index_expire_interval 10000 &

valgrind_pid=$!

cd ../..

sleep 5

./send-events.py 100 &

sleep 30

kill -INT $valgrind_pid

if ! wait $valgrind_pid; then

  echo "valgrind reported an erro"
  cat valgrind.out

  exit 1

fi

grep -q "definitely lost: 0 bytes" valgrind.out \
     || (cat valgrind.out && false)

cleanup

exit 0
