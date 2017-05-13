//-------------------------------------------------------------------
//
//  File:        FaresSectionFilter.cpp
//  Authors:     Mike Carroll
//  Created:     August 20, 2005
//  Description: Fares section filter
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

#include "FareDisplay/Templates/FaresSectionFilter.h"

#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseUtil.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TravelInfo.h"
#include "DBAccess/Brand.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/PaxTypeInfo.h"
#include "FareDisplay/FDConsts.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Fares/FDFareCurrencySelection.h"

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

namespace tse
{
FALLBACK_DECL(fallbackFareDisplayByCabinActivation);

static Logger
logger("atseintl.FareDisplay.Templates.FaresSectionFilter");

using BrandNameTextMap = std::map<BrandCode, std::pair<BrandName, BrandText>>;

void
FaresSectionFilter::initialize(FareDisplayTrx& trx)
{
  _trx = &trx;
  _prevPaxType = EMPTY_STRING();
  _prevCarrier = EMPTY_STRING();
  _prevBrand = EMPTY_STRING();
  _prevOrigDest = EMPTY_STRING();
  _prevGlobalDir = GlobalDirection::XX; // Universal Global Direction
  _travelDate = TseUtil::getTravelDate(_trx->travelSeg());
  _emptyDate = DateTime::emptyDate();
  _prevEffDate = _emptyDate;
  _prevDiscDate = _emptyDate;

  _isInternational =
      (trx.itin().front()->geoTravelType() == GeoTravelType::International || trx.isSameCityPairRqst());

  _isMultiTransportHeader = checkHeaderType(Group::GROUP_BY_MULTITRANSPORT);

  _isBrandHeader = checkHeaderType(Group::GROUP_BY_BRAND);
  // for the Branded Fares project. The line above will be removed in the future
  _isS8BrandHeader = checkHeaderType(Group::GROUP_BY_S8BRAND);
  _prevS8Brand = make_pair(EMPTY_STRING(), EMPTY_STRING());

  // price by cabin
  _isCabinHeader = checkHeaderType(Group::GROUP_BY_CABIN);
  _prevCabin = 0;

  _isAxessGlobalDirHeader = false;
  if (FareDisplayUtil::isAxessUser(trx))
  {
    if (checkHeaderType(Group::GROUP_BY_GLOBAL_DIR))
      _isAxessGlobalDirHeader = true;
    else
      _isMultiTransportHeader = false;
  }

  _needGlobalAndMPM = isGlobalDirectionNeeded();
}

bool
FaresSectionFilter::doFiltering(PaxTypeFare& paxTypeFare,
                                bool firstGroup,
                                bool hasMultipleGlobalForYY)
{
  LOG4CXX_DEBUG(logger, "In doFiltering");
  bool globalDirSectionAdded = false;
  bool psgrSectionAdded = false;
  bool yyFaresSectionAdded = false;
  bool brandSectionAdded = false;
  bool cabinSectionAdded = false;
  PaxTypeCode blankPaxType;
  // iterate through all header types that apply
  PaxTypeCode newPaxType =
      paxTypeFare.actualPaxType() != nullptr ? paxTypeFare.actualPaxType()->paxType() : blankPaxType;
  CarrierCode newCarrierCode = paxTypeFare.carrier();
  BrandCode newBrand = paxTypeFare.fareDisplayInfo()->brandCode();
  std::pair<ProgramCode, BrandCode> newS8Brand = paxTypeFare.fareDisplayInfo()->programBrand();
  // Price by Cabin
  uint8_t inclusionCabinNum = 0;
  bool singleInclusionCabin = false;
  if (!fallback::fallbackFareDisplayByCabinActivation(_trx) &&
      _trx->getRequest()->multiInclusionCodes())
  {
    inclusionCabinNum = paxTypeFare.fareDisplayInfo()->inclusionCabinNum();
  }
  else if (!fallback::fallbackFareDisplayByCabinActivation(_trx) &&
           _trx->getRequest()->inclusionNumber(_trx->getRequest()->requestedInclusionCode()) != 0)
  {
    inclusionCabinNum = _trx->getRequest()->inclusionNumber(_trx->getRequest()->requestedInclusionCode());
    singleInclusionCabin = true;
  }

  // Brand header
  if (_isBrandHeader)
  {
    if (newCarrierCode == INDUSTRY_CARRIER)
    {
      _isBrandHeader = false;
    }
    else
    {
      if (newBrand != _prevBrand)
      {
        if (!firstGroup)
          _trx->response() << " " << std::endl;

        addBrandSectionText(newBrand);
        _brandHeaderAdded = true;
        _prevBrand = newBrand;
        _prevPaxType = EMPTY_STRING();
        _prevOrigDest = EMPTY_STRING();
        _prevGlobalDir = GlobalDirection::XX; // Universal Global Direction
        brandSectionAdded = true;
      }
      else if (firstGroup && newBrand.empty())
      {
        brandSectionAdded = true;
      }
    }
  }
  else
      // S8 Brand header
      if (_isS8BrandHeader)
  {
    if (newS8Brand != _prevS8Brand)
    {
      if (!firstGroup)
        _trx->response() << " " << std::endl;

      addS8BrandSectionText(newS8Brand);
      _brandHeaderAdded = true;
      _prevS8Brand = newS8Brand;
      _prevPaxType = EMPTY_STRING();
      _prevOrigDest = EMPTY_STRING();
      _prevGlobalDir = GlobalDirection::XX; // Universal Global Direction
      brandSectionAdded = true;
    }
    else if (firstGroup && newS8Brand.first.empty() && newS8Brand.second.empty())
    {
      brandSectionAdded = true;
    }
  }

  bool isHeaderApplied(false);
  if (_isMultiTransportHeader && (newCarrierCode != INDUSTRY_CARRIER ||
                                  (newCarrierCode == INDUSTRY_CARRIER && yyFaresSectionAdded)))
  {
    if (!fallback::fallbackFareDisplayByCabinActivation(_trx))
      isHeaderApplied = addMultiTransportHeader(paxTypeFare, hasMultipleGlobalForYY,
                                                inclusionCabinNum, singleInclusionCabin);
    else
      isHeaderApplied = addMultiTransportHeader(paxTypeFare);
    if (isHeaderApplied)
      _prevGlobalDir = paxTypeFare.globalDirection();
  }

  std::vector<Group::GroupType>::const_iterator groupHdrItr =
      _trx->fdResponse()->groupHeaders().begin();
  std::vector<Group::GroupType>::const_iterator groupHdrEnd =
      _trx->fdResponse()->groupHeaders().end();

  for (; groupHdrItr != groupHdrEnd; groupHdrItr++)
  {
    _groupType = (*groupHdrItr);

    // YY Carrier header
    if (newCarrierCode != _prevCarrier && newCarrierCode == INDUSTRY_CARRIER &&
        _groupType == Group::GROUP_BY_CARRIER && FareDisplayUtil::validYYFaresExist(*_trx))
    {
      if (_trx->requestedCarrier() != INDUSTRY_CARRIER)
      {
        addYYFaresText(paxTypeFare, firstGroup, inclusionCabinNum);
      }
      _prevCarrier = newCarrierCode;
      yyFaresSectionAdded = true;

      // After the YY header message, the Global Direction be displayed even if
      // it has not changed from the last time displayed
      // add a PaxType header even if it has not changed from the carrier section

      if (_isMultiTransportHeader)
      {
        _prevOrigDest = EMPTY_STRING();
        if (!fallback::fallbackFareDisplayByCabinActivation(_trx))
          isHeaderApplied = addMultiTransportHeader(paxTypeFare, hasMultipleGlobalForYY,
                                                    inclusionCabinNum, singleInclusionCabin);
        else
          isHeaderApplied = addMultiTransportHeader(paxTypeFare);
        if (isHeaderApplied)
          _prevGlobalDir = paxTypeFare.globalDirection();
      }

      if (!firstGroup)
      {
        if (newPaxType.empty())
          newPaxType = ADULT;

        if (hasMultipleGlobalForYY)
        {
          if(_isCabinHeader && !isHeaderApplied && _prevCabin == 0 &&
             _trx->fdResponse()->isDisplayHeaderCabin())
          {
            if((singleInclusionCabin && _prevPaxType.empty()) ||
               (!singleInclusionCabin && yyFaresSectionAdded)   )
            {
              addCabinSectionText(inclusionCabinNum);
              cabinSectionAdded = true;
            }
            if(!singleInclusionCabin && yyFaresSectionAdded)
            {
              _prevCabin = inclusionCabinNum;
            }
          }

          if (checkHeaderType(Group::GROUP_BY_PSG_TYPE))
          {
            addPsgrSectionText(newPaxType, paxTypeFare.vendor());
            _prevPaxType = newPaxType;
            psgrSectionAdded = true;
          }
          if (!isHeaderApplied && !_isAxessGlobalDirHeader &&
              checkHeaderType(Group::GROUP_BY_GLOBAL_DIR))
          {
            addGlobalDirSectionText(paxTypeFare);
            _prevGlobalDir = paxTypeFare.globalDirection();
            globalDirSectionAdded = true;
          }
        }
        else
        {
          if (!isHeaderApplied && !_isAxessGlobalDirHeader &&
              checkHeaderType(Group::GROUP_BY_GLOBAL_DIR))
          {
            addGlobalDirSectionText(paxTypeFare);
            _prevGlobalDir = paxTypeFare.globalDirection();
            globalDirSectionAdded = true;
          }
          if(_isCabinHeader && !isHeaderApplied && _prevCabin == 0 &&
             _trx->fdResponse()->isDisplayHeaderCabin())
          {
            if((singleInclusionCabin && _prevPaxType.empty()) ||
               (!singleInclusionCabin && yyFaresSectionAdded)   )
            {
              addCabinSectionText(inclusionCabinNum);
              cabinSectionAdded = true;
            }
            if(!singleInclusionCabin && yyFaresSectionAdded)
            {
              _prevCabin = inclusionCabinNum;
            }
          }

          if (checkHeaderType(Group::GROUP_BY_PSG_TYPE))
          {
            addPsgrSectionText(newPaxType, paxTypeFare.vendor());
            _prevPaxType = newPaxType;
            psgrSectionAdded = true;
          }
        }
      }
      else if(_isCabinHeader && !isHeaderApplied && _prevCabin == 0 &&
              _trx->fdResponse()->isDisplayHeaderCabin())
      {
        if((singleInclusionCabin && _prevPaxType.empty()) ||
           (!singleInclusionCabin && yyFaresSectionAdded)   )
        {
          addCabinSectionText(inclusionCabinNum);
          cabinSectionAdded = true;
        }
        if(!singleInclusionCabin && yyFaresSectionAdded)
        {
          _prevCabin = inclusionCabinNum;
        }
      }
    }

    // Carrier Section Header Part
    // Global direction header
    if (!isHeaderApplied && !_isAxessGlobalDirHeader &&
        ((paxTypeFare.globalDirection() != _prevGlobalDir ||
          (_isBrandHeader && psgrSectionAdded)) &&
         _groupType == Group::GROUP_BY_GLOBAL_DIR))
    {
      if (!firstGroup && !yyFaresSectionAdded && !brandSectionAdded && !psgrSectionAdded &&
          !globalDirSectionAdded && !cabinSectionAdded)
        _trx->response() << " " << std::endl;

      addGlobalDirSectionText(paxTypeFare);
      _prevGlobalDir = paxTypeFare.globalDirection();
      globalDirSectionAdded = true;
    }

    // Global direction header for Axess
    if (!isHeaderApplied && _isAxessGlobalDirHeader && _groupType == Group::GROUP_BY_GLOBAL_DIR &&
        (paxTypeFare.globalDirection() != _prevGlobalDir ||
         getTravelEffDate(paxTypeFare) != _prevEffDate ||
         (getTravelDiscDate(paxTypeFare) != _prevDiscDate &&
          _trx->fdResponse()->isGroupedByTravelDiscDate()) ||
         (_isBrandHeader && psgrSectionAdded) || yyFaresSectionAdded))
    {
      if (!firstGroup && !yyFaresSectionAdded && !brandSectionAdded && !psgrSectionAdded &&
          !globalDirSectionAdded)
        _trx->response() << " " << std::endl;

      addAxessGlobalDirHeader(paxTypeFare);
      _prevGlobalDir = paxTypeFare.globalDirection();
      _prevEffDate = getTravelEffDate(paxTypeFare);
      _prevDiscDate = getTravelDiscDate(paxTypeFare);
      globalDirSectionAdded = true;
    }

    // Cabin Type Header for single Cabin code
    if(_isCabinHeader && _trx->fdResponse()->isDisplayHeaderCabin() &&
       singleInclusionCabin && _prevCabin == 0 &&
       (((hasMultipleGlobalForYY || _isMultiGlobalForCabin) && !globalDirSectionAdded) ||
        (!hasMultipleGlobalForYY &&
         (_trx->getOptions()->sortAscending() ||         //
          _trx->getOptions()->sortDescending() ||        // all 3 override, no global considered
          _trx->getRequest()->numberOfFareLevels() > 0 ||//
          globalDirSectionAdded || isHeaderApplied || !_isInternational))) &&
        !yyFaresSectionAdded && (newCarrierCode != INDUSTRY_CARRIER))
    {
      if(!firstGroup)
        _trx->response() << " " << std::endl;
      addCabinSectionText(inclusionCabinNum);
      _prevCabin = inclusionCabinNum;
      cabinSectionAdded = true;
    }

    // Cabin Type Header for MULTI Cabin code
    if(_groupType == Group::GROUP_BY_CABIN && !singleInclusionCabin &&
       (inclusionCabinNum != _prevCabin) &&
       !cabinSectionAdded && _trx->fdResponse()->isDisplayHeaderCabin() &&
       (((hasMultipleGlobalForYY ||
          (_isMultiGlobalForCabin && _isInternational) ||
          (_isMultiTransportHeader && _isMultiTransportCitiesFogCabin)) &&
          !globalDirSectionAdded) ||
          isSingleGlobalOrCityPair(hasMultipleGlobalForYY, globalDirSectionAdded) ||
        (!hasMultipleGlobalForYY &&
         (_trx->getOptions()->sortAscending() ||         //
          _trx->getOptions()->sortDescending() ||        // all 3 override, no global considered
          _trx->getRequest()->numberOfFareLevels() > 0 ))))
    {
      if(!firstGroup)
        _trx->response() << " " << std::endl;

      // check for the gap in fares between two inclusion codes
      if(_prevCabin != 0)
        addNoneCabinSectionText(inclusionCabinNum);

      addCabinSectionText(inclusionCabinNum);
      _prevCabin = inclusionCabinNum;
      cabinSectionAdded = true;

      if(!_prevPaxType.empty() && _prevPaxType == newPaxType)
        addPsgrSectionText(newPaxType, paxTypeFare.vendor());
    }

    // passenger Type Header
    if (newPaxType.empty())
      newPaxType = ADULT;
    if ((newPaxType != _prevPaxType || (_isBrandHeader && globalDirSectionAdded)) &&
        _groupType == Group::GROUP_BY_PSG_TYPE)
    {
      if (!firstGroup && !yyFaresSectionAdded && !brandSectionAdded && !psgrSectionAdded &&
          !globalDirSectionAdded && !cabinSectionAdded)
        _trx->response() << " " << std::endl;

      addPsgrSectionText(newPaxType, paxTypeFare.vendor());
      _prevPaxType = newPaxType;
      psgrSectionAdded = true;
    }
  }
  if (globalDirSectionAdded || psgrSectionAdded || yyFaresSectionAdded || brandSectionAdded ||
      isHeaderApplied || cabinSectionAdded)
    return true;

  return false;
}

bool
FaresSectionFilter::isSingleGlobalOrCityPair(bool multiYY, bool globalDirAdd)
{
  if(!multiYY && !_isMultiGlobalForCabin && !_isInternational &&
     !_isMultiTransportHeader && !_isMultiTransportCitiesFogCabin)
    return true;
  else
  if(!multiYY ||
     (!_isMultiGlobalForCabin && _isInternational) ||
     (_isMultiTransportHeader && !_isMultiTransportCitiesFogCabin))
  {
    if(_prevGlobalDir != GlobalDirection::XX || globalDirAdd)
      return true;
  }

  return false;
}

void
FaresSectionFilter::addCabinSectionText(uint8_t cabinInclusion, bool none)
{
  LOG4CXX_DEBUG(logger, "In addCabinSectionText");

  // Get the description for the Cabin inclusion code
  const std::string cabinVerbiage = _trx->getRequest()->inclusionVerbiage(cabinInclusion);
  if (!cabinVerbiage.empty())
  {
    _trx->response() << cabinVerbiage << std::endl;
    if(none)
      _trx->response() << "  NONE" << "\n  " << std::endl;
  }
  else
    _trx->response() << " " << std::endl;
}

void
FaresSectionFilter::addPsgrSectionText(const PaxTypeCode& paxType, const VendorCode& vendor)
{
  LOG4CXX_DEBUG(logger, "In addPsgrSectionText");

  DataHandle localDataHandle(_trx->ticketingDate());

  // Get the description for the new passenger type
  const PaxTypeInfo* info = localDataHandle.getPaxType(paxType, vendor);
  if (info != nullptr)
    _trx->response() << "  " << paxType << DASH << info->description() << std::endl;
  else
    _trx->response() << "  " << paxType << DASH << std::endl;
}

void
FaresSectionFilter::addGlobalDirSectionText(const PaxTypeFare& paxTypeFare)
{
  LOG4CXX_DEBUG(logger, "In addGlobalDirSectionText");
  std::ostringstream& oss = _trx->response();
  DataHandle localDataHandle(_trx->ticketingDate());
  std::string globalDir;
  globalDirectionToStr(globalDir, paxTypeFare.globalDirection());
  bool isShopping = _trx->isShopperRequest();

  oss.setf(std::ios::left, std::ios::adjustfield);
  if (isShopping)
    oss << std::setfill(' ') << std::setw(7) << "**";
  else
    oss << std::setfill(' ') << std::setw(7) << paxTypeFare.carrier();

  if (_isMultiTransportHeader)
  {
    addOriginDestination(paxTypeFare);
  }
  else
  {
    oss << _trx->boardMultiCity() << _trx->offMultiCity();
  }

  if (_needGlobalAndMPM)
  {
    oss << PERIOD << std::setfill(' ') << std::setw(9) << globalDir;
  }
  else
  {
    oss << std::setfill(' ') << std::setw(10) << "";
  }

  // Travel date
  DateTime travelDate = TseUtil::getTravelDate(_trx->travelSeg());
  oss << std::setfill(' ') << std::setw(17) << travelDate.dateToString(DDMMMYY, "");

  if (_needGlobalAndMPM)
  {
    // MPM
    oss << "MPM ";
    const Mileage* mileage = _trx->dataHandle().getMileage(_trx->boardMultiCity(),
                                                           _trx->offMultiCity(),
                                                           'M',
                                                           paxTypeFare.globalDirection(),
                                                           travelDate);

    oss << std::setfill(' ') << std::setw(5);
    oss.setf(std::ios::right, std::ios::adjustfield);

    if (mileage != nullptr)
      oss << mileage->mileage();
  }
  oss << std::endl;
}

void
FaresSectionFilter::addAxessGlobalDirHeader(const PaxTypeFare& paxTypeFare)
{
  LOG4CXX_DEBUG(logger, "In addAxessGlobalDirHeader");

  std::ostringstream& oss = _trx->response();
  DataHandle localDataHandle(_trx->ticketingDate());
  std::string date;
  std::string globalDir;
  globalDirectionToStr(globalDir, paxTypeFare.globalDirection());

  oss.setf(std::ios::left, std::ios::adjustfield);

  // Travel Effective Date
  date = getTravelEffDate(paxTypeFare).dateToString(DDMMMYY, "");
  if (date[0] == '0')
    date[0] = ' '; // Replace with space

  oss << "**" << date;
  oss << "*";

  if (_trx->fdResponse()->isGroupedByTravelDiscDate())
  {
    // Travel Discontinue Date
    DateTime travelDiscDate = getTravelDiscDate(paxTypeFare);

    if (!travelDiscDate.isEmptyDate())
    {
      date = travelDiscDate.dateToString(DDMMMYY, "");
      if (date[0] == '0')
        date[0] = ' '; // Replace with space

      oss << date;
    }
    else
      oss << std::setfill(' ') << std::setw(7) << " OPEN";
  }
  else
    oss << std::setfill(' ') << std::setw(7) << " ";

  // Carrier
  oss.setf(std::ios::left, std::ios::adjustfield);
  oss << "/" << std::setfill(' ') << std::setw(5);

  if (_trx->isShopperRequest() || _trx->multipleCarriersEntered())
  {
    oss << "**";
  }
  else
  {
    oss << paxTypeFare.carrier();
  }

  // City Pair
  if (_isMultiTransportHeader)
  {
    addOriginDestination(paxTypeFare);
  }
  else
  {
    oss << _trx->boardMultiCity() << _trx->offMultiCity();
  }

  // Global Direction
  if (globalDir.empty())
    globalDir = "WH";

  oss << "/" << globalDir << std::setfill(' ') << std::setw(17) << "/";

  // MPM
  oss << "/MPM.";

  if (paxTypeFare.isInternational())
  {
    const Mileage* mileage = _trx->dataHandle().getMileage(_trx->boardMultiCity(),
                                                           _trx->offMultiCity(),
                                                           'M',
                                                           paxTypeFare.globalDirection(),
                                                           _travelDate);

    oss.setf(std::ios::right, std::ios::adjustfield);
    oss << std::setfill(' ') << std::setw(5);

    if (mileage != nullptr)
      oss << mileage->mileage();
    else
      oss << ".....";
  }
  else
    oss << ".....";

  // Currency
  oss << "/";

  // As Addon Fare doesn't pass through FVO itin::calculationCurrency(),
  //  it doesn't hold the appropriate displaycurrency
  if (_trx->getRequest()->inclusionCode() == FD_ADDON)
  {
    CurrencyCode displayCurrency;
    FDFareCurrencySelection::getDisplayCurrency(*_trx, displayCurrency);
    oss << displayCurrency;
  }
  else
  {
    Itin* itin = _trx->itin().front();
    oss << itin->calculationCurrency();
  }

  oss << std::endl;
}

void
FaresSectionFilter::addYYFaresText(const PaxTypeFare& paxTypeFare,
                                   bool firstGroup,
                                   uint8_t inclusionCabinNum)
{
  LOG4CXX_DEBUG(logger, "In addPsgrSectionText");

  // Price By Cabin project for the CXR fares portion before starts YY fares
  // to display NONE for the inclusion cabin codes with no carrier fares
  if (!fallback::fallbackFareDisplayByCabinActivation(_trx) &&
      _trx->getRequest()->multiInclusionCodes() &&
      _trx->fdResponse()->isDisplayHeaderCabin())
  {
    uint16_t count = _trx->fdResponse()->multiInclusionCabins().size();

    if( _prevCabin != _trx->fdResponse()->multiInclusionCabins().at(count-1) )
    {
      auto foundIt = std::find(std::begin(_trx->fdResponse()->multiInclusionCabins()),
                               std::end(_trx->fdResponse()->multiInclusionCabins()), _prevCabin);
      if(foundIt != std::end(_trx->fdResponse()->multiInclusionCabins()))
      {
        size_t dist = std::distance(std::begin(_trx->fdResponse()->multiInclusionCabins()), foundIt);
        if( ++dist < count )
          _trx->response() << " " << std::endl; 
        for(; dist < count; ++dist)
          addCabinSectionText(_trx->fdResponse()->multiInclusionCabins().at(dist), true);
      }
    }
    _prevCabin = 0; // initialyze cabin to process YY fares
  }

  InclusionCode incCode;

  if (_trx->getRequest()->inclusionCode() != ALL_INCLUSION_CODE)
  {
    incCode = _trx->getRequest()->inclusionCode();
  }
  else
  {
    incCode = "REQUESTED";
  }

  std::string privateString = "";
  if ((_trx->getOptions()->isPrivateFares()) && (incCode == "REQUESTED"))
    privateString = " PRVT";
  else if (_trx->getOptions()->isPrivateFares())
    privateString = " PRIVATE";

  // Always leave a line before YY fares statement if it is not the
  // very first group
  const CarrierCode& carrier = _trx->requestedCarrier();

  if (!firstGroup || FareDisplayUtil::validCarrierFaresExist(*_trx))
    _trx->response() << " " << std::endl;
  _trx->response() << "*** BELOW ARE YY " << incCode << privateString << " FARES "
                   << _trx->boardMultiCity() << DASH << _trx->offMultiCity() << " VALID ON "
                   << carrier << " *** " << std::endl;
  _trx->response() << " " << std::endl;

  // Test inclusion cabin codes with no YY fares and then display NONE
  if (!fallback::fallbackFareDisplayByCabinActivation(_trx) &&
      _trx->getRequest()->multiInclusionCodes() && _trx->fdResponse()->isDisplayHeaderCabin())
  {
    if(inclusionCabinNum != _trx->fdResponse()->multiInclusionCabins().at(0) )
    {
      auto foundIt = std::find(std::begin(_trx->fdResponse()->multiInclusionCabins()),
                               std::end(_trx->fdResponse()->multiInclusionCabins()), inclusionCabinNum);
      if(foundIt != std::end(_trx->fdResponse()->multiInclusionCabins()))
      {
        size_t dist = std::distance(std::begin(_trx->fdResponse()->multiInclusionCabins()), foundIt);
        for(size_t i = 0; i < dist; ++i)
          addCabinSectionText(_trx->fdResponse()->multiInclusionCabins().at(i), true);
      }
    }
  }
}

void
FaresSectionFilter::addBrandSectionText(const BrandCode& brandCode)
{
  LOG4CXX_DEBUG(logger, "In addBrandSectionText");

  // Get the name and free text for the new brand
  std::string brandName = "UNDEFINED BRAND";
  std::string brandText = EMPTY_STRING();
  Indicator userApplType;
  UserApplCode userAppl;

  // Get brand info

  if (_trx->fdResponse()->brandNameTextMap().empty()) // no response from brand service
  {
    if (FareDisplayUtil::getCRSParams(*_trx, userApplType, userAppl))
    {
      const Brand* brand = _trx->dataHandle().getBrand(
          userApplType, userAppl, _trx->requestedCarrier(), brandCode, _trx->travelDate());

      if (brand)
      {
        brandName = brand->brandName();
        brandText = brand->brandText();
      }
    }
  }
  else
  {
    std::map<BrandCode, std::pair<BrandName, BrandText> >::const_iterator
    brandNameTextMapConstIter = _trx->fdResponse()->brandNameTextMap().find(brandCode);
    if (brandNameTextMapConstIter != _trx->fdResponse()->brandNameTextMap().end())
    {
      brandName = brandNameTextMapConstIter->second.first.c_str();
      brandText = brandNameTextMapConstIter->second.second.c_str();
    }
  }
  // Add carrier, market type code and brand name

  _trx->response() << _trx->requestedCarrier() << "-" << _trx->fdResponse()->marketTypeCode() << "/"
                   << brandName;

  // Add brand code

  if (brandCode.empty())
  {
    _trx->response() << std::endl;
    return;
  }

  _trx->response() << "-" << brandCode;

  // Add brand text

  if (brandText.empty())
  {
    _trx->response() << std::endl;
    return;
  }

  // Calculate remaining space left for brand text
  uint16_t spaceLeft = 63 - (13 + brandName.size());

  if (brandText.size() > spaceLeft)
  {
    _trx->response() << " - " << brandText.substr(0, spaceLeft) << std::endl;
  }
  else
  {
    _trx->response() << " - " << brandText << std::endl;
  }
}

void
FaresSectionFilter::addS8BrandSectionText(const std::pair<ProgramCode, BrandCode>& newS8Brand)
{
  LOG4CXX_DEBUG(logger, "In addS8BrandSectionText");

  // Get the name and free text for the new brand
  std::string brandName = "UNDEFINED BRAND";
  SystemCode systemCode = ' ';
  ProgramCode programCode = newS8Brand.first;
  boost::to_upper(programCode);

  BrandCode brandCode = newS8Brand.second;
  boost::to_upper(brandCode);

  if (_trx->fdResponse()->programBrandNameMap().empty()) // no response from brand service
  {
    // Add some code in case of empty ....????
  }
  else
  {
    std::map<std::pair<ProgramCode, BrandCode>,
             std::tuple<ProgramName, BrandNameS8, SystemCode> >::const_iterator brandNameI =
        _trx->fdResponse()->programBrandNameMap().find(newS8Brand);
    if (brandNameI != _trx->fdResponse()->programBrandNameMap().end())
    {
      brandName = std::get<1>(brandNameI->second);
      systemCode = std::get<2>(brandNameI->second);
      boost::to_upper(brandName);
    }
  }

  if (programCode.empty() && brandCode.empty())
  {
    _trx->response() << std::endl;
    return;
  }
  // Add carrier

  _trx->response() << _trx->requestedCarrier() << "-" << _trx->requestedCarrier();

  if (systemCode == ' ')
    _trx->response() << systemCode;
  else
    _trx->response() << static_cast<char>(std::toupper(systemCode));

  // Add program code and brand code
  //    _trx->response() << "/" << programCode.c_str()     // program code, up to 10
  //                     << "/" << brandCode.c_str();      // brand code, up to 10

  _trx->response() << "/" << brandCode.c_str(); // brand code, up to 10

  // Add brand name

  if (brandName.empty())
  {
    _trx->response() << std::endl;
    return;
  }

  // Calculate remaining space left for brand text
  //    uint16_t spaceLeft = 63 - (6 + programCode.size() + brandCode.size());
  uint16_t spaceLeft = 63 - (10 + brandCode.size());

  if (brandName.size() > spaceLeft)
  {
    _trx->response() << " - " << brandName.substr(0, spaceLeft) << std::endl;
  }
  else
  {
    _trx->response() << " - " << brandName << std::endl;
  }
}

bool
FaresSectionFilter::displayYYHdrMsg()
{
  bool ret = false;
  // loop through all fares in order to display header messages
  // when no valid YY fares exist in the fare markets

  if (_trx->isDomestic() || _trx->isShopperRequest() ||
      _trx->getRequest()->requestType() == FARE_MILEAGE_REQUEST)

    return ret;

  std::ostringstream& p0 = _trx->response();
  InclusionCode incCode;
  if (_trx->getRequest()->inclusionCode() != ALL_INCLUSION_CODE)
  {
    incCode = _trx->getRequest()->inclusionCode();
  }
  else
  {
    incCode = "REQUESTED";
  }

  std::string privateString = "";
  if ((_trx->getOptions()->isPrivateFares()) && (incCode == "REQUESTED"))
    privateString = " PRVT";
  else if (_trx->getOptions()->isPrivateFares())
    privateString = " PRIVATE";

  Itin* itin = _trx->itin().front();

  CarrierCode carrierCode;
  int countYY = 0, countValidYY = 0, fareCount = 0;

  for (const auto fareMarket : itin->fareMarket())
  {
    for (const auto paxTypeFare : fareMarket->allPaxTypeFare())
    {
      carrierCode = paxTypeFare->carrier();

      if (carrierCode == INDUSTRY_CARRIER)
      {
        countYY++;
        if (paxTypeFare->isValidForPricing())
          countValidYY++;
      }
      else if (carrierCode != INDUSTRY_CARRIER && paxTypeFare->isValidForPricing())
        fareCount++;
    }
  }

  bool fistcall = true;
  if (countYY > 0)
  {
    const CarrierCode& carrier = _trx->requestedCarrier();
    // YY fares are published
    if (countValidYY == 0)
    {
      if (!fallback::fallbackFareDisplayByCabinActivation(_trx) &&
          _trx->getRequest()->multiInclusionCodes())
      {
        if(incCode.size() <= 10 )
        {
          uint8_t sizeIncl = _trx->getRequest()->requestedInclusionCode().size()/2;
          for (uint8_t number = 0; number < sizeIncl;  ++number)
          {
            incCode = _trx->getRequest()->requestedInclusionCode().substr(number*2, 2);
            displayYYNotPermitted(p0, incCode, privateString, fareCount, fistcall);
            fistcall = false;
          }
          return true;
        }
        if(incCode.size() > 10 )
        {
          incCode = "AB";
        }
      }
      if (!fallback::fallbackFareDisplayByCabinActivation(_trx))
      displayYYNotPermitted(p0, incCode, privateString, fareCount);
      else
      {
      if (fareCount > 0)
        p0 << " " << std::endl;
      p0 << "*** YY " << incCode << privateString << " FARES NOT PERMITTED "
         << _trx->boardMultiCity() << DASH << _trx->offMultiCity() << " ON " << carrier << " *** "
         << std::endl;
      }
      ret = true;
    }
  }
  else
  {
    if (!fallback::fallbackFareDisplayByCabinActivation(_trx) &&
        _trx->getRequest()->multiInclusionCodes())
    {
      if(incCode.size() <= 10 )
      {
        uint8_t sizeIncl = _trx->getRequest()->requestedInclusionCode().size()/2;
        for (uint8_t number = 0; number < sizeIncl;  ++number)
        {
          incCode = _trx->getRequest()->requestedInclusionCode().substr(number*2, 2);
          displayYYNotPublished(p0, incCode, privateString, fareCount, fistcall);
          fistcall = false;
        }
        return true;
      }
      if(incCode.size() > 10 )
      {
        incCode = "AB";
      }
    }
    if (!fallback::fallbackFareDisplayByCabinActivation(_trx))
    displayYYNotPublished(p0, incCode, privateString, fareCount);
    else
    {
    if (fareCount > 0)
      p0 << " " << std::endl;
    p0 << "*** THERE ARE NO YY";

    if ((incCode != "NLX") || ((_trx->boardMultiCity()) != (_trx->offMultiCity())))
    {
      p0 << " " << incCode;
    }
    p0 << privateString;

    p0 << " FARES PUBLISHED " << _trx->boardMultiCity() << "-" << _trx->offMultiCity() << " *** "
       << std::endl;
    }
    ret = true;
  }
  return ret;
}

void
FaresSectionFilter::displayYYNotPermitted(std::ostringstream& p0, InclusionCode& incCode,
                                          std::string& privateString, int fareCount, bool fistcall)
{
  const CarrierCode& carrier = _trx->requestedCarrier();
  if (fareCount > 0 && fistcall)
    p0 << " " << std::endl;
  p0 << "*** YY " << incCode << privateString << " FARES NOT PERMITTED "
     << _trx->boardMultiCity() << DASH << _trx->offMultiCity() << " ON " << carrier << " *** "
     << std::endl;
}

void
FaresSectionFilter::displayYYNotPublished(std::ostringstream& p0, InclusionCode& incCode,
                                          std::string& privateString, int fareCount, bool fistcall)
{
  if (fareCount > 0 && fistcall)
    p0 << " " << std::endl;
  p0 << "*** THERE ARE NO YY";

  if ((incCode != "NLX") || ((_trx->boardMultiCity()) != (_trx->offMultiCity())))
  {
    p0 << " " << incCode;
  }
  p0 << privateString;

  p0 << " FARES PUBLISHED " << _trx->boardMultiCity() << "-" << _trx->offMultiCity() << " *** "
     << std::endl;
}

bool
FaresSectionFilter::checkHeaderType(Group::GroupType groupType)
{
  for (const auto& groupHdr : _trx->fdResponse()->groupHeaders())
  {
    if (groupType == groupHdr)
      return true;
  }
  return false;
}

bool
FaresSectionFilter::addMultiTransportHeader(const PaxTypeFare& paxTypeFare)
{
  const std::string& actualOrigDest = paxTypeFare.origin() + paxTypeFare.destination();
  const std::string& actualDestOrig = paxTypeFare.destination() + paxTypeFare.origin();

  if (!isSameMarket(_prevOrigDest, actualOrigDest, actualDestOrig))
  {
    if (!_prevOrigDest.empty())
      _trx->response() << " " << std::endl;

    _prevOrigDest = paxTypeFare.origin() + paxTypeFare.destination();

    if (_isAxessGlobalDirHeader)
    {
      addAxessGlobalDirHeader(paxTypeFare);
      _prevEffDate = getTravelEffDate(paxTypeFare);
      _prevDiscDate = getTravelDiscDate(paxTypeFare);
    }
    else
    {
      addGlobalDirSectionText(paxTypeFare);
    }
    return true;
  }
  return false;
}

bool
FaresSectionFilter::addMultiTransportHeader(const PaxTypeFare& paxTypeFare,
                                            bool hasMultipleGlobalForYY,
                                            uint8_t inclusionCabinNum,
                                            bool singleInclusionCabin)
{
  const std::string& actualOrigDest = paxTypeFare.origin() + paxTypeFare.destination();
  const std::string& actualDestOrig = paxTypeFare.destination() + paxTypeFare.origin();

  if (!isSameMarket(_prevOrigDest, actualOrigDest, actualDestOrig))
  {
    if (!_prevOrigDest.empty())
      _trx->response() << " " << std::endl;

    _prevOrigDest = paxTypeFare.origin() + paxTypeFare.destination();

    if (_isAxessGlobalDirHeader)
    {
      addAxessGlobalDirHeader(paxTypeFare);
      _prevEffDate = getTravelEffDate(paxTypeFare);
      _prevDiscDate = getTravelDiscDate(paxTypeFare);
    }
    else
    {
      if(_isCabinHeader && _trx->fdResponse()->isDisplayHeaderCabin() &&
         (hasMultipleGlobalForYY ||
          (_isMultiGlobalForCabin && _isInternational) ||
          (_isMultiTransportHeader && _isMultiTransportCitiesFogCabin)))
      {
        if((!singleInclusionCabin && (inclusionCabinNum != _prevCabin) ) ||
           (singleInclusionCabin && _prevPaxType.empty() && _prevCabin == 0))
        {
          // check for the gap in fares between two inclusion codes
          if(!singleInclusionCabin && _prevCabin != 0)
            addNoneCabinSectionText(inclusionCabinNum);

          addCabinSectionText(inclusionCabinNum);
          _prevCabin = inclusionCabinNum;
        }
      }
      addGlobalDirSectionText(paxTypeFare);
    }
    return true;
  }
  return false;
}

void
FaresSectionFilter::addNoneCabinSectionText(uint8_t inclusionCabinNum)
{
  auto foundIt1 = std::find(std::begin(_trx->fdResponse()->multiInclusionCabins()),
                            std::end(_trx->fdResponse()->multiInclusionCabins()), _prevCabin);

  auto foundIt2 = std::find(std::begin(_trx->fdResponse()->multiInclusionCabins()),
                            std::end(_trx->fdResponse()->multiInclusionCabins()), inclusionCabinNum);
  if(foundIt1 != std::end(_trx->fdResponse()->multiInclusionCabins()) &&
     foundIt2 != std::end(_trx->fdResponse()->multiInclusionCabins()) &&
     foundIt1 + 1 != foundIt2 )
  {
    for(foundIt1++; foundIt1 != foundIt2; ++foundIt1)
      addCabinSectionText(*foundIt1, true);
  }
}

bool
FaresSectionFilter::isSameMarket(const std::string& currentMarket,
                                 const std::string& origDest,
                                 const std::string& destOrig)
{
  return ((currentMarket == origDest) || (currentMarket == destOrig));
}

bool
FaresSectionFilter::isGlobalDirectionNeeded()
{
  if (_isInternational)
  {
    if (checkHeaderType(Group::GROUP_BY_GLOBAL_DIR))
    {
      if(!fallback::fallbackFareDisplayByCabinActivation(_trx))// determine if globals > 1
      {
        GlobalDirection firstGlobalDir = _trx->allPaxTypeFare().front()->globalDirection();
        for( auto paxTfare : _trx->allPaxTypeFare())
        {
          if(firstGlobalDir != paxTfare->globalDirection())
          {
            _isMultiGlobalForCabin = true;
            break;
          }
        }
      }
      return true;
    }

    if (_trx->allPaxTypeFare().empty())
      return false;

    // Check for multiple globals
    std::vector<PaxTypeFare*>::const_iterator iB = _trx->allPaxTypeFare().begin();
    std::vector<PaxTypeFare*>::const_iterator iE = _trx->allPaxTypeFare().end();

    GlobalDirection previousGlobalDir = (*iB)->globalDirection();

    for (; iB != iE; iB++)
    {
      if (previousGlobalDir != (*iB)->globalDirection())
        return false; // Multiple globals
    }
    return true; // Only one global
  }
  else if(_isMultiTransportHeader)
  {
    const std::string& actualOrigDest = _trx->allPaxTypeFare().front()->origin() +
                                        _trx->allPaxTypeFare().front()->destination();
    const std::string& actualDestOrig = _trx->allPaxTypeFare().front()->destination() +
                                        _trx->allPaxTypeFare().front()->origin();
    LocCode prevOrigDest = actualOrigDest;

    for( auto paxTfare : _trx->allPaxTypeFare())
    {
      const std::string& actualOrigDest = paxTfare->origin() + paxTfare->destination();
      const std::string& actualDestOrig = paxTfare->destination() + paxTfare->origin();

      if (!isSameMarket(prevOrigDest, actualOrigDest, actualDestOrig))
      {
         _isMultiTransportCitiesFogCabin = true;
        break;
      }
    }
  }

  return false;
}

void
FaresSectionFilter::addOriginDestination(const PaxTypeFare& pFare)
{
  const PaxTypeFare* ptFare = pFare.fareWithoutBase();

  if (ptFare->isReversed() && ptFare->directionality() == BOTH)
  {
    _trx->response() << ptFare->destination() << ptFare->origin();
  }
  else
  {
    _trx->response() << ptFare->origin() << ptFare->destination();
  }
}

const DateTime&
FaresSectionFilter::getTravelDiscDate(const PaxTypeFare& p) const
{
  const FareDisplayInfo* info = p.fareDisplayInfo();

  if (info)
  {
    if (!info->travelInfo().empty())
    {
      if (info->travelInfo().front()->latestTvlStartDate().isValid())
      {
        return info->travelInfo().front()->latestTvlStartDate();
      }
      else if (info->travelInfo().front()->stopDate().isValid())
      {
        return info->travelInfo().front()->stopDate();
      }
    }
  }

  return _emptyDate;
}

const DateTime
FaresSectionFilter::getTravelEffDate(const PaxTypeFare& p) const
{
  DateTime travelEffDate = _emptyDate;

  const FareDisplayInfo* info = p.fareDisplayInfo();

  if (info)
  {
    if (!info->travelInfo().empty())
    {
      if (info->travelInfo().front()->earliestTvlStartDate().isValid())
      {
        travelEffDate = info->travelInfo().front()->earliestTvlStartDate();
      }
    }
  }

  if (travelEffDate < _travelDate)
    travelEffDate = _travelDate;

  return travelEffDate;
}
} // tse namespace
