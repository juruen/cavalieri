#include <glog/logging.h>
#include <thread>
#include <mailer.h>
#include <util.h>
#include <python_interpreter.h>

streams_t email(const std::string & server, const std::string & from,
                const std::string & to)
{
  return create_stream(

    [=](forward_fn_t, const Event & event)
    {

      Event me(event);

      Attribute* attr = me.add_attributes();
      attr->set_key("smtp_server");
      attr->set_value(server);

      attr = me.add_attributes();
      attr->set_key("from_addr");
      attr->set_value(from);

      attr = me.add_attributes();
      attr->set_key("to_addr");
      attr->set_value(to);

      const std::string jsonstr = event_to_json(me);

      g_python_runner.run_function("mailer", "send_email", event_to_json(me));

  });
}
