//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2018
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "td/mtproto/HandshakeActor.h"
#include "td/mtproto/HandshakeConnection.h"

#include "td/utils/common.h"
#include "td/utils/logging.h"
#include "td/utils/Status.h"

namespace td {
namespace mtproto {
HandshakeActor::HandshakeActor(std::unique_ptr<AuthKeyHandshake> handshake,
                               std::unique_ptr<RawConnection> raw_connection,
                               std::unique_ptr<AuthKeyHandshakeContext> context, double timeout,
                               Promise<std::unique_ptr<RawConnection>> raw_connection_promise,
                               Promise<std::unique_ptr<AuthKeyHandshake>> handshake_promise)
    : handshake_(std::move(handshake))
    , connection_(
          std::make_unique<HandshakeConnection>(std::move(raw_connection), handshake_.get(), std::move(context)))
    , timeout_(timeout)
    , raw_connection_promise_(std::move(raw_connection_promise))
    , handshake_promise_(std::move(handshake_promise)) {
}

void HandshakeActor::close() {
  finish(Status::Error("Cancelled"));
  stop();
}

void HandshakeActor::start_up() {
  connection_->get_pollable().set_observer(this);
  subscribe(connection_->get_pollable());
  set_timeout_in(timeout_);
  yield();
}

void HandshakeActor::loop() {
  auto status = connection_->flush();
  if (status.is_error()) {
    finish(std::move(status));
    return stop();
  }
  if (handshake_->is_ready_for_finish()) {
    finish(Status::OK());
    return stop();
  }
}

void HandshakeActor::return_connection(Status status) {
  auto raw_connection = connection_->move_as_raw_connection();
  if (!raw_connection) {
    CHECK(!raw_connection_promise_);
    return;
  }
  unsubscribe(raw_connection->get_pollable());
  raw_connection->get_pollable().set_observer(nullptr);
  if (raw_connection_promise_) {
    if (status.is_error()) {
      if (raw_connection->stats_callback()) {
        raw_connection->stats_callback()->on_error();
      }
      raw_connection->close();
      raw_connection_promise_.set_error(std::move(status));
    } else {
      if (raw_connection->stats_callback()) {
        raw_connection->stats_callback()->on_pong();
      }
      raw_connection_promise_.set_value(std::move(raw_connection));
    }
  } else {
    if (raw_connection->stats_callback()) {
      raw_connection->stats_callback()->on_error();
    }
    raw_connection->close();
  }
}

void HandshakeActor::return_handshake() {
  if (!handshake_promise_) {
    CHECK(!handshake_);
    return;
  }
  handshake_promise_.set_value(std::move(handshake_));
}
}  // namespace mtproto
}  // namespace td
