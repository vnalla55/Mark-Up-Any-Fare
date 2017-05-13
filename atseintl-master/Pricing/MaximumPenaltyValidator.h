//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "DataModel/MaxPenaltyInfo.h"
#include "Pricing/MaximumPenaltyCalculator.h"

#include <utility>

namespace tse {

class PricingTrx;
class FarePath;
class FareMarket;

class MaximumPenaltyValidator
{
public:
  MaximumPenaltyValidator(PricingTrx& trx);

  std::pair<bool, std::string> validateFarePath(FarePath& farePath) const;

  void completeResponse(FarePath& farePath) const;

  void prevalidate16(FareMarket& fareMarket) const;

private:
  std::pair<bool, std::string> validateFilter(const MaxPenaltyInfo::Filter& filter,
                                              const MaxPenaltyResponse::Fees& fees) const;

  CurrencyCode getPenaltyCurrency(const MaxPenaltyInfo::Filter& filter) const;
  bool validateQuery(const smp::ChangeQuery& query,
                     const smp::RecordApplication& departure,
                     const MaxPenaltyResponse::Fees& fees) const;

  bool validateAmount(const Money& maxPenalty,
                      const smp::RecordApplication& departure,
                      const MaxPenaltyResponse::Fees& fees) const;

  MaxPenaltyResponse& getMaxPenaltyResponse(FarePath& farePath) const;

  static std::string
  getFailedFaresDiagnostics(const MaxPenaltyInfo& maxPenalty, const MaxPenaltyStats& stats);
  void updateFailStats(MaxPenaltyStats& stats,
                       const MaxPenaltyInfo& maxPenalty) const;

  void printDiagHeader(const PaxTypeCode& paxType) const;
  void printValidationResult(const std::pair<bool, std::string>& result) const;

  PricingTrx& _trx;
  bool _diagEnabled;
  unsigned int _failedFaresThreshold;

  friend class MaximumPenaltyValidatorTest;
};

} /* namespace tse */

