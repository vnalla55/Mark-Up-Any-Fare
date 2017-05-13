#pragma once

#include "Common/Assert.h"
#include "DataModel/ShoppingTrx.h"

namespace tse
{

class DiagCollector;

namespace shpq
{

class SoloAltDatesInfo
{
public:
  SoloAltDatesInfo(ShoppingTrx& trx);

  void addSolution(const DatePair* const datePair)
  {
    if (_altDates && datePair)
    {
      std::pair<std::map<DatePair, size_t>::iterator, bool> insResult =
          _altDatePairsSolutionsFound.insert(std::make_pair(*datePair, 1));
      if (!insResult.second)
        insResult.first->second++;

      --(_trx.altDatePairs()[*datePair])->numOfSolutionNeeded;
    }
  }

  friend DiagCollector& operator<<(DiagCollector&, const SoloAltDatesInfo&);

private:
  ShoppingTrx& _trx;
  const bool _altDates;
  const bool _displayDiag941All;
  std::map<DatePair, size_t> _altDatePairsSolutionsFound;
};
}
} // namespace tse::shpq

