#!/usr/bin/env python

import bernhard
import datetime
import sys
import random

batch_size = 1

class BatchClient(bernhard.Client):
    def send(self, events):
          message = bernhard.Message(events=[bernhard.Event(params=e) for e in events])
          response = self.transmit(message)
          return response.ok

def create_event(i):
    global services
    return {
        'host': "bar.com",
        'service': "foo-%i" %i,
        'metric': random.random(),
        'description': str(datetime.datetime.now()),
        'tags': ["stress-test"],
        'attributes' : { 'foo' : 'bar'}
    }

c = BatchClient(host='localhost')

for i in range(0, 1000):
    event = create_event(i)
    print event
    c.send([event])
