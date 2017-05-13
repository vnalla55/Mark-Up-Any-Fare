//-----------------------------------------------------------------------------
//  File:        MultiTicketUtil.cpp
//  Created:     2014
//
//  Description: Support Multi-ticket project
//
//  Copyright Sabre 2014
//
//               The copyright to the computer program(s) herein
//               is the property of Sabre.
//               The program(s) may be used and/or copied only with
//               the written permission of Sabre or in accordance
//               with the terms and conditions stipulated in the
//               agreement/contract under which the program(s)
//               have been supplied.
//
//-----------------------------------------------------------------------------

#include "Common/MultiTicketUtil.h"

#include "Common/FareCalcUtil.h"
#include "Common/Logger.h"
#include "Common/TravelSegUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag676Collector.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcCollector.h"

namespace tse
{
static Logger
logger("atseintl.Common.MultiTicketUtil");
Money MultiTicketUtil::_singleTicketTotal(0, INVALID_CURRENCYCODE);
Money MultiTicketUtil::_multiTicketTotal (0, INVALID_CURRENCYCODE);
std::vector<std::string> MultiTicketUtil::_respondMsg(3,"");

MultiTicketUtil::MultiTicketUtil(PricingTrx& trx) : _trx(trx), _diag676(nullptr) {}

void
MultiTicketUtil::createMultiTicketItins(PricingTrx& trx)
{
  MultiTicketUtil mtu(trx);
  mtu.createDiag676();

  mtu.createSubItins();

  mtu.finishDiag676();
}

void
MultiTicketUtil::createSubItins()
{
  if (trx().itin().empty())
    return;
  if (_diag676)
    _diag676->printAllItinAnalysis();
  std::vector<tse::Itin*>::iterator it = trx().itin().begin();
  std::vector<tse::Itin*>::iterator itE = trx().itin().end();

  for (int a = 1; it != itE; ++it, ++a)
  {
    if (a > 1 && _diag676)
      _diag676->printItinFooter();

    if (_diag676)
      _diag676->displayItin(**it, a);

    std::vector<TravelSeg*> tvlSegs = (*it)->travelSeg();
    if (tvlSegs.empty() || tvlSegs.size() == 1)
      continue;

    // Check and calculate for stopovers
    size_t count = 0;
    TravelSeg* tvl = nullptr;
    findStopOvers(tvlSegs, count, tvl);
    if (count == 0 || count > 1)
      continue;

    // break itinerary to 2 new itineraries.
    // Create and analyze both of them before push them to itin;
    std::vector<Itin*> subItins;
    createSubItins(**it, tvl, subItins);
    if (subItins.size() < 2)
    {
      if (_diag676)
        _diag676->displaySubItinNotBuildMsg();
      continue;
    }
    // create map (parent Itin, vector<sub1, sub2>)
    trx().multiTicketMap().insert(PricingTrx::MultiTicketMap::value_type(*it, subItins));
  }
}

void
MultiTicketUtil::findStopOvers(std::vector<TravelSeg*> tvlSegs,
                               size_t& count,
                               TravelSeg*& tvl)
{
  // Check for stopovers
  size_t segsSize = tvlSegs.size();
  std::vector<bool> stopOver = TravelSegUtil::calculateStopOversForMultiTkt(tvlSegs);

  // Calculate for stopovers
  for (size_t i = 0; i < segsSize - 1; i++)
  {
    if (!stopOver[i] && !tvlSegs[i]->isForcedStopOver())
      continue;
    if (!tvlSegs[i]->isForcedConx() &&
        !isMarriedSeg(tvlSegs[i]))     // check tvl for married segment
    {
      count++;
      tvl = tvlSegs[i];
      if (_diag676)
      {
        if (count == 1)
          _diag676->displayStopoverSegmentHeader();
        _diag676->displaySegment(*tvl);
      }
    }
  }
  if (_diag676)
    _diag676->displayTotalCountStopover(count);
}

bool
MultiTicketUtil::isMarriedSeg(TravelSeg* tvl)
{
  AirSeg* airSeg = dynamic_cast<AirSeg*>(tvl);
  if (airSeg == nullptr ||
      airSeg->marriageStatus() == AirSeg::MARRIAGE_NONE ||
      airSeg->marriageStatus() == AirSeg::MARRIAGE_END)
    return false;
  return true;
}

void
MultiTicketUtil::createSubItins(Itin& itin, TravelSeg* tvl, std::vector<Itin*>& subItins)
{
  std::vector<TravelSeg*> tvlSegs1;
  std::vector<TravelSeg*> tvlSegs2;
  if (_diag676)
    _diag676->displaySubItinStartBuild(itin);

  std::vector<TravelSeg*>::iterator it = itin.travelSeg().begin();
  std::vector<TravelSeg*>::iterator ite = itin.travelSeg().end();

  for (; it != ite; ++it)
  {
    if (*it == tvl)
    {
      tvlSegs1.push_back(*it);
      ++it;
      break;
    }
    tvlSegs1.push_back(*it);
    continue;
  }
  for (; it != ite; ++it)
  {
    tvlSegs2.push_back(*it);
  }
  if (_diag676)
    _diag676->displayProposedMultiItins(tvlSegs1, tvlSegs2);

  if (tvlSegs1.size() == 0 || tvlSegs2.size() == 0)
    return;

  if (_diag676)
    _diag676->displayCheckSubItinsMsg();

  // remove Arunks at the end of the 1st new itin
  if (tvlSegs1.size() > 1)
  {
    size_t tvlSegSize = tvlSegs1.size();
    while (tvlSegSize > 0)
    {
      TravelSeg* tvlSeg = tvlSegs1.back();
      if (tvlSeg->segmentType() == Arunk)
      {
        if (_diag676)
          _diag676->displayRemoveArunkAtFirstItins(tvlSegs1, tvlSeg);
        tvlSegs1.pop_back();
        tvlSegSize = tvlSegs1.size();
      }
      else
        break;
    }
  }
  // Make sure to turn off the following ind in the first new itin
  // last segment cound not be forced connection
  // last segment cound not be forced FareBreak
  // last segment cound not be forced NO FareBreak
  tvlSegs1.back()->forcedConx() = 'F';
  tvlSegs1.back()->forcedStopOver() = 'F';
  tvlSegs1.back()->forcedNoFareBrk() = 'F';
  tvlSegs1.back()->forcedNoFareBrk() = 'F';

  // remove Arunk at the beginning of the 2nd new itin
  if (tvlSegs2.size() > 1)
  {
    TravelSeg* tvlSeg = tvlSegs2.front();
    if (tvlSeg->segmentType() == Arunk)
    {
      if (_diag676)
        _diag676->displayRemoveArunkAtSecondItins(tvlSegs2, tvlSeg);
      tvlSegs2.erase(tvlSegs2.begin());
    }
  }

  // check new itins for the forced sidetrips
  if (tvlSegs1.back()->isForcedSideTrip() || tvlSegs2.front()->isForcedSideTrip())
  {
    if (_diag676)
      _diag676->displayForcedSideTrip(tvlSegs1.back(), tvlSegs2.front());
    return;
  }
  uint16_t num = 1;
  Itin* itin1 = createSubItin(itin, num, tvlSegs1);
  Itin* itin2 = createSubItin(itin, ++num, tvlSegs2);

  if (_diag676)
    _diag676->displaySubItinsCreated(*itin1, *itin2);
  if (itin1 && itin2)
  {
    subItins.push_back(itin1);
    trx().itin().push_back(itin1);
    subItins.push_back(itin2);
    trx().itin().push_back(itin2);
  }
}

void
MultiTicketUtil::createDiag676()
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic676 &&
      _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ITIN")
  {
    DCFactory* factory = DCFactory::instance();
    _diag676 = dynamic_cast<Diag676Collector*>(factory->create(_trx));
    if (!_diag676)
      return;

    _diag676->enable(Diagnostic676);
    _diag676->printHeader();
  }
}

