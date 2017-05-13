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

#include "Rules/SpecialTrips.h"
#include "Rules/TaxApplicationLimitUtil.h"

#include <algorithm>
#include <memory>

namespace tax
{

class LimitedJourneyInfo
{
  TaxLimitInfoIter _itinFirst;
  TaxLimitInfoIter _itinLast;
  TaxLimitInfoIter _first;
  TaxLimitInfoIter _last;

public:
  TaxPointMap _taxPointMap;

  LimitedJourneyInfo(TaxLimitInfoIter itinFirst, TaxLimitInfoIter itinLast,
                     TaxLimitInfoIter first, TaxLimitInfoIter last)
      : _itinFirst(itinFirst), _itinLast(itinLast),
        _first(first), _last(last) {}
  virtual ~LimitedJourneyInfo() {}

  template <typename T>
  static std::shared_ptr<LimitedJourneyInfo> create(TaxLimitInfoIter totalJourneyFirst,
                                                    TaxLimitInfoIter totalJourneyLast,
                                                    TaxLimitInfoIter first,
                                                    TaxLimitInfoIter last)
  {
    return std::make_shared<T>(totalJourneyFirst, totalJourneyLast, first, last);
  }

  template <typename T>
  static std::shared_ptr<LimitedJourneyInfo>
  create(TaxLimitInfoIter totalJourneyFirst, TaxLimitInfoIter totalJourneyLast, const Trip& trip)
  {
    TaxLimitInfoIter first = findBegin(totalJourneyFirst, totalJourneyLast, trip.first);
    TaxLimitInfoIter last = findEnd(totalJourneyFirst, totalJourneyLast, trip.second);

    return std::make_shared<T>(totalJourneyFirst, totalJourneyLast, first, last);
  }

  TaxLimitInfoIter getFirst() const { return _first; }
  TaxLimitInfoIter getLast() const { return _last; }

  TaxLimitInfoIter getItinFirst() const { return _itinFirst; }
  TaxLimitInfoIter getItinLast() const { return _itinLast; }

  void setTaxPointMap(const TaxPointMap& taxPointMap)
  {
    _taxPointMap = taxPointMap;
  }

  virtual std::vector<type::Index> findPassedRules() const = 0;
};

class ItineraryInfo : public LimitedJourneyInfo
{
public:
  ItineraryInfo(TaxLimitInfoIter totalJourneyFirst, TaxLimitInfoIter totalJourneyLast,
                TaxLimitInfoIter first, TaxLimitInfoIter last)
      : LimitedJourneyInfo(totalJourneyFirst, totalJourneyLast, first, last)
  {
  }

  std::vector<type::Index> findPassedRules() const override;
};

class ContinuousJourneyInfo : public LimitedJourneyInfo
{
public:
  ContinuousJourneyInfo(TaxLimitInfoIter totalJourneyFirst, TaxLimitInfoIter totalJourneyLast,
                        TaxLimitInfoIter first, TaxLimitInfoIter last)
      : LimitedJourneyInfo(totalJourneyFirst, totalJourneyLast, first, last)
  {
  }

  std::vector<type::Index> findPassedRules() const override;
};

class SingleJourneyInfo : public LimitedJourneyInfo
{
public:
  SingleJourneyInfo(TaxLimitInfoIter totalJourneyFirst, TaxLimitInfoIter totalJourneyLast,
                    TaxLimitInfoIter first, TaxLimitInfoIter last)
      : LimitedJourneyInfo(totalJourneyFirst, totalJourneyLast, first, last)
  {
  }

  std::vector<type::Index> findPassedRules() const override;
};

class UsOneWayTripInfo : public LimitedJourneyInfo
{
public:
  UsOneWayTripInfo(TaxLimitInfoIter totalJourneyFirst, TaxLimitInfoIter totalJourneyLast,
                   TaxLimitInfoIter first, TaxLimitInfoIter last)
      : LimitedJourneyInfo(totalJourneyFirst, totalJourneyLast, first, last)
  {
  }

  std::vector<type::Index> findPassedRules() const override;
};

class UsRoundTripInfo : public LimitedJourneyInfo
{
public:
  UsRoundTripInfo(TaxLimitInfoIter totalJourneyFirst, TaxLimitInfoIter totalJourneyLast,
                  TaxLimitInfoIter first, TaxLimitInfoIter last)
      : LimitedJourneyInfo(totalJourneyFirst, totalJourneyLast, first, last)
  {
  }

  std::vector<type::Index> findPassedRules() const override;
};

}
