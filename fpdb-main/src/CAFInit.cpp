//
// Created by Yifei Yang on 1/14/22.
//

#include <fpdb/main/CAFInit.h>
#include <fpdb/executor/physical/POpActor.h>
#include <fpdb/executor/physical/POpActor2.h>
#include <fpdb/executor/physical/collate/CollatePOp2.h>
#include <fpdb/executor/physical/file/FileScanPOp2.h>
#include <fpdb/executor/cache/SegmentCacheActor.h>
#include <fpdb/executor/delta/DeltaCacheActor.h>
#include <fpdb/executor/message/Envelope.h>
#include <fpdb/executor/serialization/MessageSerializer.h>
#include <fpdb/executor/serialization/POpSerializer.h>
#include <fpdb/executor/serialization/AggregateFunctionSerializer.h>
#include <fpdb/executor/serialization/HashJoinProbeAbstractKernelSerializer.h>
#include <fpdb/catalogue/serialization/TableSerializer.h>
#include <fpdb/catalogue/serialization/FormatSerializer.h>
#include <fpdb/expression/gandiva/serialization/ExpressionSerializer.h>
#include <fpdb/tuple/serialization/FileReaderSerializer.h>
#include <fpdb/delta/KeepMapCacheKey.h>
#include <fpdb/delta/KeepMapCacheData.h>
#include <fpdb/delta/StableBmCacheKey.h>
#include <fpdb/delta/StableBmCacheData.h>

namespace fpdb::main {

void CAFInit::initCAFGlobalMetaObjects() {
  ::caf::init_global_meta_objects<::caf::id_block::SegmentCacheActor>();
  ::caf::init_global_meta_objects<::caf::id_block::Envelope>();
  ::caf::init_global_meta_objects<::caf::id_block::POpActor>();
  ::caf::init_global_meta_objects<::caf::id_block::POpActor2>();
  ::caf::init_global_meta_objects<::caf::id_block::CollatePOp2>();
  ::caf::init_global_meta_objects<::caf::id_block::FileScanPOp2>();
  ::caf::init_global_meta_objects<::caf::id_block::TupleSet>();
  ::caf::init_global_meta_objects<::caf::id_block::Message>();
  ::caf::init_global_meta_objects<::caf::id_block::SegmentKey>();
  ::caf::init_global_meta_objects<::caf::id_block::Partition>();
  ::caf::init_global_meta_objects<::caf::id_block::SegmentMetadata>();
  ::caf::init_global_meta_objects<::caf::id_block::SegmentData>();
  ::caf::init_global_meta_objects<::caf::id_block::Column>();
  ::caf::init_global_meta_objects<::caf::id_block::TupleSetIndex>();
  ::caf::init_global_meta_objects<::caf::id_block::TupleKey>();
  ::caf::init_global_meta_objects<::caf::id_block::TupleKeyElement>();
  ::caf::init_global_meta_objects<::caf::id_block::POp>();
  ::caf::init_global_meta_objects<::caf::id_block::Scalar>();
  ::caf::init_global_meta_objects<::caf::id_block::AggregateFunction>();
  ::caf::init_global_meta_objects<::caf::id_block::Expression>();
  ::caf::init_global_meta_objects<::caf::id_block::POpContext>();
  ::caf::init_global_meta_objects<::caf::id_block::FileReader>();
  ::caf::init_global_meta_objects<::caf::id_block::Table>();
  ::caf::init_global_meta_objects<::caf::id_block::Format>();
  ::caf::init_global_meta_objects<::caf::id_block::HashJoinProbeAbstractKernel>();
  ::caf::init_global_meta_objects<::caf::id_block::AggregateResult>();
  // HTAP
  ::caf::init_global_meta_objects<::caf::id_block::DeltaCacheActor>();
  ::caf::init_global_meta_objects<::caf::id_block::DeltaCacheKey>();
  ::caf::init_global_meta_objects<::caf::id_block::DeltaCacheData>();
  ::caf::init_global_meta_objects<::caf::id_block::KeepMapCacheKey>();
  ::caf::init_global_meta_objects<::caf::id_block::KeepMapCacheData>();
  ::caf::init_global_meta_objects<::caf::id_block::StableBmCacheKey>();
  ::caf::init_global_meta_objects<::caf::id_block::StableBmCacheData>();

  ::caf::core::init_global_meta_objects();
  ::caf::io::middleman::init_global_meta_objects();
}

}