void
MultiTicketUtil::finishDiag676()
{
  if (!_diag676)
    return;

  _diag676->printAllItinFooter();
  _diag676->flushMsg();
}

Itin*
MultiTicketUtil::createSubItin(Itin& itin, uint16_t num, const std::vector<TravelSeg*>& tSegs)
{
  if (tSegs.empty())
    return nullptr;

  if (!tSegs.front())
    return nullptr;

  Itin* newItinPtr = nullptr;
  trx().dataHandle().get(newItinPtr);

  if (!newItinPtr)
    return nullptr;

  newItinPtr->duplicate(itin, trx().dataHandle());
  newItinPtr->travelSeg() = tSegs;
  newItinPtr->setTravelDate(tSegs.front()->departureDT());
  newItinPtr->setMultiTktItinOrderNum(num);

  return newItinPtr;
}

void
MultiTicketUtil::validateTicketingAgreement(PricingTrx& trx)
{
  MultiTicketUtil mtu(trx);
  mtu.validateValidatingCxrData();
}

void
MultiTicketUtil::validateValidatingCxrData()
{
  Itin* fullItin = getMultiTicketItin(0); // Get full Itin
  Itin* subItin1 = getMultiTicketItin(1); // Get Sub-Itin 1
  Itin* subItin2 = getMultiTicketItin(2); // Get Sub-Itin 2
  TicketSolution ts = NO_SOLUTION_FOUND;

  if ( (subItin1 && !subItin1->validatingCxrGsaData()) ||
       (subItin2 && !subItin2->validatingCxrGsaData()) )
  {
    if (fullItin)
    {
      if (fullItin->validatingCxrGsaData())
        ts = MULTITKT_NOT_APPLICABLE;
      else
        ts = NO_SOLUTION_FOUND;

      fullItin->setTicketSolution(ts);
    }
    createDiag676ForTotal();
    if (_diag676)
    {
      _diag676->printSingleTicketHeader();
      displayValidatingCarriersDataMsg(fullItin);
      _diag676->printMultiTicketHeader();
      displayValidatingCarriersDataMsg(subItin1);
      displayValidatingCarriersDataMsg(subItin2);
      _diag676->displaySolution(ts);
    }
    finishDiag676ForTotal();
    cleanUpSubItins();
    if (ts == NO_SOLUTION_FOUND) // Fail GSA on all original and sub-itins
    {
       throw ErrorResponseException(ErrorResponseException::VALIDATING_CXR_ERROR, "NO VALID TICKETING AGREEMENTS FOUND");
    }
  }
}

