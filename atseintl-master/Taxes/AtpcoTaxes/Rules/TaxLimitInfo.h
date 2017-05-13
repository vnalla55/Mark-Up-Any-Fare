// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
#pragma once

#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Common/Types.h"
#include "Rules/PaymentWithRules.h"

#include <set>
#include <vector>

namespace tax
{


class TaxPointMap
{
  std::set<type::Index> _taxPointInJourney;
public:
  TaxPointMap() {}

  void addJourney(const std::vector<Trip>& journeys)
  {
    for (const Trip& trip : journeys)
    {
      for(type::Index id = trip.first; id<= trip.second; ++id)
      {
        _taxPointInJourney.insert(id);
      }
    }
  }

  bool isOnlyInItinerary(type::Index id) const
  {
    return _taxPointInJourney.find(id) == _taxPointInJourney.end();
  }
};



class TaxLimitInfo
{
  type::MoneyAmount _moneyAmount;
  type::TaxApplicationLimit _limitType;
  type::Index _id;
  bool _isExempt;
public:
  TaxLimitInfo(type::Index id,
               const type::TaxApplicationLimit& limitType,
               const type::MoneyAmount& moneyAmount,
               bool isExempt = false)
    : _moneyAmount(moneyAmount),
      _limitType(limitType),
      _id(id),
      _isExempt(isExempt)
  {
  }

  static TaxLimitInfo
  unlimited(type::Index id, const type::MoneyAmount& money)
  {
    return TaxLimitInfo(id, type::TaxApplicationLimit::Unlimited, money);
  }

  static TaxLimitInfo
  onceForItin(type::Index id, const type::MoneyAmount&  money)
  {
    return TaxLimitInfo(id, type::TaxApplicationLimit::OnceForItin, money);
  }

  static TaxLimitInfo
  continousJourney(type::Index id, const type::MoneyAmount&  money)
  {
    return TaxLimitInfo(id, type::TaxApplicationLimit::FirstTwoPerContinuousJourney, money);
  }

  static TaxLimitInfo
  singleJourney(type::Index id, const type::MoneyAmount&  money)
  {
    return TaxLimitInfo(id, type::TaxApplicationLimit::OncePerSingleJourney, money);
  }

  static TaxLimitInfo
  usRoundTrip(type::Index id, const type::MoneyAmount&  money)
  {
    return TaxLimitInfo(id, type::TaxApplicationLimit::FirstTwoPerUSRoundTrip, money);
  }

  static TaxLimitInfo
  noRestrictions(type::Index id, const type::MoneyAmount& money)
  {
    return TaxLimitInfo(id, type::TaxApplicationLimit::Unlimited, money);
  }

  static TaxLimitInfo
  create(const PaymentDetail& paymentDetail)
  {
    return TaxLimitInfo(paymentDetail.getTaxPointBegin().id(),
                        paymentDetail.getLimitType(),
                        paymentDetail.taxAmt(),
                        paymentDetail.isExempt());
  }

  bool isLimit(const type::TaxApplicationLimit& limitType) const
  {
    return _limitType == limitType;
  }

  type::MoneyAmount& getMoneyAmount()
  {
    return _moneyAmount;
  }

  type::Index getId()
  {
    return _id;
  }

  bool isExempt() const
  {
    return _isExempt;
  }
};

typedef std::vector<TaxLimitInfo>::iterator TaxLimitInfoIter;

}
