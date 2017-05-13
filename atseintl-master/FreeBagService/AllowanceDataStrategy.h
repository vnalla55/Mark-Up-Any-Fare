//-------------------------------------------------------------------
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#pragma once

#include "FreeBagService/DataStrategyBase.h"

namespace tse
{
class AirSeg;
class PricingTrx;
class SubCodeInfo;
class Diag852Collector;

class AllowanceDataStrategy : public DataStrategyBase
{
  friend class AllowanceDataStrategyTest;

public:
  AllowanceDataStrategy(PricingTrx& trx) : DataStrategyBase(trx) {}

private:
  enum AllowanceStepResult
  {
    S5_FAIL,
    S7_FAIL,
    S7_PASS,
    S7_DEFER
  };

  enum AllowanceStepFlags
  {
    USE_MARKETING = 0x1,
    IS_DEFERRED = 0x2
  };

  struct MatchS7Context
  {
    MatchS7Context(const BaggageTravelInfo& binfo,
                   const CheckedPoint& fcp,
                   const BaggageTripType& btt,
                   Diag852Collector* dc852)
      : bagInfo(binfo), furthestCp(fcp), bagTripType(btt), dc(dc852)
    {
    }
    const BaggageTravelInfo& bagInfo;
    const CheckedPoint& furthestCp;
    const BaggageTripType bagTripType;
    Diag852Collector* dc;
  };

  virtual void processBaggageTravel(BaggageTravel* baggageTravel,
                                    const BaggageTravelInfo& bagInfo,
                                    const CheckedPoint& furthestCheckedPoint,
                                    BaggageTripType baggageTripType,
                                    Diag852Collector* dc) const override;
  AllowanceStepResult matchAllowance(BaggageTravel& bagTravel,
                                     const MatchS7Context& cxt,
                                     const AirSeg& as,
                                     const uint32_t flags) const;
  void processBaggageTravelNoDotTable(BaggageTravel& bagTravel,
                                      const MatchS7Context& cxt) const;
  void processBaggageTravelNoDotTableOld(BaggageTravel* baggageTravel,
                                         const BaggageTravelInfo& bagInfo,
                                         const CheckedPoint& furthestCheckedPoint,
                                         bool isUsDot,
                                         Diag852Collector* dc) const;
  void processBaggageTravelDotTable(BaggageTravel* baggageTravel,
                                    const BaggageTravelInfo& bagInfo,
                                    const CheckedPoint& furthestCheckedPoint,
                                    BaggageTripType btt,
                                    Diag852Collector* dc) const;
  bool findJourneyMscAllowance(BaggageTravel& baggageTravel,
                               const BaggageTravelInfo& bagInfo,
                               const CheckedPoint& furthestCP,
                               BaggageTripType btt,
                               Diag852Collector* dc) const;
  void processBaggageTravelWhollyWithinUsOrCa(BaggageTravel* baggageTravel,
                                              const BaggageTravelInfo& bagInfo,
                                              const CheckedPoint& furthestCheckedPoint,
                                              Diag852Collector* dc) const;

  const SubCodeInfo* retrieveS5Record(const CarrierCode& carrier) const;
  const SubCodeInfo* retrieveS5Record(BaggageTravel* baggageTravel, bool isUsDot) const;
  const SubCodeInfo* retrieveS5Record(const VendorCode& vendor, const CarrierCode& carrier) const;

  bool checkServiceType(const SubCodeInfo* codeInfo) const;

  OCFees& matchS7(BaggageTravel* baggageTravel,
                  const BaggageTravelInfo& bagInfo,
                  const SubCodeInfo* s5,
                  const CheckedPoint& furthestCheckedPoint,
                  bool usDot,
                  Diag852Collector* dc,
                  bool defer = false,
                  bool allowanceCarrierOverridden = false) const;
  OCFees& matchS7(BaggageTravel* baggageTravel,
                  const BaggageTravelInfo& bagInfo,
                  const SubCodeInfo* s5,
                  const CheckedPoint& furthestCheckedPoint,
                  bool usDot,
                  uint32_t lastSequence,
                  Diag852Collector* dc,
                  bool defer = false,
                  bool allowanceCarrierOverridden = false) const;

  bool shouldDisplayDiagnostic(BaggageTravel* baggageTravel,
                               const BaggageTravelInfo& bagInfo,
                               const Diag852Collector* dc) const;

  void processBaggageTravelForAllowanceCarrierOverridden(BaggageTravel* baggageTravel,
                                                         const BaggageTravelInfo& bagInfo,
                                                         const CheckedPoint& furthestCheckedPoint,
                                                         bool isUsDot,
                                                         Diag852Collector* dc) const;
};
} // tse
