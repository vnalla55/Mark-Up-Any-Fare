//  Copyright Sabre 2012
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
#include "ServiceFees/SliceAndDice.h"

#include "Common/EmdValidator.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "Common/TseUtil.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/ServiceGroupInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag875Collector.h"
#include "Diagnostic/Diag877Collector.h"
#include "Diagnostic/Diag880Collector.h"
#include "ServiceFees/OCEmdDataProvider.h"
#include "ServiceFees/OptionalFeeConcurValidator.h"
#include "ServiceFees/OptionalServicesValidator.h"
#include "ServiceFees/ServiceFeesGroup.h"

#include <utility>

namespace tse
{
FALLBACK_DECL(fallbackNewGroup99Validation);
FALLBACK_DECL(fallbackDisableIEMDForGroup99);
FALLBACK_DECL(reworkTrxAborter);

namespace
{
Logger
logger("atseintl.ServiceFees.SliceAndDice");
}

const uint16_t SliceAndDice::BEGIN_OF_TVL_PORTION = 0;
const uint16_t SliceAndDice::END_OF_TVL_PORTION = 1;
const uint16_t SliceAndDice::MARKET_DRIVEN_STRATEGY = 2;

const ServiceDisplayInd SliceAndDice::DISPLAY_CAT_STORE = "02";
const Indicator SliceAndDice::EMD_TYPE_ELECTRONIC_TKT = '5';
const Indicator SliceAndDice::EMD_TYPE_ASSOCIATED_TKT = '2';
const Indicator SliceAndDice::EMD_TYPE_STANDALONE = '3';

namespace
{
struct IsUnconfirmed : public std::unary_function<const tse::TravelSeg*, bool>
{
  bool operator()(const tse::TravelSeg* seg) const
  {
    return seg->segmentType() != tse::Arunk && seg->segmentType() != tse::Surface &&
           (seg->resStatus() != tse::CONFIRM_RES_STATUS &&
            seg->resStatus() != tse::NOSEAT_RES_STATUS);
  }
};

class IsSameOrigLoc
{
  const LocCode& _locCode;

public:
  IsSameOrigLoc(const LocCode& locCode) : _locCode(locCode) {}
  bool operator()(const TravelSeg* seg) const
  {
    return _locCode == seg->origin()->loc() || _locCode == seg->boardMultiCity();
  }
};

class IsSameDestLoc
{
  const LocCode& _locCode;

public:
  IsSameDestLoc(const LocCode& locCode) : _locCode(locCode) {}
  bool operator()(const TravelSeg* seg) const
  {
    return _locCode == seg->destination()->loc() || _locCode == seg->offMultiCity();
  }
};

class IsSameSrvGroup
{
  const ServiceGroup _srvGroup;

public:
  IsSameSrvGroup(const ServiceGroup& srvGroup) : _srvGroup(srvGroup) {}

  bool operator()(const ServiceGroup* srvGroup) const { return (*srvGroup) == _srvGroup; }
};
}

SliceAndDice::SliceAndDice(
    PricingTrx& ptrx,
    const ItinBoolMap& isInternational,
    const ItinBoolMap& isRoundTrip,
    bool& stopMatchProcess,
    const Ts2ss& ts2ss,
    const std::map<const CarrierCode, std::vector<ServiceGroup*>>& cXrGrp,
    const std::vector<ServiceGroupInfo*>& allGroupCodes,
    const bool& shopping,
    const bool& needFirstMatchOnly,
    int16_t& numberOfOcFeesForItin,
    bool& timeOut,
    boost::mutex& mutex)
  : _trx(ptrx),
    _isInternational(isInternational),
    _isRoundTrip(isRoundTrip),
    _stopMatchProcess(stopMatchProcess),
    _ts2ss(ts2ss),
    _cXrGrp(cXrGrp),
    _allGroupCodes(allGroupCodes),
    _shopping(shopping),
    _needFirstMatchOnly(needFirstMatchOnly),
    _numberOfOcFeesForItin(numberOfOcFeesForItin),
    _timeOut(timeOut),
    _mutex(mutex)
{
  trx(&_trx);
}

SliceAndDice::SliceAndDice(const SliceAndDice& sd)
  : _trx(sd._trx),
    _isInternational(sd._isInternational),
    _isRoundTrip(sd._isRoundTrip),
    _stopMatchProcess(sd._stopMatchProcess),
    _ts2ss(sd._ts2ss),
    _cXrGrp(sd._cXrGrp),
    _allGroupCodes(sd._allGroupCodes),
    _shopping(sd._shopping),
    _needFirstMatchOnly(sd._needFirstMatchOnly),
    _numberOfOcFeesForItin(sd._numberOfOcFeesForItin),
    _timeOut(sd._timeOut),
    _mutex(sd._mutex),
    _farePath(sd._farePath),
    _journeyDestination(sd._journeyDestination),
    _slice_And_Dice(true)
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  trx(&_trx);
  std::copy(sd._beginsOfUOT, sd._beginsOfUOT + 3, _beginsOfUOT);
  std::copy(sd._beginsOfLargestUOT, sd._beginsOfLargestUOT + 3, _beginsOfLargestUOT);
}

void
SliceAndDice::performTask()
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  createDiag(false);
  processSliceAndDicePortion(_thUnitNo, _thRoute);
}

void
SliceAndDice::getJourneyDestination()
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  ServiceFeeUtil util(_trx);
  _beginsOfUOT[1] = util.getJourneyDestination(*_farePath,
                                               _journeyDestination,
                                               _isRoundTrip.getValForKey(_farePath->itin()),
                                               _isInternational.getValForKey(_farePath->itin()));
}
void
SliceAndDice::setJourneyDestination()
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  getJourneyDestination();
  _beginsOfUOT[0] = _farePath->itin()->travelSeg().begin();
  if (_beginsOfUOT[1] != _farePath->itin()->travelSeg().end())
    _beginsOfUOT[1]++; // we need next segment after TurnAround
  _beginsOfUOT[2] = _farePath->itin()->travelSeg().end();
  std::copy(_beginsOfUOT, _beginsOfUOT + 3, _beginsOfLargestUOT);
}

void
SliceAndDice::processPortionOfTvl(int unitNo)
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": unitNo=" << unitNo);
  if (!_diag880 && _slice_And_Dice &&
      !ServiceFeeUtil::isPortionOfTravelSelected(_trx, _first, _last))
    return;

  printDiagHeader(unitNo);

  if (!_shopping && !checkAllSegsConfirmed(_beginsOfUOT[unitNo], _beginsOfUOT[unitNo + 1]))
    return;
  processServiceFeesGroups(unitNo);
}

