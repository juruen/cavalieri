#!/usr/bin/env python

import websocket
import urllib
import json
import sys


def ws_url(host):
    ws_host = "ws://%s/index?" % host
    q = {'subscribe' : 'true', 'query' : '(metric>0.5)'}
    return ws_host + urllib.urlencode(q)

def main():
    if len(sys.argv) < 3:
        sys.stderr.write("usage %s: host num_events\n" % sys.argv[0])
        sys.exit(1)

    ws = websocket.create_connection(ws_url(sys.argv[1]))

    try:
        for i in range(int(sys.argv[2])):
            ws.recv()
    except:
        sys.exit(1)

    ws.close()

    sys.exit(0)

main()
