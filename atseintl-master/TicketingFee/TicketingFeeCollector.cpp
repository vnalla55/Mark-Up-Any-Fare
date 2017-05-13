//-------------------------------------------------------------------
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

#include "TicketingFee/TicketingFeeCollector.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/FreqFlyerUtils.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/OBFeesUtils.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseUtil.h"
#include "DataModel/DifferentialData.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/BankIdentificationInfo.h"
#include "DBAccess/Currency.h"
#include "DBAccess/Customer.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/PaxTypeMatrix.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag870Collector.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcConsts.h"
#include "Rules/RuleUtil.h"
#include "TicketingFee/SecurityValidator.h"
#include "TicketingFee/SvcFeesAccountCodeValidator.h"
#include "TicketingFee/SvcFeesTktDesigValidator.h"

#include <boost/bind.hpp>

#include <list>

namespace tse
{

Logger
TicketingFeeCollector::_logger("atseintl.FareCalc.TicketingFeeCollector");

const Indicator TicketingFeeCollector::T183_SCURITY_PRIVATE = 'P';
const Indicator TicketingFeeCollector::CREDIT = 'C';
const Indicator TicketingFeeCollector::DEBIT = 'D';
const Indicator TicketingFeeCollector::BLANK = ' ';

TicketingFeeCollector::TicketingFeeCollector(PricingTrx* trx, FarePath* farePath)
  : _trx(trx),
    _farePath(farePath),
    _diag(nullptr),
    _diagInfo(false),
    _journeyTurnAroundPoint(nullptr),
    _journeyTurnAroundTs(nullptr),
    _journeyDestination(nullptr),
    _segmentOrderTurnAround(initialInvalidValue),
    _stopS4Processing(false),
    _stopS4ProcessingFCA(false),
    _stopS4ProcessingFDA(false),
    _binExists(false),
    _processFop(false),
    _process2CC(false),
    _cardType(BLANK),
    _secondCardType(BLANK),
    _halfRoundTripSplit(false)
{
}

TicketingFeeCollector::~TicketingFeeCollector()
{
}

void
TicketingFeeCollector::collect()
{
  std::vector<FopBinNumber> fopBinNumberVector;

  createDiag();
  setJourneyDestination();
  printDiagHeader();
  setCardType();
  _processFop = isProcessFOP(fopBinNumberVector);

  if (LIKELY(_trx->getRequest()->isCollectOBFee()))
  {
    printDiagHeaderFopBinNumber(fopBinNumberVector);
    printDiagHeaderAmountFopResidualInd();
  }

  printDiagHeader1();

  StatusS4Validation rc = isValidTrx();

  std::vector<TicketingFeesInfo*> fopMatched;
  initializeFor2CreditCards(fopMatched);

  if (rc != PASS_S4)
  {
    printCanNotCollect(rc);
    swapObFeesVecFor2CC(fopMatched);
    return;
  }

  bool isCarrierActive = false;
  // We will not collect F type fees if carrier is not activated
  // R & T type fees aren't dependent on carrier activation
  if (serviceFeeActiveForCarrier())
    isCarrierActive = true;

  DateTime emptyDate;
  if (_trx->getRequest()->originBasedRTPricing() && _trx->outboundDepartureDate() != emptyDate)
  {
    // This is the Inbound leg of 1/2 round trip for old calendar/branded fares
    _halfRoundTripSplit = true;
  }

  const std::vector<TicketingFeesInfo*>& ticketingFeeInfo = getTicketingFee();
  if (ticketingFeeInfo.empty())
  {
    printDiagS4NotFound();
    endDiag();
    swapObFeesVecFor2CC(fopMatched);
    return;
  }

  bool isShopping = ShoppingUtil::isShoppingTrx(*_trx);
  Money highestAmount(NUC);

  // called only for its side effect to set properly _binExists
  // bad design. should be refactored
  getCardType(_trx->getRequest()->formOfPayment());

  TicketingFeesInfo* processFOPFeeInfo = *(ticketingFeeInfo.begin());

  for (TicketingFeesInfo* feeInfo : ticketingFeeInfo)
  {
    checkDiagDetailedRequest(feeInfo);
    OBFeeSubType subType = getOBFeeSubType(feeInfo);
    switch (subType)
    {
    case OBFeeSubType::OB_F_TYPE:
      if (!_stopS4Processing && isCarrierActive)
      {
        processFType(rc, isShopping, highestAmount, feeInfo, fopBinNumberVector, fopMatched);
        processFOPFeeInfo = feeInfo;
      }
      break;
    case OBFeeSubType::OB_R_TYPE:
    case OBFeeSubType::OB_T_TYPE:
      processRTType(rc, feeInfo);
      break;
    default:
      break;
    }
  } // for each tfee

  if (_processFop)
  {
    if (fopMatched.empty() && _stopS4Processing)
      fopMatched.push_back(processFOPFeeInfo);
    if (fopMatched.empty())
      printFopBinNotFound();

    _farePath->collectedTktOBFees().swap(fopMatched);
  }

  endDiag();

  sortOBFeeVectors(_farePath->collectedRTypeOBFee());
  sortOBFeeVectors(_farePath->collectedTTypeOBFee());
}

void
TicketingFeeCollector::processFType(StatusS4Validation& rc,
                                    bool isShopping,
                                    Money& highestAmount,
                                    TicketingFeesInfo* feeInfo,
                                    std::vector<FopBinNumber>& fopBinNumberVector,
                                    std::vector<TicketingFeesInfo*>& fopMatched)
{
  bool stopFOP = false;

  try
  {
    if (validateSequence(feeInfo, rc))
    {
      setCurrencyNoDec(*feeInfo);

      // Here is pricing logic
      if (_processFop)
      {
        if (_process2CC ? addFopBinIfMatch(*feeInfo, fopMatched)
                        : isFopBinMatch(*feeInfo, fopBinNumberVector))
        {
          if (!_process2CC)
          {
            fopMatched.push_back(feeInfo);
          }
          else
          {
            if (_cardType == _secondCardType || feeInfo->serviceSubTypeCode() == "FDA")
            {
              _stopS4ProcessingFCA = true;
              _stopS4ProcessingFDA = true;
              stopFOP = true;
            }
            else
            {
              if (feeInfo->serviceSubTypeCode() == "FCA")
              {
                _stopS4ProcessingFCA = true;
              }
            }
          }

          if (fopBinNumberVector.empty())
            stopFOP = true;
          if (!_diag)
          {
            if (stopFOP)
              return;
          }
          else
            rc = PASS_FOP;
        }
        else if (!(feeInfo)->fopBinNumber().empty())
          rc = FAIL_FOP_BIN;
      }
      else
      {
        if (checkDuplicatedRecords(feeInfo))
          rc = DUPLICATE_S4;
        else
          ticketingFeesInfoFromFP(feeInfo).push_back(feeInfo);
      }
    } // validate SEQ
    printDiagS4Info(feeInfo, rc);
  }
  catch (...)
  {
    printDiagS4Info(feeInfo, PROCESSING_ERROR);
  }
}

bool
TicketingFeeCollector::isProcessFOP(std::vector<FopBinNumber>& fopBin)
{
  bool isFop = false;
  if (LIKELY(ShoppingUtil::isShoppingTrx(*_trx)))
    return isFop;
  const FopBinNumber& singleFOP = _trx->getRequest()->formOfPayment();
  if (!singleFOP.empty())
  {
    isFop = singleFOP.size() == FOP_BIN_SIZE &&
            std::find_if(singleFOP.begin(), singleFOP.end(), !boost::bind<bool>(&isDigit, _1)) ==
                singleFOP.end();
  }

  if (isFop && !_trx->getRequest()->secondFormOfPayment().empty())
  {
    if (_trx->isProcess2CC())
    {
      _process2CC = true;
      if (BLANK == _cardType || BLANK == _secondCardType)
      {
        isFop = false;
        _process2CC = false;
      }
    }
    else
      isFop = false;
  }

  if (isFop)
    getFopBinNumber(fopBin);
  return isFop;
}

bool
TicketingFeeCollector::serviceFeeActiveForCarrier()
{
  const CarrierCode& cxr = validatingCarrier();
  if (!_trx->dataHandle().isCxrActivatedForServiceFee(cxr, _trx->ticketingDate()))
  {
    printCxrNotValid(cxr);
    return false;
  }
  return true;
}

bool
TicketingFeeCollector::isSvcTypeMatchCardType(const ServiceSubTypeCode& svcSubCode,
                                              Indicator cardType) const
{
  return ((ANY_CREDIT == svcSubCode && CREDIT == cardType) ||
          (ANY_DEBIT == svcSubCode && DEBIT == cardType));
}

bool
TicketingFeeCollector::isFopBinMatch(TicketingFeesInfo& feeInfo, std::vector<FopBinNumber>& fopBins)
    const
{
  auto it = std::find_if(fopBins.begin(),
                         fopBins.end(),
                         [&feeInfo](const FopBinNumber& fopBin)
                         { return feeInfo.compareFopBin(fopBin); });

  if (it != fopBins.end())
  {
    fopBins.erase(it);
    return true;
  }
  return false;
}

bool
TicketingFeeCollector::addFopBinIfMatch(TicketingFeesInfo& feeInfo,
                                        std::vector<TicketingFeesInfo*>& fopMatched)
{
  FopBinNumber& firstFopBin = _trx->getRequest()->formOfPayment();
  FopBinNumber& secondFopBin = _trx->getRequest()->secondFormOfPayment();

  if (isSvcTypeMatchCardType(feeInfo.serviceSubTypeCode(), _cardType) &&
      (feeInfo.compareFopBin(firstFopBin) || feeInfo.fopBinNumber().empty()))
  {
    fopMatched[0] = &feeInfo;
    if (_cardType == _secondCardType &&
        (feeInfo.compareFopBin(secondFopBin) || feeInfo.fopBinNumber().empty()))
      fopMatched[1] = &feeInfo;
    return true;
  }
  if (isSvcTypeMatchCardType(feeInfo.serviceSubTypeCode(), _secondCardType) &&
      (feeInfo.compareFopBin(secondFopBin) || feeInfo.fopBinNumber().empty()))
  {
    fopMatched[1] = &feeInfo;
    if (_cardType == _secondCardType &&
        (feeInfo.compareFopBin(firstFopBin) || feeInfo.fopBinNumber().empty()))
      fopMatched[0] = &feeInfo;
    return true;
  }

  return false;
}

std::vector<TicketingFeesInfo*>&
TicketingFeeCollector::ticketingFeesInfoFromFP(TicketingFeesInfo* feeInfo)
{

  OBFeeSubType subType = getOBFeeSubType(feeInfo);
  switch (subType)
  {
  case OBFeeSubType::OB_F_TYPE:
    return _farePath->collectedTktOBFees();
    break;
  case OBFeeSubType::OB_R_TYPE:
    return _farePath->collectedRTypeOBFee();
    break;
  case OBFeeSubType::OB_T_TYPE:
    return _farePath->collectedTTypeOBFee();
    break;
  default:
    TSE_ASSERT(0);
    break;
  }
  return _farePath->collectedTktOBFees();
}

bool
TicketingFeeCollector::checkDuplicatedRecords(TicketingFeesInfo* feeI)
{
  if (getOBFeeSubType(feeI) == OBFeeSubType::OB_UNKNOWN)
    return false; // Do nothing for unknown type
  std::vector<TicketingFeesInfo*> tfiVec = ticketingFeesInfoFromFP(feeI);
  std::vector<TicketingFeesInfo*>::const_iterator validTktFee = tfiVec.begin();

  for (; validTktFee != tfiVec.end(); ++validTktFee)
  {
    if (feeI->serviceSubTypeCode() == (*validTktFee)->serviceSubTypeCode())
    {
      if (LIKELY((getOBFeeSubType(feeI) == OBFeeSubType::OB_F_TYPE)))
      {
        if ((*validTktFee)->compareFopBin(feeI->fopBinNumber()))
          return true;
      }
      else
        return true;
    }
  }
  return false;
}

OBFeeSubType
TicketingFeeCollector::getOBFeeSubType(const TicketingFeesInfo* feeInfo) const
{
  return TrxUtil::getOBFeeSubType(feeInfo->serviceSubTypeCode());
}

StatusS4Validation
TicketingFeeCollector::isValidTrx() const
{
  if (_trx == nullptr || _farePath == nullptr)
    return INTERNAL_ERROR;

  if (_trx->getRequest()->ticketingAgent()->agentTJR() != nullptr &&
      _trx->getRequest()->ticketingAgent()->agentTJR()->doNotApplyObTktFees() == YES)
    return TJR_NOT_APPLY_TKTFEE;

  if (validatingCarrier().empty())
    return VALIDATING_CXR_EMPTY;

  // all open segments
  if (ItinUtil::getFirstValidTravelSeg(_farePath->itin()) == nullptr)
    return ALL_SEGS_OPEN;

  if (!atleastOneSegmentConfirm())
    return NOT_ALL_SEGS_CONFIRM;

  if (OBFeesUtils::fallbackObFeesWPA(_trx))
  {
    if (isAnyNonCreditCardFOP())
      return NON_CREDIT_CARD_FOP;
  }
  else
  {
    if (_trx->isAnyNonCreditCardFOP())
      return NON_CREDIT_CARD_FOP;
  }

  return PASS_S4;
}

bool
TicketingFeeCollector::atleastOneSegmentConfirm() const
{
  const auto& tvlSegs = _farePath->itin()->travelSeg();
  return std::any_of(tvlSegs.cbegin(),
                     tvlSegs.cend(),
                     [](const TravelSeg* tvlSeg)
                     { return (LIKELY(tvlSeg->resStatus() == CONFIRM_RES_STATUS)); });
}

const CarrierCode&
TicketingFeeCollector::validatingCarrier() const
{
  if (_trx->isValidatingCxrGsaApplicable())
  {
    return _farePath->defaultValidatingCarrier();
  }
  else
  {
    if (UNLIKELY(!_trx->getRequest()->validatingCarrier().empty()))
      return _trx->getRequest()->validatingCarrier();
    return _farePath->itin()->validatingCarrier();
  }
}

const std::vector<TicketingFeesInfo*>&
TicketingFeeCollector::getTicketingFee() const
{
  if (LIKELY(!_halfRoundTripSplit))
  {
    return _trx->dataHandle().getTicketingFees(
        ATPCO_VENDOR_CODE, validatingCarrier(), _farePath->itin()->travelDate());
  }
  else
  {
    // Use Outbound travel date to find the fees we want
    return _trx->dataHandle().getTicketingFees(
        ATPCO_VENDOR_CODE, validatingCarrier(), _trx->outboundDepartureDate());
  }
}

bool
TicketingFeeCollector::validateSequence(const TicketingFeesInfo* feeInfo, StatusS4Validation& rc)
{
  rc = PASS_S4;

  if (UNLIKELY(!checkServiceType(feeInfo)))
  {
    rc = FAIL_SVC_TYPE;
    return false;
  }

  if (!checkFeeTypesEnabled(feeInfo, rc))
    return false;

  if (!checkServiceSubType(feeInfo))
  {
    rc = FAIL_SVC_TYPE;
    return false;
  }

  if (!checkTicketingDates(feeInfo))
  {
    rc = FAIL_TKT_DATE;
    return false;
  }

  if (!checkPaxType(feeInfo))
  {
    rc = FAIL_PAX_TYPE;
    return false;
  }

  if (!checkFareBasis(feeInfo))
  {
    rc = FAIL_FARE_BASIS;
    return false;
  }

  if (!checkAccountCode(feeInfo))
  {
    rc = FAIL_ACC_T172;
    return false;
  }
  if (UNLIKELY(!checkTktDesignator(feeInfo)))
  {
    rc = FAIL_TDSG_T173;
    return false;
  }
  if (!checkSecurity(feeInfo))
  {
    rc = FAIL_SECUR_T183;
    return false;
  }
  if (!validateGeoLoc1Loc2(feeInfo) || !validateGeoVia(feeInfo) || !validateGeoWhlWithin(feeInfo))
  {
    rc = FAIL_ON_GEO_LOC;
    return false;
  }
  checkFopBinNumber(feeInfo);

  return true;
}

bool
TicketingFeeCollector::checkFeeTypesEnabled(const TicketingFeesInfo* feeInfo,
                                            StatusS4Validation& rc)
{
  OBFeeSubType type = getOBFeeSubType(feeInfo);
  if (type == OBFeeSubType::OB_R_TYPE && !_trx->getRequest()->isCollectRTypeOBFee())
  {
    rc = R_TYPE_NOT_REQUIRED;
    return false;
  }
  if (type == OBFeeSubType::OB_T_TYPE && !_trx->getRequest()->isCollectTTypeOBFee())
  {
    rc = T_TYPE_NOT_REQUIRED;
    return false;
  }
  if (UNLIKELY(type == OBFeeSubType::OB_F_TYPE && !_trx->getRequest()->isCollectOBFee()))
  {
    rc = F_TYPE_NOT_REQUIRED;
    return false;
  }
  return true;
}

void
TicketingFeeCollector::checkFopBinNumber(const TicketingFeesInfo* feeInfo)
{
  if (feeInfo->fopBinNumber().empty())
  {
    if (feeInfo->serviceSubTypeCode() == ANY_CREDIT)
    {
      _stopS4ProcessingFCA = true;
    }
    if (feeInfo->serviceSubTypeCode() == ANY_DEBIT)
    {
      _stopS4ProcessingFDA = true;
    }

    if (((_stopS4ProcessingFCA && _stopS4ProcessingFDA)) || (_binExists && !_process2CC) ||
        (_process2CC && _cardType == _secondCardType))
    {
      // when bin exists we don't look for both FCA and FDA, hence we can stop
      _stopS4Processing = true;
    }
  }
}

bool
TicketingFeeCollector::checkTicketingDates(const TicketingFeesInfo* feeInfo) const
{
  if (UNLIKELY(feeInfo->ticketEffDate().isInfinity() && feeInfo->ticketDiscDate().isInfinity()))
    return true;
  // string dateTimeStr =  _trx->ticketingDate().dateToString(YYYYMMDD, "-") + " 00:00:00.000";
  // DateTime ticketingDate(dateTimeStr);
  DateTime ticketingDate(_trx->ticketingDate().date(), boost::posix_time::time_duration(0, 0, 0));

  if (LIKELY(!feeInfo->ticketEffDate().isInfinity()))
  {
    // dateTimeStr = feeInfo->ticketEffDate().dateToString(YYYYMMDD, "-") + " 00:00:00.000";
    // DateTime effDate(dateTimeStr);
    DateTime effDate(feeInfo->ticketEffDate().date(), boost::posix_time::time_duration(0, 0, 0));
    if (ticketingDate < effDate)
      return false;
  }

  if (!feeInfo->ticketDiscDate().isInfinity())
  {
    // dateTimeStr = feeInfo->ticketDiscDate().dateToString(YYYYMMDD, "-") + " 00:00:00.000";
    // DateTime discDate(dateTimeStr);
    DateTime discDate(feeInfo->ticketDiscDate().date(), boost::posix_time::time_duration(0, 0, 0));
    if (ticketingDate > discDate)
      return false;
  }

  return true;
}

bool
TicketingFeeCollector::checkServiceType(const TicketingFeesInfo* feeInfo) const
{
  return (feeInfo->serviceTypeCode().equalToConst("OB"));
}

bool
TicketingFeeCollector::checkServiceSubType(const TicketingFeesInfo* feeInfo) const
{
  OBFeeSubType type = getOBFeeSubType(feeInfo);
  if (UNLIKELY(type == OBFeeSubType::OB_R_TYPE))
    return true;
  if (UNLIKELY(type == OBFeeSubType::OB_T_TYPE))
    return true;

  if (_stopS4ProcessingFCA && feeInfo->serviceSubTypeCode() == ANY_CREDIT)
  {
    return false;
  }
  if (UNLIKELY(_stopS4ProcessingFDA && feeInfo->serviceSubTypeCode() == ANY_DEBIT))
  {
    return false;
  }
  if (LIKELY(!_processFop)) // collect FCA and FDA
  {
    if (LIKELY(feeInfo->serviceSubTypeCode().equalToConst("FDA") || feeInfo->serviceSubTypeCode().equalToConst("FCA")))
      return true;
  }

  if (_process2CC)
  {
    if (_cardType != _secondCardType)
      return (feeInfo->serviceSubTypeCode() == ANY_DEBIT ||
              feeInfo->serviceSubTypeCode() == ANY_CREDIT);
  }

  if (DEBIT == _cardType)
  {
    return (feeInfo->serviceSubTypeCode() == ANY_DEBIT);
  }
  else if (_binExists)
  {
    // BIN was passed in but not a 'Debit', collect only FCA
    return (feeInfo->serviceSubTypeCode() == ANY_CREDIT);
  }
  return (feeInfo->serviceSubTypeCode() == ANY_DEBIT ||
          feeInfo->serviceSubTypeCode() == ANY_CREDIT);
}

bool
TicketingFeeCollector::checkPaxType(const TicketingFeesInfo* tktFeesInfo) const
{
  const PaxTypeCode& farePathPtc = _farePath->paxType()->paxType();

  if (!tktFeesInfo->paxType().empty())
  {
    if (farePathPtc != tktFeesInfo->paxType()) // not exact match
    {
      if (matchSabrePaxList(farePathPtc, tktFeesInfo->paxType()))
      {
        return true;
      }
      return (paxTypeMappingMatch(*_farePath->paxType(), tktFeesInfo->paxType()));
    }
  }

  return true;
}

bool
TicketingFeeCollector::checkAccountCode(const TicketingFeesInfo* feeInfo) const
{
  if (feeInfo->svcFeesAccCodeTblItemNo() != 0)
  {
    SvcFeesAccountCodeValidator validator(*_trx, (_diag && _diagInfo) ? _diag : nullptr);
    return validator.validate(feeInfo->svcFeesAccCodeTblItemNo());
  }
  return true;
}

bool
TicketingFeeCollector::checkTktDesignator(const TicketingFeesInfo* feeInfo)
{
  if (UNLIKELY(feeInfo->svcFeesTktDsgnTblItemNo() != 0))
  {
    SvcFeesInputTktDesigValidator validator(
        *_trx, *_farePath->itin(), (_diag && _diagInfo) ? _diag : nullptr);
    return svcFeesTktDesignatorValidate(validator, feeInfo->svcFeesTktDsgnTblItemNo());
  }
  return true;
}

bool
TicketingFeeCollector::svcFeesTktDesignatorValidate(const SvcFeesTktDesigValidator& validator,
                                                    int itemNo) const
{
  return validator.validate(itemNo);
}

bool
TicketingFeeCollector::checkSecurity(const TicketingFeesInfo* feeInfo) const
{
  if (feeInfo->svcFeesSecurityTblItemNo() <= 0)
  {
    return (feeInfo->publicPrivateInd() != T183_SCURITY_PRIVATE);
  }
  bool view = false;

  SecurityValidator securityValidator(
      *_trx, _farePath->itin()->travelSeg().begin(), _farePath->itin()->travelSeg().end());
  bool isValid =
      securityValidator.validate(feeInfo->seqNo(), feeInfo->svcFeesSecurityTblItemNo(), view);

  return isValid;
}
bool
TicketingFeeCollector::matchSabrePaxList(const PaxTypeCode& farePathPtc,
                                         const PaxTypeCode& tktFeePtc) const
{
  std::list<PaxTypeCode> ptcList;

  const std::vector<const PaxTypeMatrix*>& sabrePaxTypes = getSabrePaxTypes(farePathPtc);

  std::transform(sabrePaxTypes.begin(),
                 sabrePaxTypes.end(),
                 back_inserter(ptcList),
                 [](const PaxTypeMatrix* ptm)
                 { return ptm->atpPaxType(); });

  return std::find(ptcList.begin(), ptcList.end(), tktFeePtc) != ptcList.end();
}

const std::vector<const PaxTypeMatrix*>&
TicketingFeeCollector::getSabrePaxTypes(const PaxTypeCode& farePathPtc) const
{
  return _trx->dataHandle().getPaxTypeMatrix(farePathPtc);
}

namespace
{
inline bool
isPaxTypeInMap(const std::map<CarrierCode, std::vector<PaxType*>*>& actualPaxType,
               const std::map<CarrierCode, std::vector<PaxType*>*>::const_iterator actualPaxTypeIt,
               const PaxTypeCode paxTypeCode)
{
  if (actualPaxTypeIt == actualPaxType.end())
    return false;
  const auto& paxTypes = actualPaxTypeIt->second;
  return (std::any_of(paxTypes->cbegin(),
                      paxTypes->cend(),
                      [paxTypeCode](const PaxType* paxType)
                      { return (paxType->paxType() == paxTypeCode); }));
}
}

bool
TicketingFeeCollector::paxTypeMappingMatch(const PaxType& paxChk, const PaxTypeCode paxTypeCode)
    const
{
  const auto& actualPaxTypes = paxChk.actualPaxType();
  const auto valCarrierIt = actualPaxTypes.find(validatingCarrier());
  const auto anyCarrierIt = actualPaxTypes.find(ANY_CARRIER);

  return (isPaxTypeInMap(actualPaxTypes, valCarrierIt, paxTypeCode) ||
          isPaxTypeInMap(actualPaxTypes, anyCarrierIt, paxTypeCode));
}

bool
TicketingFeeCollector::checkFareBasis(const TicketingFeesInfo* tktFeesInfo)
{
  Indicator tktFareInd = tktFeesInfo->fareInd();

  if (UNLIKELY(tktFareInd == TKT_FREQ_FLYER))
  {
    return checkFrequentFlyer(tktFeesInfo);
  }
  else if (tktFareInd == TKT_ALL)
  {
    return checkFareBasisTktIndAll(tktFeesInfo);
  }
  else if (tktFareInd == TKT_ANY)
  {
    return checkFareBasisTktIndAny(tktFeesInfo);
  }

  return true;
}

bool
TicketingFeeCollector::checkFrequentFlyer(const TicketingFeesInfo* tktFeesInfo)
{
  if (!TrxUtil::isFrequentFlyerTierOBActive(*_trx) || _trx->getRequest()->isSpecificAgencyText())
    return false;

  const ServiceSubTypeCode& subTypeCode = tktFeesInfo->serviceSubTypeCode();
  if (subTypeCode != "FCA" && subTypeCode != "FDA")
    return false;

  const uint16_t minFreqFlyerTierLevel = *tktFeesInfo->fareBasis().begin() - '0';
  // fareBasis field contains frequent flyer level
  if (!_frequentFlyerTierLevel)
  {
    const uint16_t determinedFFLevel = freqflyerutils::determineFreqFlyerTierLevel(
        nullptr, _farePath->paxType()->freqFlyerTierWithCarrier(), validatingCarrier(), _trx);
    _frequentFlyerTierLevel.reset(determinedFFLevel);
  }
  return _frequentFlyerTierLevel.get() <= minFreqFlyerTierLevel;
}

bool
TicketingFeeCollector::checkFareBasisTktIndAll(const TicketingFeesInfo* tktFeesInfo) const
{
  const FareBasisCode tktFareBasis = tktFeesInfo->fareBasis();
  const CarrierCode tktPrimCxr =
      (tktFeesInfo->primaryFareCarrier().empty() ? validatingCarrier()
                                                 : tktFeesInfo->primaryFareCarrier());

  for (PricingUnit* pu : _farePath->pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      const PaxTypeFare& paxTypeFare = *fu->paxTypeFare();
      const std::vector<TravelSeg*>& travelSeg = fu->travelSeg();
      CarrierCode ptfCxr = paxTypeFare.carrier();

      if (isIndustryFare(paxTypeFare))
        ptfCxr = paxTypeFare.fareMarket()->governingCarrier();

      if (!(ptfCxr == tktPrimCxr) ||
          !matchFareBasis(tktFareBasis.c_str(), paxTypeFare, travelSeg) ||
          !matchDiffFareBasisTktIndAll(fu, tktFareBasis.c_str(), tktPrimCxr))
        return false;
    }
  }

