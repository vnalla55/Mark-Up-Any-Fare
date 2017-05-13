// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Rules/TaxLimiter.h"

#include <cassert>

#include "Common/RangeUtils.h"
#include "DomainDataObjects/YqYrPath.h"
#include "Processor/BusinessRulesProcessor.h"
#include "Rules/LimitGroup.h"

namespace tax
{
namespace
{
typedef std::vector<type::TaxApplicationLimit> TaxApplicationLimits;
typedef std::vector<type::MoneyAmount> MoneyAmounts;
typedef std::vector<const BusinessRule*> BusinessRules;
}

void
TaxLimiter::limitYqYrs(const YqYrPath& yqYrPath, std::vector<PaymentWithRules>& paymentsToCalculate)
{
  // (detailId, yqYr in detail id)
  typedef std::pair<type::Index, type::Index> PaymentYqYrRef;

  const type::Index yqYrCount = yqYrPath.yqYrUsages().size();
  const type::Index count = paymentsToCalculate.size();

  std::vector<TaxApplicationLimits> limits(yqYrCount);
  std::vector<MoneyAmounts> amounts(yqYrCount);
  std::vector<BusinessRules> failingRules(yqYrCount);
  std::vector<std::vector<PaymentYqYrRef> > paymentReferences(yqYrCount);

  for (type::Index i = 0; i < count; ++i)
  {
    const type::MoneyAmount& amount = paymentsToCalculate[i].paymentDetail->taxAmt();
    const LimitGroup& limitGroup = *paymentsToCalculate[i].paymentDetail->getLimitGroup();
    type::TaxApplicationLimit limit = type::TaxApplicationLimit::Unlimited;
    const BusinessRule* failingRule = nullptr;
    if (limitGroup._limitType == type::TaxApplicationLimit::OnceForItin)
    {
      limit = type::TaxApplicationLimit::OnceForItin;
      failingRule = &limitGroup._onePerItinLimitRule.get();
    }
    else // assume blank if not OnceForItin - to implement full YqYr limiting later
    {
      failingRule = &limitGroup._blankLimitRule.get();
    }

    const TaxableYqYrs& taxableYqYrs = paymentsToCalculate[i].paymentDetail->getYqYrDetails();
    for (type::Index j = 0; j < taxableYqYrs._subject.size(); ++j)
    {
      if (!taxableYqYrs.isFailedRule(j))
      {
        const type::Index yqYrId = taxableYqYrs._ids[j];
        limits[yqYrId].push_back(limit);
        amounts[yqYrId].push_back(amount);
        failingRules[yqYrId].push_back(failingRule);
        paymentReferences[yqYrId].push_back(std::make_pair(i, j));
      }
    }
  }

  for (type::Index i = 0; i < yqYrCount; ++i)
  {
    if (limits[i].empty())
      continue;

    const std::vector<bool> result = limit(limits[i], amounts[i]);

    for (type::Index j = 0; j < result.size(); ++j)
    {
      if (!result[j])
      {
        paymentsToCalculate[paymentReferences[i][j].first]
            .paymentDetail->getMutableYqYrDetails()
            .setFailedRule(paymentReferences[i][j].second, *failingRules[i][j]);
      }
    }
  }
}

std::vector<bool>
TaxLimiter::limit(const std::vector<type::TaxApplicationLimit>& limits,
                  const std::vector<type::MoneyAmount>& amounts)
{
  assert(limits.size() == amounts.size());
  std::vector<bool> result(limits.size(), true);

  type::MoneyAmount maxAmount(amounts[0]);
  type::Index lastMax(0);
  for (type::Index i = 1; i < limits.size(); ++i)
  {
    if (limits[i] == type::TaxApplicationLimit::OnceForItin ||
        limits[i] == type::TaxApplicationLimit::Unlimited)
    {
      if (maxAmount < amounts[i])
      {
        if (limits[lastMax] != type::TaxApplicationLimit::Unlimited)
        {
          result[lastMax] = false;
        }

        maxAmount = amounts[i];
        lastMax = i;
      }
      else if (limits[i] != type::TaxApplicationLimit::Unlimited)
      {
        result[i] = false;
      }
    }
  }

  return result;
}

void
TaxLimiter::overlapItinerary(std::vector<PaymentWithRules>& paymentsToCalculate)
{
  const PaymentDetail* lastDetail = nullptr;
  ProperRange lastRange(0, 0);

  for(PaymentWithRules& payment : paymentsToCalculate)
  {
    PaymentDetail* currentDetail = payment.paymentDetail;
    if (currentDetail->getItineraryDetail().isFailedRule())
      continue;

    const type::Index begin = currentDetail->getTaxPointBegin().id();
    const type::Index end = currentDetail->getItineraryDetail()._data->_taxPointLoc2->id();

    ProperRange currentRange =
        currentDetail->isExempt() ? ProperRange(begin, begin) : ProperRange(begin, end);

    if (!lastDetail || (lastRange & currentRange).empty)
    {
      lastDetail = currentDetail;
      lastRange = currentRange;
    }
    else
    {
      currentDetail->getMutableItineraryDetail().setFailedRule(
          &payment.paymentDetail->getLimitGroup()->_sectorOverlappingRule);
    }
  }
}

void
TaxLimiter::overlapYqYrs(const type::Index yqYrCount,
                         std::vector<PaymentWithRules>& paymentsToCalculate)
{
  const type::Index count = paymentsToCalculate.size();

  std::vector<bool> lastFound(yqYrCount);
  std::vector<ProperRange> lastRange(yqYrCount, ProperRange(0, 0));

  for (type::Index i = 0; i < count; ++i)
  {
    PaymentDetail& currentDetail = *paymentsToCalculate[i].paymentDetail;
    const type::Index begin = currentDetail.getTaxPointBegin().id();
    const LimitGroup& limitGroup = *paymentsToCalculate[i].paymentDetail->getLimitGroup();
    TaxableYqYrs& taxableYqYrs = currentDetail.getMutableYqYrDetails();
    for (type::Index j = 0; j < taxableYqYrs._subject.size(); ++j)
    {
      if (taxableYqYrs.isFailedRule(j))
        continue;

      const type::Index yqYrId = taxableYqYrs._ids[j];
      const type::Index end = taxableYqYrs._data[j]._taxPointEnd->id();
      const ProperRange currentRange =
          currentDetail.isExempt() ? ProperRange(begin, begin) : ProperRange(begin, end);
      if (!lastFound[yqYrId] || (lastRange[yqYrId] & currentRange).empty)
      {
        lastFound[yqYrId] = true;
        lastRange[yqYrId] = currentRange;
      }
      else
      {
        taxableYqYrs.setFailedRule(j, limitGroup._sectorOverlappingRule);
      }
    }
  }
}
}
