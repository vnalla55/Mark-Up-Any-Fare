//-------------------------------------------------------------------
//
//  Authors:     Cesar Villarreal
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "DataModel/SettlementTypesTrx.h"

#include "Service/Service.h"

namespace tse
{
  bool
  SettlementTypesTrx::process(Service& srv)
  {
    return srv.process(*this);
  }
} // namespace tse