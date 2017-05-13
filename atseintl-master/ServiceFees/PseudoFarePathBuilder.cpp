//-------------------------------------------------------------------
//
//  File:        PseudoFarePathBuilder.cpp
//
//  Description:
//
//  Copyright Sabre 2010
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

#include "ServiceFees/PseudoFarePathBuilder.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/ServiceFeeUtil.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/RuleConst.h"

#include <iostream>
#include <vector>

namespace tse
{
namespace
{
ConfigurableValue<std::string>
m70IgnoreFbiCxrs("SERVICE_FEES_SVC", "M70_IGNORE_FBI_CXRS");
}

static Logger
logger("atseintl.ServiceFees.PseudoFarePathBuilder");

PseudoFarePathBuilder::PseudoFarePathBuilder(PricingTrx& trx)
  : _trx(trx),
    _currentItin(nullptr),
    _dataHandle(trx.dataHandle()),
    _isRTJourney(false),
    _processingBaggage(false),
    _applyFixedTariff(false),
    _farePath(nullptr),
    _currentPaxType(nullptr),
    _pricingUnit(nullptr),
    _nextFareCompNo(0),
    _totalNumFareComp(0),
    _currentItinAncFareBreakInfo(nullptr),
    _currentItinAncFareBreakAssoc(nullptr),
    _currentFareBreakInfo(nullptr)
{
  _processingBaggage = processingBaggage();
}

void
PseudoFarePathBuilder::build()
{
  if (_trx.itin().size() == 0)
    return;

  AncRequest* req = dynamic_cast<AncRequest*>(_trx.getRequest());
  if (!req)
    return;

  std::vector<Itin*>::const_iterator itinI = _trx.itin().begin();
  for (; itinI != _trx.itin().end(); ++itinI)
  {
    // if no pricing itins, then work as single Itin (M70) request
    if (req->pricingItins()[*itinI].empty())
      build(*itinI, *itinI);
    // if don't have master itin in trx->itin, we need to process tjis as
    // pricing itin
    else if (req->isPostTktRequest() && req->masterItin() != *itinI)
    {
      build(*itinI, *itinI);
    }

    std::vector<Itin*>::const_iterator prcItnIt = req->pricingItins()[*itinI].begin();
    std::vector<Itin*>::const_iterator prcItnIe = req->pricingItins()[*itinI].end();
    for (; prcItnIt != prcItnIe; prcItnIt++)
      build(*itinI, *prcItnIt);
  }
}
void
PseudoFarePathBuilder::build(Itin* masterItin, Itin* prcItin)
{
  AncRequest* req = static_cast<AncRequest*>(_trx.getRequest());
  std::vector<PaxType*>::const_iterator paxI = req->paxTypesPerItin()[prcItin].begin();
  std::vector<PaxType*>::const_iterator paxIEnd = req->paxTypesPerItin()[prcItin].end();

  for (; paxI != paxIEnd; ++paxI)
  {
    build(masterItin, prcItin, **paxI);
  }
}

FarePath*
PseudoFarePathBuilder::build(Itin* masterItin, Itin* prcItin, PaxType& paxType)
{
  if (!masterItin || masterItin->fareMarket().empty())
    return nullptr;

  _currentItin = masterItin;
  setJourneyType();
  findCurrentItinAncFareBreakInfo(prcItin);
  _sideTripPUs.clear();

  _currentPaxType = &paxType;

  try
  {
    _nextFareCompNo = 0;
    _totalNumFareComp = getTotalNumFareComp();

    buildPU();
    if (!_sideTripPUs.empty())
      sideTripPUType();
    _dataHandle.get(_farePath);
    _farePath->pricingUnit().push_back(_pricingUnit);
    _farePath->pricingUnit().insert(
        _farePath->pricingUnit().end(), _sideTripPUs.begin(), _sideTripPUs.end());
    insertFareUsageTvlSegToItsPU();
    _farePath->paxType() = &paxType;
    _farePath->itin() = _currentItin;
    _currentItin->farePath().push_back(_farePath);
    insertTourCode(prcItin);
    initAccountCodes(prcItin);
    checkBookingCodeForWPAEMulitItinRequest(prcItin);

    diag881(paxType);
  }
  catch (...)
  {
    return nullptr;
  }

  return _farePath;
}

void
PseudoFarePathBuilder::buildPU()
{
  _dataHandle.get(_pricingUnit);
  _pricingUnit->puType() =
      (_isRTJourney) ? PricingUnit::Type::ROUNDTRIP : PricingUnit::Type::ONEWAY;

  if (fareUsageUnknown() || ignoreFBI() || skipForRW())
  {
    try
    {
      std::vector<TravelSeg*>::const_iterator tvlSegIBgn = _currentItin->travelSeg().begin();
      std::vector<TravelSeg*>::const_iterator tvlSegIEnd = _currentItin->travelSeg().end();
      if (!_isRTJourney)
      {
        _pricingUnit->fareUsage().push_back(buildFareUsage(tvlSegIBgn, tvlSegIEnd));
        return;
      }

      std::vector<TravelSeg*>::const_iterator firstTvlSegI = tvlSegIBgn;
      bool builtByStopovers = true;
      for (std::vector<TravelSeg*>::const_iterator i = tvlSegIBgn; i < tvlSegIEnd - 1; ++i)
      {
        const TravelSeg* curSeg = *i;
        const TravelSeg* nextSeg = *(i + 1);
        if (nextSeg->isStopOver(curSeg, _currentItin->geoTravelType()))
        {
          if (!findFareMarket(*firstTvlSegI, curSeg))
          {
            builtByStopovers = false;
            _pricingUnit->fareUsage().clear();
            break;
          }
          std::vector<TravelSeg*>::const_iterator a = firstTvlSegI;
          std::vector<TravelSeg*>::const_iterator b = i + 1;
          _pricingUnit->fareUsage().push_back(buildFareUsage(a, b));
          firstTvlSegI = i + 1;
          if ((*firstTvlSegI)->segmentType() == Arunk)
          {
            ++i;
            ++firstTvlSegI;
          }
        }
      }
      if (builtByStopovers && !_pricingUnit->fareUsage().empty())
      {
        if (!findFareMarket(*firstTvlSegI, _currentItin->travelSeg().back()))
        {
          _pricingUnit->fareUsage().clear();
        }
        else
        {
          std::vector<TravelSeg*>::const_iterator b = _currentItin->travelSeg().end();
          _pricingUnit->fareUsage().push_back(buildFareUsage(firstTvlSegI, b));
          return;
        }
      }
      if (_pricingUnit->fareUsage().empty()) // Wasn't able to build from stopover to stopover
      {
        std::vector<TravelSeg*>::const_iterator tvlSegIBgn = _currentItin->travelSeg().begin();
        std::vector<TravelSeg*>::const_iterator tvlSegIEnd = tvlSegIBgn;

        while (getNextFUTvlSegs(tvlSegIBgn, tvlSegIEnd))
        {
          if (tvlSegIBgn != tvlSegIEnd)
          {
            FareUsage* fareUsage = buildFareUsage(tvlSegIBgn, tvlSegIEnd);
            _pricingUnit->fareUsage().push_back(fareUsage);
          }
        }
        return;
      }
    }
    catch (...)
    {
      buildPUWithExistingFareMarket();
      return;
    }
  }
  else
  {
    try
    {
      _applyFixedTariff = false;

      if (_totalNumFareComp < _currentItinAncFareBreakAssoc->size())
        buildPUWithMixWithFBIAndNoFBI();
      else
        buildPUWithFareBreakInfo();
    }
    catch (...)
    {
      if (!_processingBaggage)
        throw;

      _applyFixedTariff = true;

      DiagManager diag(_trx, Diagnostic852);

      if (diag.isActive())
      {
        diag << "*** WARNING: NO VALID FAREMARKET FOUND - FAKE FARES HAVE BEEN CREATED\n";
      }
      buildPUWithExistingFareMarket();
    }
  }
}

void
PseudoFarePathBuilder::buildPUWithFareBreakInfo()
{
  for (_nextFareCompNo = 0; _nextFareCompNo < _totalNumFareComp; ++_nextFareCompNo)
  {
    const AncRequest::AncFareBreakInfo& fbInfo =
        *((*_currentItinAncFareBreakInfo)[_nextFareCompNo]);
    buildPUWithSpecifiedFareBreakInfo(fbInfo);
  }
}

void
PseudoFarePathBuilder::buildPUWithSpecifiedFareBreakInfo(const AncRequest::AncFareBreakInfo& fbInfo)
{
  uint8_t sideTripID = 0;
  FareUsage* fareUsage = buildFareUsage(fbInfo, sideTripID);
  if (!fareUsage)
    throw ErrorResponseException(ErrorResponseException::PROCESSING_ERROR_DETECTED);

  if (sideTripID == 0)
  {
    _pricingUnit->fareUsage().push_back(fareUsage);
    return;
  }
  buildFareUsageIntoSideTripPU(fareUsage, sideTripID);
}

FareUsage*
PseudoFarePathBuilder::buildFareUsage(const AncRequest::AncFareBreakInfo& fbInfo,
                                      uint8_t& sideTripID)
{
  std::vector<TravelSeg*> fuTvlSeg;
  findFareUsageTvlSegs(fbInfo, fuTvlSeg, sideTripID);
  if (fuTvlSeg.empty())
  {
    LOG4CXX_ERROR(logger, "not findFareUsageTvlSegs");
    throw ErrorResponseException(ErrorResponseException::PROCESSING_ERROR_DETECTED);
  }

  std::vector<TravelSeg*>::const_iterator tvlSegIBgn = fuTvlSeg.begin();
  std::vector<TravelSeg*>::const_iterator tvlSegIEnd = fuTvlSeg.end();

  _currentFareBreakInfo = &fbInfo;
  FareUsage* fareUsage = buildFareUsage(tvlSegIBgn, tvlSegIEnd);
  _currentFareBreakInfo = nullptr;

  return fareUsage;
}

void
PseudoFarePathBuilder::findFareUsageTvlSegs(const AncRequest::AncFareBreakInfo& fbInfo,
                                            std::vector<TravelSeg*>& fuTvlSeg,
                                            uint8_t& sideTripID)
{
  const SequenceNumber fcID = fbInfo.fareComponentID();

  std::vector<AncRequest::AncFareBreakAssociation*>::const_iterator fbAssocI =
      _currentItinAncFareBreakAssoc->begin();
  std::vector<AncRequest::AncFareBreakAssociation*>::const_iterator fbAssocIEnd =
      _currentItinAncFareBreakAssoc->end();
  for (; fbAssocI != fbAssocIEnd; ++fbAssocI)
  {
    if ((*fbAssocI)->fareComponentID() == fcID)
    {
      std::vector<TravelSeg*>::const_iterator tvlSegI = _currentItin->travelSeg().begin();
      std::vector<TravelSeg*>::const_iterator tvlSegIEnd = _currentItin->travelSeg().end();
      for (; tvlSegI != tvlSegIEnd; ++tvlSegI)
      {
        if ((*tvlSegI)->segmentOrder() == (*fbAssocI)->segmentID())
        {
          fuTvlSeg.push_back(*tvlSegI);
          if ((tvlSegI + 1) != tvlSegIEnd && (*(tvlSegI + 1))->segmentType() == Arunk &&
              ((fbAssocI + 1) == fbAssocIEnd ||
               (*(tvlSegI + 1))->segmentOrder() != (*(fbAssocI + 1))->segmentID()))
          {
            // Arunk seg did not have FBA, it can not be start of
            // next FareUsage, so push it into current FareUsage
            ++tvlSegI;
            fuTvlSeg.push_back(*tvlSegI);
          }
          break;
        }
      }
      sideTripID = (*fbAssocI)->sideTripID();
    }
  }
}

class IsSameSideTripID
{
public:
  IsSameSideTripID(uint8_t sideTripID) : _sideTripID(sideTripID) {}

