#include "AddonConstruction/ConstructedFareInfoResponse.h"

#include "AddonConstruction/FareDup.h"
#include "Common/Global.h"
#include "Common/Hasher.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/HashKey.h"

namespace tse
{
namespace
{
size_t
hashCfi(const ConstructedFareInfo* cfi)
{
  Hasher hasher(Global::hasherMethod());
  hasher << cfi->fareInfo().fareTariff() << cfi->fareInfo().owrt()
         << cfi->fareInfo().globalDirection() << cfi->fareInfo().fareClass()
         << cfi->fareInfo().vendor() << cfi->fareInfo().carrier() << cfi->fareInfo().currency()
         << cfi->fareInfo().ruleNumber() << cfi->fareInfo().market1() << cfi->fareInfo().market2()
         << cfi->fareInfo().directionality();

  return hasher.hash();
}

bool
equalCfi(const ConstructedFareInfo* cf1, const ConstructedFareInfo* cf2)
{
  return FareDup::isEqual(*cf1, *cf2) == FareDup::EQUAL;
}

bool
equalCfiWithAddons(const ConstructedFareInfo* cf1, const ConstructedFareInfo* cf2)
{
  return FareDup::isEqualWithAddons(*cf1, *cf2) == FareDup::EQUAL;
}

bool
shouldCompareAddons(const PricingTrx& trx)
{
  if (trx.getTrxType() != PricingTrx::FAREDISPLAY_TRX)
    return false;

  // ERD contains restrictions on addons so we shouldn't merge constructed
  // fares that have different addon fares. Otherwise requested fare can be
  // filtered out in fare selector service.
  const FareDisplayTrx& fdTrx = static_cast<const FareDisplayTrx&>(trx);
  return fdTrx.isERD();
}
}

ConstructedFareInfoResponse::ConstructedFareInfoResponse(const PricingTrx& trx)
  : _responseHashSet(0, hashCfi, shouldCompareAddons(trx) ? equalCfiWithAddons : equalCfi)
{
}

} // end tse
