//-------------------------------------------------------------------
//
//  File:        PseudoObjectBuilder.cpp
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

#include "TicketingFee/PseudoObjectsBuilder.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "DataModel/Billing.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/TktFeesRequest.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/RuleConst.h"

#include <iostream>

namespace tse
{

static Logger
logger("atseintl.TicketingFee.PseudoObjectsBuilder");

PseudoObjectsBuilder::PseudoObjectsBuilder(TktFeesPricingTrx& trx)
  : _trx(trx),
    _currentItin(nullptr),
    _dataHandle(trx.dataHandle()),
    _isRTJourney(false),
    _farePath(nullptr),
    _currentPaxType(nullptr),
    _pricingUnit(nullptr),
    _nextFareCompNo(0),
    _totalNumFareComp(0),
    _currentItinTktFeesFareBreakInfo(nullptr),
    _currentItinTktFeesFareBreakAssoc(nullptr),
    _currentFareBreakInfo(nullptr)
{
}

void
PseudoObjectsBuilder::build()
{
  if (_trx.itin().size() == 0)
    return;

  TktFeesRequest* req = dynamic_cast<TktFeesRequest*>(_trx.getRequest());
  if (!req)
    return;

  std::vector<Itin*>::const_iterator itinI = _trx.itin().begin();
  for (; itinI != _trx.itin().end(); ++itinI)
  {
    build(*itinI);
  }
}
void
PseudoObjectsBuilder::build(Itin* prcItin)
{
  TktFeesRequest* req = static_cast<TktFeesRequest*>(_trx.getRequest());
  std::vector<PaxType*>::const_iterator paxI = req->paxTypesPerItin()[prcItin].begin();
  std::vector<PaxType*>::const_iterator paxIEnd = req->paxTypesPerItin()[prcItin].end();

  for (; paxI != paxIEnd; ++paxI)
  {
    build(prcItin, **paxI);
  }
}

FarePath*
PseudoObjectsBuilder::build(Itin* prcItin, PaxType& paxType)
{
  if (!prcItin || prcItin->fareMarket().empty())
    return nullptr;
  _currentItin = prcItin;
  findCurrentItinTktFeesFareBreakInfo(prcItin);
  _sideTripPUs.clear();

  _currentPaxType = &paxType;

  try
  {
    _nextFareCompNo = 0;
    _totalNumFareComp = getTotalNumFareComp();

    buildPU();
    _dataHandle.get(_farePath);
    _farePath->pricingUnit().push_back(_pricingUnit);
    _farePath->pricingUnit().insert(
        _farePath->pricingUnit().end(), _sideTripPUs.begin(), _sideTripPUs.end());
    insertFareUsageTvlSegToItsPU();
    _farePath->paxType() = &paxType;
    _farePath->itin() = _currentItin;
    _currentItin->farePath().push_back(_farePath);
    //        initAccountCodes(prcItin);
    //        initFOPBinNumber(prcItin);

    diag881(paxType);
  }
  catch (...) { return nullptr; }

  return _farePath;
}

void
PseudoObjectsBuilder::buildPU()
{
  _dataHandle.get(_pricingUnit);
  _pricingUnit->puType() = (_isRTJourney) ? PricingUnit::Type::ROUNDTRIP : PricingUnit::Type::ONEWAY;
  buildPUWithFareBreakInfo();
}

void
PseudoObjectsBuilder::buildPUWithFareBreakInfo()
{
  for (_nextFareCompNo = 0; _nextFareCompNo < _totalNumFareComp; ++_nextFareCompNo)
  {
    const TktFeesRequest::TktFeesFareBreakInfo& fbInfo =
        *((*_currentItinTktFeesFareBreakInfo)[_nextFareCompNo]);
    buildPUWithSpecifiedFareBreakInfo(fbInfo);
  }
}

void
PseudoObjectsBuilder::buildPUWithSpecifiedFareBreakInfo(
    const TktFeesRequest::TktFeesFareBreakInfo& fbInfo)
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
PseudoObjectsBuilder::buildFareUsage(const TktFeesRequest::TktFeesFareBreakInfo& fbInfo,
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
PseudoObjectsBuilder::findFareUsageTvlSegs(const TktFeesRequest::TktFeesFareBreakInfo& fbInfo,
                                           std::vector<TravelSeg*>& fuTvlSeg,
                                           uint8_t& sideTripID)
{
  const SequenceNumber fcID = fbInfo.fareComponentID();

  for (const TktFeesRequest::TktFeesFareBreakAssociation* fba : *_currentItinTktFeesFareBreakAssoc)
  {
    if (fba->fareComponentID() != fcID)
      continue;

    for (TravelSeg* ts : _currentItin->travelSeg())
    {
      if (ts->pnrSegment() == fba->segmentID())
      {
        fuTvlSeg.push_back(ts);
        break;
      }
    }
    sideTripID = fba->sideTripID();
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
PseudoObjectsBuilder::buildFareUsageIntoSideTripPU(FareUsage* fareUsage, const uint8_t sideTripID)
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

uint16_t
PseudoObjectsBuilder::getTotalNumFareComp()
{
  return _currentItinTktFeesFareBreakInfo->size();
}

FareUsage*
PseudoObjectsBuilder::buildFareUsage(std::vector<TravelSeg*>::const_iterator& tvlSegIBgn,
                                     std::vector<TravelSeg*>::const_iterator& tvlSegIEnd)
{
  FareUsage* fareUsage = _dataHandle.create<FareUsage>();

  fareUsage->travelSeg().assign(tvlSegIBgn, tvlSegIEnd);
  if (_currentFareBreakInfo->forbidCreditCard())
    fareUsage->mutableForbiddenFop().set(Fare::FOP_CREDIT);

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

  buildDifferentialData(fareUsage);
  return fareUsage;
}
void
PseudoObjectsBuilder::buildDifferentialData(FareUsage* fareUsage)
{
  if (_currentFareBreakInfo && !_currentFareBreakInfo->tktFeesDiffData().empty())
  {
    for (TktFeesRequest::TktFeesDifferentialData* tfdd : _currentFareBreakInfo->tktFeesDiffData())
    {
      if (tfdd)
      {
        DifferentialData* dd = buildDifferentialItem(*fareUsage, *tfdd);
        if (dd)
          fareUsage->differentialPlusUp().push_back(dd);
      }
    }
  }
}

DifferentialData*
PseudoObjectsBuilder::buildDifferentialItem(FareUsage& fareUsage,
                                            const TktFeesRequest::TktFeesDifferentialData& tfdd)
{
  DifferentialData* dd = _trx.dataHandle().create<DifferentialData>();
  FareMarket* fm = fareUsage.paxTypeFare()->fareMarket();
  if (fm == nullptr)
    return nullptr;
  PaxTypeFare* ptfD = buildPaxTypeFareDiff(*fm, tfdd);
  if (ptfD == nullptr)
    return nullptr;
  dd->fareHigh() = ptfD;
  return dd;
}

PaxTypeFare*
PseudoObjectsBuilder::buildPaxTypeFareDiff(FareMarket& fm,
                                           const TktFeesRequest::TktFeesDifferentialData& tfdd)
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
  fareInfo->carrier() = tfdd.diffCarrierCode();
  fareInfo->fareClass() = tfdd.fareBasis();
  fareInfo->market1() = fm.boardMultiCity();
  fareInfo->market2() = fm.offMultiCity();
  paxTypeFare->fareMarket() = &fm;
  paxTypeFare->fareClassAppInfo() = fcaInfo;
  paxTypeFare->fareClassAppSegInfo() = fcaInfoSeg;
  fare->initialize(Fare::FS_International, fareInfo, fm, tariffCrossRefInfo);
  paxTypeFare->setFare(fare);

  return paxTypeFare;
}

FareMarket*
PseudoObjectsBuilder::findFareMarket(const TravelSeg* tvlSegFront, const TravelSeg* tvlSegBack)
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
PseudoObjectsBuilder::buildPaxTypeFare(FareMarket& fm)
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
    LOG4CXX_ERROR(logger, "no currentFareBreakInfo");
    throw ErrorResponseException(ErrorResponseException::PROCESSING_ERROR_DETECTED);
  }

  fareInfo->vendor() = _currentFareBreakInfo->vendorCode(); // from request
  fareInfo->carrier() = _currentFareBreakInfo->governingCarrier();
  fareInfo->directionality() = BOTH;
  fareInfo->fareClass() = _currentFareBreakInfo->fareBasis();
  fareInfo->market1() = fm.boardMultiCity();
  fareInfo->market2() = fm.offMultiCity();
  paxTypeFare->paxType() = _currentPaxType;
  paxTypeFare->fareMarket() = &fm;
  paxTypeFare->fareClassAppInfo() = fcaInfo;
  paxTypeFare->fareClassAppSegInfo() = fcaInfoSeg;
  fare->initialize(Fare::FS_International, fareInfo, fm, tariffCrossRefInfo);
  paxTypeFare->setFare(fare);

  fm.allPaxTypeFare().push_back(paxTypeFare);
  return paxTypeFare;
}

void
PseudoObjectsBuilder::findCurrentItinTktFeesFareBreakInfo(Itin* itin)
{
  _currentItinTktFeesFareBreakInfo = nullptr;
  _currentItinTktFeesFareBreakAssoc = nullptr;

  TktFeesRequest* tktFeesReq = static_cast<TktFeesRequest*>(_trx.getRequest());
  std::map<const Itin*, std::vector<TktFeesRequest::TktFeesFareBreakInfo*> >::const_iterator
  fbPerItinIter = tktFeesReq->tktFeesFareBreakPerItin().find(itin);
  if (fbPerItinIter == tktFeesReq->tktFeesFareBreakPerItin().end())
    return;
  _currentItinTktFeesFareBreakInfo = &(fbPerItinIter->second);

  std::map<const Itin*, std::vector<TktFeesRequest::TktFeesFareBreakAssociation*> >::const_iterator
  fbAssocPerItinIter = tktFeesReq->tktFeesFareBreakAssociationPerItin().find(itin);
  if (fbAssocPerItinIter != tktFeesReq->tktFeesFareBreakAssociationPerItin().end() &&
      (!(fbAssocPerItinIter->second).empty()))
    _currentItinTktFeesFareBreakAssoc = &(fbAssocPerItinIter->second);
}

void
PseudoObjectsBuilder::insertFareUsageTvlSegToItsPU()
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
PseudoObjectsBuilder::diag881(PaxType& paxType)
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
    diag << " **  PU  ** FARE COMPONENT ** TRAVEL SEG ** DIFF HIGH **\n";
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