  bool operator()(PricingUnit* pu) { return pu->sideTripNumber() == _sideTripID; }

private:
  uint8_t _sideTripID;
};

void
PseudoFarePathBuilder::buildFareUsageIntoSideTripPU(FareUsage* fareUsage, const uint8_t sideTripID)
{
  PricingUnit* sideTripPU = nullptr;
  std::vector<PricingUnit*>::iterator puIter =
      std::find_if(_sideTripPUs.begin(), _sideTripPUs.end(), IsSameSideTripID(sideTripID));
  if (puIter == _sideTripPUs.end())
  {
    _dataHandle.get(sideTripPU);
    sideTripPU->isSideTripPU() = true;
    sideTripPU->sideTripNumber() = sideTripID;
    _sideTripPUs.push_back(sideTripPU);
  }
  else
  {
    sideTripPU = *puIter;
  }

  sideTripPU->fareUsage().push_back(fareUsage);
}

void
PseudoFarePathBuilder::setJourneyType()
{
  ServiceFeeUtil srvFeeUtil(_trx);

  _isRTJourney =
      srvFeeUtil.isRoundTripJourneyType(*_currentItin,
                                        _currentItin->validatingCarrier(),
                                        srvFeeUtil.isInternationalJourneyType(*_currentItin));
}

bool
PseudoFarePathBuilder::getNextFUTvlSegs(std::vector<TravelSeg*>::const_iterator& tvlSegIBgn,
                                        std::vector<TravelSeg*>::const_iterator& tvlSegIEnd)
{
  if (_nextFareCompNo >= _totalNumFareComp)
    return false;

  if (!_isRTJourney)
  {
    tvlSegIBgn = _currentItin->travelSeg().begin();
    tvlSegIEnd = _currentItin->travelSeg().end();
  }
  else
  {
    if (_nextFareCompNo == 0)
    {
      tvlSegIBgn = _currentItin->travelSeg().begin();
      tvlSegIEnd = tvlSegIBgn + _currentItin->furthestPointSegmentOrder();
    }
    else
    {
      tvlSegIBgn = tvlSegIBgn + _currentItin->furthestPointSegmentOrder();
      tvlSegIEnd = _currentItin->travelSeg().end();
    }
  }
  _nextFareCompNo++;
  return true;
}

uint16_t
PseudoFarePathBuilder::getTotalNumFareComp()
{
  if (fareUsageUnknown() || ignoreFBI())
  {
    if (_isRTJourney)
      return 2;
    else
      return 1;
  }

  return _currentItinAncFareBreakInfo->size();
}

FareUsage*
PseudoFarePathBuilder::buildFareUsage(std::vector<TravelSeg*>::const_iterator& tvlSegIBgn,
                                      std::vector<TravelSeg*>::const_iterator& tvlSegIEnd)
{
  FareUsage* fareUsage = nullptr;
  _dataHandle.get(fareUsage);

  if ((*tvlSegIBgn)->segmentType() == Arunk && tvlSegIBgn + 1 != tvlSegIEnd)
  {
    tvlSegIBgn++;
  }
  if ((*(tvlSegIEnd - 1))->segmentType() == Arunk && tvlSegIBgn != (tvlSegIEnd - 1))
  {
    --tvlSegIEnd;
  }

  fareUsage->travelSeg().insert(fareUsage->travelSeg().end(), tvlSegIBgn, tvlSegIEnd);
  FareMarket* fareMarket = findFareMarket(*tvlSegIBgn, *(tvlSegIEnd - 1));
  if (!fareMarket)
  {
    DiagManager diag(_trx, Diagnostic881);
    if (diag.isActive())
    {
      diag << "NO FARE MARKET INCLUDE TRAVELSEG " << (*tvlSegIBgn)->segmentOrder() << "-"
           << (*(tvlSegIEnd - 1))->segmentOrder() << "\n";
    }
    LOG4CXX_ERROR(logger,
                  "No FareMarket includes TravelSeg " << (*tvlSegIBgn)->segmentOrder() << "-"
                                                      << (*(tvlSegIEnd - 1))->segmentOrder());
    throw ErrorResponseException(ErrorResponseException::PROCESSING_ERROR_DETECTED);
  }

  PaxTypeFare* paxTypeFare = buildPaxTypeFare(*fareMarket);
  fareUsage->paxTypeFare() = paxTypeFare;
  fareUsage->segmentStatus().insert(fareUsage->segmentStatus().end(),
                                    fareUsage->paxTypeFare()->segmentStatus().begin(),
                                    fareUsage->paxTypeFare()->segmentStatus().end());

  return fareUsage;
}

FareMarket*
PseudoFarePathBuilder::findFareMarket(const TravelSeg* tvlSegFront, const TravelSeg* tvlSegBack)
{
  std::vector<FareMarket*>::const_iterator fmI = _currentItin->fareMarket().begin();
  const std::vector<FareMarket*>::const_iterator fmIEnd = _currentItin->fareMarket().end();

  for (; fmI != fmIEnd; ++fmI)
  {
    if ((*fmI)->travelSeg().front() == tvlSegFront && (*fmI)->travelSeg().back() == tvlSegBack)
    {
      return (*fmI);
    }
  }
  return nullptr;
}

PaxTypeFare*
PseudoFarePathBuilder::buildPaxTypeFare(FareMarket& fm)
{
  PaxTypeFare* paxTypeFare = nullptr;
  Fare* fare = nullptr;
  FareInfo* fareInfo = nullptr;
  FareClassAppInfo* fcaInfo = nullptr;
  TariffCrossRefInfo* tariffCrossRefInfo = nullptr;
  FareClassAppSegInfo* fcaInfoSeg = nullptr;
  _dataHandle.get(paxTypeFare);
  _dataHandle.get(fare);
  _dataHandle.get(fareInfo);
  _dataHandle.get(fcaInfo);
  _dataHandle.get(tariffCrossRefInfo);
  _dataHandle.get(fcaInfoSeg);

  if (!paxTypeFare || !fare || !fareInfo || !tariffCrossRefInfo || !fcaInfo || !fcaInfoSeg)
    throw ErrorResponseException(ErrorResponseException::MEMORY_EXCEPTION);

  if (!_currentFareBreakInfo)
  {
    fareInfo->vendor() = tse::ATPCO_VENDOR_CODE;
    fareInfo->carrier() = fm.governingCarrier();
    fareInfo->directionality() = BOTH;
    fareInfo->fareClass() = fm.fareBasisCode();
    fareInfo->fareTariff() = 0;
    fareInfo->ruleNumber() = "";
  }
  else
  {
    fareInfo->vendor() = _currentFareBreakInfo->vendorCode(); // from request
    fareInfo->carrier() = _currentFareBreakInfo->governingCarrier();
    fareInfo->fareAmount() = _currentFareBreakInfo->fareAmount();
    fareInfo->directionality() = BOTH;
    fareInfo->fareClass() = _currentFareBreakInfo->fareBasis();
    fareInfo->fareTariff() = _currentFareBreakInfo->fareTariff();
    fareInfo->ruleNumber() = _currentFareBreakInfo->fareRule();

    fcaInfo->_fareType = _currentFareBreakInfo->fareType();
    fcaInfo->_ruleTariff = fareInfo->fareTariff();

    AncRequest* ancReq = static_cast<AncRequest*>(_trx.getRequest());

    if (_processingBaggage || (ancReq && ancReq->ancRequestType() == AncRequest::WPAERequest) ||
        ancReq->ancRequestType() == AncRequest::PostTktRequest ||
        ancReq->ancRequestType() == AncRequest::WPBGRequest ||
        ancReq->ancRequestType() == AncRequest::BaggageRequest)
    {
      bool isInternational = false;
      if (fm.geoTravelType() == GeoTravelType::International ||
          fm.geoTravelType() == GeoTravelType::ForeignDomestic)
        isInternational = true;

      const std::vector<TariffCrossRefInfo*>& tcrList = _trx.dataHandle().getTariffXRefByFareTariff(
          _currentFareBreakInfo->vendorCode(),
          _currentFareBreakInfo->governingCarrier(),
          (isInternational ? INTERNATIONAL : DOMESTIC),
          _currentFareBreakInfo->fareTariff(),
          ItinUtil::getTravelDate(_currentItin->travelSeg()));
      if (!tcrList.empty())
      {
        if (tcrList.front()->ruleTariff() != _currentFareBreakInfo->fareTariff())
          fcaInfo->_ruleTariff = abs(tcrList.front()->ruleTariff());
      }
      else if (_applyFixedTariff && _processingBaggage && fcaInfo->_ruleTariff == 0)
        fcaInfo->_ruleTariff = -1;
    }
  }

  tariffCrossRefInfo->fareTariff() = fareInfo->fareTariff();
  tariffCrossRefInfo->ruleTariff() = fcaInfo->_ruleTariff;

  fareInfo->market1() = fm.boardMultiCity();
  fareInfo->market2() = fm.offMultiCity();
  paxTypeFare->paxType() = _currentPaxType;
  paxTypeFare->fareMarket() = &fm;
  paxTypeFare->fareClassAppInfo() = fcaInfo;
  paxTypeFare->fareClassAppSegInfo() = fcaInfoSeg;
  fare->initialize(Fare::FS_International, fareInfo, fm, tariffCrossRefInfo);
  paxTypeFare->setFare(fare);

  if (_currentFareBreakInfo)
  {
    if (!_currentFareBreakInfo->fareStat().empty())
    {
      ServiceFeeUtil::setFareStat(*paxTypeFare, _currentFareBreakInfo->fareStat());
    }
    else
    {
      if (_currentFareBreakInfo->privateIndicator())
      {
        paxTypeFare->setTcrTariffCatPrivate();
      }
      ServiceFeeUtil::setFareIndicator(*paxTypeFare, _currentFareBreakInfo->fareIndicator());
    }
    if (paxTypeFare->isDiscounted())
    {
      PaxTypeFareRuleData* rData = nullptr;
      DiscountInfo* dis = nullptr;
      _dataHandle.get(rData);
      _dataHandle.get(dis);
      if (!rData || !dis)
        throw ErrorResponseException(ErrorResponseException::MEMORY_EXCEPTION);
      rData->ruleItemInfo() = dis;
      dis->category() = RuleConst::CHILDREN_DISCOUNT_RULE;
      paxTypeFare->setRuleData(RuleConst::CHILDREN_DISCOUNT_RULE, _dataHandle, rData);
    }
  }

  addPassedSegStatus(*paxTypeFare);
  fm.allPaxTypeFare().push_back(paxTypeFare);

  return paxTypeFare;
}

void
PseudoFarePathBuilder::addPassedSegStatus(PaxTypeFare& paxTypeFare)
{
  PaxTypeFare::SegmentStatus segStat;
  segStat._bkgCodeSegStatus.setNull();

  std::vector<TravelSeg*>::iterator tsI = paxTypeFare.fareMarket()->travelSeg().begin();
  std::vector<TravelSeg*>::iterator tsEndI = paxTypeFare.fareMarket()->travelSeg().end();

  for (; tsI != tsEndI; ++tsI)
  {
    if ((*tsI)->isAir())
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS, true);
    else
      segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_SURFACE, true);

