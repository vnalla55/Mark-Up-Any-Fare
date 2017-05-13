#include "FareCalc/AltFareCalculation.h"

#include "Common/AccTvlDetailOut.h"
#include "Common/BaggageStringFormatter.h"
#include "Common/DiagMonitor.h"
#include "Common/FareCalcUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/OBFeesUtils.h"
#include "Common/ParseUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/PaxTypeInfo.h"
#include "FareCalc/AltFareCalcCollector.h"
#include "FareCalc/FareCalcConsts.h"
#include "FareCalc/FareUsageIter.h"
#include "FareCalc/FcDispFarePath.h"
#include "FareCalc/FcUtil.h"
#include "Rules/AccompaniedTravel.h"

#include <iterator>


namespace tse
{

static Logger
logger("atseintl.FareCalc.AltFareCalculation");

void
AltFareCalculation::process()
{
  ////////////////////////////////////////////////////////////////////////////
  //
  bool needDiagAccTvlOut = false;
  bool needDiagAccTvlIn = false; // part of temp demo codes
  std::vector<const std::string*> accTvlData; // part of temp demo codes
  std::vector<uint16_t> optIndexes; // part of temp demo code
  ////////////////////////////////////////////////////////////////////////////

  const FareCalcCollector::CalcTotalsMap& cm = _fcCollector->calcTotalsMap();
  int psgCount = 0;

  DiagnosticTypes diagType = _trx->diagnostic().diagnosticType();
  if (diagType == Diagnostic860)
  {
    displayRuler();
  }

  _primaryResp = true;

  if (_trx->getRequest()->ticketingAgent()->axessUser()) // JAL/AXESS agent
  {
    if (_trx->altTrxType() == PricingTrx::WPA && cm.size() == _trx->paxType().size() &&
        (_trx->getRequest()->lowFareRequested() != 'T'))
    {
      _fareCalcDisp << "VT " << std::endl;
    }
    else
      _fareCalcDisp << "VD " << std::endl;
  }

  bool dispDtlFormat = checkDetailFormat();

  if (_trx->getRequest()->multiTicketActive())
  {
    _fareCalcDisp << "SINGLE TICKET REQUIRED\n";
  }

  if (dispDtlFormat)
  {
    std::set<PaxType*, PaxType::InputOrder> inOrderPaxType(_trx->paxType().begin(),
                                                           _trx->paxType().end());

    for (std::set<PaxType*, PaxType::InputOrder>::const_iterator pti = inOrderPaxType.begin(),
                                                                 ptend = inOrderPaxType.end();
         pti != ptend;
         ++pti)
    {
      if (_fcConfig->wpaPsgLineBreak() == FareCalcConsts::FC_YES && pti != inOrderPaxType.begin())
      {
        _fareCalcDisp << "  " << std::endl;
      }

      CalcTotals* calcTotalsTemp = nullptr;
      for (const auto& elem : cm)
      {
        calcTotalsTemp = selectCalcTotals(*_trx, elem.second, _needNetRemitCalc);
        CalcTotals& calcTotals = *calcTotalsTemp;

        if (calcTotals.farePath->paxType() != (*pti))
          continue;

        if (checkExistFarePath(*_trx, calcTotals))
          calcTotals.wpaInfo.psgDetailRefNo = ++psgCount;

        displayPsgDetailFormat(calcTotals, true);
        if (diagType == Diagnostic854)
        {
          diagTruePaxType(calcTotals);
          diagJourney(calcTotals);
        }
      }
    }

    if (!_secondaryResp)
      displayCorpIdTrailerMessage();
  }
  else
  {
    checkDiagNeeds(needDiagAccTvlOut, needDiagAccTvlIn, optIndexes);
    std::vector<uint16_t>::const_iterator optIndexIter = optIndexes.begin();
    const std::vector<uint16_t>::const_iterator optIndexIterEnd = optIndexes.end();

    AccTvlDetailOut<FareCalc::FcStream> accTvlDetailOut('\n');
    if (needDiagAccTvlOut)
      accTvlDetailOut.printFormat(_fareCalcDisp);

    std::set<PaxType*, PaxType::InputOrder> inOrderPaxType(_trx->paxType().begin(),
                                                           _trx->paxType().end());

    for (std::set<PaxType*, PaxType::InputOrder>::const_iterator pti = inOrderPaxType.begin(),
                                                                 ptend = inOrderPaxType.end();
         pti != ptend;
         ++pti)
    {
      bool firstOption = true;
      uint16_t startIndex = psgCount + 1;
      bool verifyBooking = false;
      // initialize by default it's true
      _allRequireRebook = true;

      // Process the calc totals based on the WPA Sort setting:
      std::vector<CalcTotals*> calcTotalsList;
      getCalcTotals(*pti, calcTotalsList, true);

      // Fare line format - break between different PTC.
      if (_fcConfig->wpaPsgMultiLineBreak() == 'Y' && pti != inOrderPaxType.begin())
        _fareCalcDisp << "  " << std::endl;

      // Check for No-Match
      if (calcTotalsList.size() == 1 && calcTotalsList.front() &&
          calcTotalsList.front()->farePath->processed() == false)
      {
        displayPsgrInfo(*calcTotalsList.front(), false);
        displayNoMatchResponse();
        displayWpaTrailerMessage();
        continue;
      }

      // clear out the RO option list for this PAX type
      _roOptions.clear();

      // WPA mix - option numbers for rebook
      _optionsToRebook.clear();

      for (std::vector<CalcTotals*>::iterator i = calcTotalsList.begin(),
                                              iend = calcTotalsList.end();
           i != iend;
           ++i)
      {

        CalcTotals* calcTotalsTemp = nullptr;
        calcTotalsTemp = selectCalcTotals(*_trx, *i, _needNetRemitCalc);
        CalcTotals& calcTotals = *calcTotalsTemp;

        if (calcTotals.farePath->processed() == false)
          continue;

        calcTotals.wpaInfo.psgDetailRefNo = ++psgCount;

        if (_trx->altTrxType() == AltPricingTrx::WPA)
        {
          if (!calcTotals.farePath->noMatchOption())
          {
            _allRequireRebook = false;
          }
          else
          {
            _optionsToRebook.push_back(psgCount);
          }
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
          verifyBooking |= verifyBookingClass(calcTotalsList);
          displayFareLineHdr();
        }

        displayPsgFareLineFormat(calcTotals);

        ////////////////////////////////////////////////////////////////
        //
        if (needDiagAccTvlOut)
        {
          const FarePath& farePath = *calcTotals.farePath;
          accTvlDetailOut.storeAccTvlDetail(_trx, _fareCalcDisp, calcTotals.truePaxType, farePath);
          _fareCalcDisp << std::endl;
        }

        if (needDiagAccTvlIn) // Temp
        {
          if (optIndexIter != optIndexIterEnd)
          {
            if (calcTotals.wpaInfo.psgDetailRefNo == *optIndexIter)
            {
              accTvlData.push_back(&calcTotals.wpaInfo.accTvlData);
              optIndexIter++;
            }
          }
        }
        ////////////////////////////////////////////////////////////////
      }

      displayRoMessage(_roOptions);
      displayFareTypeMessage(calcTotalsList);
      if (verifyBooking)
        displayRebookMessage(_optionsToRebook);

      displayWpaTrailerMessage();

      // warning message for all options of same PaxType:
      displayPaxTypeWarnings(calcTotalsList, startIndex);
    }

    displayCorpIdTrailerMessage();
  }

  ////////////////////////////////////////////////////////////////////////////
  //
  if (needDiagAccTvlIn && !accTvlData.empty()) // Temp
  {
    AccompaniedTravel accTvl;
    std::vector<bool> resultVec;
    accTvl.validateAccTvl(*_trx, accTvlData, resultVec);
  }

  displaySegmentFeeMessage();
  ////////////////////////////////////////////////////////////////////////////

  if (_trx->getRequest()->diagnosticNumber() == Diagnostic860)
  {
    displayRuler();
  }

  {
    if (diagType == DiagnosticNone || diagType == Diagnostic854 || diagType == Diagnostic860 ||
        diagType == 1860)
    {
      DiagMonitor diag(*_trx, diagType);
      diag << _fareCalcDisp.str() << '\n';
      LOG4CXX_DEBUG(logger, _fareCalcDisp.str());
    }
  }

  // Unless it is a straight WP entry, we should always generate the
  // detail responses since the user expects to be able to do WPn
  // entries after WPA or WP-nomatch
  if (_trx->altTrxType() != PricingTrx::WP || _trx->isRfbListOfSolutionEnabled())
    createWpnResponse();
}

void
AltFareCalculation::checkDiagNeeds(bool& needDiagAccTvlOut,
                                   bool& needDiagAccTvlIn,
                                   std::vector<uint16_t>& optIndexes)
{
  uint16_t countPsgrInRequest = _trx->paxType().size();
  if (countPsgrInRequest <= 1)
    return;

  Diagnostic& diagnostic = _trx->diagnostic();
  if ((diagnostic.diagnosticType() != Diagnostic860) || (!diagnostic.isActive()))
    return;

  std::vector<const std::string*> accTvlData;

  std::map<std::string, std::string>::iterator diagOption =
      diagnostic.diagParamMap().find(Diagnostic::DISPLAY_DETAIL);

  if (diagOption == diagnostic.diagParamMap().end())
    return;

  const std::string seperator = "P";
  if (!((*diagOption).second).empty())
  {
    if ((*diagOption).second == "ATO")
    {
      needDiagAccTvlOut = true;
    }
    else if ((*diagOption).second.substr(0, 3) == "ATI")
    {
      needDiagAccTvlIn = true;
      uint16_t optLength = uint16_t((*diagOption).second.length() - 3);
      if (optLength > 0)
      {
        ParseUtil::parseIndexes(optIndexes, (*diagOption).second.substr(3, optLength), seperator);
      }
    }
  }
}

void
AltFareCalculation::displayPsgrInfo(const CalcTotals& calcTotals, bool detailFormat)
{
  if (_fcConfig->wpaFareLinePsgType() == '2')
  {
    _fareCalcDisp << "INPUT PSGR TYPE  " << calcTotals.requestedPaxType;
  }
  else if (_fcConfig->wpaFareLinePsgType() == '3')
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
    _fareCalcDisp << " PSGR TYPE";
  }
  else
  {
    if (_fcConfig->wpaFareLinePsgType() == '2')
    {
      _fareCalcDisp << "INPUT PSGR TYPE  ";
    }
    else // _fcConfig->wpaFareLinePsgType() == '1'
    {
      _fareCalcDisp << "PSGR TYPE  ";
    }

    if ((_primaryResp && detailFormat) || _secondaryResp)
    {
      displayPsgrType(calcTotals);
    }
    else
    {
      _fareCalcDisp << calcTotals.requestedPaxType;
    }
  }

