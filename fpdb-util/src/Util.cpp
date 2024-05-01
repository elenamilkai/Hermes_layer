//
// Created by Yifei Yang on 2/9/21.
//

#include <fpdb/util/Util.h>
#include <fmt/format.h>
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace fpdb::util;

string fpdb::util::readFile(const string& filePath) {
  ifstream inFile(filePath);
  if (!inFile.good()) {
    throw runtime_error(fmt::format("File not exists: {}", filePath));
  }
  ostringstream buf;
  char ch;
  while (buf && inFile.get(ch)) {
    buf.put(ch);
  }
  return buf.str();
}

vector<string> fpdb::util::readFileByLine(const string &filePath) {
  ifstream inFile(filePath);
  if (!inFile.good()) {
    throw runtime_error(fmt::format("File not exists: {}", filePath));
  }
  vector<string> lines;
  string line;
  while (getline(inFile, line)) {
    lines.emplace_back(line);
  }
  return lines;
}

unordered_map<string, string> fpdb::util::readConfig(const string &fileName) {
  unordered_map<string, string> configMap;
  string configPath = filesystem::current_path()
          .parent_path()
          .append("resources/config")
          .append(fileName)
          .string();
  vector<string> lines = readFileByLine(configPath);
  for(auto const &line: lines) {
    auto pos = line.find('=');
    if (pos == string::npos) {
      continue;
    }
    auto key = line.substr(0, pos);
    auto value = line.substr(pos + 1);
    configMap[key] = value;
  }
  return configMap;
}

vector<string> fpdb::util::readRemoteIps() {
  auto expLocalIp = getLocalIp();
  if (!expLocalIp) {
    throw runtime_error(expLocalIp.error());
  }
  const auto &localIp = *expLocalIp;

  // read cluster ips
  string clusterIpFilePath = filesystem::current_path()
          .parent_path()
          .append("resources/config/cluster_ips")
          .string();
  auto clusterIps = readFileByLine(clusterIpFilePath);

  // remove local ip
  auto localIpIt = std::find(clusterIps.begin(), clusterIps.end(), localIp);
  if (localIpIt != clusterIps.end()) {
    clusterIps.erase(localIpIt);
  }
  return clusterIps;
}

bool fpdb::util::parseBool(const string& stringToParse) {
  if (stringToParse == "TRUE") {
    return true;
  } else {
    return false;
  }
}

size_t fpdb::util::hashCombine(const vector<size_t> &hashes) {
  // reference: https://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector
  std::size_t seed = hashes.size();
  for (auto hash: hashes) {
    seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
  return seed;
}

bool fpdb::util::isInteger(const string& str) {
  try {
    int parsedInt = stoi(str);
  } catch (const logic_error& err) {
    return false;
  }
  return true;
}

tl::expected<string, string> fpdb::util::execCmd(const char *cmd) {
  char buffer[128];
  string result;
  FILE* pipe = popen(cmd, "r");
  if (!pipe) {
    return tl::make_unexpected("popen() failed!");
  }
  while (fgets(buffer, sizeof buffer, pipe) != nullptr) {
    result += buffer;
  }
  pclose(pipe);
  return result;
}

tl::expected<string, string> fpdb::util::getLocalIp() {
  return execCmd("curl -s ifconfig.me");
}
