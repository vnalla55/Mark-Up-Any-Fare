/*
 * Cat16MaxPenaltyCalculator.h
 *
 *  Created on: May 15, 2015
 *      Author: SG0217429
 */

#pragma once

#include "Common/CurrencyConversionRequest.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "DBAccess/PenaltyInfo.h"

#include <functional>
#include <unordered_set>

namespace tse
{
namespace
{
using ConversionType = CurrencyConversionRequest::ApplicationType;
}

class DiagManager;
class FareUsage;
class PaxTypeFare;
class PricingTrx;

class Cat16MaxPenaltyCalculator
{
  friend class Cat16MaxPenaltyCalculatorTest;

public:
  using PenaltiesCollection = std::unordered_set<const PenaltyInfo*>;
  using ResponseFees = MaxPenaltyResponse::Fees;

  enum PenaltyType
  { CHANGE_PEN,
    REFUND_PEN };

  explicit Cat16MaxPenaltyCalculator(PricingTrx& trx, bool skipDiag);

  static bool areNon(const PenaltiesCollection& records,
                     std::function<bool(const PenaltyInfo*)> isNon,
                     std::function<bool(const PenaltyInfo*)> recordApplies,
                     const smp::RecordApplication& application);
  static bool areNonRefundable(const PenaltiesCollection& records,
                               const smp::RecordApplication& application);
  static bool areNonChangeable(const std::unordered_set<const PenaltyInfo*>& records,
                               const smp::RecordApplication& application);

  ResponseFees calculateMaxPenalty(const PenaltiesCollection& records,
                                   const CurrencyCode& penaltyCurrencyCode,
                                   const PaxTypeFare& ptf,
                                   const FareUsage* fareUsage,
                                   smp::RecordApplication departureInd,
                                   PenaltyType penaltyType);

private:
  ResponseFees calculationImpl(const PenaltiesCollection& records,
                               const CurrencyCode& penaltyCurrencyCode,
                               const PaxTypeFare& ptf,
                               smp::RecordApplication departureInd,
                               std::function<bool(const PenaltyInfo*)> isNon,
                               std::function<bool(const PenaltyInfo*)> recordApplies,
                               const FareUsage* fareUsage = nullptr);

  static bool isNonRefundable(const PenaltyInfo* penaltyInfo);
  static bool refundApplies(const PenaltyInfo* penaltyInfo);

  static bool isNonChangeable(const PenaltyInfo* penaltyInfo);
  static bool changeApplies(const PenaltyInfo* penaltyInfo);

  void printRecordsAfterValidation(const PenaltyInfo* recordBefore,
                                   const PenaltyInfo* recordAfter,
                                   smp::RecordApplication departureInd,
                                   DiagManager& diag) const;

  PricingTrx& _trx;
  bool _diagEnabled, _diagPrevalidationEnabled;
  ResponseFees _fees;
};

} /* namespace tse */
