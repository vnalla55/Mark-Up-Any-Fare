#include "FareCalc/FareAmountAdjustment.h"

#include "Common/Logger.h"
#include "DataModel/FarePath.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FcUtil.h"

#include <functional>
#include <iomanip>
#include <sstream>

using namespace tse::FareCalc;
namespace tse
{
static Logger
logger("atseintl.FareCalc.FareAmountAdjustment");

long int
fcToLongInt(const double value, const unsigned int& noDec)
{
  std::stringstream ss;
  ss << std::fixed << std::setprecision(noDec) << value;

  double dv;
  if (LIKELY(ss >> dv))
  {
    return (long int)rint(dv * pow(10.0, noDec));
  }
  return (long int)rint(value * pow(10.00, noDec));
}

bool
FareAmountAdjustment::process()
{
  const FareUsage* prevFu = nullptr;
  long int fareCompTotal = 0;

  const FarePath& farePath = *_calcTotals.farePath;

  for (const auto fu : _fuv)
  {
    if (UNLIKELY(fu == nullptr || fu == prevFu))
      continue;
    else
      prevFu = fu;

    fareCompTotal += fcToLongInt(fu->totalFareAmount() + fu->minFarePlusUp().getSum(HIP), 2);
  }

  long int farePathTotal = fcToLongInt(farePath.getTotalNUCAmount(), 2);
  unsigned int count = abs(farePathTotal - fareCompTotal);

  if (count == 0)
  {
    return false;
  }

  if (count > _fuv.size())
  {
    // Calc Error! Cannot adjust with this big difference
    LOG4CXX_DEBUG(logger,
                  "Not enough rt fu for adjustment - count: " << count << ", fus: " << _fuv.size());

    return false;
  }

  std::vector<const PricingUnit*> mirrorPus;
  std::for_each(farePath.pricingUnit().begin(),
                farePath.pricingUnit().end(),
                FareCalc::collect(
                    mirrorPus,
                    FareCalc::Equal<PricingUnit>(std::mem_fun(&PricingUnit::isMirrorImage), true)));

  if (mirrorPus.size() > 0 && mirrorPus.front()->puType() == PricingUnit::Type::ROUNDTRIP &&
      farePathTotal > fareCompTotal)
  {
    // Adjust the outbound fare comp from the mirror PUs
    _adjSubjects.clear();
    for (auto mirrorPu : mirrorPus)
    {
      if ((fcToLongInt(mirrorPu->getTotalPuNucAmount(), 2) % 2) == 0 &&
          (fcToLongInt(mirrorPu->getTotalPuNucAmount(), 3) % 2) == 0)
        continue;

      FareCalc::forEachFareUsage(
          *mirrorPu,
          FareCalc::collect(
              _adjSubjects,
              FareCalc::Equal<FareUsage>(std::mem_fun(&FareUsage::isOutbound), true)));
    }

    if (count > 0 && count <= _adjSubjects.size())
    {
      _adjType = ADJ_FARE_COMP;
      _adjAmount = count * 0.01;
    }
    else
    {
      // Calc Error! there is not enough FU to take the adjustment

      return false;
    }
  }
  else
  {
    // There is no mirror-image PU, adjust the total:
    _adjType = ADJ_TOTAL;

    if (farePathTotal > fareCompTotal)
    {
      _adjAmount = -(count * 0.01);
    }
    else
    {
      _adjAmount = count * 0.01;
    }
  }

  return true;
}
}
