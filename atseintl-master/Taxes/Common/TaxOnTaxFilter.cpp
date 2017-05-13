//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Common/FallbackUtil.h"
#include "Common/Vendor.h"
#include "DataModel/FarePath.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/Loc.h"
#include "Taxes/Common/TaxOnTaxFilter.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

using namespace tse;


TaxOnTaxFilter&
TaxOnTaxFilter::operator =(const TaxOnTaxFilter& src)
{
  _loc1Type = src._loc1Type;
  _loc1Code = src._loc1Code;
  _taxCode = src._taxCode;
  _ticketingDT = src._ticketingDT;

  _enable = !_taxCode.empty() && !_loc1Code.empty();

  return *this;
}

bool
TaxOnTaxFilter::isFilteredItem(const TaxResponse& taxResponse,
                               const TaxItem& item) const
{
  if (!_enable)
    return false;

  if (_taxCode != item.taxCode())
    return false;

  const uint16_t& endIndex = item.travelSegEndIndex();
  const Itin* itin = taxResponse.farePath()->itin();

  for (uint16_t index = item.travelSegStartIndex(); index <= endIndex; index++)
  {
    if (isFilteredSegment(*itin->travelSeg()[index], item.taxCode()))
      return true;
  }

  return false;
}

bool
TaxOnTaxFilter::isFilteredSegment(const TravelSeg& seg, const TaxCode& code) const
{
  if (!_enable)
    return false;

  if (_taxCode != code)
    return false;

  return !LocUtil::isInLoc(*seg.origin(),
                           _loc1Type,
                           _loc1Code,
                           Vendor::SABRE,
                           MANUAL,
                           LocUtil::TAXES,
                           GeoTravelType::International,
                           EMPTY_STRING(),
                           _ticketingDT);
}
