//-------------------------------------------------------------------
//
//  File:        GroupingPreferenceSection.cpp
//  Description: This class abstracts a display template.  It maintains
//               all the data and methods necessary to describe
//               and realize the Flight Counts that
//               appear on the Sabre greenscreen.)
//
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/GroupingPreferenceSection.h"

#include "Common/TseEnums.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/Itin.h"
#include "DBAccess/Brand.h"
#include "DBAccess/BrandedFareApp.h"
#include "DBAccess/Customer.h"
#include "FareDisplay/BrandDataRetriever.h"
#include "FareDisplay/GlobalDirectionComparator.h"
#include "FareDisplay/Group.h"
#include "FareDisplay/GroupingDataRetriever.h"
#include "FareDisplay/StrategyBuilder.h"

#include <sstream>
#include <string>

namespace tse
{
GroupingPreferenceSection::GroupingPreferenceSection(FareDisplayTrx& trx) : Section(trx)
{
  _groupTextMap.insert(std::make_pair(Group::GROUP_BY_GLOBAL_DIR, "GROUP BY GLOBAL DIRECTION"));
  _groupTextMap.insert(std::make_pair(Group::GROUP_BY_ROUTING, "GROUP BY ROUTING NUMBER"));
  _groupTextMap.insert(std::make_pair(Group::GROUP_BY_OW_RT, "GROUP BY OWRT"));
  _groupTextMap.insert(std::make_pair(Group::GROUP_BY_PSG_TYPE, "GROUP BY PASSENGR TYPE "));
  _groupTextMap.insert(std::make_pair(Group::GROUP_BY_FARE_BASIS_CODE, "GROUP BY FARE BASIS "));
  _groupTextMap.insert(
      std::make_pair(Group::GROUP_BY_FARE_BASIS_CHAR_COMB, "GROUP BY FARE BASIS CHAR COMB "));
  _groupTextMap.insert(std::make_pair(Group::GROUP_BY_NORMAL_SPECIAL, "GROUP BY NORMAL SPECIAL"));
  _groupTextMap.insert(std::make_pair(Group::GROUP_BY_PUBLIC_PRIVATE, "GROUP BY PUBLIC PRIVATE"));
  _groupTextMap.insert(std::make_pair(Group::GROUP_BY_FARE_AMOUNT, "GROUP BY FARE AMOUNT"));
  _groupTextMap.insert(
      std::make_pair(Group::GROUP_BY_TRAVEL_DISCONTINUE_DATE, "GROUP BY TRAVEL DISCONTINUE DATE "));
}

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  GroupingPreferenceSection::buildDisplay()
//
// This is the main processing method of the GroupingPreferenceSection class.
// It requires a reference to the Fare Display Transaction. When
// called, it first iterates through the collection of fields,
// and displays the column headers.  It then iterates through each
// of the collected fares and for each one, iterates through the
// collection of fields maintained by the GroupingPreferenceSection object.
//
//
//
// @param   field   - a reference to a FareDisplayTrx.
//
// @return  void.
//
// </PRE>
// -------------------------------------------------------------------------
void
GroupingPreferenceSection::buildDisplay()
{
  _trx.response().clear();
  std::vector<Group*> groups;

  // Check for brand grouping info
  if (FareDisplayUtil::isBrandGrouping(_trx))
  {
    BrandDataRetriever brandRetriever(_trx);
    brandRetriever.getBrandData(groups);

    if (!groups.empty())
    {
      if (!FareDisplayUtil::isBrandServiceEnabled())
      {
        displayBrandGroupingInfo(*groups.front(), _trx);
      }
      else
      {
        displayBrandServiceGroupingInfo(*groups.front(), _trx);
      }
      groups.clear();
    }
  }

  // Check for grouping and sorting info
  GroupingDataRetriever retriever(_trx);
  retriever.getGroupAndSortPref(groups);
  if (!groups.empty())
  {
    displayUserInfo(*groups.front(), _trx);
    std::vector<Group*>::iterator i(groups.begin()), end(groups.end());
    uint16_t counter(0);
    for (; i != end; ++i)
    {
      counter++;
      displayGroupInfo(**i, _trx, counter);
    }
    // Display Legend
    _trx.response() << std::endl;
    _trx.response() << " "
                    << "\n";
    _trx.response() << "A - ASCENDING IS LOW-TO-HIGH OR A-TO-Z "
                    << "\n";
    _trx.response() << "D - DESCENDING IS HIGH-TO-LOW OR Z-TO-A " << std::endl;
  }
  else
  {
    _trx.response() << std::endl;
    _trx.response() << " NO GROUP/SORT PREFERENCE FOUND : USE DEFAULT";
  }
  _trx.response() << std::endl;
}

void
GroupingPreferenceSection::displayUserInfo(Group& group, FareDisplayTrx& trx) const
{
  trx.response() << std::endl;
  trx.response() << "************************************************************ ";
  trx.response() << std::endl;

  if (!group.sortData()->pseudoCity().empty())
  {
    if (group.sortData()->pseudoCityType() == 'T')
      trx.response() << " TVL AGENCY : " << group.sortData()->pseudoCity();
    else
      trx.response() << " HOME AGENCY : " << group.sortData()->pseudoCity();
  }
  else if (!group.sortData()->userAppl().empty())
  {
    if (group.sortData()->userApplType() == 'C')
      trx.response() << " CRS USER : " << group.sortData()->userAppl();
    else
      trx.response() << " MULTIHOST CXR :" << group.sortData()->userAppl();
  }
  else if (group.sortData()->ssgGroupNo() != 0)
  {
    trx.response() << " SSG GROUP : " << group.sortData()->ssgGroupNo();
  }

  GeoTravelType gtyp = (trx.itin().front()->geoTravelType());
  trx.response() << "  "
                 << "DOMINTAPPL : ";

  if ((gtyp == GeoTravelType::Domestic || gtyp == GeoTravelType::Transborder || gtyp == GeoTravelType::ForeignDomestic) &&
      !trx.isSameCityPairRqst())
    trx.response() << "D";
  else
    trx.response() << "I";

  trx.response() << "  "
                 << "FAREDISPTYPE : ";
  if (trx.preferredCarriers().size() == 1)
    trx.response() << "S";
  else
    trx.response() << "M";

  trx.response() << std::endl;
  trx.response() << "************************************************************ ";
  trx.response() << std::endl;
}

void
GroupingPreferenceSection::displayGroupInfo(Group& group, FareDisplayTrx& trx, uint16_t& counter)
    const
{
  trx.response() << std::endl;
  std::string groupDescription;
  std::map<Group::GroupType, std::string>::const_iterator i(_groupTextMap.end());
  i = _groupTextMap.find(group.groupType());
  if (i == _groupTextMap.end())
  {
    counter--;
    return;
    // TODO : HANDLE ERROR CONDITION
    groupDescription = " NO DESCRIPTION FOUND FOR THE GROUP ";
    trx.response() << counter << " . "
                   << " " << groupDescription;
    return;
  }
  else
  {
    trx.response() << counter << "."
                   << "SEQ NO "
                   << "-";
    trx.response() << group.sortData()->seqno() << std::endl;

    groupDescription = i->second;
    trx.response() << "    " << groupDescription << std::endl;
    trx.response() << "   "
                   << " SORT OPTION"
                   << " - ";
    if (group.sortType() == 'A')
      trx.response() << "ASCENDING";
    else if (group.sortType() == 'D')
      trx.response() << "DESCENDING";
    else if (group.sortType() == 'L')
    {
      trx.response() << "LIST  ";
      displayGroupDetails(group);
    }
    else
      trx.response() << group.sortType();
  }
}

void
GroupingPreferenceSection::displayGroupDetails(Group& group) const
{
  switch (group.groupType())
  {
  case Group::GROUP_BY_GLOBAL_DIR:
    displayGlobals(group);
    break;
  case Group::GROUP_BY_PSG_TYPE:
    displayPaxTypes(group);
    break;
  default:
    break;
  }
}

void
GroupingPreferenceSection::displayGlobals(Group& group) const
{
  GlobalDirectionComparator comparator;
  comparator.group() = &group;
  comparator.prepare(_trx);

  // Must display the globals by order number rather than alphabetical order
  // Build an order no/global multimap from the global/order no map
  // Note: Multimaps allow multiple items with the same key
  std::multimap<uint16_t, GlobalDirection> newMap;

  for (const auto& mapItem : comparator.priorityMap())
  {
    newMap.insert(std::make_pair(mapItem.second, mapItem.first));
  }

  int count = 0;
  for (const auto& globalDirectionMapItem : newMap)
  {
    std::string gdStr = *(tse::globalDirectionToStr(globalDirectionMapItem.second));
    _trx.response() << gdStr << " ";

    count++;
    if (count == 11) // 11 GD on 1 line
    {
      _trx.response() << "\n";
      _trx.response() << "                        ";
      count = 0;
    }
  }
}

void
GroupingPreferenceSection::displayBrandGroupingInfo(Group& group, FareDisplayTrx& trx) const
{
  if (group.brandedFareApps().empty())
  {
    return;
  }

  const BrandedFareApp* const front = group.brandedFareApps().front();
  Indicator userApplType = front->userApplType();
  UserApplCode userAppl = front->userAppl();
  CarrierCode carrier = front->carrier();

  trx.response() << std::endl;
  trx.response() << "***************************************************************" << std::endl;

  if (!userAppl.empty())
  {
    if (userApplType == 'C')
      trx.response() << "CRS USER : " << userAppl;
    else
      trx.response() << "MULTIHOST CXR :" << userAppl;
  }

  trx.response() << "   CARRIER : " << carrier << std::endl;
  trx.response() << "***************************************************************" << std::endl;

  trx.response() << "GROUP BY BRAND" << std::endl;

  // Loop thru all brandedFareApp items
  for (const auto groupBrandedFareApp : group.brandedFareApps())
  {
    trx.response() << "  *************************************************************"
                   << std::endl;
    trx.response() << "  BRANDEDFAREAPP SEQ NO " << groupBrandedFareApp->seqno() << std::endl;
    trx.response() << "  MARKET TYPE   - " << groupBrandedFareApp->marketTypeCode() << std::endl;
    trx.response() << "  DIRECTIONALITY- ";

    if (groupBrandedFareApp->directionality() == FROM)
      trx.response() << "FROM";
    else if (groupBrandedFareApp->directionality() == WITHIN)
      trx.response() << "WITHIN";
    else if (groupBrandedFareApp->directionality() == ORIGIN)
      trx.response() << "ORIGIN";
    else if (groupBrandedFareApp->directionality() == TERMINATE)
      trx.response() << "TERMINATE";
    else if (groupBrandedFareApp->directionality() == BETWEEN)
      trx.response() << "BETWEEN";

    trx.response() << std::endl;
    trx.response() << "  LOC TYPE 1    - ";

    switch (groupBrandedFareApp->loc1().locType())
    {
    case 'A':
      trx.response() << "AREA";
      break;
    case '*':
      trx.response() << "SUBAREA";
      break;
    case 'N':
      trx.response() << "NATION";
      break;
    case 'S':
      trx.response() << "NATION/STATE";
      break;
    case 'C':
      trx.response() << "MARKET";
      break;
    case 'Z':
      trx.response() << "ZONE";
      break;
    default:
      break;
    }

    trx.response() << std::endl;
    trx.response() << "  LOC 1         - " << groupBrandedFareApp->loc1().loc() << std::endl;
    trx.response() << "  LOC TYPE 2    - ";

    switch (groupBrandedFareApp->loc2().locType())
    {
    case 'A':
      trx.response() << "AREA";
      break;
    case '*':
      trx.response() << "SUBAREA";
      break;
    case 'N':
      trx.response() << "NATION";
      break;
    case 'S':
      trx.response() << "NATION/STATE";
      break;
    case 'C':
      trx.response() << "MARKET";
      break;
    case 'Z':
      trx.response() << "ZONE";
      break;
    default:
      break;
    }

    trx.response() << std::endl;
    trx.response() << "  LOC 2         - " << groupBrandedFareApp->loc2().loc() << std::endl;
  } // end loop

  // Brands & Booking Codes

  // Build a brand/bkg code multimap from the bkg code/brand map
  // Note: Multimaps allow multiple items with the same key
  std::multimap<BrandCode, BookingCode> newMap;

  for (const auto& brandMapItem : group.brandMap())
  {
    newMap.insert(make_pair(brandMapItem.second, brandMapItem.first));
  }

  // Get all brands for this carrier
  const std::vector<Brand*>& brands =
      trx.dataHandle().getBrands(userApplType, userAppl, carrier, trx.travelDate());

  std::pair<std::multimap<BrandCode, BookingCode>::iterator,
            std::multimap<BrandCode, BookingCode>::iterator> iterPair;

  BrandCode brandCode;
  int16_t bkgCodesLen = 0;

  for (const auto brand : brands)
  {
    // Get begin and end iterators of current brand
    iterPair = newMap.equal_range(brand->brandId());

    std::multimap<BrandCode, BookingCode>::const_iterator i;

    // Display this brand's info
    for (i = iterPair.first; i != iterPair.second; ++i)
    {
      // 1st time thru?
      if (i == iterPair.first)
      {
        std::string brandName = "UNDEFINED BRAND";
        std::string brandText = EMPTY_STRING();
        uint64_t seqno = 0;

        brandCode = i->first;

        const Brand* brand =
            trx.dataHandle().getBrand(userApplType, userAppl, carrier, brandCode, trx.travelDate());

        if (brand)
        {
          brandName = brand->brandName();
          brandText = brand->brandText();
          seqno = brand->seqno();
        }

        trx.response() << std::endl
                       << "  -------------------------------------------------------------"
                       << std::endl;
        trx.response() << "  ORDER NO " << seqno << std::endl;
        trx.response() << "  BRAND CD/NAME - " << brandCode << "/" << brandName << std::endl;
        trx.response() << "  BRAND TEXT    - " << brandText << std::endl;
        trx.response() << "  BKG CODES     - " << i->second; // 1st booking code

        bkgCodesLen = i->second.size();
      }
      else
      {
        if (bkgCodesLen > 40)
        {
          trx.response() << "," << std::endl;
          trx.response() << "                  " << i->second;
          bkgCodesLen = i->second.size();
        }
        else
        {
          trx.response() << ", " << i->second;
          bkgCodesLen += i->second.size() + 2;
        }
      }
    }
  }

  trx.response() << std::endl;
}

void
GroupingPreferenceSection::displayBrandServiceGroupingInfo(Group& group, FareDisplayTrx& trx) const
{
  if (group.brandResponseItemVec().empty())
  {
    return;
  }
  trx.response() << "***************************************************************" << std::endl;
  trx.response() << "MARKET: " << trx.travelSeg().front()->origAirport().c_str();
  trx.response() << " - " << trx.travelSeg().front()->destAirport().c_str();
  trx.response() << "     CARRIER: " << trx.requestedCarrier();
  trx.response() << "     TRAVEL DATE: " << trx.travelDate().dateToString(DDMMMYY, "").c_str()
                 << std::endl;
  trx.response() << "***************************************************************" << std::endl;
  trx.response() << "GROUP BY BRAND" << std::endl;
  trx.response() << "***************************************************************" << std::endl;

  // Brands & Booking Codes

  std::string bKGCodes;
  std::string inclFareBC;
  std::string exclFareBC;
  std::string separator = ", ";
  int16_t seqno = 0;
  int16_t lineLen = 18;

  for (const auto groupBrandResponseItem : group.brandResponseItemVec())
  {
    ++seqno;
    bKGCodes = EMPTY_STRING();
    inclFareBC = EMPTY_STRING();
    exclFareBC = EMPTY_STRING();

    uint16_t length = groupBrandResponseItem->_bookingCodeVec.size();
    for (uint16_t n = 0; n != length; ++n)
    {
      bKGCodes.append(groupBrandResponseItem->_bookingCodeVec[n]);
      ++lineLen;
      if (lineLen > 57)
      {
        bKGCodes.append("\n                  ");
        lineLen = 18;
      }
      else if (n != length - 1)
      {
        bKGCodes.append(separator);
        lineLen += 2;
      }
    }
    lineLen = 18;
    length = groupBrandResponseItem->_includedFClassVec.size();
    for (uint16_t n = 0; n != length; ++n)
    {
      inclFareBC.append(groupBrandResponseItem->_includedFClassVec[n]);
      lineLen += groupBrandResponseItem->_includedFClassVec[n].size();
      if (lineLen > 52)
      {
        inclFareBC.append("\n                  ");
        lineLen = 18;
      }
      else if (n != length - 1)
      {
        inclFareBC.append(separator);
        lineLen += 2;
      }
    }
    length = groupBrandResponseItem->_excludedFClassVec.size();
    for (uint16_t n = 0; n != length; ++n)
    {
      exclFareBC.append(groupBrandResponseItem->_excludedFClassVec[n]);
      lineLen += groupBrandResponseItem->_excludedFClassVec[n].size();
      if (lineLen > 52)
      {
        exclFareBC.append("\n                  ");
        lineLen = 18;
      }
      else if (n != length - 1)
      {
        exclFareBC.append(separator);
        lineLen += 2;
      }
    }

    trx.response() << "  ORDER NO      - " << seqno << std::endl;
    trx.response() << "  CAMPAIGN CODE - " << groupBrandResponseItem->_campaignCode << std::endl;
    trx.response() << "  BRAND CD/NAME - " << groupBrandResponseItem->_brandCode << "/"
                   << groupBrandResponseItem->_brandName << std::endl;
    trx.response() << "  BRAND TEXT    - " << groupBrandResponseItem->_brandText << std::endl;
    trx.response() << "  BKG CODES     - " << bKGCodes << std::endl; //  booking codes
    trx.response() << "  INC FAREBASIS - " << inclFareBC << std::endl; // farebasis code
    trx.response() << "  EXC FAREBASIS - " << exclFareBC << std::endl; // farebasis code
    trx.response() << std::endl << "  -------------------------------------------------------------"
                   << std::endl;
  }

  trx.response() << std::endl;
}
} // tse namespace
