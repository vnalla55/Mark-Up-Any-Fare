//-------------------------------------------------------------------
//  Created:     January 11, 2007
//  Authors:     Marco Cartolano
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

#include "FareDisplay/BrandDataRetriever.h"

#include "BrandingService/BrandingRequestFormatter.h"
#include "BrandingService/BrandingServiceCaller.h"
#include "BrandingService/BrandResponseHandler.h"
#include "BrandingService/BrandResponseItem.h"
#include "Common/ConfigurableBrandingServiceCaller.h"
#include "Common/FareDisplayUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/XMLConstruct.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/BrandedFareApp.h"
#include "DBAccess/DataHandle.h"
#include "FareDisplay/Comparator.h"
#include "FareDisplay/ComparatorFactory.h"
#include "FareDisplay/FDConsts.h"
#include "FareDisplay/Group.h"
#include "FareDisplay/GroupHeader.h"
#include "Rules/RuleUtil.h"

#include <vector>

namespace tse
{

FALLBACK_DECL(fallbackDebugOverrideBrandingServiceResponses);

static Logger
logger("atseintl.FareDisplay.BrandDataRetriever");

//-------------------------------------------------------------------
// constructor
//-------------------------------------------------------------------
BrandDataRetriever::BrandDataRetriever(FareDisplayTrx& trx)
  : _userApplType(' '), _userAppl(EMPTY_STRING()), _carrier(EMPTY_STRING()), _trx(trx)
{
}

//-------------------------------------------------------------------
// destructor
//-------------------------------------------------------------------
BrandDataRetriever::~BrandDataRetriever() {}

//------------------------------------------------------
// BrandDataRetriever::getBrandData()
//------------------------------------------------------
void
BrandDataRetriever::getBrandData(std::vector<Group*>& groups)
{
  LOG4CXX_INFO(logger, " Entered BrandDataRetriever::getBrandData()");

  if (!FareDisplayUtil::isBrandServiceEnabled())
  {
    Indicator userApplType;
    UserApplCode userAppl;

    if (FareDisplayUtil::getCRSParams(_trx, userApplType, userAppl))
    {
      if (getData(userApplType, userAppl, _trx.requestedCarrier()))
      {
        // Select data records based on directionality and geo location,
        //  and collect their booking codes with corresponding brand ids
        if (selectData())
        {
          initializeGroup(groups);
          determineBrands();
        }
      }
    }
  }
  else
  {
    if (getDataFromService())
    {
      if (selectDataFromService())
      {
        initializeGroupFromService(groups);
        determineBrandsFromService();
      }
    }
  }

  LOG4CXX_INFO(logger, " Leaving BrandDataRetriever::getBrandData()");
}

//------------------------------------------------------
// BrandDataRetriever::getData()
//------------------------------------------------------
bool
BrandDataRetriever::getData(const Indicator& userApplType,
                            const UserApplCode& userAppl,
                            const CarrierCode& carrier)
{
  _brandedFareAppData =
      _trx.dataHandle().getBrandedFareApp(userApplType, userAppl, carrier, _trx.travelDate());
  if (_brandedFareAppData.empty())
    return false;

  return true;
}

//------------------------------------------------------
// BrandDataRetriever::selectData()
//------------------------------------------------------
bool
BrandDataRetriever::selectData()
{
  LOG4CXX_INFO(logger, " Entered BrandDataRetriever::selectData()");

  _brandMap.clear();

  std::vector<BrandedFareApp*>::const_iterator itr = _brandedFareAppData.begin();
  std::vector<BrandedFareApp*>::const_iterator itrEnd = _brandedFareAppData.end();

  uint64_t seqno = 0;
  bool matchedLocation = false;

  // Search for the items that match directionality and geo location,
  //  and save their brand and corresponding booking code into the map
  for (; itr != itrEnd; ++itr)
  {
    if ((*itr)->seqno() != seqno)
    {
      if (isMatchLocation(*itr, *_trx.origin(), *_trx.destination()))
      {
        // Save matching item
        _brandedFareAppMatches.push_back(*itr);

        matchedLocation = true;
      }
      else
      {
        matchedLocation = false;
      }

      seqno = (*itr)->seqno();
    }

    if (matchedLocation)
    {
      // Add booking code/brand id to the map
      _brandMap.insert(make_pair((*itr)->bookingCode(), (*itr)->brandId()));
    }
  }

  LOG4CXX_INFO(logger, " Leaving BrandDataRetriever::selectData()");

  return !_brandMap.empty();
}

//------------------------------------------------------
// BrandDataRetriever::isMatchLocation()
//------------------------------------------------------
bool
BrandDataRetriever::isMatchLocation(const BrandedFareApp* const& brandedFareApp,
                                    const Loc& origin,
                                    const Loc& destination)
{
  LOG4CXX_INFO(logger, " Entered BrandDataRetriever::isMatchLocation()");

  const Directionality& dir = brandedFareApp->directionality();

  bool ret = false;

  if (dir == WITHIN)
    ret = LocUtil::isWithin(brandedFareApp->loc1(), origin, destination);

  if (dir == FROM)
    ret = LocUtil::isFrom(brandedFareApp->loc1(), brandedFareApp->loc2(), origin, destination);

  if (dir == BETWEEN)
    ret = LocUtil::isBetween(brandedFareApp->loc1(), brandedFareApp->loc2(), origin, destination);

  if (dir == ORIGIN)
    ret = LocUtil::isInLoc(origin.loc(),
                           brandedFareApp->loc1().locType(),
                           brandedFareApp->loc1().loc(),
                           Vendor::SABRE,
                           MANUAL);

  if (dir == TERMINATE)
    ret = LocUtil::isInLoc(destination.loc(),
                           brandedFareApp->loc2().locType(),
                           brandedFareApp->loc2().loc(),
                           Vendor::SABRE,
                           MANUAL);

  LOG4CXX_INFO(logger, " Leaving BrandDataRetriever::isMatchLocation()");
  return ret;
}

//------------------------------------------------------
// BrandDataRetriever::initializeGroup()
//------------------------------------------------------
void
BrandDataRetriever::initializeGroup(std::vector<Group*>& groups)
{
  LOG4CXX_INFO(logger, " Entered BrandDataRetriever::initializeGroup()");

  // Create Brand Group
  Group* grp = nullptr;
  _trx.dataHandle().get(grp);

  grp->groupType() = Group::GROUP_BY_BRAND;
  grp->sortType() = Group::ASCENDING;
  grp->brandMap() = _brandMap;
  grp->brandedFareApps() = _brandedFareAppMatches;

  // Save the market Type code to be displayed in the header line
  _trx.fdResponse()->marketTypeCode() = _brandedFareAppMatches.front()->marketTypeCode();

  ComparatorFactory rVFactory(_trx);

  grp->comparator() = rVFactory.getComparator(grp->groupType());

  if (grp->comparator() != nullptr)
  {
    grp->comparator()->group() = grp;
    grp->comparator()->prepare(_trx);
  }

  groups.push_back(grp);

  // Initialize Brand header
  GroupHeader header(_trx);
  header.setBrandHeader();

  LOG4CXX_INFO(logger, " Leaving BrandDataRetriever::initializeGroup()");
}

//-------------------------------------------------------------------
// BrandDataRetriever::determineBrands()
//-------------------------------------------------------------------
void
BrandDataRetriever::determineBrands()
{
  if (_trx.allPaxTypeFare().empty())
    return;

  if (_brandMap.empty())
    return;

  std::map<BookingCode, BrandCode>::const_iterator itrEnd = _brandMap.end();
  std::map<BookingCode, BrandCode>::const_iterator itr = itrEnd;

  // Loop through all fares to determine their brand

  std::vector<PaxTypeFare*>::iterator ptfItr = _trx.allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::iterator ptfItrEnd = _trx.allPaxTypeFare().end();

  while (ptfItr != ptfItrEnd)
  {
    if ((*ptfItr)->carrier() == INDUSTRY_CARRIER)
    {
      // No brand determination for YY fares
      ptfItr++;
      continue;
    }

    BookingCode bkgCode = isalpha((*ptfItr)->bookingCode()[1])
                              ? (*ptfItr)->bookingCode()
                              : BookingCode((*ptfItr)->bookingCode()[0]);

    itr = _brandMap.find(bkgCode);

    if (itr == itrEnd)
    {
      // Try to find a wild card
      itr = _brandMap.find("*");
    }

    if (itr != itrEnd)
    {
      FareDisplayInfo* fdInfo = (*ptfItr)->fareDisplayInfo();

      if (fdInfo)
      {
        fdInfo->setBrandCode(itr->second);
      }
    }

    ptfItr++;
  }
}

//------------------------------------------------------
// BrandDataRetriever::getDataFromService()
//------------------------------------------------------
bool
BrandDataRetriever::getDataFromService()
{
  LOG4CXX_INFO(logger, " Entered BrandDataRetriever::getDataFromService()");

  std::string request;

  XMLConstruct construct;

  BrandingRequestFormatter brf;

  brf.addRequestHeader(construct);
  brf.addXRAType(construct, _trx);
  brf.addSRCType(construct, _trx);
  brf.addBCRType(construct, _trx);

  // Call Branding service caller to send XML request
  request = construct.getXMLData().c_str();

  std::string response = callBrandingService(request);

  if (response.empty())
  {
    return false;
  }

  BrandResponseHandler docHandler(_trx, _brandResponseItemVec);
  docHandler.initialize();
  if (!docHandler.parse(response.c_str()))
  {
    LOG4CXX_ERROR(logger, "BRANDING SERVICE XML RESPONSE PARSER ERROR");
    return false;
  }

  LOG4CXX_INFO(logger, " Leaving BrandDataRetriever::getDataFromService()");

  return true;
}
//------------------------------------------------------
// BrandDataRetriever::selectDataFronService()
//------------------------------------------------------
bool
BrandDataRetriever::selectDataFromService()
{
  LOG4CXX_INFO(logger, " Entered BrandDataRetriever::selectDataFromService()");

  _brandMap.clear();

  std::vector<BrandResponseItem*>::const_iterator itr = _brandResponseItemVec.begin();
  std::vector<BrandResponseItem*>::const_iterator itrEnd = _brandResponseItemVec.end();

  if (!_brandResponseItemVec.empty())
  {
    _trx.fdResponse()->marketTypeCode() = _brandResponseItemVec.front()->_campaignCode;
  }
  else
  {
    return false;
  }

  for (; itr != itrEnd; ++itr)
  {
    // Add booking code/brand id to the map
    for (uint16_t n = 0; n != (*itr)->_bookingCodeVec.size(); ++n)
    {
      _brandMap.insert(make_pair((*itr)->_bookingCodeVec[n], (*itr)->_brandCode));
    }
    for (uint16_t n = 0; n != (*itr)->_includedFClassVec.size(); ++n)
    {
      _brandIncludeFareBasisCodeMap.insert(
          make_pair((*itr)->_includedFClassVec[n], (*itr)->_brandCode));
    }
    for (uint16_t n = 0; n != (*itr)->_excludedFClassVec.size(); ++n)
    {
      _brandExcludeFareBasisCodeMMap.insert(
          make_pair((*itr)->_excludedFClassVec[n], (*itr)->_brandCode));
    }

    _trx.fdResponse()->brandNameTextMap().insert(
        make_pair((*itr)->_brandCode, make_pair((*itr)->_brandName, (*itr)->_brandText)));
  }

  // Save the market Type code to be displayed in the header line

  LOG4CXX_INFO(logger, " Leaving BrandDataRetriever::selectDataFromService()");

  if (!_brandMap.empty() || !_brandIncludeFareBasisCodeMap.empty() ||
      !_brandExcludeFareBasisCodeMMap.empty())
  {
    return true;
  }
  return false;
}

//------------------------------------------------------
// BrandDataRetriever::initializeGroupFromService()
//------------------------------------------------------
void
BrandDataRetriever::initializeGroupFromService(std::vector<Group*>& groups)
{
  LOG4CXX_INFO(logger, " Entered BrandDataRetriever::initializeGroupFromService()");

  // Create Brand Group
  Group* grp = nullptr;
  _trx.dataHandle().get(grp);

  grp->groupType() = Group::GROUP_BY_BRAND;
  grp->sortType() = Group::ASCENDING;
  grp->brandResponseItemVec() = _brandResponseItemVec;

  ComparatorFactory rVFactory(_trx);

  grp->comparator() = rVFactory.getComparator(grp->groupType());

  if (grp->comparator() != nullptr)
  {
    grp->comparator()->group() = grp;
    grp->comparator()->prepare(_trx);
  }

  groups.push_back(grp);

  // Initialize Brand header
  GroupHeader header(_trx);
  header.setBrandHeader();

  LOG4CXX_INFO(logger, " Leaving BrandDataRetriever::initializeGroupFromService()");
}

//-------------------------------------------------------------------
// BrandDataRetriever::determineBrandsFromService()
//-------------------------------------------------------------------
void
BrandDataRetriever::determineBrandsFromService()
{
  if (_trx.allPaxTypeFare().empty())
    return;

  if (_brandMap.empty() && _brandIncludeFareBasisCodeMap.empty())
    return;

  std::map<BookingCode, BrandCode>::const_iterator itrEnd = _brandMap.end();
  std::map<BookingCode, BrandCode>::const_iterator itr = itrEnd;

  // Loop through all fares to determine their brand

  std::vector<PaxTypeFare*>::iterator ptfItr = _trx.allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::iterator ptfItrEnd = _trx.allPaxTypeFare().end();

  for (; ptfItr != ptfItrEnd; ++ptfItr)
  {
    if ((*ptfItr)->carrier() == INDUSTRY_CARRIER)
    {
      // No brand determination for YY fares
      continue;
    }

    FareDisplayInfo* fdInfo = (*ptfItr)->fareDisplayInfo();

    if (!fdInfo)
    {
      // skip this paxTypeFare
      continue;
    }

    // Determine paxTypeFare's fare basis code
    std::string ptfFareBasisCode = (*ptfItr)->createFareBasisCodeFD(_trx);

    bool matchedCriteria = false;
    bool exactMatch = false;
    BrandCode brand;

    //========================================================================
    // 1st: Check Included Fare Basis Codes
    //========================================================================
    std::map<std::string, BrandCode>::const_iterator itrIncFB =
        _brandIncludeFareBasisCodeMap.begin();
    std::map<std::string, BrandCode>::const_iterator itrIncFBEnd =
        _brandIncludeFareBasisCodeMap.end();

    for (; itrIncFB != itrIncFBEnd; ++itrIncFB)
    {
      if (isMatchFareBasisCode(itrIncFB->first, ptfFareBasisCode, exactMatch))
      {

        if (exactMatch)
        {
          matchedCriteria = true;
          brand = itrIncFB->second;
          break;
        }
        else if (!matchedCriteria)
        {
          matchedCriteria = true;
          brand = itrIncFB->second;
        }
      }
    } // end loop: brandIncludeFareBasisCodeMap

    if (matchedCriteria)
    {
      fdInfo->setBrandCode(brand);
    }
    else
    {
      //========================================================================
      // 2nd: Check Booking Codes
      //========================================================================
      BookingCode bkgCode = isalpha((*ptfItr)->bookingCode()[1])
                                ? (*ptfItr)->bookingCode()
                                : BookingCode((*ptfItr)->bookingCode()[0]);

      if (!_brandMap.empty())
      {
        itr = _brandMap.find(bkgCode);

        if (itr == itrEnd)
        {
          // Try to find a wild card
          itr = _brandMap.find("*");
        }

        if (itr != itrEnd)
        {
          fdInfo->setBrandCode(itr->second);
          matchedCriteria = true;
        }
      }
    }

    if (matchedCriteria)
    {
      //========================================================================
      // 3rd: Check Excluded Fare Basis Codes
      //========================================================================
      std::multimap<std::string, BrandCode>::const_iterator itrExcFB =
          _brandExcludeFareBasisCodeMMap.begin();
      std::multimap<std::string, BrandCode>::const_iterator itrExcFBEnd =
          _brandExcludeFareBasisCodeMMap.end();
      std::string preExcFB = EMPTY_STRING();

      for (; itrExcFB != itrExcFBEnd; ++itrExcFB)
      {

        if (itrExcFB->first != preExcFB &&
            isMatchFareBasisCode(itrExcFB->first, ptfFareBasisCode, exactMatch))
        {

          preExcFB = itrExcFB->first;

          std::pair<std::multimap<std::string, BrandCode>::iterator,
                    std::multimap<std::string, BrandCode>::iterator> iterPair;

          // Get begin and end iterators of current FareBasisCode

          iterPair = _brandExcludeFareBasisCodeMMap.equal_range(itrExcFB->first);
          std::multimap<std::string, BrandCode>::const_iterator i;

          for (i = iterPair.first; i != iterPair.second; ++i)
          {
            if (fdInfo->brandCode() == i->second)
            {
              // Remove brand code
              fdInfo->setBrandCode("");
              break;
            }
          }
          break;
        }
      } // end loop: brandExcludeFareBasisCodeMMap
    }
  } // end loop: allPaxTypeFare
}

//------------------------------------------------------
// BrandDataRetriever::isMatchFareBasisCode()
//------------------------------------------------------
bool
BrandDataRetriever::isMatchFareBasisCode(const std::string& brandFareBasisCode,
                                         const std::string& ptfFareBasisCode,
                                         bool& exactMatch)
{
  exactMatch = true;
  if (brandFareBasisCode == ptfFareBasisCode)
  {
    return true;
  }

  std::string brandTktDesignator = "";
  std::string ptfTktDesignator = "";
  std::string brandFareBasisOnly = brandFareBasisCode;
  std::string ptfFareBasisOnly = ptfFareBasisCode;

  // Check if any ticketing designator in brand fare basis code
  std::string::size_type pos = 0;
  pos = brandFareBasisCode.find('/');

  if (pos != std::string::npos)
  {
    brandTktDesignator.assign(brandFareBasisCode, pos + 1, brandFareBasisCode.size() - (pos + 1));
    brandFareBasisOnly.assign(brandFareBasisCode, 0, pos);
  }

  // Check if any ticketing designator in paxTypeFare fare basis code
  pos = ptfFareBasisCode.find('/');

  if (pos != std::string::npos)
  {
    ptfTktDesignator.assign(ptfFareBasisCode, pos + 1, ptfFareBasisCode.size() - (pos + 1));
    ptfFareBasisOnly.assign(ptfFareBasisCode, 0, pos);
  }

  if (!brandTktDesignator.empty())
  {
    if (brandTktDesignator != ptfTktDesignator)
    {
      return false;
    }
    else if (brandFareBasisOnly.empty())
    {
      return true;
    }
  }
  else if (brandFareBasisOnly == ptfFareBasisOnly)
  {
    return true;
  }

  // Check for hyphen
  pos = brandFareBasisOnly.find('-');

  if (pos != std::string::npos)
  {
    exactMatch = false;
    return RuleUtil::matchFareClass(brandFareBasisOnly.c_str(), ptfFareBasisOnly.c_str());
  }

  return false;
}

std::string
BrandDataRetriever::callBrandingService(std::string request)
{
  if (fallback::fallbackDebugOverrideBrandingServiceResponses(&_trx))
  {
    std::string response;
    BrandingServiceCaller<CBAS> bsc(_trx);
    bsc.callBranding(request, response);
    return response;
  }
  else
  {
    ConfigurableBrandingServiceCaller<BrandingServiceCaller<CBAS>> cbsc(_trx);
    return cbsc.callBranding(request);
  }
}

} //namespace tse
