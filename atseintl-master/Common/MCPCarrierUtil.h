//----------------------------------------------------------------------------
//
//  Copyright Sabre 2010
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

#include "Common/Config/ConfigMan.h"
#include "Common/TseCodeTypes.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace tse
{
class Trx;

class MCPCarrierUtil
{
  friend class MCPCarrierUtilTest;

private:
  static std::vector<std::pair<CarrierCode, CarrierCode> > _mcpCxrMap;
  static std::vector<CarrierCode> _mcpHosts;
  static std::multimap<CarrierCode, CarrierCode> _preferedCarriers;
  static std::vector<CarrierCode> _iapRestricted;
  static std::vector<CarrierCode> _journeyByMarriageCarriers;
  static std::vector<std::pair<CarrierCode, CarrierCode> > _mcpNeutralCxrMap;
  MCPCarrierUtil();

protected:
  static void getConfig(tse::ConfigMan& config, const std::string& argName, std::string& retValue);
  static bool getCxrPair(const std::string& cfgPair, CarrierCode& cxr1, CarrierCode& cxr2);
  static bool getPreferedCarrier(const std::string& cfgPrefCarrier,
                                 CarrierCode& hostCarrier,
                                 std::vector<CarrierCode>& carriers);
  static bool
  getPreferedCarrier(const std::string& cfgPrefCarrier, std::vector<CarrierCode>& carriers);

  template <typename T>
  static void tokenize(const std::string& input, const bool checkCarrier, std::vector<T>& output);

public:
  static void initialize(tse::ConfigMan& config);

  static CarrierCode swapToActual(const Trx* trx, const CarrierCode& cxr);
  static CarrierCode swapToPseudo(const Trx* trx, const CarrierCode& cxr);
  static CarrierCode swapFromNeutralToActual(const CarrierCode& cxr);

  static bool isPseudoCarrier(const std::string& carrier);
  static bool isMcpHost(const std::string& host);
  static bool isJourneyByMarriageCarrier(const CarrierCode& cxr);
  static bool getPreferedCarriers(const CarrierCode& hostCarrier, std::set<CarrierCode>& carriers);
  static bool isIAPCarrierRestricted(const CarrierCode& cxr);
  static bool isSameGroupCarriers(const CarrierCode& carrier1, const CarrierCode& carrier2);
  static bool isNeutralCarrier(const std::string& carrier);
};
}