  if (detailFormat && _primaryResp && _fcConfig->wpaPrimePsgRefNo() == 'Y')
  {
    _fareCalcDisp << " - " << std::right << std::setw(2) << std::setfill('0')
                  << FareCalcUtil::getPtcRefNo(*_trx, calcTotals.farePath->paxType());
  }
  else if (_secondaryResp && _fcConfig->wpa2ndPsgRefNo() == 'Y')
  {
    _fareCalcDisp << " - " << std::right << std::setw(2) << std::setfill('0') << calcTotals.wpaInfo.psgDetailRefNo;
  }

  _fareCalcDisp << std::endl;
}

void
AltFareCalculation::displayPsgDetailFormat(CalcTotals& calcTotals,
                                           bool displayObFees)
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
    displayWpaTrailerMessage();
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

  resetCounters(); // reset before process next passenger type
  displayFareTax(*_trx, *_fcCollector, calcTotals);

  if (_fcConfig->itinDisplayInd() == FareCalcConsts::FC_YES ||
      _fcConfig->fareTaxTotalInd() == FareCalcConsts::HORIZONTAL_FARECALC1)
  {
    if (!_trx->getRequest()->ticketingAgent()->abacusUser() &&
        !_trx->getRequest()->ticketingAgent()->axessUser() &&
        !_trx->getRequest()->ticketingAgent()->infiniUser())
    {
      if (_secondaryResp || FareCalcUtil::isOneSolutionPerPaxType(_trx))
      {
        processGrandTotalLineWPnn(*_trx, calcTotals);
      }
    }

    displayPsgFareCalc(*_trx, *_fcCollector, calcTotals);

    // Display Net fare result w/o Tax and Total for WPNETT
    if (_trx->getRequest()->ticketingAgent()->axessUser() &&
        _trx->getRequest()->isWpNettRequested() && _cat35CalcTotals != nullptr)
    {
      displayPsgFareCalc(*_trx, *_fcCollector, *_cat35CalcTotals);
    }
  }

  if (calcTotals.dispSegmentFeeMsg())
    _dispSegmentFeeMsg = true;

  if (!OBFeesUtils::fallbackObFeesWPA(_trx) && displayObFees)
   displayObFeesForSingleMatch(calcTotals);

  displayCommandPricingAndVariousMessages(&calcTotals);
}

