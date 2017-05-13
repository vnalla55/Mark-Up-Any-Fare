//----------------------------------------------------------------------------
//  File:        Diag910Collector.h
//  Created:     2006-04-27
//
//  Description: Diagnostic 931: Shopping Group Itinerary Display
//
//  Updates:
//
//  Copyright Sabre 2004
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

#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/DiagCollector.h"

#include <iosfwd>
#include <map>

namespace tse
{
class Itin;
class ShoppingTrx;

class Diag931Collector : public DiagCollector
{
public:
  void outputItin(const SopIdVec& sops,
                  int16_t carrierGroupIndex,
                  const ShoppingTrx& shoppingTrx,
                  int16_t itemNo,
                  bool interItin,
                  const std::string mapKey);
  void outputHeader(const ShoppingTrx& trx);
};
}

