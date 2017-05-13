#pragma once

#include <vector>

namespace tse
{

class FarePath;
class FareUsage;
class PricingTrx;
class PricingUnit;
class TravelSeg;

struct RemoveFakeTravelSeg
{
  static void process(PricingTrx& trx);

  static void removeFakePU(FarePath* fp, const TravelSeg* fakeTS);

  static std::vector<PricingUnit*>::iterator findFakePU(std::vector<PricingUnit*>& pu);

  static void removeFakeFU(FarePath* fp);

  static std::vector<FareUsage*>::iterator findFakeFU(std::vector<FareUsage*>& fu);

  static std::vector<FareUsage*>::iterator findFirstNonFakeFU(std::vector<FareUsage*>& fu);
};
}

