//
// Created by Yifei Yang on 1/12/22.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_SERIALIZATION_MESSAGESERIALIZER_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_SERIALIZATION_MESSAGESERIALIZER_H

#include <fpdb/executor/message/Message.h>
#include <fpdb/executor/message/StartMessage.h>
#include <fpdb/executor/message/ConnectMessage.h>
#include <fpdb/executor/message/CompleteMessage.h>
#include <fpdb/executor/message/ErrorMessage.h>
#include <fpdb/executor/message/ScanMessage.h>
#include <fpdb/executor/message/TupleMessage.h>
#include <fpdb/executor/message/TupleSetIndexMessage.h>
#include <fpdb/executor/message/cache/LoadRequestMessage.h>
#include <fpdb/executor/message/cache/LoadResponseMessage.h>
#include <fpdb/executor/message/cache/StoreRequestMessage.h>
#include <fpdb/executor/message/cache/WeightRequestMessage.h>
#include <fpdb/executor/message/cache/CacheMetricsMessage.h>
#include <fpdb/executor/message/delta/LoadDeltaRequestMessage.h>
#include <fpdb/executor/message/delta/LoadDeltaResponseMessage.h>
#include <fpdb/executor/message/delta/StoreDeltaRequestMessage.h>
#include <fpdb/executor/message/delta/MergedDeltaMessage.h>
#include <fpdb/caf/CAFUtil.h>
#include <fpdb/tuple/serialization/TupleKeyElementSerializer.h>

using namespace fpdb::executor::message;

using MessagePtr = std::shared_ptr<Message>;

CAF_BEGIN_TYPE_ID_BLOCK(Message, fpdb::caf::CAFUtil::Message_first_custom_type_id)
CAF_ADD_TYPE_ID(Message, (MessagePtr))
CAF_ADD_TYPE_ID(Message, (StartMessage))
CAF_ADD_TYPE_ID(Message, (ConnectMessage))
CAF_ADD_TYPE_ID(Message, (CompleteMessage))
CAF_ADD_TYPE_ID(Message, (ErrorMessage))
CAF_ADD_TYPE_ID(Message, (ScanMessage))
CAF_ADD_TYPE_ID(Message, (TupleMessage))
CAF_ADD_TYPE_ID(Message, (TupleSetIndexMessage))
// For the following cache messages, we have to implement `inspect` for concrete derived shared_ptr type one by one,
// because SegmentCacheActor directly uses the concrete derived types rather than base type Message used by other actors
CAF_ADD_TYPE_ID(Message, (LoadRequestMessage))
CAF_ADD_TYPE_ID(Message, (LoadResponseMessage))
CAF_ADD_TYPE_ID(Message, (StoreRequestMessage))
CAF_ADD_TYPE_ID(Message, (WeightRequestMessage))
CAF_ADD_TYPE_ID(Message, (CacheMetricsMessage))
CAF_ADD_TYPE_ID(Message, (std::shared_ptr<LoadResponseMessage>))
CAF_ADD_TYPE_ID(Message, (std::shared_ptr<LoadRequestMessage>))
CAF_ADD_TYPE_ID(Message, (std::shared_ptr<StoreRequestMessage>))
CAF_ADD_TYPE_ID(Message, (std::shared_ptr<WeightRequestMessage>))
CAF_ADD_TYPE_ID(Message, (std::shared_ptr<CacheMetricsMessage>))
// Delta cache message
CAF_ADD_TYPE_ID(Message, (LoadDeltaRequestMessage))
CAF_ADD_TYPE_ID(Message, (LoadDeltaResponseMessage))
CAF_ADD_TYPE_ID(Message, (StoreDeltaRequestMessage))
CAF_ADD_TYPE_ID(Message, (MergedDeltaMessage))
CAF_ADD_TYPE_ID(Message, (std::shared_ptr<LoadDeltaRequestMessage>))
CAF_ADD_TYPE_ID(Message, (std::shared_ptr<LoadDeltaResponseMessage>))
CAF_ADD_TYPE_ID(Message, (std::shared_ptr<StoreDeltaRequestMessage>))
CAF_ADD_TYPE_ID(Message, (std::shared_ptr<MergedDeltaMessage>))
CAF_END_TYPE_ID_BLOCK(Message)

