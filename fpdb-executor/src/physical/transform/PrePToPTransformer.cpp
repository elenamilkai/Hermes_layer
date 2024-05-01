//
// Created by Yifei Yang on 11/20/21.
//

#include <fpdb/executor/physical/transform/PrePToPTransformer.h>
#include <fpdb/executor/physical/transform/PrePToS3PTransformer.h>
#include <fpdb/executor/physical/sort/SortPOp.h>
#include <fpdb/executor/physical/limitsort/LimitSortPOp.h>
#include <fpdb/executor/physical/aggregate/AggregatePOp.h>
#include <fpdb/executor/physical/aggregate/function/Sum.h>
#include <fpdb/executor/physical/aggregate/function/Count.h>
#include <fpdb/executor/physical/aggregate/function/MinMax.h>
#include <fpdb/executor/physical/aggregate/function/Avg.h>
#include <fpdb/executor/physical/aggregate/function/AvgReduce.h>
#include <fpdb/executor/physical/group/GroupPOp.h>
#include <fpdb/executor/physical/project/ProjectPOp.h>
#include <fpdb/executor/physical/filter/FilterPOp.h>
#include <fpdb/executor/physical/join/hashjoin/HashJoinBuildPOp.h>
#include <fpdb/executor/physical/join/hashjoin/HashJoinProbePOp.h>
#include <fpdb/executor/physical/join/hashjoin/HashJoinPredicate.h>
#include <fpdb/executor/physical/join/nestedloopjoin/NestedLoopJoinPOp.h>
#include <fpdb/executor/physical/shuffle/ShufflePOp.h>
#include <fpdb/executor/physical/split/SplitPOp.h>
#include <fpdb/executor/physical/collate/CollatePOp.h>
#include <fpdb/expression/gandiva/Column.h>

namespace fpdb::executor::physical {
    PrePToPTransformer::PrePToPTransformer(const shared_ptr<PrePhysicalPlan> &prePhysicalPlan,
                                           const shared_ptr<AWSClient> &awsClient,
                                           const shared_ptr<Mode> &mode,
                                           int parallelDegree,
                                           int numNodes) :
            prePhysicalPlan_(prePhysicalPlan),
            awsClient_(awsClient),
            mode_(mode),
            parallelDegree_(parallelDegree),
            numNodes_(numNodes) {}

    shared_ptr<PhysicalPlan> PrePToPTransformer::transform() {
        // transform from root in dfs
        auto rootTransRes = transformDfs(prePhysicalPlan_->getRootOp());
        auto upConnPOps = rootTransRes.first;
        auto allPOps = rootTransRes.second;

        // make a collate operator
        shared_ptr<PhysicalOp> collatePOp = make_shared<collate::CollatePOp>(
                "collate",
                ColumnName::canonicalize(prePhysicalPlan_->getOutputColumnNames()),
                0);
        allPOps.emplace_back(collatePOp);
        connectManyToOne(upConnPOps, collatePOp);
        return make_shared<PhysicalPlan>(allPOps);
    }

    pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
    PrePToPTransformer::transformDfs(const shared_ptr<PrePhysicalOp> &prePOp) {
        switch (prePOp->getType()) {
            case PrePOpType::SORT: {
                const auto &sortPrePOp = std::static_pointer_cast<SortPrePOp>(prePOp);
                return transformSort(sortPrePOp);
            }
            case PrePOpType::LIMIT_SORT: {
                const auto &limitSortPrePOp = std::static_pointer_cast<LimitSortPrePOp>(prePOp);
                return transformLimitSort(limitSortPrePOp);
            }
            case PrePOpType::AGGREGATE: {
                const auto &aggregatePrePOp = std::static_pointer_cast<AggregatePrePOp>(prePOp);
                return transformAggregate(aggregatePrePOp);
            }
            case PrePOpType::GROUP: {
                const auto &groupPrePOp = std::static_pointer_cast<GroupPrePOp>(prePOp);
                return transformGroup(groupPrePOp);
            }
            case PrePOpType::PROJECT: {
                const auto &projectPrePOp = std::static_pointer_cast<ProjectPrePOp>(prePOp);
                return transformProject(projectPrePOp);
            }
            case PrePOpType::FILTER: {
                const auto &filterPrePOp = std::static_pointer_cast<FilterPrePOp>(prePOp);
                return transformFilter(filterPrePOp);
            }
            case PrePOpType::HASH_JOIN: {
                const auto &hashJoinPrePOp = std::static_pointer_cast<HashJoinPrePOp>(prePOp);
                return transformHashJoin(hashJoinPrePOp);
            }
            case PrePOpType::NESTED_LOOP_JOIN: {
                const auto &nestedLoopJoinPrePOp = std::static_pointer_cast<NestedLoopJoinPrePOp>(prePOp);
                return transformNestedLoopJoin(nestedLoopJoinPrePOp);
            }
            case PrePOpType::FILTERABLE_SCAN: {
                const auto &filterableScanPrePOp = std::static_pointer_cast<FilterableScanPrePOp>(prePOp);
                return transformFilterableScan(filterableScanPrePOp);
            }
            default: {
                throw runtime_error(fmt::format("Unsupported prephysical operator type: {}", prePOp->getTypeString()));
            }
        }
    }

    vector<pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>>
    PrePToPTransformer::transformProducers(const shared_ptr<PrePhysicalOp> &prePOp) {
        vector<pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>> transformRes;
        for (const auto &producer: prePOp->getProducers()) {
            transformRes.emplace_back(transformDfs(producer));
        }
        return transformRes;
    }

    pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
    PrePToPTransformer::transformSort(const shared_ptr<SortPrePOp> &sortPrePOp) {
        // id
        auto prePOpId = sortPrePOp->getId();

        // transform producers
        const auto &producersTransRes = transformProducers(sortPrePOp);
        if (producersTransRes.size() != 1) {
            throw runtime_error(fmt::format("Unsupported number of producers for sort, should be {}, but get {}",
                                            1, producersTransRes.size()));
        }

        // transform self
        const auto &producerTransRes = producersTransRes[0];
        auto upConnPOps = producerTransRes.first;
        auto allPOps = producerTransRes.second;

        vector<string> projectColumnNames{sortPrePOp->getProjectColumnNames().begin(),
                                          sortPrePOp->getProjectColumnNames().end()};

        shared_ptr<PhysicalOp> sortPOp = make_shared<sort::SortPOp>(fmt::format("Sort[{}]", prePOpId),
                                                                    projectColumnNames,
                                                                    0,
                                                                    sortPrePOp->getSortKeys());
        allPOps.emplace_back(sortPOp);

        // connect to upstream
        connectManyToOne(upConnPOps, sortPOp);

        return make_pair(vector<shared_ptr<PhysicalOp>>{sortPOp}, allPOps);
    }

    pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
    PrePToPTransformer::transformLimitSort(const shared_ptr<LimitSortPrePOp> &limitSortPrePOp) {
        // id
        auto prePOpId = limitSortPrePOp->getId();

        // transform producers
        const auto &producersTransRes = transformProducers(limitSortPrePOp);
        if (producersTransRes.size() != 1) {
            throw runtime_error(fmt::format("Unsupported number of producers for limitSort, should be {}, but get {}",
                                            1, producersTransRes.size()));
        }

        // transform self
        const auto &producerTransRes = producersTransRes[0];
        auto upConnPOps = producerTransRes.first;
        auto allPOps = producerTransRes.second;

        vector<string> projectColumnNames{limitSortPrePOp->getProjectColumnNames().begin(),
                                          limitSortPrePOp->getProjectColumnNames().end()};

        shared_ptr<PhysicalOp> limitSortPOp = make_shared<limitsort::LimitSortPOp>(
                fmt::format("LimitSort[{}]", prePOpId),
                projectColumnNames,
                0,
                limitSortPrePOp->getK(),
                limitSortPrePOp->getSortKeys());
        allPOps.emplace_back(limitSortPOp);

        // connect to upstream
        connectManyToOne(upConnPOps, limitSortPOp);

        return make_pair(vector<shared_ptr<PhysicalOp>>{limitSortPOp}, allPOps);
    }

    pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
    PrePToPTransformer::transformAggregate(const shared_ptr<AggregatePrePOp> &aggregatePrePOp) {
        // id
        auto prePOpId = aggregatePrePOp->getId();

        // transform producers
        const auto &producersTransRes = transformProducers(aggregatePrePOp);
        if (producersTransRes.size() != 1) {
            throw runtime_error(fmt::format("Unsupported number of producers for aggregate, should be {}, but get {}",
                                            1, producersTransRes.size()));
        }

        // transform self
        const auto &producerTransRes = producersTransRes[0];
        auto upConnPOps = producerTransRes.first;
        auto allPOps = producerTransRes.second;
        bool isParallel = upConnPOps.size() > 1;

        vector<shared_ptr<PhysicalOp>> selfPOps, selfConnUpPOps, selfConnDownPOps;
        vector<string> projectColumnNames{aggregatePrePOp->getProjectColumnNames().begin(),
                                          aggregatePrePOp->getProjectColumnNames().end()};
        vector<string> parallelAggProjectColumnNames;
        if (isParallel) {
            for (uint i = 0; i < aggregatePrePOp->getFunctions().size(); ++i) {
                const auto prePFunction = aggregatePrePOp->getFunctions()[i];
                const auto aggOutputColumnName = aggregatePrePOp->getAggOutputColumnNames()[i];
                if (prePFunction->getType() == AggregatePrePFunctionType::AVG) {
                    parallelAggProjectColumnNames.emplace_back(AggregatePrePFunction::AVG_PARALLEL_SUM_COLUMN_PREFIX + aggOutputColumnName);
                    parallelAggProjectColumnNames.emplace_back(AggregatePrePFunction::AVG_PARALLEL_COUNT_COLUMN_PREFIX + aggOutputColumnName);
                } else {
                    parallelAggProjectColumnNames.emplace_back(aggOutputColumnName);
                }
            }
        } else {
            parallelAggProjectColumnNames = projectColumnNames;
        }

        for (size_t i = 0; i < upConnPOps.size(); ++i) {
            // aggregate functions, better to let each operator has its own copy of aggregate functions
            vector<shared_ptr<aggregate::AggregateFunction>> aggFunctions;
            for (size_t j = 0; j < aggregatePrePOp->getFunctions().size(); ++j) {
                const auto &prePFunction = aggregatePrePOp->getFunctions()[j];
                const auto &aggOutputColumnName = aggregatePrePOp->getAggOutputColumnNames()[j];
                const auto &transAggFunctions = transformAggFunction(aggOutputColumnName,
                                                                     prePFunction,
                                                                     isParallel);
                aggFunctions.insert(aggFunctions.end(), transAggFunctions.begin(), transAggFunctions.end());
            }

            selfPOps.emplace_back(make_shared<aggregate::AggregatePOp>(
                    fmt::format("Aggregate[{}]-{}", prePOpId, i),
                    parallelAggProjectColumnNames,
                    upConnPOps[i]->getNodeId(),
                    aggFunctions));
        }

        // if num > 1, then we need an aggregate reduce operator
        if (upConnPOps.size() == 1) {
            selfConnUpPOps = selfPOps;
            selfConnDownPOps = selfPOps;
        } else {
            // aggregate reduce functions
            vector<shared_ptr<aggregate::AggregateFunction>> aggReduceFunctions;
            for (size_t j = 0; j < aggregatePrePOp->getFunctions().size(); ++j) {
                const auto &prePFunction = aggregatePrePOp->getFunctions()[j];
                const auto &aggOutputColumnName = aggregatePrePOp->getAggOutputColumnNames()[j];
                aggReduceFunctions.emplace_back(transformAggReduceFunction(aggOutputColumnName,
                                                                           prePFunction));
            }

            shared_ptr<PhysicalOp> aggReducePOp = make_shared<aggregate::AggregatePOp>(
                    fmt::format("Aggregate[{}]-Reduce", prePOpId),
                    projectColumnNames,
                    0,
                    aggReduceFunctions);
            connectManyToOne(selfPOps, aggReducePOp);
            selfConnUpPOps = selfPOps;
            selfConnDownPOps.emplace_back(aggReducePOp);
            selfPOps.emplace_back(aggReducePOp);
        }

        // connect to upstream
        connectOneToOne(upConnPOps, selfConnUpPOps);

        // collect all physical ops
        allPOps.insert(allPOps.end(), selfPOps.begin(), selfPOps.end());

        return make_pair(selfConnDownPOps, allPOps);
    }

    pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
    PrePToPTransformer::transformGroup(const shared_ptr<GroupPrePOp> &groupPrePOp) {
        // id
        auto prePOpId = groupPrePOp->getId();

        // transform producers
        const auto &producersTransRes = transformProducers(groupPrePOp);
        if (producersTransRes.size() != 1) {
            throw runtime_error(fmt::format("Unsupported number of producers for group, should be {}, but get {}",
                                            1, producersTransRes.size()));
        }

        // transform self
        auto producerTransRes = producersTransRes[0];
        auto upConnPOps = producerTransRes.first;
        auto allPOps = producerTransRes.second;

        vector<shared_ptr<PhysicalOp>> groupPOps;
        vector<string> projectColumnNames{groupPrePOp->getProjectColumnNames().begin(),
                                          groupPrePOp->getProjectColumnNames().end()};
        for (int i = 0; i < parallelDegree_ * numNodes_; ++i) {
            // aggregate functions, better to let each operator has its own copy of aggregate functions
            vector<shared_ptr<aggregate::AggregateFunction>> aggFunctions;
            for (size_t j = 0; j < groupPrePOp->getFunctions().size(); ++j) {
                const auto &prePFunction = groupPrePOp->getFunctions()[j];
                const auto &aggOutputColumnName = groupPrePOp->getAggOutputColumnNames()[j];
                const auto &transAggFunctions = transformAggFunction(aggOutputColumnName,
                                                                     prePFunction,
                                                                     false);
                aggFunctions.insert(aggFunctions.end(), transAggFunctions.begin(), transAggFunctions.end());
            }

            groupPOps.emplace_back(make_shared<group::GroupPOp>(fmt::format("Group[{}]-{}", prePOpId, i),
                                                                projectColumnNames,
                                                                i % numNodes_,
                                                                groupPrePOp->getGroupColumnNames(),
                                                                aggFunctions));
        }
        allPOps.insert(allPOps.end(), groupPOps.begin(), groupPOps.end());

        // if num > 1, then we add a shuffle stage ahead
        if (parallelDegree_ * numNodes_ == 1) {
            // connect to upstream
            connectManyToOne(producerTransRes.first, groupPOps[0]);
        } else {
            vector<shared_ptr<PhysicalOp>> shufflePOps;
            for (const auto &upConnPOp: producerTransRes.first) {
                shufflePOps.emplace_back(make_shared<shuffle::ShufflePOp>(
                        fmt::format("Shuffle[{}]-{}", prePOpId, upConnPOp->name()),
                        upConnPOp->getProjectColumnNames(),
                        upConnPOp->getNodeId(),
                        groupPrePOp->getGroupColumnNames()));
            }
            connectManyToMany(shufflePOps, groupPOps);
            allPOps.insert(allPOps.end(), shufflePOps.begin(), shufflePOps.end());

            // connect to upstream
            connectOneToOne(producerTransRes.first, shufflePOps);
        }

        return make_pair(groupPOps, allPOps);
    }

    pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
    PrePToPTransformer::transformProject(const shared_ptr<ProjectPrePOp> &projectPrePOp) {
        // id
        auto prePOpId = projectPrePOp->getId();

        // transform producers
        const auto &producersTransRes = transformProducers(projectPrePOp);
        if (producersTransRes.size() != 1) {
            throw runtime_error(fmt::format("Unsupported number of producers for project, should be {}, but get {}",
                                            1, producersTransRes.size()));
        }

        // transform self
        const auto &producerTransRes = producersTransRes[0];
        auto upConnPOps = producerTransRes.first;
        auto allPOps = producerTransRes.second;

        vector<shared_ptr<PhysicalOp>> selfPOps;
        vector<string> projectColumnNames{projectPrePOp->getProjectColumnNames().begin(),
                                          projectPrePOp->getProjectColumnNames().end()};
        for (size_t i = 0; i < upConnPOps.size(); ++i) {
            selfPOps.emplace_back(make_shared<project::ProjectPOp>(fmt::format("Project[{}]-{}", prePOpId, i),
                                                                   projectColumnNames,
                                                                   upConnPOps[i]->getNodeId(),
                                                                   projectPrePOp->getExprs(),
                                                                   projectPrePOp->getExprNames(),
                                                                   projectPrePOp->getProjectColumnNamePairs()));
        }

        // connect to upstream
        connectOneToOne(upConnPOps, selfPOps);

        // collect all physical ops
        allPOps.insert(allPOps.end(), selfPOps.begin(), selfPOps.end());

        return make_pair(selfPOps, allPOps);
    }

    pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
    PrePToPTransformer::transformFilter(const shared_ptr<FilterPrePOp> &filterPrePOp) {
        // id
        auto prePOpId = filterPrePOp->getId();

        // transform producers
        const auto &producersTransRes = transformProducers(filterPrePOp);
        if (producersTransRes.size() != 1) {
            throw runtime_error(fmt::format("Unsupported number of producers for filter, should be {}, but get {}",
                                            1, producersTransRes.size()));
        }

        // transform self
        const auto &producerTransRes = producersTransRes[0];
        auto upConnPOps = producerTransRes.first;
        auto allPOps = producerTransRes.second;

        vector<shared_ptr<PhysicalOp>> selfPOps;
        vector<string> projectColumnNames{filterPrePOp->getProjectColumnNames().begin(),
                                          filterPrePOp->getProjectColumnNames().end()};
        for (size_t i = 0; i < upConnPOps.size(); ++i) {
            selfPOps.emplace_back(make_shared<filter::FilterPOp>(fmt::format("Filter[{}]-{}", prePOpId, i),
                                                                 projectColumnNames,
                                                                 upConnPOps[i]->getNodeId(),
                                                                 filterPrePOp->getPredicate(),
                                                                 nullptr));
        }

        // connect to upstream
        connectOneToOne(upConnPOps, selfPOps);

        // collect all physical ops
        allPOps.insert(allPOps.end(), selfPOps.begin(), selfPOps.end());

        return make_pair(selfPOps, allPOps);
    }

    pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
    PrePToPTransformer::transformHashJoin(const shared_ptr<HashJoinPrePOp> &hashJoinPrePOp) {
        // id
        auto prePOpId = hashJoinPrePOp->getId();

        // transform producers
        const auto &producersTransRes = transformProducers(hashJoinPrePOp);
        if (producersTransRes.size() != 2) {
            throw runtime_error(fmt::format("Unsupported number of producers for hashJoin, should be {}, but get {}",
                                            2, producersTransRes.size()));
        }

        auto leftTransRes = producersTransRes[0];
        auto rightTransRes = producersTransRes[1];
        auto allPOps = leftTransRes.second;
        allPOps.insert(allPOps.end(), rightTransRes.second.begin(), rightTransRes.second.end());

        // transform self
        vector<string> projectColumnNames{hashJoinPrePOp->getProjectColumnNames().begin(),
                                          hashJoinPrePOp->getProjectColumnNames().end()};
        auto joinType = hashJoinPrePOp->getJoinType();
        const auto &leftColumnNames = hashJoinPrePOp->getLeftColumnNames();
        const auto &rightColumnNames = hashJoinPrePOp->getRightColumnNames();
        join::HashJoinPredicate hashJoinPredicate(leftColumnNames, rightColumnNames);
        const auto &hashJoinPredicateStr = hashJoinPredicate.toString();

        vector<shared_ptr<PhysicalOp>> hashJoinBuildPOps, hashJoinProbePOps;
        for (int i = 0; i < parallelDegree_ * numNodes_; ++i) {
            hashJoinBuildPOps.emplace_back(make_shared<join::HashJoinBuildPOp>(
                    fmt::format("HashJoinBuild[{}]-{}-{}", prePOpId, hashJoinPredicateStr, i),
                    projectColumnNames,
                    i % numNodes_,
                    leftColumnNames));
            hashJoinProbePOps.emplace_back(make_shared<join::HashJoinProbePOp>(
                    fmt::format("HashJoinProbe[{}]-{}-{}", prePOpId, hashJoinPredicateStr, i),
                    projectColumnNames,
                    i % numNodes_,
                    hashJoinPredicate,
                    joinType));
        }
        allPOps.insert(allPOps.end(), hashJoinBuildPOps.begin(), hashJoinBuildPOps.end());
        allPOps.insert(allPOps.end(), hashJoinProbePOps.begin(), hashJoinProbePOps.end());
        connectOneToOne(hashJoinBuildPOps, hashJoinProbePOps);

        // if num > 1, then we need shuffle operators
        if (parallelDegree_ * numNodes_ == 1) {
            // connect to upstream
            connectManyToOne(leftTransRes.first, hashJoinBuildPOps[0]);
            connectManyToOne(rightTransRes.first, hashJoinProbePOps[0]);
        } else {
            vector<shared_ptr<PhysicalOp>> shuffleLeftPOps, shuffleRightPOps;
            for (const auto &upLeftConnPOp: leftTransRes.first) {
                shuffleLeftPOps.emplace_back(make_shared<shuffle::ShufflePOp>(
                        fmt::format("Shuffle[{}]-{}", prePOpId, upLeftConnPOp->name()),
                        upLeftConnPOp->getProjectColumnNames(),
                        upLeftConnPOp->getNodeId(),
                        leftColumnNames));
            }
            for (const auto &upRightConnPOp : rightTransRes.first) {
                shuffleRightPOps.emplace_back(make_shared<shuffle::ShufflePOp>(
                        fmt::format("Shuffle[{}]-{}", prePOpId, upRightConnPOp->name()),
                        upRightConnPOp->getProjectColumnNames(),
                        upRightConnPOp->getNodeId(),
                        rightColumnNames));
            }
            connectManyToMany(shuffleLeftPOps, hashJoinBuildPOps);
            connectManyToMany(shuffleRightPOps, hashJoinProbePOps);
            allPOps.insert(allPOps.end(), shuffleLeftPOps.begin(), shuffleLeftPOps.end());
            allPOps.insert(allPOps.end(), shuffleRightPOps.begin(), shuffleRightPOps.end());

            // connect to upstream
            connectOneToOne(leftTransRes.first, shuffleLeftPOps);
            connectOneToOne(rightTransRes.first, shuffleRightPOps);
        };

        return make_pair(hashJoinProbePOps, allPOps);
    }

    pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
    PrePToPTransformer::transformNestedLoopJoin(const shared_ptr<NestedLoopJoinPrePOp> &nestedLoopJoinPrePOp) {
        // id
        auto prePOpId = nestedLoopJoinPrePOp->getId();

        // transform producers
        const auto &producersTransRes = transformProducers(nestedLoopJoinPrePOp);
        if (producersTransRes.size() != 2) {
            throw runtime_error(fmt::format("Unsupported number of producers for nestedLoopJoin, should be {}, but get {}",
                                            2, producersTransRes.size()));
        }

        auto leftTransRes = producersTransRes[0];
        auto rightTransRes = producersTransRes[1];
        auto allPOps = leftTransRes.second;
        allPOps.insert(allPOps.end(), rightTransRes.second.begin(), rightTransRes.second.end());

        // transform self
        vector<string> projectColumnNames{nestedLoopJoinPrePOp->getProjectColumnNames().begin(),
                                          nestedLoopJoinPrePOp->getProjectColumnNames().end()};
        auto joinType = nestedLoopJoinPrePOp->getJoinType();
        optional<shared_ptr<fpdb::expression::gandiva::Expression>> predicate;
        if (nestedLoopJoinPrePOp->getPredicate()) {
            predicate = nestedLoopJoinPrePOp->getPredicate();
        }

        vector<shared_ptr<PhysicalOp>> nestedLoopJoinPOps;
        for (int i = 0; i < parallelDegree_ * numNodes_; ++i) {
            shared_ptr<join::NestedLoopJoinPOp> nestedLoopJoinPOp =
                    make_shared<join::NestedLoopJoinPOp>(fmt::format("NestedLoopJoin[{}]-{}", prePOpId, i),
                                                         projectColumnNames,
                                                         i % numNodes_,
                                                         predicate,
                                                         joinType);
            // connect to left inputs
            for (const auto &upLeftConnPOp: leftTransRes.first) {
                upLeftConnPOp->produce(nestedLoopJoinPOp);
                nestedLoopJoinPOp->addLeftProducer(upLeftConnPOp);
            }
            nestedLoopJoinPOps.emplace_back(nestedLoopJoinPOp);
        }
        allPOps.insert(allPOps.end(), nestedLoopJoinPOps.begin(), nestedLoopJoinPOps.end());

        // connect to right inputs, if num > 1, then we need split operators for right producers
        if (parallelDegree_ * numNodes_ == 1) {
            for (const auto &upRightConnPOp: rightTransRes.first) {
                upRightConnPOp->produce(nestedLoopJoinPOps[0]);
                static_pointer_cast<join::NestedLoopJoinPOp>(nestedLoopJoinPOps[0])->addRightProducer(upRightConnPOp);
            }
        } else {
            vector<shared_ptr<PhysicalOp>> splitPOps;
            for (const auto &upRightConnPOp : rightTransRes.first) {
                shared_ptr<split::SplitPOp> splitPOp = make_shared<split::SplitPOp>(
                        fmt::format("Split[{}]-{}", prePOpId, upRightConnPOp->name()),
                        upRightConnPOp->getProjectColumnNames(),
                        upRightConnPOp->getNodeId());
                splitPOps.emplace_back(splitPOp);

                // Connect splitPOp with all nestedLoopJoinPOps
                for (const auto &nestedLoopJoinPOp: nestedLoopJoinPOps) {
                    splitPOp->produce(nestedLoopJoinPOp);
                    static_pointer_cast<join::NestedLoopJoinPOp>(nestedLoopJoinPOp)->addRightProducer(splitPOp);
                }
            }
            allPOps.insert(allPOps.end(), splitPOps.begin(), splitPOps.end());

            // connect to upstream
            connectOneToOne(rightTransRes.first, splitPOps);
        }

        return make_pair(nestedLoopJoinPOps, allPOps);
    }

    pair<vector<shared_ptr<PhysicalOp>>, vector<shared_ptr<PhysicalOp>>>
    PrePToPTransformer::transformFilterableScan(const shared_ptr<FilterableScanPrePOp> &filterableScanPrePOp) {
        const auto s3PTransformer = make_shared<PrePToS3PTransformer>(filterableScanPrePOp->getId(),
                                                                      awsClient_,
                                                                      mode_,
                                                                      numNodes_);
        return s3PTransformer->transformFilterableScan(filterableScanPrePOp);

    }

