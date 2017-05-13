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
#include "FreeBagService/BaggageOcValidationAdapter.h"

#include "Common/OcTypes.h"
#include "Common/ServiceFeeUtil.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/Itin.h"
#include "Diagnostic/Diag877Collector.h"
#include "FreeBagService/AllowanceSoftPassCollector.h"
#include "FreeBagService/AllowanceUtil.h"
#include "FreeBagService/BaggageAllowanceValidator.h"
#include "FreeBagService/BaggageAncillaryChargesValidator.h"
#include "FreeBagService/BaggageChargesValidator.h"
#include "FreeBagService/BaggageFastForwardAllowanceValidator.h"
#include "FreeBagService/BagValidationOpt.h"
#include "FreeBagService/CarryOnBaggageAllowanceValidator.h"
#include "FreeBagService/ChargeSoftPassCollector.h"
#include "FreeBagService/EmbargoesValidator.h"
#include "FreeBagService/SoftpassBtaSubvalidator.h"
#include "FreeBagService/SoftpassNonBtaFareSubvalidator.h"
#include "ServiceFees/OCFees.h"
#include "Util/IteratorRange.h"

namespace tse
{
namespace BaggageOcValidationAdapter
{
namespace
{
void
displayTravelInfo(const BaggageTravel& baggageTravel, const Ts2ss& ts2ss, Diag877Collector* diag)
{
  if (UNLIKELY(diag))
  {
    diag->printTravelInfo(&baggageTravel, ts2ss);
    diag->flushMsg();
  }
}

void
collectFaresForFarex(const BaggageTravel& bt, Ts2ss& ts2ss)
{
  for (const PricingUnit* pu : bt.farePath()->pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      PaxTypeFare* const ptf = fu->paxTypeFare();

      if (ptf->fareMarket()->travelSeg().empty())
        continue;

      const int16_t ptfFirstSegOrder = ptf->fareMarket()->travelSeg().front()->segmentOrder();
      const int16_t ptfLastSegOrder = ptf->fareMarket()->travelSeg().back()->segmentOrder();

      for (TravelSeg* ts : makeIteratorRange(bt.getTravelSegBegin(), bt.getTravelSegEnd()))
      {
        if (ts->segmentOrder() < ptfFirstSegOrder || ts->segmentOrder() > ptfLastSegOrder)
          continue;

        auto& tsData = ts2ss[ts];
        if (!tsData.second)
          tsData.second = ptf;
      }
    }
  }
}

void
setValidationContext(const BaggageTravel& bt,
                     const FarePath& farePath,
                     Ts2ss& ts2ss,
                     Diag877Collector* diag)
{
  ServiceFeeUtil::collectSegmentStatus(farePath, ts2ss);
  if (bt._trx->getOptions()->fareX())
    collectFaresForFarex(bt, ts2ss);
  displayTravelInfo(bt, ts2ss, diag);
}
}

std::vector<OCFees*>
matchS7AllowanceSkipFareChecks(const BagValidationOpt& opt, const SubCodeInfo& s5, uint32_t startSeqNo)
{
  BaggageAllowanceValidator validator(opt);
  validator.setBtaSubvalidator(
      new SoftpassBtaSubvalidator(opt._bt, validator, opt._ts2ss, nullptr));
  validator.setNonBtaFareSubvalidator(new SoftpassNonBtaFareSubvalidator());

  std::vector<OCFees*> result;
  AllowanceSoftPassCollector collector(result);

  validator.collectAllowance(s5, startSeqNo, collector);
  return result;
}

std::vector<BaggageCharge*>
matchS7ChargeSkipFareChecks(const BagValidationOpt& opt, const SubCodeInfo& s5, uint32_t freePieces)
{
  std::vector<BaggageCharge*> possibleFees;
  ChargeSoftPassCollector collector(*opt._bt._trx, freePieces, possibleFees);
  BaggageChargesValidator validator(opt);

  validator.setBtaSubvalidator(
      new SoftpassBtaSubvalidator(opt._bt, validator, opt._ts2ss, nullptr));
  validator.setNonBtaFareSubvalidator(new SoftpassNonBtaFareSubvalidator());
  validator.collectCharges(s5, freePieces, collector);

  // it's a softpass processing so freePieces actually indicates: "least possible number
  // of free pieces" and exact number will be determined later. That's why we have to normalize
  // the bitset of matched pieces. It's now more like a bitset of matched EXCESS pieces.
  for (BaggageCharge* bc : possibleFees)
    bc->mutableMatchedBags() >>= freePieces;

  // once we have a list of possible charges convert them into calculation currency
  // to speed up later processing in FarePathFactory
  ServiceFeeUtil sfu(*opt._bt._trx);
  CurrencyCode calcCurrency = opt._bt.itin()->calculationCurrency();

  if (calcCurrency == NUC)
    calcCurrency = USD;

  for (BaggageCharge* bc : possibleFees)
  {
    const Money calcCurrFee = sfu.convertBaggageFeeCurrency(
        bc->orginalFeeAmount(), bc->orginalFeeCurrency(), calcCurrency);
    bc->feeAmount() = calcCurrFee.value();
    bc->feeCurrency() = calcCurrFee.code();
    bc->feeNoDec() = calcCurrFee.noDec();
  }

  return possibleFees;
}

OCFees&
matchS7AllowanceRecord(const SubCodeInfo& subCodeInfo,
                       BaggageTravel& bt,
                       const CheckedPoint& fcp,
                       Diag877Collector* diag,
                       bool allowanceCarrierOverridden)
{
  Ts2ss ts2ss;
  setValidationContext(bt, *bt.farePath(), ts2ss, diag);
  const CarrierCode deferTarget =
      !allowanceCarrierOverridden ? AllowanceUtil::getDeferTargetCxr(bt) : CarrierCode();
  BagValidationOpt opt(bt,
                       fcp,
                       ts2ss,
                       ServiceFeeUtil::isInternationalJourneyType(*bt.itin()),
                       diag,
                       deferTarget);

  BaggageAllowanceValidator validator(opt);
  return validator.validate(subCodeInfo);
}

OCFees&
matchS7FastForwardAllowanceRecord(const SubCodeInfo& subCodeInfo,
                                  BaggageTravel& bt,
                                  const CheckedPoint& fcp,
                                  uint32_t lastSequence,
                                  Diag877Collector* diag)
{
  Ts2ss ts2ss;
  setValidationContext(bt, *bt.farePath(), ts2ss, diag);
  BagValidationOpt opt(
      bt, fcp, ts2ss, ServiceFeeUtil::isInternationalJourneyType(*bt.itin()), diag);

  BaggageFastForwardAllowanceValidator validator(opt);
  return validator.validate(subCodeInfo, lastSequence + 1);
}

OCFees&
matchS7CarryOnAllowanceRecord(const SubCodeInfo& subCodeInfo,
                              BaggageTravel& bt,
                              const CheckedPoint& fcp,
                              Diag877Collector* diag)
{
  Ts2ss ts2ss;
  setValidationContext(bt, *bt.farePath(), ts2ss, diag);
  BagValidationOpt opt(
      bt, fcp, ts2ss, ServiceFeeUtil::isInternationalJourneyType(*bt.itin()), diag);

  CarryOnBaggageAllowanceValidator validator(opt);
  return validator.validate(subCodeInfo);
}

OCFees&
matchS7EmbargoRecord(const SubCodeInfo& subCodeInfo,
                     BaggageTravel& bt,
                     const CheckedPoint& fcp,
                     Diag877Collector* diag)
{
  Ts2ss ts2ss;
  setValidationContext(bt, *bt.farePath(), ts2ss, diag);
  BagValidationOpt opt(
      bt, fcp, ts2ss, ServiceFeeUtil::isInternationalJourneyType(*bt.itin()), diag);

  EmbargoesValidator validator(opt);
  return validator.validate(subCodeInfo);
}

void
matchS7ChargesRecords(const SubCodeInfo& subCodeInfo,
                      BaggageTravel& bt,
                      const CheckedPoint& fcp,
                      Diag877Collector* diag,
                      ChargeVector& matchedCharges)
{
  Ts2ss ts2ss;
  setValidationContext(bt, *bt.farePath(), ts2ss, diag);
  BagValidationOpt opt(
      bt, fcp, ts2ss, ServiceFeeUtil::isInternationalJourneyType(*bt.itin()), diag);

  BaggageChargesValidator validator(opt);
  validator.validate(subCodeInfo, matchedCharges);
}

void
matchS7AncillaryChargesRecords(const SubCodeInfo& subCodeInfo,
                               BaggageTravel& bt,
                               const CheckedPoint& fcp,
                               Diag877Collector* diag,
                               ChargeVector& matchedCharges)
{
  Ts2ss ts2ss;
  setValidationContext(bt, *bt.farePath(), ts2ss, diag);
  BagValidationOpt opt(
      bt, fcp, ts2ss, ServiceFeeUtil::isInternationalJourneyType(*bt.itin()), diag);

  BaggageAncillaryChargesValidator validator(opt);
  validator.filterSegments();
  if (validator.hasSegments())
    validator.validate(subCodeInfo, matchedCharges);
}

void
matchS7CarryOnChargesRecords(const SubCodeInfo& subCodeInfo,
                             BaggageTravel& bt,
                             const CheckedPoint& fcp,
                             Diag877Collector* diag,
                             ChargeVector& matchedCharges)
{
  Ts2ss ts2ss;
  setValidationContext(bt, *bt.farePath(), ts2ss, diag);
  BagValidationOpt opt(
      bt, fcp, ts2ss, ServiceFeeUtil::isInternationalJourneyType(*bt.itin()), diag);

  BaggageAncillaryChargesValidator validator(opt);
  validator.validate(subCodeInfo, matchedCharges);
}
}
} // tse
