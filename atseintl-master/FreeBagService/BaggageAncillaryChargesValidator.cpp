//-------------------------------------------------------------------
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
#include "FreeBagService/BaggageAncillaryChargesValidator.h"

#include "Common/CurrencyRoundingUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/FreeBaggageUtil.h"
#include "Common/Money.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/RepricingTrx.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SvcFeesCurrencyInfo.h"
#include "Diagnostic/Diag852Collector.h"
#include "FreeBagService/AncillaryResultProcessorUtils.h"
#include "FreeBagService/BaggageAncillarySecurityValidator.h"
#include "FreeBagService/BaggageAncillarySecurityValidatorAB240.h"

#include <boost/foreach.hpp>

namespace tse
{
FALLBACK_DECL(ab240FixSsdmps171);
FALLBACK_DECL(fallbackSkipRecordsWithOverlappingOccurenceForAB240);
FALLBACK_DECL(fallbackFdoAndSdoBaggageDefinedByPiecesOnly);
FALLBACK_DECL(fallbackAb240SkipOccurencesAfterInfinity);

class AlreadyMatchedSequences
{
public:
  bool operator()(const OptionalServicesInfo* rec) const
  {
    return _alreadyMatchedS7sSequences.find(rec->seqNo()) != _alreadyMatchedS7sSequences.end();
  }
  void registerNextMatchedS7SequenceNo(uint32_t nextMatchedS7SequenceNo)
  {
    _alreadyMatchedS7sSequences.insert(nextMatchedS7SequenceNo);
  }

private:
  std::set<uint32_t> _alreadyMatchedS7sSequences;
};

class CurrentTicketingDateScopeSetter
{
public:
  CurrentTicketingDateScopeSetter(PricingTrx& trx) : _trx(trx)
  {
    if (_trx.getBaggagePolicy().changeTktDateForAncillaryCharges())
    {
      _ticketingDate = _trx.ticketingDate();
      _trx.ticketingDate() = DateTime::localTime();
    }
  }

