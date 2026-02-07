#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <vsomeip/vsomeip.hpp>

using namespace vsomeip_v3;

#define SAMPLE_SERVICE_ID 0x1001
#define SAMPLE_INSTANCE_ID 0x1001
#define SAMPLE_METHOD_ID 0x1001

#define SAMPLE_EVENTGROUP_ID 0x8001
#define SAMPLE_EVENT_ID 0x8001

std::shared_ptr<vsomeip::application> app;

bool on_subscribe(client_t _client, std::uint32_t _uid, std::uint32_t _gid,
                  bool subscribe) {
  printf("%ssubscribe %x:%x:%x\n", subscribe ? "" : "stop ", _client, _uid,
         _gid);
  return true;
}

void notify_thread(void) {
  uint8_t cnt = 0;
  for (;;) {
    vsomeip::byte_t its_data[1396];
    for (int i = 0; i < 1396; i++) {
      its_data[i] = (vsomeip::byte_t)(cnt + i);
    }
    cnt++;
    auto payload = vsomeip::runtime::get()->create_payload();
    payload->set_data(its_data, sizeof(its_data));
    app->notify(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID, SAMPLE_EVENT_ID,
                payload);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

void on_message(const std::shared_ptr<vsomeip::message> &_request) {
  static uint8_t cnt = 0;
  cnt++;
  std::shared_ptr<vsomeip::payload> its_payload = _request->get_payload();
  vsomeip::length_t l = its_payload->get_length();
  auto data = its_payload->get_data();

  // Get payload
  std::stringstream ss;
  for (vsomeip::length_t i = 0; i < l; i++) {
    ss << std::setw(2) << std::setfill('0') << std::hex << (int)data[i] << " ";
    if (i > 16)
      break;
  }

  std::cout << "SERVICE: Received message with Client/Session [" << std::setw(4)
            << std::setfill('0') << std::hex << _request->get_client() << "/"
            << std::setw(4) << std::setfill('0') << std::hex
            << _request->get_session() << "] " << ss.str() << std::endl;

  // Create response
  std::shared_ptr<vsomeip::message> its_response =
      vsomeip::runtime::get()->create_response(_request);
  its_payload = vsomeip::runtime::get()->create_payload();
  std::vector<vsomeip::byte_t> its_payload_data;
  if (l > 1396) {
    l = 1396;
  }
  for (int i = l - 1; i >= 0; i--) {
    its_payload_data.push_back(data[i]);
  }
  its_payload->set_data(its_payload_data);
  its_response->set_payload(its_payload);
  app->send(its_response);
}

int main() {

  app = vsomeip::runtime::get()->create_application("hello_world_service");
  app->init();
  app->register_message_handler(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID,
                                SAMPLE_METHOD_ID, on_message);
  app->offer_service(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID);
  std::set<vsomeip::eventgroup_t> its_groups;
  its_groups.insert(SAMPLE_EVENTGROUP_ID);
  app->offer_event(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID, SAMPLE_EVENT_ID,
                   its_groups);
  app->register_subscription_handler(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID,
                                     SAMPLE_EVENTGROUP_ID, on_subscribe);
  std::thread th(notify_thread);
  app->start();
}