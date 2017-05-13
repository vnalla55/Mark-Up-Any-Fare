//-------------------------------------------------------------------
//
//  File:        ERDRequestProcessor.cpp
//  Created:     October 26, 2008
//  Authors:     Konrad Koch
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

#include "Xform/ERDRequestProcessor.h"

#include "Common/FareCalcUtil.h"
#include "Common/Money.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/PaxTypeUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/ERDFareComp.h"
#include "DataModel/ERDFltSeg.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/Itin.h"
#include "DBAccess/FareCalcConfig.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"

namespace tse
{
// to fix compile error
static const char* NO_DATA_U = "INVALID XML REQUEST - NO PRICING RESPONSE DATA";
static const std::string ERROR_INVALID_ENTRY = "INVALID ENTRY - PLEASE SEE WPRDHELP";
static const std::string ERROR_INVALID_ENTRY_WQ = "INVALID ENTRY - PLEASE SEE WQRDHELP";
static const std::string ERROR_DIFFERENT_RULE =
    "MULTIPLE RULE CONDITIONS - SEGMENT SELECT. SEE WPRDHELP";
static const std::string ERROR_DIFFERENT_RULE_WQ =
    "MULTIPLE RULE CONDITIONS - SEGMENT SELECT. SEE WQRDHELP";
static const std::string ERROR_SABRE_MULTI_PAX_TYPE =
    "MULT PSGR TYPES - MUST ADD P FOLLOWED BY PTC AFTER WPRD*";
static const std::string ERROR_ABACUS_MULTI_PAX_TYPE =
    "MULTIPLE PSGR TYPES - MUST ADD 1, 2, 3 OR 4 AFTER WPRD*";
static const std::string ERROR_NO_FARE_BASIS =
    "VERIFY FARE BASIS CODES - SEGMENT SELECT. SEE WPRDHELP";
static const std::string ERROR_NO_FARE_BASIS_WQ =
    "VERIFY FARE BASIS CODES - SEGMENT SELECT. SEE WQRDHELP";
static const std::string ERROR_NO_SEGMENT = "REQUEST MUST INCLUDE FLIGHT SEGMENT";
static const std::string ERROR_NO_PRICED_SEGMENT = "INCORRECT SEGMENT NUMBER - MODIFY AND REENTER";
static const std::string ERROR_NO_PTC_SABRE = "VERIFY PTC CODE REQUESTED";
static const std::string ERROR_NO_PTC_ABACUS = "VERIFY PTC NUMBER REQUESTED";
static const std::string ERROR_MULTIPLE_CODES = "*M, *H, *RTG NOT ALLOWED WITH OTHER CODES";
static const std::string ERROR_INVALID_LINE = "INCORRECT LINE NUMBER - MODIFY AND REENTER";
static const std::string ERROR_INVALID_OPTION = "$INVLD DISPLAY OPTION$";
static const std::string ERROR_ALPHA_FB = "ALPHA CODES NOT ALLOWED WITH FB ENTRY";
static const std::string ERROR_MAX_ALPHA = "MAX 5 ALPHA CODES ALLOWED";
static const std::string COLUMN_SPACING = "   ";
static const std::string COLUMN_SPACING2 = "  ";
static const std::string SUM_TAG_START = "<SUM";
static const std::string SUM_TAG_END = "</SUM>";

static const char PASSENGER_NOT_FIRST = 'P';
static const char MULTIPLE_PASSENGERS = 'Q';
static const char SEGMENT_NOT_EXIST = 'S';
static const char TWO_FBC_REQUSTED = 'F';
static const char ALPHA_AND_CAT_REQUESTED = 'C';
static const char M_H_RTG_COMBINED = 'M';
static const char INVALID_DISPLAY_OPTION = 'I';
static const char ALPHA_AND_FB_REQUESTED = 'X';
static const char MAX_ALPHA = 'A';

//----------------------------------------------------------------------------
// ERDRequestProcessor::ERDRequestProcessor
//----------------------------------------------------------------------------
ERDRequestProcessor::ERDRequestProcessor(FareDisplayTrx& fareDisplayTrx)
  : _trx(fareDisplayTrx), _useInternationalRounding(false)
{
}

//----------------------------------------------------------------------------
// ERDRequestProcessor::ERDRequestProcessor
//----------------------------------------------------------------------------
ERDRequestProcessor::~ERDRequestProcessor()
{
}

//----------------------------------------------------------------------------
// ERDRequestProcessor::process
//----------------------------------------------------------------------------
bool
ERDRequestProcessor::process()
{
  if (_trx.getOptions()->erdFareComps().empty())
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, NO_DATA_U);