  return true;
}

bool
TicketingFeeCollector::checkFareBasisTktIndAny(const TicketingFeesInfo* tktFeesInfo) const
{
  FareBasisCode tktFareBasis = tktFeesInfo->fareBasis();
  CarrierCode tktPrimCxr =
      (tktFeesInfo->primaryFareCarrier().empty() ? validatingCarrier()
                                                 : tktFeesInfo->primaryFareCarrier());
  for (PricingUnit* pu : _farePath->pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      const PaxTypeFare& paxTypeFare = *fu->paxTypeFare();
      const std::vector<TravelSeg*>& travelSeg = fu->travelSeg();
      CarrierCode ptfCxr = paxTypeFare.carrier();
      if (isIndustryFare(paxTypeFare))
        ptfCxr = paxTypeFare.fareMarket()->governingCarrier();

      if ((ptfCxr == tktPrimCxr) && matchFareBasis(tktFareBasis.c_str(), paxTypeFare, travelSeg))
        return true;

      if (matchDiffFareBasisTktIndAny(fu, tktFareBasis.c_str(), tktPrimCxr))
        return true;
    }
  }

  return false;
}

bool
TicketingFeeCollector::matchDiffFareBasisTktIndAll(const FareUsage* fu,
                                                   std::string tktFareBasis,
                                                   const CarrierCode tktPrimCxr) const
{
  if (!fu->differentialPlusUp().empty())
  {
    for (std::vector<DifferentialData*>::const_iterator diffI = fu->differentialPlusUp().begin(),
                                                        diffIEnd = fu->differentialPlusUp().end();
         diffI != diffIEnd;
         ++diffI)
    {
      DifferentialData* diff = *diffI;
      if (diff == nullptr)
        continue;

      const std::vector<TravelSeg*>& travelSeg = fu->travelSeg();

      if ((diff->status() == DifferentialData::SC_PASSED ||
           diff->status() == DifferentialData::SC_CONSOLIDATED_PASS) &&
          diff->amount())
      {
        const PaxTypeFare& diffPtf = *diff->fareHigh();
        CarrierCode diffCxr = diffPtf.carrier();

        if (isIndustryFare(diffPtf))
          diffCxr = diffPtf.fareMarket()->governingCarrier();

        if (!(diffCxr == tktPrimCxr) || !matchFareBasis(tktFareBasis.c_str(), diffPtf, travelSeg))
          return false;
      }
    }
  }

  return true;
}

