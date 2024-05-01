//
// Created by Elena Milkai on 25/4/22.
//

#ifndef __INCLUDE_HISTORY_D_HH
#define __INCLUDE_HISTORY_D_HH

#include <sstream>
#include "boost/any.hpp"
#include "avro/Specific.hh"
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"

namespace i {
    struct history {
        int64_t h_orderkey;
        int64_t h_custkey;
        int64_t h_amount;
        std::string type;
        history() :
                h_orderkey(int64_t()),
                h_custkey(int64_t()),
                h_amount(int64_t()),
                type(std::string())
        { }
    };

}
namespace avro {
    template<> struct codec_traits<i::history> {
        static void encode(Encoder& e, const i::history& v) {
            avro::encode(e, v.h_orderkey);
            avro::encode(e, v.h_custkey);
            avro::encode(e, v.h_amount);
            avro::encode(e, v.type);
        }
        static void decode(Decoder& d, i::history& v) {
            if (avro::ResolvingDecoder *rd =
                    dynamic_cast<avro::ResolvingDecoder *>(&d)) {
                const std::vector<size_t> fo = rd->fieldOrder();
                for (std::vector<size_t>::const_iterator it = fo.begin();
                     it != fo.end(); ++it) {
                    switch (*it) {
                        case 0:
                            avro::decode(d, v.h_orderkey);
                            break;
                        case 1:
                            avro::decode(d, v.h_custkey);
                            break;
                        case 2:
                            avro::decode(d, v.h_amount);
                            break;
                        case 3:
                            avro::decode(d, v.type);
                            break;
                        default:
                            break;
                    }
                }
            } else {
                avro::decode(d, v.h_orderkey);
                avro::decode(d, v.h_custkey);
                avro::decode(d, v.h_amount);
                avro::decode(d, v.type);
            }
        }
    };

}
#endif //__INCLUDE_HISTORY_D_HH