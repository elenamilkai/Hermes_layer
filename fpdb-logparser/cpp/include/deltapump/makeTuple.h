#ifndef NORMAL_MAKETUPLE_H
#define NORMAL_MAKETUPLE_H

#include <tuple>
#include <string>

#include "lineorder_d.hh"
#include "customer_d.hh"
#include "supplier_d.hh"
#include "part_d.hh"
#include "date_d.hh"
#include "history_d.hh"

namespace normal::avro_tuple::make {

    typedef std::tuple<
            int64_t,
            int64_t,
            int64_t,
            int64_t,
            int64_t,
            int64_t,
            std::string,
            std::string,
            int64_t,
            int64_t,
            int64_t,
            int64_t,
            int64_t,
            int64_t,
            int64_t,
            std::string,
            std::string> LineorderDelta_t;


    typedef std::tuple<
            int64_t,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            int64_t,
            std::string> CustomerDelta_t;


    typedef std::tuple<
            int64_t,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            int64_t,
            std::string> SupplierDelta_t;

    typedef std::tuple<
            int64_t,
            std::string,
            std::string,
            std::string,
            int64_t,
            int64_t,
            std::string,
            int64_t,
            int64_t,
            int64_t,
            int64_t,
            int64_t,
            std::string,
            bool,
            bool,
            bool,
            bool,
            std::string> DateDelta_t;

    typedef std::tuple<
            int64_t,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            std::string,
            int64_t,
            std::string,
            int64_t,
            std::string> PartDelta_t;

    typedef std::tuple<
            int64_t,
            int64_t,
            int64_t,
            std::string> HistoryDelta_t;

    class MakeTuple {
    public:
        static LineorderDelta_t makeLineorderDeltaTuple(i::lineorder& linorderDeltaStruct);
        static CustomerDelta_t makeCustomerDeltaTuple(i::customer& customerDeltaStruct);
        static SupplierDelta_t makeSupplierDeltaTuple(i::supplier& supplierDeltaStruct);
        static PartDelta_t makePartDeltaTuple(i::part& partDeltaStruct);
        static DateDelta_t makeDateDeltaTuple(i::date& dateDeltaStruct);
        static HistoryDelta_t makeHistoryDeltaTuple(i::history& historyDeltaStruct);
    };
}

#endif //NORMAL_MAKETUPLE_H