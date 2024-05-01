//
// Created by Yifei Yang on 2/9/21.
//

#ifndef FPDB_UTIL_UTIL_H
#define FPDB_UTIL_UTIL_H

#include <tl/expected.hpp>
#include <vector>
#include <unordered_map>
#include <set>

using namespace std;

namespace fpdb::util {
  /**
   * File utils
   * @param filePath
   * @return
   */
  string readFile(const string& filePath);
  vector<string> readFileByLine(const string& filePath);

  /**
   * Config utils
   * @param fileName
   * @return
   */
  unordered_map<string, string> readConfig(const string &fileName);

  /**
   * Read ip of all remote nodes from "cluster_ip"
   * @return
   */
  vector<string> readRemoteIps();

  /**
   * Parsing bool string
   * @param stringToParse
   * @return
   */
  bool parseBool(const string& stringToParse);

  /**
   * Union two vectors or one vector with one set
   */
  template<typename T>
  vector<T> union_(const vector<T> &vec1, const set<T> &set2) {
    set<T> unionSet{vec1.begin(), vec1.end()};
    unionSet.insert(set2.begin(), set2.end());
    return vector<T>{unionSet.begin(), unionSet.end()};
  }

  template<typename T>
  vector<T> union_(const vector<T> &vec1, const vector<T> &vec2) {
    set<T> unionSet{vec1.begin(), vec1.end()};
    unionSet.insert(vec2.begin(), vec2.end());
    return vector<T>{unionSet.begin(), unionSet.end()};
  }

  size_t hashCombine(const vector<size_t> &hashes);

  /**
   * Given a start and finish number, will create pairs of numbers from start to finish (inclusive)
   * evenly split across the number of ranges given.
   *
   * E.g.
   * ranges(0,8,3) -> [[0,1,2][3,4,5][6,7,8]]
   * ranges(0,9,3) -> [[0,1,2,3][4,5,6,7][8,9]]
   * ranges(0,10,3) -> [[0,1,2,3][4,5,6,7][8,9,10]]
   *
   * @tparam T
   * @param start
   * @param finish
   * @param numRanges
   * @return
   */
  template<typename T>
  static vector<pair<T, T>> ranges(T start, T finish, T numRanges) {
    vector<pair<T, T>> result;

    T rangeSize = ((finish - start) / numRanges) + 1;

    for (int i = 0; i < numRanges; ++i) {
      T rangeStart = rangeSize * i;
      T rangeStop = min((rangeStart + rangeSize) - 1, finish);
      result.push_back(pair{rangeStart, rangeStop});
    }

    return result;
  }

  bool isInteger(const string& str);
  tl::expected<string, string> execCmd(const char *cmd);
  tl::expected<string, string> getLocalIp();
}


#endif //FPDB_UTIL_UTIL_H