void
MultiTicketUtil::displayValidatingCarriersDataMsg(Itin* itin)
{
  if (itin == nullptr)
  {
    *_diag676 << "ITIN IS NOT FOUND\n";
    return;
  }
  _diag676->displayItinTravelSeg(*itin);
  if (itin->validatingCxrGsaData())
    *_diag676 << "VALID TICKETING AGREEMENTS FOUND\n";
  else
    *_diag676 << "NO VALID TICKETING AGREEMENTS FOUND\n";
}

bool
MultiTicketUtil::isMultiTicketSolutionFound(PricingTrx& trx)
{
  MultiTicketUtil mtu(trx);
  return mtu.isSolutionFound();
}

bool
MultiTicketUtil::isSolutionFound()
{
  // fullItin was constructed with fullItin->setTicketSolution(NO_SOLUTION_FOUND);
  bool ret = true;
  Itin* fullItin = getMultiTicketItin(0); // Get full Itin
  Itin* subItin1 = getMultiTicketItin(1); // Get Sub-Itin 1
  Itin* subItin2 = getMultiTicketItin(2); // Get Sub-Itin 2
  if (fullItin == nullptr)
    ret = false;
  else if (trx().multiTicketMap().empty())
  {
    ret = false;
    if (!fullItin->farePath().empty())
      fullItin->setTicketSolution(MULTITKT_NOT_APPLICABLE);
  }
  else
  {
    if (subItin1 == nullptr || subItin1->farePath().empty() ||
        subItin2 == nullptr || subItin2->farePath().empty())
    {
      ret = false;
      if (!fullItin->farePath().empty())
        fullItin->setTicketSolution(MULTITKT_NOT_FOUND);
    }
  }

  if (!ret)
  {
    createDiag676ForTotal();
    if (_diag676)
    {
      _diag676->printSingleTicketHeader();
      displayPricingSolutionMsg(fullItin);
      _diag676->printMultiTicketHeader();
      if (_trx.multiTicketMap().empty())
      {
        *_diag676 << "NOT APPLICABLE - OW OR MULTI DESTINATION\n";
      }
      else
      {
        displayPricingSolutionMsg(subItin1);
        displayPricingSolutionMsg(subItin2);
      }
      if (fullItin)
        _diag676->displaySolution(fullItin->ticketSolution());
    }
    finishDiag676ForTotal();
    cleanUpSubItins();
  }
  return ret;
}

void
MultiTicketUtil::displayPricingSolutionMsg(Itin* itin)
{
  if (itin == nullptr)
  {
    *_diag676 << "ITIN IS NOT FOUND\n";
    return;
  }
  _diag676->displayItinTravelSeg(*itin);
  if (itin->farePath().empty())
    *_diag676 << "PRICING SOLUTION IS NOT FOUND\n";
  else
    *_diag676 << "PRICING SOLUTION IS FOUND\n";
}

