//-------------------------------------------------------------------
//
//  File:     DiagFaresSection.cpp
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

#include "FareDisplay/Templates/DiagFaresSection.h"

#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/DataHandle.h"
#include "FareDisplay/Templates/DataAggregator.h"
#include "FareDisplay/Templates/ElementField.h"
#include "FareDisplay/Templates/ElementFilter.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "FareDisplay/Templates/Section.h"

#include "Common/FallbackUtil.h"

namespace tse
{
namespace
{
static const std::string PROGRAMMER_DUTY_CODE = "$";
static const std::string HDQ = "HDQ";
}


static Logger
logger("atseintl.FareDisplay.Templates.DiagFaresSection");

void
DiagFaresSection::buildDisplay()
{
  // DataHandle localDataHandle(_trx->ticketingDate());
  std::ostringstream localDisplayLine;

  LOG4CXX_INFO(logger, "In buildDisplay: columnFields size: " << _columnFields.size());
  if (_columnFields.empty())
    return;

  // Legend first
  if (_trx.getRequest()->diagnosticNumber() == DIAG_200_ID)
  {
    // Display Fail Code Legend
    _trx.response() << "FAIL CODE LEGEND -" << std::endl;
    _trx.response() << "0 - VALID FARE                1 - ACTUAL PAX TYPE NULL" << std::endl;
    _trx.response() << "2 - FARE MARKET NULL          3 - EMPTY FARE" << std::endl;
    _trx.response() << "4 - FARECLASSAPPINFO NULL     5 - FARECLASSAPPSEGINFO NULL" << std::endl;
    _trx.response() << "6 - ALL CATEGORIES NOT VALID  7 - FAILED BOOKING CODE" << std::endl;
    _trx.response() << "8 - FAILED ROUTING            9 - FARE INVALID" << std::endl;
    _trx.response() << "10- FAILED FARE GROUP        11 - WEB INVALID" << std::endl;
    _trx.response() << "12- INCLUSION CODE FAIL      13 - FAILED OUTBOUND CURRENCY" << std::endl;
    _trx.response() << "14- FARE NOT SELECTED FOR RD 15 - FARECLASS APP TAG UNAVAILABLE"
                    << std::endl;
    _trx.response() << "16- NOT VALID FOR DIRECTION  17 - FAILED FARE CLASS MATCH" << std::endl;
    _trx.response() << "18- FAILED TKTDES MATCH      19 - FAILED BOOKING CODE MATCH" << std::endl;
    _trx.response() << "20- FAILED PAX TYPE MATCH    21 - FAILED DATE CHECK" << std::endl;
    _trx.response() << "22- NOT A PRIVATE FARE       23 - NOT A PUBLIC FARE" << std::endl;
    _trx.response() << "24- INDUSTRY FARE FILTERED   25 - BASE FARE EXCEPTION THROWN" << std::endl;
    _trx.response() << "26- CAT 35 INVALID AGENCY    27 - FAILED OWRT DISPLAY TYPE" << std::endl;
    _trx.response() << "28- CAT 35 INCOMPLETE DATA   29 - CAT 35 VIEW NET INDICATOR" << std::endl;
    _trx.response() << "30- CAT 35 VALIDATION FAILED 31 - CAT 25 VALIDATION FAILED" << std::endl;
    _trx.response() << "32- MISSING FOOTNOTE DATA    33 - GLOBAL DIRECTION FAILED" << std::endl;
    _trx.response() << "34- DUPLICATE DATA           35 - MERGED FARE" << std::endl;
    _trx.response() << "36 - NOT VALID FOR CWT USER  37 - MISSING NUC RATE FOR CURRENCY"
                    << std::endl;
    _trx.response() << "38- MATCHED FARE FOCUS RULE " << std::endl;
    _trx.response() << "39- FAILED MATCH RULE TARIFF 40 - FAILED MATCH RULE NUMBER" << std::endl;
    _trx.response() << "41- FAILED MATCH FARE TYPE   42 - FAILED MATCH DISPLAY TYPE" << std::endl;
    _trx.response() << "43- FAILED PRIVATE INDICATOR 44 - FAILED MATCH CAT35 SELECT" << std::endl;
    _trx.response() << "45- FAILED CAT25 SELECT      46 - FAILED MATCH CAT15 SELECT" << std::endl;

    _trx.response() << "-------------------------------------------------------------" << std::endl;

    // Display Fare Type Legend
    _trx.response() << " " << std::endl;
    _trx.response() << "FARE TYPE LEGEND -" << std::endl;
    _trx.response() << "P - PUBLISHED FARE              D - DISCOUNTED FARE-CATS 19-23"
                    << std::endl;
    _trx.response() << "B - CAT 25 FARE BY RULE         N - NEGOTIATED CAT 35 FARE" << std::endl;
    _trx.response() << "C - CONSTRUCTED FARE            M - INDUSTRY MULTI-LATERAL FARE"
                    << std::endl;
    _trx.response() << "Z - DISCOUNTED CAT 25 FARE      Y - INDUSTRY FARE" << std::endl;
    _trx.response() << "FARE TYPE APPENDED CHARACTERS" << std::endl;
    _trx.response() << "@ NEGOTIATED CAT 35 FARE    * WEB FARE    - VALID INDUSTRY FARE"
                    << std::endl;
    _trx.response() << " " << std::endl;
  }

  // Header next
  localDisplayLine.setf(std::ios::right, std::ios::adjustfield);
  localDisplayLine << std::setw(MAX_PSS_LINE_SIZE) << std::setfill(' ') << "\n";
  for (const auto diagElementField : _columnFields)
  {
    if (diagElementField->headerPosition() == HEADER_NOT_USED)
      continue;
    localDisplayLine.seekp(diagElementField->headerPosition() - 1, std::ios_base::beg);
    localDisplayLine << diagElementField->headerText();
  }
  _trx.response() << localDisplayLine.str();

  int32_t fareCount = 0;

  for (const auto paxTypeFare : _trx.allPaxTypeFare())
  {
    if (fareCount >= MAX_DIAG_FARES)
    {
      LOG4CXX_WARN(logger, "More fares available then returned");
      break;
    }

    if (!securityCheck(paxTypeFare))
      continue;

    // FareClass Filtering (qualifier FC)
    if (!_trx.diagnostic().shouldDisplay(*paxTypeFare))
      continue;

    // FareBasisCode Filtering ( qualifier FB)
    std::map<std::string, std::string>::const_iterator diagParamIterEnd =
        _trx.diagnostic().diagParamMap().end();
    std::map<std::string, std::string>::const_iterator diagParamIter;
    diagParamIter = _trx.diagnostic().diagParamMap().find(Diagnostic::FARE_BASIS_CODE);
    if (diagParamIter != diagParamIterEnd)
    {
      if (diagParamIter->second != paxTypeFare->createFareBasisCodeFD(_trx))
        continue;
    }

    fareCount++;

    localDisplayLine.str("");
    localDisplayLine.setf(std::ios::right, std::ios::adjustfield);
    localDisplayLine << std::setw(MAX_PSS_LINE_SIZE) << std::setfill(' ') << "\n";

    std::vector<DiagElementField*>::const_iterator iter = _columnFields.begin();
    std::vector<DiagElementField*>::const_iterator iterEnd = _columnFields.end();

    int diagElement = 0;
    for (; iter != iterEnd; iter++)
    {
      diagElement = (*iter)->columnElement();
      switch (diagElement)
      {
      case LINE_NUMBER:
        LOG4CXX_DEBUG(logger, "Got LINE_NUMBER");
        (*iter)->intValue() += 1;
        (*iter)->render(&localDisplayLine, FieldValueType(INT_VALUE));
        break;
      case FARE_BASIS_TKT_DSG:
        LOG4CXX_DEBUG(logger, "Got FARE_BASIS_TKT_DSG");
        (*iter)->strValue() = paxTypeFare->createFareBasisCodeFD(_trx);
        (*iter)->render(&localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case CAT_1_IND:
      case CAT_2_IND:
      case CAT_3_IND:
      case CAT_5_IND:
      case CAT_6_IND:
      case CAT_7_IND:
      case CAT_11_IND:
      case CAT_14_IND:
      case CAT_15_IND:
      case CAT_16_IND:
      case CAT_23_IND:
        LOG4CXX_DEBUG(logger, "Got CAT_XX_IND");
        if (paxTypeFare->isCategoryProcessed(diagElement - CAT_BASE))
        {
          (*iter)->boolValue() = paxTypeFare->isCategoryValid(diagElement - CAT_BASE);
          ElementFilter::passFail(**iter);
          (*iter)->render(&localDisplayLine, FieldValueType(STRING_VALUE));
        }
        break;
      case PAX_TYPE:
        LOG4CXX_DEBUG(logger, "Got PAX_TYPE");
        if (paxTypeFare->actualPaxType() != 0)
        {
          (*iter)->strValue() = paxTypeFare->actualPaxType()->paxType();
        }
        else
        {
          (*iter)->strValue() = paxTypeFare->fcasPaxType();
        }
        ElementFilter::paxType(**iter);
        (*iter)->render(&localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case FARE_CLASS:
        LOG4CXX_DEBUG(logger, "Got FARE_CLASS");
        ElementFilter::fareClassification(**iter, *paxTypeFare);
        (*iter)->render(&localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case ORIGINAL_FARE:
        LOG4CXX_DEBUG(logger, "Got ORIGINAL_FARE");
        (*iter)->moneyValue() = paxTypeFare->originalFareAmount();
        ElementFilter::formatOriginalFare(**iter, _trx, paxTypeFare->currency());
        (*iter)->render(&localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case BASE_FARE:
        LOG4CXX_DEBUG(logger, "Got BASE_FARE");
        (*iter)->moneyValue() = paxTypeFare->nucFareAmount();
        if (_trx.getRequest()->diagnosticNumber() == DIAG_202_ID)
        {
          if (_trx.getOptions()->roundTripFare() && (paxTypeFare->owrt() == ONE_WAY_MAY_BE_DOUBLED))
            (*iter)->moneyValue() *= 2;
        }
        ElementFilter::formatMoneyAmount(**iter, *paxTypeFare, _trx);
        (*iter)->render(&localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case SURCHARGES:
        LOG4CXX_DEBUG(logger, "Got SURCHARGES");
        DataAggregator::surcharges(**iter, *paxTypeFare, _trx);
        ElementFilter::formatMoneyAmount(**iter, *paxTypeFare, _trx);
        (*iter)->render(&localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case US_TAX:
        LOG4CXX_DEBUG(logger, "Got US_TAX");
        DataAggregator::US1Taxes(**iter, *paxTypeFare, _trx);
        ElementFilter::formatMoneyAmount(**iter, *paxTypeFare, _trx);
        (*iter)->render(&localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case TOTAL_FARE:
        LOG4CXX_DEBUG(logger, "Got TOTAL_FARE");
        if (_trx.getRequest()->diagnosticNumber() == DIAG_202_ID)
        {
          DataAggregator::surcharges(**iter, *paxTypeFare, _trx);
          if (_trx.getOptions()->roundTripFare() && (paxTypeFare->owrt() == ONE_WAY_MAY_BE_DOUBLED))
            (*iter)->moneyValue() = paxTypeFare->nucFareAmount() * 2 + (*iter)->moneyValue();
          else
            (*iter)->moneyValue() = paxTypeFare->nucFareAmount() + (*iter)->moneyValue();
        }
        else
        {
          (*iter)->moneyValue() = paxTypeFare->convertedFareAmount();
        }
        ElementFilter::formatMoneyAmount(**iter, *paxTypeFare, _trx);
        (*iter)->render(&localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case JOURNEY_TYPE:
        LOG4CXX_DEBUG(logger, "Got JOURNEY_TYPE");
        ElementFilter::journeyType(**iter, *paxTypeFare, _trx);
        (*iter)->render(&localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case FARE_TYPE_CODE:
        LOG4CXX_DEBUG(logger, "Got FARE_TYPE_CODE");
        (*iter)->strValue() = paxTypeFare->fcaFareType().c_str();
        (*iter)->render(&localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case DISPLAY_TYPE:
        LOG4CXX_DEBUG(logger, "Got DISPLAY_TYPE");
        ElementFilter::displayCatType(**iter, *paxTypeFare);
        (*iter)->render(&localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case DISPLAY_CURRENCY_CODE:
        LOG4CXX_DEBUG(logger, "Got DISPLAY_CURRENCY_CODE");
        ElementFilter::currencyCode(**iter, _trx);
        (*iter)->render(&localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case FARE_CURRENCY_CODE:
        LOG4CXX_DEBUG(logger, "Got FARE_CURRENCY_CODE");
        (*iter)->strValue() = paxTypeFare->currency();
        (*iter)->render(&localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case CONVERTED_FARE:
        LOG4CXX_DEBUG(logger, "Got CONVERTED_FARE");
        (*iter)->moneyValue() = paxTypeFare->nucFareAmount();
        ElementFilter::formatMoneyAmount(**iter, *paxTypeFare, _trx);
        (*iter)->render(&localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      case FAIL_CODE:
        LOG4CXX_DEBUG(logger, "Got FAIL_CODE");
        ElementFilter::getFailCode(**iter, *paxTypeFare);
        (*iter)->render(&localDisplayLine, FieldValueType(STRING_VALUE));
        break;
      default:
        LOG4CXX_WARN(logger, "Unknown element: " << diagElement);
        break;
      }
    }
    LOG4CXX_DEBUG(logger, "Line: '" << localDisplayLine.str() << "'");
    _trx.response() << localDisplayLine.str();
  }
}
bool
DiagFaresSection::securityCheck(PaxTypeFare* ptf)
{
  // Bypass security check if agent has programmer duty code or HDQ home city
  if (_trx.getRequest()->ticketingAgent()->agentDuty() == PROGRAMMER_DUTY_CODE ||
      _trx.billing()->userPseudoCityCode() == HDQ)
  {
    return true;
  }

  if (ptf->isCategoryProcessed(CAT_15_IND - CAT_BASE))
  {
    if (ptf->isCategoryValid(CAT_15_IND - CAT_BASE))
      return true;
    else
      return false;
  }

  return true;
}
}
