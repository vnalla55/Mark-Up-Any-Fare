//-------------------------------------------------------------------
//
//  File:        RefundPermutation.cpp
//  Created:     August 05, 2009
//
//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DataModel/RefundPermutation.h"

#include "Common/Assert.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RefundPenalty.h"
#include "DataModel/RefundPricingTrx.h"
#include "DBAccess/VoluntaryRefundsInfo.h"

#include <algorithm>
#include <vector>

namespace tse
{

const Indicator RefundPermutation::BLANK = ' ';
const Indicator RefundPermutation::HISTORICAL_TICKET_BASED = 'A';
const Indicator RefundPermutation::HISTORICAL_TRAVELCOMMEN_BASED = 'B';
const Indicator RefundPermutation::TAX_NON_REFUNDABLE = 'X';
const Indicator RefundPermutation::HUNDRED_PERCENT_PENALTY = 'X';

namespace
{

typedef Indicator (VoluntaryRefundsInfo::*R3IndicatorGetter)() const;

template <R3IndicatorGetter getter>
struct HasIndicator
{
  bool operator()(const RefundProcessInfo* info, Indicator ind) const
  {
    return (info->record3().*getter)() == ind;
  }
};

typedef HasIndicator<&VoluntaryRefundsInfo::repriceInd> HasRepriceIndicator;

struct HasPaxTypeFare
{
  HasPaxTypeFare(const PaxTypeFare* ptf) : _ptf(ptf) {}

  bool operator()(const RefundProcessInfo* info) { return &info->paxTypeFare() == _ptf; }

private:
  const PaxTypeFare* _ptf;
};
}

void
RefundPermutation::setRepriceIndicator()
{
  Indicator ind[2] = { HISTORICAL_TICKET_BASED, BLANK };
  _repriceInd =
      std::find_first_of(
          _processInfos.begin(), _processInfos.end(), ind, ind + 2, HasRepriceIndicator()) !=
              _processInfos.end()
          ? HISTORICAL_TICKET_BASED
          : HISTORICAL_TRAVELCOMMEN_BASED;
}

std::vector<RefundProcessInfo*>::const_iterator
RefundPermutation::find(const PaxTypeFare* ptf) const
{
  return std::find_if(_processInfos.begin(), _processInfos.end(), HasPaxTypeFare(ptf));
}

bool
RefundPermutation::refundable(const PricingUnits& pricingUnits) const
{
  for (const PricingUnit* pricingUnit : pricingUnits)
    if (_penaltyFees.at(pricingUnit)->isRefundable(pricingUnit->fareUsage()))
      return true;

  return false;
}

MoneyAmount
RefundPermutation::overallPenalty(const RefundPricingTrx& trx) const
{
  return _minimumPenalty.value() > EPSILON
             ? trx.convertCurrency(_minimumPenalty, trx.exchangeItinCalculationCurrency()).value()
             : _totalPenalty.value();
}

namespace
{

struct MostRestrictive
{
  bool operator()(const RefundProcessInfo* elem1, const RefundProcessInfo* elem2) const
  {
    return hierarchy(elem1->record3().formOfRefund()) < hierarchy(elem2->record3().formOfRefund());
  }

protected:
  char hierarchy(const Indicator& rf) const
  {
    switch (rf)
    {
    case RefundPermutation::ORIGINAL_FOP:
      return 1;
    case RefundPermutation::ANY_FORM_OF_PAYMENT:
      return 0;
    case RefundPermutation::MCO:
      return 2;
    case RefundPermutation::SCRIPT:
      return 4;
    case RefundPermutation::VOUCHER:
      return 3;
    default:
      TSE_ASSERT(false && "RefundPermutation - MostRestricticve::hierarchy() : unknown indicator");
      return 0;
    }
  }
};

}

Indicator
RefundPermutation::formOfRefundInd() const
{
  return (**std::max_element(_processInfos.begin(), _processInfos.end(), MostRestrictive()))
      .record3()
      .formOfRefund();
}

bool
RefundPermutation::taxRefundable() const
{
  auto taxRefundable = [] (const RefundProcessInfo* rpi)
  {
    return !(rpi->record3().taxNonrefundableInd() == RefundPermutation::TAX_NON_REFUNDABLE &&
            rpi->record3().cancellationInd() == RefundPermutation::HUNDRED_PERCENT_PENALTY);
  };

  return std::find_if(_processInfos.begin(), _processInfos.end(), taxRefundable)
      != _processInfos.end();
}

}
