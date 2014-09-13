#include <glog/logging.h>
#include <util/util.h>
#include <mailer_pool.h>

using namespace std::placeholders;

namespace {

struct mailer_extra {
  const std::string server;
  const std::string from;
  const std::vector<std::string> to;
};

typedef std::pair<std::vector<char>, size_t> payload_t;

size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{

  if((size == 0) || (nmemb == 0) || ((size * nmemb) < 1)) {
    return 0;
  }

  auto payload_data = static_cast<payload_t*>(userp);

  size_t copied = payload_data->second;
  size_t left = payload_data->first.size() - copied;
  size_t to_copy = std::min(size * nmemb, left);

  if (to_copy > 0) {

    memcpy(ptr, static_cast<void*>(&payload_data->first[copied]), to_copy);
    payload_data->second += to_copy;

    return to_copy;

  } else {
    return 0;
  }

}

std::vector<char> payload_text(const mailer_extra extra, const Event & e) {

  std::string to = "unknown";

  if (!extra.to.empty()) {
    to = extra.to[0];
  }

  std::string subject = e.host() + " " + e.service() + " is " + e.state()
                        + e.metric_to_str();

  std::string payload = "To: " + to + "\r\n" +
                        "From: " + extra.from + "\r\n" +
                        "Subject: " + subject + "\r\n" +
                        "\r\n" +
                        e.json_str();

  return std::vector<char>(payload.begin(), payload.end());
}

}

mailer_pool::mailer_pool(const size_t thread_num, const bool enable_debug)
  :
  curl_pool_(
      thread_num,
      std::bind(&mailer_pool::curl_event, this, _1, _2, _3)
  ),
  enable_debug_(enable_debug)
{
}

void mailer_pool::push_event(const std::string server, const std::string from,
                             const std::vector<std::string> to,
                             const Event & event)
{
  VLOG(3) << "push_event()  server: " << server << " from: " << from;

  mailer_extra extra = {server, from, to};
  curl_pool_.push_event(event, extra);

}

void mailer_pool::curl_event(const queued_event_t queued_event,
                             const std::shared_ptr<CURL> easy,
                             std::function<void()> & clean_fn)
{

  const auto extra = boost::any_cast<mailer_extra>(queued_event.extra);
  const auto & e = queued_event.event;

  std::shared_ptr<std::string> server(
      new std::string("smtp://" + extra.server));

  std::shared_ptr<std::string> from(new std::string(extra.from));

  struct curl_slist *recipients = NULL;
  for (const auto & to : extra.to) {
    recipients = curl_slist_append(recipients, to.c_str());
  }

  auto payload = std::make_shared<payload_t>(
                                      payload_t({payload_text(extra, e), 0}));

  curl_easy_setopt(easy.get(), CURLOPT_URL, server->c_str());
  curl_easy_setopt(easy.get(), CURLOPT_MAIL_FROM, from->c_str());
  curl_easy_setopt(easy.get(), CURLOPT_MAIL_RCPT, recipients);
  curl_easy_setopt(easy.get(), CURLOPT_UPLOAD, 1L);
  curl_easy_setopt(easy.get(), CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
  curl_easy_setopt(easy.get(), CURLOPT_READFUNCTION, payload_source);
  curl_easy_setopt(easy.get(), CURLOPT_READDATA, payload.get());
  curl_easy_setopt(easy.get(), CURLOPT_VERBOSE, enable_debug_);

  clean_fn = [=]()
  {
    UNUSED_VAR(server);
    UNUSED_VAR(from);
    UNUSED_VAR(payload);
    if (recipients) {
      curl_slist_free_all(recipients);
    }
  };

}

void mailer_pool::stop() {
  VLOG(3) << "stop()";

  curl_pool_.stop();
}
