#include "Pricing/Shopping/PQ/SoloFMPathKey.h"

#include "DataModel/FareMarket.h"
#include "DBAccess/Loc.h"
#include "Pricing/Shopping/PQ/CxrFMCollector.h"

#include <algorithm>

namespace tse
{
namespace shpq
{

struct LocLess
{
  bool operator()(const Loc* lhp, const Loc* rhp)
  {
    if (lhp->loc() < rhp->loc())
      return true;
    else if (lhp->loc() == rhp->loc())
      if (UNLIKELY(lhp->cityInd() < rhp->cityInd()))
        return true;
    return false;
  }
};

MergedFMKey::MergedFMKey(const FareMarket* fm)
  : _origin(fm->origin()),
    _destination(fm->destination()),
    _carrier(fm->governingCarrier()),
    _fmType(fm->getFmTypeSol())
{
}

bool
MFMKeyLess::
operator()(const MergedFMKey& lhp, const MergedFMKey& rhp) const
{
  const Loc* lhpOrigin = lhp.origin();
  const Loc* rhpOrigin = rhp.origin();
  LocLess locLess;

  if (locLess(lhpOrigin, rhpOrigin))
    return true;
  else if (*lhpOrigin == *rhpOrigin)
  {
    const Loc* lhpDest = lhp.destination();
    const Loc* rhpDest = rhp.destination();
    if (locLess(lhpDest, rhpDest))
      return true;
    else if (*lhpDest == *rhpDest)
    {      
      if (lhp.carrier() != rhp.carrier())
        return lhp.carrier() < rhp.carrier();
      return lhp.fmType() < rhp.fmType();
    }
  }
  return false;
}

SoloFMPathKey::SoloFMPathKey() {}

SoloFMPathKey::SoloFMPathKey(const FareMarket* fm) { addFareMarket(fm); }

SoloFMPathKey::SoloFMPathKey(const FareMarket* fm1, const FareMarket* fm2)
{
  addFareMarket(fm1);
  addFareMarket(fm2);
}

void
SoloFMPathKey::addFareMarket(const FareMarket* fm)
{
  if (LIKELY(fm))
    _mfmKeys.push_back(MergedFMKey(fm));
}

bool
SoloFMPathKeyLess::
operator()(const SoloFMPathKey& lhp, const SoloFMPathKey& rhp) const
{
  return std::lexicographical_compare(lhp.begin(), lhp.end(), rhp.begin(), rhp.end(), MFMKeyLess());
}
}
} // namespace tse::shpq