    paxTypeFare.segmentStatus().push_back(segStat);
  }
}

bool
PseudoFarePathBuilder::fareUsageUnknown()
{
  return (_currentItinAncFareBreakInfo == nullptr);
}

void
PseudoFarePathBuilder::findCurrentItinAncFareBreakInfo(Itin* itin)
{
  _currentItinAncFareBreakInfo = nullptr;
  _currentItinAncFareBreakAssoc = nullptr;

  AncRequest* ancReq = static_cast<AncRequest*>(_trx.getRequest());
  std::map<const Itin*, std::vector<AncRequest::AncFareBreakInfo*>>::const_iterator fbPerItinIter =
      ancReq->fareBreakPerItin().find(itin);
  if (fbPerItinIter == ancReq->fareBreakPerItin().end())
    return;

  _currentItinAncFareBreakInfo = &(fbPerItinIter->second);

  std::map<const Itin*, std::vector<AncRequest::AncFareBreakAssociation*>>::const_iterator
  fbAssocPerItinIter = ancReq->fareBreakAssociationPerItin().find(itin);

  if (fbAssocPerItinIter != ancReq->fareBreakAssociationPerItin().end() &&
      (!(fbAssocPerItinIter->second).empty()))
    _currentItinAncFareBreakAssoc = &(fbAssocPerItinIter->second);
  else
    _currentItinAncFareBreakInfo = nullptr;

  if (ancReq->isPostTktRequest() && _currentItinAncFareBreakAssoc)
  {
    std::vector<AncRequest::AncFareBreakAssociation*>::const_iterator fbab =
        _currentItinAncFareBreakAssoc->begin();
    std::vector<AncRequest::AncFareBreakAssociation*>::const_iterator fbae =
        _currentItinAncFareBreakAssoc->end();
    int sideTripID = 0;
    bool withinSideTrip = false;
    for (; fbab != fbae; fbab++)
    {
      // if no side trip id assigned, but side trip begin indicator is set
      if (!withinSideTrip && (*fbab)->sideTripID() == 0 && (*fbab)->sideTripStart())
      {
        withinSideTrip = true;
        ++sideTripID;
      }
      // if within side trip set new id
      if (withinSideTrip)
      {
        (*fbab)->sideTripID() = sideTripID;
        // end of side trip
        if ((*fbab)->sideTripEnd())
          withinSideTrip = false;
      }
    }
  }
}

