#include "FareCalc/NoPNRFareCalculation.h"

#include "Common/AccTvlDetailOut.h"
#include "Common/CurrencyUtil.h"
#include "Common/DiagMonitor.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/ParseUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/Billing.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PrivateIndicator.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/NoPNROptions.h"
#include "DBAccess/PaxTypeInfo.h"
#include "FareCalc/FareCalcConsts.h"
#include "FareCalc/FareCalcHelper.h"
#include "FareCalc/FareUsageIter.h"
#include "FareCalc/FcDispFarePath.h"
#include "FareCalc/FcDispFareUsage.h"
#include "FareCalc/FcUtil.h"
#include "FareCalc/NoPNRFareCalcCollector.h"
#include "FareCalc/NoPNRFcConfigWrapper.h"
#include "Rules/AccompaniedTravel.h"

#include <iterator>


namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrMultiSp);


static Logger
logger("atseintl.FareCalc.NoPNRFareCalculation");

namespace
{
char OPTION_1 = '1';
char OPTION_2 = '2';
char OPTION_3 = '3';
}

void
NoPNRFareCalculation::initialize(PricingTrx* trx,
                                 const FareCalcConfig* fcConfig,
                                 FareCalcCollector* fcCollector)
{
  FareCalculation::initialize(trx, fcConfig, fcCollector);
  NoPNRPricingTrx* noPnrTrx = static_cast<NoPNRPricingTrx*>(trx);
  _noPNRConfigOptions = noPnrTrx->noPNROptions();
}

void
NoPNRFareCalculation::process()
{
  _primaryCalcTotals.clear();

  const FareCalcCollector::CalcTotalsMap& cm = _fcCollector->calcTotalsMap();

  DiagnosticTypes diagType = _trx->diagnostic().diagnosticType();
  if (diagType == Diagnostic860)
  {
    displayRuler();
  }

  _primaryResp = true;

  bool dispDtlFormat = checkDetailFormat();

  if (_trx->getRequest()->ticketingAgent()->axessUser()) // JAL/AXESS agent
    _fareCalcDisp << (dispDtlFormat ? "VT " : "VD ") << std::endl;

  if (dispDtlFormat)
  {
    processDetailedFormat(cm);
  }
  else
  {
    //_fcConfig will be updated at fcConfWrapper destruction
    NoPNRFcConfigWrapper fcConfWrapper(_fcConfig, (&_fcConfig), _noPNRConfigOptions);
    processPrimaryFormat();
  }

  displaySegmentFeeMessage();

  if (_trx->getRequest()->diagnosticNumber() == Diagnostic860)
  {
    displayRuler();
  }

  if ((diagType == DiagnosticNone || diagType == Diagnostic854 || diagType == Diagnostic860 ||
       diagType == 1860) &&
      _trx->billing()->requestPath() != SWS_PO_ATSE_PATH)
  {
    DiagMonitor diag(*_trx, diagType);
    diag << _fareCalcDisp.str() << '\n';
    LOG4CXX_DEBUG(logger, _fareCalcDisp.str());
  }
  // Unless it is a straight WP entry, we should always generate the
  // detail responses since the user expects to be able to do WPn
  // entries after WPA or WP-nomatch
  if (_trx->altTrxType() != PricingTrx::WP)
    createWpnResponse();
}

void
NoPNRFareCalculation::processPrimaryFormat()
{
  int psgCount = 0;

  // this code may have to be removed (when WQ is finished, we might
  // expect that the processing flow won't enter FareCalc in case
  // when no fares are found
  if (noFaresForNoPaxType())
  {
    displayNoMatchResponse();
    return;
  }

  std::set<PaxType*, PaxType::InputOrder> inOrderPaxType(_trx->paxType().begin(),
                                                         _trx->paxType().end());

  bool firstPaxType = true;
  for (const auto& elem : inOrderPaxType)
  {
    processPrimaryFormatSinglePaxType(elem, psgCount, firstPaxType);
    firstPaxType = false;
  }

  // display invalid Corp ID trailer message
  displayCorpIdTrailerMessage();

  // display final trailer message
  displayFinalTrailerMessage();
}

void
NoPNRFareCalculation::displayFinalTrailerMessage()
{
  if ((_primaryResp && _noPNRConfigOptions->displayFinalWarningMsg() == 'Y') ||
      (_secondaryResp && _noPNRConfigOptions->displayFinalWarningMsg2() == 'Y'))
  {
    _fareCalcDisp << std::endl << "** TOTALS INCLUDE KNOWN TAXES AND FEES **" << std::endl
                  << "** TOTAL FARE, TAXES AND FEES MAY CHANGE ONCE FLIGHTS ARE " << std::endl
                  << "   CONFIRMED **" << std::endl;
  }
}

// -------------------------------------------------------
// display Global Indicator Error/Warning message
// -------------------------------------------------------
void
NoPNRFareCalculation::displayGIMessage()
{
  NoPNRPricingTrx* noPNRTrx = static_cast<NoPNRPricingTrx*>(_trx);
  std::map<const TravelSeg*, const std::string>& globalWarnMap = noPNRTrx->GIWarningMap();

  std::vector<const std::string*> sortedMessages(noPNRTrx->itin().front()->travelSeg().size(), nullptr);

  // sort the messages by travel segment order
  for (std::map<const TravelSeg*, const std::string>::const_iterator iter = globalWarnMap.begin(),
                                                                     end = globalWarnMap.end();
       iter != end;
       ++iter)
    sortedMessages.at(iter->first->segmentOrder() - 1) = &(iter->second);

  // display the messages in correct order
  for (const auto sortedMessage : sortedMessages)
    if (sortedMessage != nullptr)
    {
      if (_fcConfig->warningMessages() == FareCalcConsts::FC_YES)
        _fareCalcDisp << "ATTN*";
      _fareCalcDisp << *sortedMessage << std::endl;
    }
}

