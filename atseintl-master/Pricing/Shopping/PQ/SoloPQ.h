// vim:ts=2:sts=2:sw=2:cin:et
// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Diagnostic/Diag910Collector.h"
#include "Pricing/Shopping/PQ/DiagSoloPQCollector.h"
#include "Pricing/Shopping/PQ/SoloPQItem.h"

#include <boost/noncopyable.hpp>
#include <boost/heap/priority_queue.hpp>

#include <limits>
#include <queue>
#include <vector>

namespace tse
{
class FarePath;
class Logger;
class ShoppingTrx;
}

namespace tse
{

class ItinStatistic;
class DiversityModel;

namespace shpq
{

class SoloTrxData;

class SoloPQ : private boost::noncopyable
{
public:
  explicit SoloPQ(ShoppingTrx& trx, const ItinStatistic& stats, DiagCollector* diag942);

  ~SoloPQ();

  void setDiversityModel(DiversityModel* dm) { _dm = dm; }

  /**
   *  Standard priority queue API
   */
  //!@{
  void enqueue(const SoloPQItemPtr& item, const SoloPQItem* const expandedFrom = nullptr);
  SoloPQItemPtr dequeue();
  SoloPQItemPtr peek() const;
  size_t size() const;
  bool empty() const;
  //!@}

  DiagSoloPQCollector& diagCollector() { return _diagCollector; }

  /**
   *  Expand items on queue until next FarePath is produced
   */
  SoloPQItemPtr getNextFarepathCapableItem(SoloTrxData& soloTrxData);
  void incrementFailedFPs() { _failedFPs++; }

  bool isOnlyThruFM() const { return _onlyThruFM; }
  void setOnlyThruFM(bool value) { _onlyThruFM = value; }

  const ItinStatistic& getItinStats() { return _stats; }

private:
  bool skipLocalPattern(const SoloPQItemPtr& item);
  bool checkHurryOut();
  bool checkOBSchedules();
  bool checkNotUsedCondition();
  bool checkFailedFPs();
  void checkDirectFlightsTuning(const SoloPQItemPtr& item);
  bool isThroughFarePrecedenceCompatible(const SoloPQItem& item);

private:
  typedef boost::heap::priority_queue<SoloPQItemPtr,
    boost::heap::compare<SoloPQItem::SoloPQItemComparator> > PQType;

  ShoppingTrx& _trx;
  PQType _pq;
  DiagSoloPQCollector _diagCollector;
  DiversityModel* _dm = nullptr;
  bool _onlyThruFM = false;
  const time_t _hurryOutTime;
  const size_t _uniqueOBSchedules;
  const size_t _maxNotUsedFPs;
  const size_t _maxFailedFPs;
  size_t _notUsedFPs = 0;
  size_t _failedFPs = 0;
  size_t _lastOptionsCount = std::numeric_limits<size_t>::max();
  const ItinStatistic& _stats;
  Diag910Collector* _diag910 = nullptr;

}; // class PQ
}
} // namespace tse::shpq