// default build when OW, RT PU build without FareBreakInfo failed
void
PseudoFarePathBuilder::buildPUWithExistingFareMarket()
{
  _pricingUnit->fareUsage().clear();

  std::vector<TravelSeg*>::const_iterator tvlSegIEnd = _currentItin->travelSeg().end();

  std::vector<TravelSeg*>::const_iterator fmTvlSegIFront = _currentItin->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator fmTvlSegIBack = fmTvlSegIFront;

  buildPUForTravelSegs(fmTvlSegIFront, fmTvlSegIBack, tvlSegIEnd);
}

void
PseudoFarePathBuilder::insertFareUsageTvlSegToItsPU()
{
  std::vector<PricingUnit*>::const_iterator puI = _farePath->pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puIEnd = _farePath->pricingUnit().end();
  for (; puI != puIEnd; ++puI)
  {
    PricingUnit& pu = **puI;
    std::vector<FareUsage*>::const_iterator fuI = pu.fareUsage().begin();
    const std::vector<FareUsage*>::const_iterator fuIEnd = pu.fareUsage().end();
    for (; fuI != fuIEnd; ++fuI)
    {
      pu.travelSeg().insert(
          pu.travelSeg().end(), (*fuI)->travelSeg().begin(), (*fuI)->travelSeg().end());
    }
  }
}