void
NoPNRFareCalculation::processPrimaryFormatSinglePaxType(PaxType* pt,
                                                        int& psgCount,
                                                        bool firstPaxType)
{
  uint16_t startIndex = psgCount + 1;

  // Process the calc totals based on the WPA Sort setting:
  std::vector<CalcTotals*>& calcTotalsList = getCalcTotals(pt);

  // Fare line format - break between different PTC.
  if (!firstPaxType && _noPNRConfigOptions->fareLinePTCBreak() == 'Y')
    _fareCalcDisp << "  " << std::endl;

  // check for no fares for pax
  if (noFaresForPsg(calcTotalsList))
  {
    const CalcTotals* dummyCalcTotals = nullptr;
    // single dummy (not processed) calcTotal found - no fares for PTC
    if (calcTotalsList.size() == 1)
    {
      dummyCalcTotals = calcTotalsList.front();
    }
    else
    {
      // list is empty, no 'dummy' calcTotals - noMatch fares found, but
      // failed fareTypeGroup processing
      // get any calcTotals with correct paxType - to display psgInfo line and
      // 'no fares' message
      FareCalcCollector::CalcTotalsMap::const_iterator it = _fcCollector->calcTotalsMap().begin();
      while (it != _fcCollector->calcTotalsMap().end() &&
             (!it->second->farePath || (it->second->farePath->paxType() != pt)))
        ++it;

      if (it == _fcCollector->calcTotalsMap().end())
      {
        // processing error - no calcTotals found for PTC
        LOG4CXX_ERROR(logger, "error - no CalcTotals found for pax type: " << pt);
        // display no match response anyway - without the pax type line
        displayNoMatchResponse();
        return;
      }
      dummyCalcTotals = it->second;
    }
    // no fares for pax type
    displayPsgrInfo(*dummyCalcTotals, false);
    displayNoMatchResponse();
    return;
  }

  // clear out the RO option list for this PAX type
  _roOptions.clear();
  _allRequireRebook = true;
  // clear options to rebook for this PAX type
  _optionsToRebook.clear();

  std::string preVerifyBookingClassResponse = "";

  bool firstOption = true;
  for (const auto elem : calcTotalsList)
  {

    CalcTotals* calcTotalsTemp = elem;

    if (calcTotalsTemp->farePath->processed() == false)
      continue;

    calcTotalsTemp = selectCalcTotals(*_trx, elem);

    CalcTotals& calcTotals = *calcTotalsTemp;

    calcTotals.wpaInfo.psgDetailRefNo = ++psgCount;
    _primaryCalcTotals.push_back(&calcTotals);

    if (!isNomatch(calcTotals))
    {
      _allRequireRebook = false;
    }
    else
    {
      _optionsToRebook.push_back(psgCount);
    }

    if (_trx->getRequest()->ticketingAgent()->axessUser() &&
        _trx->getRequest()->isWpNettRequested() && calcTotals.netRemitCalcTotals != nullptr)
    {
      calcTotals.netRemitCalcTotals->wpaInfo.psgDetailRefNo = calcTotals.wpaInfo.psgDetailRefNo;
    }

    if (firstOption)
    {
      firstOption = false;
      displayPsgrInfo(calcTotals, false);

      // here we save current response and clear it (to later
      // display 'verify booking class header' if there is a need)
      preVerifyBookingClassResponse = _fareCalcDisp.str();
      _fareCalcDisp.clear();
      displayNoPNRFareLineHdr();
    }

    displayPsgFareLineFormat(calcTotals);

    checkForROmessages(calcTotals);
  }

  // now, display 'verify booking class header' (if there is a need)
  // first, save the current output & clear stream
  std::string postVerifyBookingClassResponse = _fareCalcDisp.str();
  _fareCalcDisp.clear();

  // display the pre-header output
  _fareCalcDisp << preVerifyBookingClassResponse;

  // at this point we know, if this header should be appended
  if (_optionsToRebook.size() > 0)
    displayVerifyBookingClassMsg();

  // append the rest of response
  _fareCalcDisp << postVerifyBookingClassResponse;

  displayRoMessage(_roOptions);

  displayFareTypeMessage(calcTotalsList);

  displayGIMessage();

  displayWqTrailerMessage(); // displays 'see other fares ...

  // warning message for all options of same PaxType:
  displayPaxTypeWarnings(calcTotalsList, startIndex);

  if (_optionsToRebook.size() > 0)
    displayRebookMessage(_optionsToRebook);
}

void
NoPNRFareCalculation::checkForROmessages(CalcTotals& calcTotals)
{
  if (_noPNRConfigOptions->applyROInFareDisplay() != 'Y')
    return;
}

void
NoPNRFareCalculation::processDetailedFormat(const FareCalcCollector::CalcTotalsMap& cm)
{
  int psgCount = 0;

  _primaryResp = false;
  _secondaryResp = true;

  std::set<PaxType*, PaxType::InputOrder> inOrderPaxType(_trx->paxType().begin(),
                                                         _trx->paxType().end());

  std::ostringstream accumulatedResponse;

  for (std::set<PaxType*, PaxType::InputOrder>::const_iterator pti = inOrderPaxType.begin(),
                                                               ptend = inOrderPaxType.end();
       pti != ptend;
       ++pti)
  {
    if (_noPNRConfigOptions->passengerDetailPTCBreak() == 'Y' && pti != inOrderPaxType.begin())
    {
      accumulatedResponse << "  " << std::endl;
    }

    CalcTotals* calcTotalsTemp = nullptr;

    for (const auto& elem : cm)
    {
      bool cat35CalcTotalsFound = false;
      if (_trx->getRequest()->ticketingAgent()->axessUser() &&
          _trx->getRequest()->isWpNettRequested() && elem.second->netRemitCalcTotals != nullptr)
        cat35CalcTotalsFound = true;

      calcTotalsTemp = selectCalcTotals(*_trx, elem.second, cat35CalcTotalsFound);
      CalcTotals& calcTotals = *calcTotalsTemp;

      if (!checkExistFarePath(*_trx, calcTotals) || calcTotals.farePath->paxType() != (*pti))
        continue;

      calcTotals.wpaInfo.psgDetailRefNo = ++psgCount;

      resetCounters();
      getTotalFare(calcTotals);

      displayPsgDetailFormat(calcTotals);

      calcTotalsTemp->wpaInfo.wpnDetailResponse =
          getPSSBookingCodesLine(calcTotals) + _fareCalcDisp.str();

      accumulatedResponse << _fareCalcDisp.str();
      _fareCalcDisp.clear();
    }
  }
  _fareCalcDisp << accumulatedResponse.str();

  // display invalid Corp ID trailer message
  if (!_trx->getRequest()->incorrectCorpIdVec().empty())
    _fareCalcDisp << std::endl;
  displayCorpIdTrailerMessage();

  displayFinalTrailerMessage();
}

void
NoPNRFareCalculation::displayNoPNRFareLineHdrMultiVersion(bool activeNewVersion)
{
  if (activeNewVersion)
  {
    if (_noPNRConfigOptions->fareLineHeaderFormat() == 'N')
    {
      _fareCalcDisp << "   FARE BASIS BOOK CODE           FARE" << std::endl;
    }
    else
    {
      _fareCalcDisp << "   FARE BASIS BOOK CODE           FARE TAX/FEES/CHGS  TOTAL" << std::endl;
    }
  }
  else
  {
    if (_noPNRConfigOptions->fareLineHeaderFormat() == 'N')
    {
      _fareCalcDisp << "   FARE BASIS BOOK CODE           FARE" << std::endl;
    }
    else
    {
      _fareCalcDisp << "   FARE BASIS BOOK CODE           FARE      TAX       TOTAL" << std::endl;
    }
  }
}

void
NoPNRFareCalculation::displayNoPNRFareLineHdr()
{
  const Agent* agent = _trx->getRequest()->ticketingAgent();
  if (agent->tvlAgencyPCC().empty() || agent->axessUser())
  { // AS
    displayNoPNRFareLineHdrMultiVersion(TrxUtil::isTaxesNewHeaderASActive(*_trx));
  }
  else
  { // TN
    displayNoPNRFareLineHdrMultiVersion(true);
  }
}

void
NoPNRFareCalculation::displayPsgrInfo(const CalcTotals& calcTotals, bool detailFormat)
{
  if (detailFormat)
  {
    const std::string& displayedPaxType = (_fcConfig->truePsgrTypeInd() == 'Y')
                                              ? calcTotals.truePaxType
                                              : calcTotals.requestedPaxType;

    _fareCalcDisp << "PSGR TYPE  " << displayedPaxType;

    if (_noPNRConfigOptions->secondaryPTCRefNo() == 'Y')
      _fareCalcDisp << " - " << std::setw(2) << std::setfill('0') << std::right
                    << (_primaryResp
                            ? FareCalcUtil::getPtcRefNo(*_trx, calcTotals.farePath->paxType())
                            : calcTotals.wpaInfo.psgDetailRefNo);
  }
  else
  {
    if (_noPNRConfigOptions->fareLinePTC() == OPTION_2)
    {
      _fareCalcDisp << "INPUT PSGR TYPE  ";
    }
    else if (_noPNRConfigOptions->fareLinePTC() == OPTION_3)
    {
      int paxTypeNbr = FareCalcUtil::getPtcRefNo(*_trx, calcTotals.farePath->paxType());
      _fareCalcDisp << paxTypeNbr;

      switch (paxTypeNbr % 10)
      {
      case 1:
        _fareCalcDisp << "ST";
        break;
      case 2:
        _fareCalcDisp << "ND";
        break;
      case 3:
        _fareCalcDisp << "RD";
        break;
      default:
        _fareCalcDisp << "TH";
        break;
      }
      _fareCalcDisp << " PSGR TYPE  ";
    }
    else
    {
      // fareLinePTC() == 1
      _fareCalcDisp << "PSGR TYPE  ";
    }

    _fareCalcDisp << calcTotals.requestedPaxType;

    if (_noPNRConfigOptions->primePTCRefNo() == 'Y')
    {
      // also display passenger detail reference number
      _fareCalcDisp << " - " << std::setw(2) << std::setfill('0') << std::right
                    << FareCalcUtil::getPtcRefNo(*_trx, calcTotals.farePath->paxType());
    }
  }
  _fareCalcDisp << std::endl;
}