    vector<shared_ptr<aggregate::AggregateFunction>>
    PrePToPTransformer::transformAggFunction(const string &outputColumnName,
                                             const shared_ptr<AggregatePrePFunction> &prePFunction,
                                             bool hasReduceOp) {
        switch (prePFunction->getType()) {
            case plan::prephysical::SUM: {
                return {make_shared<aggregate::Sum>(outputColumnName, prePFunction->getExpression())};
            }
            case plan::prephysical::COUNT: {
                return {make_shared<aggregate::Count>(outputColumnName, prePFunction->getExpression())};
            }
            case plan::prephysical::MIN: {
                return {make_shared<aggregate::MinMax>(true, outputColumnName, prePFunction->getExpression())};
            }
            case plan::prephysical::MAX: {
                return {make_shared<aggregate::MinMax>(false, outputColumnName, prePFunction->getExpression())};
            }
            case plan::prephysical::AVG: {
                if (hasReduceOp) {
                    auto sumFunc = make_shared<aggregate::Sum>(
                            AggregatePrePFunction::AVG_PARALLEL_SUM_COLUMN_PREFIX + outputColumnName,
                            prePFunction->getExpression());
                    auto countFunc = make_shared<aggregate::Count>(
                            AggregatePrePFunction::AVG_PARALLEL_COUNT_COLUMN_PREFIX + outputColumnName,
                            prePFunction->getExpression());
                    return {sumFunc, countFunc};
                } else {
                    return {make_shared<aggregate::Avg>(outputColumnName, prePFunction->getExpression())};
                }
            }
            default: {
                throw runtime_error(fmt::format("Unsupported aggregate function type: {}", prePFunction->getTypeString()));
            }
        }
    }

    shared_ptr<aggregate::AggregateFunction>
    PrePToPTransformer::transformAggReduceFunction(const string &outputColumnName,
                                                   const shared_ptr<AggregatePrePFunction> &prePFunction) {
        switch (prePFunction->getType()) {
            case plan::prephysical::SUM:
            case plan::prephysical::COUNT: {
                return make_shared<aggregate::Sum>(outputColumnName,
                                                   fpdb::expression::gandiva::col(outputColumnName));
            }
            case plan::prephysical::MIN: {
                return make_shared<aggregate::MinMax>(true,
                                                      outputColumnName,
                                                      fpdb::expression::gandiva::col(outputColumnName));
            }
            case plan::prephysical::MAX: {
                return make_shared<aggregate::MinMax>(false,
                                                      outputColumnName,
                                                      fpdb::expression::gandiva::col(outputColumnName));
            }
            case plan::prephysical::AVG: {
                return make_shared<aggregate::AvgReduce>(outputColumnName,nullptr);
            }
            default: {
                throw runtime_error(fmt::format("Unsupported aggregate function type for parallel execution: {}", prePFunction->getTypeString()));
            }
        }
    }

    void PrePToPTransformer::connectOneToOne(vector<shared_ptr<PhysicalOp>> &producers,
                                             vector<shared_ptr<PhysicalOp>> &consumers) {
        if (producers.size() != consumers.size()) {
            throw runtime_error(fmt::format("Bad one-to-one operator connection input, producers has {}, but consumers has {}",
                                            producers.size(), consumers.size()));
        }
        for (size_t i = 0; i < producers.size(); ++i) {
            producers[i]->produce(consumers[i]);
            consumers[i]->consume(producers[i]);
        }
    }

    void PrePToPTransformer::connectManyToMany(vector<shared_ptr<PhysicalOp>> &producers,
                                               vector<shared_ptr<PhysicalOp>> &consumers) {
        for (const auto &producer: producers) {
            for (const auto &consumer: consumers) {
                producer->produce(consumer);
                consumer->consume(producer);
            }
        }
    }

    void PrePToPTransformer::connectManyToOne(vector<shared_ptr<PhysicalOp>> &producers,
                                              shared_ptr<PhysicalOp> &consumer) {
        for (const auto &producer: producers) {
            producer->produce(consumer);
            consumer->consume(producer);
        }
    }
}