    printFareUsage(pu, diag);
  }
  printAccountCode(diag);
  printCorpId(diag);
  printTktDesignator(diag);
  printFopBin(diag);
}

void
PseudoObjectsBuilder::printFareUsage(PricingUnit& pu, DiagCollector& diag)
{
  int16_t fcNum = 1;
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
        diag << std::setw(1) << std::right << " " << ts->origAirport() << "*" << ts->destAirport()
             << "\n";
      else
        diag << std::setw(1) << std::right << " " << ts->origAirport() << "-" << ts->destAirport()
             << "\n";
    }
    std::vector<DifferentialData*>::const_iterator ddI = (*fuI)->differentialPlusUp().begin();
    std::vector<DifferentialData*>::const_iterator ddIEnd = (*fuI)->differentialPlusUp().end();
    for (; ddI != ddIEnd; ++ddI)
    {
      if ((*ddI)->fareHigh() != nullptr)
        diag << std::setw(44) << std::right << (*ddI)->fareHigh()->carrier() << "-"
             << (*ddI)->fareHigh()->fareClass() << "\n";
    }
  }
}

void
PseudoObjectsBuilder::printAccountCode(DiagCollector& diag)
{
  TktFeesRequest* req = static_cast<TktFeesRequest*>(_trx.getRequest());
  std::vector<std::string>& acc = req->accountCodeIdPerItin()[_currentItin];
  if (acc.size() == 0)
    return;
  diag << "ACCOUNT CODE: ";
  std::vector<std::string>::const_iterator tsIBegin = acc.begin();
  std::vector<std::string>::const_iterator tsI = tsIBegin;
  for (; tsI != acc.end(); ++tsI)
  {
    if (tsI == tsIBegin)
      diag << (*tsI) << "\n";
    else
      diag << "              " << (*tsI) << "\n";
  }
}