void
NoPNRFareCalculation::displayPsgDetailFormat(CalcTotals& calcTotals)
{
  if (calcTotals.farePath && calcTotals.farePath->intlSurfaceTvlLimit())
  {
    displayPsgrInfo(calcTotals, true);
    _fareCalcDisp << "ISSUE SEPARATE TICKETS-INTL SURFACE RESTRICTED\n";
    return;
  }

  if (!checkExistFarePath(*_trx, calcTotals))
  {
    displayPsgrInfo(calcTotals, true);
    displayNoMatchResponse();
    displayWqTrailerMessage();
    return;
  }

  if (!_trx->getRequest()->ticketingAgent()->abacusUser() &&
      !_trx->getRequest()->ticketingAgent()->axessUser() &&
      !_trx->getRequest()->ticketingAgent()->infiniUser())
  {
    displayItineraryLine(*_trx, *_fcCollector, calcTotals, true);
  }
  else
  {
    displayItineraryLine(*_trx, *_fcCollector, calcTotals, false);
  }

  displayFareTax(*_trx, *_fcCollector, calcTotals);

  if (_fcConfig->itinDisplayInd() == FareCalcConsts::FC_YES ||
      _fcConfig->fareTaxTotalInd() == FareCalcConsts::HORIZONTAL_FARECALC1)
  {
    if (!_trx->getRequest()->ticketingAgent()->abacusUser() &&
        !_trx->getRequest()->ticketingAgent()->axessUser() && _secondaryResp)
    {
      if (!_trx->getRequest()->ticketingAgent()->infiniUser())
        processGrandTotalLineWPnn(*_trx, calcTotals);
    }

    displayPsgFareCalc(*_trx, *_fcCollector, calcTotals);

    // Display Net fare result w/o Tax and Total for WPNETT
    if (_trx->getRequest()->ticketingAgent()->axessUser() &&
        _trx->getRequest()->isWpNettRequested() && _cat35CalcTotals != nullptr)
    {
      displayPsgFareCalc(*_trx, *_fcCollector, *_cat35CalcTotals);
    }
  }
  _fareCalcDisp << getApplicableBookingCodesLine(calcTotals);

  if (calcTotals.dispSegmentFeeMsg())
    _dispSegmentFeeMsg = true;

  displayCommandPricingAndVariousMessages(&calcTotals);
}

void
NoPNRFareCalculation::displayPaxTypeWarnings(const std::vector<CalcTotals*>& calcTotalList,
                                             const uint16_t& startIndex)
{
  displayPaxTypeWarningsAccTvl(calcTotalList);
}

void
NoPNRFareCalculation::displayPaxTypeWarningsAccTvl(const std::vector<CalcTotals*>& calcTotalList)
{
  if (_noPNRConfigOptions->accompaniedTvlTrailerMsg() == OPTION_3)
    return; // do not display the message

  std::vector<CalcTotals*>::const_iterator calcTotalIter = calcTotalList.begin();
  const std::vector<CalcTotals*>::const_iterator calcTotalIterEnd = calcTotalList.end();

  bool accTvlMayApply = false;
  for (; calcTotalIter != calcTotalIterEnd; calcTotalIter++)
  {
    if ((*calcTotalIter)->wpaInfo.reqAccTvl)
    {
      accTvlMayApply = true;
      break;
    }
  }

  if (accTvlMayApply)
  {
    if (_noPNRConfigOptions->accompaniedTvlTrailerMsg() == OPTION_1)
      _fareCalcDisp << "ATTN*";
    _fareCalcDisp << "ACCOMPANYING TRAVEL RESTRICTIONS MAY APPLY\n";
  }
}

std::vector<CalcTotals*>&
NoPNRFareCalculation::getCalcTotals(const PaxType* paxType)
{
  LOG4CXX_DEBUG(logger,
                "Getting CalcTotals for pax: '" << (paxType ? paxType->paxType() : "NULL!") << "'");
  std::map<const PaxType*, std::vector<CalcTotals*> >::iterator calcTotalsObtainedIt =
      calcTotalsObtained.find(paxType);
  if (calcTotalsObtainedIt != calcTotalsObtained.end())
    return (calcTotalsObtainedIt->second);

  calcTotalsObtained[paxType] = std::vector<CalcTotals*>();
  std::vector<CalcTotals*>& calcTotalsList = calcTotalsObtained[paxType];

  calcTotalsList.clear();

  fillCalcTotalsList(paxType, calcTotalsList);
  LOG4CXX_DEBUG(logger, "calc totals  size: " << calcTotalsList.size());

  if (int(calcTotalsList.size()) > 1)
  {
    bool asc = (_noPNRConfigOptions->wqSort() != 'H');
    std::sort(calcTotalsList.begin(), calcTotalsList.end(), CalcTotalsCompare(asc));
  }

  // after the sorting, resize the vector if it is longer than maxNoOptions
  if (int(calcTotalsList.size()) > _noPNRConfigOptions->maxNoOptions())
    calcTotalsList.resize(_noPNRConfigOptions->maxNoOptions());
  return calcTotalsObtained[paxType];
}

void
NoPNRFareCalculation::fillCalcTotalsList(const PaxType* paxType,
                                         std::vector<CalcTotals*>& calcTotalsList) const
{
  LOG4CXX_DEBUG(logger, "fillCalcTotalsList");
  for (const auto& elem : _fcCollector->calcTotalsMap())
  {
    if (elem.second->farePath && elem.second->farePath->paxType() == paxType &&
        elem.second->farePath->processed())
    {
      calcTotalsList.push_back(elem.second);
    }
  }
}

void
NoPNRFareCalculation::createWpnResponse()
{
  if (_trx->getRequest()->diagnosticNumber() == DiagnosticNone)
  {
    const FareCalcCollector::CalcTotalsMap& cm = _fcCollector->calcTotalsMap();
    LOG4CXX_DEBUG(logger, "Number of WPn options: " << cm.size());
    _psgrCount = 1;
    _primaryResp = false;
    _secondaryResp = true;

    for (FareCalcCollector::CalcTotalsMap::const_iterator i = cm.begin(), iend = cm.end();
         i != iend;
         ++i)
    {
      bool netRemitCalcTotals_found = false;
      _cat35CalcTotals = nullptr;
      if (_trx->getRequest()->ticketingAgent()->axessUser() &&
          _trx->getRequest()->isWpNettRequested() && i->second->netRemitCalcTotals != nullptr)
      {
        netRemitCalcTotals_found = true;
        _cat35CalcTotals = i->second;
      }

      CalcTotals* tmp_calcTotals = nullptr;
      tmp_calcTotals = selectCalcTotals(*_trx, i->second, netRemitCalcTotals_found);

      if (tmp_calcTotals == nullptr)
        continue;

      if (std::find(_primaryCalcTotals.begin(), _primaryCalcTotals.end(), tmp_calcTotals) ==
          _primaryCalcTotals.end())
        continue;

      CalcTotals& calcTotals = *tmp_calcTotals;

      if (!checkExistFarePath(*_trx, calcTotals))
        continue;

      resetCounters();
      getTotalFare(calcTotals);

      _fareCalcDisp.clear();
      displayPsgDetailFormat(calcTotals);

      tmp_calcTotals->wpaInfo.wpnDetailResponse =
          getPSSBookingCodesLine(calcTotals) + _fareCalcDisp.str();

//      i->second->wpaInfo.wpnDetailResponse =
//          getPSSBookingCodesLine(calcTotals) + _fareCalcDisp.str();

      LOG4CXX_DEBUG(logger,
                    "WPN RESP:" << i->second->wpaInfo.psgDetailRefNo << "\n"
                                << i->second->wpaInfo.wpnDetailResponse);
    }
  }
}

