//-------------------------------------------------------------------
//
//  File:        ServiceFeesGroup.cpp
//
//  Description:
//
//  Copyright Sabre 2009
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#include "ServiceFees/ServiceFeesGroup.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/CurrencyConverter.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/ServiceFeeUtil.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/SubCodeInfo.h"
#include "Diagnostic/Diag880Collector.h"
#include "ServiceFees/MerchCarrierStrategy.h"
#include "ServiceFees/OCFees.h"

#include <utility>

#include <boost/bind.hpp>

namespace tse
{
FALLBACK_DECL(reworkTrxAborter);
FALLBACK_DECL(ocFeesAmountRoundingRefactoring);
namespace
{
ConfigurableValue<uint32_t>
maxPssOutput("OUTPUT_LIMITS", "MAX_PSS_OUTPUT", 1024000);
}
}

namespace
{
class IsSameTravel : public std::unary_function<const tse::OCFees*, bool>
{
public:
  IsSameTravel(
      std::vector<std::pair<const tse::TravelSeg*, const tse::TravelSeg*> >& winningSolution)
    : _winningSolution(winningSolution)
  {
  }
  bool operator()(const tse::OCFees* ocfee) const
  {
    return std::find(_winningSolution.begin(),
                     _winningSolution.end(),
                     std::pair<const tse::TravelSeg*, const tse::TravelSeg*>(
                         ocfee->travelStart(), ocfee->travelEnd())) != _winningSolution.end();
  }

private:
  std::vector<std::pair<const tse::TravelSeg*, const tse::TravelSeg*> >& _winningSolution;
};

class IsSameSubTypeCode
{
public:
  IsSameSubTypeCode(tse::ServiceSubTypeCode serviceSubTypeCode)
    : _serviceSubTypeCode(serviceSubTypeCode)
  {
  }

  bool operator()(const tse::OCFees* ocfee) const
  {
    return ocfee->subCodeInfo()->serviceSubTypeCode() == _serviceSubTypeCode;
  }

private:
  tse::ServiceSubTypeCode _serviceSubTypeCode;
};

class IsSameServiceType
{
public:
  IsSameServiceType(const tse::Indicator& serviceType) : _serviceType(serviceType) {}
  bool operator()(const tse::OCFees* ocfee) const
  {
    return ocfee->subCodeInfo()->fltTktMerchInd() == _serviceType;
  }

private:
  tse::Indicator _serviceType;
};

class IsSameUnit
{
public:
  IsSameUnit(tse::TravelSeg* travelStart, tse::TravelSeg* travelEnd)
    : _travelStart(travelStart), _travelEnd(travelEnd)
  {
  }

  bool operator()(const tse::OCFees* ocfee) const
  {
    return ocfee->travelStart()->segmentOrder() >= _travelStart->segmentOrder() &&
           ocfee->travelEnd()->segmentOrder() <= _travelEnd->segmentOrder();
  }

private:
  tse::TravelSeg* _travelStart;
  tse::TravelSeg* _travelEnd;
};

class IsSameUnitShopping
{
public:
  IsSameUnitShopping(const tse::Itin& itin, tse::TravelSeg* travelStart, tse::TravelSeg* travelEnd)
    : _travelStartId(itin.segmentOrder(travelStart)),
      _travelEndId(itin.segmentOrder(travelEnd)),
      _itin(itin)
  {
  }

  bool operator()(const tse::OCFees* ocfee) const
  {
    return _itin.segmentOrder(ocfee->travelStart()) >= _travelStartId &&
           _itin.segmentOrder(ocfee->travelEnd()) <= _travelEndId;
  }

private:
  int _travelStartId;
  int _travelEndId;
  const tse::Itin& _itin;
};

class RemoveSoftMatches
{
public:
  void operator()(tse::OCFees* ocFee) const
  {
    std::vector<tse::OCFees::OCFeesSeg*>::iterator ocFeesSegIt = ocFee->segments().begin();
    for (; ocFeesSegIt != ocFee->segments().end();)
    {
      if ((*ocFeesSegIt)->_optFee && (*ocFeesSegIt)->_softMatchS7Status.value() != 0)
        ocFee->segments().erase(ocFeesSegIt);
      else
        ++ocFeesSegIt;
    }
    ocFee->pointToFirstOCFee();
  }
};

struct IsAirSeg
{
  bool operator()(const tse::TravelSeg* ts) const { return ts->isAir(); }
};
}

