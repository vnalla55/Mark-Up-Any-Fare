//-------------------------------------------------------------------
//  Copyright Sabre 2011
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
#include "FreeBagService/BaggageChargesValidator.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/FreeBaggageUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/FarePath.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "Diagnostic/Diag852Collector.h"
#include "ServiceFees/OCFees.h"
#include "Util/FlatSet.h"

#include <boost/range/irange.hpp>

namespace tse
{
static Logger
logger("atseintl.FreeBagService.BaggageChargesValidator");

void
BaggageChargesValidator::collectCharges(const SubCodeInfo& s5,
                                        const uint32_t freePieces,
                                        IChargeCollector& out)
{
  if (!out.needAny())
    return;

  const std::vector<OptionalServicesInfo*>& optSrvInfos = getOptionalServicesInfo(s5);
  const uint32_t requestedPieces = _trx.getBaggagePolicy().getRequestedBagPieces();
  ServiceFeeUtil util(_trx);
  BaggageCharge* charge = buildCharge(s5);

  for (OptionalServicesInfo* s7 : optSrvInfos)
  {
    if (isCancelled(*s7))
      continue;

    for (uint32_t bagNo = freePieces; bagNo < requestedPieces; ++bagNo)
      charge->setMatchedBag(bagNo, matchOccurrence(*s7, bagNo - freePieces + 1));

    StatusS7Validation rc = out.needThis(*charge) ? PASS_S7 : FAIL_S7_BAGGAGE_OCCURRENCE;

    if (rc == PASS_S7)
      rc = validateS7Data(*s7, *charge);

    if (rc == PASS_S7 && !retrieveSpecifiedFee(*s7, *charge))
      rc = FAIL_S7_SFC_T170;

    if (rc != PASS_S7)
    {
      charge->cleanBaggageResults();
      continue;
    }

    convertCurrency(util, *charge);
    supplementFees(s5, charge);

    if (out.collect(*charge))
      break;

    charge = buildCharge(s5);
  }
}

void
BaggageChargesValidator::validate(const SubCodeInfo& subCodeInfo, ChargeVector& matchedCharges)
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__
                             << "subCodeInfo={serviceGroup=" << subCodeInfo.serviceGroup()
                             << ",servceSubGroup=" << subCodeInfo.serviceSubGroup() << "}"
                             << ", matchedCharges.size()=" << matchedCharges.size());

  const uint32_t firstChargedPiece =
      FreeBaggageUtil::calcFirstChargedPiece(_baggageTravel._allowance);
  const uint32_t requestedPieces = _trx.getBaggagePolicy().getRequestedBagPieces();

  if (firstChargedPiece >= requestedPieces)
    return;

  ServiceFeeUtil util(_trx);
  BaggageCharge* charge = buildCharge(subCodeInfo);

  const auto excessPieces = boost::irange(firstChargedPiece, requestedPieces);
  FlatSet<uint32_t> remainingPieces(excessPieces.begin(), excessPieces.end());
  PerBagPiece<std::vector<S7StatusTuple>> s7DiagStatus;

  const std::vector<OptionalServicesInfo*>& optSrvInfos = getOptionalServicesInfo(subCodeInfo);

  if (optSrvInfos.empty())
    printDiagS7NotFound(subCodeInfo);

  for (OptionalServicesInfo* optSrv : optSrvInfos)
  {
    if (remainingPieces.empty())
      break;

    if (isCancelled(*optSrv))
      continue;

    checkDiagS7ForDetail(optSrv);
    charge->cleanOutCurrentSeg();

    // Validate occurences against any of remaining excess bag pieces.
    // Note that if there's one free bag piece the second one becomes first excess bag piece etc.
    std::bitset<MAX_BAG_PIECES> matchedPieces;

    for (const uint32_t bagNo : remainingPieces)
      matchedPieces.set(bagNo, matchOccurrence(*optSrv, bagNo - firstChargedPiece + 1));

    StatusS7Validation rc = PASS_S7;

    if (matchedPieces.none())
      rc = FAIL_S7_BAGGAGE_OCCURRENCE;

    if (rc == PASS_S7)
      rc = validateS7Data(*optSrv, *charge);

    if (rc == PASS_S7 && !retrieveSpecifiedFee(*optSrv, *charge))
      rc = FAIL_S7_SFC_T170;

    // Update diagnostic status
    for (const uint32_t bagNo : remainingPieces)
    {
      const StatusS7Validation status = matchedPieces.test(bagNo) ? rc : FAIL_S7_BAGGAGE_OCCURRENCE;
      s7DiagStatus[bagNo].emplace_back(optSrv, *charge, status);
    }

    // In case of PASS save the charge and update remaining bag pieces
    if (rc == PASS_S7)
    {
      for (const uint32_t bagNo : excessPieces)
      {
        if (!matchedPieces.test(bagNo))
          continue;
        charge->setMatchedBag(bagNo);
        remainingPieces.erase(bagNo);
      }

      convertCurrency(util, *charge);
      supplementAndAppendCharge(subCodeInfo, charge, matchedCharges);
      charge = buildCharge(subCodeInfo);
    }
  }

  for (const uint32_t bagNo : excessPieces)
    printDiagS7Info(s7DiagStatus[bagNo], bagNo);
}

