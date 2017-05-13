#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/YQYR/YQYRCalculator.h"

#include <array>
#include <unordered_map>

namespace tse
{
class Agent;
class Loc;
class PaxTypeFare;
class ShoppingTrx;
class YQYRFees;

namespace YQYR
{
class DiagCollectorShopping;
class YQYRBucket;

class YQYRFilterBase
{
public:
  YQYRFilterBase(ShoppingTrx& trx,
                 const char* filterName,
                 Validations::Validation validation = Validations::NONE)
    : _trx(trx), _filterName(filterName), _validation(validation)
  {
  }

  const char* getFilterName() const { return _filterName; } // for diagnostics
  Validations::Validation replacesValidation() const { return _validation; }

protected:
  ShoppingTrx& _trx;
  const char* _filterName;
  Validations::Validation _validation;
};

class YQYRFilter : public YQYRFilterBase
{
public:
  using YQYRFilterBase::YQYRFilterBase;

  virtual ~YQYRFilter() {}

  virtual bool isFilteredOut(const YQYRFees* fee) const = 0;
  virtual bool isNeeded() const { return true; }
};

// Classifies the fee to specific bucket(s)
class YQYRClassifier : public YQYRFilterBase
{
public:
  using YQYRFilterBase::YQYRFilterBase;

  virtual ~YQYRClassifier() {}

  bool
  classify(std::vector<YQYRBucket>& buckets, const YQYRFees* fee, DiagCollectorShopping* dc) const;

protected:
  virtual bool classify(std::vector<YQYRBucket>& buckets, const YQYRFees* fee) const = 0;

  static void addToAllBuckets(std::vector<YQYRBucket>& buckets, const YQYRFees* fee);
};

class YQYRFilters
{
public:
  static constexpr size_t MaxFilters = 6;
  using Array = std::array<YQYRFilter*, MaxFilters>;
  using Iterator = Array::const_iterator;

  bool append(YQYRFilter* filter);
  bool prepend(YQYRFilter* filter);
  Iterator begin() const { return _filters.begin(); }
  Iterator end() const { return _filters.begin() + _filterCount; }

  bool isFilteredOut(const YQYRFees* fee, DiagCollectorShopping* dc) const;

private:
  Array _filters;
  size_t _filterCount = 0;
};

template <typename Filter>
class YQYRFilterFareCachingAdapter : public Filter
{
public:
  using Filter::Filter;

  virtual bool isFilteredOut(const YQYRFees* fee) const override
  {
    auto processedIt(_processedFees.find(fee));
    if (processedIt != _processedFees.end())
      return processedIt->second;

    const bool result(Filter::isFilteredOut(fee));
    _processedFees[fee] = result;

    return result;
  }

private:
  typedef std::unordered_map<const YQYRFees*, bool> ProcessedFeesCache;
  mutable ProcessedFeesCache _processedFees;
};

class YQYRFilterReturnsToOrigin : public YQYRFilter
{
public:
  YQYRFilterReturnsToOrigin(ShoppingTrx& trx, const bool returnsToOrigin);

  virtual bool isFilteredOut(const YQYRFees* fee) const override;

private:
  const bool _returnsToOrigin;
};

class YQYRFilterPointOfSale : public YQYRFilter
{
public:
  YQYRFilterPointOfSale(ShoppingTrx& trx);

  virtual bool isFilteredOut(const YQYRFees* fee) const override;
  virtual bool isNeeded() const override;

private:
  const Loc* _posLoc;
  const DateTime _ticketingDate;
};

class YQYRFilterPointOfTicketing : public YQYRFilter
{
public:
  YQYRFilterPointOfTicketing(ShoppingTrx& trx);

  virtual bool isFilteredOut(const YQYRFees* fee) const override;
  virtual bool isNeeded() const override;

private:
  const Loc* _ticketLoc;
  const DateTime _ticketingDate;
};

class YQYRFilterAgencyPCC : public YQYRFilter
{
public:
  YQYRFilterAgencyPCC(ShoppingTrx& trx);

  virtual bool isFilteredOut(const YQYRFees* fee) const override;

private:
  const Agent& _ticketingAgent;
};

class YQYRFilterTicketingDate : public YQYRFilter
{
public:
  YQYRFilterTicketingDate(ShoppingTrx& trx);

  virtual bool isFilteredOut(const YQYRFees* fee) const override;
  virtual bool isNeeded() const override;

private:
  const DateTime _ticketingDate;
};

class YQYRFilterFareBasisCode : public YQYRFilter
{
public:
  YQYRFilterFareBasisCode(ShoppingTrx& trx, const std::vector<PaxTypeFare*>& applicableFares);

  virtual bool isFilteredOut(const YQYRFees* fee) const override;

private:
  std::vector<std::string> _applicableFareBasisCodes;
};

class YQYRFilterPassengerType : public YQYRFilter
{
public:
  YQYRFilterPassengerType(ShoppingTrx& trx, const std::vector<PaxTypeFare*>& applicableFares);

  virtual bool isFilteredOut(const YQYRFees* fee) const override;

private:
  std::vector<PaxTypeCode> _applicablePaxTypes;
};

class YQYRFilterJourneyLocation : public YQYRFilter
{
public:
  YQYRFilterJourneyLocation(ShoppingTrx& trx, const Loc* journeyOrigin, const Loc* furthestPoint);

  virtual bool isFilteredOut(const YQYRFees* fee) const override;

private:
  const DateTime _ticketingDate;
  const Loc* _journeyOrigin;
  const Loc* _furthestPoint;
};

class YQYRClassifierFurthestPoint : public YQYRClassifier
{
public:
  YQYRClassifierFurthestPoint(ShoppingTrx& trx, const Loc* journeyOrigin);

protected:
  virtual bool classify(std::vector<YQYRBucket>& buckets, const YQYRFees* fee) const override;

private:
  const DateTime _ticketingDate;
  const Loc* _journeyOrigin;
};

class YQYRClassifierPassengerType : public YQYRClassifier
{
public:
  YQYRClassifierPassengerType(ShoppingTrx& trx);

protected:
  virtual bool classify(std::vector<YQYRBucket>& buckets, const YQYRFees* fee) const override;
};
}
}
