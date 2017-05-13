//----------------------------------------------------------------------------
//
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Diagnostic/TracksPrinter.h"

#include "Common/TrackCollector.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/DiagCollector.h"

#include <vector>

namespace tse
{

namespace
{
const std::string& ALL = "ALL";

struct IsSpecificFareMarket
{
  explicit IsSpecificFareMarket(const std::string& market) : _market(market) {}

  bool operator()(const FareMarket* fm) const
  {
    return _market == (fm->boardMultiCity() + fm->offMultiCity());
  }

private:
  std::string _market;
};
}

void
TracksPrinter::process()
{
  _diag.printLine();
  if (isParam(Diagnostic::FARE_MARKET))
  {
    print(_trx.fareMarket());
    return;
  }

  if (!_trx.itin().empty() && !_trx.itin().front()->farePath().empty())
    print(*_trx.itin().front()->farePath().front());
}

std::vector<PaxTypeFare*>
TracksPrinter::getPaxTypeFares(const FarePath& fp) const
{
  std::vector<PaxTypeFare*> ptfs;

  const std::vector<PricingUnit*>& pus = fp.pricingUnit();
  for (const auto pu : pus)
  {
   const std::vector<FareUsage*>& fus = pu->fareUsage();
    for (const auto fu : fus)
      ptfs.push_back(fu->paxTypeFare());
  }
  return ptfs;
}

void
TracksPrinter::print(const FarePath& fp)
{
  _diag.DiagCollector::operator<<(fp) << '\n';
  _diag.lineSkip(0);

  std::vector<PaxTypeFare*> ptfs = getPaxTypeFares(fp);

  if (isParam(Diagnostic::DISPLAY_DETAIL))
    printByLabel(ptfs);
  else
    print(ptfs, &TracksPrinter::printWithTracks);
}

void
TracksPrinter::print(const FareMarket& fm)
{
  _diag << "FAREMARKET: ";
  _diag.DiagCollector::operator<<(fm) << '\n';
  _diag.lineSkip(0);

  if (isParam(Diagnostic::DISPLAY_DETAIL))
    printByLabel(fm.allPaxTypeFare());
  else
    print(fm.allPaxTypeFare(), &TracksPrinter::printWithTracks);
}

void
TracksPrinter::printWithTracks(const PaxTypeFare& ptf)
{
#ifdef TRACKING
  if (ptf.tracks().empty())
    return;
#endif

  printWithoutTracks(ptf);

#ifdef TRACKING
  ptf.tracks().print(_diag);
#endif

  _diag.lineSkip(0);
}

void
TracksPrinter::printWithoutTracks(const PaxTypeFare& ptf)
{
  _diag.DiagCollector::operator<<(ptf) << '\n';
}

void
TracksPrinter::print(const std::vector<PaxTypeFare*>& ptfs, PaxTypeFarePrint ptfPrint)
{

  if (isParamAll(Diagnostic::FARE_CLASS_CODE))
  {
    for (const auto ptf : ptfs)
      (this->*ptfPrint)(*ptf);
    return;
  }

  const std::string& param = getParam(Diagnostic::FARE_CLASS_CODE);
  for (const auto ptf : ptfs)
    if (_diag.rootDiag()->matchFareClass(param, *ptf))
      (this->*ptfPrint)(*ptf);
}

void
TracksPrinter::print(const std::vector<FareMarket*>& fms)
{
  typedef std::vector<FareMarket*>::const_iterator It;

  if (isParamAll(Diagnostic::FARE_MARKET))
  {
    for (const auto fm : fms)
      print(*fm);
    return;
  }

  It market =
      std::find_if(fms.begin(), fms.end(), IsSpecificFareMarket(getParam(Diagnostic::FARE_MARKET)));
  if (market != fms.end())
    print(**market);
}

void
TracksPrinter::printByLabel(const std::vector<PaxTypeFare*>& ptfs)
{
  Collection collection;
  recollect(ptfs, collection);
  print(collection);
}

const std::string&
TracksPrinter::getParam(const std::string& key) const
{
  return _diag.rootDiag()->diagParamMapItem(key);
}

bool
TracksPrinter::isParam(const std::string& key) const
{
  return _diag.rootDiag()->diagParamMapItemPresent(key);
}

bool
TracksPrinter::isParamAll(const std::string& key) const
{
  return !isParam(key) || getParam(key) == ALL;
}

void
TracksPrinter::print(const Collection& collection)
{
  typedef Collection::const_iterator It;
  if (isParamAll(Diagnostic::DISPLAY_DETAIL))
  {
    for (const auto& elem : collection)
      print(elem);
    return;
  }

  It i = collection.find(getParam(Diagnostic::DISPLAY_DETAIL));
  if (i != collection.end())
    print(*i);
}

void
TracksPrinter::print(const Collection::value_type& value)
{
  _diag << value.first << ":\n";
  print(value.second, &TracksPrinter::printWithoutTracks);
  _diag.lineSkip(0);
}

void
TracksPrinter::recollect(PaxTypeFare* ptf, Collection& collection) const
{
  std::vector<TrackCollector::Label> labels;

#ifdef TRACKING
  ptf->tracks().getLabels(labels);
#endif

  typedef std::vector<TrackCollector::Label>::const_iterator It;
  for (It i = labels.begin(); i != labels.end(); ++i)
    collection[*i].push_back(ptf);
}

void
TracksPrinter::recollect(const std::vector<PaxTypeFare*>& ptfs, Collection& collection) const
{
  for (const auto ptf : ptfs)
    recollect(ptf, collection);
}

} // tse
