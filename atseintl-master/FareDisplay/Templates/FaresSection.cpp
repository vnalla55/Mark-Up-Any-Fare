//-------------------------------------------------------------------
//
//  File:     FaresSection.cpp
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

#include "FareDisplay/Templates/FaresSection.h"

#include "BookingCode/FareDisplayBookingCode.h"
#include "Common/BrandingUtil.h"
#include "Common/FareDisplayTax.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TSEException.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/FareRetailerRuleInfo.h"  
#include "FareDisplay/InclusionCodeConsts.h"
#include "FareDisplay/Templates/ElementField.h"
#include "FareDisplay/Templates/ElementFilter.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "FareDisplay/Templates/Section.h"

#include <vector>

namespace tse
{
FALLBACK_DECL(fallbackFareDisplayByCabinActivation);
FALLBACK_DECL(fallbackFRRProcessingRetailerCode);

static Logger
logger("atseintl.FareDisplay.Templates.FaresSection");

void
FaresSection::buildDisplay()
{
  DataHandle localDataHandle(_trx.ticketingDate());
  std::ostringstream* localDisplayLine;

  LOG4CXX_DEBUG(logger, "In buildDisplay: columnFields size: " << _columnFields.size());
  if (_columnFields.empty())
    return;

  uint32_t fareCount = 0;

  // Call filter, if one is available and check for Carrier fare header
  if (_sectionFilter != nullptr)
  {
    FareDisplayUtil::getCarrierFareHeaderMsg(_trx, _trx.response());
  }

  // If no filter is applied, output column header
  if (_sectionFilter == nullptr && _headerSection != nullptr)
    _headerSection->buildDisplay();

  // Add surcharge info line with FL entries for online agencies
  if (_trx.getOptions()->applySurcharges() == YES &&
      _trx.getOptions()->displayBaseTaxTotalAmounts() == TRUE_INDICATOR)
  {
    _trx.response() << SPACE << std::endl;
    _trx.response() << SURCHARGE_INFO << std::endl;
    _trx.response() << SPACE << std::endl;
  }

  // Get date filter values
  Indicator seasonDF = getDateFilter(FieldColumnElement(SEASONS));

  // Do we have Multiple Globals for Industry Fare?
  bool hasMultipleGlobalForYY = hasMultipleGlobal(INDUSTRY_CARRIER);

  bool firstGroup = true;

  // If fares with multiple currencies are selected for display, nonCOCCurrency will
  // be used by ElementFilter::nonCOCCurrency to determine if the NONCOC Currency asterisk
  // indicator is to be displayed.  Otherwise, nonCOCCurrency will be blanked and
  // cause the NONCOC Currency Indicator processing to be skipped.
  CurrencyCode nonCOCCurrency;
  Itin* itin = _trx.itin().front();

  nonCOCCurrency = itin->originationCurrency();

  if (!_trx.multipleCurrencies() && !_trx.isRD())
  {
    nonCOCCurrency.clear();
  }

  uint32_t fareLevelCount = 0;
  MoneyAmount fareAmt = 0;
  const uint32_t numFareLevelRequest = _trx.getRequest()->numberOfFareLevels();
  const uint32_t maxFares = FareDisplayUtil::getMaxFares();
  bool populateFareDataMapVec = BrandingUtil::isPopulateFareDataMapVec(_trx);

  // If Cabin in the request, then check for brands in all fares
  // No brands - display Cabin Header, otherwise do not display
  allowDisplayCabinHeader();
  // Price By Cabin project to display NONE with the cabin verbiage
  if (!fallback::fallbackFareDisplayByCabinActivation(&_trx) &&
      _trx.getRequest()->requestType() == FARE_DISPLAY_REQUEST &&
      _trx.getRequest()->multiInclusionCodes() && !_trx.allPaxTypeFare().empty() &&
      _sectionFilter != nullptr && _trx.fdResponse()->isDisplayHeaderCabin())
  {
    uint8_t inclusionCabinNum = _trx.allPaxTypeFare().front()->fareDisplayInfo()->inclusionCabinNum();
    if(inclusionCabinNum != _trx.fdResponse()->multiInclusionCabins().at(0) )
    {
      auto foundIt = std::find(std::begin(_trx.fdResponse()->multiInclusionCabins()),
                               std::end(_trx.fdResponse()->multiInclusionCabins()), inclusionCabinNum);
      if(foundIt != std::end(_trx.fdResponse()->multiInclusionCabins()))
      {
        size_t dist = std::distance(std::begin(_trx.fdResponse()->multiInclusionCabins()), foundIt);
        for(size_t i = 0; i < dist; ++i)
          addCabinSectionText(_trx.fdResponse()->multiInclusionCabins().at(i), true);
      }
    }
  }

  std::vector<PaxTypeFare*>::const_iterator ptfIter = _trx.allPaxTypeFare().begin();
  std::vector<PaxTypeFare*>::const_iterator ptfIterEnd = _trx.allPaxTypeFare().end();
  for (; ptfIter != ptfIterEnd; ptfIter++)
  {
    if (fareCount >= maxFares)
    {
      LOG4CXX_WARN(logger, "More fares available then returned");
      displayMoreFaresExistMessage(_trx);
      break;
    }
    else
      fareCount++;

    if (numFareLevelRequest > 0)
    {
      MoneyAmount totalFareAmt = (*ptfIter)->convertedFareAmount();
      if ((*ptfIter)->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
        totalFareAmt *= 2;
      else if ((*ptfIter)->owrt() == ONE_WAY_MAY_BE_DOUBLED)
        totalFareAmt = (*ptfIter)->convertedFareAmountRT();

      if (fareAmt != totalFareAmt)
      {
        fareAmt = totalFareAmt;
        fareLevelCount++;
      }
      if (fareLevelCount > numFareLevelRequest)
        break;
    }

    // Call filter, if one is available
    if (_sectionFilter != nullptr)
    {
      bool ret = _sectionFilter->doFiltering(**ptfIter, firstGroup, hasMultipleGlobalForYY);
      if (ret)
        firstGroup = false;
      if (ret || ptfIter == _trx.allPaxTypeFare().begin())
      {
        if (_headerSection != nullptr)
          _headerSection->buildDisplay();
      }
    }

    localDataHandle.get(localDisplayLine);
    initializeLine(localDisplayLine, 1);
    uint16_t line = 1;
    int fareBasisPosition = 0;
    std::map<FieldColumnElement, std::string> *fareDataMap = nullptr;
    if (populateFareDataMapVec)
      _trx.dataHandle().get(fareDataMap);

    for (ElementField* const elementField : _columnFields)
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
        LOG4CXX_DEBUG(logger, "Got SAME_DAY_IND...not done");
        {
          FareDisplayInfo* fdi = (*ptfIter)->fareDisplayInfo();
          ElementFilter::sameDayChange(*elementField, fdi);
          elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
          if (fareDataMap)
            fareDataMap->insert(std::map<FieldColumnElement, std::string>::value_type(
                SAME_DAY_IND, elementField->strValue()));
        }
        break;
      case VENDOR_TRAVELOCITY_CODE:
        LOG4CXX_DEBUG(logger, "Got VENDOR_TRAVELOCITY_CODE");
        elementField->strValue() = (*ptfIter)->vendor();
        ElementFilter::vendorCode(*elementField, _trx, (*ptfIter)->vendorFWS());
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case PRIVATE_FARE_IND:
        LOG4CXX_DEBUG(logger, "Got PRIVATE_FARE_IND");
        {
          ElementFilter::privateFareIndicator(*elementField, **ptfIter, _trx);
          elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        }
        break;
      case FARE_BASIS_TKT_DESIG:
        LOG4CXX_DEBUG(logger, "Got FARE_BASIS_TKT_DESIG");
        if (_trx.isERD() && _trx.getRequest()->uniqueFareBasisCode().length())
        {
          elementField->strValue() = _trx.getRequest()->uniqueFareBasisCode().c_str();
        }
        else
        {
          elementField->strValue() = (*ptfIter)->createFareBasisCodeFD(_trx);
        }
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));

        fareBasisPosition = elementField->valuePosition();
        break;
      case BOOKING_CODE:
        LOG4CXX_DEBUG(logger, "Got BOOKING_CODE");
        elementField->strValue() = (*ptfIter)->bookingCode();
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case JOURNEY_IND:
        LOG4CXX_DEBUG(logger, "Got JOURNEY_IND");
        ElementFilter::journeyType(*elementField, **ptfIter, _trx);
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        if(fareDataMap)
          fareDataMap->insert(std::map<FieldColumnElement, std::string>::value_type(
              JOURNEY_IND, elementField->strValue()));
        break;
      case FD_FARE_AMOUNT:
        LOG4CXX_DEBUG(logger, "Got FARE_AMOUNT");

        elementField->moneyValue() = (*ptfIter)->convertedFareAmount();

        if (_trx.getOptions()->roundTripFare() && ((*ptfIter)->owrt() == ONE_WAY_MAY_BE_DOUBLED))
          ElementFilter::roundTripAmount(*elementField, **ptfIter, _trx);
        else if (_trx.getOptions()->halfRoundTripFare() &&
                 (*ptfIter)->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
          ElementFilter::halfRoundTripAmount(*elementField, **ptfIter, _trx);
        else
          ElementFilter::formatMoneyAmount(*elementField, _trx);

        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case OW_FARE_AMOUNT:
        LOG4CXX_DEBUG(logger, "Got OW_FARE_AMOUNT");

        if (_trx.getOptions()->halfRoundTripFare())
          break;

        elementField->moneyValue() = (*ptfIter)->convertedFareAmount();
        ElementFilter::onewayTripAmount(*elementField, **ptfIter, _trx);
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case RT_FARE_AMOUNT:
        LOG4CXX_DEBUG(logger, "Got RT_FARE_AMOUNT");

        if (_trx.getOptions()->halfRoundTripFare())
          ElementFilter::halfRoundTripAmount(*elementField, **ptfIter, _trx);
        else
          ElementFilter::roundTripAmount(*elementField, **ptfIter, _trx);

        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case NET_FARE_IND:
        LOG4CXX_DEBUG(logger, "Got NET_FARE_IND");
        ElementFilter::netFareIndicator(*elementField, **ptfIter, _trx);
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        if(fareDataMap)
          fareDataMap->insert(std::map<FieldColumnElement, std::string>::value_type(
              NET_FARE_IND, elementField->strValue()));
        break;
      case TRAVEL_TICKET:
        LOG4CXX_DEBUG(logger, "Got TRAVEL_TICKET");
        {
          elementField->justify() = LEFT;
          ElementField field2;
          FareDisplayInfo* fdi = (*ptfIter)->fareDisplayInfo();
          ElementFilter::travelTicket(*elementField, field2, fdi);
          elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));

          if (!field2.strValue().empty())
          {
            field2.valueFieldSize() = elementField->valueFieldSize();
            field2.valuePosition() = MAX_PSS_LINE_SIZE + elementField->valuePosition();
            field2.justify() = elementField->justify();
            if (1 == line)
            {
              initializeLine(localDisplayLine, ++line);
            }
            field2.render(localDisplayLine, FieldValueType(STRING_VALUE));
          }
        }
        break;
      case IN_OUT_INDICATOR:
        LOG4CXX_DEBUG(logger, "Got IN_OUT_INDICATOR");
        {
          std::vector<ElementField> fields;
          FareDisplayInfo* fdi = (*ptfIter)->fareDisplayInfo();
          ElementFilter::inOutInd(fields, *elementField, fdi, seasonDF);
          elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));

          for (uint16_t i = 1; i < fields.size(); ++i)
          {
            ElementField& nextField = fields[i];

            nextField.valueFieldSize() = elementField->valueFieldSize();
            nextField.valuePosition() = (MAX_PSS_LINE_SIZE * i) + elementField->valuePosition();
            nextField.justify() = elementField->justify();
            if (i >= line)
            {
              initializeLine(localDisplayLine, ++line);
            }
            LOG4CXX_DEBUG(logger, nextField.strValue());

            nextField.render(localDisplayLine, FieldValueType(STRING_VALUE));
            LOG4CXX_DEBUG(logger, localDisplayLine->str());
          }
        }
        break;

      case SEASONS:
        LOG4CXX_DEBUG(logger, "Got SEASONS");
        {
          elementField->justify() = LEFT;
          std::vector<ElementField> fields;
          FareDisplayInfo* fdi = (*ptfIter)->fareDisplayInfo();
          ElementFilter::season(fields, *elementField, fdi, _trx, seasonDF);
          elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));

          for (uint16_t i = 1; i < fields.size(); ++i)
          {
            ElementField& nextField = fields[i];

            nextField.valueFieldSize() = elementField->valueFieldSize();
            nextField.valuePosition() = (MAX_PSS_LINE_SIZE * i) + elementField->valuePosition();
            nextField.justify() = elementField->justify();
            if (i >= line)
            {
              initializeLine(localDisplayLine, ++line);
            }

            nextField.render(localDisplayLine, FieldValueType(STRING_VALUE));
          }
        }
        break;
      case ADVANCE_PURCHASE:
        LOG4CXX_DEBUG(logger, "Got ADVANCED_PURCHASE");
        {
          elementField->justify() = LEFT;
          FareDisplayInfo* fdi = (*ptfIter)->fareDisplayInfo();
          ElementFilter::advancePurchase(*elementField, fdi);
          elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
          if (fareDataMap)
            fareDataMap->insert(std::map<FieldColumnElement, std::string>::value_type(
                ADVANCE_PURCHASE, elementField->strValue()));
        }
        break;
      case MIN_MAX_STAY:
        LOG4CXX_DEBUG(logger, "Got MIN_MAX_STAY");
        {
          elementField->justify() = LEFT;
          FareDisplayInfo* fdi = (*ptfIter)->fareDisplayInfo();
          ElementFilter::minMaxStay(*elementField, fdi);
          elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
          if (fareDataMap)
            fareDataMap->insert(std::map<FieldColumnElement, std::string>::value_type(
                MIN_MAX_STAY, elementField->strValue()));
        }
        break;
      case ROUTING:
        LOG4CXX_DEBUG(logger, "Got ROUTING");
        ElementFilter::routing(*elementField, **ptfIter, _trx);
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case CONSTRUCTED_FARE_IND:
        LOG4CXX_WARN(logger, "Got CONSTRUCTED_FARE_IND...not in FRD");
        break;
      case ONE_WAY_TAX:
        LOG4CXX_DEBUG(logger, "Got ONE_WAY_TAX");
        ElementFilter::getAmount(*elementField, **ptfIter, _trx, ONE_WAY_TAX);
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case ONE_WAY_TOTAL:
        LOG4CXX_DEBUG(logger, "Got ONE_WAY_TOTAL");
        ElementFilter::getAmount(*elementField, **ptfIter, _trx, ONE_WAY_TOTAL);
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case RT_TAX:
        LOG4CXX_DEBUG(logger, "Got RT_TAX");
        ElementFilter::getAmount(*elementField, **ptfIter, _trx, RT_TAX);
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case RT_TOTAL:
        LOG4CXX_DEBUG(logger, "Got RT_TOTAL");
        ElementFilter::getAmount(*elementField, **ptfIter, _trx, RT_TOTAL);
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case DIRECTIONAL_IND:
        LOG4CXX_DEBUG(logger, "Got DIRECTIONAL_IND...not done");
        ElementFilter::directionality(*elementField, **ptfIter);
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case CARRIER:
        LOG4CXX_DEBUG(logger, "Got CARRIER");
        elementField->strValue() = (*ptfIter)->carrier();
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case ADDON_DIRECTION:
        LOG4CXX_DEBUG(logger, "Got ADDON_DIRECTION...not done");
        break;
      case NON_COC_CURRENCY:
        // displayCurrency will be blank if single currency found
        if (nonCOCCurrency.empty())
        {
          break;
        }

        LOG4CXX_DEBUG(logger, "Got NON_COC CURRENCY");
        ElementFilter::nonCOCCurrency(*elementField, **ptfIter, nonCOCCurrency);
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;

      case NET_FARE_NON_COC_IND:
        LOG4CXX_DEBUG(logger, "Got Net Fare/NonCOC...not done");
        ElementFilter::netFareNONCOC(*elementField, **ptfIter, _trx, nonCOCCurrency);
        elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        break;

      case FARE_CONSTRUCTION_IND:

        if ((*ptfIter)->isConstructed())
        {
          LOG4CXX_DEBUG(logger, "Got FARE_CONSTRUCTION_IND");
          ElementFilter::fareConstrInd(*elementField);
          elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        }

        break;
      case FARE_BY_RULE_IND:
        {
          if(!TrxUtil::isFqFareRetailerEnabled(_trx))
            break;
          LOG4CXX_DEBUG(logger, "Got FARE BY RULE IND");
          ElementFilter::fareByRuleInd(*elementField, **ptfIter);
          elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        }
        break;
      case RULE_TARIFF:
        {
          if(!TrxUtil::isFqFareRetailerEnabled(_trx))
            break;
          LOG4CXX_DEBUG(logger, "Got FARE TARIFF");
          ElementFilter::ruleTariff(*elementField, **ptfIter);
          elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        }
        break;
      case FARE_RULE:
        {
          if(!TrxUtil::isFqFareRetailerEnabled(_trx))
            break;
          LOG4CXX_DEBUG(logger, "Got FARE RULE");
          ElementFilter::fareRule(*elementField, **ptfIter);
          elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        }
        break;
      case FARE_TYPE:
        {
          if(!TrxUtil::isFqFareRetailerEnabled(_trx))
            break;
          LOG4CXX_DEBUG(logger, "Got FARE TYPE");
          ElementFilter::fareType(*elementField, **ptfIter);
          elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        }
        break;
      case ROUTING_TYPE_NUMBER:
        {
          if(!TrxUtil::isFqFareRetailerEnabled(_trx))
            break;
          LOG4CXX_DEBUG(logger, "Got ROUTING TYPE/NUMBER");
          ElementFilter::routingTypeNumber(*elementField, **ptfIter, _trx);
          elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        }
        break;
      case REDISTRIBUTION:
        {
          if(!TrxUtil::isFqFareRetailerEnabled(_trx))
            break;
          LOG4CXX_DEBUG(logger, "Got REDISTRIBUTION PCC");
          ElementFilter::redistribution(*elementField, **ptfIter, _trx);
          elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        }
        break;
      case DISPLAY_CAT_TYPE:
        {
          if(!TrxUtil::isFqFareRetailerEnabled(_trx))
            break;
          LOG4CXX_DEBUG(logger, "Got DISPLAY CAT TYPE IND");
          ElementFilter::displayCatType(*elementField, **ptfIter);
          elementField->render(localDisplayLine, FieldValueType(STRING_VALUE));
        }
        break;
      default:
        break;
      }
    }

    if (displayAccountCode(**ptfIter))
    {
      if (line == 1)
        initializeLine(localDisplayLine, ++line);

      ElementField accCodeField;
      accCodeField.initialize(UNKNOWN_ELEMENT,
                              fareBasisPosition + MAX_PSS_LINE_SIZE,
                              ACCOUNT_CODE_LEN,
                              JustificationType(LEFT));

      fareBasisPosition = accCodeField.valuePosition();

      if (!(*ptfIter)->matchedAccCode().empty())
      {
        accCodeField.strValue() = (*ptfIter)->matchedAccCode();
      }
      else
      {
        const FBRPaxTypeFareRuleData* fbrData = (*ptfIter)->getFbrRuleData(RuleConst::FARE_BY_RULE);
        TSE_ASSERT(fbrData);
        accCodeField.strValue() = fbrData->fbrApp()->accountCode().c_str();
      }

      accCodeField.render(localDisplayLine, FieldValueType(STRING_VALUE));
    }

    if (!fallback::fallbackFRRProcessingRetailerCode(&_trx))
    {
      if (displayRetailerCode(**ptfIter))
      {
        std::string sRetailerCode = "";

        if ((*ptfIter)->hasCat35Filed())
        {
          const NegPaxTypeFareRuleData* negRuleData = (*ptfIter)->getNegRuleData();
          if (negRuleData && !negRuleData->sourcePseudoCity().empty() &&
              !negRuleData->fareRetailerCode().empty())
           {
             sRetailerCode = "RRQ" + negRuleData->fareRetailerCode();
           }
        }

        const AdjustedSellingCalcData* adjSellingCalcData  = (*ptfIter)->getAdjustedSellingCalcData();
        if (adjSellingCalcData)
        {
          if (adjSellingCalcData->getFareRetailerRuleInfo() &&
              !adjSellingCalcData->getSourcePcc().empty() &&
              !adjSellingCalcData->getFareRetailerRuleInfo()->fareRetailerCode().empty())
          {
            if (sRetailerCode.length() > 0)
              sRetailerCode += " ";
            sRetailerCode += "RRQ" + adjSellingCalcData->getFareRetailerRuleInfo()->fareRetailerCode();
          }
        }

        if (sRetailerCode.length() > 0)
        {
          initializeLine(localDisplayLine, ++line);
          ElementField retCodeField1;
          retCodeField1.initialize(UNKNOWN_ELEMENT,
                                  fareBasisPosition + MAX_PSS_LINE_SIZE,
                                  sRetailerCode.length(),
                                  JustificationType(LEFT));

          retCodeField1.strValue() = sRetailerCode;
          retCodeField1.render(localDisplayLine, FieldValueType(STRING_VALUE));
          fareBasisPosition = retCodeField1.valuePosition();
        }
      }
    }

    LOG4CXX_DEBUG(logger, "Line: '" << localDisplayLine->str() << "'");
    _trx.response() << localDisplayLine->str();
    if(fareDataMap)
      _trx.addFareDataMapVec(fareDataMap);
  }
  // Price By Cabin project to display NONE with the cabin verbiage

  if (!fallback::fallbackFareDisplayByCabinActivation(&_trx) &&
      _trx.getRequest()->requestType() == FARE_DISPLAY_REQUEST &&
      _trx.getRequest()->multiInclusionCodes() &&
      !_trx.allPaxTypeFare().empty() && _trx.fdResponse()->isDisplayHeaderCabin())
  {
    uint8_t inclusionCabinNum = _trx.allPaxTypeFare().back()->fareDisplayInfo()->inclusionCabinNum();
    uint16_t count = _trx.fdResponse()->multiInclusionCabins().size();

    if(inclusionCabinNum != _trx.fdResponse()->multiInclusionCabins().at(count-1) )
    {
      auto foundIt = std::find(std::begin(_trx.fdResponse()->multiInclusionCabins()),
                               std::end(_trx.fdResponse()->multiInclusionCabins()), inclusionCabinNum);
      if(foundIt != std::end(_trx.fdResponse()->multiInclusionCabins()))
      {
        size_t dist = std::distance(std::begin(_trx.fdResponse()->multiInclusionCabins()), foundIt);
        for(dist++; dist < count; ++dist)
          addCabinSectionText(_trx.fdResponse()->multiInclusionCabins().at(dist));
      }
    }
  }

  // Call filter, if one is available
  if (_sectionFilter != nullptr)
  {
    _sectionFilter->displayYYHdrMsg();
  }
}

