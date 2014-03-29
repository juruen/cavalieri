import json
import urllib2

PD_URL = "https://events.pagerduty.com/generic/2010-04-15/create_event.json"
TIMEOUT = 10

def request(action, json_str):
    obj = json.loads(json_str)

    description = "%s %s is %s ( %s )" % (
            obj.get('host', 'unknown host'),
            obj.get('service', 'unknown service'),
            obj.get('state', 'unknown state'),
            obj.get('metric', 'nil'))

    pg_key = obj.pop('pg_key')
    event = {
        'service_key': pg_key,
        'event_type': action,
        'incident_key': "%s %s" % (obj['host'], obj['service']),
        'description': description,
        'details': json.dumps(obj)
    }
    try:
        result = json.loads(
                urllib2.urlopen(PD_URL, json.dumps(event), TIMEOUT).read())
        print result
    except Exception, e:
        print str(e)
        return False

    return result['status'] == 'success'

def trigger(json_str):
    return request('trigger', json_str)

def acknowledge(json_str):
    return request('acknowledge', json_str)

def resolve(json_str):
    return request('resolve', json_str)