namespace tse
{
static Logger
logger("atseintl.ServiceFees.ServiceFeesGroup");

ServiceFeesGroup::ServiceFeesGroup()
  : _maxDiagSize(maxPssOutput.getValue()) {}

ServiceFeesGroup::~ServiceFeesGroup() {}

ServiceFeesGroup::SubCodeInitializer::SubCodeInitializer(PricingTrx& trx,
                                                         FarePath* farePath,
                                                         TravelSeg* first,
                                                         TravelSeg* last,
                                                         MerchCarrierStrategy& merchStrategy)
  : _trx(trx), _farePath(farePath), _first(first), _end(last), _merchStrategy(merchStrategy)
{
}

ServiceFeesGroup::SubCodeInitializer::~SubCodeInitializer() {}

void
ServiceFeesGroup::SubCodeInitializer::getSubCode(std::vector<SubCodeInfo*>& subCodes,
                                                 const CarrierCode& carrier,
                                                 const ServiceTypeCode& srvTypeCode,
                                                 const ServiceGroup& groupCode,
                                                 const DateTime& travelDate) const
{
  _merchStrategy.getSubCode(
      _trx.dataHandle(), subCodes, carrier, srvTypeCode, groupCode, travelDate);
}

void
ServiceFeesGroup::SubCodeInitializer::
operator()(ServiceFeesGroup* srvFeesGroup,
           const CarrierCode& carrier,
           const ServiceTypeCode& srvTypeCode,
           const ServiceGroup& srvGroup,
           const DateTime& travelDate) const
{
  const boost::lock_guard<boost::mutex> guard(srvFeesGroup->mutex());
  std::vector<SubCodeInfo*> subCodeInfo;
  getSubCode(subCodeInfo, carrier, srvTypeCode, srvGroup, travelDate);

  for_each(subCodeInfo.begin(),
           subCodeInfo.end(),
           boost::bind<void>(&SubCodeInitializer::addOCFees, this, _1, srvFeesGroup, carrier));
}

OCFees*
ServiceFeesGroup::SubCodeInitializer::newOCFee() const
{
  OCFees* ocFee;
  _trx.dataHandle().get(ocFee);
  return ocFee;
}

void
ServiceFeesGroup::SubCodeInitializer::addOCFees(const SubCodeInfo* subCodeInfo,
                                                ServiceFeesGroup* srvFeesGroup,
                                                const CarrierCode& carrier) const
{
  OCFees* ocFee = newOCFee();
  if (!ocFee)
    return;
  srvFeesGroup->ocFeesMap()[_farePath].push_back(ocFee);

  ocFee->subCodeInfo() = subCodeInfo;
  ocFee->carrierCode() = carrier;
  ocFee->travelStart() = _first;
  ocFee->travelEnd() = _end;
}

//  void
//  ServiceFeesGroup::SubCodeInitializer::copyUnique(std::vector<SubCodeInfo*>* subCodes,
// SubCodeInfo* newElement)
//  {
//    std::vector<SubCodeInfo*>::iterator i = subCodes->begin();
//    std::vector<SubCodeInfo*>::iterator ie = subCodes->end();
//
//    for (; i != ie; i++)
//    {
//      if ((*i)->serviceSubTypeCode() == newElement->serviceSubTypeCode() &&
//          (*i)->fltTktMerchInd()     == newElement->fltTktMerchInd())
//      {
//        return;
//      }
//    }
//
//    subCodes->push_back(newElement);
//  }

bool
ServiceFeesGroup::isSubCodePassed(int unitNo,
                                  FarePath* farePath,
                                  TravelSeg* firstTvl,
                                  TravelSeg* lastTvl,
                                  const ServiceSubTypeCode& subCode,
                                  const Indicator& srvType) const
{
  ServiceFeesGroup::SubCodeMap::const_iterator subCodeI =
      _subCodeMap[unitNo].find(SubCodeMapKey(farePath, subCode, srvType));
  if (subCodeI != _subCodeMap[unitNo].end())
    return subCodeI->second.find(std::make_pair(firstTvl, lastTvl)) != subCodeI->second.end();
  return false;
}

bool
ServiceFeesGroup::foundSolution(uint16_t& skippedSegs,
                                const uint16_t currSkippedSegs,
                                const MoneyAmount minFeeAmountSum) const
{
  bool res = false;
  if (!(minFeeAmountSum - EPSILON < -1.0 && minFeeAmountSum + EPSILON > -1.0) &&
      currSkippedSegs > skippedSegs)
    res = true;
  skippedSegs = currSkippedSegs;
  return res;
}

bool
ServiceFeesGroup::getAmountSum(Money& sum, const std::vector<TvlSegPair*>& routes, KeyItemMap& item)
    const
{
  std::vector<OCFees*> fees;

  typedef std::vector<TvlSegPair*>::const_iterator RouteIt;
  for (RouteIt routeI = routes.begin(); routeI != routes.end(); ++routeI)
  {
    KeyItemMap::const_iterator keyItemMap =
        item.find(std::make_pair(*(*routeI)->first, *((*routeI)->second - 1)));
    if (keyItemMap == item.end())
      return false;

    fees.push_back(keyItemMap->second.first);
  }

  return getConvertedAmountSum(fees, sum);
}

bool
ServiceFeesGroup::getConvertedAmountSum_old(std::vector<OCFees*>& fees, Money& sum) const
{
  RoundingFactor roundingFactor = 0;
  CurrencyNoDec roundingNoDec = 0;
  RoundingRule roundingRule = NONE;
  CurrencyConverter curConverter;

  bool isRoundingRetrieved =
      getFeeRounding_old(sum.code(), roundingFactor, roundingNoDec, roundingRule);

  sum.value() = 0.0;

  for (std::vector<OCFees*>::iterator it = fees.begin(); it != fees.end(); it++)
  {
    Money targetMoney((*it)->feeAmount(), sum.code());
    (*it)->displayCurrency() = sum.code();

    if (sum.code() != (*it)->feeCurrency())
    {
      convertCurrency(targetMoney, Money((*it)->feeAmount(), (*it)->feeCurrency()));
      if (isRoundingRetrieved)
        curConverter.round(targetMoney, roundingFactor, roundingRule);

      (*it)->displayAmount() = targetMoney.value();
    }
    else
    {
      (*it)->displayAmount() = (*it)->feeAmount();
    }

    sum.value() += targetMoney.value();
  }

  return true;
}

bool
ServiceFeesGroup::getConvertedAmountSum(std::vector<OCFees*>& fees, Money& sum) const
{
  if (fallback::ocFeesAmountRoundingRefactoring(_trx))
   return getConvertedAmountSum_old(fees, sum);

  sum.value() = 0.0;

  for (std::vector<OCFees*>::iterator it = fees.begin(); it != fees.end(); it++)
  {
    Money targetMoney((*it)->feeAmount(), sum.code());
    (*it)->displayCurrency() = sum.code();

    if (sum.code() != (*it)->feeCurrency())
    {
      OCFees::OcAmountRounder ocAmountRounder(*_trx);

      convertCurrency(targetMoney, Money((*it)->feeAmount(), (*it)->feeCurrency()));
      targetMoney.value() = ocAmountRounder.getRoundedFeeAmount(targetMoney);
    }

    (*it)->displayAmount() = targetMoney.value();
    sum.value() += targetMoney.value();
  }

  return true;
}

void
ServiceFeesGroup::getMktCxr(std::multiset<CarrierCode>& marketingCxrs,
                            const std::vector<TvlSegPair*>& routes,
                            const KeyItemMap& item) const
{
  typedef std::vector<TvlSegPair*>::const_iterator RouteIt;
  for (RouteIt routeI = routes.begin(); routeI != routes.end(); ++routeI)
  {
    KeyItemMap::const_iterator keyItemMap =
        item.find(std::make_pair(*(*routeI)->first, *((*routeI)->second - 1)));
    if (keyItemMap == item.end())
      return;

    typedef std::vector<TravelSeg*>::const_iterator TSIt;
    for (TSIt tsIt = (*routeI)->first; tsIt != (*routeI)->second; ++tsIt)
    {
      const AirSeg* seg = dynamic_cast<const AirSeg*>(*tsIt);
      if (!seg)
        continue;
      marketingCxrs.insert((keyItemMap->second).second ? seg->marketingCarrierCode()
                                                       : seg->operatingCarrierCode());
    }
  }
}

void
ServiceFeesGroup::countCarrierOccurrences(
    std::multiset<uint16_t, std::greater<uint16_t> >& cxrCounter,
    const std::multiset<CarrierCode>& marketingCxrs)
{
  if (!marketingCxrs.empty())
  {
    std::pair<std::multiset<CarrierCode>::const_iterator,
              std::multiset<CarrierCode>::const_iterator> pos;
    pos.second = marketingCxrs.begin();
    do
    {
      pos = std::equal_range(pos.second, marketingCxrs.end(), *pos.second);
      cxrCounter.insert(std::distance(pos.first, pos.second));
    } while (pos.second != marketingCxrs.end());
  }
}

void
ServiceFeesGroup::chooseSolution(
    MoneyAmount& minFeeAmountSum,
    const std::vector<TvlSegPair*>*& winningSolution,
    std::multiset<uint16_t, std::greater<uint16_t> >& winningCxrCounter,
    std::multiset<uint16_t, std::greater<uint16_t> >& cxrCounter,
    const MoneyAmount feeAmountSum,
    const std::vector<TvlSegPair*>& routes) const
{
  if ((minFeeAmountSum - EPSILON < -1.0 && minFeeAmountSum + EPSILON > -1.0) ||
      (feeAmountSum + EPSILON) < minFeeAmountSum ||
      ((feeAmountSum - EPSILON) < minFeeAmountSum && (feeAmountSum + EPSILON) > minFeeAmountSum &&
       winningCxrCounter < cxrCounter))
  {
    minFeeAmountSum = feeAmountSum;
    winningSolution = &routes;
    std::swap(winningCxrCounter, cxrCounter);
  }
}

void
ServiceFeesGroup::printSolutions(Diag880Collector* diag880,
                                 uint16_t& solNo,
                                 const std::vector<TvlSegPair*>& routes,
                                 const KeyItemMap& item,
                                 const std::vector<TvlSegPair*>* winningSolution)
{
  diag880->printSolution(routes, solNo++);
  typedef std::vector<TvlSegPair*>::const_iterator RouteIt;
  for (RouteIt routeI = routes.begin(); routeI != routes.end(); ++routeI)
  {
    KeyItemMap::const_iterator keyItemMap =
        item.find(std::make_pair(*(*routeI)->first, *((*routeI)->second - 1)));
    if (keyItemMap == item.end())
    {
      diag880->printPieceNotFound(**routeI);
    }
    else
    {
      OCFees::OCFeesSeg& ocFeesSeg = *(keyItemMap->second.first->segments().back());

      if (!ocFeesSeg._optFee)
        diag880->printPieceNotFound(**routeI);
      else if (ocFeesSeg._softMatchS7Status.value() == 0)
        diag880->printHardMatchFound(**routeI);
      else
        diag880->printSoftMatchFound(**routeI);
    }
  }

  diag880->printResult(&routes == winningSolution);
}

void
ServiceFeesGroup::printSolutions(Diag880Collector* diag880,
                                 const DateTime& tktDate,
                                 uint16_t& solNo,
                                 const std::vector<TvlSegPair*>& routes,
                                 const KeyItemMap& item,
                                 const std::vector<TvlSegPair*>* winningSolution)
{
  bool isValidSol = true;
  CurrencyCode feeCurrency;
  MoneyAmount feeAmountSum = 0.0;
  diag880->printSolution(routes, solNo++);
  typedef std::vector<TvlSegPair*>::const_iterator RouteIt;
  for (RouteIt routeI = routes.begin(); routeI != routes.end(); ++routeI)
  {
    KeyItemMap::const_iterator keyItemMap =
        item.find(std::make_pair(*(*routeI)->first, *((*routeI)->second - 1)));
    if (keyItemMap == item.end())
    {
      isValidSol = false;
      diag880->printPieceNotFound(**routeI);
    }
    else
    {
      diag880->printPiece(**routeI,
                          Money((keyItemMap->second).first->displayAmount(),
                                (keyItemMap->second).first->displayCurrency()),
                          (keyItemMap->second).first->carrierCode(),
                          (keyItemMap->second).second,
                          tktDate);
      feeCurrency = (keyItemMap->second).first->displayCurrency();
      feeAmountSum += (keyItemMap->second).first->displayAmount();
    }
  }
  if (isValidSol)
    diag880->printResult(Money(feeAmountSum, feeCurrency), winningSolution == &routes, tktDate);
  else
    diag880->printResultFail(Money(feeAmountSum, feeCurrency), tktDate);
}

bool
ServiceFeesGroup::shouldProcessSliceAndDice(
    std::pair<const SubCodeMapKey, KeyItemMap>& subCodeItem,
    const std::vector<TravelSeg*>::const_iterator& firstSegI,
    const std::vector<TravelSeg*>::const_iterator& endSegI)
{
  const boost::lock_guard<boost::mutex> guard(_mutex);
  if (subCodeItem.second.size() == 1 && subCodeItem.second.begin()->first.first == *firstSegI &&
      subCodeItem.second.begin()->first.second == *(endSegI - 1))
    return false;

  if (!_trx)
    return false;

  _sliceDicePassed = false; // turn off before start slice & dice

  OCFees ocf = *(subCodeItem.second.begin()->second.first);
  LOG4CXX_DEBUG(logger,
                "Entered ServiceFeesGroup::findSolutionForSubCode(): for "
                    << ocf.subCodeInfo()->serviceGroup()
                    << " and SubTypeCode = " << std::get<1>(subCodeItem.first));
  bool mustHurry = false;
  if (fallback::reworkTrxAborter(_trx))
    mustHurry = checkTrxMustHurry(*_trx);
  else
    mustHurry = _trx->checkTrxMustHurry();

  if (mustHurry)
  {
    LOG4CXX_DEBUG(logger, "Leaving ServiceFeesGroup::findSolutionForSubCode(): time out");
    return false;
  }
  return true;
}

const std::vector<ServiceFeesGroup::TvlSegPair*>*
ServiceFeesGroup::findSolutionForSubCode(std::pair<const SubCodeMapKey, KeyItemMap>& subCodeItem,
                                         TseUtil::SolutionSet& solutions,
                                         std::vector<TravelSeg*>::const_iterator firstSegI,
                                         std::vector<TravelSeg*>::const_iterator endSegI,
                                         Diag880Collector* diag880,
                                         const DateTime& tktDate,
                                         uint16_t multiTicketNbr)
{
  if (!shouldProcessSliceAndDice(subCodeItem, firstSegI, endSegI))
    return nullptr;

  MoneyAmount minFeeAmountSum = -1.0;
  const std::vector<TvlSegPair*>* winningSolution = nullptr;
  std::multiset<uint16_t, std::greater<uint16_t> > winningCxrCounter;

  TseUtil::SolutionSet::nth_index<0>::type& index = solutions.get<0>();
  uint16_t skippedSegs = index.begin()->_skipped;
  bool foundSol = false;

  CurrencyCode outputCurrency = determineOutputCurrency(index, subCodeItem.second);

  typedef TseUtil::SolutionSet::nth_index<0>::type::iterator SolutionIt;
  for (SolutionIt solIt = index.begin(); solIt != index.end(); ++solIt)
  {
    if (foundSolution(skippedSegs, solIt->_skipped, minFeeAmountSum))
    {
      foundSol = true;
      break;
    }
    std::multiset<CarrierCode> marketingCxrs;
    Money feeAmountSum(outputCurrency);
    if (!getAmountSum(feeAmountSum, solIt->_routes, subCodeItem.second))
      continue;
    getMktCxr(marketingCxrs, solIt->_routes, subCodeItem.second);

    std::multiset<uint16_t, std::greater<uint16_t> > cxrCounter;
    countCarrierOccurrences(cxrCounter, marketingCxrs);

    chooseSolution(minFeeAmountSum,
                   winningSolution,
                   winningCxrCounter,
                   cxrCounter,
                   feeAmountSum.value(),
                   solIt->_routes);
  }
  if (!foundSol)
    skippedSegs = index.rbegin()->_skipped + 1;
  if (diag880 && diag880->str().size() < _maxDiagSize)
  {
    uint16_t solNo = 1;
    diag880->printHeader(_groupCode,
                         std::get<1>(subCodeItem.first),
                         std::get<0>(subCodeItem.first)->paxType()->paxType(),
                         firstSegI,
                         endSegI,
                         multiTicketNbr);
    diag880->printSliceAndDiceMatrix(solutions, _maxDiagSize);
    for (SolutionIt solIt = index.begin(); solIt != index.end() && solIt->_skipped < skippedSegs &&
                                               diag880->str().size() < _maxDiagSize;
         ++solIt)
      printSolutions(diag880, tktDate, solNo, solIt->_routes, subCodeItem.second, winningSolution);
    diag880->flushMsg();
  }
  _sliceDicePassed = true; // turn on after completion slice & dice
  return winningSolution;
}

const std::vector<ServiceFeesGroup::TvlSegPair*>*
ServiceFeesGroup::findSolutionForSubCodeForAncillaryPricing(
    std::pair<const SubCodeMapKey, KeyItemMap>& subCodeItem,
    TseUtil::SolutionSet& solutions,
    std::vector<TravelSeg*>::const_iterator firstSegI,
    std::vector<TravelSeg*>::const_iterator endSegI,
    Diag880Collector* diag880,
    const DateTime& tktDate,
    uint16_t multiTicketNbr)
{
  if (!shouldProcessSliceAndDice(subCodeItem, firstSegI, endSegI))
    return nullptr;

  TseUtil::SolutionSet::nth_index<0>::type& index = solutions.get<0>();
  const std::vector<TvlSegPair*>* winningSolution = &index.rbegin()->_routes;

  typedef TseUtil::SolutionSet::nth_index<0>::type::iterator SolutionIt;
  for (SolutionIt solIt = index.begin(); solIt != index.end(); ++solIt)
  {
    bool isHardMatch = true;

    typedef std::vector<TvlSegPair*>::const_iterator RouteIt;
    for (RouteIt routeI = solIt->_routes.begin(); routeI != solIt->_routes.end(); ++routeI)
    {
      KeyItemMap::const_iterator keyItemMap =
          subCodeItem.second.find(std::make_pair(*(*routeI)->first, *((*routeI)->second - 1)));
      if (keyItemMap == subCodeItem.second.end() ||
          !(keyItemMap->second.first->segments().back()->_optFee &&
            keyItemMap->second.first->segments().back()->_softMatchS7Status.value() == 0))
      {
        isHardMatch = false;
        break;
      }
    }

    if (isHardMatch)
    {
      winningSolution = &solIt->_routes;
      break;
    }
  }

  if (diag880 && diag880->str().size() < _maxDiagSize)
  {
    diag880->printHeader(_groupCode,
                         std::get<1>(subCodeItem.first),
                         std::get<0>(subCodeItem.first)->paxType()->paxType(),
                         firstSegI,
                         endSegI,
                         multiTicketNbr);
    diag880->printSliceAndDiceMatrix(solutions, _maxDiagSize);
    SolutionIt solIt = index.begin();
    uint16_t solNo = 1;
    bool isHardMatch;
    do
    {
      isHardMatch = &solIt->_routes == winningSolution;
      printSolutions(diag880, solNo, solIt->_routes, subCodeItem.second, winningSolution);
      ++solIt;
    } while (solIt != index.end() && !isHardMatch && diag880->str().size() < _maxDiagSize);
    diag880->flushMsg();
  }

  _sliceDicePassed = true;
  return winningSolution;
}

void
ServiceFeesGroup::removeSoftMatches(
    std::vector<std::pair<const TravelSeg*, const TravelSeg*> >& winningSolutionSeg,
    FarePath* farePath,
    const ServiceSubTypeCode srvSubTypeCode,
    const Indicator& serviceType,
    TravelSeg* travelStart,
    TravelSeg* travelEnd)
{
  std::vector<OCFees*> winningOCFees;

  if (_trx->getTrxType() == PricingTrx::PRICING_TRX)
  {
    std::remove_copy_if(_ocFeesMap[farePath].begin(),
                        _ocFeesMap[farePath].end(),
                        std::back_inserter(winningOCFees),
                        !boost::bind<bool>(IsSameSubTypeCode(srvSubTypeCode), _1) ||
                            !boost::bind<bool>(IsSameServiceType(serviceType), _1) ||
                            !boost::bind<bool>(IsSameUnit(travelStart, travelEnd), _1) ||
                            !boost::bind<bool>(IsSameTravel(winningSolutionSeg), _1));
  }
  else
  {
    const Itin* itin = farePath->itin();
    assert(itin && "Itin is null");

    std::remove_copy_if(
        _ocFeesMap[farePath].begin(),
        _ocFeesMap[farePath].end(),
        std::back_inserter(winningOCFees),
        !boost::bind<bool>(IsSameSubTypeCode(srvSubTypeCode), _1) ||
            !boost::bind<bool>(IsSameServiceType(serviceType), _1) ||
            !boost::bind<bool>(IsSameUnitShopping(*itin, travelStart, travelEnd), _1) ||
            !boost::bind<bool>(IsSameTravel(winningSolutionSeg), _1));
  }

  std::for_each(winningOCFees.begin(), winningOCFees.end(), RemoveSoftMatches());
}

void
ServiceFeesGroup::removeInvalidOCFees(
    const std::vector<ServiceFeesGroup::TvlSegPair*>* winningSolution,
    FarePath* farePath,
    const ServiceSubTypeCode srvSubTypeCode,
    const Indicator& serviceType,
    TravelSeg* travelStart,
    TravelSeg* travelEnd,
    bool keepOnlyHardMatch)
{
  if (!winningSolution)
    return;

  std::vector<std::pair<const TravelSeg*, const TravelSeg*> > winningSolutionsSegPairs;
  std::vector<ServiceFeesGroup::TvlSegPair*>::const_iterator winSolIter = winningSolution->begin();
  for (; winSolIter != winningSolution->end(); ++winSolIter)
    winningSolutionsSegPairs.push_back(std::pair<const TravelSeg*, const TravelSeg*>(
        *(*winSolIter)->first, *((*winSolIter)->second - 1)));

  const std::vector<OCFees*>& ocFees = _ocFeesMap[farePath];

  std::vector<OCFees*> filteredOCFees;

  if (_trx->getTrxType() == PricingTrx::PRICING_TRX)
  {
    std::remove_copy_if(ocFees.begin(),
                        ocFees.end(),
                        std::back_inserter(filteredOCFees),
                        boost::bind<bool>(IsSameSubTypeCode(srvSubTypeCode), _1) &&
                            boost::bind<bool>(IsSameServiceType(serviceType), _1) &&
                            boost::bind<bool>(IsSameUnit(travelStart, travelEnd), _1) &&
                            !boost::bind<bool>(IsSameTravel(winningSolutionsSegPairs), _1));
  }
  else
  {
    const Itin* itin = farePath->itin();
    assert(itin && "Itin is null");

    std::remove_copy_if(
        ocFees.begin(),
        ocFees.end(),
        std::back_inserter(filteredOCFees),
        boost::bind<bool>(IsSameSubTypeCode(srvSubTypeCode), _1) &&
            boost::bind<bool>(IsSameServiceType(serviceType), _1) &&
            boost::bind<bool>(IsSameUnitShopping(*itin, travelStart, travelEnd), _1) &&
            !boost::bind<bool>(IsSameTravel(winningSolutionsSegPairs), _1));
  }

  _ocFeesMap[farePath].swap(filteredOCFees);

  if (keepOnlyHardMatch)
    removeSoftMatches(
        winningSolutionsSegPairs, farePath, srvSubTypeCode, serviceType, travelStart, travelEnd);
}

void
ServiceFeesGroup::findSolution(int unitNo,
                               TseUtil::SolutionSet& solutions,
                               std::vector<TravelSeg*>::const_iterator firstSegI,
                               std::vector<TravelSeg*>::const_iterator endSegI,
                               Diag880Collector* diag880,
                               const DateTime& tktDate,
                               FindSolution findSolutionForSub,
                               uint16_t multiTicketNbr)
{
  for (SubCodeMap::iterator scMI = _subCodeMap[unitNo].begin(); scMI != _subCodeMap[unitNo].end();
       ++scMI)
  {
    const std::vector<ServiceFeesGroup::TvlSegPair*>* winningSolution =
        (this->*findSolutionForSub)(*scMI, solutions, firstSegI, endSegI, diag880, tktDate, multiTicketNbr);

    removeInvalidOCFees(winningSolution,
                        std::get<0>(scMI->first),
                        std::get<1>(scMI->first),
                        std::get<2>(scMI->first),
                        *firstSegI,
                        *(endSegI - 1),
                        findSolutionForSub == getFindSolutionForSubCodeForAncillaryPricing() &&
                            &solutions.get<0>().rbegin()->_routes != winningSolution);
  }
}

CurrencyCode
ServiceFeesGroup::determineOutputCurrency(const TseUtil::SolutionSet::nth_index<0>::type& index,
                                          const KeyItemMap& item) const
{
  CurrencyCode ret = "";
  CurrencyCode pointOfSaleCur = ServiceFeeUtil::getSellingCurrency(*_trx);

  typedef TseUtil::SolutionSet::nth_index<0>::type::iterator SolutionIt;
  for (SolutionIt solIt = index.begin(); solIt != index.end(); ++solIt)
  {
    typedef std::vector<TvlSegPair*>::const_iterator RouteIt;
    for (RouteIt routeI = solIt->_routes.begin(); routeI != solIt->_routes.end(); ++routeI)
    {
      KeyItemMap::const_iterator keyItemMap =
          item.find(std::make_pair(*(*routeI)->first, *((*routeI)->second - 1)));
      if (keyItemMap == item.end())
        continue;

      if (ret.empty())
        ret = keyItemMap->second.first->feeCurrency();
      else if (ret != keyItemMap->second.first->feeCurrency())
      {
        return pointOfSaleCur;
      }
    }
  }

  return ret.empty() ? pointOfSaleCur : ret;
}

bool
ServiceFeesGroup::getFeeRounding_old(const CurrencyCode& currencyCode,
                                 RoundingFactor& roundingFactor,
                                 CurrencyNoDec& roundingNoDec,
                                 RoundingRule& roundingRule) const
{
  ServiceFeeUtil util(*_trx);
  return util.getFeeRounding_old(currencyCode, roundingFactor, roundingNoDec, roundingRule);
}

void
ServiceFeesGroup::convertCurrency(Money& target, const Money& source) const
{
  ServiceFeeUtil util(*_trx);
  util.convertCurrency(target, source);
}

bool
ServiceFeesGroup::findTravelWhenNoArkunksOnBegOrEnd(
    std::pair<ServiceFeesGroup::TSIt, ServiceFeesGroup::TSIt>& noArunkTvl,
    const TSIt& begIt,
    const TSIt& endIt) const
{
  typedef std::vector<tse::TravelSeg*>::const_reverse_iterator RTSIt;
  TSIt begIndIt = std::find_if(begIt, endIt, IsAirSeg());
  if (begIndIt != endIt)
  {
    TSIt endIndIt = std::find_if(RTSIt(endIt), RTSIt(begIndIt), IsAirSeg()).base();
    if (begIndIt < endIndIt)
    {
      noArunkTvl.first = begIndIt;
      noArunkTvl.second = endIndIt;
      return true;
    }
  }
  return false;
}

void
ServiceFeesGroup::collectUnitsOfTravel(std::vector<std::tuple<TSIt, TSIt, bool> >& unitsOfTravel,
                                       TSIt begIt,
                                       const TSIt endIt)
{
  bool isPadisAllowed = true;

  if (_trx->billing()->requestPath() == ACS_PO_ATSE_PATH)
  {
    isPadisAllowed = false;
  }

  if (_merchCxrPref.empty())
  {
    if (_groupCode.equalToConst("SA") && isPadisAllowed)
    {
      collectUnitsOfTravelForSA(unitsOfTravel, begIt, endIt);
    }
    return;
  }
  TSIt prevBegIt = begIt;
  static bool MARKET_DRIVEN = true;
  static bool MULTIPLE_SEGMENT = false;

  for (; begIt != endIt; ++begIt)
  {
    if (!(*begIt)->isAir())
      continue;

    AirSeg& seg = static_cast<AirSeg&>(**begIt);
    if (seg.marketingCarrierCode() == seg.operatingCarrierCode() &&
        _merchCxrPref.find(seg.marketingCarrierCode()) != _merchCxrPref.end())
    {
      if (prevBegIt != begIt)
      {
        if (_groupCode.equalToConst("SA") && isPadisAllowed)
        {
          collectUnitsOfTravelForSA(unitsOfTravel, prevBegIt, begIt);
        }
        else
        {
          std::pair<TSIt, TSIt> noArunkTvl;
          if (findTravelWhenNoArkunksOnBegOrEnd(noArunkTvl, prevBegIt, begIt))
            unitsOfTravel.push_back(std::tuple<TSIt, TSIt, bool>(
                noArunkTvl.first, noArunkTvl.second, MULTIPLE_SEGMENT));
        }
      }
      prevBegIt = begIt + 1;
      unitsOfTravel.push_back(std::tuple<TSIt, TSIt, bool>(begIt, prevBegIt, MARKET_DRIVEN));
    }
  }
  if (prevBegIt != begIt && !unitsOfTravel.empty())
  {
    if (_groupCode.equalToConst("SA") && isPadisAllowed)
    {
      collectUnitsOfTravelForSA(unitsOfTravel, prevBegIt, begIt);
    }
    else
    {
      std::pair<TSIt, TSIt> noArunkTvl;
      if (findTravelWhenNoArkunksOnBegOrEnd(noArunkTvl, prevBegIt, begIt))
        unitsOfTravel.push_back(
            std::tuple<TSIt, TSIt, bool>(noArunkTvl.first, noArunkTvl.second, MULTIPLE_SEGMENT));
    }
  }
}

void
ServiceFeesGroup::collectUnitsOfTravelForSA(
    std::vector<std::tuple<TSIt, TSIt, bool> >& unitsOfTravel, TSIt begIt, const TSIt endIt)
{
  TSIt prevBegIt = begIt;
  static bool MULTIPLE_SEGMENT = false;

  for (; begIt != endIt; ++begIt)
  {
    if (!(*begIt)->isAir())
      continue;

    if (prevBegIt != begIt)
    {
      std::pair<TSIt, TSIt> noArunkTvl;
      if (findTravelWhenNoArkunksOnBegOrEnd(noArunkTvl, prevBegIt, begIt))
        unitsOfTravel.push_back(
            std::tuple<TSIt, TSIt, bool>(noArunkTvl.first, noArunkTvl.second, MULTIPLE_SEGMENT));
    }
    prevBegIt = begIt + 1;
    unitsOfTravel.push_back(std::tuple<TSIt, TSIt, bool>(begIt, prevBegIt, MULTIPLE_SEGMENT));
  }
  if (prevBegIt != begIt && !unitsOfTravel.empty())
  {
    std::pair<TSIt, TSIt> noArunkTvl;
    if (findTravelWhenNoArkunksOnBegOrEnd(noArunkTvl, prevBegIt, begIt))
      unitsOfTravel.push_back(
          std::tuple<TSIt, TSIt, bool>(noArunkTvl.first, noArunkTvl.second, MULTIPLE_SEGMENT));
  }
}

const char* const ServiceFeesGroup::_stateCodeStrings[] = {
    "VALID",
    "EMPTY",
    "INVALID",
    "NOT_AVAILABLE"
};

}