void
SliceAndDice::processServiceFeesGroups(int unitNo)
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": unitNo=" << unitNo);
  OptionalFeeConcurValidator s6Validator(_trx, _farePath);
  bool needS6Validation =
      s6Validator.checkMarkOperCxrs(_marketingCxr[unitNo], _operatingCxr[unitNo], _first, _last);

  typedef std::vector<ServiceFeesGroup*>::const_iterator SrvFeesGrpI;
  for (SrvFeesGrpI srvFeesGrpI = _farePath->itin()->ocFeesGroup().begin();
       srvFeesGrpI != _farePath->itin()->ocFeesGroup().end();
       ++srvFeesGrpI)
  {
    if ((*srvFeesGrpI)->groupDescription().empty())
    {
      printGroupDescriptionEmpty((*srvFeesGrpI)->groupCode());
      continue;
    }
    if ((*srvFeesGrpI)->groupDescription().empty() ||
        (TrxUtil::isUSAirAncillaryActivated(_trx) && _singleFeesGroupValidation &&
         _singleFeesGroupValidation != *srvFeesGrpI))
      continue;
    if (!_trx.diagnostic().diagParamMapItem(Diagnostic::SERVICE_GROUP).empty() &&
        _trx.diagnostic().diagParamMap()[Diagnostic::SERVICE_GROUP] != (*srvFeesGrpI)->groupCode())
      continue;

    if (isTimeOut(unitNo, (*srvFeesGrpI)->groupCode()))
    {
      LOG4CXX_DEBUG(logger,
                    "Leaving SliceAndDice::processServiceFeesGroups(): time out, "
                        << (*srvFeesGrpI)->groupCode() << " group is not processed ");
      break;
    }

    if (TrxUtil::isUSAirAncillaryActivated(_trx))
    {
      if (processMarketDrivenFeesGroup(unitNo, *srvFeesGrpI, s6Validator))
      {
        const boost::lock_guard<boost::mutex> guard(_mutex);
        if (_stopMatchProcess)
          break;
        continue;
      }
    }

    if (!processSingleServiceFeesGroup(unitNo, *srvFeesGrpI, needS6Validation, s6Validator))
      break;
  }
}

bool
SliceAndDice::checkAllSegsConfirmed(std::vector<TravelSeg*>::const_iterator begin,
                                    std::vector<TravelSeg*>::const_iterator end,
                                    bool doDiag) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  std::vector<TravelSeg*>::const_iterator pos = std::find_if(begin, end, IsUnconfirmed());
  if (pos == end)
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning true");
    return true;
  }

  _farePath->itin()->allSegsConfirmed() = false;

  if (doDiag)
  {
    DiagCollector* dc = diag875();
    dc = (dc ? dc : diag877());
    if (dc)
    {
      bool partiallyConfirmed =
          pos != begin || std::find_if(begin, end, std::not1(IsUnconfirmed())) != end;
      if (partiallyConfirmed)
        (*dc) << "***  SOME SECTORS UNCONFIRMED  ***       NOT PROCESSED\n";
      else
        (*dc) << "***  ALL SECTORS UNCONFIRMED  ***        NOT PROCESSED\n";
    }
  }
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning false");
  return false;
}

void
SliceAndDice::updateTravelParams(int unitNo, ServiceFeesGroup* srvFeesGrp, const TSResIt& begIt)
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": unitNo=" << unitNo);
  TSELatencyData metrics(_trx, "OC UPDATE TRAVEL PARAMS");
  if (std::get<MARKET_DRIVEN_STRATEGY>(*begIt) && (*std::get<BEGIN_OF_TVL_PORTION>(*begIt))->isAir())
  {
    AirSeg& seg = static_cast<AirSeg&>(**std::get<BEGIN_OF_TVL_PORTION>(*begIt));
    _merchCrxStrategy = &_singleStrategy;
    _singleStrategy.setPreferedVendor(srvFeesGrp->merchCxrPref()[seg.carrier()]->prefVendor());
  }
  else
    _merchCrxStrategy = &_multipleStrategy;

  _beginsOfUOT[unitNo] = std::get<BEGIN_OF_TVL_PORTION>(*begIt);
  _beginsOfUOT[unitNo + 1] = std::get<END_OF_TVL_PORTION>(*begIt);
  _beginsOfLargestUOT[unitNo] = std::get<BEGIN_OF_TVL_PORTION>(*begIt);
  _beginsOfLargestUOT[unitNo + 1] = std::get<END_OF_TVL_PORTION>(*begIt);
  _first = *_beginsOfUOT[unitNo];
  _last = *(_beginsOfUOT[unitNo + 1] - 1);
  defineMarketingOperatingCarriers();
}

void
SliceAndDice::defineMarketingOperatingCarriers()
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  for (int unitNo = 0; unitNo < 2; unitNo++) // Loop to process portions of tarvel
  {
    _operatingCxr[unitNo].clear();
    _marketingCxr[unitNo].clear();

    std::vector<tse::TravelSeg*>::const_iterator curSeg = _beginsOfUOT[unitNo];
    std::vector<tse::TravelSeg*>::const_iterator endSeg = _beginsOfUOT[unitNo + 1];

    _operatingCxr[unitNo].clear();
    _marketingCxr[unitNo].clear();

    for (; curSeg < endSeg; ++curSeg)
    {
      if ((*curSeg)->segmentType() == Air)
      {
        AirSeg* airSeg = static_cast<AirSeg*>(*curSeg);

        _operatingCxr[unitNo].insert(airSeg->operatingCarrierCode());
        _marketingCxr[unitNo].insert(airSeg->marketingCarrierCode());
      }
    }
  }
}

bool
SliceAndDice::isOperatingInCxrMap(int unitNo, ServiceFeesGroup* srvFeesGrp) const
{
  TSE_ASSERT(srvFeesGrp);
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": unitNo=" << unitNo);
  for (CarrierCode cxr : _operatingCxr[unitNo])
  {
    if (srvFeesGrp->merchCxrPref().end() != srvFeesGrp->merchCxrPref().find(cxr))
    {
      LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning true");
      return true;
    }
  }
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning false");
  return false;
}

