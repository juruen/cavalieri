#include <glog/logging.h>
#include <algorithm>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <instrumentation/mem.h>
#include <instrumentation/instrumentation.h>



namespace {

const int k_max_rates = 1000;
const int k_max_latencies = 1000;
const int k_max_gauges= 1000;
const std::string k_instrumentation_tag = "cavalieri::internal";
const std::string k_vm_mem_service = "vm memory usage";
const std::string k_vm_mem_description = "vm memory in MB";
const std::string k_rss_mem_service = "rss memory usage";
const std::string k_rss_mem_description = "rss memory in MB";
const float k_default_ttl = 60;

void set_vm_meausre(Event & event, const double vm) {

  event.set_service(k_vm_mem_service);
  event.set_description(k_vm_mem_description);
  event.set_metric_d(vm / 1024);


}

void set_rss_measure(Event & event, const double rss) {

  event.set_service(k_rss_mem_service);
  event.set_description(k_rss_mem_description);
  event.set_metric_d(rss / 1024);

}

void set_mem_meausres(std::vector<Event> & events, Event & event) {

  double vm, rss;
  process_mem_usage(vm, rss);

  set_vm_meausre(event, vm);
  events.push_back(event);
  set_rss_measure(event, rss);
  events.push_back(event);

}

const std::string fqdn() {
  char hostname[1024];
  hostname[1023] = '\0';
  gethostname(hostname, 1023);

  return gethostbyname(hostname)->h_name;
}

const std::string k_fqdn(fqdn());

};

instrumentation::instrumentation(const config)
  :
    rates_id_(0),
    latencies_id_(0),
    gauges_id_(0),
    rates_(k_max_rates),
    latencies_(k_max_latencies),
    gauges_(k_max_gauges)

{
}

id_t instrumentation::add_rate(const std::string service,
                              const std::string description)
{

  CHECK(rates_id_.load() < k_max_rates) << "max number of rates reached";

  int id = rates_id_.fetch_add(1);

  rates_[id].service = service;
  rates_[id].description = description;
  rates_[id].rate.reset();

  return id;
}

void instrumentation::update_rate(const id_t id,
                                  const unsigned int ticks)
{
  rates_[id].rate.add(ticks);
}

id_t instrumentation::add_latency(const std::string service,
                                 const std::string description,
                                 std::vector<double> percentiles)
{

  CHECK(rates_id_.load() < k_max_latencies)
        << "max number of percentiles reached";

  int id = latencies_id_.fetch_add(1);

  latencies_[id].service = service;
  latencies_[id].description = description;
  latencies_[id].percentiles = percentiles;

  return id;
}

void instrumentation::update_latency(const id_t id,
                                     const double value)
{
  latencies_[id].reservoir.add_sample(value);
}

unsigned int instrumentation::add_gauge(const std::string service,
                                        const std::string description)
{

  CHECK(gauges_id_.load() < k_max_gauges) << "max number of gauges reached";

  int id = gauges_id_.fetch_add(1);

  gauges_[id].service = service;
  gauges_[id].description = description;

  return id;
}

void instrumentation::update_gauge(id_t id,
                                   const unsigned int ticks)
{
  gauges_[id].gauge.update(ticks);
}

void instrumentation::incr_gauge(id_t id,
                                 const unsigned int ticks)
{
  gauges_[id].gauge.incr(ticks);
}

void instrumentation::decr_gauge(id_t id,
                                 const unsigned int ticks)
{
  gauges_[id].gauge.decr(ticks);
}

std::vector<Event> instrumentation::snapshot() {

  VLOG(3) << "snapshot()";

  std::vector<Event> events;

  Event event;

  event.add_tag(k_instrumentation_tag);

  event.set_host(k_fqdn);
  event.set_state("ok");
  event.set_ttl(k_default_ttl);

  set_rates(events, event);
  set_latencies(events, event);
  set_gauges(events, event);
  set_mem_meausres(events, event);

  return events;
}

void instrumentation::set_rates(std::vector<Event> & events, Event event) {

  for (int i = 0; i < rates_id_.load(); i++) {

    event.set_service(rates_[i].service);
    event.set_description(rates_[i].description);
    event.set_metric_d(rates_[i].rate.snapshot());

    events.push_back(event);
  }

}

void instrumentation::set_latencies(std::vector<Event> & events, Event event) {

  for (int i = 0; i < latencies_id_.load(); i++) {

    event.set_service(latencies_[i].service);
    event.set_description(latencies_[i].description);

    auto new_events = reservoir_to_events(latencies_[i].reservoir,
                                          latencies_[i].percentiles,
                                          event);

    std::copy(begin(new_events), end(new_events), back_inserter(events));

  }

}

void instrumentation::set_gauges(std::vector<Event> & events, Event event)
{

  for (int i = 0; i < gauges_id_.load(); i++) {

    event.set_service(gauges_[i].service);
    event.set_description(gauges_[i].description);
    event.set_metric_d(gauges_[i].gauge.snapshot());

    events.push_back(event);
  }

}


std::vector<Event> instrumentation::reservoir_to_events(
    reservoir & reservoir,
    std::vector<double> percentiles,
    const Event event_base)
{

  auto samples = reservoir.snapshot();

  if (samples.empty()) {
    return {};
  }

  std::stable_sort(begin(samples), end(samples));

  std::vector<Event> events;

  auto n = samples.size();
  for (const auto & p : percentiles) {

    auto event(event_base);
    auto index = p == 1 ? n - 1 : static_cast<int>(n * p);

    event.set_service(event.service() + " " + std::to_string(p));
    event.set_metric_d(samples[index]);

    events.push_back(event);

  }

  return events;
}

