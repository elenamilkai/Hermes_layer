#include <deltapump/makeTuple.h>

using namespace normal::avro_tuple::make;

LineorderDelta_t MakeTuple::makeLineorderDeltaTuple(i::lineorder& linorderDeltaStruct) {
    return std::make_tuple(
            linorderDeltaStruct.lo_orderkey,
            linorderDeltaStruct.lo_linenumber,
            linorderDeltaStruct.lo_custkey,
            linorderDeltaStruct.lo_partkey,
            linorderDeltaStruct.lo_suppkey,
            linorderDeltaStruct.lo_orderdate,
            linorderDeltaStruct.lo_orderpriority,
            linorderDeltaStruct.lo_shippriority,
            linorderDeltaStruct.lo_quantity,
            linorderDeltaStruct.lo_extendedprice,
            linorderDeltaStruct.lo_discount,
            linorderDeltaStruct.lo_revenue,
            linorderDeltaStruct.lo_supplycost,
            linorderDeltaStruct.lo_tax,
            linorderDeltaStruct.lo_commitdate,
            linorderDeltaStruct.lo_shipmode,
            linorderDeltaStruct.type);
}

CustomerDelta_t MakeTuple::makeCustomerDeltaTuple(i::customer &customerDeltaStruct) {
    return std::make_tuple(customerDeltaStruct.c_custkey,
                           customerDeltaStruct.c_name,
                           customerDeltaStruct.c_address,
                           customerDeltaStruct.c_city,
                           customerDeltaStruct.c_nation,
                           customerDeltaStruct.c_region,
                           customerDeltaStruct.c_phone,
                           customerDeltaStruct.c_mktsegment,
                           customerDeltaStruct.c_paymentcnt,
                           customerDeltaStruct.type);
}

SupplierDelta_t MakeTuple::makeSupplierDeltaTuple(i::supplier &supplierDeltaStruct) {
    return std::make_tuple(supplierDeltaStruct.s_suppkey,
                           supplierDeltaStruct.s_name,
                           supplierDeltaStruct.s_address,
                           supplierDeltaStruct.s_city,
                           supplierDeltaStruct.s_nation,
                           supplierDeltaStruct.s_region,
                           supplierDeltaStruct.s_phone,
                           supplierDeltaStruct.s_ytd,
                           supplierDeltaStruct.type);
}

PartDelta_t MakeTuple::makePartDeltaTuple(i::part &partDeltaStruct) {
    return std::make_tuple(partDeltaStruct.p_partkey,
                           partDeltaStruct.p_name,
                           partDeltaStruct.p_mfgr,
                           partDeltaStruct.p_category,
                           partDeltaStruct.p_brand1,
                           partDeltaStruct.p_color,
                           partDeltaStruct.p_type,
                           partDeltaStruct.p_size,
                           partDeltaStruct.p_container,
                           partDeltaStruct.p_price,
                           partDeltaStruct.type);
}

DateDelta_t MakeTuple::makeDateDeltaTuple(i::date &dateDeltaStruct) {
    return std::make_tuple(dateDeltaStruct.d_datekey,
                           dateDeltaStruct.d_date,
                           dateDeltaStruct.d_dayofweek,
                           dateDeltaStruct.d_month,
                           dateDeltaStruct.d_year,
                           dateDeltaStruct.d_yearmonthnum,
                           dateDeltaStruct.d_yearmonth,
                           dateDeltaStruct.d_daynuminweek,
                           dateDeltaStruct.d_daynuminmonth,
                           dateDeltaStruct.d_daynuminyear,
                           dateDeltaStruct.d_monthnuminyear,
                           dateDeltaStruct.d_weeknuminyear,
                           dateDeltaStruct.d_sellingseason,
                           dateDeltaStruct.d_lastdayinweekfl,
                           dateDeltaStruct.d_lastdayinmonthfl,
                           dateDeltaStruct.d_holidayfl,
                           dateDeltaStruct.d_weekdayfl,
                           dateDeltaStruct.type);
}

HistoryDelta_t MakeTuple::makeHistoryDeltaTuple(i::history& historyDeltaStruct) {
    return std::make_tuple(historyDeltaStruct.h_orderkey,
                           historyDeltaStruct.h_custkey,
                           historyDeltaStruct.h_amount,
                           historyDeltaStruct.type);
}
