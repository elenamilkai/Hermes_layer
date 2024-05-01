//
// Created by matt on 21/7/20.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_MESSAGE_SCANMESSAGE_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_MESSAGE_SCANMESSAGE_H

#include <fpdb/executor/message/Message.h>
#include <string>
#include <vector>

namespace fpdb::executor::message {

/**
 * A message to a scan operator telling it to scan a subset of columns from its data source
 */
class ScanMessage : public Message {

public:
  ScanMessage(std::vector<std::string> columnNames_, const std::string &sender, bool resultNeeded);
  ScanMessage() = default;
  ScanMessage(const ScanMessage&) = default;
  ScanMessage& operator=(const ScanMessage&) = default;

  std::string getTypeString() const override;
  
  [[nodiscard]] const std::vector<std::string> &getColumnNames() const;
  [[nodiscard]] bool isResultNeeded() const;

private:
  std::vector<std::string> columnNames_;
  bool resultNeeded_;

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, ScanMessage& msg) {
    return f.object(msg).fields(f.field("type", msg.type_),
                                f.field("sender", msg.sender_),
                                f.field("columnNames", msg.columnNames_),
                                f.field("resultNeeded", msg.resultNeeded_));
  };
};

}

#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_MESSAGE_SCANMESSAGE_H
