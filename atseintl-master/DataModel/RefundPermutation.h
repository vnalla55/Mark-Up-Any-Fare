//-------------------------------------------------------------------
//
//  File:        RefundPermutation.h
//  Created:     July 29, 2009
//
//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/Money.h"
#include "Common/TseConsts.h"
#include "DataModel/RefundProcessInfo.h"

#include <deque>
#include <vector>

namespace tse
{

class PricingTrx;
class RefundPenalty;
class PricingUnit;
class RefundPricingTrx;

class RefundPermutation
{
public:
  typedef std::vector<PricingUnit*> PricingUnits;

  RefundPermutation()
    : _number(0),
      _repriceInd(BLANK),
      _totalPenalty(NUC),
      _minimumPenalty(NUC),
      _waivedPenalty(false)
  {
  }

  void assign(unsigned number, std::deque<RefundProcessInfo*>& processInfos)
  {
    _number = number;
    _processInfos.assign(processInfos.begin(), processInfos.end());
  }

  unsigned& number() { return _number; }
  unsigned number() const { return _number; }

  std::vector<RefundProcessInfo*>& processInfos() { return _processInfos; }
  const std::vector<RefundProcessInfo*>& processInfos() const { return _processInfos; }

  typedef std::map<const PricingUnit*, RefundPenalty*> PenaltyFees;
  PenaltyFees& penaltyFees() { return _penaltyFees; }
  const PenaltyFees& penaltyFees() const { return _penaltyFees; }

  const Money& totalPenalty() const { return _totalPenalty; }
  Money& totalPenalty() { return _totalPenalty; }

  const Money& minimumPenalty() const { return _minimumPenalty; }
  Money& minimumPenalty() { return _minimumPenalty; }

  bool taxRefundable() const;

  bool waivedPenalty() const { return _waivedPenalty; }
  bool& waivedPenalty() { return _waivedPenalty; }

  static const Indicator BLANK;
  static const Indicator HISTORICAL_TICKET_BASED;
  static const Indicator HISTORICAL_TRAVELCOMMEN_BASED;
  static const Indicator TAX_NON_REFUNDABLE;
  static const Indicator HUNDRED_PERCENT_PENALTY;

  Indicator repriceIndicator() const { return _repriceInd; }
  Indicator& repriceIndicator() { return _repriceInd; }
  void setRepriceIndicator();

  std::vector<RefundProcessInfo*>::const_iterator find(const PaxTypeFare* ptf) const;

  bool refundable(const PricingUnits& pricingUnits) const;

  enum FormOfRefund
  {
    ORIGINAL_FOP = ' ',
    ANY_FORM_OF_PAYMENT = 'A',
    MCO = 'M',
    SCRIPT = 'S',
    VOUCHER = 'V'
  };
  Indicator formOfRefundInd() const;

  MoneyAmount overallPenalty(const RefundPricingTrx& trx) const;

private:
  std::vector<RefundProcessInfo*> _processInfos;
  PenaltyFees _penaltyFees;
  unsigned _number;
  Indicator _repriceInd;
  Money _totalPenalty;
  Money _minimumPenalty;
  bool _waivedPenalty;

  friend class RefundPermutationTest;
};
}

