//----------------------------------------------------------------------------
//  File:        Diag898Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 898 - MMGR S8 Branded Fares programs, fares, services
//
//  Updates:
//          date - initials - description.
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
//----------------------------------------------------------------------------

#pragma once

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class RuleExecution;
class MarketRule;
class MarketCriteria;
class S8BrandingSecurity;
struct PatternInfo;

class Diag898Collector : public DiagCollector
{
  friend class Diag898CollectorTest;

public:
  explicit Diag898Collector(Diagnostic& root) : DiagCollector(root), _trx(nullptr) {}
  Diag898Collector() : _trx(nullptr) {}

  void printDiagnostic(const std::map<int, std::vector<FareMarket*> >& marketIDFareMarketMap);
  void initTrx(PricingTrx& trx) { _trx = &trx; }
  void printBrandedDataError() const;

private:
  void processMarketResponse(const MarketResponse* marketResponse,
                             const std::map<int, std::vector<FareMarket*> >& marketIDFareMarketMap,
                             bool& hasData);
  void printS8FareMarket(const MarketResponse* marketResponse);
  void
  printRuleExecutionContent(const RuleExecution* ruleExecution,
                            const MarketResponse* marketResponse,
                            const std::map<int, std::vector<FareMarket*> >& marketIDFareMarketMap);
  void printDetailRuleExecutionContent(
      const RuleExecution* ruleExecution,
      const MarketResponse* marketResponse,
      const std::map<int, std::vector<FareMarket*> >& marketIDFareMarketMap);

  void displayVendor(const VendorCode& vendor, bool isDetailDisp = false);
  void getStatusStr(StatusS8 status, std::string& statusStr) const;
  bool isDdInfo() const;
  void printS8BrandingSecurity(const S8BrandingSecurity* s8BrandingSecurity);
  void printMarketRule(const MarketRule* marketRule, bool& firstMarket);
  void printS8CommonHeader();
  void printS8DetailHeader();
  void printSeparator();
  bool shouldDisplay(const std::vector<PatternInfo*>& cxrVec) const;
  bool shouldDisplay(const ProgramCode& programCode) const;
  bool matchFareMarket(const MarketResponse* marketResponse) const;
  void printBlankLine() const;
  bool isDDPass() const;
  bool shouldDisplay(const StatusS8& status) const;
  void printDataNotFound() const;
  bool
  matchGlobalDirection(const MarketCriteria* marketCriteria, const MarketRule* marketRule) const;
  bool matchMarket(const MarketCriteria* marketCriteria, const MarketRule* marketRule) const;
  bool matchAirport(const MarketCriteria* marketCriteria, const MarketRule* marketRule) const;

  bool isLocInBetween(const LocCode& originLoc,
                      const LocCode& destinationLoc,
                      const LocTypeCode& originLocType,
                      const LocTypeCode& destinationLocType,
                      const Loc& origin,
                      const Loc& destination,
                      const VendorCode& vendor,
                      const GeoTravelType geoTvlType,
                      const CarrierCode& carrier) const;

  bool isZoneInBetween(const LocCode& originLoc,
                       const LocCode& destinationLoc,
                       const LocTypeCode& originLocType,
                       const LocTypeCode& destinationLocType,
                       const Loc& origin,
                       const Loc& destination,
                       const VendorCode& vendor,
                       const GeoTravelType geoTvlType,
                       const CarrierCode& carrier) const;

  bool matchLocation(const MarketCriteria* marketCriteria,
                     const MarketRule* marketRule,
                     const Loc* origin,
                     const Loc* destination,
                     const GeoTravelType geoTvlType) const;

  void setStatusForMarketFailure(const MarketCriteria* marketCriteria,
                                 const MarketRule* marketRule,
                                 std::string& statusStr);
  void setStatusPassSeclected(const RuleExecution* ruleExecution,
                              const MarketResponse* marketResponse,
                              std::string& statusStr);
  void setStatusStr(const RuleExecution* ruleExecution,
                    const MarketResponse* marketResponse,
                    std::string& statusStr);

  bool matchGeo(const MarketCriteria* marketCriteria, const MarketRule* marketRule) const;

  bool isGeoLocInBetween(const LocCode& originLoc,
                         const LocCode& destinationLoc,
                         const LocTypeCode& originLocType,
                         const LocTypeCode& destinationLocType,
                         const Loc& origin,
                         const Loc& destination,
                         const VendorCode& vendor,
                         const GeoTravelType geoTvlType,
                         const CarrierCode& carrier) const;

  bool isGeoZoneInBetween(const LocCode& originLoc,
                          const LocCode& destinationLoc,
                          const LocTypeCode& originLocType,
                          const LocTypeCode& destinationLocType,
                          const Loc& origin,
                          const Loc& destination,
                          const VendorCode& vendor,
                          const GeoTravelType geoTvlType,
                          const CarrierCode& carrier) const;

  bool matchGeoLocation(const MarketCriteria* marketCriteria,
                        const MarketRule* marketRule,
                        const Loc* origin,
                        const Loc* destination,
                        const GeoTravelType geoTvlType) const;

  bool matchGeoAirport(const MarketCriteria* marketCriteria, const MarketRule* marketRule) const;

  PricingTrx* _trx;
};
} // namespace tse