void
MultiTicketUtil::processMultiTicketItins(PricingTrx& trx)
{
  MultiTicketUtil mtu(trx);
  mtu.createDiag676ForTotal();
  mtu.validateMultiTicketItins();
  mtu.finishDiag676ForTotal();
}

void
MultiTicketUtil::validateMultiTicketItins()
{
  if (trx().multiTicketMap().empty())
    return;
  Itin* fullItin = getMultiTicketItin(0); // Get full Itin
  if (fullItin == nullptr)
    return;

  _singleTicketTotal.setCode(INVALID_CURRENCYCODE);
  _multiTicketTotal.setCode(INVALID_CURRENCYCODE);

  if (_diag676)
    _diag676->printSingleTicketHeader();

  bool singleTicketFound = getFareTotal(fullItin, _singleTicketTotal);

  if (_diag676)
    _diag676->printMultiTicketHeader();

  Itin* subItin1 = getMultiTicketItin(1); // Get Sub-Itin 1
  if (subItin1 == nullptr)
    return;

  Money multiTicket1Total(0, INVALID_CURRENCYCODE);
  bool multiTicket1Found = getFareTotal(subItin1, multiTicket1Total);

  Itin* subItin2 = getMultiTicketItin(2); // Get Sub-Itin 2
  if (subItin2 == nullptr)
    return;

  Money multiTicket2Total(0, INVALID_CURRENCYCODE);
  bool multiTicket2Found = getFareTotal(subItin2, multiTicket2Total);

  if (!singleTicketFound && (!multiTicket1Found || !multiTicket2Found))
    fullItin->setTicketSolution(NO_SOLUTION_FOUND);
  else if (singleTicketFound && (!multiTicket1Found || !multiTicket2Found))
    fullItin->setTicketSolution(MULTITKT_NOT_FOUND);
  else
  {
    if (multiTicket1Total.code() != multiTicket2Total.code())
    {
      LOG4CXX_DEBUG(logger, "Multi tickets result in different currency code");
      cleanUpSubItins();
      return;
    }

    _multiTicketTotal = multiTicket1Total + multiTicket2Total;
    if (_diag676)
    {
      _diag676->displayMultiTicketTotalAmount(_multiTicketTotal);
    }

    if (!singleTicketFound && (multiTicket1Found && multiTicket2Found))
      fullItin->setTicketSolution(SINGLETKT_NOT_APPLICABLE);
    else
    {
      if (_singleTicketTotal.code() != multiTicket1Total.code())
      {
        LOG4CXX_DEBUG(logger, "Single ticket and multi tickets result in different currency code");
        cleanUpSubItins();
        return;
      }

      //Compare single ticket solution against multi ticket solution
      MoneyAmount diff = _singleTicketTotal.value() - _multiTicketTotal.value();
      if (diff > EPSILON)
        fullItin->setTicketSolution(MULTITKT_LOWER_THAN_SINGLETKT);
      else if (-diff > EPSILON)
        fullItin->setTicketSolution(SINGLETKT_LOWER_THAN_MULTITKT);
      else
        fullItin->setTicketSolution(SINGLETKT_SAME_AS_MULTITKT);
    }
  }

  if (_diag676)
  {
    _diag676->displayFinalAmount(_singleTicketTotal, _multiTicketTotal);
    _diag676->displaySolution(fullItin->ticketSolution());
  }

  if (fullItin->ticketSolution() == SINGLETKT_NOT_APPLICABLE ||
      fullItin->ticketSolution() == MULTITKT_LOWER_THAN_SINGLETKT)
    return;

  cleanUpSubItins();
}