  FareClassCode requestedFareBasis = _trx.getRequest()->fareBasisCode();
  _fareComponents = _trx.getOptions()->erdFareComps();

  calculateInternationalRoundingValue();

  completeFareBreaks();

  if (!validatePassenger())
  {
    return false;
  }

  // Remember FC's list just after filtering by Pax Type
  _originalFareComponents = _fareComponents;

  if (!validateSegments(requestedFareBasis) || !filterByFareBasisCode(requestedFareBasis) ||
      !validateFareComponents() || !validateRDErrors() || !storeResults())
  {
    return false;
  }

  return true;
}
//----------------------------------------------------------------------------
// ERDRequestProcessor::findSegment
//----------------------------------------------------------------------------
ERDFltSeg*
ERDRequestProcessor::findSegment(uint16_t number) const
{
  std::vector<ERDFareComp*>::const_iterator fcIter = _fareComponents.begin();
  std::vector<ERDFareComp*>::const_iterator fcIterEnd = _fareComponents.end();

  for (; fcIter != fcIterEnd; ++fcIter)
  {
    ERDFareComp* currentFC = *fcIter;

    if (!currentFC)
      continue;

    std::vector<ERDFltSeg*>::const_iterator fsIter = find_if(currentFC->segments().begin(),
                                                             currentFC->segments().end(),
                                                             ERDFltSeg::MatchSegmentNumber(number));

    if (fsIter != currentFC->segments().end())
      return *fsIter;
  }
  return nullptr;
}
//----------------------------------------------------------------------------
// ERDRequestProcessor::removeSurfaceSegments
//----------------------------------------------------------------------------
void
ERDRequestProcessor::removeSurfaceSegments(std::vector<uint16_t>& requestedSNs) const
{
  if (_trx.getOptions()->isWqrd())
  {
    int16_t maxSegNumber = _fareComponents.back()->segments().back()->itinSegNumber();

    std::vector<uint16_t> copyOfSNs(requestedSNs);
    std::vector<uint16_t>::iterator snIter = copyOfSNs.begin();
    std::vector<uint16_t>::iterator snIterEnd = copyOfSNs.end();

    for (; snIter != snIterEnd; ++snIter)
    {
      if (*snIter <= maxSegNumber &&
          find_if(_fareComponents.begin(),
                  _fareComponents.end(),
                  ERDFareComp::MatchSegmentNumbers(*snIter)) == _fareComponents.end())
      {
        requestedSNs.erase(find(requestedSNs.begin(), requestedSNs.end(), *snIter));
      }
    }
  }
  else
  {
    // Remove known surface segments from requested segments
    std::vector<uint16_t>::const_iterator ssIter = _trx.getOptions()->surfaceSegments().begin();
    std::vector<uint16_t>::const_iterator ssIterEnd = _trx.getOptions()->surfaceSegments().end();

    for (; ssIter != ssIterEnd; ++ssIter)
    {
      std::vector<uint16_t>::iterator surfaceSegment =
          find(requestedSNs.begin(), requestedSNs.end(), *ssIter);

      if (surfaceSegment != requestedSNs.end())
      {
        requestedSNs.erase(surfaceSegment);
      }
    }
  }
}

