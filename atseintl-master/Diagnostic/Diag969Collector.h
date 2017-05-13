//----------------------------------------------------------------------------
//  File:        Diag969Collector.h
//  Created:     2007-12-12
//
//  Description: Diagnostic 969 formatter
//
//  Updates:
//
//  Copyright Sabre 2007
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

#include "DataModel/FlightFinderTrx.h"
#include "Diagnostic/DiagCollector.h"

#include <string>

namespace tse
{
class Diag969Collector : public DiagCollector
{
public:
  explicit Diag969Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag969Collector() {}

  virtual Diag969Collector& operator<<(FlightFinderTrx& flightFinderTrx);
  std::string
  getSOPInfo(FlightFinderTrx& fFTrx, const uint16_t& legID, FlightFinderTrx::SopInfo* sopInfo);
  PaxTypeFare* getFrontPaxTypeFare(const FlightFinderTrx::FlightDataInfo& flightInfo);
  bool showSOPs(FlightFinderTrx& flightFinderTrx, bool outbound);
  bool showFareBasisCode(FlightFinderTrx& flightFinderTrx, bool outbound);
  std::string getAvailability(const FlightFinderTrx::SopInfo* sopInfo, const size_t count);
};

} // namespace tse

