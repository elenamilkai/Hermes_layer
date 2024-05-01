//
// Created by Elena Milkai on 8/6/22.
//

#ifndef FPDB_DELTACACHEMETADATA_H
#define FPDB_DELTACACHEMETADATA_H

#include <fpdb/tuple/TupleSet.h>

using namespace fpdb::tuple;
enum mode {resultGenerator , selectionVector, keepMapCache, keepMapCache2};
extern mode deltaMergeMode;
extern bool flag;
extern std::queue<std::shared_ptr<arrow::Table>> lo_batchQueue;
extern std::queue<std::shared_ptr<arrow::Table>> c_batchQueue;
extern std::queue<std::shared_ptr<arrow::Table>> s_batchQueue;
extern std::queue<std::shared_ptr<arrow::Table>> p_batchQueue;
extern std::queue<std::shared_ptr<arrow::Table>> d_batchQueue;

extern std::mutex lo_mtx_;
extern std::mutex c_mtx_;
extern std::mutex p_mtx_;
extern std::mutex s_mtx_;
extern std::mutex d_mtx_;

extern std::condition_variable lo_vBatchReady;
extern std::condition_variable c_vBatchReady;
extern std::condition_variable s_vBatchReady;
extern std::condition_variable p_vBatchReady;
extern std::condition_variable d_vBatchReady;

///extern std::atomic<int> tablesScanned;

#endif//FPDB_DELTACACHEMETADATA_H
