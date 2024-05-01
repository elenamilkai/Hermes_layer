//
// Created by matt on 29/4/20.
//

#include <fpdb/executor/physical/join/hashjoin/HashJoinPredicate.h>
#include <utility>
#include <sstream>
#include <cassert>

using namespace fpdb::executor::physical::join;

HashJoinPredicate::HashJoinPredicate(vector<string> leftColumnNames,
                                     vector<string> rightColumnNames) :
	leftColumnNames_(move(leftColumnNames)),
	rightColumnNames_(move(rightColumnNames)) {
  assert(leftColumnNames_.size() == rightColumnNames_.size());
}

const vector<string> &HashJoinPredicate::getLeftColumnNames() const {
  return leftColumnNames_;
}

const vector<string> &HashJoinPredicate::getRightColumnNames() const {
  return rightColumnNames_;
}

string HashJoinPredicate::toString() const {
  stringstream ss;
  for (uint i = 0; i < leftColumnNames_.size(); ++i) {
    ss << leftColumnNames_[i];
    ss << "-";
    ss << rightColumnNames_[i];
    if (i < leftColumnNames_.size() - 1) {
      ss << "-";
    }
  }
  return ss.str();
}
