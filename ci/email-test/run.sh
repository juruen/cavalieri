#!/bin/bash -x

set -e

cleanup() {
  pkill cavalieri || true
  pkill send-events.py || true

  rm -rf cavalieri-rules || true

  sudo sh -c "cat > /var/mail/ubuntu"
}

git clone https://github.com/juruen/cavalieri-rules.git

cp rules-mail.cpp cavalieri-rules/rules.cpp
cd cavalieri-rules && mkdir build && cd build && cmake .. && make

cavalieri -v 0 -rules_directory . >/dev/null 2>&1 &

cd ../..

sleep 5

./send-events.py 800 &

for i in $(seq 0 120); do

  EMAILS=$(sudo sh -c 'grep "^To:" /var/mail/ubuntu | wc -l') || true

  echo "EMAILS: ${EMAILS}"

  if [ "$EMAILS" == "800" ]; then
    cleanup
    exit 0
  fi

  sleep 1

done

echo "email test failed"

cleanup

exit 1