void
PseudoFarePathBuilder::setNoFollowByArunkSeg(
    std::vector<TravelSeg*>::const_iterator& fmTvlSegIBack,
    const std::vector<TravelSeg*>::const_iterator& tvlSegIEnd)
{
  if (fmTvlSegIBack != tvlSegIEnd && (fmTvlSegIBack + 1) != tvlSegIEnd &&
      (*(fmTvlSegIBack + 1))->segmentType() == Arunk)
  {
    fmTvlSegIBack++;
  }
}

void
PseudoFarePathBuilder::insertTourCode(Itin* prcItin)
{
  AncRequest* ancReq = static_cast<AncRequest*>(_trx.getRequest());

  const std::map<const Itin*, std::string>::const_iterator tC =
      ancReq->tourCodePerItin().find(prcItin);
  if (tC == ancReq->tourCodePerItin().end())
    return;

  std::string tourCode = (*tC).second;
  _farePath->cat27TourCode() = tourCode;
}

void
PseudoFarePathBuilder::sideTripPUType()
{
  if (_sideTripPUs.size() == 1 && _sideTripPUs.front()->travelSeg().size() == 1)
    return;
  for (uint8_t sideTripID = 1; sideTripID <= _sideTripPUs.size(); sideTripID++)
  {
    std::vector<PricingUnit*>::iterator puIter =
        std::find_if(_sideTripPUs.begin(), _sideTripPUs.end(), IsSameSideTripID(sideTripID));

    if (puIter == _sideTripPUs.end())
      break;
    if ((*puIter)->fareUsage().size() == 1 &&
        (*puIter)->fareUsage().front()->travelSeg().size() == 1)
      continue;
    if ((*puIter)->fareUsage().front()->travelSeg().front()->boardMultiCity() ==
        (*puIter)->fareUsage().back()->travelSeg().back()->offMultiCity())
      (*puIter)->puType() = PricingUnit::Type::ROUNDTRIP;
  }
}

