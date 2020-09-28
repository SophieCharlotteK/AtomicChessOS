#pragma once

#ifndef SESSION_H_5KG6ZMAB
#define SESSION_H_5KG6ZMAB

#include "asio.hpp"
#include <memory>
#include <vector>

#include "../config.h"
#include "../msgpack.hpp"

#include "../dispatcher.h"
#include "async_writer.h"
#include "log.h"

namespace rpc {

class server;

namespace detail {

class server_session : public async_writer {
public:
    server_session(server *srv, RPCLIB_ASIO::io_service *io,
                   RPCLIB_ASIO::ip::tcp::socket socket,
                   std::shared_ptr<dispatcher> disp, bool suppress_exceptions);
    void start();

    void close();

private:
    void do_read();

private:
    server* parent_;
    RPCLIB_ASIO::io_service *io_;
    RPCLIB_ASIO::strand read_strand_;
    std::shared_ptr<dispatcher> disp_;
    RPCLIB_MSGPACK::unpacker pac_;
    RPCLIB_MSGPACK::sbuffer output_buf_;
    const bool suppress_exceptions_;
    RPCLIB_CREATE_LOG_CHANNEL(session)
};
} /* detail */
} /* rpc */

#endif /* end of include guard: SESSION_H_5KG6ZMAB */
