#pragma once

#include "Rules/CommonPredicates.h"

namespace tse
{
class TravelSeg;

class IsDate : public Predicate
{
public:
  IsDate();

  void initialize(int year1, int month1, int day1, int year2, int month2, int day2);

  void
  initialize(int year1, int month1, int day1, int year2, int month2, int day2, uint32_t itemCat);

  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

  std::string toString(int level = 0) const override;

protected:
  int _year1, _month1, _day1;
  int _year2, _month2, _day2;
  static constexpr int ANY_YEAR = -1;
  uint32_t _itemCat;
};

class IsDateBetween : public Predicate
{
public:
  IsDateBetween();

  void initialize(int fromYear, int fromMonth, int fromDay, int toYear, int toMonth, int toDay);
  void initialize(int fromYear,
                  int fromMonth,
                  int fromDay,
                  int toYear,
                  int toMonth,
                  int toDay,
                  uint32_t itemCat);

  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

  std::string toString(int level = 0) const override;

protected:
  virtual bool isWholeRangeBetween(const TravelSeg* seg) const;

protected:
  static constexpr int ANY_YEAR = -1;
  int _fromYear, _fromMonth, _fromDay;
  int _toYear, _toMonth, _toDay;
  uint32_t _itemCat;
};

class IsDateRangeBetween : public IsDateBetween
{
public:
  IsDateRangeBetween();

protected:
  bool isWholeRangeBetween(const TravelSeg* seg) const override;
};

} // namespace tse