void
FaresSection::addCabinSectionText(uint8_t cabinInclusionNum, bool top)
{
  LOG4CXX_DEBUG(logger, "In addCabinSectionText");

  // Get the description for the Cabin inclusion code
  const std::string cabinVerbiage = _trx.getRequest()->inclusionVerbiage(cabinInclusionNum);
  if (!cabinVerbiage.empty())
  {
    if(top)
    {
      _trx.response() << cabinVerbiage << "\n"<< "  NONE" <<std::endl;
      _trx.response() << " " << std::endl;
    }
    else
    {
      _trx.response() << " " << std::endl;
      _trx.response() << cabinVerbiage << "\n"<< "  NONE" <<std::endl;
    }
  }
  else
    _trx.response() << " " << std::endl;
}

bool
FaresSection::displayAccountCode(const PaxTypeFare& paxTypeFare) const
{
  bool isFBRData = false;

  FBRPaxTypeFareRuleData* fbrData = paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);
  if (fbrData != nullptr && !fbrData->fbrApp()->accountCode().empty())
  {
    isFBRData = true;
  }

  return _trx.getRequest()->isMultiAccCorpId() && _trx.getRequest()->displayAccCode() &&
         (!paxTypeFare.matchedAccCode().empty() || isFBRData);
}

bool
FaresSection::displayRetailerCode(const PaxTypeFare& paxTypeFare) const
{
  return !_trx.getRequest()->rcqValues().empty() && _trx.getRequest()->displayAccCode();
}

