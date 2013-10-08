#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <ev++.h>
#include <map>
#include <unordered_map>
#include <pubsub.h>

struct WSConnection {
    int sfd;
    size_t bytes_read;
    size_t bytes_written;
    std::string send_buffer;
    std::string header;
    std::string frames;
    ev::io io;
    PubSub& pubsub;

    enum State {
      READ_HTTP_HEADER = 0x1,
      WRITE_HTTP_HEADER = 0x2,
      READ_FRAME_HEADER = 0x4,
      READ_FRAME_PAYLOAD = 0x8,
      WRITE_FRAME = 0x10
    };

    uint32_t state;

    struct header_values {
      std::string method;
      std::string uri;
      std::string version;
      std::unordered_map<std::string, const std::string> headers;
    } header_vals;

    WSConnection(int socket_fd, PubSub& pubsub);
    virtual ~WSConnection();
    void set_io();
    bool read_cb();
    bool write_cb();
    bool try_read_header();
    bool try_read_frame();
    void try_send_frame(const std::string& payload);
    bool parse_header();
    bool write_response_header();
};

class Websocket {
  private:
    ev::io io;
    ev::timer tio;
    ev::sig sio;
    int s;
    std::map<const int, WSConnection*> conn_map;
    PubSub& pubsub;

  public:
    Websocket(int port, PubSub& pubsub);
    void io_accept(ev::io &watcher, int revents);
    void remove_connection(WSConnection* conn);
    void callback(ev::io &watcher, int revents);
    static void signal_cb(ev::sig &signal, int revents);
    void timer_cb(ev::timer &timer, int revents);
    virtual ~Websocket();
};

#endif
