// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "DBAccess/Nation.h"
#include "Common/Money.h"
#include "Common/LocUtil.h"
#include "Taxes/LegacyTaxes/LocationDescriptionUtil.h"
#include <sstream>
#include <vector>
#include <string>
#include <iostream>

using namespace tse;
using namespace std;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function  Decode Nation Description
//
// Description:  Nation Decoder
//
// </PRE>
// ----------------------------------------------------------------------------

std::string
LocationDescriptionUtil::description(TaxTrx& taxTrx, LocType locType, LocCode locCode)
{
  switch (locType)
  {
  case LOCTYPE_NATION:
  {
    return nationDescription(taxTrx, locCode);
  }

  case LOCTYPE_STATE:
  case LOCTYPE_CITY:
  {
    return locCode;
  }

  case LOCTYPE_ZONE:
  {
    return zoneDescription(taxTrx, locCode);
  }
  case LOCTYPE_SUBAREA:
  {
    return subAreaDescription(taxTrx, locCode);
  }

  case LOCTYPE_AREA:
  {
    return areaDescription(taxTrx, locCode);
  }

  default:
    break;
  } // end of switch
  return EMPTY_STRING();
}

std::string
LocationDescriptionUtil::areaDescription(TaxTrx& taxTrx, LocCode areaCode)
{
  return "AREA " + areaCode;
};

std::string
LocationDescriptionUtil::subAreaDescription(TaxTrx& taxTrx, LocCode subAreaCode)
{
  return "SUB AREA " + subAreaCode;
};

// ----------------------------------------------------------------------------
// <PRE>
//
// @function  Decode Nation Description
//
// Description:  Nation Decoder
//
// </PRE>
// ----------------------------------------------------------------------------

std::string
LocationDescriptionUtil::nationDescription(TaxTrx& taxTrx, LocCode nationCode)
{
  const std::vector<Nation*> allNationList =
      taxTrx.dataHandle().getAllNation(taxTrx.getRequest()->ticketingDT());

  std::vector<Nation*>::const_iterator nationsListIter = allNationList.begin();
  std::vector<Nation*>::const_iterator nationsListEnd = allNationList.end();

  if (allNationList.empty())
  {
    return EMPTY_STRING();
  }

  for (; nationsListIter != nationsListEnd; nationsListIter++)
  {
    if (nationCode != (*nationsListIter)->nation())
      continue;

    return (*nationsListIter)->description();
  }
  return EMPTY_STRING();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function  zoneDescription
//
// Description:  Zone Descriptor
//
// </PRE>
// ----------------------------------------------------------------------------

std::string
LocationDescriptionUtil::zoneDescription(TaxTrx& taxTrx,
                                         LocCode zoneCode,
                                         const VendorCode& vendorCode,
                                         const ZoneType& zoneType)
{
  DateTime date = taxTrx.getRequest()->ticketingDT();

  if (taxTrx.getRequest()->travelDate() != DateTime::emptyDate())
  {
    date = (taxTrx.getRequest()->travelDate());
  }

  Zone zoneNo = zoneCode;

  // if the zone is less than seven characters, then
  // pad the front of it with '0' characters
  if (zoneNo.size() < 7)
  {
    const int diff = 7 - int(zoneNo.size());
    for (int n = 6; n >= 0; --n)
    {
      if (n - diff < 0)
      {
        zoneNo[n] = '0';
      }
      else
      {
        zoneNo[n] = zoneNo[n - diff];
      }
    }
  }

  const ZoneInfo* zoneInfo = taxTrx.dataHandle().getZone(vendorCode, zoneNo, zoneType, date);
  if (zoneInfo == nullptr)
    return EMPTY_STRING();

  std::string locDescription = LocationDescriptionUtil::zoneDecode(taxTrx, zoneInfo, false);
  std::string locDescriptionExcl = LocationDescriptionUtil::zoneDecode(taxTrx, zoneInfo, true);

  if (locDescriptionExcl != EMPTY_STRING())
  {
    if (locDescription == EMPTY_STRING() || locDescription == "ANYWHERE")
      locDescription = "ANYWHERE EXCEPT " + locDescriptionExcl;
    else
      locDescription += " EXCEPT " + locDescriptionExcl;
  }

  return locDescription;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function  zoneDecode
//
// Description:  Zone Decoder
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
LocationDescriptionUtil::zoneDecode(TaxTrx& taxTrx, const ZoneInfo* zoneInfo, bool isExcl)
{
  uint16_t count = 0;
  std::string locIncludeDescription;
  std::string locDescription;

  char inclExclInd;
  if (isExcl)
    inclExclInd = 'E';
  else
    inclExclInd = 'I';

  std::vector<std::vector<ZoneInfo::ZoneSeg> >::const_iterator i = zoneInfo->sets().begin();

  for (; i != zoneInfo->sets().end(); ++i)
  {
    std::vector<ZoneInfo::ZoneSeg>::const_iterator j = i->begin();
    for (; j != i->end(); ++j)
    {
      if (j->locType() != LOCTYPE_AREA)
        continue;

      if (j->inclExclInd() == inclExclInd)
      {
        if (!locIncludeDescription.empty())
        {
          locIncludeDescription += ", ";
        }
        else
        {
          locIncludeDescription += "AREA ";
        }
        locIncludeDescription += j->loc();
        count += atoi(j->loc().c_str());
      }
    }

    if (count == 6 && !isExcl)
      locIncludeDescription = "ANYWHERE";
    else
    {
      j = i->begin();
      for (; j != i->end(); ++j)
      {
        if (j->locType() != LOCTYPE_NATION)
          continue;

        if (j->inclExclInd() == inclExclInd)
        {
          if (!locIncludeDescription.empty())
            locIncludeDescription += ", ";

          locIncludeDescription += nationDescription(taxTrx, j->loc());
          // locIncludeDescription += j->loc();
        }
      }

      j = i->begin();
      for (; j != i->end(); ++j)
      {
        if (j->locType() != LOCTYPE_CITY)
          continue;

        if (j->inclExclInd() == inclExclInd)
        {
          if (!locIncludeDescription.empty())
            locIncludeDescription += ", ";

          locIncludeDescription += j->loc();
        }
      }
      locDescription += locIncludeDescription;
    }
  }
  return locDescription;
}