void
NoPNRFareCalculation::displayNoMatchResponse()
{
  // <No Match No Fares Error Message>
  if (_trx->getOptions() && _trx->getOptions()->isCat35Net())
  {
    _fareCalcDisp << "NO NET FARE AMOUNT" << std::endl;
  }
  else
  {
    std::string noMatchNoFare;
    if (_fcConfig->getMsgAppl(FareCalcConfigText::WPA_NO_MATCH_NO_FARE, noMatchNoFare))
    {
      _fareCalcDisp << noMatchNoFare << std::endl;
    }
  }
}

void
NoPNRFareCalculation::displayRoMessage(const std::vector<int>& roOptions)
{
  std::string roRbdTktOverride;
  if (_fcConfig->getMsgAppl(FareCalcConfigText::WPA_RO_INDICATOR, roRbdTktOverride))
  {
    if (roOptions.size() > 0)
    {
      displayIndicesVector("*", roOptions, roRbdTktOverride, false, _fareCalcDisp);
    }
  }
}

// display message and indices vector; takes care of wrapping
void
NoPNRFareCalculation::displayIndicesVector(const std::string& prefix,
                                           const std::vector<int>& indicesToDisplay,
                                           const std::string& postfix,
                                           bool canBreakPostfix,
                                           std::ostream& output,
                                           bool appendFinalNewline,
                                           bool alwaysDisplayTwoDigits)
{
  // copy & sort the indices
  std::vector<int> indices;
  indices.insert(indices.begin(), indicesToDisplay.begin(), indicesToDisplay.end());
  std::sort(indices.begin(), indices.end());

  std::vector<int>::const_iterator range_beginning = indices.end();
  std::vector<int>::const_iterator prev_element = indices.end();
  bool first_token = true;
  std::ostringstream currentLine;

  // show prefix, can be broken if too long
  addToOutput(currentLine, prefix, true, output);

  for (std::vector<int>::const_iterator iter = indices.begin(); iter != indices.end();
       prev_element = iter++)
  {
    if (range_beginning != indices.end())
    {
      if (*prev_element != *iter - 1)
      {
        // range broken, display something
        displayRangeOrCommaSeparated(currentLine,
                                     indices,
                                     range_beginning,
                                     prev_element,
                                     first_token,
                                     output,
                                     alwaysDisplayTwoDigits);
        // invalidate pointer
        range_beginning = indices.end();
      }
    }
    if (range_beginning == indices.end()) // no current range - beginning of another
    {
      range_beginning = iter;
    }
  }
  // display last range/elements
  displayRangeOrCommaSeparated(currentLine,
                               indices,
                               range_beginning,
                               prev_element,
                               first_token,
                               output,
                               alwaysDisplayTwoDigits);

  // show the postfix
  addToOutput(currentLine, postfix, canBreakPostfix, output);

  // flush the remaining text
  if (!currentLine.str().empty())
    output << currentLine.str();

  if (appendFinalNewline)
    output << std::endl;
}

void
NoPNRFareCalculation::addToOutput(std::ostringstream& currentLine,
                                  const std::string& txt,
                                  bool canBreakBetweenLines,
                                  std::ostream& output)
{
  size_t currTxtLen = currentLine.str().size();
  size_t txtLen = txt.size();

  // make sure that text isn't bigger than 64 chars
  if (txtLen > WINDOW_WIDTH)
  {
    // split the text & call recursively
    for (size_t i = 0; i < txtLen / WINDOW_WIDTH; ++i)
    {
      addToOutput(
          currentLine, txt.substr(i * WINDOW_WIDTH, WINDOW_WIDTH), canBreakBetweenLines, output);
    }
  }
  else
  {
    if (txtLen + currTxtLen > WINDOW_WIDTH)
    {
      if (canBreakBetweenLines)
      {
        output << currentLine.str();
        currentLine.str("");
        output << txt.substr(0, WINDOW_WIDTH - currTxtLen) << std::endl;
        currentLine << txt.substr(WINDOW_WIDTH - currTxtLen);
      }
      else
      {
        // can't break the text between lines
        output << currentLine.str() << std::endl;
        currentLine.str("");
        currentLine << txt;
      }
    }
    else
    {
      // current line won't be finished yet
      currentLine << txt;
    }
  }
}

void
NoPNRFareCalculation::displayRangeOrCommaSeparated(std::ostringstream& currentLine,
                                                   const std::vector<int>& indices,
                                                   std::vector<int>::const_iterator start,
                                                   std::vector<int>::const_iterator end,
                                                   bool& first_token,
                                                   std::ostream& output,
                                                   bool alwaysDisplayTwoDigits)
{
  if (start == indices.end() || end == indices.end())
    return;

  if (first_token)
  {
    first_token = false;
  }
  else
  {
    addToOutput(currentLine, ",", false, output);
  }
  // if range size is > 2 , display 'a-b' format ; comma-separated otherwise
  int dist = std::distance(start, end);
  std::ostringstream tmp;
  if (dist > 1)
  {
    // display range
    if (alwaysDisplayTwoDigits)
      tmp << std::right << std::setw(2) << std::setfill('0');
    tmp << *start << '-';
    if (alwaysDisplayTwoDigits)
      tmp << std::right << std::setw(2) << std::setfill('0');
    tmp << *end;

    // the range display can't be broken between lines
    addToOutput(currentLine, tmp.str(), false, output);
  }
  else
  {
    // display comma-separated
    if (alwaysDisplayTwoDigits)
      tmp << std::right << std::setw(2) << std::setfill('0');
    tmp << *start;
    addToOutput(currentLine, tmp.str(), false, output);
    if (dist == 1)
    {
      tmp.str(""); // clear
      tmp << ',';
      if (alwaysDisplayTwoDigits)
        tmp << std::right << std::setw(2) << std::setfill('0');
      tmp << *end;
      addToOutput(currentLine, tmp.str(), false, output);
    }
  }
}

// Sorter function for CalcTotals
bool
NoPNRFareCalculation::CalcTotalsCompare::less(const CalcTotals* c1, const CalcTotals* c2) const
{
  MoneyAmount m1 = c1->equivFareAmount + c1->taxAmount();
  MoneyAmount m2 = c2->equivFareAmount + c2->taxAmount();

  if (m1 == m2)
    return c1 < c2;

  return m1 < m2;
}

bool
NoPNRFareCalculation::isNomatch(const CalcTotals& calcTotals)
{
  // TODO: is this check enough / ok?
  return (!calcTotals.farePath) || calcTotals.farePath->noMatchOption();
}

bool
NoPNRFareCalculation::checkDetailFormat()
{
  LOG4CXX_DEBUG(logger, "checkDetailFormat()");
  if (_secondaryResp)
    return true;

  LOG4CXX_DEBUG(logger, "not _secondaryResp");

  const char passengerDetailLineFormat = _noPNRConfigOptions->passengerDetailLineFormat();

  // this is primary response
  if (passengerDetailLineFormat == OPTION_2)
  {
    // 'never display Passenger Detail Format'
    return false;
  }

  LOG4CXX_DEBUG(logger, "passengerDetailLineFormat != 2");

  // for primary response, if there is exactly 1 'match' fare
  // for each paxtype - may also display detailed format
  for (std::vector<PaxType*>::const_iterator i = _trx->paxType().begin(),
                                             iend = _trx->paxType().end();
       i != iend;
       ++i)
  {
    LOG4CXX_DEBUG(logger, "checking pax type " << (*i)->paxType());
    std::vector<CalcTotals*>& calcTotalsList = getCalcTotals(*i);
    if (calcTotalsList.size() > 1)
    {
      // more that 1 line to display for this pax type
      LOG4CXX_DEBUG(logger, "more that 1 CalcTotals");
      return false;
    }
    else
    {
      // no fares for pax type
      if (noFaresForPsg(calcTotalsList))
        return false;

      if (isNomatch(*calcTotalsList.front()))
        return false;

      LOG4CXX_DEBUG(logger, "1 CalcTotals exist");

      // checking if calcTotals is noMatch - not needed
      // - transaction isn't noMatch(), and there is
      // exactly 1 calcTotals - this must be 'match'

      // check  <Passenger Detail Line Format>
      // '1' - continues anyway, '2' - checked at the beginning
    }
  }
  return true;
}