bool
TicketingFeeCollector::matchDiffFareBasisTktIndAny(const FareUsage* fu,
                                                   std::string tktFareBasis,
                                                   const CarrierCode tktPrimCxr) const
{
  if (!fu->differentialPlusUp().empty())
  {
    for (const DifferentialData* diff : fu->differentialPlusUp())
    {
      if (diff == nullptr)
        continue;

      const std::vector<TravelSeg*>& travelSeg = fu->travelSeg();

      if ((diff->status() == DifferentialData::SC_PASSED ||
           diff->status() == DifferentialData::SC_CONSOLIDATED_PASS) &&
          diff->amount())
      {
        const PaxTypeFare& diffPtf = *diff->fareHigh();
        CarrierCode diffCxr = diffPtf.carrier();

        if (isIndustryFare(diffPtf))
          diffCxr = diffPtf.fareMarket()->governingCarrier();

        if ((diffCxr == tktPrimCxr) && matchFareBasis(tktFareBasis.c_str(), diffPtf, travelSeg))
          return true;
      }
    }
  }

  return false;
}

bool
TicketingFeeCollector::isIndustryFare(const PaxTypeFare& paxTypeFare) const
{
  return (paxTypeFare.fare()->isIndustry() || paxTypeFare.carrier() == INDUSTRY_CARRIER);
}

