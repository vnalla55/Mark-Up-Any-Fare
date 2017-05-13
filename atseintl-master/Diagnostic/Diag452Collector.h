//-----------------------------------------------------------------------------
//
//  File:     Diag452Collector.h
//
//  Author :  Slawek Machowicz
//
//  Copyright Sabre 2009
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#pragma once

#include "Diagnostic/RoutingDiagCollector.h"
#include "Routing/SouthAtlanticTPMExclusion.h"

#include <string>

namespace tse
{
class TPMExclusion;

class Diag452Collector : public RoutingDiagCollector
{
public:
  friend class Diag452CollectorTest;

  explicit Diag452Collector(Diagnostic& root)
    : RoutingDiagCollector(root), _matched(false), _separatorNeeded(true), _ticketingDT(0)
  {
  }
  Diag452Collector() : _matched(false), _separatorNeeded(true), _ticketingDT(0) {}

  void printHeader() override;
  void printFooter();
  Diag452Collector& operator<<(const TPMExclusion& tpmExclusion);
  Diag452Collector& operator<<(SouthAtlanticTPMExclusion::TPMExclusionFailCode failCode);
  void printFareMarketHeader(const MileageRoute& mRoute);

private:
  void displayMileageRouteItems(const MileageRouteItems& mrItems);
  void printLineTitle(SouthAtlanticTPMExclusion::TPMExclusionFailCode failCode);
  void printDirectionality(Directionality dir);
  void printDates(const TPMExclusion& tpmExclusion);
  void printCrsMultihost(const TPMExclusion& tpmExclusion);
  void decodeLoc(LocTypeCode locType, const LocCode& locCode, int& pos);
  void decodeLoc(LocTypeCode locType, const LocCode& locCode);
  void printZone(const ZoneInfo& zone, int& pos);
  void printTextWithNewLine(const std::string& text, int& pos);

  virtual const ZoneInfo* getZoneInfo(const LocCode& locCode);

  // Data
private:
  bool _matched;
  bool _separatorNeeded;
  std::string _gd;
  DateTime _ticketingDT;
};
} // namespace tse

