//----------------------------------------------------------------------------
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

#include "Common/TseCodeTypes.h"

namespace tse
{

class FareByRuleApp;
class PricingTrx;
class FareMarket;
class Loc;
class FareByRuleProcessingInfo;
class TariffCrossRefInfo;
class DiagManager;
class MarkupSecFilter;
class Agent;

class IsTestRuleTariff
{
public:
  bool operator()(TariffCrossRefInfo* tcrInfo);
};

/** @class FareByRuleAppValidator
 *  This class validates Record 8 against the passenger/itinerary information.
 *  Record 8 is called Fare By Rule Application. More than one Record 8 may be
 *  found that relates to the application market and passenger. A positive match
 *  to Record 8 must be made in order to continue and select Record 2s for that
 *  tariff/rule.
 */

class FareByRuleAppValidator final
{
public:
  explicit FareByRuleAppValidator(DiagManager* diagMan = nullptr) : _diagManager(diagMan) {}

  friend class FareByRuleAppValidatorTest;

  bool
  isValid(FareByRuleProcessingInfo& fbrProcessingInfo, std::map<std::string, bool>& ruleTariffMap);

  static const char FROM_LOC_1;
  static const char TO_LOC_1;
  static const char TEST_RULE;
  static const char TEST_RULE_1;
  static const char TEST_RULE_S;

private:
  bool matchTariffXRef(FareByRuleProcessingInfo& fbrProcessingInfo,
                       std::map<std::string, bool>& ruleTariffMap) const;
  bool matchPassengerStatus(FareByRuleApp& fbrApp,
                            PricingTrx& trx,
                            FareByRuleProcessingInfo& fbrProcessingInfo) const;
  bool matchPaxLoc(const FareByRuleApp& fbrApp,
                   const std::string paxNationOrState,
                   PricingTrx& trx) const;
  bool matchLocation(PricingTrx& trx,
                     FareByRuleApp& fbrApp,
                     FareMarket& fareMarket,
                     FareByRuleProcessingInfo& fbrProcessingInfo) const;
  bool matchGeo(PricingTrx& trx,
                FareByRuleApp& fbrApp,
                const Loc& loc1,
                const Loc& loc2,
                GeoTravelType geoTvlType) const;
  bool matchWhollyWithin(FareByRuleApp& fbrApp, FareMarket& fareMarket) const;
  bool checkUpFrontSecurity(FareByRuleApp& fbrApp,
                            PricingTrx& trx,
                            FareMarket& fareMarket,
                            TariffCrossRefInfo& tcrInfo,
                            bool isFdTrx,
                            FareByRuleProcessingInfo& fbrProcessingInfo) const;
  void doDiag208Collector(FareByRuleApp& fbrApp,
                          PricingTrx& trx,
                          const char* failCode,
                          const FareMarket& fareMarket) const;
  bool isUsCaRuleTariff(FareByRuleApp& fbrApp) const;
  bool validateMarkUpSecFilter(const std::vector<MarkupSecFilter*>& lstMSF,
                               FareByRuleProcessingInfo& fbrProcessingInfo) const;
  bool validateMarkUpSecFilterEnhanced(const std::vector<MarkupSecFilter*>& lstMSF,
                                       FareByRuleProcessingInfo& fbrProcessingInfo) const;
  bool isMatchCarrierCrs(CarrierCode& carrierCrs, PricingTrx& trx, const Agent* agent) const;
  bool isVendorPCC(const VendorCode& vendor, PricingTrx& trx) const;

  DiagManager* _diagManager;
};
}