bool
TicketingFeeCollector::matchFareBasis(std::string tktFareBasis,
                                      const PaxTypeFare& paxTypeFare,
                                      const std::vector<TravelSeg*>& travelSeg) const
{
  const TravelSeg* lastAirSeg = TravelSegUtil::lastAirSeg(travelSeg);
  if (UNLIKELY(lastAirSeg == nullptr))
    return false;

  std::string ptfFareBasis = getS1FareBasisCode(paxTypeFare, lastAirSeg);
  size_t tktFareBasisSize = tktFareBasis.size();
  size_t ptfFareBasisSize = ptfFareBasis.size();

  if (UNLIKELY(ptfFareBasis.empty()))
    return false;

  if (tktFareBasis.empty())
    return true;

  // length of the tkt fare basis before the first '/'
  std::string::size_type tktFbcLength = tktFareBasis.find("/");
  std::string::size_type ptfFbcLength = ptfFareBasis.find("/");

  // if no tkt designator, check only the fare class
  if (tktFbcLength == std::string::npos)
  {
    if (ptfFbcLength != std::string::npos)
      return false;

    return matchFareClass(tktFareBasis.c_str(), ptfFareBasis.c_str());
  }

  if (ptfFbcLength == std::string::npos && tktFbcLength != std::string::npos)
    return false;

  // matching the fare basis code
  std::string tktFbc;
  tktFbc.append(tktFareBasis, 0, tktFbcLength);

  std::string ptfFbc;
  ptfFbc.append(ptfFareBasis, 0, ptfFbcLength);

  if (tktFbc != "-" && !matchFareClass(tktFbc.c_str(), ptfFbc.c_str()))
    return false;

  tktFbcLength++;

  // end of fare basis ?
  if (UNLIKELY(tktFbcLength >= tktFareBasisSize))
    return true;

  // matching the first ticket designator
  std::string tktDsgBoth = "";
  tktDsgBoth.append(tktFareBasis, tktFbcLength, tktFareBasisSize - tktFbcLength);
  std::string::size_type tktDsgOneLength = tktDsgBoth.find("/");
  std::string tktDsgOne = "";
  tktDsgOne.append(tktFareBasis, tktFbcLength, tktDsgOneLength);

  ptfFbcLength++;
  std::string ptfDsgBoth = "";
  ptfDsgBoth.append(ptfFareBasis, ptfFbcLength, ptfFareBasisSize - ptfFbcLength);
  std::string::size_type ptfDsgOneLength = ptfDsgBoth.find("/");
  std::string ptfDsgOne = "";
  ptfDsgOne.append(ptfFareBasis, ptfFbcLength, ptfDsgOneLength);

  if (LIKELY(tktDsgOneLength == std::string::npos))
  {
    if (!tktDsgOne.empty() && tktDsgOne[tktDsgOne.size() - 1] == '*')
    {
      if (tktDsgOne.size() == 1)
        return true;

      tktDsgOne[tktDsgOne.size() - 1] = '-';
    }

    if (LIKELY(!matchFareClass(tktDsgOne.c_str(), ptfDsgOne.c_str())))
      return false;

    return true;
  }

  if (ptfDsgOneLength == std::string::npos && tktDsgOneLength != std::string::npos)
    return false;

  if (!tktDsgOne.empty() && tktDsgOne[tktDsgOne.size() - 1] == '*')
  {
    tktDsgOne[tktDsgOne.size() - 1] = '-';
  }

  if (!matchFareClass(tktDsgOne.c_str(), ptfDsgOne.c_str()))
    return false;

  if (tktDsgOne.empty() && !ptfDsgOne.empty())
    return false;

  // matching the second ticket designator
  tktDsgOneLength++;
  std::string tktDsgTwo = "";
  tktDsgTwo.append(tktDsgBoth, tktDsgOneLength, tktDsgBoth.size() - tktDsgOneLength);

  ptfDsgOneLength++;
  std::string ptfDsgTwo = "";
  ptfDsgTwo.append(ptfDsgBoth, ptfDsgOneLength, ptfDsgBoth.size() - ptfDsgOneLength);

  if (!tktDsgTwo.empty() && tktDsgTwo[tktDsgTwo.size() - 1] == '*')
  {
    if (tktDsgTwo.size() == 1)
      return true;

    tktDsgTwo[tktDsgTwo.size() - 1] = '-';
  }

  return matchFareClass(tktDsgTwo.c_str(), ptfDsgTwo.c_str());
}

