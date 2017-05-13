// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         04-03-2013
//! \file         ShoppingResponseQ4QGrouping.cpp
//! \brief
//!
//!  Copyright (C) Sabre 2013
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
//
// -------------------------------------------------------------------

#include "Xform/ShoppingResponseQ4QGrouping.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/ValidatingCxrUtil.h"
#include "DataModel/ShoppingTrx.h"

namespace tse
{
Logger
ShoppingResponseQ4QGrouping::_logger("atseintl.Xform.ShoppingResponseQ4QGrouping");

FALLBACK_DECL(fallbackValidatingCxrGTC);

namespace details
{
ConfigQ4Q::ConfigQ4Q()
  : _initialized(false),
    _groupItin(true),
    _groupInterlineItin(true),
    _maxGroup(-1),
    _groupMaxItin(-1)
{
}

void
ConfigQ4Q::initialize(const ConfigMan& config, const ShoppingTrx& shoppingTrx, Logger& logger)
{
  if (_initialized)
    return;

  _initialized = true;
  char groupItin = 'Y';
  if (!config.getValue("GROUP_ITINS", groupItin, "SHOPPING_OPT"))
  {
    CONFIG_MAN_LOG_KEY_ERROR(logger, "GROUP_ITINS", "SHOPPING_OPT");
  }
  _groupItin = groupItin == 'Y';

  char groupInterlineItin = 'Y';
  if (!config.getValue("GROUP_INTERLINE_ITINS", groupInterlineItin, "SHOPPING_OPT"))
  {
    CONFIG_MAN_LOG_KEY_ERROR(logger, "GROUP_INTERLINE_ITINS", "SHOPPING_OPT");
  }
  _groupInterlineItin = groupInterlineItin == 'Y';

  if (!config.getValue("MAX_GROUP", _maxGroup, "SHOPPING_OPT"))
  {
    CONFIG_MAN_LOG_KEY_ERROR(logger, "MAX_GROUP", "SHOPPING_OPT");
  }

  // altdate and calendar will not use multi_pax_max_itn for multipax for now
  if (!shoppingTrx.isAltDates())
  {
    if (shoppingTrx.paxType().size() + shoppingTrx.excludedPaxType().size() > 1)
    {
      if (!config.getValue("MULTI_PAX_MAX_ITN", _groupMaxItin, "SHOPPING_OPT"))
      {
        CONFIG_MAN_LOG_KEY_ERROR(logger, "MULTI_PAX_MAX_ITN", "SHOPPING_OPT");
      }
    }
    else if (shoppingTrx.legs().size() > 2)
    {
      if (!config.getValue("COMPLEX_TRIP_MAX_ITN", _groupMaxItin, "SHOPPING_OPT"))
      {
        CONFIG_MAN_LOG_KEY_ERROR(logger, "COMPLEX_TRIP_MAX_ITN", "SHOPPING_OPT");
      }
    }
  }
}
} /* namespace details */

ShoppingResponseQ4QGrouping::ShoppingResponseQ4QGrouping(const ShoppingTrx* const shoppingTrx)
  : _shoppingTrx(shoppingTrx), _configQ4Q(), _carrierGroupIndex(0), _carrierGroupMap()
{
}

// assumption: mapKey contain carrier at the beginning
void
ShoppingResponseQ4QGrouping::updateMapKeyForAltDateTrx(const SopIndexVector& sops,
                                                       std::string& /* in-out*/ mapKey) const
{
  const ShoppingTrx& shoppingTrx(*_shoppingTrx);
  if (!shoppingTrx.isAltDates())
  {
    return;
  }

  uint32_t neededDates = 0u;

  switch (shoppingTrx.groupMethodForMip())
  {
  case ShoppingTrx::BLANK: // default = outboundDate Only
    mapKey.clear();
    neededDates = 1u;
    break;

  case ShoppingTrx::GROUPBYDATE: // outboundDate + inboundDate
    mapKey.clear();
    neededDates = 2u;
    break;

  case ShoppingTrx::GROUPBYCARRIERANDDATE: // carrier + outboundDate + inboundDate
    neededDates = 2u;
    break;
  }

  mapKey.reserve(mapKey.size() + 7 * neededDates); // 7 is sizeof(DDMMMYY)

  const size_t sopsSize = sops.size();
  for (uint32_t item = 0u; item != sopsSize && item != neededDates; ++item)
  {
    const TravelSeg* const tvlSeg =
        shoppingTrx.legs()[item].sop()[sops[item]].itin()->travelSeg().front();

    TSE_ASSERT(tvlSeg);

    const std::string& depDTString = tvlSeg->departureDT().dateToString(DDMMMYY, "");
    mapKey += depDTString;
  }
}

void
ShoppingResponseQ4QGrouping::updateMapKeyForGuaranteedTicketingCarriers(const SopIndexVector& sops,
                                                             std::string& /* in-out*/ mapKey) const
{
  if (fallback::fallbackValidatingCxrGTC(_shoppingTrx) ||
      !_shoppingTrx->isValidatingCxrGsaApplicable())
    return;

  TSE_ASSERT(_shoppingTrx);
  const tse::ShoppingTrx& shoppingTrx(*_shoppingTrx);
  const Itin* itin = shoppingTrx.legs().front().sop()[sops.front()].itin();

  std::vector<CarrierCode> marketingCxrs;
  ValidatingCxrUtil::getMarketingItinCxrs(*itin, marketingCxrs);
  if (ValidatingCxrUtil::isGTCCarriers(*_shoppingTrx, marketingCxrs))
    mapKey += "_g";
}

inline bool
ShoppingResponseQ4QGrouping::isInterlineGroupingForcedForAltDates() const
{
  return _shoppingTrx->isAltDates() &&
         (_shoppingTrx->groupMethodForMip() == ShoppingTrx::BLANK ||
          _shoppingTrx->groupMethodForMip() == ShoppingTrx::GROUPBYDATE ||
          _shoppingTrx->groupMethodForMip() == ShoppingTrx::GROUPBYCARRIERANDDATE);
}

uint32_t
ShoppingResponseQ4QGrouping::getFirstInterlineSopIndex(const CarrierCode& carrier,
                                                       const SopIndexVector& sops) const
{
  const tse::ShoppingTrx& shoppingTrx(*_shoppingTrx);
  const size_t sopsSize = sops.size();
  for (uint32_t sopId = 0u; sopId != sopsSize; ++sopId)
  {
    const Itin* const itin = shoppingTrx.legs()[sopId].sop()[sops[sopId]].itin();
    for (const auto elem : itin->travelSeg())
    {
      const AirSeg* const airSeg = elem->toAirSeg();

      if (airSeg && airSeg->segmentType() != Arunk && airSeg->marketingCarrierCode() != carrier)
      {
        return sopId;
      }
    }
  }
  return sopsSize;
}

inline int16_t
ShoppingResponseQ4QGrouping::findOrCreateGroupForKey(const SopIndexVector& sops,
                                                     const std::string& mapKey)
{
  std::pair<CarrierGroupMap::iterator, bool> insertStatus =
      _carrierGroupMap.insert(std::make_pair(mapKey, GroupItn()));

  GroupItn& groupItn = insertStatus.first->second;
  if (insertStatus.second) // new item
  {
    groupItn._groupSize = 1;
    // note: the comparison below is most probably incorrect (_carrierGroupIndex might be modified
    // in another place and may never be equal to _maxGroup)
    groupItn._groupNumber = (_configQ4Q.getMaxGroup() == _carrierGroupIndex) ? _carrierGroupIndex
                                                                             : ++_carrierGroupIndex;
  }
  else if (groupItn._groupSize ==
           _configQ4Q.getGroupMaxItin()) // existing item, but max itins in group reached
  {
    groupItn._groupSize = 1;
    groupItn._groupNumber = ++_carrierGroupIndex;
  }
  else // existing item
  {
    ++groupItn._groupSize;
  }
  return groupItn._groupNumber;
}

int16_t
ShoppingResponseQ4QGrouping::generateQ4Q(const SopIndexVector& sops, std::string& mapKey)
{
  TSE_ASSERT(_shoppingTrx);

  const tse::ShoppingTrx& shoppingTrx(*_shoppingTrx);

  // read configuration variables
  _configQ4Q.initialize(Global::config(), shoppingTrx, _logger);

  const CarrierCode& firstCarrier =
      dynamic_cast<AirSeg*>(
          shoppingTrx.legs().front().sop()[sops.front()].itin()->travelSeg().front())
          ->marketingCarrierCode();
  // note that if solution is fully online, fistInterlineSopIndex will be equal to sopSize
  const uint32_t fistInterlineSopIndex = getFirstInterlineSopIndex(firstCarrier, sops);

  const size_t sopsSize = sops.size();
  // for generating group index value, solution is considered as interline if any of its sops is
  // interline
  const bool useInterlineGroupsForIndexing =
      fistInterlineSopIndex < sopsSize // solution contains at least one interline sop
      || isInterlineGroupingForcedForAltDates(); // a piece of strange logic

  // for generating mapKey, solution is considered as interline if is interline for indexing and:
  // - the first sop is interline when interline grouping is enabled (i.e. validateInterline() was
  // called in previous version of this function)
  // - or any of its sops is interline when the interline groups is disabled
  const bool useInterlineGroupsForMapKey =
      useInterlineGroupsForIndexing &&
      (!_configQ4Q.isGroupInterlineItin() ||
       fistInterlineSopIndex < 1u); // the first sop of solution is interline

  // set mapKey (Q6Q attribute)
  mapKey = useInterlineGroupsForMapKey
               ? "_" + shoppingTrx.legs().front().sop()[sops.front()].governingCarrier()
               : static_cast<std::string>(firstCarrier);
  updateMapKeyForAltDateTrx(sops, mapKey);
  updateMapKeyForGuaranteedTicketingCarriers(sops, mapKey);

  // calculate and return group index value (Q4Q attribute, BTW. the attribute is documented in
  // schema
  // as `Carrier group', but in reality - unless configQ4Q contains non-default values - Q4Q is
  // just is an integer equivalent of Q6Q)
  const bool useItinGroups =
      useInterlineGroupsForIndexing ? _configQ4Q.isGroupInterlineItin() : _configQ4Q.isGroupItin();

  if (UNLIKELY(!useItinGroups))
  {
    return ++_carrierGroupIndex; // create new group
  }

  // otherwise find existing group or create new one (based on mapKey)
  return findOrCreateGroupForKey(sops, mapKey);
}
} /* namespace tse */