void
BaggageChargesValidator::printDiagS7Info(const std::vector<S7StatusTuple>& s7statuses,
                                         uint32_t bagNo) const
{
  if (!_diag)
    return;

  (static_cast<Diag852Collector*>(_diag))->printS7ChargesHeader(bagNo);

  for (const S7StatusTuple& s7status : s7statuses)
  {
    BaggageCharge baggageCharge(std::get<1>(s7status));
    StatusS7Validation status = std::get<2>(s7status);

    if (status == FAIL_S7_BAGGAGE_OCCURRENCE)
      baggageCharge.cleanOutCurrentSeg();

    OptionalServicesValidator::printDiagS7Info(std::get<0>(s7status), baggageCharge, status);
  }
}

StatusS7Validation
BaggageChargesValidator::validateS7Data(OptionalServicesInfo& optSrvInfo, OCFees& ocFees)
{
  StatusS7Validation status = BaggageAllowanceValidator::validateS7Data(optSrvInfo, ocFees);
  if (status == PASS_S7)
  {
    if (!checkAndOrIndicator(optSrvInfo))
      return FAIL_S7_AND_OR_IND;

    if (!checkBaggageWeightUnit(optSrvInfo))
      return FAIL_S7_BAGGAGE_WEIGHT_UNIT;

    if (!checkFeeApplication(optSrvInfo))
      return FAIL_S7_FEE_APPLICATION;

    if (!checkOccurrence(optSrvInfo))
      return FAIL_S7_BAGGAGE_OCCURRENCE;
  }
  return status;
}

StatusS7Validation
BaggageChargesValidator::checkServiceNotAvailNoCharge(const OptionalServicesInfo& info,
                                                      OCFees& /*ocFees*/) const
{
  switch (info.notAvailNoChargeInd())
  {
  case CHAR_BLANK:
  case SERVICE_FREE_NO_EMD_ISSUED:
  case SERVICE_FREE_EMD_ISSUED:
  case SERVICE_FREE_NO_BOOK_NO_EMD:
  case SERVICE_FREE_NO_BOOK_EMD_ISSUED:
  case SERVICE_NOT_AVAILABLE:
    return PASS_S7;
    break;
  default:
    return FAIL_S7_NOT_AVAIL_NO_CHANGE;
    break;
  }
}

bool
BaggageChargesValidator::checkAndOrIndicator(const OptionalServicesInfo& optSrvInfo) const
{
  return optSrvInfo.andOrInd() == CHAR_BLANK;
}

bool
BaggageChargesValidator::checkBaggageWeightUnit(const OptionalServicesInfo& optSrvInfo) const
{
  return optSrvInfo.baggageWeightUnit() == CHAR_BLANK;
}

