//
// Created by Yifei Yang on 1/18/22.
//

#include <fpdb/tuple/Util.h>
#include <fpdb/tuple/ArrayAppenderWrapper.h>

namespace fpdb::tuple {

tl::expected<std::shared_ptr<arrow::Array>, std::string>
Util::makeEmptyArray(const std::shared_ptr<arrow::DataType> &type) {
  auto expAppender = ArrayAppenderBuilder::make(type);
  if (!expAppender.has_value()) {
    return tl::make_unexpected(expAppender.error());
  }
  const auto &appender = *expAppender;

  auto expArray = appender->finalize();
  if (!expArray.has_value()) {
    return tl::make_unexpected(expArray.error());
  }
  return *expArray;
}

}
