#include "FareCalc/FareUsageIter.h"

#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "FareCalc/FcUtil.h"

namespace tse
{
void
FareUsageIter::init(const FarePath& fp)
{
  std::vector<FareUsage*> fuVect;
  FareCalc::forEachFareUsage(_fp, FareCalc::collect(fuVect));

  init(fuVect);
}

void
FareUsageIter::init(const std::vector<PricingUnit*>& sideTrip)
{
  std::vector<FareUsage*> fuVect;
  FareCalc::forEachFareUsage(sideTrip, FareCalc::collect(fuVect));

  init(fuVect);
}

void
FareUsageIter::init(std::vector<FareUsage*>& fuVect)
{
  std::map<const TravelSeg*, FareUsage*> tsMap;
  for (std::vector<FareUsage*>::const_iterator i = fuVect.begin(), iend = fuVect.end(); i != iend;
       ++i)
  {
    tsMap.insert(std::make_pair((*i)->travelSeg().front(), *i));
  }

  const std::vector<TravelSeg*>& tsVect = _fp.itin()->travelSeg();
  for (const auto ts : tsVect)
  {
    std::map<const TravelSeg*, FareUsage*>::iterator tsIter = tsMap.find(ts);

    if (tsIter != tsMap.end())
    {
      _fuv.push_back(tsIter->second);
    }
  }
}
}