bool
BaggageChargesValidator::checkFeeApplication(const OptionalServicesInfo& optSrvInfo) const
{
  if (optSrvInfo.notAvailNoChargeInd() == CHAR_BLANK)
  {
    if (TrxUtil::isBaggageDotPhase2ADisplayEnabled(_trx))
    {
      return (optSrvInfo.serviceFeesCurrencyTblItemNo() > 0) &&
             ((optSrvInfo.frequentFlyerMileageAppl() == '3') ||
              (optSrvInfo.frequentFlyerMileageAppl() == '4'));
    }
    return (optSrvInfo.serviceFeesCurrencyTblItemNo() > 0) &&
           (optSrvInfo.frequentFlyerMileageAppl() == '4');
  }
  else
  {
    return (optSrvInfo.serviceFeesCurrencyTblItemNo() == 0) &&
           (optSrvInfo.frequentFlyerMileageAppl() == CHAR_BLANK) &&
           (optSrvInfo.applicationFee() == 0);
  }
}

bool
BaggageChargesValidator::isOccurrenceBlankBlank(const OptionalServicesInfo& optSrvInfo)
{
  return (optSrvInfo.baggageOccurrenceFirstPc() <= 0) &&
         (optSrvInfo.baggageOccurrenceLastPc() <= 0);
}

bool
BaggageChargesValidator::checkOccurrence(const OptionalServicesInfo& optSrvInfo) const
{
  if (isOccurrenceBlankBlank(optSrvInfo))
    return true;

  return optSrvInfo.baggageOccurrenceFirstPc() > 0 &&
         optSrvInfo.baggageOccurrenceFirstPc() <= int32_t(MAX_BAG_PIECES);
}

bool
BaggageChargesValidator::matchOccurrence(const OptionalServicesInfo& s7, int32_t bagNo)
{
  return FreeBaggageUtil::matchOccurrence(s7, bagNo);
}

void
BaggageChargesValidator::convertCurrency(ServiceFeeUtil& util, OCFees& fee) const
{
  fee.orginalFeeAmount() = fee.feeAmount();
  fee.orginalFeeCurrency() = fee.feeCurrency();
  fee.orginalFeeNoDec() = fee.feeNoDec();

  Money tmpMoney = util.convertBaggageFeeCurrency(&fee);
  fee.feeAmount() = tmpMoney.value();
  fee.feeCurrency() = tmpMoney.code();
  fee.feeNoDec() = tmpMoney.noDec();
}

const Loc&
BaggageChargesValidator::getLocForSfcValidation() const
{
  if (TrxUtil::isBaggage302GlobalDisclosureActivated(_trx))
  {
    if (_isUsDot)
      return *(_baggageTravel.itin()->firstTravelSeg()->origin());
    else
      return *(_trx.getRequest()->ticketingAgent()->agentLocation());
  }
  else
    return *(_baggageTravel.itin()->firstTravelSeg()->origin());
}

BaggageCharge*
BaggageChargesValidator::buildCharge(const SubCodeInfo& subCodeInfo) const
{
  BaggageCharge* charges = _trx.dataHandle().create<BaggageCharge>();
  charges->subCodeInfo() = &subCodeInfo;
  return charges;
}

void
BaggageChargesValidator::supplementAndAppendCharge(const SubCodeInfo& subCodeInfo,
                                                   BaggageCharge* baggageCharge,
                                                   ChargeVector& matchedCharges) const
{
  supplementFees(subCodeInfo, baggageCharge);
  matchedCharges.push_back(baggageCharge);
}

bool
BaggageChargesValidator::checkFrequentFlyerStatus(const OptionalServicesInfo& optSrvInfo,
                                                  OCFees& ocFees) const
{
  if (!TrxUtil::isFrequentFlyerTierActive(_trx) || _trx.getRequest()->isSpecificAgencyText())
  {
    return BaggageAllowanceValidator::checkFrequentFlyerStatus(optSrvInfo, ocFees);
  }
  if (optSrvInfo.frequentFlyerStatus() == 0)
    return true;

  if (!_trx.getOptions()->isWPwithOutAE())
    return false;

  return _baggageTravel.getFreqFlyerTierLevel() <= optSrvInfo.frequentFlyerStatus();
}
} // tse
