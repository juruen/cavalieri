#!/usr/bin/python

import bernhard
import socket
import struct
import time
import sys

def events(n):
    events = list()
    for i in range(0, n):
        events.append(bernhard.Event(params={'host': "host-%i" % i,
                                             'service' : 'service-foo',
                                             'tags': ['foo', 'bar', 'baz']}))
    return events


def encoded_events(n):
    msg = bernhard.Message(events=events(n))
    return msg.raw


def send(n):
    raw = encoded_events(n)

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('localhost', 5555))

    buff = struct.pack('!I', len(raw)) + raw

    sock.sendall(buff[0:200])
    time.sleep(0.1)
    sock.sendall(buff[200:10000])
    time.sleep(0.3)
    sock.sendall(buff[10000:15000])
    time.sleep(0.5)
    sock.sendall(buff[15000:len(buff)])

    rxlen = struct.unpack('!I', sock.recv(4))[0]
    msg = bernhard.Message(raw=sock.recv(rxlen, socket.MSG_WAITALL))
    assert(msg.ok)

send(5000)
