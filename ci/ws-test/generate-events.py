#!/usr/bin/env python

import bernhard
import datetime
import sys
import random

batch_size = 1000

class BatchClient(bernhard.Client):
    def send(self, events):
          message = bernhard.Message(events=[bernhard.Event(params=e) for e in events])
          response = self.transmit(message)
          return response.ok

def create_event(i):
    global services
    return {
        'host': "foo%i.com" % i  ,
        'service': "requests_rate",
        'metric': 100,
        'description': str(datetime.datetime.now()),
        'tags': ["stress-test"],
        'ttl': 60,
        'attributes' : { 'foo' : 'bar'}
    }

c = BatchClient(host='localhost')

c.send([create_event(i) for i in range(batch_size)])

sys.exit(0)