class IsTravelSegHere
{
public:
  IsTravelSegHere(SequenceNumber number) : _number(number) {}

  bool operator()(TravelSeg* tS) { return tS->segmentOrder() == _number; }

private:
  SequenceNumber _number;
};

void
PseudoFarePathBuilder::buildPUWithMixWithFBIAndNoFBI()
{
  // Build PU and FU based on FareBreakAssociation count
  std::set<uint16_t> fareCompProcessed;
  std::vector<AncRequest::AncFareBreakAssociation*>::const_iterator fbAssocI =
      _currentItinAncFareBreakAssoc->begin();
  std::vector<AncRequest::AncFareBreakAssociation*>::const_iterator fbAssocIEnd =
      _currentItinAncFareBreakAssoc->end();
  for (; fbAssocI != fbAssocIEnd; ++fbAssocI)
  {
    SequenceNumber fcID = (*fbAssocI)->fareComponentID();

    std::vector<AncRequest::AncFareBreakInfo*>::const_iterator fbI =
        _currentItinAncFareBreakInfo->begin();
    std::vector<AncRequest::AncFareBreakInfo*>::const_iterator fbIEnd =
        _currentItinAncFareBreakInfo->end();
    bool processed = false;
    for (; fbI != fbIEnd; ++fbI)
    {
      if ((*fbI)->fareComponentID() == fcID)
      {
        if (find(fareCompProcessed.begin(), fareCompProcessed.end(), fcID) ==
            fareCompProcessed.end())
        {
          buildPUWithSpecifiedFareBreakInfo(*(*fbI));
          fareCompProcessed.insert(fcID);
          processed = true;
          break;
        }
        else
        {
          processed = true;
          break;
        }
      }
    }
    if (!processed)
    {
      // buld PU & FU w/o FBI
      // Collect all travel segments for the same FareComponent.
      if (find(fareCompProcessed.begin(), fareCompProcessed.end(), fcID) == fareCompProcessed.end())
      {
        std::vector<TravelSeg*> tvlSegs;
        std::vector<AncRequest::AncFareBreakAssociation*>::const_iterator fbISFC = fbAssocI;

        for (; fbISFC != fbAssocIEnd; ++fbISFC)
        {
          if ((*fbISFC)->fareComponentID() == fcID)
          {
            std::vector<TravelSeg*>::iterator iter =
                std::find_if(_currentItin->travelSeg().begin(),
                             _currentItin->travelSeg().end(),
                             IsTravelSegHere((*fbISFC)->segmentID()));
            if (iter != _currentItin->travelSeg().end())
              tvlSegs.push_back(*iter);
            fareCompProcessed.insert(fcID);
          }
        }
        std::vector<TravelSeg*>::const_iterator tvlSegIEnd = tvlSegs.end();
        std::vector<TravelSeg*>::const_iterator fmTvlSegIFront = tvlSegs.begin();
        std::vector<TravelSeg*>::const_iterator fmTvlSegIBack = fmTvlSegIFront;

        buildPUForTravelSegs(fmTvlSegIFront, fmTvlSegIBack, tvlSegIEnd);
      }
    }
  }
}

