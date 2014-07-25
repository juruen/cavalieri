#!/usr/bin/env python

import multiprocessing
import subprocess
import sys
import time

client_processes = 5
ws_processes = 20;
total_processes = client_processes + ws_processes

events = "5000"
ws_client = "./ws-read-events.py"
riemann_client = "./generate-events.py"

def work(i):
    if i < client_processes:
        time.sleep(2)
        return subprocess.call([riemann_client], shell=False)
    else:
        return subprocess.call([ws_client,  "localhost:5556",  events],
                shell=False)

pool = multiprocessing.Pool(processes=total_processes)
result = pool.map(work, range(total_processes))

print result

sys.exit(len([i for i in result if i > 0]))
