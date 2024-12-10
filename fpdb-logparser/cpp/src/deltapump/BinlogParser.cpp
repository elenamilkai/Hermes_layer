#include <deltapump/BinlogParser.h>
#include <filesystem>
#include  <fpdb/logparser/LogParserClient.h>


using namespace std::filesystem;
using namespace fpdb::logparser;
using namespace normal::avro_tuple;
using namespace avro;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;
using std::chrono::steady_clock;
using std::chrono::high_resolution_clock;
BinlogParser::BinlogParser() {
}

std::shared_ptr<LogParserClient> logParserClient = std::make_shared<LogParserClient>();

avro::ValidSchema loadSchema(const char* filename) {
    std::ifstream ifs(filename);
    avro::ValidSchema result;
    avro::compileJsonSchema(ifs, result);
    return result;
}

long BinlogParser::parse(std::unordered_map<int, std::set<struct lineorder_record>> **lineorder_record_ptr,
                         std::unordered_map<int, std::set<struct customer_record>> **customer_record_ptr,
                         std::unordered_map<int, std::set<struct supplier_record>> **supplier_record_ptr,
                         std::unordered_map<int, std::set<struct part_record>> **part_record_ptr,
                         std::unordered_map<int, std::set<struct date_record>> **date_record_ptr,
                         std::unordered_map<int, std::set<struct history_record>> **history_record_ptr,
                         std::unordered_map<std::string , long> partitionInfo){
    auto startTime = std::chrono::steady_clock::now();
    std::vector<std::vector<int8_t>> binlogVector;
    logParserClient->startClient();
    binlogVector = logParserClient->parser();
    auto endTime = duration_cast<milliseconds>(steady_clock::now() - startTime).count();

    auto startTime1 = std::chrono::steady_clock::now();

    long timestamp = 0;
    for(uint8_t i : binlogVector[6])
    {
      timestamp <<= 8;
      timestamp |= i;
    }
    int lineorder_dim = binlogVector[0].size();
    int8_t * input_lineorder = &binlogVector[0][0];

    int customer_dim = binlogVector[1].size();
    int8_t * input_customer = &binlogVector[1][0];

    int supplier_dim = binlogVector[2].size();
    int8_t * input_supplier = &binlogVector[2][0];

    int part_dim = binlogVector[3].size();
    int8_t * input_part = &binlogVector[3][0];

    int date_dim = binlogVector[4].size();
    int8_t * input_date = &binlogVector[4][0];

    int history_dim = binlogVector[5].size();
    int8_t * input_history = &binlogVector[5][0];

    // put data into input streams
    std::unique_ptr<avro::InputStream> in_lineorder = avro::memoryInputStream(
            reinterpret_cast<const uint8_t *>(input_lineorder), lineorder_dim);

    std::unique_ptr<avro::InputStream> in_customer = avro::memoryInputStream(
            reinterpret_cast<const uint8_t *>(input_customer), customer_dim);

    std::unique_ptr<avro::InputStream> in_supplier = avro::memoryInputStream(
            reinterpret_cast<const uint8_t *>(input_supplier), supplier_dim);

    std::unique_ptr<avro::InputStream> in_part = avro::memoryInputStream(
            reinterpret_cast<const uint8_t *>(input_part), part_dim);

    std::unique_ptr<avro::InputStream> in_date = avro::memoryInputStream(
            reinterpret_cast<const uint8_t *>(input_date), date_dim);

    std::unique_ptr<avro::InputStream> in_history = avro::memoryInputStream(
            reinterpret_cast<const uint8_t *>(input_history), history_dim);


    // load schemas
    std::string workingDir = std::filesystem::current_path().parent_path().parent_path().string();
    avro::ValidSchema lineorderSchema = loadSchema((workingDir+"/fpdb-logparser/cpp/include/deltapump/schemas/delta/lineorder_d.json").c_str());
    avro::ValidSchema customerSchema = loadSchema((workingDir+"/fpdb-logparser/cpp/include/deltapump/schemas/delta/customer_d.json").c_str());
    avro::ValidSchema supplierSchema = loadSchema((workingDir+"/fpdb-logparser/cpp/include/deltapump/schemas/delta/supplier_d.json").c_str());
    avro::ValidSchema partSchema = loadSchema((workingDir+"/fpdb-logparser/cpp/include/deltapump/schemas/delta/part_d.json").c_str());
    avro::ValidSchema dateSchema = loadSchema((workingDir+"/fpdb-logparser/cpp/include/deltapump/schemas/delta/date_d.json").c_str());
    avro::ValidSchema historySchema = loadSchema((workingDir+"/fpdb-logparser/cpp/include/deltapump/schemas/delta/history_d.json").c_str());

    //maps of partitions
    auto *lineorder_record_map = new std::unordered_map<int, std::set<struct lineorder_record>>;
    auto *customer_record_map = new std::unordered_map<int, std::set<struct customer_record>>;
    auto *date_record_map = new std::unordered_map<int, std::set<struct date_record>>;
    auto *part_record_map = new std::unordered_map<int, std::set<struct part_record>>;
    auto *supplier_record_map = new std::unordered_map<int, std::set<struct supplier_record>>;
    auto *history_record_map = new std::unordered_map<int, std::set<struct history_record>>;

    // read the data input stream with the given valid schema
    avro::DataFileReader <i::lineorder> lineorderReader(move(in_lineorder), lineorderSchema);
    i::lineorder l1;

    avro::DataFileReader <i::customer> customerReader(move(in_customer), customerSchema);
    i::customer c1;

    avro::DataFileReader <i::date> dateReader(move(in_date), dateSchema);
    i::date d1;

    avro::DataFileReader <i::part> partReader(move(in_part), partSchema);
    i::part p1;

    avro::DataFileReader <i::supplier> supplierReader(move(in_supplier), supplierSchema);
    i::supplier s1;

    avro::DataFileReader <i::history> historyReader(move(in_history), historySchema);
    i::history h1;

    //get table_name, offset and range(fixed) of partitions for each table
    std::unordered_map<std::string, std::tuple<int, int>> range_result;

    // read the data input stream with the given valid schema
    while (lineorderReader.read(l1)) {
        //int key = (int) partitionInfo["lineorder"] - 1;
        int key;
        /**if(static_cast<int>((l1).lo_orderkey) <= 500250){
          key = 0;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 500250 &&  static_cast<int>((l1).lo_orderkey) <= 1000306){
          key = 1;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 1000306 &&  static_cast<int>((l1).lo_orderkey) <= 1499943){
          key = 2;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 1499943 &&  static_cast<int>((l1).lo_orderkey) <= 1999829){
          key = 3;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 1999829 &&  static_cast<int>((l1).lo_orderkey) <= 2499484){
          key = 4;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 2499484 &&  static_cast<int>((l1).lo_orderkey) <= 2999685){
          key = 5;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 2999685 &&  static_cast<int>((l1).lo_orderkey) <= 3499333){
          key = 6;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 3499333 &&  static_cast<int>((l1).lo_orderkey) <= 3998959){
          key = 7;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 3998959 &&  static_cast<int>((l1).lo_orderkey) <= 4499022){
          key = 8;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 4499022 &&  static_cast<int>((l1).lo_orderkey) <= 4998022){
          key = 9;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 4998022 &&  static_cast<int>((l1).lo_orderkey) <= 5498202){
          key = 10;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 5498202 &&  static_cast<int>((l1).lo_orderkey) <= 5998361){
          key = 11;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 5998361 &&  static_cast<int>((l1).lo_orderkey) <= 6498118){
          key = 12;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 6498118 &&  static_cast<int>((l1).lo_orderkey) <= 6998447){
          key = 13;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 6998447 &&  static_cast<int>((l1).lo_orderkey) <= 7497962){
          key = 14;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 7497962 &&  static_cast<int>((l1).lo_orderkey) <= 7998152){
          key = 15;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 7998152 &&  static_cast<int>((l1).lo_orderkey) <= 8498551){
          key = 16;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 8498551 &&  static_cast<int>((l1).lo_orderkey) <= 8998395){
          key = 17;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 8998395 &&  static_cast<int>((l1).lo_orderkey) <= 9498066){
          key = 18;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 9498066 &&  static_cast<int>((l1).lo_orderkey) <= 9998147){
          key = 19;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 9998147 &&  static_cast<int>((l1).lo_orderkey) <= 10498245){
          key = 20;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 10498245 &&  static_cast<int>((l1).lo_orderkey) <= 10998080){
          key = 21;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 10998080 &&  static_cast<int>((l1).lo_orderkey) <= 11498052){
          key = 22;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 11498052 &&  static_cast<int>((l1).lo_orderkey) <= 11998152){
          key = 23;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 11998152 &&  static_cast<int>((l1).lo_orderkey) <= 12498135){
          key = 24;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 12498135 &&  static_cast<int>((l1).lo_orderkey) <= 12997734){
          key = 25;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 12997734 &&  static_cast<int>((l1).lo_orderkey) <= 13498303){
          key = 26;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 13498303 &&  static_cast<int>((l1).lo_orderkey) <= 13998537){
          key = 27;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 13998537 &&  static_cast<int>((l1).lo_orderkey) <= 14498877){
          key = 28;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 14498877 &&  static_cast<int>((l1).lo_orderkey) <= 14998603){
          key = 29;
        }
        //else if (static_cast<int>((l1).lo_orderkey) > 14998603){
        //  key = 30;
        //}
        **/

        if(static_cast<int>((l1).lo_orderkey) <= 1499927){
          key = 0;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 1499927 &&  static_cast<int>((l1).lo_orderkey) <= 2999416){
          key = 1;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 2999416 &&  static_cast<int>((l1).lo_orderkey) <= 4498779){
          key = 2;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 4498779 &&  static_cast<int>((l1).lo_orderkey) <= 5998297){
          key = 3;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 5998297 &&  static_cast<int>((l1).lo_orderkey) <= 7497903){
          key = 4;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 7497903 &&  static_cast<int>((l1).lo_orderkey) <= 8998097){
          key = 5;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 8998097 &&  static_cast<int>((l1).lo_orderkey) <= 10498020){
          key = 6;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 10498020 &&  static_cast<int>((l1).lo_orderkey) <= 11998075){
          key = 7;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 11998075 &&  static_cast<int>((l1).lo_orderkey) <= 13498458){
          key = 8;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 13498458 &&  static_cast<int>((l1).lo_orderkey) <= 14998646){
          key = 9;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 14998646 &&  static_cast<int>((l1).lo_orderkey) <= 16498453){
          key = 10;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 16498453 &&  static_cast<int>((l1).lo_orderkey) <= 17998741){
          key = 11;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 17998741 &&  static_cast<int>((l1).lo_orderkey) <= 19497640){
          key = 12;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 19497640 &&  static_cast<int>((l1).lo_orderkey) <= 20997451){
          key = 13;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 20997451 &&  static_cast<int>((l1).lo_orderkey) <= 22498629){
          key = 14;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 22498629 &&  static_cast<int>((l1).lo_orderkey) <= 23997983){
          key = 15;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 23997983 &&  static_cast<int>((l1).lo_orderkey) <= 25498325){
          key = 16;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 25498325 &&  static_cast<int>((l1).lo_orderkey) <= 26997453){
          key = 17;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 26997453 &&  static_cast<int>((l1).lo_orderkey) <= 28497830){
          key = 18;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 28497830 &&  static_cast<int>((l1).lo_orderkey) <= 29997849){
          key = 19;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 29997849 &&  static_cast<int>((l1).lo_orderkey) <= 31498442){
          key = 20;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 31498442 &&  static_cast<int>((l1).lo_orderkey) <= 32999006){
          key = 21;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 32999006 &&  static_cast<int>((l1).lo_orderkey) <= 34499232){
          key = 22;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 34499232 &&  static_cast<int>((l1).lo_orderkey) <= 35999683){
          key = 23;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 35999683 &&  static_cast<int>((l1).lo_orderkey) <= 37498854){
          key = 24;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 37498854 &&  static_cast<int>((l1).lo_orderkey) <= 38999562){
          key = 25;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 38999562 &&  static_cast<int>((l1).lo_orderkey) <= 40499592){
          key = 26;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 40499592 &&  static_cast<int>((l1).lo_orderkey) <= 41999189){
          key = 27;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 41999189 &&  static_cast<int>((l1).lo_orderkey) <= 43499770){
          key = 28;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 43499770 &&  static_cast<int>((l1).lo_orderkey) <= 44999813){
          key = 29;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 44999813 &&  static_cast<int>((l1).lo_orderkey) <= 46499004){
          key = 30;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 46499004 &&  static_cast<int>((l1).lo_orderkey) <= 47997977){
          key = 31;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 47997977 &&  static_cast<int>((l1).lo_orderkey) <= 49498110){
          key = 32;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 49498110 &&  static_cast<int>((l1).lo_orderkey) <= 50997836){
          key = 33;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 50997836 &&  static_cast<int>((l1).lo_orderkey) <= 52497202){
          key = 34;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 52497202 &&  static_cast<int>((l1).lo_orderkey) <= 53997854){
          key = 35;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 53997854 &&  static_cast<int>((l1).lo_orderkey) <= 55497373){
          key = 36;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 55497373 &&  static_cast<int>((l1).lo_orderkey) <= 56997633){
          key = 37;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 56997633 &&  static_cast<int>((l1).lo_orderkey) <= 58497825){
          key = 38;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 58497825 &&  static_cast<int>((l1).lo_orderkey) <= 59997045){
          key = 39;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 59997045 &&  static_cast<int>((l1).lo_orderkey) <= 61496667){
          key = 40;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 61496667 &&  static_cast<int>((l1).lo_orderkey) <= 62996883){
          key = 41;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 62996883 &&  static_cast<int>((l1).lo_orderkey) <= 64496661){
          key = 42;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 64496661 &&  static_cast<int>((l1).lo_orderkey) <= 65996687){
          key = 43;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 65996687 &&  static_cast<int>((l1).lo_orderkey) <= 67497709){
          key = 44;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 67497709 &&  static_cast<int>((l1).lo_orderkey) <= 68997780){
          key = 45;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 68997780 &&  static_cast<int>((l1).lo_orderkey) <= 70497949){
          key = 46;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 70497949 &&  static_cast<int>((l1).lo_orderkey) <= 71998259){
          key = 47;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 71998259 &&  static_cast<int>((l1).lo_orderkey) <= 73498852){
          key = 48;
        }
        else if (static_cast<int>((l1).lo_orderkey) > 73498852 &&  static_cast<int>((l1).lo_orderkey) <= 74999217){
          key = 49;
        }
        //else if (static_cast<int>((l1).lo_orderkey) > 74999217 &&  static_cast<int>((l1).lo_orderkey) <= 75000000){
        //  key = 50;
        //}
        //insert record into a partition (create a new partition if not exist)
        lineorder_record r = {static_cast<int>((l1).lo_orderkey), static_cast<int>((l1).lo_linenumber), MakeTuple::makeLineorderDeltaTuple(l1)} ;
        auto it = (*lineorder_record_map).find(key);
        if(it == (*lineorder_record_map).end()){
            std::set<struct lineorder_record> new_set;
            new_set.insert(r);
            (*lineorder_record_map).insert(std::make_pair(key, new_set));
        }
        else{
            (it->second).insert(r);
        }
    }

    while (customerReader.read(c1)) {
        int key = 0;
        customer_record r = {static_cast<int>((c1).c_custkey), MakeTuple::makeCustomerDeltaTuple(c1)} ;
        auto it = (*customer_record_map).find(key);
        if(it == (*customer_record_map).end()){
            std::set<struct customer_record> new_set;
            new_set.insert(r);
            (*customer_record_map).insert(std::make_pair(key, new_set));
        }
        else{
            (it->second).insert(r);
        }
    }

    while (dateReader.read(d1)) {
        int key = 0;
        date_record r = {static_cast<int>((d1).d_datekey), MakeTuple::makeDateDeltaTuple(d1)} ;
        auto it = (*date_record_map).find(key);
        if(it == (*date_record_map).end()){
            std::set<struct date_record> new_set;
            new_set.insert(r);
            (*date_record_map).insert(std::make_pair(key, new_set));
        }
        else{
            (it->second).insert(r);
        }
    }

    while (partReader.read(p1)) {
        int key = 0;
        part_record r = {static_cast<int>((p1).p_partkey), MakeTuple::makePartDeltaTuple(p1)} ;
        auto it = (*part_record_map).find(key);
        if(it == (*part_record_map).end()){
            std::set<struct part_record> new_set;
            new_set.insert(r);
            (*part_record_map).insert(std::make_pair(key, new_set));
        }
        else{
            (it->second).insert(r);
        }
    }

    while (supplierReader.read(s1)) {
        int key = 0;
        supplier_record r = {static_cast<int>((s1).s_suppkey), MakeTuple::makeSupplierDeltaTuple(s1)} ;
        auto it = (*supplier_record_map).find(key);
        if(it == (*supplier_record_map).end()){
            std::set<struct supplier_record> new_set;
            new_set.insert(r);
            (*supplier_record_map).insert(std::make_pair(key, new_set));
        }
        else{
            (it->second).insert(r);
        }
    }

    while (historyReader.read(h1)) {
        int key = 0;
        history_record r = {static_cast<int>((h1).h_orderkey), MakeTuple::makeHistoryDeltaTuple(h1)} ;
        auto it = (*history_record_map).find(key);
        if(it == (*history_record_map).end()){
            std::set<struct history_record> new_set;
            new_set.insert(r);
            (*history_record_map).insert(std::make_pair(key, new_set));
        }
        else{
            (it->second).insert(r);
        }
    }
    //Assign pointers
    *lineorder_record_ptr = lineorder_record_map;
    *customer_record_ptr = customer_record_map;
    *supplier_record_ptr = supplier_record_map;
    *part_record_ptr = part_record_map;
    *date_record_ptr = date_record_map;
    *history_record_ptr = history_record_map;
    auto endTime1 = duration_cast<milliseconds>(steady_clock::now() - startTime1).count();
    return timestamp;
}
