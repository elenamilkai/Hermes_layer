//
// Created by matt on 5/12/19.
//

#include <fpdb/executor/physical/PhysicalOp.h>
#include <spdlog/spdlog.h>
#include <cassert>               // for assert
#include <utility>               // for move

namespace fpdb::executor::physical {

PhysicalOp::PhysicalOp(std::string name,
                       POpType type,
                       std::vector<std::string> projectColumnNames,
                       int nodeId) :
  name_(std::move(name)),
  type_(type),
  projectColumnNames_(std::move(projectColumnNames)),
  nodeId_(nodeId) {}

POpType PhysicalOp::getType() const {
  return type_;
}

std::string &PhysicalOp::name() {
  return name_;
}

const std::vector<std::string> &PhysicalOp::getProjectColumnNames() const {
  return projectColumnNames_;
}

int PhysicalOp::getNodeId() const {
  return nodeId_;
}

void PhysicalOp::produce(const std::shared_ptr<PhysicalOp> &op) {
  consumers_.emplace(op->name());
}

void PhysicalOp::consume(const std::shared_ptr<PhysicalOp> &op) {
  producers_.emplace(op->name());
}

std::set<std::string>PhysicalOp::consumers() {
  return consumers_;
}

std::set<std::string> PhysicalOp::producers() {
  return producers_;
}

std::shared_ptr<POpContext> PhysicalOp::ctx() {
  return opContext_;
}

void PhysicalOp::create(const std::shared_ptr<POpContext>& ctx) {
  assert (ctx);
  SPDLOG_DEBUG("Creating operator  |  name: '{}'", this->name_);
  opContext_ = ctx;
}

void PhysicalOp::setName(const std::string &Name) {
  name_ = Name;
}

void PhysicalOp::setProjectColumnNames(const std::vector<std::string> &projectColumnNames) {
  projectColumnNames_ = projectColumnNames;
}

void PhysicalOp::destroyActor() {
  opContext_->destroyActorHandles();
}

long PhysicalOp::getQueryId() const {
  return queryId_;
}

void PhysicalOp::setQueryId(long queryId) {
  queryId_ = queryId;
}

} // namespace

