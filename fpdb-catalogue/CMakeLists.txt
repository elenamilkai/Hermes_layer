project(fpdb-catalogue VERSION "${PROJECT_VERSION}" LANGUAGES C CXX)
#-------------------------------------------------------------------------------------------------------------
# Target
#-----------------------------------------------------------------------------------------------------------------------

add_library(fpdb-catalogue STATIC
        include/fpdb/catalogue/Globals.h
        include/fpdb/catalogue/CatalogueEntryType.h
        src/Catalogue.cpp include/fpdb/catalogue/Catalogue.h
        src/CatalogueEntry.cpp include/fpdb/catalogue/CatalogueEntry.h
        src/Table.cpp include/fpdb/catalogue/Table.h
        src/Partition.cpp include/fpdb/catalogue/Partition.h
        src/s3/S3CatalogueEntry.cpp include/fpdb/catalogue/s3/S3CatalogueEntry.h
        src/s3/S3Table.cpp include/fpdb/catalogue/s3/S3Table.h
        src/s3/S3Partition.cpp include/fpdb/catalogue/s3/S3Partition.h
        src/s3/S3CatalogueEntryReader.cpp include/fpdb/catalogue/s3/S3CatalogueEntryReader.h
        src/local-fs/LocalFSCatalogueEntry.cpp include/fpdb/catalogue/local-fs/LocalFSCatalogueEntry.h
        src/local-fs/LocalFSTable.cpp include/fpdb/catalogue/local-fs/LocalFSTable.h
        src/local-fs/LocalFSPartition.cpp include/fpdb/catalogue/local-fs/LocalFSPartition.h
        include/fpdb/catalogue/format/FormatType.h
        src/format/Format.cpp include/fpdb/catalogue/format/Format.h
        src/format/CSVFormat.cpp include/fpdb/catalogue/format/CSVFormat.h
        src/format/ParquetFormat.cpp include/fpdb/catalogue/format/ParquetFormat.h
        include/fpdb/catalogue/serialization/PartitionSerializer.h
        include/fpdb/catalogue/serialization/TableSerializer.h
        include/fpdb/catalogue/serialization/FormatSerializer.h)

target_include_directories(fpdb-catalogue PUBLIC include)

target_link_libraries(fpdb-catalogue PUBLIC nlohmann_json)
target_link_libraries(fpdb-catalogue PUBLIC fpdb-tuple)
target_link_libraries(fpdb-catalogue PUBLIC fpdb-aws)
target_link_libraries(fpdb-catalogue PUBLIC fpdb-util)


#-----------------------------------------------------------------------------------------------------------------------
# Diagnostics
#-----------------------------------------------------------------------------------------------------------------------

#showTargetProps(fpdb-catalogue)
