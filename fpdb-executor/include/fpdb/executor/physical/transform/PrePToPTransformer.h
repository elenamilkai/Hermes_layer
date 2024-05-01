//
// Created by Yifei Yang on 11/20/21.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_PREPTOPTRANSFORMER_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_PREPTOPTRANSFORMER_H

#include <fpdb/executor/physical/PhysicalPlan.h>
#include <fpdb/executor/physical/aggregate/function/AggregateFunction.h>
#include <fpdb/plan/prephysical/PrePhysicalPlan.h>
#include <fpdb/plan/prephysical/SortPrePOp.h>
#include <fpdb/plan/prephysical/LimitSortPrePOp.h>
#include <fpdb/plan/prephysical/AggregatePrePOp.h>
#include <fpdb/plan/prephysical/AggregatePrePFunction.h>
#include <fpdb/plan/prephysical/GroupPrePOp.h>
#include <fpdb/plan/prephysical/ProjectPrePOp.h>
#include <fpdb/plan/prephysical/FilterPrePOp.h>
#include <fpdb/plan/prephysical/HashJoinPrePOp.h>
#include <fpdb/plan/prephysical/NestedLoopJoinPrePOp.h>
#include <fpdb/plan/prephysical/FilterableScanPrePOp.h>
#include <fpdb/plan/Mode.h>
#include <fpdb/aws/AWSClient.h>

using namespace fpdb::plan;
using namespace fpdb::plan::prephysical;
using namespace fpdb::aws;

namespace fpdb::executor::physical {

    class PrePToPTransformer {
    public:
        PrePToPTransformer(const shared_ptr<PrePhysicalPlan> &prePhysicalPlan,
                           const shared_ptr<AWSClient> &awsClient,
                           const shared_ptr<Mode> &mode,
                           int parallelDegree,
                           int numNodes);
        shared_ptr<PhysicalPlan> transform();


    private:
        /**
         * Transform prephysical op to physical op in dfs style
         * @param prePOp: prephysical op
         * @return a pair of connect physical ops (to producer) and current all (cumulative) physical ops
         */
        pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
        transformDfs(const shared_ptr<PrePhysicalOp> &prePOp);

        vector<pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>>
        transformProducers(const shared_ptr<PrePhysicalOp> &prePOp);

        pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
        transformSort(const shared_ptr<SortPrePOp> &sortPrePOp);

        pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
        transformLimitSort(const shared_ptr<LimitSortPrePOp> &limitSortPrePOp);

        pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
        transformAggregate(const shared_ptr<AggregatePrePOp> &aggregatePrePOp);

        pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
        transformGroup(const shared_ptr<GroupPrePOp> &groupPrePOp);

        pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
        transformProject(const shared_ptr<ProjectPrePOp> &projectPrePOp);

        pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
        transformFilter(const shared_ptr<FilterPrePOp> &filterPrePOp);

        pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
        transformHashJoin(const shared_ptr<HashJoinPrePOp> &hashJoinPrePOp);

        pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
        transformNestedLoopJoin(const shared_ptr<NestedLoopJoinPrePOp> &nestedLoopJoinPrePOp);

        pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
        transformFilterableScan(const shared_ptr<FilterableScanPrePOp> &filterableScanPrePOp);

        /**
         * Transform aggregate and aggregate reduce function
         * @param outputColumnName
         * @param prePFunction
         * @param hasReduceOp whether there is a reduce op as the consumer for all parallel ops
         * @return
         */
        vector<shared_ptr<aggregate::AggregateFunction>>
        transformAggFunction(const string &outputColumnName,
                             const shared_ptr<AggregatePrePFunction> &prePFunction,
                             bool hasReduceOp);
        shared_ptr<aggregate::AggregateFunction> transformAggReduceFunction(const string &outputColumnName,
                                                                            const shared_ptr<AggregatePrePFunction> &prePFunction);

        /**
         * Connect producers and consumers
         * @param producers
         * @param consumers
         */
        void connectOneToOne(vector<shared_ptr<PhysicalOp>> &producers,
                             vector<shared_ptr<PhysicalOp>> &consumers);
        void connectManyToMany(vector<shared_ptr<PhysicalOp>> &producers,
                               vector<shared_ptr<PhysicalOp>> &consumers);
        void connectManyToOne(vector<shared_ptr<PhysicalOp>> &producers,
                              shared_ptr<PhysicalOp> &consumer);
        shared_ptr<PrePhysicalPlan> prePhysicalPlan_;
        shared_ptr<AWSClient> awsClient_;
        shared_ptr<Mode> mode_;
        int parallelDegree_;
        int numNodes_;

    };

}

#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_PREPTOPTRANSFORMER_H