void
AltFareCalculation::displayPaxTypeWarnings(const std::vector<CalcTotals*>& calcTotalList,
                                           const uint16_t& startIndex)
{
  displayPaxTypeWarningsAccTvl(calcTotalList);
}

void
AltFareCalculation::displayPaxTypeWarningsAccTvl(const std::vector<CalcTotals*>& calcTotalList)
{
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
    _fareCalcDisp << _warning << "ACCOMPANYING TRAVEL RESTRICTIONS MAY APPLY\n";
  }
}

void
AltFareCalculation::getCalcTotals(const PaxType* paxType,
                                  std::vector<CalcTotals*>& calcTotalsList,
                                  bool sortList) const
{
  calcTotalsList.clear();

  for (const auto& elem : _fcCollector->calcTotalsMap())
  {
    if (elem.second->farePath && elem.second->farePath->paxType() == paxType)
    {
      calcTotalsList.push_back(elem.second);
    }
  }

  if (sortList)
  {
    bool asc = (_fcConfig->wpaSort() != 'H');
    std::sort(calcTotalsList.begin(), calcTotalsList.end(), CalcTotalsCompare(asc));
  }
}

void
AltFareCalculation::createWpnResponse()
{
  if (_trx->getRequest()->diagnosticNumber() == DiagnosticNone)
  {
    const FareCalcCollector::CalcTotalsMap& cm = _fcCollector->calcTotalsMap();

    _psgrCount = 1;
    _primaryResp = false;
    _secondaryResp = true;

    for (FareCalcCollector::CalcTotalsMap::const_iterator i = cm.begin(), iend = cm.end();
         i != iend;
         ++i)
    {
      _cat35CalcTotals = nullptr;
      if (_trx->getRequest()->ticketingAgent()->axessUser() &&
          _trx->getRequest()->isWpNettRequested() && i->second->netRemitCalcTotals != nullptr)
      {
         _cat35CalcTotals = i->second;
      }

      CalcTotals* tmp_calcTotals = nullptr;

      tmp_calcTotals = selectCalcTotals(*_trx, i->second, _needNetRemitCalc);

      if (tmp_calcTotals == nullptr)
        continue;

      CalcTotals& calcTotals = *tmp_calcTotals;

      if (!checkExistFarePath(*_trx, calcTotals))
        continue;

      resetCounters();
      getTotalFare(calcTotals);

      _fareCalcDisp.clear();
      displayPsgDetailFormat(calcTotals, false);
      tmp_calcTotals->wpaInfo.wpnDetailResponse = _fareCalcDisp.str();

      LOG4CXX_DEBUG(logger,
                    "WPN RESP:" << i->second->wpaInfo.psgDetailRefNo << "\n"
                                << i->second->wpaInfo.wpnDetailResponse);
    }
  }
}