bool
SliceAndDice::marketingSameAsOperating(int unitNo) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": unitNo=" << unitNo);
  std::set<CarrierCode>::iterator cxrSt = _operatingCxr[unitNo].begin();

  for (; cxrSt != _operatingCxr[unitNo].end(); ++cxrSt)
  {
    if (*cxrSt != *(_marketingCxr[unitNo].begin()))
    {
      LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning false");
      // Concurrence
      return false;
    }
  }
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning true");
  return true;
}

bool
SliceAndDice::partitionSameAsOperatingAndOcFeeCarrier(int unitNo, const CarrierCode& ocfeeCarrier) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": unitNo=" << unitNo);
  if (!_trx.billing() || _trx.billing()->partitionID().empty())
    return false;
  return (_operatingCxr[unitNo].size() == 1) &&
         (_operatingCxr[unitNo].count(_trx.billing()->partitionID()) != 0) &&
         (_trx.billing()->partitionID() == ocfeeCarrier);
}

void
SliceAndDice::initializeSubCodes(ServiceFeesGroup* srvFeesGroup,
                                 const CarrierCode& carrier,
                                 const ServiceTypeCode& srvTypeCode,
                                 const ServiceGroup& srvGroup) const
{
  TSE_ASSERT(srvFeesGroup);
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  // set in constructor strategy which you want to use.
  boost::bind<void>(
      ServiceFeesGroup::SubCodeInitializer(_trx, _farePath, _first, _last, *_merchCrxStrategy),
      _1,
      _2,
      _3,
      _4,
      _5)(srvFeesGroup, carrier, srvTypeCode, srvGroup, _farePath->itin()->travelDate());
}

bool
SliceAndDice::getOperatingMarketingInd() const
{
  return _carrierStrategy->getOperatingMarketingInd();
}

void
SliceAndDice::processSubCodes(ServiceFeesGroup& srvFeesGrp,
                              const CarrierCode& candCarrier,
                              int unitNo,
                              bool isOneCarrier) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " unitNo=" << unitNo);
  OcValidationContext ctx(_trx, *_farePath->itin(), *_farePath->paxType(), _farePath);
  OptionalServicesValidator optSrvValidator(ctx,
                                            _beginsOfUOT[unitNo],
                                            _beginsOfUOT[unitNo + 1],
                                            _beginsOfLargestUOT[2],
                                            _ts2ss,
                                            _isInternational.getValForKey(_farePath->itin()),
                                            isOneCarrier,
                                            getOperatingMarketingInd(),
                                            diag877());
  optSrvValidator.setMerchStrategy(_merchCrxStrategy);
  processSubCodes(optSrvValidator, srvFeesGrp, candCarrier, unitNo, isOneCarrier);
}

bool
SliceAndDice::isAnyS5Found(const ServiceFeesGroup& group, const CarrierCode& cxr) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  const boost::lock_guard<boost::mutex> guard(group.mutex());
  std::map<const FarePath*, std::vector<OCFees*> >::const_iterator ocFees =
      group.ocFeesMap().find(_farePath);

  if (ocFees == group.ocFeesMap().end() || ocFees->second.empty())
    return false;

  bool found = false;
  for (const auto ocFeesElem : ocFees->second)
  {
    if (ocFeesElem->carrierCode() == cxr &&
        (ocFeesElem->subCodeInfo()->fltTktMerchInd() == FLIGHT_RELATED_SERVICE ||
         ocFeesElem->subCodeInfo()->fltTktMerchInd() == PREPAID_BAGGAGE) &&
        ocFeesElem->travelStart() == _first && ocFeesElem->travelEnd() == _last)
    {
      found = true;
      break;
    }
  }
  return found;
}

bool
SliceAndDice::isSubCodePassed(const ServiceFeesGroup& srvFeesGrp,
                              int unitNo,
                              const ServiceSubTypeCode& subCode,
                              const Indicator& serviceType) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  TravelSeg* first = *_beginsOfLargestUOT[unitNo];
  if (unitNo > 0 && (first->segmentType() == Surface || first->segmentType() == Arunk))
    first = *(_beginsOfLargestUOT[unitNo] + 1);
  TravelSeg* last = *(_beginsOfLargestUOT[unitNo + 1] - 1);
  if (srvFeesGrp.isSubCodePassed(unitNo, _farePath, _first, _last, subCode, serviceType) ||
      ((_first != first || _last != last) &&
       srvFeesGrp.isSubCodePassed(unitNo, _farePath, first, last, subCode, serviceType)))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning true");
    return true;
  }
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning false");
  return false;
}

bool
SliceAndDice::validateS7(const OptionalServicesValidator& validator, OCFees& ocFee, bool stopMatch)
    const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(_trx, "OC VALIDATE S7");
  bool ret = validator.validate(ocFee, stopMatch);
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning " << (ret ? "true" : "false"));
  return ret;
}

bool
SliceAndDice::prePaidBaggageActive() const
{
  if (TrxUtil::isPrepaidBaggageActivated(_trx))
  {
    return true;
  }
  LOG4CXX_DEBUG(logger,
                "Leaving processSubCodes::processSubCodes() - Prepaid Baggage is not Active");
  return false;
}

bool
SliceAndDice::checkNumbersToStopProcessing(bool ret) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": ret=" << ret);
  if (ret && _shopping)
  {
    accumulatePassedServices();
    const boost::lock_guard<boost::mutex> guard(_mutex);
    if (_trx.getOptions()->maxNumberOfOcFeesForItin() > 0 &&
        (_numberOfOcFeesForItin + 1) > _trx.getOptions()->maxNumberOfOcFeesForItin())
    {
      _stopMatchProcess = true;
      LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning false");
      return false;
    }
  }

  if (ret && _needFirstMatchOnly)
  {
    const boost::lock_guard<boost::mutex> guard(_mutex);
    _stopMatchProcess = true;
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning false");
    return false;
  }
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning true");
  return true;
}

