import json
import smtplib
from email.mime.text import MIMEText

def send_email(json_str):
    try:
        obj = json.loads(json_str)

        description = "%s %s is %s ( %s )" % (
                obj.get('host', 'unknown host'),
                obj.get('service', 'unknown service'),
                obj.get('state', 'unknown state'),
                obj.get('metric', 'nil'))

        smtp_server = obj.pop('smtp_server')
        from_addr = obj.pop('from_addr')
        to_addr = obj.pop('to_addr')

        msg = MIMEText(str(obj))
        msg['Subject'] = description
        msg['From'] = from_addr
        msg['To'] = to_addr

        server = smtplib.SMTP(smtp_server)
        server.set_debuglevel(1)
        server.sendmail(from_addr, to_addr, msg.as_string())
        server.quit()
    except:
        print "failed to send email"
