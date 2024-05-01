//
// Created by Yifei Yang on 1/14/22.
//

#ifndef FPDB_FPDB_MAIN_INCLUDE_FPDB_MAIN_CAFINIT_H
#define FPDB_FPDB_MAIN_INCLUDE_FPDB_MAIN_CAFINIT_H

#include <caf/all.hpp>
#include <caf/io/all.hpp>

namespace fpdb::main {

class CAFInit {

public:
  /**
   * Required before creating actor_system
   */
  static void initCAFGlobalMetaObjects();

};

}


#endif //FPDB_FPDB_MAIN_INCLUDE_FPDB_MAIN_CAFINIT_H