void
PseudoFarePathBuilder::buildPUForTravelSegs(std::vector<TravelSeg*>::const_iterator& fmTvlSegIFront,
                                            std::vector<TravelSeg*>::const_iterator& fmTvlSegIBack,
                                            std::vector<TravelSeg*>::const_iterator& tvlSegIEnd)
{
  setNoFollowByArunkSeg(fmTvlSegIBack, tvlSegIEnd);
  while (fmTvlSegIBack != tvlSegIEnd)
  {
    if ((*fmTvlSegIFront)->segmentType() == Arunk)
    {
      ++fmTvlSegIFront;
    }
    if ((*fmTvlSegIBack)->segmentType() == Arunk)
    {
      --fmTvlSegIBack;
      FareMarket* fareMarket = findFareMarket(*fmTvlSegIFront, *fmTvlSegIBack);
      if (!fareMarket)
      {
        fmTvlSegIBack += 2;
        setNoFollowByArunkSeg(fmTvlSegIBack, tvlSegIEnd);
        continue;
      }
    }
    FareMarket* fareMarket = findFareMarket(*fmTvlSegIFront, *fmTvlSegIBack);
    if (!fareMarket)
    {
      ++fmTvlSegIBack;
      setNoFollowByArunkSeg(fmTvlSegIBack, tvlSegIEnd);
      continue;
    }
    fmTvlSegIBack++;
    FareUsage* fareUsage = buildFareUsage(fmTvlSegIFront, fmTvlSegIBack);
    _pricingUnit->fareUsage().push_back(fareUsage);

    fmTvlSegIFront = fmTvlSegIBack;
    setNoFollowByArunkSeg(fmTvlSegIBack, tvlSegIEnd);
  }
}

