//
// Created by Elena Milkai on 4/20/22.
//

#include "../include/deltapump/BinlogParser.h"

using namespace normal::avro_tuple::make;
using namespace fpdb::logparser;

std::unique_ptr<BinlogParser> binlogParser = std::make_unique<BinlogParser>();

int main() {
    // clock to measure duration
    std::clock_t start;
    double duration;
    start = std::clock();

    for (int i=0; i<10; i++) {
      // vector to stored parsed delta
      std::unordered_map<int, std::set<struct lineorder_record>> *lineorder_record_ptr = nullptr;
      std::unordered_map<int, std::set<struct customer_record>> *customer_record_ptr = nullptr;
      std::unordered_map<int, std::set<struct supplier_record>> *supplier_record_ptr = nullptr;
      std::unordered_map<int, std::set<struct part_record>> *part_record_ptr = nullptr;
      std::unordered_map<int, std::set<struct date_record>> *date_record_ptr = nullptr;
      std::unordered_map<int, std::set<struct history_record>> *history_record_ptr = nullptr;

      /*binlogParser->parse(&lineorder_record_ptr,
                          &customer_record_ptr,
                          &supplier_record_ptr,
                          &part_record_ptr,
                          &date_record_ptr,
                          &history_record_ptr);*/
      std::cout << "Result size: " << lineorder_record_ptr->size() << std::endl;

      for (auto lineorder_pair : (*lineorder_record_ptr)) {
        std::set<struct lineorder_record> lineorder_partition = lineorder_pair.second;
        long count = 0;
        for (auto lineorder_record : lineorder_partition) {
          /*LineorderDelta_t lineorder_delta = lineorder_record.lineorder_delta;
                         std::cout << std::get<0>(lineorder_delta) << ", "
                                   << std::get<1>(lineorder_delta) << ", "
                                   << std::get<2>(lineorder_delta) << ", "
                                   << std::get<3>(lineorder_delta) << ", "
                                   << std::get<4>(lineorder_delta) << ", "
                                   << std::get<5>(lineorder_delta) << ", "
                                   << std::get<6>(lineorder_delta) << ", "
                                   << std::get<7>(lineorder_delta) << ", "
                                   << std::get<8>(lineorder_delta) << ", "
                                   << std::get<9>(lineorder_delta) << ", "
                                   << std::get<10>(lineorder_delta) << ", "
                                   << std::get<11>(lineorder_delta) << ", "
                                   << std::get<12>(lineorder_delta) << ", "
                                   << std::get<13>(lineorder_delta) << ", "
                                   << std::get<14>(lineorder_delta) << ", "
                                   << std::get<15>(lineorder_delta) << ", "
                                   << std::get<16>(lineorder_delta) << ", "
                                   << std::get<17>(lineorder_delta) << std::endl;*/
          count++;
        }
        std::cout << "[LINEORDER] Number of rows for the delta: " << count << std::endl;
      }

      for (auto customer_pair : (*customer_record_ptr)) {
        std::set<struct customer_record> customer_partition = customer_pair.second;
        long count = 0;
        for (auto customer_record : customer_partition) {
          /*CustomerDelta_t customer_delta = customer_record.customer_delta;
          std::cout << std::get<0>(customer_delta) << ", "
                      << std::get<1>(customer_delta) << ", "
                      << std::get<2>(customer_delta) << ", "
                      << std::get<3>(customer_delta) << ", "
                      << std::get<4>(customer_delta) << ", "
                      << std::get<5>(customer_delta) << ", "
                      << std::get<6>(customer_delta) << ", "
                      << std::get<7>(customer_delta) << ", "
                      << std::get<8>(customer_delta) << ", "
                      << std::get<9>(customer_delta) << ", "
                      << std::get<10>(customer_delta) << std::endl;*/
          count++;
        }
        std::cout << "[CUSTOMER] Number of rows for the delta: " << count << std::endl;
      }
      for (auto supplier_pair : (*supplier_record_ptr)) {
        std::set<struct supplier_record> supplier_partition = supplier_pair.second;
        long count = 0;
        for (auto supplier_record : supplier_partition) {
          SupplierDelta_t supplier_delta = supplier_record.supplier_delta;
          std::cout << std::get<0>(supplier_delta) << ", "
                      << std::get<1>(supplier_delta) << ", "
                      << std::get<2>(supplier_delta) << ", "
                      << std::get<3>(supplier_delta) << ", "
                      << std::get<4>(supplier_delta) << ", "
                      << std::get<5>(supplier_delta) << ", "
                      << std::get<6>(supplier_delta) << ", "
                      << std::get<7>(supplier_delta) << ", "
                      << std::get<8>(supplier_delta) << std::endl;
          count++;
        }
        std::cout << "[SUPPLIER] Number of rows for the delta: " << count << std::endl;
      }

      for (auto history_pair : (*history_record_ptr)) {
        std::set<struct history_record> history_partition = history_pair.second;
        long count = 0;
        for (auto history_record : history_partition) {
          HistoryDelta_t history_delta = history_record.history_delta;
          /*std::cout << std::get<0>(history_delta) << ", "
                      << std::get<1>(history_delta) << ", "
                      << std::get<2>(history_delta) << ", "
                      << std::get<3>(history_delta) << ", "
                      << std::get<4>(history_delta) << std::endl;*/
          count++;
        }
        std::cout << "[HISTORY] Number of rows for the delta: " << count << std::endl;
      }

      std::cout << "[DATE] Number of rows for the delta: " << date_record_ptr->size() << std::endl;
      std::cout << "[PART] Number of rows for the delta: " << part_record_ptr->size() << std::endl;

      //duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
      //std::cout<<"main end: "<< duration <<'\n';
    }

    return 0;
}