std::string
TicketingFeeCollector::getFareBasis(const PaxTypeFare& paxTypeFare) const
{
  return paxTypeFare.createFareBasis(_trx);
}

bool
TicketingFeeCollector::matchFareClass(const std::string ptfFareBasis,
                                      const std::string tktFareBasis) const
{
  return RuleUtil::matchFareClass(ptfFareBasis.c_str(), tktFareBasis.c_str());
}

std::string
TicketingFeeCollector::getS1FareBasisCode(const PaxTypeFare& paxTypeFare,
                                          const TravelSeg* lastAirSeg) const
{
  char tktDesLength = '5';
  char childInfantCode = '3';

  CalcTotals calcTotals;
  for (const PricingUnit* pu : _farePath->pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      for (const TravelSeg* tvlSeg : fu->travelSeg())
      {
        calcTotals.fareUsages[tvlSeg] = fu;
      }
    }
  }

  std::string fareBasisCode =
      calcTotals.getFareBasisCode(*_trx, lastAirSeg, tktDesLength, childInfantCode);
  return fareBasisCode;
}

bool
TicketingFeeCollector::internationalJouneyType()
{
  bool isInternational = false;

  const Itin* itin = _farePath->itin();
  TravelSeg* tsFirst = itin->travelSeg().front();

  for (TravelSeg* curSeg : itin->travelSeg())
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(curSeg);
    if (!airSeg || airSeg->segmentType() == Arunk || airSeg->segmentType() == Surface)
      continue;

    if (tsFirst->origin()->nation() != airSeg->destination()->nation() ||
        airSeg->origin()->nation() != airSeg->destination()->nation())
    {
      isInternational = true;
      break;
    }
  }
  return isInternational;
}

bool
TicketingFeeCollector::roundTripJouneyType()
{
  const Itin* itin = _farePath->itin();
  bool isInternational = internationalJouneyType();
  bool isRT = false;

  TravelSeg* tsFirst = itin->travelSeg().front();
  TravelSeg* endS = itin->travelSeg().back();

  if ((isInternational && tsFirst->origin()->nation() == endS->destination()->nation()))
    isRT = true;

  if (!isInternational)
  {
    if (tsFirst->origin()->loc() == endS->destination()->loc())
      isRT = true;
    else
      isRT = (LocUtil::getMultiTransportCity(tsFirst->origin()->loc(),
                                             validatingCarrier(),
                                             itin->geoTravelType(),
                                             _trx->ticketingDate()) ==
              LocUtil::getMultiTransportCity(endS->destination()->loc(),
                                             validatingCarrier(),
                                             itin->geoTravelType(),
                                             _trx->ticketingDate()));
  }
  return isRT;
}

