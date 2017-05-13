//-------------------------------------------------------------------
//
//  File:        FareDisplayResponseRoutingXMLTags.h
//  Description: Populates Routing XML tags for the FareDisplay SDS response.
//
//  Copyright Sabre 2006
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#pragma once

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/XMLConstruct.h"

#include <sstream>
#include <vector>

namespace tse
{
class FareDisplayTrx;
class PaxTypeFare;
class Routing;
class RoutingInfo;
class RoutingRestriction;

class FareDisplayResponseRoutingXMLTags final
{
public:
  void buildTags(FareDisplayTrx& trx, XMLConstruct& construct);

private:
  std::ostringstream oss;

  void addMPM(RoutingInfo* rInfo, XMLConstruct& construct);

  typedef std::vector<RoutingRestriction*> RoutingRestrictions;
  typedef RoutingRestrictions::const_iterator RoutingRestrictionsConstIter;
  typedef std::vector<std::string> RoutingMapStrings;
  typedef std::vector<const Routing*> Routings;
  typedef Routings::const_iterator RoutingsConstIter;

  bool buildFDDisplay(tse::FareDisplayTrx& trx, XMLConstruct& construct);
  bool buildADDisplay(tse::FareDisplayTrx& trx, XMLConstruct& construct);
  bool addRouteString(FareDisplayTrx& trx,
                      const RoutingMapStrings* strings,
                      bool indent,
                      bool useLineNumbers,
                      bool fdDisplay);
  bool addGlobalDescription(FareDisplayTrx& trx, GlobalDirection global, bool inLine);
  bool addMPMvsRTG(FareDisplayTrx&, const RoutingNumber&, const RoutingInfo*, bool);

  void addRestrictions(FareDisplayTrx&, const RoutingRestrictions&, bool, bool);
  void
  splitLine(FareDisplayTrx& trx, const std::string& theLine, bool inLine, uint32_t lineNum, bool);
  void displayPSR(FareDisplayTrx& trx, const RoutingInfo& rtgInfo);
  void displayTPD(FareDisplayTrx& trx, const RoutingInfo& rtgInfo);
  bool displayDRV(FareDisplayTrx&, const Routing*, const PaxTypeFare&, bool);
  void displayDRV(FareDisplayTrx&, const Routings&, const PaxTypeFare&, bool);
  void displayConstrVsPubl(FareDisplayTrx&, const PaxTypeFare&);
  void displayConstructed(FareDisplayTrx&, const PaxTypeFare&, const RoutingInfo*, bool);
  void displayConstrMsg(FareDisplayTrx&, const std::string&, const std::string&, bool);
  bool isNonstop(const FareDisplayTrx&, const RoutingMapStrings*);
  void displayNonstop(FareDisplayTrx&, bool);
  std::string translateNation(const NationCode&);
  std::string gateway(const std::string&);
};
}