//-------------------------------------------------------------------
void
AltFareCalculation::displayNoMatchResponse()
{
  std::string noMatchNoFare;
  if (_fcConfig->getMsgAppl(FareCalcConfigText::WPA_NO_MATCH_NO_FARE, noMatchNoFare))
  {
    _fareCalcDisp << noMatchNoFare << std::endl;
  }
}

void
AltFareCalculation::displayRoMessage(const std::vector<int>& roOptions)
{
  std::string roRbdTktOverride;
  if (_fcConfig->getMsgAppl(FareCalcConfigText::WPA_RO_INDICATOR, roRbdTktOverride))
  {
    if (roOptions.size() > 0)
    {
      _fareCalcDisp << '*';
      for (unsigned i = 0; i < roOptions.size(); i++)
      {
        if (i > 0)
          _fareCalcDisp << ',';

        _fareCalcDisp << std::right << std::setw(2) << std::setfill('0') << roOptions[i];
      }
      _fareCalcDisp << roRbdTktOverride << std::endl;
    }
  }
}

////////////////////////////////////////////////////////////////////////////
//
// Check RO
//
void
AltFareCalculation::CheckRO::
operator()(const FareUsage* fu)
{
 // If there are side trips, we need to check there also
  if (fu != nullptr && fu->hasSideTrip())
  {
    for (const auto& elem : fu->sideTripPUs())
    {
      FareCalc::forEachFareUsage(*elem, *this);
    }
  }
}