bool
TicketingFeeCollector::isStopOverPoint(const TravelSeg* travelSeg,
                                       const TravelSeg* travelSegTo,
                                       const FareUsage* fareUsage) const
{
  if (fareUsage->paxTypeFare() && fareUsage->paxTypeFare()->fareMarket() &&
      travelSegTo->isStopOver(
          travelSeg, fareUsage->paxTypeFare()->fareMarket()->geoTravelType(), TravelSeg::OTHER))
    return true;

  return (fareUsage->stopovers().find(travelSeg) != fareUsage->stopovers().end());
}

bool
TicketingFeeCollector::isStopOver(const TravelSeg* travelSeg,
                                  const TravelSeg* travelSegTo,
                                  const FareUsage* fareUsage) const
{
  bool isStopOver = isStopOverPoint(travelSeg, travelSegTo, fareUsage);

  if (!isStopOver)
  {
    const TravelSeg* lastTvlSeg = fareUsage->travelSeg().back();
    const TravelSeg* lastAirSeg = TravelSegUtil::lastAirSeg(fareUsage->travelSeg());

    isStopOver = !(travelSeg != lastAirSeg || lastAirSeg == lastTvlSeg);
  }
  return isStopOver;
}

void
TicketingFeeCollector::getValidSegs()
{
  _validSegs.clear(); // Fresh Start

  for (PricingUnit* pu : _farePath->pricingUnit())
  {
    if (pu->isSideTripPU())
      continue;

    for (FareUsage* curFu : pu->fareUsage())
    {
      // Load up the FareBreaks first ...
      _validSegs.insert(curFu->travelSeg().back());

      // Now find the Stopovers
      TravelSegVecIC prevTs = curFu->travelSeg().begin();
      TravelSegVecIC curTs = prevTs + 1;
      for (; curTs != curFu->travelSeg().end(); ++curTs)
      {
        if (*prevTs == curFu->travelSeg().back() || (*prevTs)->isForcedConx())
        {
          prevTs = curTs;
          continue; // Already in the list or a Forced Connection
        }

        if ((*curTs)->segmentType() == Arunk || (*prevTs)->isForcedStopOver() ||
            isStopOver(*prevTs, *curTs, curFu))
          _validSegs.insert(*prevTs);
        prevTs = curTs; // On to the next pair
      } // for (TS iteration)
    } // for (FU iteration)
  } // for (PU iteration)
} // getValidSegs()

GlobalDirection
TicketingFeeCollector::getGlobalDirection(std::vector<TravelSeg*>& travelSegs) const
{
  GlobalDirection gd = GlobalDirection::XX;
  GlobalDirectionFinderV2Adapter::getGlobalDirection(
      _trx, _farePath->itin()->travelDate(), travelSegs, gd);
  return gd;
}

uint32_t
TicketingFeeCollector::getTPM(const Loc& market1,
                              const Loc& market2,
                              const GlobalDirection& glbDir,
                              const DateTime& tvlDate) const
{
  return (glbDir == GlobalDirection::XX ? TseUtil::greatCircleMiles(market1, market2)
                                        : // check only for performence reason
              LocUtil::getTPM(market1, market2, glbDir, tvlDate, _trx->dataHandle()));
}

bool
TicketingFeeCollector::setMlgTurnAround()
{
  Itin* itin = _farePath->itin();
  _mlgMap.clear(); // Fresh Start
  getValidSegs(); // Get List of all Stopovers and FareBreaks

  TravelSeg* firstTs = itin->travelSeg().front();
  std::vector<TravelSeg*> partialTravelSegs;

  for (TravelSeg* curTs : _validSegs)
  {
    partialTravelSegs.clear();

    for (TravelSeg* tvlSeg : itin->travelSeg())
    {
      partialTravelSegs.push_back(tvlSeg);

      if (tvlSeg->segmentType() == Arunk && !isArunkPartOfSideTrip(tvlSeg))
      {
        GlobalDirection gd = getGlobalDirection(partialTravelSegs);
        const uint32_t mlg = getTPM(
            *(firstTs->origin()), *(tvlSeg->destination()), gd, _trx->getRequest()->ticketingDT());
        _mlgMap.insert(std::map<uint32_t, int, std::greater<uint32_t>>::value_type(
            mlg, itin->segmentOrder(tvlSeg)));
      }
      if (itin->segmentOrder(curTs) == itin->segmentOrder(tvlSeg))
        break;
    }

    if (*(firstTs->origin()) == *(curTs->destination()))
      continue; // Why bother? Always gonna be 0

    GlobalDirection gd = getGlobalDirection(partialTravelSegs);
    const uint32_t mlg = getTPM(
        *(firstTs->origin()), *(curTs->destination()), gd, _trx->getRequest()->ticketingDT());
    _mlgMap.insert(std::map<uint32_t, int, std::greater<uint32_t>>::value_type(
        mlg, itin->segmentOrder(curTs)));
  } // for (validSegs iteration)

  if (_mlgMap.empty())
    return false;

  // Got ourselves a legitimate TurnAround Point
  _segmentOrderTurnAround = (*_mlgMap.begin()).second;

  for (const TravelSeg* tvlSeg : itin->travelSeg())
  {
    if (itin->segmentOrder(tvlSeg) == _segmentOrderTurnAround)
    {
      _journeyTurnAroundPoint = tvlSeg->destination();
      _journeyTurnAroundTs = tvlSeg;
      return true;
    }
  }
  return false;
}

void
TicketingFeeCollector::setJourneyDestination()
{
  if (roundTripJouneyType())
  {
    if (!setMlgTurnAround())
      return;
    if (internationalJouneyType())
    {
      if (_journeyTurnAroundPoint->nation() !=
          _farePath->itin()->travelSeg().front()->origin()->nation())
      {
        _journeyDestination = _journeyTurnAroundPoint;
      }
      else // determine the furthest point outside the origin country
      {
        std::map<uint32_t, int, std::greater<uint32_t>>::iterator mlgCurIt = _mlgMap.begin();
        std::map<uint32_t, int, std::greater<uint32_t>>::iterator mlgEndIt = _mlgMap.end();
        ++mlgCurIt;
        const Itin* itin = _farePath->itin();
        const NationCode firstSegNation = itin->travelSeg().front()->origin()->nation();
        for (; mlgCurIt != mlgEndIt; ++mlgCurIt)
        {
          const int segOrder = (*mlgCurIt).second;
          const auto tournAroundSeg = std::find_if(itin->travelSeg().begin(),
                                                   itin->travelSeg().end(),
                                                   [=](const TravelSeg* tvlSeg)
                                                   {
            return itin->segmentOrder(tvlSeg) == segOrder &&
                   tvlSeg->destination()->nation() != firstSegNation;
          });
          if (tournAroundSeg != itin->travelSeg().end())
          {
            _journeyDestination = (*tournAroundSeg)->destination();
            _journeyTurnAroundTs = *(tournAroundSeg);
            _journeyTurnAroundPoint = (*tournAroundSeg)->destination();

            return;
          }
        }
      }
    }
    else // domestic
    {
      _journeyDestination = _journeyTurnAroundPoint;
    }
  }
  else // one way
    _journeyDestination = _farePath->itin()->travelSeg().back()->destination();
}

