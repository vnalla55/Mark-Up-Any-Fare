/*
 * CarryOnAllowanceDataStrategy.h
 *
 *  Created on: Jan 9, 2013
 *      Author: SG0216830
 */

#pragma once

#include "FreeBagService/DataStrategyBase.h"

namespace tse
{
class CarryOnAllowanceDataStrategy : public DataStrategyBase
{
  friend class CarryOnAllowanceDataStrategyTest;

public:
  CarryOnAllowanceDataStrategy(PricingTrx& trx);
  virtual ~CarryOnAllowanceDataStrategy();

  virtual void processBaggageTravel(BaggageTravel* baggageTravel,
                                    const BaggageTravelInfo& bagInfo,
                                    const CheckedPoint& furthestCheckedPoint,
                                    BaggageTripType btt,
                                    Diag852Collector* dc) const override;

private:
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
                               const Diag852Collector* dc) const;
};
}