////////////////////////////////////////////////////////////////////////////
//
// Sorter function for CalcTotals
//
bool
AltFareCalculation::CalcTotalsCompare::less(const CalcTotals* c1, const CalcTotals* c2) const
{
  MoneyAmount m1 = c1->equivFareAmount + c1->taxAmount();
  MoneyAmount m2 = c2->equivFareAmount + c2->taxAmount();

  if (m1 == m2)
    return c1 < c2;

  return m1 < m2;
}

bool
AltFareCalculation::checkDetailFormat()
{
  if (FareCalcUtil::isOnlyOneMatchSolution(_trx))
    return true;

  // PL 14835 - Per Sterling, if it is a no-match, we should display
  // in fare-line format
  if (_trx->getRequest()->lowFareRequested() || _trx->altTrxType() == PricingTrx::WP_NOMATCH)
    return false;

  if (_secondaryResp)
    return true;

  for (std::vector<PaxType*>::const_iterator i = _trx->paxType().begin(),
                                             iend = _trx->paxType().end();
       i != iend;
       ++i)
  {
    std::vector<CalcTotals*> calcTotalsList;
    getCalcTotals(*i, calcTotalsList, false);

    if (calcTotalsList.size() > 1)
    {
      return false;
    }
    else if (calcTotalsList.size() == 1)
    {
      if (_trx->getRequest()->ticketingAgent()->infiniUser())
      {
        if (calcTotalsList.front() && calcTotalsList.front()->farePath->processed() == false)
        {
          return false;
        }
      }
      if (_fcConfig->wpaPsgDtlFormat() == '1')
      {
        continue;
      }
      else if (_fcConfig->wpaPsgDtlFormat() == '2')
      {
        return false;
      }
    }
  }

  return true;
}