bool
TicketingFeeCollector::validateGeoLoc1Loc2(const TicketingFeesInfo* feeInfo) const
{
  const LocKey& loc1 = feeInfo->loc1();
  const LocKey& loc2 = feeInfo->loc2();
  const Indicator journeyInd = feeInfo->journeyInd();
  const LocCode zone1 = feeInfo->loc1ZoneItemNo();
  const LocCode zone2 = feeInfo->loc2ZoneItemNo();
  const Loc& journeyOrigin = *_farePath->itin()->travelSeg().front()->origin();
  if (journeyInd == 'A')
  {
    return isInLoc(loc1, journeyOrigin, zone1) &&
           (loc2.isNull() || isInLoc(loc2, *_journeyDestination, zone2));
  } // else journeyInd is BLANK
  if (!loc1.isNull() && !loc2.isNull())
  {
    return ((isInLoc(loc1, journeyOrigin, zone1) && isInLoc(loc2, *_journeyDestination, zone2)) ||
            (isInLoc(loc2, journeyOrigin, zone2) && isInLoc(loc1, *_journeyDestination, zone1)));
  }
  if (!loc1.isNull())
  {
    return (isInLoc(loc1, journeyOrigin, zone1) || isInLoc(loc1, *_journeyDestination, zone1));
  }

  return true;
}

bool
TicketingFeeCollector::validateGeoVia(const TicketingFeesInfo* feeInfo) const
{
  const LocKey& viaLoc = feeInfo->locVia();
  if (viaLoc.isNull())
    return true;
  const LocCode zoneVia = feeInfo->locZoneViaItemNo();
  const std::vector<TravelSeg*>::const_iterator tsBegin = _farePath->itin()->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tsCur = _farePath->itin()->travelSeg().begin();
  const std::vector<TravelSeg*>::const_iterator tsEnd = _farePath->itin()->travelSeg().end();
  const TravelSeg* tsBack = _farePath->itin()->travelSeg().back();
  for (; tsCur != tsEnd; ++tsCur)
  {
    if ((tsCur != tsBegin && *(tsCur - 1) != _journeyTurnAroundTs &&
         isInLoc(viaLoc, *(*tsCur)->origin(), zoneVia)) ||
        (*tsCur != tsBack && *tsCur != _journeyTurnAroundTs &&
         isInLoc(viaLoc, *(*tsCur)->destination(), zoneVia)))
      return true;
  }
  return false;
}

bool
TicketingFeeCollector::validateGeoWhlWithin(const TicketingFeesInfo* feeInfo) const
{
  const LocKey& withinLoc = feeInfo->locWhlWithin();
  if (withinLoc.isNull())
    return true;
  const LocCode zoneWithin = feeInfo->locZoneWhlWithinItemNo();
  std::vector<TravelSeg*>& tvlSegs = _farePath->itin()->travelSeg();
  return std::none_of(tvlSegs.begin(),
                      tvlSegs.end(),
                      [=](const TravelSeg* tvlSeg)
                      {
    return (!isInLoc(withinLoc, *tvlSeg->origin(), zoneWithin) ||
            !isInLoc(withinLoc, *tvlSeg->destination(), zoneWithin));
  });
}

bool
TicketingFeeCollector::isInLoc(const LocKey& locKey, const Loc& loc, const LocCode& zone) const
{
  if (&loc == nullptr)
    return false;
  switch (locKey.locType())
  {
  case LOCTYPE_AREA:
  case LOCTYPE_ZONE:
  case LOCTYPE_NATION:
  case LOCTYPE_CITY:
  case LOCTYPE_STATE:
    return LocUtil::isInLoc(loc,
                            locKey.locType(),
                            locKey.loc(),
                            ATPCO_VENDOR_CODE,
                            RESERVED,
                            LocUtil::OTHER,
                            GeoTravelType::International,
                            EMPTY_STRING(),
                            _trx->ticketingDate());
  case LOCTYPE_USER:
    return LocUtil::isInZone(loc,
                             ATPCO_VENDOR_CODE,
                             zone,
                             TAX_ZONE,
                             LocUtil::OTHER,
                             GeoTravelType::International,
                             EMPTY_STRING(),
                             _trx->ticketingDate());

  default:
    return false;
  }
}

void
TicketingFeeCollector::createDiag()
{
  if (LIKELY(!_trx->diagnostic().isActive() ||
              _trx->diagnostic().diagnosticType() != Diagnostic870))
    return;

  if (!_diag)
  {
    DiagCollector* diagCollector = DCFactory::instance()->create(*_trx);
    _diag = dynamic_cast<Diag870Collector*>(diagCollector);
    if (_diag == nullptr)
      return;
    _diag->enable(Diagnostic870);
    _diag->printHeader();
  }
}

void
TicketingFeeCollector::printDiagHeader()
{
  if (_diag)
  {
    LocCode city = EMPTY_STRING();
    if (_journeyDestination)
    {
      city = _journeyDestination->city();
      if (city.empty())
        city = _journeyDestination->loc();
    }
    _diag->printSolutionHeader(*_trx,
                               validatingCarrier(),
                               *_farePath,
                               internationalJouneyType(),
                               roundTripJouneyType(),
                               city);
  }
}

void
TicketingFeeCollector::getFopBinNumber(std::vector<FopBinNumber>& fopBinNumberVector)
{
  if (!_trx->getRequest()->formOfPayment().empty())
    fopBinNumberVector.push_back(_trx->getRequest()->formOfPayment());

  if (_process2CC && _diag)
  {
    fopBinNumberVector.push_back(_trx->getRequest()->secondFormOfPayment());
  }
}

void
TicketingFeeCollector::printDiagHeaderFopBinNumber(
    const std::vector<FopBinNumber>& fopBinNumberVector)
{
  if (LIKELY(!_diag))
    return;
  if (!_process2CC)
  {
    _diag->printFopBinNumber(fopBinNumberVector);
    return;
  }
  CurrencyCode paymentCurrencyCode = TrxUtil::getEquivCurrencyCode(*_trx);
  std::vector<Indicator> cardTypes;
  cardTypes.push_back(_cardType);
  cardTypes.push_back(_secondCardType);
  _diag->printFopBinInfo(
      fopBinNumberVector, cardTypes, paymentCurrencyCode, _trx->getRequest()->paymentAmountFop());
}

void
TicketingFeeCollector::printDiagHeaderAmountFopResidualInd()
{
  if (_diag)
  {
    if (_process2CC)
      return;
    CurrencyCode paymentCurrencyCode = TrxUtil::getEquivCurrencyCode(*_trx);
    _diag->printAmountFopResidualInd(paymentCurrencyCode,
                                     _trx->getRequest()->paymentAmountFop(),
                                     _trx->getRequest()->chargeResidualInd());
  }
}

void
TicketingFeeCollector::printDiagHeader1()
{
  if (_diag)
  {
    const std::string& diagDD = _trx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL);
    if (diagDD != "INFO")
      _diag->printS4CommonHeader();
  }
}

void
TicketingFeeCollector::printFopBinNotFound()
{
  if (_diag)
    _diag->printFopBinNotFound();
}

void
TicketingFeeCollector::printCxrNotValid(const CarrierCode& cxr)
{
  if (_diag)
  {
    _diag->printCxrNotActive(cxr);
    endDiag();
  }
}

void
TicketingFeeCollector::printCanNotCollect(const StatusS4Validation rc) const
{
  if (_diag == nullptr)
    return;

  _diag->printCanNotCollect(rc);
  endDiag();
}

void
TicketingFeeCollector::printDiagS4NotFound() const
{
  if (_diag)
    _diag->printS4NotFound();
}