Indicator
FaresSection::getDateFilter(const FieldColumnElement& fieldElement)
{
  for (const auto fareDisplayTemplateSeg : *_templateSegRecs)
  {
    if (fieldElement == fareDisplayTemplateSeg->columnElement())
      return fareDisplayTemplateSeg->elementDateFormat();
  }
  return 0;
}

void
FaresSection::displayMoreFaresExistMessage(FareDisplayTrx& trx) const
{
  std::ostringstream& p0 = trx.response();
  p0 << "MORE FARES EXIST - USE ADDITIONAL QUALIFIERS TO REDUCE OUTPUT";
  p0 << std::endl;
}

bool
FaresSection::hasMultipleGlobal(const CarrierCode& carrierCode)
{
  GlobalDirection previousGlobalDir = GlobalDirection::NO_DIR;
  int32_t iFound = 0;
  for (const auto paxTypeFare : _trx.allPaxTypeFare())
  {
    if (paxTypeFare->carrier() == carrierCode &&
        previousGlobalDir != paxTypeFare->globalDirection())
    {
      previousGlobalDir = paxTypeFare->globalDirection();
      iFound++;
    }
    if (iFound > 1)
      return true;
  }
  return false;
}

void
FaresSection::allowDisplayCabinHeader()
{
  bool cabinGroupExists = false;
  bool brandGroupExists = false;
  for (auto groupType : _trx.fdResponse()->groupHeaders())
  {
    if(groupType == Group::GROUP_BY_CABIN)
      cabinGroupExists = true;
    if(groupType == Group::GROUP_BY_S8BRAND)
      brandGroupExists = true;
  }
  if(!cabinGroupExists)
  {
    _trx.fdResponse()->setDoNotDisplayHeaderCabin();
    return;
  }

  if(brandGroupExists && _trx.fdResponse()->programBrandNameMap().empty())
    return;

  for (auto ptf : _trx.allPaxTypeFare())
  {
    std::pair<ProgramCode, BrandCode> brand = ptf->fareDisplayInfo()->programBrand();
    if(!ptf->fareDisplayInfo()->brandCode().empty() ||
       !brand.first.empty() || !brand.second.empty())
    {
      _trx.fdResponse()->setDoNotDisplayHeaderCabin();
      break;
    }
  }
  return;
}

} // tse namespace
