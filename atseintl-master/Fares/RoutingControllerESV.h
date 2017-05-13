//----------------------------------------------------------------------------
//  File:        RoutingControllerESV.h
//  Created:     2008-07-20
//
//  Description: ESV routing controller
//
//  Updates:
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "DataModel/ShoppingTrx.h"
#include "Server/TseServer.h"

namespace tse
{
class Itin;

class RoutingControllerESV final
{
public:
  RoutingControllerESV(ShoppingTrx& trx) : _trx(trx) {}

  bool validateRouting(Itin* itin, PaxTypeFare* paxTypeFare);

private:
  ShoppingTrx& _trx;
};
} // End namespace tse
