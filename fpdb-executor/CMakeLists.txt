project(fpdb-executor VERSION "${PROJECT_VERSION}" LANGUAGES C CXX)


#-----------------------------------------------------------------------------------------------------------------------
# Target
#-----------------------------------------------------------------------------------------------------------------------

add_library(fpdb-executor STATIC
#        messages
        src/message/Envelope.cpp include/fpdb/executor/message/Envelope.h
        include/fpdb/executor/message/MessageType.h
        src/message/Message.cpp include/fpdb/executor/message/Message.h
        src/message/StartMessage.cpp include/fpdb/executor/message/StartMessage.h
        src/message/ConnectMessage.cpp include/fpdb/executor/message/ConnectMessage.h
        src/message/CompleteMessage.cpp include/fpdb/executor/message/CompleteMessage.h
        src/message/TupleMessage.cpp include/fpdb/executor/message/TupleMessage.h
        src/message/ScanMessage.cpp include/fpdb/executor/message/ScanMessage.h
        src/message/TupleSetIndexMessage.cpp include/fpdb/executor/message/TupleSetIndexMessage.h
        src/message/ErrorMessage.cpp include/fpdb/executor/message/ErrorMessage.h
        src/message/cache/CacheMetricsMessage.cpp include/fpdb/executor/message/cache/CacheMetricsMessage.h
        src/message/cache/LoadRequestMessage.cpp include/fpdb/executor/message/cache/LoadRequestMessage.h
        src/message/cache/LoadResponseMessage.cpp include/fpdb/executor/message/cache/LoadResponseMessage.h
        src/message/cache/StoreRequestMessage.cpp include/fpdb/executor/message/cache/StoreRequestMessage.h
        src/message/cache/WeightRequestMessage.cpp include/fpdb/executor/message/cache/WeightRequestMessage.h
#        base physical operators, operator actors, segment cache actors and required sources
        src/cache/SegmentCacheActor.cpp include/fpdb/executor/cache/SegmentCacheActor.h
        include/fpdb/executor/physical/Forward.h
        include/fpdb/executor/physical/POpRelationshipType.h
        src/physical/POpConnection.cpp include/fpdb/executor/physical/POpConnection.h
        src/physical/LocalPOpDirectoryEntry.cpp include/fpdb/executor/physical/LocalPOpDirectoryEntry.h
        src/physical/LocalPOpDirectory.cpp include/fpdb/executor/physical/LocalPOpDirectory.h
        src/physical/POpDirectory.cpp include/fpdb/executor/physical/POpDirectory.h
        src/physical/POpDirectoryEntry.cpp include/fpdb/executor/physical/POpDirectoryEntry.h
        src/physical/POpActor.cpp include/fpdb/executor/physical/POpActor.h
        include/fpdb/executor/physical/POpActor2.h
        src/physical/POpContext.cpp include/fpdb/executor/physical/POpContext.h
        src/physical/PhysicalOp.cpp include/fpdb/executor/physical/PhysicalOp.h
        include/fpdb/executor/physical/POpType.h
#        physical operators
        include/fpdb/executor/physical/Globals.h
        src/physical/sort/SortPOp.cpp include/fpdb/executor/physical/sort/SortPOp.h
        src/physical/limitsort/LimitSortPOp.cpp include/fpdb/executor/physical/limitsort/LimitSortPOp.h
        src/physical/aggregate/AggregatePOp.cpp include/fpdb/executor/physical/aggregate/AggregatePOp.h
        src/physical/aggregate/AggregateResult.cpp include/fpdb/executor/physical/aggregate/AggregateResult.h
        include/fpdb/executor/physical/aggregate/function/AggregateFunctionType.h
        src/physical/aggregate/function/AggregateFunction.cpp include/fpdb/executor/physical/aggregate/function/AggregateFunction.h
        src/physical/aggregate/function/Sum.cpp include/fpdb/executor/physical/aggregate/function/Sum.h
        src/physical/aggregate/function/Count.cpp include/fpdb/executor/physical/aggregate/function/Count.h
        src/physical/aggregate/function/MinMax.cpp include/fpdb/executor/physical/aggregate/function/MinMax.h
        src/physical/aggregate/function/AvgBase.cpp include/fpdb/executor/physical/aggregate/function/AvgBase.h
        src/physical/aggregate/function/Avg.cpp include/fpdb/executor/physical/aggregate/function/Avg.h
        src/physical/aggregate/function/AvgReduce.cpp include/fpdb/executor/physical/aggregate/function/AvgReduce.h
        src/physical/cache/CacheLoadPOp.cpp include/fpdb/executor/physical/cache/CacheLoadPOp.h
        src/physical/cache/CacheHelper.cpp include/fpdb/executor/physical/cache/CacheHelper.h
        src/physical/collate/CollatePOp.cpp include/fpdb/executor/physical/collate/CollatePOp.h
        src/physical/collate/CollatePOp2.cpp include/fpdb/executor/physical/collate/CollatePOp2.h
        src/physical/file/FileScanPOp.cpp include/fpdb/executor/physical/file/FileScanPOp.h
        src/physical/file/FileScanPOp2.cpp include/fpdb/executor/physical/file/FileScanPOp2.h
        src/physical/file/FileScanKernel.cpp include/fpdb/executor/physical/file/FileScanKernel.h
        src/physical/filter/FilterPOp.cpp include/fpdb/executor/physical/filter/FilterPOp.h
        src/physical/group/GroupPOp.cpp include/fpdb/executor/physical/group/GroupPOp.h
        src/physical/group/GroupKernel.cpp include/fpdb/executor/physical/group/GroupKernel.h
        src/physical/join/hashjoin/HashJoinBuildPOp.cpp include/fpdb/executor/physical/join/hashjoin/HashJoinBuildPOp.h
        src/physical/join/hashjoin/HashJoinProbePOp.cpp include/fpdb/executor/physical/join/hashjoin/HashJoinProbePOp.h
        src/physical/join/hashjoin/HashJoinBuildKernel.cpp include/fpdb/executor/physical/join/hashjoin/HashJoinBuildKernel.h
        src/physical/join/hashjoin/HashJoinProbeAbstractKernel.cpp include/fpdb/executor/physical/join/hashjoin/HashJoinProbeAbstractKernel.h
        src/physical/join/hashjoin/HashJoinProbeKernel.cpp include/fpdb/executor/physical/join/hashjoin/HashJoinProbeKernel.h
        src/physical/join/hashjoin/HashSemiJoinProbeKernel.cpp include/fpdb/executor/physical/join/hashjoin/HashSemiJoinProbeKernel.h
        src/physical/join/hashjoin/HashJoinPredicate.cpp include/fpdb/executor/physical/join/hashjoin/HashJoinPredicate.h
        src/physical/join/hashjoin/RecordBatchHashJoiner.cpp include/fpdb/executor/physical/join/hashjoin/RecordBatchHashJoiner.h
        src/physical/join/hashjoin/RecordBatchHashSemiJoiner.cpp include/fpdb/executor/physical/join/hashjoin/RecordBatchHashSemiJoiner.h
        src/physical/join/nestedloopjoin/NestedLoopJoinPOp.cpp include/fpdb/executor/physical/join/nestedloopjoin/NestedLoopJoinPOp.h
        src/physical/join/nestedloopjoin/NestedLoopJoinKernel.cpp include/fpdb/executor/physical/join/nestedloopjoin/NestedLoopJoinKernel.h
        src/physical/join/nestedloopjoin/RecordBatchNestedLoopJoiner.cpp include/fpdb/executor/physical/join/nestedloopjoin/RecordBatchNestedLoopJoiner.h
        src/physical/join/OuterJoinHelper.cpp include/fpdb/executor/physical/join/OuterJoinHelper.h
        src/physical/merge/MergePOp.cpp include/fpdb/executor/physical/merge/MergePOp.h
        src/physical/merge/MergeKernel.cpp include/fpdb/executor/physical/merge/MergeKernel.h
        src/physical/project/ProjectPOp.cpp include/fpdb/executor/physical/project/ProjectPOp.h
        src/physical/shuffle/ShufflePOp.cpp include/fpdb/executor/physical/shuffle/ShufflePOp.h
        src/physical/shuffle/ShuffleKernel.cpp include/fpdb/executor/physical/shuffle/ShuffleKernel.h
        src/physical/shuffle/RecordBatchShuffler.cpp include/fpdb/executor/physical/shuffle/RecordBatchShuffler.h
        src/physical/split/SplitPOp.cpp include/fpdb/executor/physical/split/SplitPOp.h
        src/physical/s3/S3GetPOp.cpp include/fpdb/executor/physical/s3/S3GetPOp.h
        src/physical/s3/S3SelectPOp.cpp include/fpdb/executor/physical/s3/S3SelectPOp.h
        src/physical/s3/S3SelectScanAbstractPOp.cpp include/fpdb/executor/physical/s3/S3SelectScanAbstractPOp.h
        src/physical/s3/S3CSVParser.cpp include/fpdb/executor/physical/s3/S3CSVParser.h
        src/physical/s3/S3SelectCSVParseOptions.cpp include/fpdb/executor/physical/s3/S3SelectCSVParseOptions.h
        src/physical/s3/S3SelectScanKernel.cpp include/fpdb/executor/physical/s3/S3SelectScanKernel.h
        src/physical/deltamerge/DeltaMergePOp.cpp include/fpdb/executor/physical/deltamerge/DeltaMergePOp.h
        src/physical/deltamerge/DeleteMapGeneratorKernel.cpp include/fpdb/executor/physical/deltamerge/DeleteMapGeneratorKernel.h
        src/physical/deltamerge/ResultGeneratorKernel.cpp include/fpdb/executor/physical/deltamerge/ResultGeneratorKernel.h
#        physical plan
        src/physical/PhysicalPlan.cpp include/fpdb/executor/physical/PhysicalPlan.h
        src/physical/transform/PrePToPTransformer.cpp include/fpdb/executor/physical/transform/PrePToPTransformer.h
        src/physical/transform/PrePToS3PTransformer.cpp include/fpdb/executor/physical/transform/PrePToS3PTransformer.h
        src/physical/prune/PartitionPruner.cpp include/fpdb/executor/physical/prune/PartitionPruner.h
#        CAF serializer
        include/fpdb/executor/serialization/MessageSerializer.h
        include/fpdb/executor/serialization/POpSerializer.h
        include/fpdb/executor/serialization/AggregateFunctionSerializer.h
        include/fpdb/executor/serialization/HashJoinProbeAbstractKernelSerializer.h
#        executor
        src/Executor.cpp include/fpdb/executor/Executor.h
        src/Execution.cpp include/fpdb/executor/Execution.h
        # delta cache actor
        src/delta/DeltaCacheActor.cpp include/fpdb/executor/delta/DeltaCacheActor.h
        # delta cache message
        src/message/delta/LoadDeltaRequestMessage.cpp include/fpdb/executor/message/delta/LoadDeltaRequestMessage.h
        src/message/delta/LoadDeltaResponseMessage.cpp include/fpdb/executor/message/delta/LoadDeltaResponseMessage.h
        src/message/delta/StoreDeltaRequestMessage.cpp include/fpdb/executor/message/delta/StoreDeltaRequestMessage.h
        src/message/delta/LoadDiskDeltasResponseMessage.cpp include/fpdb/executor/message/delta/LoadDiskDeltasResponseMessage.h
        src/message/delta/MergedDeltaMessage.cpp include/fpdb/executor/message/delta/MergedDeltaMessage.h

        # delta merge operator
        src/physical/deltamerge/DeleteMapGeneratorKernel.cpp include/fpdb/executor/physical/deltamerge/DeleteMapGeneratorKernel.h
        src/physical/deltamerge/ResultGeneratorKernel.cpp include/fpdb/executor/physical/deltamerge/ResultGeneratorKernel.h
        src/physical/deltamerge/DeltaMergePOp.cpp include/fpdb/executor/physical/deltamerge/DeltaMergePOp.h
        src/physical/deltacache/CacheHandlerPOp.cpp include/fpdb/executor/physical/deltacache/CacheHandlerPOp.h
        src/physical/deltamerge/DeltaMergeFilter.cpp include/fpdb/executor/physical/deltamerge/DeltaMergeFilter.h
        src/physical/deltamerge/KeepMapCacheMode.cpp include/fpdb/executor/physical/deltamerge/KeepMapCacheMode.h

        # delta pump helper
        src/delta/DeltaPumpHelper.cpp include/fpdb/executor/delta/DeltaPumpHelper.h
        src/physical/deltacache/CacheHandlerPOp.cpp include/fpdb/executor/physical/deltacache/CacheHandlerPOp.h

        include/fpdb/executor/delta/PeriodicGetTailHandler.h
        src/delta/PeriodicGetTailHandler.cpp include/fpdb/executor/message/delta/MergedDeltaMessage.h src/message/delta/MergedDeltaMessage.cpp
        test/testGandivaOrder.cpp)