bool
SliceAndDice::isTimeOut(int unitNo, ServiceGroup sg) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": unitNo=" << unitNo << ", sg=" << sg);
  if (dynamic_cast<AncillaryPricingTrx*>(&_trx) != nullptr)
  {
    AncRequest* ancReq = static_cast<AncRequest*>(_trx.getRequest());
    if (ancReq && ancReq->ancRequestType() == AncRequest::M70Request)
    {
      if (fallback::reworkTrxAborter(&_trx))
        checkTrxAborted(_trx);
      else
        _trx.checkTrxAborted();
      LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning false");
      return false;
    }
  }

  bool mustHurry = false;
  if (fallback::reworkTrxAborter(&_trx))
    mustHurry = checkTrxMustHurry(_trx);
  else
    mustHurry = _trx.checkTrxMustHurry();
  if (mustHurry)
  {
    {
      const boost::lock_guard<boost::mutex> guard(_mutex);
      _stopMatchProcess = true;
      _timeOut = true;
    }
    if (sg != BLANK_CODE)
      printTimeOutMsg(unitNo, sg);
    else
      printTimeOutMsg(unitNo);
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning true");
    return true;
  }
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning false");
  return false;
}

void
SliceAndDice::processSubCodes(OptionalServicesValidator& optSrvValidator,
                              ServiceFeesGroup& srvFeesGrp,
                              const CarrierCode& candCarrier,
                              int unitNo,
                              bool isOneCarrier) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(_trx, "OC SUBCODE PROCESS");

  Diag875Collector* diag875 = _diag875;
  Diag877Collector* diag877 = _diag877;

  if ((diag875 || diag877) && !ServiceFeeUtil::isPortionOfTravelSelected(_trx, _first, _last))
  {
    _diag875 = nullptr;
    _diag877 = nullptr;
  }

  StateRestorer<Diag875Collector*> diag875Restorer(_diag875, _diag875, diag875);
  StateRestorer<Diag877Collector*> diag877Restorer(_diag877, _diag877, diag877);

  if (!isAnyS5Found(srvFeesGrp, candCarrier))
  {
    printDiagS5NotFound(srvFeesGrp.groupCode(), candCarrier);
    return;
  }

  printS7Header(srvFeesGrp.groupCode(), candCarrier);

  std::vector<OCFees*> threadSafeVector;
  {
    // find matching OCFees
    const boost::lock_guard<boost::mutex> g(srvFeesGrp.mutex());
    std::map<const FarePath*, std::vector<OCFees*> >::const_iterator ocFees =
        srvFeesGrp.ocFeesMap().find(_farePath);

    for (const auto ocFeesElem : ocFees->second)
    {
      if (!(ocFeesElem->carrierCode() == candCarrier && ocFeesElem->travelStart() == _first &&
            ocFeesElem->travelEnd() == _last) ||
          isSubCodePassed(srvFeesGrp,
                          unitNo,
                          ocFeesElem->subCodeInfo()->serviceSubTypeCode(),
                          ocFeesElem->subCodeInfo()->fltTktMerchInd()))
        continue;

      if (!_trx.diagnostic().diagParamMapItem(Diagnostic::SUB_CODE).empty() &&
          _trx.diagnostic().diagParamMap()[Diagnostic::SUB_CODE] !=
              ocFeesElem->subCodeInfo()->serviceSubTypeCode())
        continue;

      threadSafeVector.push_back(ocFeesElem);
    }
  }

  CarrierCode emdValidatingCarrier;
  if (_trx.activationFlags().isEmdForFlightRelatedServiceAndPrepaidBaggage())
  {
    if (_trx.billing() && !_trx.billing()->partitionID().empty())
      emdValidatingCarrier = _trx.billing()->partitionID();
  }

  std::vector<OCFees*>::iterator ocFeesI = threadSafeVector.begin();
  std::vector<OCFees*>::iterator ocFeesE = threadSafeVector.end();
  for (; ocFeesI != ocFeesE; ++ocFeesI)
  {
    {
      const boost::lock_guard<boost::mutex> g(srvFeesGrp.mutex());
      if (isSubCodePassed(srvFeesGrp,
                          unitNo,
                          (*ocFeesI)->subCodeInfo()->serviceSubTypeCode(),
                          (*ocFeesI)->subCodeInfo()->fltTktMerchInd()))
        continue;
    }

    bool ret = false;
    checkDiagForS5Detail((*ocFeesI)->subCodeInfo()); // if DDINFO requested for S5

    if (_trx.activationFlags().isEmdForFlightRelatedServiceAndPrepaidBaggage())
    {
      if (emdValidatingCarrier.empty())
        emdValidatingCarrier = (*ocFeesI)->carrierCode();
    }

    StatusS5Validation rc = validateS5Data((*ocFeesI)->subCodeInfo(), unitNo, emdValidatingCarrier);

    if (PASS_S5 == rc && (*ocFeesI)->failS6())
      rc = FAIL_S6;

    if (TrxUtil::isEmdValidationRelaxedActivated(_trx) &&
        rc == StatusS5Validation::FAIL_S5_EMD_AGREEMENT_CHECK &&
        partitionSameAsOperatingAndOcFeeCarrier(unitNo, (*ocFeesI)->carrierCode()))
    {
      rc = PASS_S5;
    }

    if (PASS_S5 == rc)
    {
      if (validateS7(optSrvValidator, **ocFeesI, _needFirstMatchOnly))
      {
        ret = true;
        const boost::lock_guard<boost::mutex> guard(srvFeesGrp.mutex());
        srvFeesGrp.subCodeMap(unitNo)
            [ServiceFeesGroup::SubCodeMapKey(_farePath,
                                             (*ocFeesI)->subCodeInfo()->serviceSubTypeCode(),
                                             (*ocFeesI)->subCodeInfo()->fltTktMerchInd())]
                .insert(std::make_pair(std::make_pair(_first, _last),
                                       std::make_pair(*ocFeesI, getOperatingMarketingInd())));
      }
      else
        rc = FAIL_S7;
    }
    printDiagS5Info((*ocFeesI)->subCodeInfo(), rc);
    if (!checkNumbersToStopProcessing(ret))
      break;
  }
}

