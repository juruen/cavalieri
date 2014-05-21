#!/bin/bash

tcpserver 0 2003 sh -c 'cat > graphite-$TCPREMOTEPORT.out'