void
PseudoObjectsBuilder::printCorpId(DiagCollector& diag)
{
  TktFeesRequest* req = static_cast<TktFeesRequest*>(_trx.getRequest());
  std::vector<std::string>& corpId = req->corpIdPerItin()[_currentItin];
  if (corpId.size() == 0)
    return;
  diag << "CORPORATE ID: ";
  std::vector<std::string>::const_iterator tsIBegin = corpId.begin();
  std::vector<std::string>::const_iterator tsI = tsIBegin;
  for (; tsI != corpId.end(); ++tsI)
  {
    if (tsI == tsIBegin)
      diag << (*tsI) << "\n";
    else
      diag << "              " << (*tsI) << "\n";
  }
}

void
PseudoObjectsBuilder::printTktDesignator(DiagCollector& diag)
{
  TktFeesRequest* req = static_cast<TktFeesRequest*>(_trx.getRequest());
  std::map<int16_t, TktDesignator>& tDes = req->tktDesignatorPerItin()[_currentItin];
  if (tDes.size() == 0)
    return;
  diag << "TKT DESIGNATOR: ";

  std::map<int16_t, TktDesignator>::const_iterator tsIBegin = tDes.begin();
  std::map<int16_t, TktDesignator>::const_iterator tsI = tsIBegin;
  for (; tsI != tDes.end(); ++tsI)
  {
    if (tsI == tsIBegin)
      diag << "SEG-" << tsI->first << " " << tsI->second << "\n";
    else
      diag << "                "
           << "SEG-" << tsI->first << " " << tsI->second << "\n";
  }
}