target_include_directories(fpdb-executor PUBLIC include)

target_link_libraries(fpdb-executor PUBLIC fpdb-plan)
target_link_libraries(fpdb-executor PUBLIC fpdb-cache)
target_link_libraries(fpdb-executor PUBLIC fpdb-delta)
target_link_libraries(fpdb-executor PUBLIC fpdb-logparser)
target_link_libraries(fpdb-executor PUBLIC fpdb-caf)
target_link_libraries(fpdb-executor PUBLIC graphviz_gvc_shared)
target_link_libraries(fpdb-executor PUBLIC graphviz_dot_layout_shared)
target_link_libraries(fpdb-executor PUBLIC graphviz_neato_layout_shared)
target_link_libraries(fpdb-executor PUBLIC /usr/local/lib/libavrocpp.so)
target_link_libraries(fpdb-expression-gandiva PUBLIC fpdb-expression)
#-----------------------------------------------------------------------------------------------------------------------
# Diagnostics
#-----------------------------------------------------------------------------------------------------------------------

#showTargetProps(fpdb-executor)

add_executable(fpdb-gandivaOrder-test
        test/testGandivaOrder.cpp)

target_link_libraries(fpdb-gandivaOrder-test
        PUBLIC fpdb-executor)

target_link_libraries(fpdb-gandivaOrder-test
        PUBLIC fpdb-expression)