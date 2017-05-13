//----------------------------------------------------------------------------
//  File:        Diag981Collector.h
//  Created:     2008-08-20
//    Authors:     Marek Sliwa
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

#include "Diagnostic/DiagCollector.h"

#include <iosfwd>
#include <map>

namespace tse
{
class Diag981Collector : public DiagCollector
{
public:
  Diag981Collector() : adrelfares(false) {}

  Diag981Collector& operator<<(const FareMarket& fm) override;
  Diag981Collector& operator<<(const MergedFareMarket& fm) override;
  Diag981Collector& operator<<(const PaxTypeFare& paxTypeFare) override;
  void showRemFaresForDatePair(std::map<PaxTypeFare*, PaxTypeFare*>& ptFareMap);
  void setAdrelfares(bool adRelfares) { adrelfares = adRelfares; }

private:
  bool adrelfares;
};
}