StatusS5Validation
SliceAndDice::validateS5Data(const SubCodeInfo* subCode,
                             int unitNo,
                             const CarrierCode& emdValidatingCarrier) const
{
  TSE_ASSERT(subCode);
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(_trx, "OC VALIDATE S5");
  if (!_trx.ticketingDate().isBetween(subCode->effDate().date(), subCode->discDate().date()))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " FAIL_S5_TKT_DATE");
    return FAIL_S5_TKT_DATE;
  }

  if (!checkFlightRelated(*subCode))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " FAIL_S5_FLIGHT_RELATED");
    return FAIL_S5_FLIGHT_RELATED;
  }

  if (!checkIndCrxInd(subCode->industryCarrierInd()))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " FAIL_S5_INDUSTRY_INDICATOR");
    return FAIL_S5_INDUSTRY_INDICATOR;
  }

  if (!checkDisplayCategory(subCode->displayCat()))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " FAIL_S5_DISPLAY_CATEGORY");
    return FAIL_S5_DISPLAY_CATEGORY;
  }

  if (!checkEMDType(subCode->emdType()))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " FAIL_S5_EMD_TYPE");
    return FAIL_S5_EMD_TYPE;
  }

  if (fallback::fallbackDisableIEMDForGroup99(&_trx))
  {
    if (!checkEMDAgreement_old(subCode->emdType(), unitNo, emdValidatingCarrier))
    {
      LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " FAIL_S5_EMD_AGREEMENT_CHECK");
      return FAIL_S5_EMD_AGREEMENT_CHECK;
    }
  }
  else
  {
    if (!checkEMDAgreement(*subCode, unitNo, emdValidatingCarrier))
    {
      LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " FAIL_S5_EMD_AGREEMENT_CHECK");
      return FAIL_S5_EMD_AGREEMENT_CHECK;
    }
  }

  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " PASS_S5");
  return PASS_S5;
}

bool
SliceAndDice::checkFlightRelated(const SubCodeInfo& subCode) const
{
  return (subCode.fltTktMerchInd() == FLIGHT_RELATED_SERVICE ||
          (subCode.fltTktMerchInd() == PREPAID_BAGGAGE && prePaidBaggageActive()));
}

bool
SliceAndDice::checkIndCrxInd(const Indicator& indCrxInd) const
{
  return _carrierStrategy->checkIndCrxInd(indCrxInd);
}

bool
SliceAndDice::checkDisplayCategory(const ServiceDisplayInd& displayCat) const
{
  return displayCat != DISPLAY_CAT_STORE;
}

bool
SliceAndDice::checkEMDType(const Indicator& emdType) const
{
  return emdType != EMD_TYPE_ELECTRONIC_TKT;
}

ServiceFeesGroup*
SliceAndDice::addNewServiceGroup(const ServiceGroup& srvGroup) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": srvGroup=" << srvGroup);
  ServiceFeesGroup* srvFeesGroup;
  _trx.dataHandle().get(srvFeesGroup);
  srvFeesGroup->initialize(&_trx);
  srvFeesGroup->groupCode() = srvGroup;

  std::vector<ServiceGroupInfo*>::const_iterator allI = _allGroupCodes.begin();
  std::vector<ServiceGroupInfo*>::const_iterator allE = _allGroupCodes.end();
  for (; allI != allE; allI++)
  {
    if ((*allI)->svcGroup() == srvGroup)
    {
      srvFeesGroup->groupDescription() = (*allI)->definition();
    }
  }
  LOG4CXX_DEBUG(logger, "Leaving SliceAndDice::addNewServiceGroup() - " << srvGroup);
  return srvFeesGroup;
}

bool
SliceAndDice::isMktCrxMktDrivenCrx(int unitNo, ServiceFeesGroup* srvFeesGrp) const
{
  TSE_ASSERT(srvFeesGrp);
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": unitNo=" << unitNo);
  return TrxUtil::isUSAirAncillaryActivated(_trx) && !srvFeesGrp->merchCxrPref().empty() &&
         srvFeesGrp->merchCxrPref().end() !=
             srvFeesGrp->merchCxrPref().find(*(_marketingCxr[unitNo].begin()));
}

bool
SliceAndDice::validMerchCxrGroup(const CarrierCode& carrier, const ServiceGroup& srvGroup) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": carrier=" << carrier << ", srvGroup=" << srvGroup);
  std::map<const CarrierCode, std::vector<ServiceGroup*> >::const_iterator it;

  std::map<const CarrierCode, std::vector<ServiceGroup*> >::const_iterator cXrGrpI =
      _cXrGrp.find(carrier);
  if (cXrGrpI != _cXrGrp.end())
  {
    const std::vector<ServiceGroup*>& groups = (*cXrGrpI).second;

    std::vector<ServiceGroup*>::const_iterator it = groups.begin();
    std::vector<ServiceGroup*>::const_iterator itE = groups.end();
    for (; it != itE; ++it)
    {
      LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": checking \"" << **it << "\"<->\"" << srvGroup << "\"");
      if ((**it) == srvGroup)
      {
        LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning true");
        return true;
      }
    }
  }
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning false");
  return false;
}

