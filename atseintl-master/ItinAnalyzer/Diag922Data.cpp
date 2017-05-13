#include "ItinAnalyzer/Diag922Data.h"

#include "DataModel/ShoppingTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"
#include "Diagnostic/Diag922Collector.h"
#include "Diagnostic/DCFactory.h"
#include "ItinAnalyzer/ItinAnalyzerUtils.h"
#include "Util/BranchPrediction.h"

namespace tse
{
namespace
{
/**
     * To do not display duplicate fare market skip message
     * we count fare market+skip reason occurrences instead
     */
class FMToDiagUniqueList
{
public:
  using FMMap = std::tr1::unordered_map<std::string, int>; // int as instance count
  using value_type = FMMap::value_type;
  using const_iterator = FMMap::const_iterator;

  void add(const ShoppingTrx& trx, const FareMarket* fm, const std::string& msg)
  {
    std::ostringstream ss;
    Diag922Collector::printFareMarket(ss, trx, *fm);
    if (!msg.empty())
      ss << "reason: " << msg << "\n";

    std::pair<FMMap::iterator, bool> insResult(_uniqFM.insert(std::make_pair(ss.str(), 1)));
    if (!insResult.second) // if element already exists,
      ++insResult.first->second; // update instance count
  }

  const_iterator begin() const { return _uniqFM.begin(); }
  const_iterator end() const { return _uniqFM.end(); }

private:
  FMMap _uniqFM;
};
}

int
Diag922Data::totalSkippedFM() const
{
  return _skippedFM.size() + _skippedFMByMileage.size();
}

void
Diag922Data::printSkippedFM(Diag922Collector& dc) const
{
  if (totalSkippedFM() == 0)
    return;

  //
  // accumulate statistics
  //
  FMToDiagUniqueList uniqueList; // one more map to avoid print the same FM+skip reason multiple
  // times

  using CStrReasonToNumMap =
      std::tr1::unordered_map<std::string, // std::string is because of overloaded operator<
                              int>;
  CStrReasonToNumMap fmCountBySkipCstrReason;
  for (const SkippedFM& fm : _skippedFM)
  {
    const std::string& key(fm.second);
    CStrReasonToNumMap::iterator found = fmCountBySkipCstrReason.find(key);
    if (found != fmCountBySkipCstrReason.end())
      found->second++; // increase count
    else
      fmCountBySkipCstrReason[key] = 1;
    uniqueList.add(_trx, fm.first, fm.second);
  }

  const size_t fmCountSkippedByConnectingCityMileage = _skippedFMByMileage.size();
  for (const SkippedFM& fm : _skippedFMByMileage)
  {
    uniqueList.add(_trx, fm.first, fm.second);
  }

  //
  // print statistics
  //
  dc << "# of skipped per specified reason:\n";
  // when SkipReason is const char*
  for (CStrReasonToNumMap::value_type fmCountBySkipReasonIt : fmCountBySkipCstrReason)
  {
    dc << std::setw(4) << std::right << fmCountBySkipReasonIt.second << " "
       << fmCountBySkipReasonIt.first << "\n";
  }
  // when SkipReason is from SkipFareMarketByMileageSOL
  if (fmCountSkippedByConnectingCityMileage != 0)
  {
    dc << std::setw(4) << std::right << fmCountSkippedByConnectingCityMileage << " "
       << "skipped by connecting city mileage"
       << "\n";
  }

  dc << "====== Skipped Fare Markets =======\n\n";
  dc.printFareMarketHeader(_trx);

  for (const auto& elem : uniqueList)
  {
    dc << "\n";
    dc << elem.first;
    if (elem.second > 1)
      dc << "(" << (elem.second - 1) << " more occurrence(s))\n";
  }
}

void
Diag922Data::reportFareMktToDiag(const FareMarket* fareMarket,
                                 const std::string& skipReason,
                                 const bool skippedByMileage)
{
  if (UNLIKELY(Diagnostic922 == _trx.diagnostic().diagnosticType()))
  {
    if (skippedByMileage)
      _skippedFMByMileage.push_back(SkippedFM(fareMarket, skipReason));
    else
      _skippedFM.push_back(SkippedFM(fareMarket, skipReason));
  }
}

void
Diag922Data::reportFareMktToDiag(const Itin* itin,
                                 const std::vector<TravelSeg*>& fmts,
                                 int legIdx,
                                 const GovCxrGroupParameters& params,
                                 const std::string& skipReason)
{
  if (UNLIKELY(Diagnostic922 == _trx.diagnostic().diagnosticType()))
  {
    FareMarket* fareMarket = itinanalyzerutils::buildFareMkt(_trx, itin, fmts, legIdx, params);
    _skippedFM.push_back(SkippedFM(fareMarket, skipReason));
  }
}

void
Diag922Data::printSkippedFareMarkets()
{
  if (UNLIKELY(Diagnostic922 == _trx.diagnostic().diagnosticType()))
  {
    Diag922Collector* diag = dynamic_cast<Diag922Collector*>(DCFactory::instance()->create(_trx));
    if (diag != nullptr)
    {
      diag->enable(Diagnostic922);
      *diag << "#skipped fare markets " << totalSkippedFM() << '\n';
      printSkippedFM(*diag);
      diag->flushMsg();
    }
  }
}
}
