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

#include "FreeBagService/BaggageAllowanceValidator.h"

#include <tuple>

namespace tse
{
class OptionalServicesInfo;
class BaggageCharge;

class BaggageChargesValidator : public BaggageAllowanceValidator
{
  friend class BaggageChargesValidatorTest;

public:
  using BaggageAllowanceValidator::BaggageAllowanceValidator;

  void collectCharges(const SubCodeInfo& s5,
                      const uint32_t freePieces,
                      IChargeCollector& out);
  virtual void validate(const SubCodeInfo& subCodeInfo, ChargeVector& matchedCharges);

protected:
  typedef std::tuple<const OptionalServicesInfo*, BaggageCharge, StatusS7Validation>
  S7StatusTuple;

  virtual StatusS7Validation validateS7Data(OptionalServicesInfo& optSrvInfo, OCFees& ocFees) override;
  virtual StatusS7Validation
  checkServiceNotAvailNoCharge(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const override;
  bool checkAndOrIndicator(const OptionalServicesInfo& optSrvInfo) const;
  virtual bool checkBaggageWeightUnit(const OptionalServicesInfo& optSrvInfo) const;
  virtual bool checkFeeApplication(const OptionalServicesInfo& optSrvInfo) const;
  virtual bool checkOccurrence(const OptionalServicesInfo& optSrvInfo) const;
  static bool matchOccurrence(const OptionalServicesInfo& s7, int32_t bagNo);
  static bool isOccurrenceBlankBlank(const OptionalServicesInfo& optSrvInfo);
  void convertCurrency(ServiceFeeUtil& util, OCFees& fee) const;
  const Loc& getLocForSfcValidation() const override;
  void printDiagS7Info(const std::vector<S7StatusTuple>& s7statuses, uint32_t bagNo) const;

  BaggageCharge* buildCharge(const SubCodeInfo& subCodeInfo) const;
  void supplementAndAppendCharge(const SubCodeInfo& subCodeInfo,
                                 BaggageCharge* baggageCharge,
                                 ChargeVector& matchedCharges) const;

  bool
  checkFrequentFlyerStatus(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const override;
};
} // tse