bool
SliceAndDice::processMarketDrivenFeesGroup(int unitNo,
                                           ServiceFeesGroup* srvFeesGrp,
                                           OptionalFeeConcurValidator& s6Validator)
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(_trx, "OC MARKET DRIVEN PROCESS");
  if (!_slice_And_Dice)
  {
    std::vector<std::tuple<ServiceFeesGroup::TSIt, ServiceFeesGroup::TSIt, bool> > unitsOfTravel;
    srvFeesGrp->collectUnitsOfTravel(unitsOfTravel, _beginsOfUOT[unitNo], _beginsOfUOT[unitNo + 1]);

    if (!unitsOfTravel.empty())
    {
      TSResIt begIt = unitsOfTravel.begin();
      for (; begIt != unitsOfTravel.end(); ++begIt)
      {
        updateTravelParams(unitNo, srvFeesGrp, begIt);

        if (std::get<MARKET_DRIVEN_STRATEGY>(*begIt) || !isOperatingInCxrMap(unitNo, srvFeesGrp))
        {
          bool needS6Validation = s6Validator.checkMarkOperCxrs(
              _marketingCxr[unitNo], _operatingCxr[unitNo], _first, _last);
          printDiagHeader(unitNo);
          if (!processSingleServiceFeesGroup(unitNo, srvFeesGrp, needS6Validation, s6Validator))
            break;
        }

        if (!std::get<MARKET_DRIVEN_STRATEGY>(*begIt))
        {
          _singleFeesGroupValidation = srvFeesGrp;
          _slice_And_Dice = true;
          sliceAndDice(unitNo);
          _slice_And_Dice = false;
          _singleFeesGroupValidation = nullptr;
          const boost::lock_guard<boost::mutex> guard(_mutex);
          if (_stopMatchProcess)
            break;
        }
      }
      _merchCrxStrategy = &_multipleStrategy;
      srvFeesGrp->setSliceAndDiceProcessed();
      setJourneyDestination();
      _first = *_beginsOfUOT[unitNo];
      _last = *(_beginsOfUOT[unitNo + 1] - 1);
      defineMarketingOperatingCarriers();
      printDiagHeader(unitNo);
      LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning true");
      return true;
    }
  }
  else if (srvFeesGrp->isSliceAndDiceProcessed())
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning true");
    return true;
  }

  if (srvFeesGrp->merchCxrPref().empty())
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning false");
    return false;
  }

  bool ret = isOperatingInCxrMap(unitNo, srvFeesGrp);
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning " << (ret ? "true" : "false"));
  return ret;
}
bool
SliceAndDice::processSingleServiceFeesGroup(int unitNo,
                                            ServiceFeesGroup* srvFeesGrp,
                                            bool needS6Validation,
                                            OptionalFeeConcurValidator& s6Validator) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(_trx, "OC SINGLE SERVICE GROUP PROCESS");
  if (!fallback::fallbackNewGroup99Validation(&_trx) &&
      TrxUtil::isRequestFromAS(_trx) &&
      srvFeesGrp->groupCode().equalToConst("99"))
  {
    setStrategy(_partitionCarrierStrategy);
    processServiceFeesGroup99(srvFeesGrp, unitNo);

    const boost::lock_guard<boost::mutex> guard(_mutex);
    if (_stopMatchProcess)
    {
      LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning false");
      return false;
    }

    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning true");
    return true;
  }

  if (isOneMarketingCxr(unitNo)) // single marketing
  {
    setStrategy(_marketingCarrierStrategy);
    if (marketingSameAsOperating(unitNo))
    {
      processServiceFeesGroup(srvFeesGrp, _marketingCxr[unitNo], unitNo);
    }
    const boost::lock_guard<boost::mutex> guard(_mutex);
    if (_stopMatchProcess)
    {
      LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning false");
      return false;
    }
  }

  if (needS6Validation)
  {
    processServiceFeesGroup(srvFeesGrp, unitNo, &s6Validator);
    const boost::lock_guard<boost::mutex> guard(_mutex);
    if (_stopMatchProcess)
    {
      LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning false");
      return false;
    }
  }

  if (isOneOperatingCxr(unitNo)) // single operating
  {
    setStrategy(_operatingCarrierStrategy);
    processServiceFeesGroup(srvFeesGrp, _operatingCxr[unitNo], unitNo);
    const boost::lock_guard<boost::mutex> guard(_mutex);
    if (_stopMatchProcess)
    {
      LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning false");
      return false;
    }
  }
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning true");
  return true;
}

void
SliceAndDice::processServiceFeesGroup(ServiceFeesGroup* srvFeesGrp,
                                      const std::set<CarrierCode>& candidateCarriers,
                                      int unitNo) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  ServiceTypeCode srvTypeCode = "OC"; // temporary static values, we wait for real data from PSS

  typedef std::set<CarrierCode>::const_iterator CandCarrierI;

  for (CandCarrierI candCarrierI = candidateCarriers.begin();
       candCarrierI != candidateCarriers.end();
       ++candCarrierI)
  {
    if (!validMerchCxrGroup(*candCarrierI, srvFeesGrp->groupCode()))
      continue;

    if (!isOneMarketingCxr(unitNo) || getOperatingMarketingInd() ||
        *candCarrierI != *_marketingCxr[unitNo].begin())
      initializeSubCodes(srvFeesGrp, *candCarrierI, srvTypeCode, srvFeesGrp->groupCode());

    processSubCodes(*srvFeesGrp, *candCarrierI, unitNo, candidateCarriers.size() == 1);
    const boost::lock_guard<boost::mutex> guard(_mutex);
    if (_stopMatchProcess)
      break;
  }
}

void
SliceAndDice::processServiceFeesGroup99(ServiceFeesGroup* srvFeesGrp,
                                        int unitNo) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);

  CarrierCode carrier;
  if (!_trx.billing()->validatingCarrier().empty())
  {
    // for group 99 if validating carrier is set use it instead of partition carrier
    carrier = MCPCarrierUtil::swapToActual(&_trx, _trx.billing()->validatingCarrier());
  }
  else
    carrier = MCPCarrierUtil::swapToActual(&_trx, _trx.billing()->partitionID());

  if (!validMerchCxrGroup(carrier, srvFeesGrp->groupCode()))
    return;

  const ServiceTypeCode group99SrvTypeCode = "OC";
  initializeSubCodes(srvFeesGrp, carrier, group99SrvTypeCode, srvFeesGrp->groupCode());
  processSubCodes(*srvFeesGrp, carrier, unitNo, true);
}

void
SliceAndDice::processServiceFeesGroup(ServiceFeesGroup* srvFeesGrp,
                                      int unitNo,
                                      OptionalFeeConcurValidator* s6Validator) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(_trx, "OC SINGLE SERVICE GROUP PROCESS");
  ServiceTypeCode srvTypeCode = "OC"; // temporary static values, we wait for real data from PSS

  setStrategy(_marketingCarrierStrategy);
  // initialize marketing carrier group
  if (_marketingCxr[unitNo].size() == 1 && !isMktCrxMktDrivenCrx(unitNo, srvFeesGrp))
  {
    for (CarrierCode cxr : _marketingCxr[unitNo])
    {
      ServiceFeesGroup* tmpMarketingServGrp = addNewServiceGroup(srvFeesGrp->groupCode());

      if (!validMerchCxrGroup(cxr, tmpMarketingServGrp->groupCode()))
        continue;

      initializeSubCodes(tmpMarketingServGrp, cxr, srvTypeCode, tmpMarketingServGrp->groupCode());

      if (s6Validator->validateS6(
              cxr, _operatingCxr[unitNo], tmpMarketingServGrp, true, srvFeesGrp))
      {
        processSubCodes(*srvFeesGrp, cxr, unitNo, _marketingCxr[unitNo].size() == 1);
        const boost::lock_guard<boost::mutex> guard(_mutex);
        if (_stopMatchProcess)
          break;
      }
    }
  }
  bool thCond = false;
  {
    const boost::lock_guard<boost::mutex> guard(_mutex);
    thCond = !_stopMatchProcess && _operatingCxr[unitNo].size() > 1;
  }

  if (thCond)
  {
    std::vector<CarrierCode> cxrs;
    // initialize all operating carriers
    s6Validator->getOptCarrierVec(
        _marketingCxr[unitNo], _beginsOfUOT[unitNo], _beginsOfUOT[unitNo + 1], cxrs);

    if (cxrs.size() > 0)
      setStrategy(_multipleOperatingCarrierStrategy);
    else
      setStrategy(_operatingCarrierStrategy);

    for (CarrierCode cxr : cxrs)
    {
      ServiceFeesGroup* tmpOperatingServGrp = addNewServiceGroup(srvFeesGrp->groupCode());

      if (!validMerchCxrGroup(cxr, tmpOperatingServGrp->groupCode()))
        continue;

      initializeSubCodes(tmpOperatingServGrp, cxr, srvTypeCode, tmpOperatingServGrp->groupCode());
      if (s6Validator->validateS6(
              cxr, _operatingCxr[unitNo], tmpOperatingServGrp, false, srvFeesGrp))
      {
        processSubCodes(*srvFeesGrp, cxr, unitNo, _operatingCxr[unitNo].size() == 1);
        const boost::lock_guard<boost::mutex> guard(_mutex);
        if (_stopMatchProcess)
          break;
      }
    }
  }
}

