//
// Created by Yifei Yang on 1/16/22.
//

#ifndef FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_SERIALIZATION_FILEREADERSERIALIZER_H
#define FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_SERIALIZATION_FILEREADERSERIALIZER_H

#include <fpdb/tuple/csv/CSVReader.h>
#include <fpdb/tuple/parquet/ParquetReader.h>
#include <fpdb/caf/CAFUtil.h>

using FileReaderPtr = std::shared_ptr<fpdb::tuple::FileReader>;

CAF_BEGIN_TYPE_ID_BLOCK(FileReader, fpdb::caf::CAFUtil::FileReader_first_custom_type_id)
CAF_ADD_TYPE_ID(FileReader, (FileReaderPtr))
CAF_ADD_TYPE_ID(FileReader, (fpdb::tuple::CSVReader))
CAF_ADD_TYPE_ID(FileReader, (fpdb::tuple::ParquetReader))
CAF_END_TYPE_ID_BLOCK(FileReader)

namespace caf {

template<>
struct variant_inspector_traits<FileReaderPtr> {
  using value_type = FileReaderPtr;

  // Lists all allowed types and gives them a 0-based index.
  static constexpr type_id_t allowed_types[] = {
          type_id_v<none_t>,
          type_id_v<fpdb::tuple::CSVReader>,
          type_id_v<fpdb::tuple::ParquetReader>
  };

  // Returns which type in allowed_types corresponds to x.
  static auto type_index(const value_type &x) {
    if (!x)
      return 0;
    else if (x->getType() == fpdb::tuple::FileType::CSV)
      return 1;
    else if (x->getType() == fpdb::tuple::FileType::Parquet)
      return 2;
    else
      return -1;
  }

  // Applies f to the value of x.
  template<class F>
  static auto visit(F &&f, const value_type &x) {
    switch (type_index(x)) {
      case 1:
        return f(dynamic_cast<fpdb::tuple::CSVReader &>(*x));
      case 2:
        return f(dynamic_cast<fpdb::tuple::ParquetReader &>(*x));
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
      case type_id_v<fpdb::tuple::CSVReader>: {
        auto tmp = fpdb::tuple::CSVReader{};
        continuation(tmp);
        return true;
      }
      case type_id_v<fpdb::tuple::ParquetReader>: {
        auto tmp = fpdb::tuple::ParquetReader{};
        continuation(tmp);
        return true;
      }
    }
  }
};

template <>
struct inspector_access<FileReaderPtr> : variant_inspector_access<FileReaderPtr> {
  // nop
};

} // namespace caf

#endif //FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_SERIALIZATION_FILEREADERSERIALIZER_H