void
AltFareCalculation::displayRebookMessage(const std::vector<int>& optionsToRebook)
{
  AltPricingTrx* altTrx = dynamic_cast<AltPricingTrx*>(_trx);
  if ((altTrx && altTrx->altTrxType() == AltPricingTrx::WPA &&
      _trx->getRequest()->lowFareRequested() == 'T') ||
      (altTrx && altTrx->altTrxType() == AltPricingTrx::WP_NOMATCH))
  {
    if (_trx->isLowestFareOverride() && !_allRequireRebook && !altTrx->getRequest()->isWpas())
    {
      if (!optionsToRebook.empty())
      {
        _fareCalcDisp << _warning << "REBOOK REQUIRED FOR OPTIONS - ";

        std::ostringstream options;
        uint16_t count = 0;
        uint16_t size = optionsToRebook.size();
        bool wrap = false;

        for (std::vector<int>::const_iterator i = optionsToRebook.begin(),
                                              iend = optionsToRebook.end();
             i != iend;
             ++i, ++count)
        {
          if (!options.str().empty())
            options << ',';

          if (size > 11 && (count == 11) && !wrap)
          {
            options << '\n';
            wrap = true;
          }
          options << std::setw(2) << std::setfill('0') << (*i);
        }
        _fareCalcDisp << options.str() << std::endl;
      }
    }
    else if (altTrx->getRequest()->isWpas())
    {
      _fareCalcDisp << _warning << "VERIFY BOOKING CLASS - AVAILABILITY NOT CHECKED\n";
    }
    else
    {
      std::string noMatchRebook;
      if (_fcConfig->getMsgAppl(FareCalcConfigText::WPA_NO_MATCH_REBOOK, noMatchRebook))
      {
        _fareCalcDisp << _warning << noMatchRebook << std::endl;
      }
    }
  }
}

void
AltFareCalculation::displayFareTypeMessage(const std::vector<CalcTotals*>& ctList)
{
  if (!_trx->getOptions()->isFareFamilyType())
    return;

  if (ctList.empty())
    return;

  for (const auto ct : _fcCollector->multiMessage())
  {
    std::ostringstream indexes;
    for (const auto& cL : ctList)
    {
      if (ct.second.chkFarePath(cL->farePath))
      {
        if (!indexes.str().empty())
          indexes << ',';
        indexes << std::setw(2) << std::setfill('0') << cL->wpaInfo.psgDetailRefNo;
      }
    }

    if (!indexes.str().empty())
    {
      if (!_warning.empty())
        _fareCalcDisp << _warning;
      else
        _fareCalcDisp << '*';

      _fareCalcDisp << indexes.str() << '*' << ct.first << std::endl;
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
// @function void AltFareCalculation::processGrandTotalLineWPnn()
// Description:   Prepare to display total fare and tax amount by passenger
//                by one FarePath
// @param         trx, calcTotals
// @return
// </PRE>
// ----------------------------------------------------------------------------

void
AltFareCalculation::processGrandTotalLineWPnn(PricingTrx& trx, CalcTotals& calcTotals)
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
AltFareCalculation::getBaseFareTotal(CalcTotals& totals,
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
AltFareCalculation::getEquivFareAmountTotal(CalcTotals& totals,
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
AltFareCalculation::getTaxTotal(CalcTotals& totals,
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
AltFareCalculation::displayWarnings(const CalcTotals& calcTotals)
{
  if (calcTotals.farePath == nullptr)
    return;

  if (TrxUtil::isBaggage302DotActivated(*_trx))
  {
    DisplayWarningsForFC displayWarnings(
        *_trx, _fareCalcDisp, _fcConfig, _warningFopMsgs, _warningEtktCat15, calcTotals);

    displayWarnings.display();
  }
}

void
AltFareCalculation::displayObFeesForSingleMatch(const CalcTotals& calcTotals)
{
  if (!_trx->isSingleMatch() ||
      (calcTotals.farePath->collectedTktOBFees().empty() &&
       !calcTotals.farePath->maximumObFee() ))
    return;

  _fareCalcDisp << OBFeesUtils::displayGreenScreenMsg(*_trx, calcTotals);
}

} // tse