void
TicketingFeeCollector::checkDiagDetailedRequest(const TicketingFeesInfo* feeInfo)
{
  if (LIKELY(!_diag))
    return;

  const std::string& diagDD = _trx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL);
  const std::string& diagSQ = _trx->diagnostic().diagParamMapItem(Diagnostic::SEQ_NUMBER);
  const std::string& diagSC = _trx->diagnostic().diagParamMapItem(Diagnostic::SRV_CODE);

  if (diagDD == "INFO")
  {
    if ((!diagSQ.empty() && !isDiagSequenceMatch(feeInfo)) ||
        (!diagSC.empty() && !isDiagServiceCodeMatch(feeInfo)))
    {
      _diagInfo = false;
      return;
    }
    _diagInfo = true;
    printDiagFirstPartDetailedRequest(feeInfo);
    if (_diag)
      _diag->flushMsg();
  }
}
void
TicketingFeeCollector::printDiagS4Info(const TicketingFeesInfo* feeInfo,
                                       const StatusS4Validation& status)
{
  if (UNLIKELY(_diag))
  {
    const std::string& diagSQ = _trx->diagnostic().diagParamMapItem(Diagnostic::SEQ_NUMBER);
    const std::string& diagSC = _trx->diagnostic().diagParamMapItem(Diagnostic::SRV_CODE);
    if (!_diagInfo && (diagSQ.empty() || isDiagSequenceMatch(feeInfo)) &&
        (diagSC.empty() || isDiagServiceCodeMatch(feeInfo)))
    {
      const std::string& diagPASS = _trx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL);
      if (diagPASS == "PASS")
      {
        if (status == PASS_S4 && feeInfo->feeAmount() != 0)
          _diag->printS4GeneralInfo(feeInfo, status);
      }
      else
      {
        _diag->printS4GeneralInfo(feeInfo, status);
      }

      _diag->flushMsg();
      return;
    }

    if ((diagSQ.empty() || isDiagSequenceMatch(feeInfo)) &&
        (diagSC.empty() || isDiagServiceCodeMatch(feeInfo)))
    {
      printDiagFinalPartDetailedRequest(status);
    }
  }
}

bool
TicketingFeeCollector::isDiagSequenceMatch(const TicketingFeesInfo* feeInfo)
{
  const std::string& diagSQ = _trx->diagnostic().diagParamMapItem(Diagnostic::SEQ_NUMBER);
  int inputSeqNumber = std::atoi(diagSQ.c_str());
  return (inputSeqNumber == feeInfo->seqNo());
}

bool
TicketingFeeCollector::isDiagServiceCodeMatch(const TicketingFeesInfo* feeInfo)
{
  const std::string& diagSC = _trx->diagnostic().diagParamMapItem(Diagnostic::SRV_CODE);
  return (diagSC == feeInfo->serviceTypeCode() + feeInfo->serviceSubTypeCode());
}

void
TicketingFeeCollector::printDiagFirstPartDetailedRequest(const TicketingFeesInfo* feeInfo)
{
  if (!_diag)
    return;

  _diag->printFistPartDetailedRequest(feeInfo);
}

void
TicketingFeeCollector::printDiagFinalPartDetailedRequest(const StatusS4Validation& status)
{
  if (!_diag)
    return;

  _diag->printFinalPartDetailedRequest(status);
  _diag->lineSkip(0);
}

void
TicketingFeeCollector::endDiag() const
{
  if (LIKELY(!_diag))
    return;
  _diag->lineSkip(1);
  _diag->flushMsg();
}

void
TicketingFeeCollector::setCurrencyNoDec(TicketingFeesInfo& fee) const
{
  const tse::Currency* currency = nullptr;
  DataHandle dataHandle;
  currency = dataHandle.getCurrency(fee.cur());
  if (currency)
    fee.noDec() = currency->noDec();
}

bool
TicketingFeeCollector::isArunkPartOfSideTrip(const TravelSeg* tvlSeg)
{
  return std::any_of(_farePath->pricingUnit().cbegin(),
                     _farePath->pricingUnit().cend(),
                     [tvlSeg](const PricingUnit* pu)
                     {
    return (pu->isSideTripPU() &&
            tvlSeg->segmentOrder() >= pu->travelSeg().front()->segmentOrder() &&
            tvlSeg->segmentOrder() <= pu->travelSeg().back()->segmentOrder());
  });
}

bool
TicketingFeeCollector::isAnyNonCreditCardFOP() const
{
  return (!_trx->getRequest()->isFormOfPaymentCard() &&
          (_trx->getRequest()->isFormOfPaymentCash() ||
           _trx->getRequest()->isFormOfPaymentCheck() || _trx->getRequest()->isFormOfPaymentGTR()));
}

Indicator
TicketingFeeCollector::getCardType(const FopBinNumber& bin)
{
  if (OBFeesUtils::fallbackObFeesWPA(_trx))
  {
    if (!bin.empty() && bin.size() == FOP_BIN_SIZE &&
        std::find_if(bin.begin(), bin.end(), !boost::bind<bool>(&isDigit, _1)) == bin.end())
    {
      if (_processFop)
        _binExists = true;
    }
    else
    {
      return BLANK;
    }
  }
  else
  {
    if (UNLIKELY(OBFeesUtils::isBinCorrect(bin)))
    {
      if (_processFop)
        _binExists = true;
    }
    else
    {
      return BLANK;
    }
  }

  const BankIdentificationInfo* matchedBin =
      _trx->dataHandle().getBankIdentification(bin, _farePath->itin()->travelDate());

  if (matchedBin != nullptr)
  {
    return matchedBin->cardType();
  }

  return CREDIT;
}

void
TicketingFeeCollector::processRTType(StatusS4Validation& rc, TicketingFeesInfo* feeInfo)
{
  try
  {
    if (UNLIKELY(validateSequence(feeInfo, rc)))
    {
      setCurrencyNoDec(*feeInfo);
      if (checkDuplicatedRecords(feeInfo))
        rc = DUPLICATE_S4;
      else
      {
        if (isTTypeSkip(feeInfo))
          rc = SKIP_TTYPE_NOT_EXCHANGE_REFUND;
        else
          ticketingFeesInfoFromFP(feeInfo).push_back(feeInfo);
      }
    }
    printDiagS4Info(feeInfo, rc);
  }
  catch (...)
  {
    printDiagS4Info(feeInfo, PROCESSING_ERROR);
  }
}

bool
TicketingFeeCollector::isTTypeSkip(TicketingFeesInfo* feeInfo)
{
  return (feeInfo->serviceSubTypeCode()[0] == 'T' && feeInfo->serviceSubTypeCode()[1] == '9' &&
          _trx->isNotExchangeTrx());
}

void
TicketingFeeCollector::setCardType()
{
  _cardType = getCardType(_trx->getRequest()->formOfPayment());

  _secondCardType = getCardType(_trx->getRequest()->secondFormOfPayment());
  // credit card type may be 'C', 'G' and BLANK
  adjustAnyCreditCardType(_cardType);
  adjustAnyCreditCardType(_secondCardType);
}

void
TicketingFeeCollector::adjustAnyCreditCardType(Indicator& cardType)
{
  if (LIKELY(cardType != DEBIT))
    cardType = CREDIT;
}

void
TicketingFeeCollector::sortOBFeeVectors(std::vector<TicketingFeesInfo*>& sortVector)
{
  if (!sortVector.empty())
  {
    std::sort(sortVector.begin(), sortVector.end(), TicketingFeeComparator());
  }
}

void
TicketingFeeCollector::initializeFor2CreditCards(std::vector<TicketingFeesInfo*>& fopMatched) const
{
  if (LIKELY(!_process2CC))
    return;
  fopMatched.push_back(nullptr);
  fopMatched.push_back(nullptr);
}

void
TicketingFeeCollector::swapObFeesVecFor2CC(std::vector<TicketingFeesInfo*>& fopMatched)
{
  if (_process2CC)
    _farePath->collectedTktOBFees().swap(fopMatched);
}
} // namespace tse