void
SliceAndDice::processSliceAndDicePortion(
    int unitNo,
    const std::pair<std::vector<TravelSeg*>::const_iterator,
                    std::vector<TravelSeg*>::const_iterator>& route)
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(_trx, "OC SLICE AND DICE PORTION PROCESS");

  _beginsOfUOT[unitNo] = route.first;
  _beginsOfUOT[unitNo + 1] = route.second;

  defineMarketingOperatingCarriers();

  processPortionOfTravel(unitNo,
                         std::bind1st(std::mem_fun(&SliceAndDice::processPortionOfTvl), this));
}

void
SliceAndDice::processPortionOfTravel(int unitNo, boost::function<void(int)> fun)
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  TSELatencyData metrics(_trx, "OC PORTION PROCESS");
  std::vector<TravelSeg*>::const_iterator firstSeg = _beginsOfUOT[unitNo];
  std::vector<TravelSeg*>::const_iterator endSeg = _beginsOfUOT[unitNo + 1];

  if (firstSeg != endSeg)
  {
    if (std::distance(firstSeg, endSeg) > 1 && !_slice_And_Dice &&
        dynamic_cast<ArunkSeg*>(*_beginsOfUOT[unitNo]))
    {
      _beginsOfUOT[unitNo]++;
      firstSeg = _beginsOfUOT[unitNo];
    }
    _first = *firstSeg;
    _last = *(endSeg - 1);
    fun(unitNo);
  }
}

uint64_t
SliceAndDice::getFilterMask(int unitNo) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  std::vector<TravelSeg*>::const_iterator firstSeg = _beginsOfUOT[unitNo];
  std::vector<TravelSeg*>::const_iterator endSeg = _beginsOfUOT[unitNo + 1];

  uint64_t combFilterMask = 0;
  for (; firstSeg != endSeg; ++firstSeg)
  {
    if (dynamic_cast<ArunkSeg*>(*firstSeg))
    {
      combFilterMask += (1ULL << (std::distance(_beginsOfUOT[unitNo], firstSeg) << 1));
    }
  }
  return combFilterMask;
}

void
SliceAndDice::printDiagHeader(int unitNo) const
{
  if (!ServiceFeeUtil::isPortionOfTravelSelected(_trx, _first, _last))
    return;

  if (diag875() != nullptr)
  {
    Diag875Collector* d875 = diag875();
    d875->printTravelPortionHeader(
        *_first, *_last, _marketingCxr[unitNo], _operatingCxr[unitNo], *_farePath, _trx);

    if (_diagInfo)
      return;

    d875->printS5CommonHeader();
  }
  else if (diag877() != nullptr)
  {
    diag877()->printS7PortionOfTravel(*_first, *_last, *_farePath, _trx);
  }
}

void
SliceAndDice::printDiagS5NotFound(const ServiceGroup& group, const CarrierCode& cxr) const
{
  if (diag875() == nullptr)
    return;
  if (isDDPass())
    return;

  if (!_diagInfo)
  {
    if (fallback::fallbackNewGroup99Validation(&_trx))
      diag875()->printS5NotFound_old(group, cxr, getOperatingMarketingInd());
    else
    {
      if (!_trx.billing()->validatingCarrier().empty() &&
          cxr == _trx.billing()->validatingCarrier() && group == "99")
        diag875()->printS5NotFound(group, cxr, 'V');
      else
        diag875()->printS5NotFound(group, cxr, _carrierStrategy->getCarrierStrategyTypeShort());
    }
  }
}

void
SliceAndDice::printS7Header(const ServiceGroup& group, const CarrierCode& cxr) const
{
  if (diag877() != nullptr)
  {
    Diag877Collector* d877 = diag877();

    if (fallback::fallbackNewGroup99Validation(&_trx))
      d877->printS7GroupCxrService_old(*_farePath, group, cxr, getOperatingMarketingInd());
    else
    {
      if (!_trx.billing()->validatingCarrier().empty() &&
          cxr == _trx.billing()->validatingCarrier() && group == "99")
        d877->printS7GroupCxrService(*_farePath, group, cxr, "VALIDATING");
      else
        d877->printS7GroupCxrService(*_farePath, group, cxr, _carrierStrategy->getCarrierStrategyTypeFull());
    }

    if (!_diagInfo)
      d877->printS7CommonHeader();
  }
}

void
SliceAndDice::checkDiagForS5Detail(const SubCodeInfo* subInfo) const
{
  if (_diagInfo && diag875() != nullptr)
  {
    if (subInfo->fltTktMerchInd() != FLIGHT_RELATED_SERVICE &&
        subInfo->fltTktMerchInd() != TICKET_RELATED_SERVICE &&
        subInfo->fltTktMerchInd() != MERCHANDISE_SERVICE &&
        subInfo->fltTktMerchInd() != RULE_BUSTER_SERVICE &&
        subInfo->fltTktMerchInd() != PREPAID_BAGGAGE)
      return;

    if (fallback::fallbackNewGroup99Validation(&_trx))
      diag875()->printDetailS5Info_old(subInfo, getOperatingMarketingInd());
    else
      diag875()->printDetailS5Info(subInfo, _carrierStrategy->getCarrierStrategyTypeShort());
  }
}