//----------------------------------------------------------------------------
// ERDRequestProcessor::checkSegments
//----------------------------------------------------------------------------
bool
ERDRequestProcessor::checkSegments(std::vector<uint16_t>& requestedSNs) const
{
  removeSurfaceSegments(requestedSNs);

  if (requestedSNs.empty())
  {
    buildErrorMessage(ERROR_NO_SEGMENT);
    return false;
  }

  // Check if all requested segments are available
  std::vector<uint16_t>::const_iterator snIter = requestedSNs.begin();
  std::vector<uint16_t>::const_iterator snIterEnd = requestedSNs.end();

  for (; snIter != snIterEnd; ++snIter)
  {
    if (findSegment(*snIter) == nullptr)
    {
      if (_trx.getOptions()->isWqrd())
        buildErrorMessage(ERROR_NO_SEGMENT, true);
      else
        buildErrorMessage(ERROR_NO_PRICED_SEGMENT, false);
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------------------
// ERDRequestProcessor::multiplePaxTypesInItin
//----------------------------------------------------------------------------
bool
ERDRequestProcessor::multiplePaxTypesLeft() const
{
  PaxTypeCode firstPaxTypeCode = _fareComponents.front()->paxTypeCode();

  std::vector<ERDFareComp*>::const_iterator fcIter =
      find_if(_fareComponents.begin(),
              _fareComponents.end(),
              not1(ERDFareComp::MatchPaxTypeCode(firstPaxTypeCode)));

  return fcIter != _fareComponents.end();
}

//----------------------------------------------------------------------------
// ERDRequestProcessor::filterByFareBasisCode
//----------------------------------------------------------------------------
bool
ERDRequestProcessor::filterByFareBasisCode(const FareClassCode& requestedFBC)
{
  if (_trx.getOptions()->errorFromPSS() == TWO_FBC_REQUSTED)
  {
    if (_trx.getOptions()->isWqrd())
      buildErrorMessage(ERROR_NO_FARE_BASIS_WQ, true, true);
    else
      buildErrorMessage(ERROR_NO_FARE_BASIS, true, true);

    return false;
  }

  if (requestedFBC.empty())
  {
    return true;
  }

  _fareComponents.erase(remove_if(_fareComponents.begin(),
                                  _fareComponents.end(),
                                  not1(ERDFareComp::MatchConditionalFareBasis(requestedFBC))),
                        _fareComponents.end());

  if (_fareComponents.size() > 1)
  {
    std::vector<ERDFareComp*> exactlyMatchedFCs(_fareComponents.size());
    std::vector<ERDFareComp*>::iterator newEnd =
        remove_copy_if(_fareComponents.begin(),
                       _fareComponents.end(),
                       exactlyMatchedFCs.begin(),
                       not1(ERDFareComp::MatchFareBasis(requestedFBC)));

    exactlyMatchedFCs.erase(newEnd, exactlyMatchedFCs.end());

    if (!exactlyMatchedFCs.empty())
    {
      _fareComponents = exactlyMatchedFCs;
    }
  }

  // If nothing left, show an error
  if (_fareComponents.empty())
  {
    if (_trx.getOptions()->isWqrd())
      buildErrorMessage(ERROR_NO_FARE_BASIS_WQ, true, true);
    else
      buildErrorMessage(ERROR_NO_FARE_BASIS, true, true);
    return false;
  }
  return true;
}
//----------------------------------------------------------------------------
// ERDRequestProcessor::filterSegmentNumbers
//----------------------------------------------------------------------------
bool
ERDRequestProcessor::filterBySegmentNumbers(const std::vector<uint16_t>& requestedSNs)
{
  _fareComponents.erase(remove_if(_fareComponents.begin(),
                                  _fareComponents.end(),
                                  not1(ERDFareComp::MatchSegmentNumbers(requestedSNs))),
                        _fareComponents.end());

  // Show an error if nothing left
  if (_fareComponents.empty())
  {
    buildErrorMessage(ERROR_NO_SEGMENT);
    return false;
  }
  return true;
}
//----------------------------------------------------------------------------
// ERDRequestProcessor::filterByPaxTypeCode
//----------------------------------------------------------------------------
bool
ERDRequestProcessor::filterByPaxTypeCode(const PaxTypeCode& requestedPTC)
{
  _fareComponents.erase(remove_if(_fareComponents.begin(),
                                  _fareComponents.end(),
                                  not1(ERDFareComp::MatchPaxTypeCode(requestedPTC))),
                        _fareComponents.end());

  // Show an error if nothing left
  if (_fareComponents.empty())
  {
    buildPtcErrorMessage();
    return false;
  }
  return true;
}
//----------------------------------------------------------------------------
// ERDRequestProcessor::filterByPaxTypeNumber
//----------------------------------------------------------------------------
bool
ERDRequestProcessor::filterByPaxTypeNumber(uint16_t requestedPTN)
{
  _fareComponents.erase(remove_if(_fareComponents.begin(),
                                  _fareComponents.end(),
                                  not1(ERDFareComp::MatchPaxTypeNumber(requestedPTN))),
                        _fareComponents.end());

  // Show an error if nothing left
  if (_fareComponents.empty())
  {
    buildPtcErrorMessage();
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
// ERDRequestProcessor::filterByChildTypes
// If all priced passengers are childs return only first
//----------------------------------------------------------------------------
void
ERDRequestProcessor::filterByChildTypes()
{
  if (_fareComponents.size() < 2)
    return;

  std::vector<ERDFareComp*>::const_iterator fcIter = _fareComponents.begin();

  if (!PaxTypeUtil::isPaxWithAge((*fcIter)->paxTypeCode()))
    return;

  char firstLetter = (*fcIter)->paxTypeCode()[0];

  for (++fcIter; fcIter != _fareComponents.end(); ++fcIter)
  {
    if (firstLetter != (*fcIter)->paxTypeCode()[0] ||
        !PaxTypeUtil::isPaxWithAge((*fcIter)->paxTypeCode()))
    {
      return;
    }
  }

  // All types are childs so we choose first
  filterByPaxTypeNumber(_fareComponents.front()->paxTypeNumber());
}

//----------------------------------------------------------------------------
// ERDRequestProcessor::validateFareComponents
//----------------------------------------------------------------------------
bool
ERDRequestProcessor::validateFareComponents(bool validateForSegments)
{
  ERDFareComp* firstFC = _fareComponents.front();

  std::vector<ERDFareComp*>::const_iterator fcIter = _fareComponents.begin();
  std::vector<ERDFareComp*>::const_iterator fcIterEnd = _fareComponents.end();

  for (; fcIter != fcIterEnd; ++fcIter)
  {
    ERDFareComp* currentFC = *fcIter;

    if (!currentFC)
      continue;

    if (currentFC->paxTypeCode() != firstFC->paxTypeCode())
    {
      buildPtcErrorMessage();
      return false;
    }

    if (validateForSegments)
    {
      if (currentFC->ruleNumber() == firstFC->ruleNumber() &&
          currentFC->pricingUnitNumber() == firstFC->pricingUnitNumber() &&
          currentFC->fareBasis() != firstFC->fareBasis() &&
          (((currentFC->departureCity() != firstFC->departureCity() &&
             currentFC->departureCity() != firstFC->arrivalCity())) ||
           ((currentFC->arrivalCity() != firstFC->arrivalCity() &&
             currentFC->arrivalCity() != firstFC->departureCity()))))
      {
        if (_trx.getOptions()->isWqrd())
          buildErrorMessage(ERROR_DIFFERENT_RULE_WQ, true, true);
        else
          buildErrorMessage(ERROR_DIFFERENT_RULE, true, true);

        return false;
      }
    }
    else
    {
      if (currentFC->ruleNumber() != firstFC->ruleNumber() ||
          currentFC->trueGoverningCarrier() != firstFC->trueGoverningCarrier() ||
          currentFC->fareBasis() != firstFC->fareBasis() ||
          currentFC->pricingUnitNumber() != firstFC->pricingUnitNumber() ||
          currentFC->fareAmount() != firstFC->fareAmount() ||
          currentFC->fareCurrency() != firstFC->fareCurrency() ||
          (currentFC->departureCity() != firstFC->departureCity() &&
           currentFC->departureCity() != firstFC->arrivalCity()) ||
          (currentFC->arrivalCity() != firstFC->arrivalCity() &&
           currentFC->arrivalCity() != firstFC->departureCity()) ||
          currentFC->globalIndicator() != firstFC->globalIndicator())
      {
        if (_trx.getOptions()->isWqrd())
          buildErrorMessage(ERROR_DIFFERENT_RULE_WQ, true, true);
        else
          buildErrorMessage(ERROR_DIFFERENT_RULE, true, true);

        return false;
      }
    }
  }
  return true;
}
//----------------------------------------------------------------------------
// ERDRequestProcessor::buildErrorMessage
//----------------------------------------------------------------------------
void
ERDRequestProcessor::buildErrorMessage(const std::string& message, bool warning, bool trailingMsg)
    const
{
  // Check if we should prefix message with ATTN*
  if (warning)
  {
    const FareCalcConfig* fcConfig = FareCalcUtil::getFareCalcConfig(_trx);

    if (fcConfig && fcConfig->warningMessages() == YES)
    {
      _trx.response() << "ATTN*";
    }
  }

  _trx.response() << message << std::endl;

  if (trailingMsg)
  {
    _trx.response() << " " << std::endl;
    buildTrailingMessage();
  }
}
//----------------------------------------------------------------------------
// ERDRequestProcessor::buildPtcErrorMessage
//----------------------------------------------------------------------------
void
ERDRequestProcessor::buildPtcErrorMessage() const
{
  if (_trx.getRequest()->ticketingAgent()->abacusUser() ||
      _trx.getRequest()->ticketingAgent()->infiniUser())
  {
    buildErrorMessage(ERROR_NO_PTC_ABACUS);
  }
  else
  {
    buildErrorMessage(ERROR_NO_PTC_SABRE);
  }
}
//----------------------------------------------------------------------------
// ERDRequestProcessor::buildTrailingMessage
//----------------------------------------------------------------------------
void
ERDRequestProcessor::buildTrailingMessage() const
{
  _trx.response() << std::setfill(' ') << std::left;
  _trx.response() << std::setw(5) << "SEG" << std::setw(9) << "CTYPAIR" << std::setw(9) << "FQ"
                  << std::setw(7) << "RULE" << std::setw(5) << "CXR" << std::setw(5) << "PU"
                  << std::setw(10) << "FARE" << std::setw(10) << "FAREBASIS" << std::endl;

  std::vector<ERDFareComp*>::const_iterator fcIter = _originalFareComponents.begin();
  std::vector<ERDFareComp*>::const_iterator fcIterEnd = _originalFareComponents.end();
  std::vector<ERDFltSeg*>::const_iterator fsFirst;

  if (fcIter != fcIterEnd)
    fsFirst = (*fcIter)->segments().begin();

  for (; fcIter != fcIterEnd; ++fcIter)
  {
    ERDFareComp* currentFC = *fcIter;

    if (!currentFC)
      continue;

    std::vector<ERDFltSeg*>::const_iterator fsIter = currentFC->segments().begin();
    std::vector<ERDFltSeg*>::const_iterator fsIterEnd = currentFC->segments().end();

    for (; fsIter != fsIterEnd; ++fsIter)
    {
      ERDFltSeg* currentSeg = *fsIter;

      if (!currentSeg || currentSeg->surface())
        continue;

      if (*fsFirst != *fsIter &&
          (*fsFirst)->itinSegNumber() ==
              currentSeg->itinSegNumber()) // We displayed this segment before
      {
        return;
      }

      _trx.response() << std::right << std::setfill('0') << std::setw(2)
                      << currentSeg->itinSegNumber() << COLUMN_SPACING;
      _trx.response() << std::left << std::setfill(' ') << std::setw(3)
                      << currentSeg->departureAirport() << std::setw(3)
                      << currentSeg->arrivalAirport() << COLUMN_SPACING;

      if (currentFC->directionality() == "FR")
        _trx.response() << std::left << std::setw(3) << currentFC->departureCity() << std::setw(3)
                        << currentFC->arrivalCity() << COLUMN_SPACING;
      else
        _trx.response() << std::left << std::setw(3) << currentFC->arrivalCity() << std::setw(3)
                        << currentFC->departureCity() << COLUMN_SPACING;

      _trx.response() << std::left << std::setw(4) << currentFC->ruleNumber() << COLUMN_SPACING;
      _trx.response() << std::left << std::setw(2) << currentFC->trueGoverningCarrier()
                      << COLUMN_SPACING;
      _trx.response() << std::right << std::setfill('0') << std::setw(2)
                      << currentFC->pricingUnitNumber() << COLUMN_SPACING;
      _trx.response() << std::left << std::setfill(' ') << std::setw(8) << std::fixed
                      << std::setprecision(2) << currentFC->nucFareAmount() << COLUMN_SPACING2;

      if (currentFC->uniqueFareBasis().length())
        _trx.response() << std::left << currentFC->uniqueFareBasis().substr(0, 13) << std::endl;
      else
        _trx.response() << std::left << currentFC->fareBasis().substr(0, 13) << std::endl;
    }
  }
}

bool
ERDRequestProcessor::prepareDTS(std::string& dtsContent)
{
  int16_t requestedLineNumber = _trx.getOptions()->requestedLineNumber();
  int16_t maxLineNumber = _trx.getOptions()->lineNumber();

  if (maxLineNumber < requestedLineNumber)
  {
    buildErrorMessage(ERROR_INVALID_LINE, false);
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, "");
  }

  // Return invalid entry message if no DTS tag available but line number has been requested
  if (dtsContent.empty())
  {
    // Check if line has been requested
    if (requestedLineNumber != -1)
    {
      // If so, return error message
      buildErrorMessage(ERROR_INVALID_LINE, false);
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, "");
    }
    return false;
  }

  // If no line number has been requested by user, we have to use first line
  if (requestedLineNumber == -1)
    requestedLineNumber = 1;

  // Search for the selected SUM section
  const size_t sumTagStartLen = SUM_TAG_START.length();
  size_t selectedSumStartPos = std::string::npos;
  size_t findBegin = 0;

  for (uint16_t i = 0; i < requestedLineNumber; ++i)
  {
    selectedSumStartPos = dtsContent.find(SUM_TAG_START, findBegin);

    if (selectedSumStartPos == std::string::npos)
      break;

    findBegin = selectedSumStartPos + sumTagStartLen;
  }

  if (selectedSumStartPos == std::string::npos)
  {
    buildErrorMessage(ERROR_INVALID_LINE, false);
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, "");
  }

  // Find end of the selected SUM section
  size_t selectedSumEndPos = dtsContent.find(SUM_TAG_END, selectedSumStartPos);

  if (selectedSumEndPos == std::string::npos)
    return true;

  // Get selected SUM section
  std::string selectedSumSection =
      dtsContent.substr(selectedSumStartPos, selectedSumEndPos - selectedSumStartPos);

  // Replace all SUM's sections with one selected
  size_t firstSumStartPos = dtsContent.find(SUM_TAG_START);
  dtsContent.replace(
      firstSumStartPos, dtsContent.rfind(SUM_TAG_END) - firstSumStartPos, selectedSumSection);

  return true;
}

//----------------------------------------------------------------------------
// ERDRequestProcessor::ifFareBasisExists
//----------------------------------------------------------------------------
bool
ERDRequestProcessor::ifFareBasisExists(const FareClassCode& requestedFBC) const
{
  return _fareComponents.end() != find_if(_fareComponents.begin(),
                                          _fareComponents.end(),
                                          ERDFareComp::MatchConditionalFareBasis(requestedFBC));
}

//----------------------------------------------------------------------------
// ERDRequestProcessor::ifFareBasisWithoutTktDesExists
//----------------------------------------------------------------------------
bool
ERDRequestProcessor::ifFareBasisWithoutTktDesExists(const FareClassCode& requestedFBC) const
{
  return _fareComponents.end() != find_if(_fareComponents.begin(),
                                          _fareComponents.end(),
                                          ERDFareComp::MatchFareBasis(requestedFBC));
}

//----------------------------------------------------------------------------
// ERDRequestProcessor::validateRDErrors
//----------------------------------------------------------------------------
bool
ERDRequestProcessor::validateRDErrors() const
{
  switch (_trx.getOptions()->errorFromPSS())
  {
  case ALPHA_AND_CAT_REQUESTED:

    if (_trx.getOptions()->isWqrd())
      buildErrorMessage(ERROR_INVALID_ENTRY_WQ, false);
    else
      buildErrorMessage(ERROR_INVALID_ENTRY, false);

    return false;
  case M_H_RTG_COMBINED:
    buildErrorMessage(ERROR_MULTIPLE_CODES, false);
    return false;
  case INVALID_DISPLAY_OPTION:
    buildErrorMessage(ERROR_INVALID_OPTION, false);
    return false;
  case ALPHA_AND_FB_REQUESTED:
    buildErrorMessage(ERROR_ALPHA_FB, false);
    return false;
  case MAX_ALPHA:
    buildErrorMessage(ERROR_MAX_ALPHA, false);
    return false;
  default:
    break;
  }

  if (_trx.getOptions()->ruleMenuDisplay() == TRUE_INDICATOR ||
      _trx.getOptions()->routingDisplay() == TRUE_INDICATOR ||
      _trx.getOptions()->headerDisplay() == TRUE_INDICATOR)
  {
    if (!_trx.getOptions()->ruleCategories().empty() || !_trx.getOptions()->alphaCodes().empty() ||
        _trx.getOptions()->IntlConstructionDisplay() == TRUE_INDICATOR ||
        _trx.getOptions()->combScoreboardDisplay() == TRUE_INDICATOR ||
        (_trx.getOptions()->ruleMenuDisplay() == TRUE_INDICATOR &&
         _trx.getOptions()->routingDisplay() == TRUE_INDICATOR) ||
        (_trx.getOptions()->ruleMenuDisplay() == TRUE_INDICATOR &&
         _trx.getOptions()->headerDisplay() == TRUE_INDICATOR) ||
        (_trx.getOptions()->routingDisplay() == TRUE_INDICATOR &&
         _trx.getOptions()->headerDisplay() == TRUE_INDICATOR))
    {
      buildErrorMessage(ERROR_MULTIPLE_CODES, false);
      return false;
    }
  }

  if (_trx.isERDFromSWS())
    return true;

  // Validate cat numbers less or equal to 50
  std::vector<CatNumber>::const_iterator iCat = _trx.getOptions()->ruleCategories().begin();
  std::vector<CatNumber>::const_iterator iCatEnd = _trx.getOptions()->ruleCategories().end();
  for (; iCat != iCatEnd; iCat++)
  {
    if (*iCat > 50)
    {
      buildErrorMessage(ERROR_INVALID_OPTION, false);
      return false;
    }
  }

  return true;
}

//----------------------------------------------------------------------------
// ERDRequestProcessor::validatePassenger
//----------------------------------------------------------------------------
bool
ERDRequestProcessor::validatePassenger()
{
  PaxTypeCode requestedPaxTypeCode = _trx.getOptions()->requestedPaxTypeCode();
  int16_t requestedPaxTypeNumber = _trx.getOptions()->requestedPaxTypeNumber();

  if (_trx.getOptions()->errorFromPSS() == PASSENGER_NOT_FIRST)
  {
    if (_trx.getOptions()->isWqrd())
      buildErrorMessage(ERROR_INVALID_ENTRY_WQ, false);
    else
      buildErrorMessage(ERROR_INVALID_ENTRY, false);

    return false;
  }

  if (_trx.getOptions()->errorFromPSS() == MULTIPLE_PASSENGERS)
  {
    if (_trx.getRequest()->ticketingAgent()->abacusUser() ||
        _trx.getRequest()->ticketingAgent()->infiniUser())
      buildErrorMessage(ERROR_ABACUS_MULTI_PAX_TYPE);
    else
      buildErrorMessage(ERROR_SABRE_MULTI_PAX_TYPE);

    return false;
  }

  if (_trx.getRequest()->ticketingAgent()->abacusUser() ||
      _trx.getRequest()->ticketingAgent()->infiniUser())
  {
    if (requestedPaxTypeNumber != -1)
    {
      if (!filterByPaxTypeNumber(requestedPaxTypeNumber))
        return false;
    }
    else
    {
      filterByChildTypes();
    }
  }
  else
  {
    if (!requestedPaxTypeCode.empty())
    {
      if (!filterByPaxTypeCode(requestedPaxTypeCode))
        return false;
    }
  }

  // Check for multiple passenger types left
  if (multiplePaxTypesLeft())
  {
    if (_trx.getRequest()->ticketingAgent()->abacusUser() ||
        _trx.getRequest()->ticketingAgent()->infiniUser())
      buildErrorMessage(ERROR_ABACUS_MULTI_PAX_TYPE);
    else
      buildErrorMessage(ERROR_SABRE_MULTI_PAX_TYPE);

    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
// ERDRequestProcessor::validateSegments
//----------------------------------------------------------------------------
bool
ERDRequestProcessor::validateSegments(const FareClassCode requestedFareBasis)
{
  if (_trx.getOptions()->errorFromPSS() == SEGMENT_NOT_EXIST)
  {
    buildErrorMessage(ERROR_NO_SEGMENT);
    return false;
  }

  bool fareBasisExists = requestedFareBasis.empty() ? false : ifFareBasisExists(requestedFareBasis);
  bool fareBasisWithoutTktDesExists =
      !fareBasisExists ? false : ifFareBasisWithoutTktDesExists(requestedFareBasis);
  std::vector<uint16_t> requestedSegmentNumbers = _trx.getOptions()->requestedSegments();

  if (!requestedSegmentNumbers.empty())
  {
    // Check if all requested segment numbers are available
    if (!checkSegments(requestedSegmentNumbers))
    {
      return false;
    }

    // Filter Fc's by Seg nbr's
    if (!filterBySegmentNumbers(requestedSegmentNumbers))
      return false;

    if (!requestedFareBasis.empty() && fareBasisExists)
    {
      // correct FBC was filtered out by segments
      if ((fareBasisWithoutTktDesExists && !ifFareBasisWithoutTktDesExists(requestedFareBasis)) ||
          !ifFareBasisExists(requestedFareBasis))
      {
        if (_trx.getOptions()->isWqrd())
          buildErrorMessage(ERROR_DIFFERENT_RULE_WQ, true, true);
        else
          buildErrorMessage(ERROR_DIFFERENT_RULE, true, true);

        return false;
      }

      // Check if selected fare component is unique except FBC
      if (!validateFareComponents(false))
        return false;
    }
  }

  return true;
}

//----------------------------------------------------------------------------
// ERDRequestProcessor::storeResults
//----------------------------------------------------------------------------
bool
ERDRequestProcessor::storeResults() const
{
  // Get first fare component
  ERDFareComp* selectedFC = _fareComponents.front();

  if (selectedFC->segments().empty()) // it should never happen
  {
    buildErrorMessage(ERROR_NO_SEGMENT);
    return false;
  }

  // Get first segment of selected fare component
  ERDFltSeg* selectedSegment = selectedFC->segments().front();

  if (!selectedSegment) // it should never happen
  {
    buildErrorMessage(ERROR_NO_SEGMENT);
    return false;
  }

  // Fill FD trx with data from selected Fc and Seg
  selectedSegment->select(_trx);

  if (!_trx.itin().empty() && _useInternationalRounding)
  {
    _trx.itin().front()->useInternationalRounding() = true;
  }

  return true;
}

//----------------------------------------------------------------------------
// ERDRequestProcessor::completeFareBreaks
//----------------------------------------------------------------------------
void
ERDRequestProcessor::completeFareBreaks()
{
  std::vector<ERDFareComp*>::const_iterator fcIter = _fareComponents.begin();
  std::vector<ERDFareComp*>::const_iterator fcIterEnd = _fareComponents.end();

  for (; fcIter != fcIterEnd; ++fcIter)
  {
    if ((*fcIter)->fareBreak() == FALSE_INDICATOR)
    {
      std::vector<ERDFareComp*>::const_iterator fcIter2 = _fareComponents.begin();

      for (; fcIter2 != fcIterEnd; ++fcIter2)
      {
        if ((*fcIter)->completeFareBreak(**fcIter2))
          break;
      }
    }
  }
}

//----------------------------------------------------------------------------
// ERDRequestProcessor::calculateInternationalRoundingValue
//----------------------------------------------------------------------------
void
ERDRequestProcessor::calculateInternationalRoundingValue()
{
  std::vector<ERDFareComp*>::const_iterator fcIter = _fareComponents.begin();

  for (; fcIter != _fareComponents.end(); ++fcIter)
  {
    const std::vector<ERDFltSeg*>& segments = (*fcIter)->segments();
    std::vector<ERDFltSeg*>::const_iterator fsIter = segments.begin();

    for (; fsIter != segments.end(); ++fsIter)
    {
      if (GeoTravelType::International == (*fsIter)->geoTravelType() &&
          !(*fsIter)->isOriginAndDestinationInRussia(_trx))
      {
        _useInternationalRounding = true;
        return;
      }
    }
  }
}
} // tse