void
NoPNRFareCalculation::displayRebookMessage(const std::vector<int>& optionsToRebook)
{
  if (!_allRequireRebook)
  {
    if (!optionsToRebook.empty())
    {
      // WQ fare combination output include Integrated Match/No Match fare combinations
      //<RBD Match/No Match Integrated Trailer Message-Primary>
      std::string bookingClassReqMsg;
      if (_noPNRConfigOptions->rbdMatchTrailerMsg() == OPTION_3) // do not display
        return;
      if (_noPNRConfigOptions->rbdMatchTrailerMsg() == OPTION_1)
        bookingClassReqMsg = "ATTN*";

      bookingClassReqMsg += "APPLICABLE BOOKING CLASS REQUIRED FOR OPTIONS - ";

      displayIndicesVector(bookingClassReqMsg, optionsToRebook, "", false, _fareCalcDisp);
    }
  }
  else
  {
    // only no-matches
    std::string noMatchRebook;
    if (_fcConfig->getMsgAppl(FareCalcConfigText::WPA_NO_MATCH_REBOOK, noMatchRebook))
      _fareCalcDisp << noMatchRebook << std::endl;
  }
}

void
NoPNRFareCalculation::displayFareTypeMessage(const std::vector<CalcTotals*>& ctList)
{
  if (_noPNRConfigOptions->displayFareRuleWarningMsg() == 'N')
    return; // don't display the rule warnings for primary screen

  LOG4CXX_DEBUG(logger, "displayFareTypeMessage START");
  if (ctList.empty())
    return;

  LOG4CXX_DEBUG(logger,
                "_fcCollector->multiMessage().size() = " << _fcCollector->multiMessage().size());
  for (const auto& elem : _fcCollector->multiMessage())
  {
    std::vector<int> indexes;

    for (const auto ct : ctList)
    {
      if (elem.second.chkFarePath(ct->farePath))
      {
        indexes.push_back(ct->wpaInfo.psgDetailRefNo);
      }
    }
    LOG4CXX_DEBUG(logger, "displayFareTypeMessage - indexes.size() = " << indexes.size());

    if (!indexes.empty())
    {
      std::string prefix = "*";
      std::string postfix = "*";
      if (elem.first.size() > 3)
        postfix += (elem.first).substr(3);
      else
        postfix += elem.first;

      displayIndicesVector(prefix, indexes, postfix, false, _fareCalcDisp);
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void NoPNRFareCalculation::processGrandTotalLineWPnn()
// Description:   Prepare to display total fare and tax amount by passenger
//                by one FarePath
// @param         trx, calcTotals
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
NoPNRFareCalculation::processGrandTotalLineWPnn(PricingTrx& trx, CalcTotals& calcTotals)
{
  getCurrencyNoDec(trx, trx.getOptions()->currencyOverride());
  _equivNoDec = _nbrDec;

  CurrencyCode curCode;
  CurrencyCode baseCurCode;
  CurrencyNoDec noDec;

  _fareCalcDisp << std::setfill(' ') << std::setw(16);

  MoneyAmount totalBaseFare = getBaseFareTotal(calcTotals, baseCurCode, noDec);

  _fareCalcDisp << std::setprecision(_fareNoDec);
  _fareCalcDisp << totalBaseFare;

  MoneyAmount totalEquivAmount = getEquivFareAmountTotal(calcTotals, curCode, noDec);

  _fareCalcDisp << std::setprecision(_equivNoDec);
  _fareCalcDisp << std::setw(15);

  if (totalEquivAmount == 0 || curCode == baseCurCode)
  {
    _fareCalcDisp << ' ';
    totalEquivAmount = 0;
  }
  else
  {
    _fareCalcDisp << totalEquivAmount;
  }

  MoneyAmount totalTax = getTaxTotal(calcTotals, curCode, noDec);

  _fareCalcDisp << std::setprecision(_equivNoDec);
  _fareCalcDisp << std::setw(11);
  if (totalTax == 0)
    _fareCalcDisp << ' ';
  else
    _fareCalcDisp << totalTax;

  MoneyAmount grandTotal = totalTax;
  if (totalEquivAmount != 0)
    grandTotal += totalEquivAmount;
  else if (totalBaseFare != 0)
    grandTotal += totalBaseFare;
  else
    grandTotal += getEquivFareAmountTotal(calcTotals, curCode, noDec);

  if (grandTotal != 0)
  {
    _fareCalcDisp << std::setw(18) << grandTotal << "TTL";
  }

  _fareCalcDisp << std::endl;

  // Check grand total amount length
  Money amount(grandTotal, curCode);
  int16_t i = amount.toString().size() - curCode.size();
  checkFareAmountLength(i, _fareAmountLen);

  return;
}

//----------------------------------------------------------------------------
// getBaseFareTotal
//----------------------------------------------------------------------------
MoneyAmount
NoPNRFareCalculation::getBaseFareTotal(CalcTotals& totals,
                                       CurrencyCode& currencyCodeUsed,
                                       CurrencyNoDec& noDecUsed)
{
  MoneyAmount theTotal = 0;

  theTotal += (totals.convertedBaseFare) * (totals.farePath->paxType()->number());
  if (!totals.convertedBaseFareCurrencyCode.empty()) // Some PAX may not be priced with XO.
  {
    currencyCodeUsed = totals.convertedBaseFareCurrencyCode;
    noDecUsed = totals.convertedBaseFareNoDec;
  }
  return (theTotal);
}

//----------------------------------------------------------------------------
// getEquivFareAmountTotal
//----------------------------------------------------------------------------
MoneyAmount
NoPNRFareCalculation::getEquivFareAmountTotal(CalcTotals& totals,
                                              CurrencyCode& currencyCodeUsed,
                                              CurrencyNoDec& noDecUsed)
{
  MoneyAmount theTotal = 0;

  theTotal += (totals.equivFareAmount) * (totals.farePath->paxType()->number());
  if (!totals.equivCurrencyCode.empty()) // Some PAX may not be priced with XO.
  {
    currencyCodeUsed = totals.equivCurrencyCode;
    noDecUsed = totals.taxNoDec();
  }
  return (theTotal);
}

//----------------------------------------------------------------------------
// getTaxTotal
//----------------------------------------------------------------------------
MoneyAmount
NoPNRFareCalculation::getTaxTotal(CalcTotals& totals,
                                  CurrencyCode& currencyCodeUsed,
                                  CurrencyNoDec& noDecUsed)
{
  MoneyAmount theTotal = 0;

  theTotal += (totals.taxAmount()) * (totals.farePath->paxType()->number());
  currencyCodeUsed = totals.taxCurrencyCode();
  noDecUsed = totals.taxNoDec();

  return (theTotal);
}

void
NoPNRFareCalculation::displayFareLineInfo(CalcTotals& calcTotals)
{
  LOG4CXX_DEBUG(logger, "NoPNRFareCalculation::displayFareLineInfo ENTER");
  calcTotals.fcConfig = _fcConfig->fcc();
  resetCounters();
  getTotalFare(calcTotals);

  // fare line number
  _fareCalcDisp << std::right << std::setw(2) << std::setfill('0')
                << calcTotals.wpaInfo.psgDetailRefNo;

  _fareCalcDisp << PrivateIndicator::privateFareIndicator(calcTotals.privateFareIndSeq);

  // obtain & display fareBasis and booking code(s)
  std::string fareBasis = getFareBasis(calcTotals);
  SpanishFamilyDiscountDesignator appender =
      spanishFamilyDiscountDesignatorBuilder(*_trx,
                                             calcTotals,
                                             FareCalcConsts::MAX_FARE_BASIS_SIZE_WQ_WPA);
  appender(fareBasis);

  _fareCalcDisp << std::setw(FareCalcConsts::MAX_FARE_BASIS_SIZE_WQ_WPA) << std::left
                << std::setfill(' ') << fareBasis << ' ';

  const std::string& bookingCode = getBookingCode(calcTotals);
  firstBookingCodes.push_back(bookingCode);
  _fareCalcDisp << std::setw(9) << std::left << std::setfill(' ') << bookingCode << ' ';

  if (calcTotals.farePath && calcTotals.farePath->intlSurfaceTvlLimit())
  {
    _fareCalcDisp << "ISSUE SEP TKTS-INTL SURFACE RESTRICTED";
  }
  else
  {
    if (_noPNRConfigOptions->fareLineHeaderFormat() == 'N')
    {
      displayTotalFareAmount(calcTotals);
      displayNonCOCCurrencyIndicator(calcTotals);
      displayCurrencyCode(calcTotals);
      _fareCalcDisp << std::setw(17) << "INCL TAX";
      displayTruePaxType(calcTotals);
    }
    else
    {
      displayCurrencyCode(calcTotals);
      displayBaseFareAmount(calcTotals);
      displayNonCOCCurrencyIndicator(calcTotals);
      displayTaxAmount(calcTotals);
      displayTotalFareAmount(calcTotals);
      displayTruePaxType(calcTotals);
    }
  }
  _fareCalcDisp << std::endl;
}

void
NoPNRFareCalculation::displayTotalFareAmount(CalcTotals& calcTotals)
{
  _fareCalcDisp << std::fixed << std::right << std::setw(12) << std::setfill(' ');
  _equivNoDec = _fareNoDec;

  if (!(_trx->getOptions()->currencyOverride().empty() ||
        _trx->getOptions()->currencyOverride() == _fareCurrencyCode) &&
      !(calcTotals.convertedBaseFareCurrencyCode == calcTotals.equivCurrencyCode))
  {
    _equivNoDec = calcTotals.equivNoDec;
  }

  if (_trx->getOptions()->currencyOverride().empty() ||
      _trx->getOptions()->currencyOverride() == _fareCurrencyCode)
  {
    if (calcTotals.convertedBaseFareCurrencyCode == calcTotals.equivCurrencyCode)
    {
      _totalFareAmount = _totalBaseAmount + calcTotals.taxAmount();
    }
    else
    {
      _totalFareAmount = calcTotals.convertedBaseFare + calcTotals.taxAmount();
    }
  }
  else
  {
    if (calcTotals.convertedBaseFareCurrencyCode == calcTotals.equivCurrencyCode)
    {
      _totalFareAmount = _totalBaseAmount + calcTotals.taxAmount();
    }
    else
    {
      _totalFareAmount = calcTotals.equivFareAmount + calcTotals.taxAmount();
    }
  }

  if (_noPNRConfigOptions->fareLineHeaderFormat() == 'N')
    _fareCalcDisp << std::setw(14);

  _fareCalcDisp << std::setprecision(_equivNoDec) << _totalFareAmount;
}

void
NoPNRFareCalculation::displayNonCOCCurrencyIndicator(CalcTotals& calcTotals)
{
  // was the base fare converted ?
  if (_noPNRConfigOptions->displayNonCOCCurrencyInd() == 'Y' &&
      (calcTotals.convertedBaseFareCurrencyCode != calcTotals.equivCurrencyCode))
  {
    CurrencyCode originationCurrency = calcTotals.convertedBaseFareCurrencyCode;
    determineOriginationCurrency(calcTotals, originationCurrency);
    // fare has been converted, was the base fare currecy the
    // same as currency of Country Of Commencement of travel?
    if (originationCurrency != calcTotals.convertedBaseFareCurrencyCode)
    {
      // display NonCOC indicator ('*')
      _fareCalcDisp << '*';
      return;
    }
  }
  // do not display anything (' ')
  _fareCalcDisp << ' ';
}

void
NoPNRFareCalculation::displayTruePaxType(const CalcTotals& calcTotals)
{
  if (_noPNRConfigOptions->displayTruePTCInFareLine() == 'Y')
  {
    _fareCalcDisp << " " << std::left << std::setw(3) << calcTotals.truePaxType;
  }
  else
  {
    _fareCalcDisp << "    ";
  }
}

void
NoPNRFareCalculation::displayVerifyBookingClassMsg()
{
  if (_noPNRConfigOptions->noMatchRBDMessage() != OPTION_3)
  {
    std::string noMatchRBDMessage;
    if (_fcConfig->getMsgAppl(FareCalcConfigText::WPA_NO_MATCH_VERIFY_BOOKING_CLASS,
                              noMatchRBDMessage))
    {
      if (_noPNRConfigOptions->noMatchRBDMessage() == OPTION_1)
        _fareCalcDisp << "ATTN*";

      _fareCalcDisp << noMatchRBDMessage << std::endl;
    }
  }
}

void
NoPNRFareCalculation::displayWqTrailerMessage()
{
  NoPNRPricingOptions* nppo = dynamic_cast<NoPNRPricingOptions*>(_trx->getOptions());
  if (!_allRequireRebook && nppo && !nppo->isNoMatch() && !nppo->isMatchAndNoMatchRequested())
  {
    std::string strPrefix("");
    if (_optionsToRebook.empty())
    {
      if (_noPNRConfigOptions->allMatchTrailerMessage() == OPTION_3)
        return;
      if (_noPNRConfigOptions->allMatchTrailerMessage() == OPTION_1)
        strPrefix = "ATTN*";
    }
    else
    {
      if (_noPNRConfigOptions->matchIntegratedTrailer() == OPTION_3)
        return;
      if (_noPNRConfigOptions->matchIntegratedTrailer() == OPTION_1)
        strPrefix = "ATTN*";
    }
    _fareCalcDisp << strPrefix << "SEE OTHER FARES - USE XM QUALIFIER, E.G. WQCTY/ACRCTY-XM"
                  << std::endl;
    _fareCalcDisp << strPrefix << "SEE ALL FARES - USE AL QUALIFIER, E.G. WQCTY/ACRCTY-AL"
                  << std::endl;
  }
}

bool
NoPNRFareCalculation::noFaresForPsg(const std::vector<CalcTotals*>& calcTotalsList)
{
  return ((calcTotalsList.size() == 1 && calcTotalsList.front() &&
           (calcTotalsList.front()->farePath == nullptr ||
            !(calcTotalsList.front()->farePath->processed()))) ||
          (calcTotalsList.size() == 0));
}

bool
NoPNRFareCalculation::noFaresForNoPaxType()
{
  for (std::vector<PaxType*>::const_iterator pti = _trx->paxType().begin(),
                                             iend = _trx->paxType().end();
       pti != iend;
       ++pti)
  {
    std::vector<CalcTotals*>& calcTotalsList = getCalcTotals(*pti);
    if (!noFaresForPsg(calcTotalsList))
      return false;
  }
  return true;
}

void
NoPNRFareCalculation::displayWarnings(const CalcTotals& calcTotals)
{
  if (calcTotals.farePath == nullptr)
    return;

  DisplayWarningsForNoPnrFC displayWarnings(*_trx,
                                            _fareCalcDisp,
                                            _fcConfig,
                                            _warningFopMsgs,
                                            _warningEtktCat15,
                                            calcTotals,
                                            _noPNRConfigOptions,
                                            *this);
  displayWarnings.display();
}

// sort the warnings in following order:
//   1. true ptc messages
//   2. rule warning messages
//   3. all other warning messages

//FIXME
/* We don't have any documentation how it have to work,
 * we can't fix it in proper way
 */
struct _sortFcMessages
{
  bool operator()(const FcMessage* msg1, const FcMessage* msg2)
  {
    if (msg1 == nullptr || msg2 == nullptr)
      return true;
    else if (msg1->messageContent() == FcMessage::TRUE_PTC_MESSAGE ||
             msg2->messageContent() == FcMessage::TRUE_PTC_MESSAGE)
      return (msg1->messageContent() == FcMessage::TRUE_PTC_MESSAGE);
    else if (msg1->messageType() != FcMessage::NOPNR_RULE_WARNING &&
             msg2->messageType() != FcMessage::NOPNR_RULE_WARNING)
    {
      return msg1->messageSubType() <= msg2->messageSubType();
    }
    else if (msg1->messageType() == FcMessage::NOPNR_RULE_WARNING &&
             msg2->messageType() == FcMessage::NOPNR_RULE_WARNING)
      return msg1->messageText() < msg2->messageText();
    else
      return msg1->messageType() == FcMessage::NOPNR_RULE_WARNING;
  }
} sortFcMessages;

// find booking codes for all travel segments
void
NoPNRFareCalculation::collectBookingCodes(std::vector<std::string>& appplicableBookingClasses,
                                          CalcTotals& calcTotals)
{
  unsigned int bcrCount = calcTotals.bookingCodeRebook.size();
  unsigned int tsIndex = 0;
  appplicableBookingClasses.resize(calcTotals.farePath->itin()->travelSeg().size());

  // iterate through TravelSeg vector
  for (std::vector<TravelSeg*>::const_iterator
           i = calcTotals.farePath->itin()->travelSeg().begin(),
           iend = calcTotals.farePath->itin()->travelSeg().end();
       i != iend;
       ++i, ++tsIndex)
  {
    // get another AirSeg
    AirSeg* as = dynamic_cast<AirSeg*>(*i);
    if (as == nullptr)
      continue;

    // check segment type - for WQ only open segments are applicable here
    if (as->segmentType() != Open)
    {
      appplicableBookingClasses[tsIndex] = "";
    }
    else if (bcrCount > 0 && tsIndex < bcrCount)
    {
      // check bookingCodeRebook vector for updated booking codes
      if (calcTotals.bookingCodeRebook[tsIndex].empty())
        appplicableBookingClasses[tsIndex] = (*i)->getBookingCode();
      else
        appplicableBookingClasses[tsIndex] = calcTotals.bookingCodeRebook[tsIndex];
    }
    else // apply user - entered booking code
    {
      appplicableBookingClasses[tsIndex] = (*i)->getBookingCode();
    }
  }
}

// returns "APPLICABLE BOOKING CLASS - 1X 2Y 3Z (...)" line
std::string
NoPNRFareCalculation::getApplicableBookingCodesLine(CalcTotals& calcTotals)
{
  if (!isNomatch(calcTotals) || (_noPNRConfigOptions->rbdNoMatchTrailerMsg2() == OPTION_3))
    return "";

  std::ostringstream bookingCodesLine;
  if (_noPNRConfigOptions->rbdNoMatchTrailerMsg2() == OPTION_1)
    bookingCodesLine << "ATTN*";

  bookingCodesLine << "APPLICABLE BOOKING CLASS - ";

  std::vector<std::string> appplicableBookingClasses;

  // get the booking codes
  collectBookingCodes(appplicableBookingClasses, calcTotals);

  // now prepare reponse line
  int currLineLen = bookingCodesLine.str().length();
  std::ostringstream newItem;

  for (size_t i = 0; i < appplicableBookingClasses.size(); ++i)
  {
    // don't show, if not open segment
    const TravelSeg* seg = calcTotals.farePath->itin()->travelSeg().at(i);
    if (!seg || seg->segmentType() != Open)
      continue;

    // don't show, if booking code matches user-entered
    if (seg->getBookingCode() == appplicableBookingClasses.at(i))
      continue;

    newItem.str(std::string()); // clear new item
    newItem << i + 1 << appplicableBookingClasses.at(i);
    if (currLineLen + newItem.str().length() > WINDOW_WIDTH)
    {
      currLineLen = 0;
      bookingCodesLine << std::endl;
    }
    currLineLen += newItem.str().length() + 1;
    bookingCodesLine << newItem.str() << ' ';
  }

  bookingCodesLine << std::endl;
  return bookingCodesLine.str();
}

// returns "[X/Y/Z(...)] line - to be later used in secondary responses
std::string
NoPNRFareCalculation::getPSSBookingCodesLine(CalcTotals& calcTotals)
{
  std::vector<std::string> appplicableBookingClasses;
  collectBookingCodes(appplicableBookingClasses, calcTotals);

  std::ostringstream bookingCodesLine;
  bookingCodesLine << '[';
  std::vector<std::string>::iterator bkListEnd = appplicableBookingClasses.end();

  for (std::vector<std::string>::iterator bk = appplicableBookingClasses.begin(); bk != bkListEnd;
       ++bk)
    bookingCodesLine << (*bk) << (bk + 1 != bkListEnd ? "/" : "");
  bookingCodesLine << "]" << std::endl;

  return bookingCodesLine.str();
}

// function formatting fare basis in accordance with FARECALCCONFIG 'FAREBASISTKTDESLNG' field
void
NoPNRFareCalculation::applyTksDsgLen(char tktDesLength, std::string& fareBasis)
{
  std::string::size_type designatorMaximumLen, fbCodeMaximumLen;

  // maximum length of designator (part after '/' sign)
  designatorMaximumLen =
      tktDesLength == OPTION_1 ? 6 : tktDesLength == OPTION_2 ? 5 : tktDesLength == OPTION_3
                                                                        ? 4
                                                                        : std::string::npos;

  // maximum length of fare basis code (part before '/' sign)
  fbCodeMaximumLen =
      (designatorMaximumLen != std::string::npos) ? 14 - designatorMaximumLen : std::string::npos;

  if (designatorMaximumLen != std::string::npos)
  {
    // apply tktDesLenght
    std::string::size_type slashPos = fareBasis.find("/");

    std::string fbCode =
        (slashPos == std::string::npos) ? fareBasis : fareBasis.substr(0, slashPos);

    std::string fbDesignator = (slashPos == std::string::npos) ? "" // no designator
                                                               : fareBasis.substr(slashPos + 1);

    if (fbDesignator.size() > designatorMaximumLen)
      fbDesignator = fbDesignator.substr(0, designatorMaximumLen);

    if (fbCode.size() > fbCodeMaximumLen)
      fbCode = fbCode.substr(0, fbCodeMaximumLen);

    fareBasis = (fbDesignator.size() > 0) ? fbCode + "/" + fbDesignator : fbCode;
  }
  else
  {
    // default - 15 characters free field
    if (fareBasis.length() > 15)
      fareBasis = fareBasis.substr(0, 15);
  }
}

void
NoPNRFareCalculation::displayCxrBkgCodeFareBasis(PricingTrx& trx,
                                                 CalcTotals& calcTotals,
                                                 AirSeg* airSeg,
                                                 std::string& _fareBasisCode)
{
  std::string updatedFareBasis = _fareBasisCode;
  applyTksDsgLen(_fcConfig->fareBasisTktDesLng(), updatedFareBasis);

  _fareCalcDisp << std::left << std::setfill(' ');
  _fareCalcDisp << ' ';
  _fareCalcDisp << std::setw(4) << airSeg->carrier();

  uint16_t i = airSeg->pnrSegment() - 1;
  {
    std::vector<TravelSeg*>::const_iterator tsI =
        std::find(calcTotals.farePath->itin()->travelSeg().begin(),
                  calcTotals.farePath->itin()->travelSeg().end(),
                  airSeg);
    if (tsI != calcTotals.farePath->itin()->travelSeg().end())
    {
      i = std::distance(calcTotals.farePath->itin()->travelSeg().begin(), tsI);
    }
  }

  if (i < calcTotals.bookingCodeRebook.size() && !calcTotals.bookingCodeRebook[i].empty())
  {
    _fareCalcDisp << std::setw(4) << calcTotals.bookingCodeRebook[i] /*[0]*/;
  }
  else
    _fareCalcDisp << std::setw(4) << airSeg->getBookingCode();

  _fareCalcDisp << std::setw(6);

  displayItinTravelSegmentDepartureDate(airSeg);

  _fareCalcDisp << std::setw(15) << updatedFareBasis << ' ';
}

void
NoPNRFareCalculation::displayItinTravelSegmentDepartureDate(AirSeg* airSeg)
{
  if (airSeg->segmentOrder() == 1 ||
      !(airSeg->hasEmptyDate() || _trx->itin().front()->dateType() == Itin::NoDate))
    _fareCalcDisp << airSeg->departureDT().dateToString(DDMMM, "");
  else
    _fareCalcDisp << "OPEN";
}

void
NoPNRFareCalculation::displayTaxAmount(CalcTotals& calcTotals, unsigned int taxFieldWidth)
{
  std::ostringstream taxAmountString;
  taxAmountString << std::fixed << std::right << std::setw(taxFieldWidth) << std::setfill(' ');

  if (_trx->getRequest()->isExemptAllTaxes())
  {
    taxAmountString << ' ';
  }
  else
  {
    taxAmountString << std::setprecision(calcTotals.taxNoDec()) << calcTotals.taxAmount();
  }

  if (taxAmountString.str().size() > taxFieldWidth)
  {
    LOG4CXX_FATAL(logger,
                  "Tax amount " << taxAmountString.str() << " exceeds max tax field length "
                                << taxFieldWidth);
    throw ErrorResponseException(ErrorResponseException::EXCEED_LENGTH_UNABLE_TO_CALCULATE_FARE);
  }
  else
  {
    _fareCalcDisp << taxAmountString.str();
  }
}

void
NoPNRFareCalculation::determineOriginationCurrency(CalcTotals& calcTotals,
                                                   CurrencyCode& originationCurrency)
{
  // to determine origination currency we must find first international fare
  originationCurrency = _trx->itin().front()->originationCurrency(); // default
  FareUsageIter fuIter(*calcTotals.farePath);
  for (auto& elem : fuIter)
  {
    if (elem->paxTypeFare()->isInternational())
    {
      // found first international fare
      const TravelSeg* ts = elem->travelSeg().front();
      if (ts)
      {
        NationCode nation(ts->origin()->nation());
        LOG4CXX_DEBUG(logger,
                      "COC determination - found first international fare originating in "
                          << nation << " starting segment: " << ts->segmentOrder());
        CurrencyUtil::getInternationalPricingCurrency(
            nation, originationCurrency, _trx->ticketingDate());
        break;
      }
    }
  }
}

DisplayWarningsForNoPnrFC::DisplayWarningsForNoPnrFC(PricingTrx& trx,
                                                     FareCalc::FcStream& fareCalcDisp,
                                                     const FcConfig* fcConfig,
                                                     std::vector<std::string>& warningFopMsgs,
                                                     bool& warningEtktCat15,
                                                     const CalcTotals& calcTotals,
                                                     const NoPNROptions* options,
                                                     NoPNRFareCalculation& noPNRFareCalculation)
  : DisplayWarningsForFC(trx, fareCalcDisp, fcConfig, warningFopMsgs, warningEtktCat15, calcTotals),
    _noPnrOptions(options),
    _noPNRFareCalculation(noPNRFareCalculation)
{
}

void
DisplayWarningsForNoPnrFC::display()
{
  // save the current stream value, then some different warning messages
  std::string currResp = _fareCalcDisp.str();
  _fareCalcDisp.clear();

  // call base method
  DisplayWarningsForFC::display();

  std::string otherWarnings = _fareCalcDisp.str();
  _fareCalcDisp.clear();
  _fareCalcDisp << currResp;

  // now, display FcMessages

  // don't sort the records (elements stored by value) - sort pointers instead
  // first, collect the pointers to messages; save validating carrier message for now
  // to be shown later
  std::vector<const FcMessage*> ptrVec;
  std::vector<const FcMessage*> ptrVCXVec;
  const FcMessage* valCarrier = nullptr;
  // Remove the following 3 pointers when removing  fallbackValidatingCxrMultiSp flag
  const FcMessage* valCarrier1 = nullptr;
  const FcMessage* valCarrier2 = nullptr;
  const FcMessage* valCarrier3 = nullptr;
  const FcMessage* matchedAcc = nullptr;
  const FcMessage* privateInd = nullptr;
  const FcMessage* ietMsg = nullptr;
  std::vector<FcMessage>& toSort = const_cast<CalcTotals&>(_calcTotals).fcMessage;
  if ( _trx.isValidatingCxrGsaApplicable() )
  {
    for (std::vector<FcMessage>::const_iterator it = toSort.begin(); it != toSort.end(); ++it)
    {
      if (!valCarrier && (*it).messageContent() == FcMessage::CARRIER_MESSAGE)
      {
        valCarrier = &(*it); // save validating carrier message
      }
      else if ((!fallback::fallbackValidatingCxrMultiSp(&_trx) || _trx.overrideFallbackValidationCXRMultiSP())
              && ((*it).messageContent() == FcMessage::CARRIER_MESSAGE))
      {
        ptrVCXVec.push_back(&(*it));
      }
      else if (!valCarrier1 && (*it).messageContent() == FcMessage::CARRIER_MESSAGE)
         valCarrier1 = &(*it); // save validating carrier message
      else if (!valCarrier2  && (*it).messageContent() == FcMessage::CARRIER_MESSAGE)
        valCarrier2 = &(*it); // save validating carrier message
      else if (!valCarrier3 && (*it).messageContent() == FcMessage::CARRIER_MESSAGE)
        valCarrier3 = &(*it); // save validating carrier message
      else if ((*it).messageContent() == FcMessage::INTERLINE_MESSAGE)
        ietMsg = &(*it); // save matched IET message
      else if ((*it).messageContent() == FcMessage::MATCHED_ACCOUNT_MESSAGE)
        matchedAcc = &(*it); // save matched AccCode message
      else if ((*it).messageContent() == FcMessage::PVT_INDICATOR)
       privateInd = &(*it); // save private Indicator message
      else
        ptrVec.push_back(&(*it));
    }
  }
  else
  {
    for (std::vector<FcMessage>::const_iterator it = toSort.begin(); it != toSort.end(); ++it)
    {
      if ((*it).messageContent() == FcMessage::CARRIER_MESSAGE)
        valCarrier = &(*it); // save validating carrier message
      else if ((*it).messageContent() == FcMessage::INTERLINE_MESSAGE)
        ietMsg = &(*it); // save matched IET message
      else if ((*it).messageContent() == FcMessage::MATCHED_ACCOUNT_MESSAGE)
        matchedAcc = &(*it); // save matched AccCode message
      else if ((*it).messageContent() == FcMessage::PVT_INDICATOR)
        privateInd = &(*it); // save private Indicator message
      else
        ptrVec.push_back(&(*it));
    }
  }
  // sort the messages via pointers
  std::sort(ptrVec.begin(), ptrVec.end(), sortFcMessages);

  for (const auto ptr : ptrVec)
    display(*ptr);

  // now, display the rest of the messages, and finally -
  // - validating carrier message (if there is one)
  _fareCalcDisp << otherWarnings;

  // display GI messages just before validating carrier msg
  _noPNRFareCalculation.displayGIMessage();

  // dispaly "private IND" message before validating carrier msg
  if (privateInd)
    display(*privateInd);
  // display Interline ticketing carrier message
  if (ietMsg)
    display(*ietMsg);
  if (valCarrier)
  {
    display(*valCarrier);
    if ( _trx.isValidatingCxrGsaApplicable() )
    {
      if ( !fallback::fallbackValidatingCxrMultiSp(&_trx)  || _trx.overrideFallbackValidationCXRMultiSP())
      {
        for (std::vector<const FcMessage*>::iterator it = ptrVCXVec.begin(); it != ptrVCXVec.end(); ++it)
          display(**it);
      }
      else
      {
        if (valCarrier1)
          display(*valCarrier1);
        if (valCarrier2)
          display(*valCarrier2);
        if (valCarrier3)
          display(*valCarrier3);
      }
    }
  }
  // display matched AccCode msg after validating carrier msg
  if (matchedAcc)
    display(*matchedAcc);
}

void
DisplayWarningsForNoPnrFC::display(const FcMessage& message)
{
  if (message.messageType() == FcMessage::NOPNR_RULE_WARNING &&
      (_noPnrOptions->displayFareRuleWarningMsg2() == 'N'))
    return;

  DisplayWarningsForFC::display(message);
}

void
DisplayWarningsForNoPnrFC::displayFcMessages()
{
  // do not display them here - they should be displayed in 'display()'
}

} // tse
