//-------------------------------------------------------------------
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
#include "FreeBagService/BaggageResultProcessor.h"

#include "Common/BaggageTripType.h"
#include "Common/FreeBaggageUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/FarePath.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "Diagnostic/Diag852Collector.h"
#include "Diagnostic/DiagManager.h"
#include "FreeBagService/BaggageTextFormatter.h"
#include "FreeBagService/BaggageTicketingDateScope.h"
#include "ServiceFees/OCFees.h"
#include "Util/BranchPrediction.h"
#include "Util/IteratorRange.h"

#include <tuple>

namespace tse
{

namespace
{
const std::string BAG_ALLOWANCE_TEXT = "BAG ALLOWANCE     -";
const std::string BAG_FEE_TEXT[MAX_BAG_PIECES] = {
    "1STCHECKED BAG FEE-", "2NDCHECKED BAG FEE-", "3RDCHECKED BAG FEE-", "4THCHECKED BAG FEE-"};
const std::string BAG_FEE_APPL_MSG = "BAG FEES APPLY AT EACH CHECK IN LOCATION";
const std::string ADDITIONAL_ALLOWANCES_TAG = "<ADD>\n";
}

BaggageResultProcessor::BaggageResultProcessor(PricingTrx& trx) : _trx(trx) {}

void
BaggageResultProcessor::buildAllowanceText(const std::vector<BaggageTravel*>& baggageTravels,
                                            bool isUsDot) const
{
  for (BaggageTravel* bt : baggageTravels)
  {
    const OptionalServicesInfo* const s7 = bt->_allowance ? bt->_allowance->optFee() : nullptr;
    const std::string allowance = BaggageTextFormatter::formatPQAllowanceText(s7, isUsDot);

    if (allowance.empty())
      continue;

    FarePath::SegToAllowanceTextMap& segToAllowance = bt->farePath()->mutableBaggageAllowance();

    for (const TravelSeg* ts : makeIteratorRange(bt->getTravelSegBegin(), bt->getTravelSegEnd()))
    {
      const AirSeg* as = ts->toAirSeg();
      if (as)
        segToAllowance[as] = allowance;
    }
  }
}

void
BaggageResultProcessor::buildBaggageResponse(const std::vector<FarePath*>& farePaths)
{
  for (FarePath* farePath : farePaths)
    farePath->baggageResponse() += buildBaggageResponse(farePath);
}

void
BaggageResultProcessor::processAdditionalAllowances() const
{
  FarePath* farePath;
  bool allowancesApply;

  for (auto& item : _additionalAllowancesApply)
  {
    std::tie(farePath, allowancesApply) = item;

    if (allowancesApply)
      farePath->baggageResponse() += ADDITIONAL_ALLOWANCES_TAG;
  }
}

void
BaggageResultProcessor::processFeesAtEachCheck() const
{
  FarePath* farePath;
  bool feesAtEachCheckApply;

  for (auto& item : _feesAtEachCheckApply)
  {
    std::tie(farePath, feesAtEachCheckApply) = item;

    if (feesAtEachCheckApply)
      farePath->baggageResponse() +=
          BaggageTextFormatter::TWO_ASTERISKS + BAG_FEE_APPL_MSG + BaggageTextFormatter::NEW_LINE;
  }
}

std::string
BaggageResultProcessor::buildBaggageResponse(FarePath* farePath)
{
  if (!farePath)
    return std::string();

  const std::vector<const BaggageTravel*> baggageTravels = farePath->baggageTravels();
  if (baggageTravels.empty())
    return std::string();

  DiagManager diagMgr(_trx, Diagnostic852);
  Diag852Collector* diag =
      UNLIKELY(diagMgr.isActive()) ? static_cast<Diag852Collector*>(&diagMgr.collector()) : nullptr;

  BaggageTextFormatter textFormatter(_trx, diag);

  std::string response;
  bool addAdditionalAllowancesMayApplyText = false;
  bool addFeesApplyAtEachCheckInText = false;
  uint32_t checkedPortion = 1;

  BaggageTicketingDateScope scopedDateSetter(_trx, farePath);

  for (const BaggageTravel* baggageTravel : baggageTravels)
  {
    if (!baggageTravel)
      continue;

    const OptionalServicesInfo* s7 =
        baggageTravel->_allowance ? baggageTravel->_allowance->optFee() : nullptr;

    if (UNLIKELY(diag))
      diag->printTable196ForBaggageDetailSetup(checkedPortion, s7, *farePath, _trx);

    ++checkedPortion;

    if (!baggageTravel->shouldAttachToDisclosure())
      continue;

    const std::string carrierText = textFormatter.getCarrierText(
        _trx, baggageTravel, farePath->itin()->getBaggageTripType().isUsDot());
    const std::string travelText = textFormatter.getTravelText(baggageTravel);

    response += BAG_ALLOWANCE_TEXT + travelText + BaggageTextFormatter::DASH +
                textFormatter.formatAllowanceText(s7, carrierText) + BaggageTextFormatter::NEW_LINE;

    if (s7)
      addAdditionalAllowancesMayApplyText = true;
    else
      response += BaggageTextFormatter::UNKNOWN_INDICATOR +
                  BaggageTextFormatter::UNKNOWN_FEE_MSG + carrierText +
                  BaggageTextFormatter::NEW_LINE;

    const uint32_t requestedBagPieces = _trx.getBaggagePolicy().getRequestedBagPieces();
    const uint32_t firstChargedPiece =
        FreeBaggageUtil::calcFirstChargedPiece(baggageTravel->_allowance);

    for (size_t bagNo = firstChargedPiece; bagNo < requestedBagPieces; ++bagNo)
    {
      response += BAG_FEE_TEXT[bagNo] + travelText + BaggageTextFormatter::DASH +
                  textFormatter.formatChargeText(baggageTravel->_charges[bagNo],
                                                 carrierText,
                                                 addFeesApplyAtEachCheckInText,
                                                 addAdditionalAllowancesMayApplyText) +
                  BaggageTextFormatter::NEW_LINE;
    }
  }

  _feesAtEachCheckApply[farePath] = addFeesApplyAtEachCheckInText;
  _additionalAllowancesApply[farePath] = addAdditionalAllowancesMayApplyText;

  return response;
}
} // tse
