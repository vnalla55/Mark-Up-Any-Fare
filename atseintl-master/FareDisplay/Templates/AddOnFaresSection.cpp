//-------------------------------------------------------------------
//
//  File:     AddOnFaresSection.cpp
//  Author:   Mike Carroll
//  Date:     July 26, 2005
//
//  Copyright Sabre 2005
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/AddOnFaresSection.h"

#include "BookingCode/FareDisplayBookingCode.h"
#include "Common/BrandingUtil.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/DataHandle.h"
#include "FareDisplay/Templates/ElementField.h"
#include "FareDisplay/Templates/ElementFilter.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "FareDisplay/Templates/FaresHeaderSection.h"
#include "FareDisplay/Templates/SeasonFilter.h"
#include "FareDisplay/Templates/Section.h"
#include "FareDisplay/RoutingSequenceGenerator.h"
#include "Fares/FDFareCurrencySelection.h"
#include "Rules/RuleConst.h"

namespace tse
{
namespace
{
Logger
logger("atseintl.FareDisplay.Templates.AddOnFaresSection");

static const std::string ADDON_ALSO_AVAILABLE_IN = "ADDONS ALSO AVAILABLE IN ";
static const std::string TRY_REVERSE_CITY_PAIR_ORDER = "TRY REVERSE CITY PAIR ORDER";
}

void
AddOnFaresSection::buildDisplay()
{
  DataHandle localDataHandle(_trx.ticketingDate());
  bool line2Available = false;

  LOG4CXX_DEBUG(logger, "In buildDisplay: columnFields size: " << _columnFields.size());
  if (_columnFields.empty())
    return;

  // Setup FareDisplayObject
  FareDisplayInfo fareDisplayInfo;
  RecordScope crossRefType = (_trx.isRecordScopeDomestic()) ? DOMESTIC : INTERNATIONAL;

  // -------------------------------------
  // Initialize FareDisplayInfo
  // -------------------------------------
  fareDisplayInfo.minStay() = "///";
  fareDisplayInfo.maxStay() = "///";
  fareDisplayInfo.displayOnly() = false;
  fareDisplayInfo.incompleteR3Rule() = false;
  fareDisplayInfo.unavailableR1Rule() = false;
  fareDisplayInfo.unavailableR3Rule() = false;

  // --------------------------------------------
  // create Object of FDAddOnFareController class
  // --------------------------------------------
  FDAddOnFareController fdAddOnFareController(_trx.allFDAddOnFare());
  std::vector<CurrencyCode> altCurrency;

  // --------------------------------------------------
  // Don't have any AddOn Fares ? Try reverse city pair.
  // --------------------------------------------------
  if (fdAddOnFareController.isEmpty())
  {

    std::vector<CurrencyCode> alternateCurrencyVec;
    if (fdAddOnFareController.isAvaiableInReverseCityPair(_trx, altCurrency))
    {
      if (!altCurrency.empty())
      {
        // "ADDONS ALSO AVAILABLE IN JPY"
        _trx.response() << ADDON_ALSO_AVAILABLE_IN;
        AddOnFaresSection::DisplayType dType = DisplayType::ALTERNATE_CURRENCY;
        for_each(altCurrency.begin(), altCurrency.end(), Display(_trx, dType));
        _trx.response() << std::endl;
      }
      // ------------------------------------
      // Display Arbitrary Fares GateWay Info
      // ------------------------------------
      _trx.response() << "*************** ARBITRARY FARES GATEWAY ";
      _trx.response() << _trx.origin()->loc() << " *****************" << std::endl;

      // Display "TRY REVERSE CITY PAIR"
      _trx.response() << TRY_REVERSE_CITY_PAIR_ORDER << std::endl;
    }
    else
    {
      CarrierCode cxr = _trx.requestedCarrier();
      _trx.response() << "NO FARES FOR THE REQUESTED ADDON MARKET ON " << cxr;
    }

    return;
  }

  // -----------------------------------------
  // Get Alternate Currencies for addon fares
  // -----------------------------------------
  if (fdAddOnFareController.alternateCurrencyVec(_trx, altCurrency))
  {
    _trx.response() << ADDON_ALSO_AVAILABLE_IN;
    AddOnFaresSection::DisplayType dType = DisplayType::ALTERNATE_CURRENCY;
    for_each(altCurrency.begin(), altCurrency.end(), Display(_trx, dType));
    _trx.response() << std::endl;
  }

  // ------------------------------------
  // Display Arbitrary Fares GateWay Info
  // ------------------------------------
  _trx.response() << "*************** ARBITRARY FARES GATEWAY ";
  _trx.response() << _trx.origin()->loc() << " *****************" << std::endl;

  // -----------------------------------
  // Add the ref of FDAddOnFare from Trx
  // -----------------------------------
  const std::vector<FDAddOnFareInfo*>& addOnFareList = _trx.allFDAddOnFare();

  FaresHeaderSection header(_trx, _templateSegRecs);

  std::ostringstream* localDisplayLine;

  std::map<std::string, std::string> globalDirMap;
  std::map<std::string, std::string> routingMap;
  std::string strGlobalDir;

  uint32_t fareCount = 0;
  MoneyAmount fare = 0;
  const uint32_t maxFares = FareDisplayUtil::getMaxFares();
  bool populateFareDataMapVec = BrandingUtil::isPopulateFareDataMapVec(_trx);

  std::vector<FDAddOnFareInfo*>::const_iterator addOnIter = addOnFareList.begin();
  std::vector<FDAddOnFareInfo*>::const_iterator addOnIterEnd = addOnFareList.end();

  for (; addOnIter != addOnIterEnd; addOnIter++)
  {
    if (fareCount >= maxFares)
    {
      LOG4CXX_WARN(logger, "More fares available then returned");
      displayMoreFaresExistMessage(_trx);
      break;
    }
    else
      fareCount++;

    // Convert Addon fares to display currency
    CurrencyCode displayCurrency;
    FDFareCurrencySelection::getDisplayCurrency(_trx, displayCurrency);

    fare = (*addOnIter)->fareAmt();
    if ((*addOnIter)->cur() != displayCurrency)
    {
      bool isInternational = true;
      CurrencyConversionFacade ccFacade;
      Money targetMoney(displayCurrency);
      Money sourceMoney(fare, (*addOnIter)->cur());

      if (ccFacade.convert(targetMoney,
                           sourceMoney,
                           _trx,
                           isInternational,
                           CurrencyConversionRequest::FAREDISPLAY))
      {
        fare = targetMoney.value();
      }
    }

    localDataHandle.get(localDisplayLine);
    initializeLine(localDisplayLine, 1);
    line2Available = false;

    // -------------------------
    // Display Global Header
    // -------------------------
    globalDirectionToStr(strGlobalDir, (*addOnIter)->globalDir());
    bool isNewGlobal = false;
    if (!strGlobalDir.empty() && (globalDirMap.end() == globalDirMap.find(strGlobalDir)))
    {
      globalDirMap[strGlobalDir] = strGlobalDir;
      const DateTime& depDate = _trx.getRequest()->requestedDepartureDT();

      // if (addOnIter == addOnFareList.begin())
      {
        addBlankLine();
        _trx.response() << (*addOnIter)->carrier() << "     " << (*addOnIter)->gatewayMarket();
        _trx.response() << (*addOnIter)->interiorMarket() << PERIOD << strGlobalDir;
        _trx.response() << "       " << depDate.dateToString(DDMMMYY, "");
        _trx.response() << "          MPM                    " << std::endl;
        isNewGlobal = true;
      }
    }

    // -----------------------------------
    // Determine whether we don't have globals and we
    // have new routing no - domestic
    // ------------------------------------
    bool isNewRouting = false;
    if (crossRefType == DOMESTIC)
    {
      if ((isNewRouting = (routingMap.end() == routingMap.find((*addOnIter)->routing()))))
      {
        if (addOnIter != addOnFareList.begin())
          addBlankLine();
        routingMap[(*addOnIter)->routing()] = (*addOnIter)->routing();
      }
    }

    // ----------------------------------------------
    // Display column header if we have new global or new routing no
    // ----------------------------------------------
    if ((crossRefType == INTERNATIONAL && isNewGlobal) ||
        (crossRefType == DOMESTIC && isNewRouting))
    {
      // addBlankLine();
      header.buildDisplay();
    }

    // Check for NP condition (Not auto-Priceable)
    if ((*addOnIter)->inhibit() == RuleConst::FARE_FOR_DISPLAY_ONLY)
    {
      fareDisplayInfo.setDisplayOnly();
    }
    else
    {
      fareDisplayInfo.displayOnly() = false;
    }

    std::map<FieldColumnElement, std::string> *fareDataMap = nullptr;
    if (populateFareDataMapVec)
      _trx.dataHandle().get(fareDataMap);

    for (const auto elementField : _columnFields)
    {
      switch (elementField->columnElement())
      {
      case LINE_NUMBER:
        LOG4CXX_DEBUG(logger, "Got LINE_NUMBER");
        {
          if (_trx.getOptions()->lineNumber() == 0)
            elementField->intValue() += 1;
          else
            elementField->intValue() = _trx.getOptions()->lineNumber();
          elementField->render(localDisplayLine, FieldValueType(INT_VALUE));
        }
        break;
      case SAME_DAY_IND:
        LOG4CXX_DEBUG(logger, "Got SAME_DAY_IND");
        break;
      case VENDOR_TRAVELOCITY_CODE:
        LOG4CXX_DEBUG(logger, "Got VENDOR_TRAVELOCITY_CODE");
        elementField->strValue() = (*addOnIter)->vendor();
        ElementFilter::vendorCode(*elementField, _trx);
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case PRIVATE_FARE_IND:
        LOG4CXX_DEBUG(logger, "Got PRIVATE_FARE_IND");
        break;
      case FARE_BASIS_TKT_DESIG:
        LOG4CXX_DEBUG(logger, "Got FARE_BASIS_TKT_DESIG");
        elementField->strValue() = (*addOnIter)->fareClass().c_str();
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case BOOKING_CODE:
        LOG4CXX_DEBUG(logger, "Got BOOKING_CODE");
        break;
      case JOURNEY_IND:
        LOG4CXX_DEBUG(logger, "Got JOURNEY_IND");
        ElementFilter::addOnJourneyType(*elementField, **addOnIter);
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        if(fareDataMap)
          fareDataMap->insert(std::map<FieldColumnElement, std::string>::value_type(
              JOURNEY_IND, elementField->strValue()));
        break;
      case FD_FARE_AMOUNT:
        LOG4CXX_DEBUG(logger, "Got FARE_AMOUNT");
        elementField->moneyValue() = fare;
        ElementFilter::formatMoneyAmount(*elementField, _trx);
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case OW_FARE_AMOUNT:
        LOG4CXX_DEBUG(logger, "Got OW_FARE_AMOUNT");
        break;
      case RT_FARE_AMOUNT:
        LOG4CXX_DEBUG(logger, "Got RT_FARE_AMOUNT");
        break;
      case NET_FARE_IND:
        LOG4CXX_DEBUG(logger, "Got NET_FARE_IND");
        break;
      case TRAVEL_TICKET:
        LOG4CXX_DEBUG(logger, "Got TRAVEL_TICKET");
        elementField->justify() = LEFT;
        ElementFilter::addOnTravelTicket(*elementField, fareDisplayInfo);
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case SEASONS:
        LOG4CXX_DEBUG(logger, "Got SEASONS");
        {
          elementField->justify() = LEFT;
          std::vector<ElementField> fields;
          SeasonFilter::formatData(fareDisplayInfo, *elementField, fields, _trx);
          elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
          uint16_t line = 2;
          for (uint16_t i = 1; i < fields.size(); ++i, ++line)
          {
            ElementField& nextField = fields[i];

            nextField.valueFieldSize() = elementField->valueFieldSize();
            nextField.valuePosition() = (MAX_PSS_LINE_SIZE * i) + elementField->valuePosition();
            nextField.justify() = elementField->justify();
            if (!line2Available)
            {
              initializeLine(localDisplayLine, line);
            }
            nextField.render(localDisplayLine, FieldValueType(STRING_VALUE));
          }
        }
        break;
      case ADVANCE_PURCHASE:
        LOG4CXX_DEBUG(logger, "Got ADVANCED_PURCHASE");
        break;
      case MIN_MAX_STAY:
        LOG4CXX_DEBUG(logger, "Got MIN_MAX_STAY");
        elementField->justify() = LEFT;
        ElementFilter::addOnMinMaxStay(*elementField, fareDisplayInfo);
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        if(fareDataMap)
          fareDataMap->insert(std::map<FieldColumnElement, std::string>::value_type(
              MIN_MAX_STAY, elementField->strValue()));
        break;
      case ROUTING:
        LOG4CXX_DEBUG(logger, "Got ROUTING");
        // fareDisplayInfo.addOnFareInfoPtr() = (*addOnIter);
        ElementFilter::addOnRouting(*elementField, (*addOnIter));
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case CONSTRUCTED_FARE_IND:
        LOG4CXX_DEBUG(logger, "Got CONSTRUCTED_FARE_IND");
        break;
      case ONE_WAY_TAX:
        LOG4CXX_DEBUG(logger, "Got ONE_WAY_TAX");
        break;
      case ONE_WAY_TOTAL:
        LOG4CXX_DEBUG(logger, "Got ONE_WAY_TOTAL");
        break;
      case RT_TAX:
        LOG4CXX_DEBUG(logger, "Got RT_TAX");
        break;
      case RT_TOTAL:
        LOG4CXX_DEBUG(logger, "Got RT_TOTAL");
        break;
      case DIRECTIONAL_IND:
        LOG4CXX_DEBUG(logger, "Got DIRECTIONAL_IND");
        ElementFilter::addOnDirection(*elementField, **addOnIter);
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case CARRIER:
        LOG4CXX_DEBUG(logger, "Got CARRIER");
        elementField->strValue() = (*addOnIter)->carrier();
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case ADDON_DIRECTION:
        LOG4CXX_ERROR(logger, "Got ADDON_DIRECTION");
        break;
      default:
        break;
      }
    }
    LOG4CXX_DEBUG(logger, "Line: '" << localDisplayLine->str() << "'");
    _trx.response() << localDisplayLine->str();
    if(fareDataMap)
      _trx.addFareDataMapVec(fareDataMap);
  }
}

void
AddOnFaresSection::displayMoreFaresExistMessage(FareDisplayTrx& trx) const
{
  std::ostringstream& p0 = trx.response();
  p0 << "MORE FARES EXIST - USE ADDITIONAL QUALIFIERS TO REDUCE OUTPUT";
  p0 << std::endl;
}
} // tse namespace