void
PseudoObjectsBuilder::printFopBin(DiagCollector& diag)
{
  TktFeesRequest* req = static_cast<TktFeesRequest*>(_trx.getRequest());
  TktFeesRequest::PaxTypePayment* ptp = req->paxTypePaymentPerItin()[_currentItin];
  if (ptp == nullptr)
    return;
  std::vector<TktFeesRequest::PassengerPaymentInfo*>& ppiV = ptp->ppiV();
  if (ppiV.empty())
    return;
  TktFeesRequest::PassengerPaymentInfo* ppi = ppiV[0]; // all FOP_BIN's are the same
  std::vector<TktFeesRequest::FormOfPayment*>& fopV = ppi->fopVector();
  if (fopV.empty())
    return;
  diag << "FOP BIN NUMBER: " << fopV[0]->fopBinNumber() << "\n";
  diag << "CHARGE AMOUNT : " << Money(fopV[0]->chargeAmount(), ptp->currency()) << "\n";
}

void
PseudoObjectsBuilder::initAccountCodes(Itin* itin)
{
  TktFeesRequest* req = dynamic_cast<TktFeesRequest*>(_trx.getRequest());
  if (!req)
    return;
  // copy tkt info data
  req->tktDesignator() = req->tktDesignatorPerItin()[itin];
  // copy acount code and corp id's
  req->accCodeVec() = req->accountCodeIdPerItin()[itin];
  req->corpIdVec() = req->corpIdPerItin()[itin];
  req->incorrectCorpIdVec() = req->invalidCorpIdPerItin()[itin];
  req->isMultiAccCorpId() =
      !req->accCodeVec().empty() || !req->corpIdVec().empty() || !req->incorrectCorpIdVec().empty();
}

void
PseudoObjectsBuilder::initFOPBinNumber(Itin* itin)
{
  TktFeesRequest* req = dynamic_cast<TktFeesRequest*>(_trx.getRequest());
  if (!req)
    return;
  TktFeesRequest::PaxTypePayment* ptp = req->paxTypePaymentPerItin()[itin];
  if (ptp == nullptr)
  {
    LOG4CXX_ERROR(logger, "Null pointer to PTP data");
    throw ErrorResponseException(ErrorResponseException::PROCESSING_ERROR_DETECTED);
  }
  std::vector<TktFeesRequest::PassengerPaymentInfo*>& ppiV = ptp->ppiV();
  if (ppiV.empty())
  {
    LOG4CXX_ERROR(logger, "Empty ppiVector in PTP data");
    throw ErrorResponseException(ErrorResponseException::PROCESSING_ERROR_DETECTED);
  }
  TktFeesRequest::PassengerPaymentInfo* ppi = ppiV[0];
  std::vector<TktFeesRequest::FormOfPayment*>& fopV = ppi->fopVector();
  if (!fopV.empty())
  {
    req->formOfPayment() = fopV[0]->fopBinNumber();
  }
  if (fopV.size() == 2) // secondary BIN#
    req->secondFormOfPayment() = fopV[1]->fopBinNumber();
}
}