// Variant-based approach on MessagePtr
namespace caf {

template<>
struct variant_inspector_traits<MessagePtr> {
  using value_type = MessagePtr;

  // Lists all allowed types and gives them a 0-based index.
  static constexpr type_id_t allowed_types[] = {
          type_id_v<none_t>,
          type_id_v<StartMessage>,
          type_id_v<ConnectMessage>,
          type_id_v<CompleteMessage>,
          type_id_v<ErrorMessage>,
          type_id_v<ScanMessage>,
          type_id_v<TupleMessage>,
          type_id_v<TupleSetIndexMessage>
  };

  // Returns which type in allowed_types corresponds to x.
  static auto type_index(const value_type &x) {
    if (!x)
      return 0;
    else if (x->type() == MessageType::START)
      return 1;
    else if (x->type() == MessageType::CONNECT)
      return 2;
    else if (x->type() == MessageType::COMPLETE)
      return 3;
    else if (x->type() == MessageType::ERROR)
      return 4;
    else if (x->type() == MessageType::SCAN)
      return 5;
    else if (x->type() == MessageType::TUPLE)
      return 6;
    else if (x->type() == MessageType::TUPLESET_INDEX)
      return 7;
    else
      return -1;
  }

  // Applies f to the value of x.
  template<class F>
  static auto visit(F &&f, const value_type &x) {
    switch (type_index(x)) {
      case 1:
        return f(dynamic_cast<StartMessage &>(*x));
      case 2:
        return f(dynamic_cast<ConnectMessage &>(*x));
      case 3:
        return f(dynamic_cast<CompleteMessage &>(*x));
      case 4:
        return f(dynamic_cast<ErrorMessage &>(*x));
      case 5:
        return f(dynamic_cast<ScanMessage &>(*x));
      case 6:
        return f(dynamic_cast<TupleMessage &>(*x));
      case 7:
        return f(dynamic_cast<TupleSetIndexMessage &>(*x));
      default: {
        none_t dummy;
        return f(dummy);
      }
    }
  }

  // Assigns a value to x.
  template<class U>
  static void assign(value_type &x, U value) {
    if constexpr (std::is_same<U, none_t>::value)
      x.reset();
    else
      x = std::make_shared<U>(std::move(value));
  }

  // Create a default-constructed object for `type` and then call the
  // continuation with the temporary object to perform remaining load steps.
  template<class F>
  static bool load(type_id_t type, F continuation) {
    switch (type) {
      default:
        return false;
      case type_id_v<none_t>: {
        none_t dummy;
        continuation(dummy);
        return true;
      }
      case type_id_v<StartMessage>: {
        auto tmp = StartMessage{};
        continuation(tmp);
        return true;
      }
      case type_id_v<ConnectMessage>: {
        auto tmp = ConnectMessage{};
        continuation(tmp);
        return true;
      }
      case type_id_v<CompleteMessage>: {
        auto tmp = CompleteMessage{};
        continuation(tmp);
        return true;
      }
      case type_id_v<ErrorMessage>: {
        auto tmp = ErrorMessage{};
        continuation(tmp);
        return true;
      }
      case type_id_v<ScanMessage>: {
        auto tmp = ScanMessage{};
        continuation(tmp);
        return true;
      }
      case type_id_v<TupleMessage>: {
        auto tmp = TupleMessage{};
        continuation(tmp);
        return true;
      }
      case type_id_v<TupleSetIndexMessage>: {
        auto tmp = TupleSetIndexMessage{};
        continuation(tmp);
        return true;
      }
    }
  }
};

template <>
struct inspector_access<MessagePtr> : variant_inspector_access<MessagePtr> {
  // nop
};

} // namespace caf

#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_SERIALIZATION_MESSAGESERIALIZER_H