void
PseudoFarePathBuilder::diag881(PaxType& paxType)
{
  DiagManager diag881(_trx, Diagnostic881);
  if (!diag881.isActive())
    return;

  DiagCollector& diag = diag881.collector();

  diag << "**** PSEUDO FAREPATH DETAIL FOR PASSENGER = " << paxType.paxType() << " ****\n";

  std::vector<PricingUnit*>::const_iterator puI = _farePath->pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puIEnd = _farePath->pricingUnit().end();
  for (; puI != puIEnd; ++puI)
  {
    diag << " **  PU  ** FARE COMPONENT **  TRAVEL SEG  **\n";
    PricingUnit& pu = **puI;

    if (pu.isSideTripPU())
      diag << "SIDE";
    else
      diag << "    ";
    if (pu.puType() == PricingUnit::Type::ROUNDTRIP)
      diag << " RT";
    else
      diag << " OW";

    diag << "\n";

    uint16_t fcNum = 1;
    std::vector<FareUsage*>::const_iterator fuI = pu.fareUsage().begin();
    const std::vector<FareUsage*>::const_iterator fuIEnd = pu.fareUsage().end();
    for (; fuI != fuIEnd; ++fuI, ++fcNum)
    {
      diag << std::setw(13) << std::right << fcNum << " ";

      if ((*fuI)->paxTypeFare()->fareClass().empty())
        diag << std::setw(12) << std::right << " ";
      else
        diag << std::setw(12) << std::right << (*fuI)->paxTypeFare()->fareClass();
      std::vector<TravelSeg*>::const_iterator tsIBegin = (*fuI)->travelSeg().begin();
      std::vector<TravelSeg*>::const_iterator tsI = tsIBegin;
      for (; tsI != (*fuI)->travelSeg().end(); ++tsI)
      {
        TravelSeg* ts = (*tsI);
        if (tsI == tsIBegin)
          diag << std::setw(4) << std::right << _currentItin->segmentOrder(ts);
        else
          diag << std::setw(30) << std::right << _currentItin->segmentOrder(ts);

        if (ts->segmentType() == Arunk)
          diag << " " << ts->origAirport() << "*" << ts->destAirport() << "\n";
        else
          diag << " " << ts->origAirport() << "-" << ts->destAirport() << "\n";
      }
    }
  }
}
void
PseudoFarePathBuilder::checkBookingCodeForWPAEMulitItinRequest(Itin* prcItin)
{
  AncRequest* ancReq = static_cast<AncRequest*>(_trx.getRequest());
  if (ancReq->ancRequestType() == AncRequest::M70Request || AncRequest::BaggageRequest ||
      prcItin == _currentItin)
    return;

  AncRequest::AncAttrMapTree& map = ancReq->itinAttrMap()[_currentItin]["FLI"];
  AncRequest::AncAttrMapTree& map2 = ancReq->itinAttrMap()[prcItin]["FLI"];
  for (PricingUnit* pu : _farePath->pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      PaxTypeFare* ptf = fu->paxTypeFare();
      int index = 0;
      for (TravelSeg* ts : ptf->fareMarket()->travelSeg())
      {
        Indicator cabin = map[ts->pnrSegment()].getAttributeValue("N0E")[0];
        if (cabin == 'R')
          ts->bookedCabin().setPremiumFirstClass();
        else if (cabin == 'P')
          ts->bookedCabin().setPremiumEconomyClass();
        else
          ts->bookedCabin().setClassFromAlphaNum(cabin);

        if (map2[ts->pnrSegment()].getAttributeValue("B30") !=
            map[ts->pnrSegment()].getAttributeValue("B30"))
        {
          ts->setBookingCode(map[ts->pnrSegment()].getAttributeValue("B30"));
          ptf->segmentStatus().at(index)._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
          ptf->segmentStatus().at(index)._reBookCabin = ts->bookedCabin();
          ptf->segmentStatus().at(index)._bkgCodeReBook = ts->getBookingCode();

          fu->segmentStatus().at(index)._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, true);
          fu->segmentStatus().at(index)._reBookCabin = ts->bookedCabin();
          fu->segmentStatus().at(index)._bkgCodeReBook = ts->getBookingCode();
        }
        index++;
      }
    }
  }
}

void
PseudoFarePathBuilder::initAccountCodes(Itin* itin)
{
  AncRequest* req = dynamic_cast<AncRequest*>(_trx.getRequest());
  if (!req)
    return;
  // copy tkt info data
  req->tktDesignator() = req->tktDesignatorPerItin()[itin];
  // copy aacont code and corp id's
  req->accCodeVec() = req->accountCodeIdPerItin()[itin];
  req->corpIdVec() = req->corpIdPerItin()[itin];
  req->incorrectCorpIdVec() = req->invalidCorpIdPerItin()[itin];
  req->isMultiAccCorpId() =
      !req->accCodeVec().empty() || !req->corpIdVec().empty() || !req->incorrectCorpIdVec().empty();
}

bool
PseudoFarePathBuilder::ignoreFBI()
{
  const AncRequest* req = static_cast<AncRequest*>(_trx.getRequest());
  if (req->majorSchemaVersion() < 2 && req->ancRequestType() == AncRequest::M70Request)
  {
    return m70IgnoreFbiCxrs.getValue().find(_trx.billing()->partitionID(), 0) != std::string::npos;
  }

  return false;
}

bool
PseudoFarePathBuilder::processingBaggage() const
{
  const AncRequest* request = dynamic_cast<AncRequest*>(_trx.getRequest());

  if (request && request->majorSchemaVersion() >= 2)
  {
    if (_trx.billing()->requestPath() == PSS_PO_ATSE_PATH)
      return true;
    else if (_trx.billing()->requestPath() == ACS_PO_ATSE_PATH)
    {
      std::vector<ServiceGroup> groupCodes;
      _trx.getOptions()->getGroupCodes(_trx.getOptions()->serviceGroupsVec(), groupCodes);

      if (groupCodes.empty() ||
          (std::find_if(groupCodes.begin(),
                        groupCodes.end(),
                        ServiceFeeUtil::checkServiceGroupForAcs) != groupCodes.end()))
        return true;
    }
  }
  return false;
}

bool
PseudoFarePathBuilder::skipForRW() const
{
  return false;
}
}
