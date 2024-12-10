//
// Created by Elena Milkai on 9/25/24.
//
/**auto s3_global_options = arrow::fs::S3GlobalOptions{};
auto init_status = arrow::fs::InitializeS3(s3_global_options);
if (!init_status.ok()) {
  std::cerr << "S3 Init Error: " << init_status.ToString() << std::endl;
} else {
  std::cout << "S3 Initialized successfully!" << std::endl;
}
arrow::fs::S3Options s3_options = arrow::fs::S3Options::Defaults();
s3_options.region = "us-west-2";
auto s3fs_result = arrow::fs::S3FileSystem::Make(s3_options);
if (!s3fs_result.ok()) {
  std::cerr << "Failed to create S3FileSystem: "
            << s3fs_result.status().ToString() << std::endl;
} else {
  std::cout << "S3FileSystem created successfully!" << std::endl;
}
std::shared_ptr<arrow::fs::S3FileSystem> s3fs = s3fs_result.ValueOrDie();
std::shared_ptr<arrow::io::RandomAccessFile> input;
arrow::Result<std::shared_ptr<arrow::io::RandomAccessFile>> input_result = s3fs->OpenInputFile(s3Object_);
if (!input_result.ok()) {
  std::cerr << "Failed to open input file: " << input_result.status().ToString() << std::endl;
  return;
}
input = input_result.ValueOrDie();
std::unique_ptr<parquet::arrow::FileReader> parquet_reader;
parquet_reader->set_use_threads(false);
std::vector<int> neededColumnIndices;
for (const auto &fieldName : getProjectColumnNames()) {
  neededColumnIndices.emplace_back(table_->getSchema()->GetFieldIndex(fieldName));
}
parquet::arrow::OpenFile(input, arrow::default_memory_pool(), &parquet_reader);
int num_row_groups = parquet_reader->num_row_groups();
SPDLOG_CRITICAL("---------------------> Number of row groups {} and number of columns {}", num_row_groups, neededColumnIndices.size());

for (int i = 0; i < num_row_groups; ++i) {
  std::shared_ptr<arrow::Table> table;
  arrow::Status status = parquet_reader->RowGroup(i)->ReadTable(neededColumnIndices, &table);
  if (!status.ok()) {
    std::cerr << "Error reading row group " << i << ": " << status.ToString() << std::endl;
    continue;
  }
  std::cout << "Processed row group " << i << ", table has " << table->num_rows()
            << " rows and " << table->num_columns() << " columns." << std::endl;
}**/