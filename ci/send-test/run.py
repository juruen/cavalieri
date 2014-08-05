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


def send(n, msgs):
    raw = encoded_events(n)

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('localhost', 5555))

    buff = struct.pack('!I', len(raw)) + raw

    for i in range(0, msgs):
        sock.sendall(buff)

    received = 0
    for i in range(0, msgs):
        rxlen = struct.unpack('!I', sock.recv(4, socket.MSG_WAITALL))[0]
        msg = bernhard.Message(raw=sock.recv(rxlen, socket.MSG_WAITALL))
        assert(msg.ok)
        received += 1
        print "ok: %i" % received

send(1, 170000)
