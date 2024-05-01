//
// Created by matt on 21/7/20.
//

#include <fpdb/executor/message/ScanMessage.h>
#include <utility>

using namespace fpdb::executor::message;

ScanMessage::ScanMessage(std::vector<std::string> ColumnNames, const std::string &Sender, bool resultNeeded) :
	Message(SCAN, Sender),
	columnNames_(std::move(ColumnNames)),
	resultNeeded_(resultNeeded) {}

std::string ScanMessage::getTypeString() const {
  return "ScanMessage";
}

const std::vector<std::string> &ScanMessage::getColumnNames() const {
  return columnNames_;
}

bool ScanMessage::isResultNeeded() const {
  return resultNeeded_;
}
