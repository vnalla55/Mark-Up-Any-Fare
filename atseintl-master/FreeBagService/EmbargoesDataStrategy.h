//-------------------------------------------------------------------
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
//-------------------------------------------------------------------
#pragma once

#include "FreeBagService/AncillaryChargesDataStrategy.h"

namespace tse
{

class EmbargoesDataStrategy : public AncillaryChargesDataStrategy
{
  friend class EmbargoesDataStrategyTest;

public:
  EmbargoesDataStrategy(PricingTrx& trx);
  virtual ~EmbargoesDataStrategy();

private:
  virtual void processBaggageTravel(BaggageTravel* baggageTravel,
                                    const BaggageTravelInfo& bagInfo,
                                    const CheckedPoint& furthestCheckedPoint,
                                    BaggageTripType btt,
                                    Diag852Collector* dc) const override;

  virtual void retrieveS5Records(const BaggageTravel* baggageTravel,
                                 std::vector<const SubCodeInfo*>& subCodes) const override;

  const SubCodeInfo* retrieveS5Record(const CarrierCode& carrier) const;

  OCFees& matchS7(BaggageTravel* baggageTravel,
                  const BaggageTravelInfo& bagInfo,
                  const SubCodeInfo* s5,
                  const CheckedPoint& furthestCheckedPoint,
                  bool isUsDot,
                  Diag852Collector* dc) const;

  void printS7ProcessingContext(BaggageTravel* baggageTravel,
                                const BaggageTravelInfo& bagInfo,
                                const SubCodeInfo* s5,
                                bool isUsDot,
                                Diag852Collector* dc) const;

  bool shouldDisplayDiagnostic(BaggageTravel* baggageTravel,
                               const BaggageTravelInfo& bagInfo,
                               const ServiceSubTypeCode& subTypeCode,
                               const Diag852Collector* dc) const;

  bool shouldDisplayDiagnostic(BaggageTravel* baggageTravel,
                               const BaggageTravelInfo& bagInfo,
                               const Diag852Collector* dc) const;
};
}

