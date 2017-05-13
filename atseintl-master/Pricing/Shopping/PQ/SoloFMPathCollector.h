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

#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/PQ/SoloFMPathKey.h"

#include <map>
#include <memory>
#include <vector>

namespace tse
{
class FareMarketPath;
class Itin;
class MergedFareMarket;
class ShoppingTrx;
}

namespace tse
{
namespace shpq
{

class ConxRouteCxrPQItem;
class OwrtFareMarket;

class SoloFMPathCollector
{
  typedef std::shared_ptr<OwrtFareMarket> OwrtFareMarketPtr;
  typedef std::vector<OwrtFareMarketPtr> OwrtFMVector;
  typedef std::map<MergedFMKey, MergedFareMarket*, MFMKeyLess> MergedFMMap;

public:
  typedef std::vector<MergedFareMarket*> MergedFareMktVector;
  typedef std::pair<FareMarketPath*, Itin*> FMPathItinPair;
  typedef std::map<SoloFMPathKey, FMPathItinPair, SoloFMPathKeyLess> FMPathItinMap;

  SoloFMPathCollector(ShoppingTrx& trx) : _trx(trx) {}

  FMPathItinPair getOrCreateFareMarketPath(const ConxRouteCxrPQItem* const);

private:
  FMPathItinPair buildFareMarketPath(OwrtFMVector&);
  void addPaxTypeFare(OwrtFareMarketPtr fromFM, MergedFareMarket* toMFM);
  bool addOwrtFareMarket(OwrtFareMarketPtr, MergedFareMktVector&);
  void setItin(Itin&);

private:
  ShoppingTrx& _trx;

  MergedFMMap _mergedFMMap;
  FMPathItinMap _fmPathItinMap;
};
}
} // namespace tse::shpq

