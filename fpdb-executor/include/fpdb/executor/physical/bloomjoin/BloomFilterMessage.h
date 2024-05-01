//
// Created by matt on 6/8/20.
//

#ifndef FPDB_FPDB_PUSHDOWN_INCLUDE_FPDB_PUSHDOWN_BLOOMJOIN_BLOOMFILTERMESSAGE_H
#define FPDB_FPDB_PUSHDOWN_INCLUDE_FPDB_PUSHDOWN_BLOOMJOIN_BLOOMFILTERMESSAGE_H

#include <memory>
#include <utility>

#include <fpdb/core/message/Message.h>

#include "SlicedBloomFilter.h"

using namespace fpdb::core::message;

class BloomFilterMessage : public Message {

public:
  BloomFilterMessage(std::shared_ptr<SlicedBloomFilter> bloomFilter, const std::string &sender) :
	  Message("BloomFilterMessage", sender), bloomFilter_(std::move(bloomFilter)) {
  }

  [[nodiscard]] const std::shared_ptr<SlicedBloomFilter> &getBloomFilter() const {
	return bloomFilter_;
  }

private:
  std::shared_ptr<SlicedBloomFilter> bloomFilter_;

};

#endif //FPDB_FPDB_PUSHDOWN_INCLUDE_FPDB_PUSHDOWN_BLOOMJOIN_BLOOMFILTERMESSAGE_H
