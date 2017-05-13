#include "Common/FareTypeDesignator.h"

namespace tse
{

void
FareTypeDesignator::setFareTypeDesignator(int ftd)
{
  switch (ftd)
  {
  case FTD_PREMIUM_FIRST:
    _fareTypeDesignator = FTD_PREMIUM_FIRST;
    break;

  case FTD_FIRST:
    _fareTypeDesignator = FTD_FIRST;
    break;

  case FTD_PREMIUM_BUSINESS:
    _fareTypeDesignator = FTD_PREMIUM_BUSINESS;
    break;

  case FTD_BUSINESS:
    _fareTypeDesignator = FTD_BUSINESS;
    break;

  case FTD_PREMIUM_ECONOMY:
    _fareTypeDesignator = FTD_PREMIUM_ECONOMY;
    break;

  case FTD_ECONOMY:
    _fareTypeDesignator = FTD_ECONOMY;
    break;

  case FTD_EXCURSION:
    _fareTypeDesignator = FTD_EXCURSION;
    break;

  case FTD_ONEWAY_ADVANCE_PURCHASE:
    _fareTypeDesignator = FTD_ONEWAY_ADVANCE_PURCHASE;
    break;

  case FTD_ROUNDTRIP_ADVANCE_PURCHASE:
    _fareTypeDesignator = FTD_ROUNDTRIP_ADVANCE_PURCHASE;
    break;

  case FTD_ONEWAY_INSTANT_PURCHASE:
    _fareTypeDesignator = FTD_ONEWAY_INSTANT_PURCHASE;
    break;

  case FTD_ROUNDTRIP_INSTANT_PURCHASE:
    _fareTypeDesignator = FTD_ROUNDTRIP_INSTANT_PURCHASE;
    break;

  case FTD_SPECIAL:
    _fareTypeDesignator = FTD_SPECIAL;
    break;

  case FTD_ADDON:
    _fareTypeDesignator = FTD_ADDON;
    break;

  case FTD_PROMOTIONAL:
    _fareTypeDesignator = FTD_PROMOTIONAL;
    break;

  default:
    _fareTypeDesignator = FTD_UNKNOWN;
  }
}
}
