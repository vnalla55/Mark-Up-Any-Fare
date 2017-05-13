//----------------------------------------------------------------
//
//  File:	SecSurchargeAppl.h
//  Authors:
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
//-----------------------------------------------------------------
#pragma once

#include "Common/TseEnums.h"
#include "Rules/RuleApplicationBase.h"
#include "Rules/RuleUtil.h"


namespace tse
{

class PricingTrx;
class FareUsage;
class SectorSurcharge;
class SurchargeData;
class TravelSeg;
class DCFactory;

class SecSurchargeAppl
{
  friend class SecSurchargeApplTest;
  friend class Diag512Collector;

public:
  typedef std::vector<SectorSurcharge*> SecSurchRuleList;
  typedef std::map<CarrierCode, const SecSurchRuleList*> SecSurchRuleMap;

  void process(PricingTrx& trx,
               const CarrierCode& validatingCxr,
               FareUsage& fareUsage,
               SecSurchRuleMap& ssrMap);

  static constexpr Indicator NOT_APPLY = ' ';

private:
  Record3ReturnTypes validate(PricingTrx& trx,
                              const CarrierCode& validatingCxr,
                              const SectorSurcharge& surchInfo,
                              const TravelSeg& tvlSeg,
                              FareUsage& fareUsage);

  bool
  matchExclPsgType(PricingTrx& trx, const FareUsage& fareUsage, const SectorSurcharge& surchInfo);

  bool matchLocations(PricingTrx& trx,
                      const TravelSeg& tvlSeg,
                      const SectorSurcharge& surchInfo,
                      const FareUsage& fareUsage);

  bool matchDirection(PricingTrx& trx,
                      const TravelSeg& tvlSeg,
                      const FareUsage& fareUsage,
                      const SectorSurcharge& surchInfo);

  bool matchTvlDate(const TravelSeg& tvlSeg, const SectorSurcharge& surchInfo);

  bool matchDOWandTime(const TravelSeg& tvlSeg, const SectorSurcharge& surchInfo);

  void calcSurcharge(PricingTrx& trx,
                     const FareUsage& fareUsage,
                     const SectorSurcharge& surchInfo,
                     SurchargeData& surchData,
                     const TravelSeg& tvlSeg);

  Record3ReturnTypes diagAndRtn(const Record3ReturnTypes result,
                                DiagCollector& diag,
                                DCFactory* factory,
                                const std::string& diagMsg);

  bool findCat12Surcharge(const FareUsage& fareUsage,
                          const TravelSeg& tvlSeg,
                          const SurchargeData& surchData);

  void displayRuleToDiag(const SectorSurcharge& surchInfo, DiagCollector& diag);

  inline bool isValidTOD(int16_t tod) { return (tod != 0); }

  static const std::string CRS_USER_UNMATCH;
  static const std::string MULTIHOST_USER_UNMATCH;
  static const std::string SALES_LOC_UNMATCH;
  static const std::string TICKET_LOC_UNMATCH;
  static const std::string PAX_TYPE_EXCEPT;
  static const std::string SURCHARGE_TYPE;
  static const std::string TRAVEL_DATE;
  static const std::string CAT12_SURCHARGE;
  static const std::string LOC_NOT_MATCHED;
  static const std::string DIRECTION_NOT_MATCHED;
  static const std::string DATE_NOT_MATCHED;
  static const std::string TICKET_CARRIER_EXCEPTION;
  static const std::string MEMORY_ERROR;
  static const std::string LOG_FAIL_MSG;
  static const std::string SURCHARGE_DESC;
  static const std::string EXCLUDE_SURCHARGE_SECTOR;

};

} // namespace tse

