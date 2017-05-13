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

#include <boost/function.hpp>


namespace tse
{
class Diag852Collector;

class ChargesDataStrategy : public DataStrategyBase
{
  friend class ChargesDataStrategyTest;

public:
  ChargesDataStrategy(PricingTrx& trx) : DataStrategyBase(trx) {}

  virtual void processBaggageTravel(BaggageTravel* baggageTravel,
                                    const BaggageTravelInfo& bagInfo,
                                    const CheckedPoint& furthestCheckedPoint,
                                    BaggageTripType btt,
                                    Diag852Collector* dc) const override;

protected:
  void retrieveS5Records(const BaggageTravel* baggageTravel,
                         bool isUsDot,
                         std::vector<const SubCodeInfo*>& subCodes) const;

  void retrieveS5Records(
      const CarrierCode& carrier,
      std::vector<const SubCodeInfo*>& subCodes,
      const boost::function<bool(const SubCodeInfo* const subCodeInfo)>& checkSubCodeInfo) const;

  void retrieveCharges(BaggageTravel* baggageTravel,
                       const BaggageTravelInfo& bagInfo,
                       const std::vector<const SubCodeInfo*> subCodes,
                       const CheckedPoint& furthestCheckedPoint,
                       bool isUsDot,
                       Diag852Collector* dc) const;

  void matchS7s(BaggageTravel* baggageTravel,
                const BaggageTravelInfo& bagInfo,
                const SubCodeInfo* s5,
                const CheckedPoint& furthestCheckedPoint,
                bool isUsDot,
                Diag852Collector* dc,
                ChargeVector& charges) const;
  virtual void matchS7s(BaggageTravel& baggageTravel,
                        const SubCodeInfo* s5,
                        const CheckedPoint& furthestCheckedPoint,
                        bool isUsDot,
                        Diag852Collector* dc,
                        ChargeVector& charges) const;

  bool matchOccurrence(const OptionalServicesInfo* s7, int32_t bagNo) const;

  virtual void findLowestCharges(BaggageTravel* bt,
                                 const BaggageTravelInfo& bagInfo,
                                 ChargeVector& charges,
                                 Diag852Collector* dc) const;

  bool shouldDisplayDiagnostic(BaggageTravel* baggageTravel,
                               const BaggageTravelInfo& bagInfo,
                               const ServiceSubTypeCode& subTypeCode,
                               const Diag852Collector* dc) const;

  bool shouldDisplayS7Diagnostic(const Diag852Collector* dc) const;

  virtual bool shouldDisplayChargesDiagnostic(BaggageTravel* baggageTravel,
                                              const BaggageTravelInfo& bagInfo,
                                              const Diag852Collector* dc) const;

  virtual void printS7ProcessingContext(BaggageTravel* baggageTravel,
                                        const BaggageTravelInfo& bagInfo,
                                        const SubCodeInfo* s5,
                                        bool isUsDot,
                                        Diag852Collector* dc,
                                        bool defer = false,
                                        bool isCarrierOverride = false) const;

  const OptionalServicesInfo* getAllowanceS7(const BaggageTravel* baggageTravel) const;

  bool allSegmentsOnTheSameCarrier(const Itin&) const;
};
} // tse