  ~CurrentTicketingDateScopeSetter()
  {
    if (_trx.getBaggagePolicy().changeTktDateForAncillaryCharges())
    {
      _trx.ticketingDate() = _ticketingDate;
    }
  }

private:
  PricingTrx& _trx;
  DateTime _ticketingDate;
};

bool
BaggageAncillaryChargesValidator::ByWeightFilter::select(const OptionalServicesInfo& s7)
{
  const int32_t weight = s7.baggageWeight();

  if (weight == 0)
  {
    return true;
  }
  else if (weight == -1)
  {
    if (!_blankWeight)
    {
      _blankWeight = true;
      return true;
    }
  }
  else if (_maxWeight == 0 || weight > _maxWeight)
  {
    _maxWeight = weight;
    return true;
  }
  return false;
}


bool
BaggageAncillaryChargesValidator::ByOccurrenceFilter::select(const OptionalServicesInfo& s7)
{
  const bool firstPcBlank =
      s7.baggageOccurrenceFirstPc() == -1 || s7.baggageOccurrenceFirstPc() == 0;
  const bool lastPcBlank = s7.baggageOccurrenceLastPc() == -1 || s7.baggageOccurrenceLastPc() == 0;
  const bool blankBlank = firstPcBlank && lastPcBlank;

  switch (_state)
  {
  case MATCHING_FIRST:
    return matchFirst(s7, lastPcBlank, blankBlank);

  case MATCHING_BLANKBLANK:
    return matchBlankBlank(blankBlank);

  case MATCHING_SEQUENCE:
    return matchSequence(s7, firstPcBlank, lastPcBlank, blankBlank);

  default:
    return false;
  }
}

bool
BaggageAncillaryChargesValidator::ByOccurrenceFilter::matchFirst(const OptionalServicesInfo& s7,
                                                                 const bool lastPcBlank,
                                                                 const bool blankBlank)
{
  if (blankBlank)
    _state = SKIP;
  else
  {
    if (lastPcBlank)
      _state = MATCHING_BLANKBLANK;
    else
    {
      _prevLastPc = s7.baggageOccurrenceLastPc();
      _state = MATCHING_SEQUENCE;
    }
  }
  return true;
}

bool
BaggageAncillaryChargesValidator::ByOccurrenceFilter::matchBlankBlank(const bool blankBlank)
{
  if (blankBlank)
  {
    _state = SKIP;
    return true;
  }
  return false;
}

bool
BaggageAncillaryChargesValidator::ByOccurrenceFilter::matchSequence(const OptionalServicesInfo& s7,
                                                                    const bool firstPcBlank,
                                                                    const bool lastPcBlank,
                                                                    const bool blankBlank)
{
  if (firstPcBlank)
  {
    _state = SKIP;

    if (lastPcBlank)
      return true;
  }
  else if (s7.baggageOccurrenceFirstPc() == _prevLastPc + 1)
  {
    _prevLastPc = s7.baggageOccurrenceLastPc();

    if (lastPcBlank)
      _state = MATCHING_BLANKBLANK;

    return true;
  }
  _state = MATCHING_BLANKBLANK;

  return false;
}

BaggageAncillaryChargesValidator::ByOccurrenceFilterAB240::ByOccurrenceFilterAB240(const PricingTrx* trx)
  : _trx(trx)
{
  assert(_trx != nullptr);
}

bool
BaggageAncillaryChargesValidator::ByOccurrenceFilterAB240::matchFirst(const OptionalServicesInfo& s7,
                                                                      const bool lastPcBlank,
                                                                      const bool blankBlank)
{
  if (blankBlank)
    _state = SKIP;
  else
  {
    if (lastPcBlank)
    {
      if (fallback::fallbackAb240SkipOccurencesAfterInfinity(_trx))
        _state = MATCHING_BLANKBLANK;
      else
        _state = SKIP;
    }
    else
    {
      _prevLastPc = s7.baggageOccurrenceLastPc();
      _state = MATCHING_SEQUENCE;
    }
  }
  return true;
}

bool
BaggageAncillaryChargesValidator::ByOccurrenceFilterAB240::matchSequence(const OptionalServicesInfo& s7,
                                                                         const bool firstPcBlank,
                                                                         const bool lastPcBlank,
                                                                         const bool blankBlank)
{
  if (firstPcBlank)
  {
    _state = SKIP;

    if (lastPcBlank)
      return true;
  }
  else if (s7.baggageOccurrenceFirstPc() == _prevLastPc + 1)
  {
    _prevLastPc = s7.baggageOccurrenceLastPc();

    if (lastPcBlank)
    {
      if (fallback::fallbackAb240SkipOccurencesAfterInfinity(_trx))
        _state = MATCHING_BLANKBLANK;
      else
        _state = SKIP;
    }

    return true;
  }

  // Don't switch to MATCHING_BLANKBLANK state when processing AB240 request -
  // just skip this record and continue serching for the next matching one

  return false;
}

BaggageAncillaryChargesValidator::DisclosureFinder::DisclosureFinder(
    PricingTrx& trx, const BaggageTravel& baggageTravel)
  : _trx(trx), _baggageTravel(baggageTravel)
{
  if (_trx.activationFlags().isAB240() &&
      trx.getOptions()->isOcOrBaggageDataRequested(RequestedOcFeeGroup::DisclosureData) &&
      !isAllowanceInWeight())
  {
    if (getFreeBaggagePcs() < 2)
    {
      _lookForFdo = true;
      _lookForSdo = true;

      if (getFreeBaggagePcs() == 1)
        _lookForSdoOnly = true;
    }
  }
}

void
BaggageAncillaryChargesValidator::DisclosureFinder::processBaggageCharge(const OptionalServicesInfo& optSrv, BaggageCharge& baggageCharge)
{
  if (!_lookForFdo && !_lookForSdo)
    return;

  if (baggageCharge.subCodeInfo()->fltTktMerchInd() == BAGGAGE_CHARGE &&
      FreeBaggageUtil::S5MatchLogic::isSecondConditionOk(baggageCharge.subCodeInfo()))
  {
    const bool matching1stBag = !_lookForSdoOnly && matchOccurrence(optSrv, 1);
    const bool matching2ndBag = matchOccurrence(optSrv, _lookForSdoOnly ? 1 : 2);

    if (_lookForFdo && matching1stBag)
    {
      _lookForFdo = false;
      baggageCharge.setMatched1stBag(true);
    }

    if (_lookForSdo && matching2ndBag)
    {
      _lookForSdo = false;
      baggageCharge.setMatched2ndBag(true);
    }
  }
}

int32_t
BaggageAncillaryChargesValidator::DisclosureFinder::getFreeBaggagePcs()
{
  if (_baggageTravel._allowance && _baggageTravel._allowance->optFee())
    return _baggageTravel._allowance->optFee()->freeBaggagePcs();

  return 0;
}

bool
BaggageAncillaryChargesValidator::DisclosureFinder::isAllowanceInWeight()
{
  if (_baggageTravel._allowance &&
      _baggageTravel._allowance->optFee() &&
      _baggageTravel._allowance->optFee()->freeBaggagePcs() == -1 &&
      _baggageTravel._allowance->optFee()->baggageWeight() != -1)
    return true;

  return false;
}

BaggageAncillaryChargesValidator::S7Filter*
BaggageAncillaryChargesValidator::selectS7Filter(const ServiceSubTypeCode& subTypeCode)
{
  const AncRequest* request(nullptr);
  if (_isAncillaryBaggage)
    request = static_cast<const AncRequest*>(_trx.getRequest());
  BaggageAncillaryChargesValidator::S7Filter* filter = nullptr;
  if (!request || (request->selectFirstChargeForOccurrence() && request->hardMatchIndicator()))
  {
    if (subTypeCode.equalToConst("0DG"))
      filter = &_byWeightFilter;
    else if (_trx.activationFlags().isAB240() && !fallback::fallbackSkipRecordsWithOverlappingOccurenceForAB240(&_trx))
      filter = &_byOccurrenceFilterAB240;
    else
      filter = &_byOccurrenceFilter;

    filter->reset();
  }
  return filter;
}

bool BaggageAncillaryChargesValidator::skipBaggageDefinedByWeightForAB240(const OptionalServicesInfo& optSrv) const
{
  return _trx.activationFlags().isAB240() && optSrv.baggageWeightUnit() != CHAR_BLANK;
}

void
BaggageAncillaryChargesValidator::validate(const SubCodeInfo& subCodeInfo,
                                           ChargeVector& matchedCharges)
{
  if (_diag && !isDDInfo())
    (static_cast<Diag852Collector*>(_diag))->printS7CommonHeader();

  const std::vector<OptionalServicesInfo*>& optSrvInfos = getOptionalServicesInfo(subCodeInfo);

  if (optSrvInfos.empty())
    printDiagS7NotFound(subCodeInfo);

  DisclosureFinder disclosureFinder(_trx, _baggageTravel);

  S7Filter* filter = selectS7Filter(subCodeInfo.serviceSubTypeCode());

  BaggageCharge* baggageCharge(nullptr);
  bool baggageChargeUsed(true);

  AlreadyMatchedSequences alreadyMachedSequence;

  for (OptionalServicesInfo* optSrv : optSrvInfos)
  {
    if (isCancelled(*optSrv) || alreadyMachedSequence(optSrv))
      continue;

    if (baggageChargeUsed)
    {
      baggageCharge = buildCharge(subCodeInfo);
      baggageChargeUsed = false;
    }
    else
      baggageCharge->cleanOutCurrentSeg();

    bool markAsSelected = false;

    checkDiagS7ForDetail(optSrv);
    StatusS7Validation rc = validateS7Data(*optSrv, *baggageCharge);
    if (rc == PASS_S7)
    {
      if (retrieveSpecifiedFee(*optSrv, *baggageCharge))
      {
        alreadyMachedSequence.registerNextMatchedS7SequenceNo(optSrv->seqNo());

        if (baggageCharge->isAnyS7SoftPass())
          rc = SOFT_PASS_S7;

        if (filter)
        {
          if (filter->select(*optSrv))
          {
            supplementAndAppendCharge(subCodeInfo, baggageCharge, matchedCharges);
            baggageChargeUsed = true;
            markAsSelected = true;
          }
        }
        else
        {
          supplementAndAppendCharge(subCodeInfo, baggageCharge, matchedCharges);
          baggageChargeUsed = true;
        }
      }
      else
        rc = FAIL_S7_SFC_T170;
    }
    if (baggageChargeUsed)
    {
      if (!fallback::fallbackFdoAndSdoBaggageDefinedByPiecesOnly(&_trx))
      {
        if (!skipBaggageDefinedByWeightForAB240(*optSrv))
          disclosureFinder.processBaggageCharge(*optSrv, *baggageCharge);
      }
      else
      {
        disclosureFinder.processBaggageCharge(*optSrv, *baggageCharge);
      }
    }

    OptionalServicesValidator::printDiagS7Info(optSrv, *baggageCharge, rc, markAsSelected);
    if (checkForPrintFareInfo(*optSrv))
      (static_cast<Diag852Collector*>(_diag))->printFareInfo(_fareForFee, _trx);
  }
}

bool
BaggageAncillaryChargesValidator::checkForPrintFareSelectionInfo(const OptionalServicesInfo& s7)
    const
{
  return _diag && isDDInfo() && ServiceFeeUtil::isFeeFarePercentage(s7);
}

bool
BaggageAncillaryChargesValidator::checkForPrintFareInfo(const OptionalServicesInfo& s7) const
{
  return _diag && isDDInfo() && ServiceFeeUtil::isFeeFarePercentage(s7) && _fareForFee &&
         isDiagSequenceMatch(s7);
}

StatusS7Validation
BaggageAncillaryChargesValidator::validateS7Data(OptionalServicesInfo& optSrvInfo, OCFees& ocFees)
{
  StatusS7Validation status = BaggageChargesValidator::validateS7Data(optSrvInfo, ocFees);
  if (status == PASS_S7)
  {
    if (!checkAdvPurchaseTktInd(optSrvInfo))
      return FAIL_S7_ADV_PURCHASE_TKT_IND;

    if (!checkAdvPur(optSrvInfo, ocFees))
      return FAIL_S7_ADVPUR;

    if (!checkFreeBagPieces(optSrvInfo))
      return FAIL_S7_FREE_BAG_PIECES;

    if (!checkCollectSubtractFee(optSrvInfo))
      return FAIL_S7_COLLECT_SUBTRACT;

    if (!checkNetSellFeeAmount(optSrvInfo))
      return FAIL_S7_NET_SELL;

    if (!checkTaxApplication(optSrvInfo))
      return FAIL_S7_TAX_APPLICATION;

    if (!checkAvailability(optSrvInfo))
      return FAIL_S7_AVAILABILITY;

    if (!checkRuleBusterRuleMatchingFareClass(optSrvInfo))
      return FAIL_S7_RULE_BUSTER_RMFC;

    if (!checkPassengerOccurrence(optSrvInfo))
      return FAIL_S7_PAX_OCCURRENCE;
  }
  return status;
}

bool
BaggageAncillaryChargesValidator::checkAdvPurchaseTktInd(const OptionalServicesInfo& optSrvInfo)
    const
{
  return optSrvInfo.advPurchTktIssue() == CHAR_BLANK;
}

bool
BaggageAncillaryChargesValidator::checkFreeBagPieces(const OptionalServicesInfo& optSrvInfo) const
{
  return optSrvInfo.freeBaggagePcs() < 0;
}

StatusS7Validation
BaggageAncillaryChargesValidator::checkServiceNotAvailNoCharge(const OptionalServicesInfo& info,
                                                               OCFees& /*ocFees*/) const
{
  switch (info.notAvailNoChargeInd())
  {
  case CHAR_BLANK:
  case SERVICE_NOT_AVAILABLE:
  case SERVICE_FREE_NO_EMD_ISSUED:
  case SERVICE_FREE_EMD_ISSUED:
  case SERVICE_FREE_NO_BOOK_NO_EMD:
  case SERVICE_FREE_NO_BOOK_EMD_ISSUED:
    return PASS_S7;
    break;
  default:
    return FAIL_S7_NOT_AVAIL_NO_CHANGE;
    break;
  }
}

bool
BaggageAncillaryChargesValidator::checkFeeApplication(const OptionalServicesInfo& optSrvInfo) const
{
  if (optSrvInfo.notAvailNoChargeInd() == CHAR_BLANK)
  {
    if (optSrvInfo.serviceFeesCurrencyTblItemNo() == 0)
    {
      return ((optSrvInfo.frequentFlyerMileageAppl() == 'C') ||
              (optSrvInfo.frequentFlyerMileageAppl() == 'P') ||
              (optSrvInfo.frequentFlyerMileageAppl() == 'H'));
    }
    else
    {
      return ((optSrvInfo.frequentFlyerMileageAppl() == '3') ||
              (optSrvInfo.frequentFlyerMileageAppl() == '4') ||
              (optSrvInfo.frequentFlyerMileageAppl() == 'K') ||
              (optSrvInfo.frequentFlyerMileageAppl() == 'F'));
    }
  }
  else
  {
    return ((optSrvInfo.serviceFeesCurrencyTblItemNo() == 0) &&
            (optSrvInfo.frequentFlyerMileageAppl() == CHAR_BLANK) &&
            (optSrvInfo.applicationFee() == 0));
  }
}

bool
BaggageAncillaryChargesValidator::checkOccurrence(const OptionalServicesInfo& optSrvInfo) const
{
  return !(optSrvInfo.baggageOccurrenceFirstPc() <= 0 && optSrvInfo.baggageOccurrenceLastPc() > 0);
}

const Loc&
BaggageAncillaryChargesValidator::getLocForSfcValidation() const
{
  if (_isUsDot)
  {
    return BaggageChargesValidator::getLocForSfcValidation();
  }
  else
  {
    return *(_trx.getRequest()->ticketingAgent()->agentLocation());
  }
}

bool
BaggageAncillaryChargesValidator::shouldProcessAdvPur(const OptionalServicesInfo& /*info*/) const
{
  return true;
}

bool
BaggageAncillaryChargesValidator::checkBaggageWeightUnit(const OptionalServicesInfo& /*optSrvInfo*/)
    const
{
  return true;
}

bool
BaggageAncillaryChargesValidator::checkTaxApplication(const OptionalServicesInfo& optSrvInfo) const
{
  return optSrvInfo.taxInclInd() == 'X' || optSrvInfo.taxInclInd() == CHAR_BLANK;
}

bool
BaggageAncillaryChargesValidator::checkAvailability(const OptionalServicesInfo& optSrvInfo) const
{
  return optSrvInfo.availabilityInd() == 'Y' || optSrvInfo.availabilityInd() == 'N';
}

bool
BaggageAncillaryChargesValidator::checkRuleBusterRuleMatchingFareClass(
    const OptionalServicesInfo& optSrvInfo) const
{
  return optSrvInfo.ruleBusterFcl().empty();
}

bool
BaggageAncillaryChargesValidator::checkPassengerOccurrence(const OptionalServicesInfo& optSrvInfo)
    const
{
  return optSrvInfo.firstOccurence() == 0 && optSrvInfo.lastOccurence() == 0;
}

bool
BaggageAncillaryChargesValidator::retrieveSpecifiedFee(const OptionalServicesInfo& optSrvInfo,
                                                       OCFees& ocFees)
{
  CurrentTicketingDateScopeSetter setter(_trx);

  if (ServiceFeeUtil::isFeeFarePercentage(optSrvInfo))
  {
    if (!_fareForFeeAlreadyQueried)
    {
      _fareForFee = getFare(optSrvInfo);

      _fareForFeeAlreadyQueried = true;
    }
    if (_fareForFee)
    {
      const MoneyAmount farePercentageAmount =
          getFarePercentageAmount(_fareForFee->fareAmount(), optSrvInfo);

      SvcFeesCurrencyInfo sfcInfo;
      sfcInfo.feeAmount() = farePercentageAmount;
      sfcInfo.currency() = _fareForFee->currency();
      sfcInfo.noDec() = _fareForFee->numDecimal();
      setOCFees(optSrvInfo, ocFees, sfcInfo);

      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return OptionalServicesValidator::retrieveSpecifiedFee(optSrvInfo, ocFees);
  }
}

const FareMarket*
BaggageAncillaryChargesValidator::getFareMarket(const CarrierCode& carrier) const
{
  AirSeg* airSeg(nullptr);
  _trx.dataHandle().get(airSeg);
  if (!airSeg)
    return nullptr;

  airSeg->carrier() = carrier;
  airSeg->origin() = (*_baggageTravel.getTravelSegBegin())->origin();
  airSeg->destination() = (*(_baggageTravel.getTravelSegEnd() - 1))->destination();
  airSeg->boardMultiCity() = (*_baggageTravel.getTravelSegBegin())->boardMultiCity();
  airSeg->offMultiCity() = (*(_baggageTravel.getTravelSegEnd() - 1))->offMultiCity();
  airSeg->departureDT() = (*_baggageTravel.getTravelSegBegin())->departureDT();
  airSeg->arrivalDT() = (*(_baggageTravel.getTravelSegEnd() - 1))->arrivalDT();

  std::vector<TravelSeg*> travelSegs;
  travelSegs.push_back(airSeg);

  RepricingTrx* repricingTrx = TrxUtil::reprice(
      _trx, travelSegs, FMDirection::UNKNOWN, true, &carrier, nullptr, ADULT, false, false, 'I', 0, true);
  if (repricingTrx)
    return repricingTrx->fareMarket().front();
  else
    return nullptr;
}

const PaxTypeFare*
BaggageAncillaryChargesValidator::getFare(const OptionalServicesInfo& optSrvInfo) const
{
  AirSeg* airSeg(nullptr);
  _trx.dataHandle().get(airSeg);
  if (!airSeg)
    return nullptr;

  airSeg->carrier() = optSrvInfo.carrier();
  airSeg->origin() = (*_baggageTravel.getTravelSegBegin())->origin();
  airSeg->destination() = (*(_baggageTravel.getTravelSegEnd() - 1))->destination();
  airSeg->boardMultiCity() = (*_baggageTravel.getTravelSegBegin())->boardMultiCity();
  airSeg->offMultiCity() = (*(_baggageTravel.getTravelSegEnd() - 1))->offMultiCity();
  airSeg->departureDT() = (*_baggageTravel.getTravelSegBegin())->departureDT();
  airSeg->arrivalDT() = (*(_baggageTravel.getTravelSegEnd() - 1))->arrivalDT();

  std::vector<TravelSeg*> travelSegs = {airSeg};

  RepricingTrx* repricingTrx = TrxUtil::reprice(_trx,
                                                travelSegs,
                                                FMDirection::UNKNOWN,
                                                true,
                                                &optSrvInfo.carrier(),
                                                nullptr,
                                                ADULT,
                                                false,
                                                false,
                                                'I',
                                                0,
                                                true);
  const PaxTypeFare* retFare = getFareFromTrx(repricingTrx);
  if (!retFare)
  {
    CarrierCode overrideCarrier("YY");
    repricingTrx = TrxUtil::reprice(_trx,
                                    travelSegs,
                                    FMDirection::UNKNOWN,
                                    true,
                                    &overrideCarrier,
                                    nullptr,
                                    ADULT,
                                    false,
                                    false,
                                    'I',
                                    0,
                                    true);
    retFare = getFareFromTrx(repricingTrx);
  }

  if (repricingTrx)
  {
    FareMarket* fareMarket = repricingTrx->fareMarket().front();

    if (checkForPrintFareSelectionInfo(optSrvInfo))
      (static_cast<Diag852Collector*>(_diag))
          ->printFareSelectionInfo(*fareMarket, optSrvInfo.carrier(), _trx, retFare);
  }
  return retFare;
}

const PaxTypeFare*
BaggageAncillaryChargesValidator::getFareFromTrx(RepricingTrx* repricingTrx) const
{
  if (repricingTrx)
  {
    FareMarket* fareMarket = repricingTrx->fareMarket().front();
    BOOST_REVERSE_FOREACH (const PaxTypeFare* paxTypeFare,
                           std::make_pair(fareMarket->ow_begin(), fareMarket->ow_end()))
    {
      if (checkFare(paxTypeFare))
        return paxTypeFare;
    }
  }
  return nullptr;
}

bool
BaggageAncillaryChargesValidator::checkFare(const PaxTypeFare* paxTypeFare) const
{
  return (paxTypeFare->tcrTariffCat() != RuleConst::PRIVATE_TARIFF) && paxTypeFare->isPublished() &&
         paxTypeFare->isNormal() && (paxTypeFare->fcaFareType()[0] == E_TYPE) &&
         (paxTypeFare->directionality() == FROM) &&
         (paxTypeFare->paxType()->paxType().empty() || paxTypeFare->paxType()->paxType() == ADULT);
}

MoneyAmount
BaggageAncillaryChargesValidator::getFarePercentageAmount(MoneyAmount amount,
                                                          const OptionalServicesInfo& info) const
{
  float percentAmount;
  switch (info.frequentFlyerMileageAppl())
  {
  case 'H':
    percentAmount = 0.5;
    break;
  case 'C':
    percentAmount = 1.0;
    break;
  case 'P':
    percentAmount = 1.5;
    break;
  default:
    percentAmount = 0;
    break;
  }
  return (percentAmount / 100.0) * amount;
}

bool
BaggageAncillaryChargesValidator::checkSecurity(const OptionalServicesInfo& optSrvInfo) const
{
  if (_isAncillaryBaggage)
  {
    if (optSrvInfo.serviceFeesSecurityTblItemNo() <= 0)
      return optSrvInfo.publicPrivateInd() != T183_SCURITY_PRIVATE;

    if (fallback::ab240FixSsdmps171(&_trx))
    {
      BaggageAncillarySecurityValidator securityValidator(
          static_cast<AncillaryPricingTrx&>(_trx),
          _segI,
          _segIE,
          _t183TableBuffer.bufferingT183TableDiagRequired(),
          true);
      bool view = false;
      return securityValidator.validate(optSrvInfo.seqNo(),
                                        optSrvInfo.serviceFeesSecurityTblItemNo(),
                                        view,
                                        optSrvInfo.vendor(),
                                        _t183TableBuffer.getT183DiagBuffer());
    }
    else
    {
      if (_trx.activationFlags().isAB240())
      {
        BaggageAncillarySecurityValidatorAB240 securityValidator(
            static_cast<AncillaryPricingTrx&>(_trx),
            _segI,
            _segIE,
            _t183TableBuffer.bufferingT183TableDiagRequired(),
            BAGGAGE_CHARGE);
        bool view = false;
        return securityValidator.validate(optSrvInfo.seqNo(),
                                          optSrvInfo.serviceFeesSecurityTblItemNo(),
                                          view,
                                          optSrvInfo.vendor(),
                                          _t183TableBuffer.getT183DiagBuffer());
      }
      else
      {
        BaggageAncillarySecurityValidator securityValidator(
            static_cast<AncillaryPricingTrx&>(_trx),
            _segI,
            _segIE,
            _t183TableBuffer.bufferingT183TableDiagRequired(),
            true);
        bool view = false;
        return securityValidator.validate(optSrvInfo.seqNo(),
                                          optSrvInfo.serviceFeesSecurityTblItemNo(),
                                          view,
                                          optSrvInfo.vendor(),
                                          _t183TableBuffer.getT183DiagBuffer());
      }

    }
  }
  else
  {
    return BaggageAllowanceValidator::checkSecurity(optSrvInfo);
  }
}

void
BaggageAncillaryChargesValidator::filterSegments()
{
  if(_trx.activationFlags().isAB240() && !_isUsDot )
  {
    _segI = std::find_if(_segI, _segIE, CheckPortionOfTravelIndicator('T'));
    _segIE = std::find_if_not(_segI, _segIE, CheckPortionOfTravelIndicator('T'));
  }
}
} // tse