void
SliceAndDice::printDiagS5Info(const SubCodeInfo* sCInfo, const StatusS5Validation& rc) const
{
  if (diag875() != nullptr)
  {
    if ((rc == FAIL_S5_TKT_DATE || rc == FAIL_S5_FLIGHT_RELATED) &&
        (sCInfo->fltTktMerchInd() != FLIGHT_RELATED_SERVICE &&
         sCInfo->fltTktMerchInd() != TICKET_RELATED_SERVICE &&
         sCInfo->fltTktMerchInd() != MERCHANDISE_SERVICE &&
         sCInfo->fltTktMerchInd() != RULE_BUSTER_SERVICE &&
         sCInfo->fltTktMerchInd() != PREPAID_BAGGAGE))
      return;

    if (isDDPass() && rc != PASS_S5)
      return;

    if (!_diagInfo)
    {
      if (fallback::fallbackNewGroup99Validation(&_trx))
        diag875()->printS5SubCodeInfo_old(sCInfo, getOperatingMarketingInd(), rc);
      else
      {
        if (!_trx.billing()->validatingCarrier().empty() &&
            sCInfo->carrier() == _trx.billing()->validatingCarrier() && sCInfo->serviceGroup() == "99")
          diag875()->printS5SubCodeInfo(sCInfo, 'V', rc);
        else
          diag875()->printS5SubCodeInfo(sCInfo, _carrierStrategy->getCarrierStrategyTypeShort(), rc);
      }
    }
    else
    {
      diag875()->printS5SubCodeStatus(rc);
      diag875()->lineSkip(0);
    }
  }
}

void
SliceAndDice::printTimeOutMsg(int unitNo, ServiceGroup sg) const
{
  if (!_diag875 && !_diag877 && !_diag880)
    return;

  char buf[5];
  sprintf(buf, "%d", unitNo);
  std::stringstream oStr;
  if (sg != BLANK_CODE)
    oStr << "TIME OUT OCCURED ON BEFORE START " << sg << " PROCESS\n" << std::endl;
  else
    oStr << "TIME OUT OCCURED ON " << buf << " PORTION FOR SLICE/DICE PROCESS\n" << std::endl;

  if (_diag875)
    *_diag875 << oStr.str();
  else if (_diag877)
    *_diag877 << oStr.str();
  else
    *_diag880 << oStr.str();
}

void
SliceAndDice::createDiag(bool header)
{
  if (!_trx.diagnostic().isActive())
    return;

  DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();
  if (diagType == Diagnostic875 || diagType == Diagnostic877 || diagType == Diagnostic880)
  {
    DiagCollector* diag = DCFactory::instance()->create(_trx);

    if (diagType == Diagnostic877 && _trx.diagnostic().diagParamMapItem("TA") == "X")
      return;

    if (diag != nullptr)
      diag->enable(Diagnostic875, Diagnostic877, Diagnostic880);
    else
      return;
    if (header)
      diag->printHeader();

    if (diagType == Diagnostic875)
      _diag875 = dynamic_cast<Diag875Collector*>(diag);
    else if (diagType == Diagnostic877)
    {
      _diag877 = dynamic_cast<Diag877Collector*>(diag);
      if (header)
        _diag877->printS7Banner();
    }
    else if (diagType == Diagnostic880)
      _diag880 = dynamic_cast<Diag880Collector*>(diag);

    const std::string& diagDD = _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL);
    if (diagDD == "INFO")
      _diagInfo = true;
  }
}

void
SliceAndDice::reclaimDiag(const SliceAndDice& sd)
{
  if (_diag875 && sd.diag875())
    *_diag875 << sd.diag875()->str();

  if (_diag877 && sd.diag877())
    *_diag877 << sd.diag877()->str();

  if (_diag880 && sd.diag880())
    *_diag880 << sd.diag880()->str();
}

void
SliceAndDice::printGroupDescriptionEmpty(const ServiceGroup sg) const
{
  if (diag875() == nullptr)
    return;
  if (!_diagInfo)
    diag875()->displayGroupDescriptionEmpty(sg);
}

bool
SliceAndDice::isDDPass() const
{
  return _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "PASSED";
}

bool
SliceAndDice::checkEMDAgreement(const SubCodeInfo& subCode,
                                int unitNo,
                                const CarrierCode& emdValidatingCarrier) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  if (!ServiceFeeUtil::isPerformEMDCheck(_trx, subCode))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning true");
    return true;
  }

  if (subCode.emdType() != EMD_TYPE_ASSOCIATED_TKT && subCode.emdType() != EMD_TYPE_STANDALONE)
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning true");
    return true;
  }

  if (emdValidatingCarrier.empty())
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " Missing emdValidatingCarrier - returning false");
    return false;
  }

  OCEmdDataProvider ocEmdDataProvider;
  ocEmdDataProvider.emdValidatingCarrier() = emdValidatingCarrier;
  ocEmdDataProvider.operatingCarriers() = _operatingCxr[unitNo];
  ocEmdDataProvider.marketingCarriers() = _marketingCxr[unitNo];

  EmdValidator emdValidator(_trx, ocEmdDataProvider, diag875());
  bool ret = emdValidator.validate(
      const_cast<std::map<CarrierCode, std::vector<EmdInterlineAgreementInfo*> >&>(
          _carrierEmdInfoMap),
      _mutex,
      const_cast<NationCode&>(_nation));
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning " << (ret ? "true" : "false"));
  return ret;
}

bool
SliceAndDice::checkEMDAgreement_old(const Indicator& emdType,
                                int unitNo,
                                const CarrierCode& emdValidatingCarrier) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  if (!ServiceFeeUtil::isPerformEMDCheck_old(_trx))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning true");
    return true;
  }

  if (emdType != EMD_TYPE_ASSOCIATED_TKT && emdType != EMD_TYPE_STANDALONE)
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning true");
    return true;
  }

  if (emdValidatingCarrier.empty())
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " Missing emdValidatingCarrier - returning false");
    return false;
  }

  OCEmdDataProvider ocEmdDataProvider;
  ocEmdDataProvider.emdValidatingCarrier() = emdValidatingCarrier;
  ocEmdDataProvider.operatingCarriers() = _operatingCxr[unitNo];
  ocEmdDataProvider.marketingCarriers() = _marketingCxr[unitNo];

  EmdValidator emdValidator(_trx, ocEmdDataProvider, diag875());
  bool ret = emdValidator.validate(
      const_cast<std::map<CarrierCode, std::vector<EmdInterlineAgreementInfo*> >&>(
          _carrierEmdInfoMap),
      _mutex,
      const_cast<NationCode&>(_nation));
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << ": " << __LINE__ << " returning " << (ret ? "true" : "false"));
  return ret;
}

} // tse

