//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Common/TFPUtil.h"

#include "Common/Config/ConfigurableValue.h"
#ifdef CONFIG_HIERARCHY_REFACTOR
#include "Common/Config/DynamicConfigurableValue.h"
#else
#include "Common/Config/DynamicConfigurableValueImpl.h"
#endif
#include "Common/ConfigList.h"
#include "Common/FallbackUtil.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"

namespace tse
{
FALLBACK_DECL(dynamicThroughFarePrecedence)

namespace
{
ConfigurableValue<ConfigSet<CarrierCode>>
throughFarePrecedenceListOfCarriersOld("THROUGH_FARE_PRECEDENCE", "LIST_OF_CARRIERS");
ConfigurableValue<ConfigSet<LocCode>>
throughFarePrecedenceCountries("THROUGH_FARE_PRECEDENCE", "COUNTRIES");

#ifdef CONFIG_HIERARCHY_REFACTOR
DynamicConfigurableValue<ConfigSet<CarrierCode>>
throughFarePrecedenceListOfCarriers("THROUGH_FARE_PRECEDENCE", "LIST_OF_CARRIERS");
#else
struct EmptyConfigSet
{
  const ConfigSet<CarrierCode> operator()(const Trx*) { return {}; }
};

using ConfigSetDynCfg = DynamicConfigurableValueImpl<ConfigSet<CarrierCode>,
                                                     std::equal_to<ConfigSet<CarrierCode>>,
                                                     EmptyConfigSet>;

ConfigSetDynCfg
throughFarePrecedenceListOfCarriers("THROUGH_FARE_PRECEDENCE",
                                    "LIST_OF_CARRIERS",
                                    ConfigSet<CarrierCode>());
#endif

//TODO Inline when removing the fallback.
#ifdef CONFIG_HIERARCHY_REFACTOR
const ConfigSet<CarrierCode>&
#else
ConfigSet<CarrierCode>
#endif
getListOfCarriers(const Trx& trx)
{
  if (fallback::dynamicThroughFarePrecedence(&trx))
    return throughFarePrecedenceListOfCarriersOld.getValue();
  return throughFarePrecedenceListOfCarriers.getValue(&trx);
}
}

namespace TFPUtil
{
bool
isThroughFarePrecedencePossibleAtGeography(const std::vector<TravelSeg*>& segments)
{
  if (segments.empty())
    return false;

  const auto& countries = throughFarePrecedenceCountries.getValue();

  for (const auto ts : segments)
  {
    if (!countries.has(ts->origin()->nation()))
      return false;
    if (!countries.has(ts->destination()->nation()))
      return false;
  }

  return true;
}

bool
isThroughFarePrecedenceNeeded(const Trx& trx, const std::vector<TravelSeg*>& segments)
{
  if (segments.empty())
    return false;

  const auto& listOfCarriers = getListOfCarriers(trx);
  const auto& countries = throughFarePrecedenceCountries.getValue();

  for (const auto ts : segments)
  {
    const auto as = ts->toAirSeg();
    if (!as)
      continue;

    if (!countries.has(as->origin()->nation()))
      return false;
    if (!countries.has(as->destination()->nation()))
      return false;
    if (!listOfCarriers.has(as->carrier()))
      return false;
  }

  return true;
}

bool
isThroughFarePrecedenceNeededNGS(const Trx& trx, const FareMarket& fareMarket)
{
  if (!getListOfCarriers(trx).has(fareMarket.governingCarrier()))
    return false;

  const ApplicableSOP* appSOPs = fareMarket.getApplicableSOPs();
  if (!appSOPs)
    return false;

  for (const auto& appSop : *appSOPs)
  {
    for (const SOPUsage& sopUsage : appSop.second)
    {
      if (!sopUsage.itin_)
        continue;

      // If there is at least one SOP that might need through fare precedence,
      // flag this fare market.
      if (isThroughFarePrecedenceNeeded(trx, sopUsage.itin_->travelSeg()))
        return true;
    }
  }

  return false;
}
}
}
