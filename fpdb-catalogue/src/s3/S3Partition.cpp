//
// Created by matt on 15/4/20.
//

#include <fpdb/catalogue/s3/S3Partition.h>
#include <utility>
#include <sstream>

namespace fpdb::catalogue::s3 {

S3Partition::S3Partition(string s3Bucket, 
                         string s3Object) :
	s3Bucket_(move(s3Bucket)),
	s3Object_(move(s3Object)) {}

S3Partition::S3Partition(string s3Bucket,
                         string s3Object,
                         long numBytes) :
  s3Bucket_(move(s3Bucket)),
  s3Object_(move(s3Object)) {
  setNumBytes(numBytes);
}

const string &S3Partition::getBucket() const {
  return s3Bucket_;
}

const string &S3Partition::getObject() const {
  return s3Object_;
}

string S3Partition::toString() {
  return "s3://" + s3Bucket_ + "/" + s3Object_;
}

size_t S3Partition::hash() {
  return std::hash<string>()("s3://" + s3Bucket_ + "/" + s3Object_);
}

bool S3Partition::equalTo(shared_ptr<Partition> other) {
  auto typedOther = static_pointer_cast<const S3Partition>(other);
  if(!typedOther){
	return false;
  }
  else{
	return this->operator==(*typedOther);
  }
}

CatalogueEntryType S3Partition::getCatalogueEntryType() {
  return S3;
}

long S3Partition::toRawPartition() {
  // process the s3 object string
  std::stringstream s3ObjectStream(this->s3Object_);
  std::string segment;
  std::vector<std::string> segmentList;
  while (std::getline(s3ObjectStream, segment, '.')) {
    segmentList.push_back(segment);
  }
  // check the segment length
  if (segmentList.size() <= 2) {
    // if there is only one partition
    return 0;
  } else {
    // if there is multiple partition, then the last number is the partition number
    return std::stol(segmentList[segmentList.size() - 1]);
  }
}

bool S3Partition::operator==(const S3Partition &other) {
  return s3Bucket_ == other.s3Bucket_ && s3Object_ == other.s3Object_;
}

bool S3Partition::operator==(long rawPartition) {
  return toRawPartition() == rawPartition;
}

}
