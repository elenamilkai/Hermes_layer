//
// Created by Yifei Yang on 1/14/22.
//

#include <fpdb/main/Server.h>
#include <fpdb/main/ExecConfig.h>
#include <fpdb/main/CAFInit.h>
#include <fpdb/aws/AWSClient.h>
#include <fpdb/util/Util.h>
#include <iostream>
//#include "fpdb/hermesserver/Server.h"

using namespace fpdb::util;

namespace fpdb::main {

void Server::start() {
  // start the daemon AWS client
  const auto &awsConfig = AWSConfig::parseAWSConfig();
  fpdb::aws::AWSClient::daemonClient_ = make_shared<AWSClient>(awsConfig);
  fpdb::aws::AWSClient::daemonClient_->init();
  std::cout << "Daemon AWS client started" << std::endl;

  // read remote Ips and server port
  const auto &remoteIps = readRemoteIps();
  int CAFServerPort = ExecConfig::parseCAFServerPort();

  // create the actor system
  actorSystemConfig_ = std::make_shared<ActorSystemConfig>(CAFServerPort, remoteIps, true);
  CAFInit::initCAFGlobalMetaObjects();
  actorSystem_ = std::make_shared<::caf::actor_system>(*actorSystemConfig_);

  // open the port
  auto res = actorSystem_->middleman().open(actorSystemConfig_->port_);
  if (!res) {
    throw std::runtime_error("Cannot open CAF server at port: " + to_string(res.error()));
  } else {
    std::cout << "CAF server opened at port: " << actorSystemConfig_->port_ << std::endl;
  }

  std::cout << "Server started" << std::endl;
}

void Server::stop() {
  // stop the daemon AWS client
  fpdb::aws::AWSClient::daemonClient_->shutdown();
  std::cout << "Daemon AWS client stopped" << std::endl;

  if (actorSystem_) {
    auto res = actorSystem_->middleman().close(actorSystemConfig_->port_);
    if (!res) {
      throw std::runtime_error("Cannot close CAF server at port: " + to_string(res.error()));
    }
  }
  std::cout << "CAF server closed at port: " << actorSystemConfig_->port_ << std::endl;

  std::cout << "Server stopped" << std::endl;
}

}