bool
MultiTicketUtil::getFareTotal(Itin* itin, Money& fareTotalAmount)
{
  if (_diag676)
    _diag676->displayItinTravelSeg(*itin);

  if (itin->farePath().empty())
  {
    if (_diag676)
      *_diag676 << "PRICING SOLUTION IS NOT FOUND\n";
    return false;
  }

  FareCalcCollector* fareCalcCollector = FareCalcUtil::getFareCalcCollectorForItin(_trx, itin);
  if (!fareCalcCollector)
  {
    LOG4CXX_DEBUG(logger, "Cannot get fareCalcCollector for Itin");
    return false;
  }

  MoneyAmount totalAmount = 0;
  CurrencyCode equivCurr;
  for (FarePath* fp : itin->farePath())
  {
    CalcTotals* calcTotals = fareCalcCollector->findCalcTotals(fp);
    if (!calcTotals)
    {
      LOG4CXX_DEBUG(logger, "CalcTotals is not found");
      return false;
    }
    if (_diag676)
     _diag676->displayAmount(*fp, *calcTotals); // For each PTC
    totalAmount += ( (calcTotals->equivFareAmount * calcTotals->farePath->paxType()->number()) +
                     (calcTotals->taxAmount() * calcTotals->farePath->paxType()->number()) );
    equivCurr = calcTotals->equivCurrencyCode;
  }

  Money result(totalAmount, equivCurr);
  fareTotalAmount = result;
  if (_diag676)
    _diag676->displayTotalAmount(fareTotalAmount); // For all PTCs

  return true;
}

void
MultiTicketUtil::cleanUpSubItins()
{
  if (trx().getTrxType() != PricingTrx::MIP_TRX &&
      !trx().multiTicketMap().empty()           &&
      trx().itin().size() > 1)
  {
    trx().multiTicketMap().clear();
    trx().itin().erase(trx().itin().begin()+1, trx().itin().end());
  }
}

Itin*
MultiTicketUtil::getMultiTicketItin(const uint16_t& itinMultiTktOrderNum)
{
  std::vector<tse::Itin*>::iterator it = trx().itin().begin();
  std::vector<tse::Itin*>::iterator itE = trx().itin().end();

  for (; it != itE; ++it)
  {
    Itin* itin = (*it);
    if (itin && itin->getMultiTktItinOrderNum() == itinMultiTktOrderNum)
    {
      return itin;
    }
  }
  return nullptr;
}

void
MultiTicketUtil::createDiag676ForTotal()
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic676 &&
      _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL).empty())
  {
    DCFactory* factory = DCFactory::instance();
    _diag676 = dynamic_cast<Diag676Collector*>(factory->create(_trx));
    if (!_diag676)
      return;
    _diag676->enable(Diagnostic676);
    _diag676->printHeaderForTotal();
  }
}

void
MultiTicketUtil::finishDiag676ForTotal()
{
  if (!_diag676)
    return;
  _diag676->printFooterForTotal();
  _diag676->flushMsg();
}

MultiTicketUtil::TicketSolution
MultiTicketUtil::getTicketSolution(PricingTrx& trx)
{
  std::vector<tse::Itin*>::iterator it = trx.itin().begin();
  std::vector<tse::Itin*>::iterator itE = trx.itin().end();

  for (; it != itE; ++it)
  {
    Itin* itin = (*it);
    if (itin && itin->getMultiTktItinOrderNum() == 0)
    {
      return itin->ticketSolution();
    }
  }

  return NO_SOLUTION_FOUND;
}

Itin*
MultiTicketUtil::getMultiTicketItin(PricingTrx& trx, const uint16_t& itinMultiTktOrderNum)
{
  std::vector<tse::Itin*>::iterator it = trx.itin().begin();
  std::vector<tse::Itin*>::iterator itE = trx.itin().end();

  for (; it != itE; ++it)
  {
    Itin* itin = (*it);
    if (itin && itin->getMultiTktItinOrderNum() == itinMultiTktOrderNum)
    {
      return itin;
    }
  }
  return nullptr;
}

std::vector<std::string>&
MultiTicketUtil::getAllRespondMsgs()
{
  return _respondMsg;
}

void
MultiTicketUtil::setRespondMsg(uint16_t position, const std::string& msg)
{
  if (position >= 3)
    return;

  _respondMsg[position].clear();
  _respondMsg[position] = msg;
}

void
MultiTicketUtil::cleanUpMultiTickets(PricingTrx& trx)
{
  _respondMsg.clear();
  _respondMsg.resize(3);
  _singleTicketTotal.value() = 0;
  _multiTicketTotal.value() = 0;

  std::vector<tse::Itin*>::iterator it = trx.itin().begin();
  std::vector<tse::Itin*>::iterator itE = trx.itin().end();

  for (; it != itE; ++it)
  {
    Itin* itin = (*it);
    if (itin && (itin->getMultiTktItinOrderNum() == 1 || itin->getMultiTktItinOrderNum() == 2))
      itin->farePath().clear();
  }
}
}
