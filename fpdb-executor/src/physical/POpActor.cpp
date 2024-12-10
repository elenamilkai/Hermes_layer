//
// Created by Matt Youill on 31/12/19.
//

#include <fpdb/executor/physical/POpActor.h>
#include <fpdb/executor/message/Envelope.h>
#include <fpdb/executor/message/ConnectMessage.h>
#include <fpdb/executor/message/CompleteMessage.h>
#include <spdlog/spdlog.h>
#include <utility>

namespace fpdb::executor::physical {

POpActor::POpActor(::caf::actor_config &cfg, std::shared_ptr<PhysicalOp> opBehaviour) :
	::caf::event_based_actor(cfg),
	opBehaviour_(std::move(opBehaviour)) {

  name_ = opBehaviour_->name();
}

::caf::behavior behaviour(POpActor *self) {

  auto ctx = self->operator_()->ctx();
  ctx->operatorActor(self);

  return {
	  [=](GetProcessingTimeAtom) {
		auto start = std::chrono::steady_clock::now();
		auto finish = std::chrono::steady_clock::now();
		auto elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count();
		self->incrementProcessingTime(elapsedTime);
		return self->getProcessingTime();
	  },
	  [=](const fpdb::executor::message::Envelope &msg) {

		auto start = std::chrono::steady_clock::now();

    SPDLOG_DEBUG("Message received  |  recipient: '{}', sender: '{}', type: '{}'",
                 self->operator_()->name(),
                 msg.message().sender(),
                 msg.message().type());

		if (msg.message().type() == MessageType::CONNECT) {
		  auto connectMessage = dynamic_cast<const message::ConnectMessage &>(msg.message());

		  for (const auto &element: connectMessage.connections()) {
        auto localEntry = LocalPOpDirectoryEntry(element.getName(),
                                element.getActorHandle(),
                                element.getConnectionType(),
                                false);

        auto result = self->operator_()->ctx()->operatorMap().insert(localEntry);
        if (!result.has_value()) {
          self->operator_()->ctx()->notifyError(result.error());
        }
		  }
		}
		else if (msg.message().type() == MessageType::START) {
		  auto startMessage = dynamic_cast<const message::StartMessage &>(msg.message());

		  self->running_ = true;

		  self->operator_()->onReceive(msg);

		  while(!self->buffer_.empty()){
			auto item = self->buffer_.front();
			self->overriddenMessageSender_ = item.second;
			self->call_handler(self->current_behavior(), item.first);
			self->buffer_.pop();
		  }

		  self->overriddenMessageSender_ = std::nullopt;

		} else if (msg.message().type() == MessageType::STOP) {
		  self->running_ = false;

		  self->operator_()->onReceive(msg);
		}
		else{
		  if(!self->running_){
			self->buffer_.emplace(self->current_mailbox_element()->content(), self->current_sender());
		  }
		  else {
			if (msg.message().type() == MessageType::COMPLETE) {
			  auto completeMessage = dynamic_cast<const message::CompleteMessage &>(msg.message());
			  auto result = self->operator_()->ctx()->operatorMap().setComplete(msg.message().sender());
        if (!result.has_value()) {
          return self->operator_()->ctx()->notifyError(result.error());
        }
			}

			self->operator_()->onReceive(msg);
		  }
		}

		auto finish = std::chrono::steady_clock::now();
		auto elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start).count();
		self->incrementProcessingTime(elapsedTime);
	  }
  };
}

::caf::behavior POpActor::make_behavior() {
  return behaviour(this);
}

std::shared_ptr<fpdb::executor::physical::PhysicalOp> POpActor::operator_() const {
  return opBehaviour_;
}

long POpActor::getProcessingTime() const {
  return processingTime_;
}

void POpActor::incrementProcessingTime(long time) {
  processingTime_ += time;
}

void POpActor::on_exit() {
  SPDLOG_DEBUG("Stopping operator  |  name: '{}'", this->opBehaviour_->name());

  /*
   * Need to delete the actor handle in operator otherwise CAF will never release the actor
   */
  this->opBehaviour_->destroyActor();
}

}
