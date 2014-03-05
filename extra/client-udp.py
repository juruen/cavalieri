#!/usr/bin/env python

import bernhard
import datetime
import sys
import random

batch_size = 1
services = ["test1," "test2", "test3", "foo", "bar", "baz", "xyzzy", "attack", "cat", "treat"]

class BatchClient(bernhard.Client):
    def send(self, events):
          message = bernhard.Message(events=[bernhard.Event(params=e) for e in events])
          response = self.transmit(message)
          return response.ok

def create_event(i):
    global services
    return {
        'host': "host%i" %i ,
        'service': services[i % len(services)],
        'metric': random.random(),
        'description': str(datetime.datetime.now()),
        'tags': ["stress-test"],
        'attributes' : { 'foo' : 'bar'}
    }

c = BatchClient(host='localhost', transport=bernhard.UDPTransport)

#while True:
c.send([create_event(i) for i in range(batch_size)])
