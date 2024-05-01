//
// Created by Yifei Yang on 1/12/22.
//

#ifndef FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_SERIALIZATION_ARROWSERIALIZER_H
#define FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_SERIALIZATION_ARROWSERIALIZER_H

#include <caf/all.hpp>
#include <arrow/api.h>

namespace fpdb::tuple {

class ArrowSerializer {

public:
  /**
   * Serialization and deserialization methods of arrow::Table.
   */
  static std::shared_ptr<arrow::Table> bytes_to_table(const std::vector<std::uint8_t>& bytes_vec);
  static std::vector<std::uint8_t> table_to_bytes(const std::shared_ptr<arrow::Table>& table);
  static std::vector<std::int8_t> table_to_bytes2(const std::shared_ptr<arrow::Table>& table);

  /**
   * Serialization and deserialization methods of arrow::RecordBatch.
   */
  static std::shared_ptr<arrow::RecordBatch> bytes_to_recordBatch(const std::vector<std::uint8_t>& bytes_vec);
  static std::vector<std::uint8_t> recordBatch_to_bytes(const std::shared_ptr<arrow::RecordBatch>& recordBatch);

  /**
   * Serialization and deserialization methods of arrow::Schema.
   */
  static std::shared_ptr<arrow::Schema> bytes_to_schema(const std::vector<std::uint8_t>& bytes_vec);
  static std::vector<std::uint8_t> schema_to_bytes(const std::shared_ptr<arrow::Schema>& schema);

  /**
   * Serialization and deserialization methods of arrow::ChunkedArray.
   * As the primitive serialization unit of Arrow is RecordBatch, here we do indirectly by making a Table first
   */
  static std::shared_ptr<arrow::ChunkedArray> bytes_to_chunkedArray(const std::vector<std::uint8_t>& bytes_vec);
  static std::vector<std::uint8_t> chunkedArray_to_bytes(const std::shared_ptr<arrow::ChunkedArray>& chunkedArray);

  /**
   * Serialization and deserialization methods of arrow::Scalar
   * Do this by converting the scalar to a record batch
   */
  static std::shared_ptr<arrow::Scalar> bytes_to_scalar(const std::vector<std::uint8_t>& bytes_vec);
  static std::vector<std::uint8_t> scalar_to_bytes(const std::shared_ptr<arrow::Scalar>& scalar);

  /**
   * Serialization and deserialization methods of arrow::DataType
   * Do this in a straightforward way (using name()) instead of using schema serialization
   */
  static std::shared_ptr<arrow::DataType> bytes_to_dataType(const std::vector<std::uint8_t>& bytes_vec);
  static std::vector<std::uint8_t> dataType_to_bytes(const std::shared_ptr<arrow::DataType>& dataType);
};

}


#endif //FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_SERIALIZATION_ARROWSERIALIZER_H
