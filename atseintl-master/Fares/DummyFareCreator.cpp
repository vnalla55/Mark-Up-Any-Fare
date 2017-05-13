#include "Fares/DummyFareCreator.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/NUCCurrencyConverter.h"
#include "Common/TravelSegUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "DBAccess/PenaltyInfo.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "RexPricing/ReissuePenaltyCalculator.h"
#include "Rules/Penalties.h"
#include "Util/BranchPrediction.h"
#include "Util/FlatSet.h"

#include <algorithm>
#include <cstdlib>

namespace tse
{
namespace DummyFareCreator
{
static TariffCrossRefInfo*
createTCRInfo(PricingTrx& trx, const FareMarket& fm)
{
  TariffCrossRefInfo* const tcrInfo = trx.dataHandle().create<TariffCrossRefInfo>();

  tcrInfo->fareTariff() = 0;
  tcrInfo->carrier() = fm.governingCarrier();
  tcrInfo->vendor() = ATPCO_VENDOR_CODE;
  tcrInfo->globalDirection() = fm.getGlobalDirection();

  if (!fm.travelSeg().empty() && !fm.travelSeg().front()->globalDirectionOverride().empty())
  {
    GlobalDirection gbd;
    strToGlobalDirection(gbd, fm.travelSeg().front()->globalDirectionOverride());
    tcrInfo->globalDirection() = gbd;
  }

  return tcrInfo;
}

static FareInfo*
createFareInfo(PricingTrx& trx, const FareMarket& fm, const FareSettings& settings)
{
  FareInfo* const fi = trx.dataHandle().create<FareInfo>();

  fi->fareAmount() = strtod(fm.fareCalcFareAmt().c_str(), nullptr);
  if (fi->fareAmount() < 0)
    return nullptr;

  const size_t decPos = fm.fareCalcFareAmt().find('.');
  if (decPos != std::string::npos)
    fi->noDec() = fm.fareCalcFareAmt().size() - decPos - 1;

  fi->vendor() = tse::ATPCO_VENDOR_CODE;
  fi->fareTariff() = 0;
  fi->carrier() = fm.governingCarrier();
  fi->ruleNumber() = "";
  fi->directionality() = BOTH;
  fi->globalDirection() = fm.getGlobalDirection();
  fi->fareClass() = fm.fareBasisCode();
  fi->market1() = fm.boardMultiCity();
  fi->market2() = fm.offMultiCity();
  fi->currency() = settings._currency;
  fi->owrt() = settings._owrt;
  fi->originalFareAmount() == fi->isRoundTrip() ? fi->fareAmount() * 2 : fi->fareAmount();
  return fi;
}

static MoneyAmount
determineNucFareAmount(PricingTrx& trx,
                       const Itin& itin,
                       const FareInfo& fi,
                       const FareSettings& sett)
{
  if (!sett._convertToCalcCurrency || itin.calculationCurrency() == fi.currency())
    return fi.fareAmount();

  const Money sourceAmt(fi.fareAmount(), fi.currency());
  Money targetAmt(itin.calculationCurrency());

  if (itin.calculationCurrency() == NUC)
  {
    ExchangePricingTrx* excTrx = dynamic_cast<ExchangePricingTrx*>(&trx);
    const DateTime& tktDate = excTrx ? excTrx->originalTktIssueDT() : trx.ticketingDate();

    CurrencyConversionRequest request(
        targetAmt, sourceAmt, tktDate, *trx.getRequest(), trx.dataHandle(), false);

    NUCCurrencyConverter ncc;
    if (ncc.convert(request, nullptr))
      return targetAmt.value();
  }
  else
  {
    CurrencyConversionFacade ccFacade;
    if (ccFacade.convert(targetAmt, sourceAmt, trx, itin.geoTravelType() != GeoTravelType::UnknownGeoTravelType))
      return targetAmt.value();
  }

  return 0;
}

static Fare*
createFare(PricingTrx& trx, const Itin& itin, const FareMarket& fm, const FareSettings& settings)
{
  TariffCrossRefInfo* const tcrInfo = createTCRInfo(trx, fm);

  if (UNLIKELY(!tcrInfo))
    return nullptr;

  FareInfo* const fareInfo = createFareInfo(trx, fm, settings);

  if (UNLIKELY(!fareInfo))
    return nullptr;

  Fare* const fare = trx.dataHandle().create<Fare>();
  fare->nucFareAmount() = determineNucFareAmount(trx, itin, *fareInfo, settings);
  fare->initialize(Fare::FS_PublishedFare, fareInfo, fm, tcrInfo);
  fare->nucOriginalFareAmount() =
      fare->isRoundTrip() ? fare->nucFareAmount() * 2 : fare->nucFareAmount();

  return fare;
}

inline static FareClassAppInfo*
createFCAInfo(PricingTrx& trx)
{
  FareClassAppInfo* const fcaInfo = trx.dataHandle().create<FareClassAppInfo>();
  fcaInfo->_fareType = DUMMY_FARE_TYPE;
  return fcaInfo;
}

inline static FareClassAppSegInfo*
createFCAInfoSeg(PricingTrx& trx, const FareSettings& settings)
{
  FareClassAppSegInfo* const fcasInfo = trx.dataHandle().create<FareClassAppSegInfo>();
  fcasInfo->_paxType = settings._pax;
  return fcasInfo;
}

inline static CombinabilityRuleInfo*
createCat10Info(PricingTrx& trx)
{
  CombinabilityRuleInfo* const pCat10 = trx.dataHandle().create<CombinabilityRuleInfo>();

  pCat10->sojInd() = PERMITTED;
  pCat10->dojInd() = PERMITTED;
  pCat10->ct2plusInd() = PERMITTED;
  pCat10->eoeInd() = PERMITTED;
  pCat10->ct2Ind() = 'Y';

  return pCat10;
}

PaxTypeFare*
createPtf(PricingTrx& trx, const Itin& itin, FareMarket& fm, const FareSettings& settings)
{
  DataHandle& dh = trx.dataHandle();

  Fare* const fare = createFare(trx, itin, fm, settings);

  if (UNLIKELY(!fare))
    return nullptr;

  PaxTypeFare* const ptf = dh.create<PaxTypeFare>();

  if (!trx.paxType().empty())
    ptf->actualPaxType() = trx.paxType().front();

  if (fm.primarySector())
    ptf->cabin() = fm.primarySector()->bookedCabin();
  else if (const TravelSeg* ts = TravelSegUtil::firstNoArunkSeg(fm.travelSeg()))
    ptf->cabin() = ts->bookedCabin();

  ptf->setFare(fare);
  ptf->fareMarket() = &fm;
  ptf->fareClassAppInfo() = createFCAInfo(trx);
  ptf->fareClassAppSegInfo() = createFCAInfoSeg(trx, settings);
  ptf->rec2Cat10() = createCat10Info(trx);

  if (settings._privateTariff)
    ptf->setTcrTariffCatPrivate();

  // Reset validation results
  ptf->resetRuleStatus();
  ptf->fare()->resetRuleStatus();
  ptf->skipAllCategoryValidation();
  fare->setRoutingProcessed(true);
  fare->setRoutingValid(true);
  ptf->bookingCodeStatus() = PaxTypeFare::BKS_PASS;

  if (smp::isPenaltyCalculationRequired(trx))
  {
    ptf->initializeMaxPenaltyStructure(dh);
    ptf->addPenaltyInfo(createDummyPenaltyInfo(dh));
    ptf->addVoluntaryChangesInfo(createDummyVoluntaryChangesInfo(dh));
    ptf->addVoluntaryRefundsInfo(createDummyVoluntaryRefundsInfo(dh));
  }

  return ptf;
}

static void
addFareToFareMarket(FareMarket& fm, PaxTypeFare& ptf, const PaxTypeCode requestedPax = "")
{
  fm.allPaxTypeFare().push_back(&ptf);

  for (PaxTypeBucket& ptc : fm.paxTypeCortege())
  {
    if (!requestedPax.empty() && ptc.requestedPaxType()->paxType() != requestedPax)
      continue;
    ptc.paxTypeFare().push_back(&ptf);
  }
}

void
createFaresForExchange(PricingTrx& trx, const Itin& itin, FareMarket& fm)
{
  if (fm.paxTypeCortege().empty())
    return;

  FareSettings settings;
  settings._pax = fm.paxTypeCortege().front().requestedPaxType()->paxType();
  settings._currency = itin.calcCurrencyOverride();
  settings._owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
  settings._convertToCalcCurrency = true;

  PaxTypeFare* const ptf = createPtf(trx, itin, fm, settings);

  if (!ptf)
    return;

  addFareToFareMarket(fm, *ptf);
}

inline static bool
isAdjacentFm(const FareMarket& refFm, const FareMarket& fm)
{
  return refFm.offMultiCity() == fm.boardMultiCity() || refFm.destination() == fm.origin();
}

inline static const FareMarket*
findOppositeThruFm(const Itin& itin, const FareMarket& dummyFm, const bool requireFares)
{
  for (const FareMarket* fm : itin.fareMarket())
  {
    if (requireFares && fm->allPaxTypeFare().empty())
      continue;
    if (isAdjacentFm(dummyFm, *fm) && isAdjacentFm(*fm, dummyFm))
      return fm;
  }
  return nullptr;
}

inline static const FareMarket*
findAdjacentFm(const Itin& itin, const FareMarket& dummyFm)
{
  for (const FareMarket* fm : itin.fareMarket())
    if (isAdjacentFm(dummyFm, *fm))
      return fm;
  return nullptr;
}

inline static bool
fareCanBeDoubled(const FareMarket& fm)
{
  for (const PaxTypeFare* ptf : fm.allPaxTypeFare())
    if (ptf->owrt() == ONE_WAY_MAY_BE_DOUBLED)
      return true;

  return false;
}

static void
setupDummyFm(PricingTrx& trx, const Itin& itin, FareMarket& dummyFm, CurrencyCode& cur)
{
  cur = trx.calendarCurrencyCode();

  const FareMarket* oppositeFm = findOppositeThruFm(itin, dummyFm, false);

  if (!oppositeFm)
    oppositeFm = findAdjacentFm(itin, dummyFm);

  if (!oppositeFm)
    return;

  for (PaxTypeFare* const ptf : oppositeFm->allPaxTypeFare())
  {
    if (dummyFm.direction() == FMDirection::OUTBOUND && ptf->directionality() == FROM)
      continue;

    if (dummyFm.direction() == FMDirection::INBOUND && ptf->directionality() == TO)
      continue;

    if (!ptf->isValid())
      continue;

    dummyFm.setGlobalDirection(oppositeFm->getGlobalDirection());
    cur = ptf->currency();
    break;
  }
}

static void
createAndAddFare(PricingTrx& trx, const Itin& itin, FareMarket& fm, const FareSettings& settings)
{
  PaxTypeFare* const ptf = createPtf(trx, itin, fm, settings);

  if (UNLIKELY(!ptf))
    return;

  addFareToFareMarket(fm, *ptf, settings._pax);
}

static void
updateCurrencyInCorteges(FareMarket& fm, const CurrencyCode cur)
{
  for (PaxTypeBucket& ptc : fm.paxTypeCortege())
  {
    if (fm.direction() != FMDirection::INBOUND)
      ptc.outboundCurrency() = cur;
    if (fm.direction() != FMDirection::OUTBOUND)
      ptc.inboundCurrency() = cur;
  }
}

void
createFaresForOriginBasedRT(PricingTrx& trx, const Itin& itin, FareMarket& fm)
{
  const FareMarket* thruFm = findOppositeThruFm(itin, fm, true);

  const bool createTag1Fare = !thruFm || fareCanBeDoubled(*thruFm);

  CurrencyCode fareCurrency;
  setupDummyFm(trx, itin, fm, fareCurrency);

  FlatSet<PaxTypeCode> processedPaxTypes;

  for (const PaxTypeBucket& ptc : fm.paxTypeCortege())
  {
    const PaxTypeCode& pax = ptc.requestedPaxType()->paxType();

    if (!processedPaxTypes.insert(pax).second)
      continue;

    FareSettings settings;
    settings._pax = pax;
    settings._currency = fareCurrency;
    settings._owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
    settings._privateTariff = trx.getOptions()->isPrivateFares();

    createAndAddFare(trx, itin, fm, settings);

    if (!createTag1Fare)
      continue;

    settings._owrt = ONE_WAY_MAY_BE_DOUBLED;
    createAndAddFare(trx, itin, fm, settings);
  }

  updateCurrencyInCorteges(fm, fareCurrency);
}

const PenaltyInfo* createDummyPenaltyInfo(DataHandle& dh)
{
  PenaltyInfo* penaltyInfo = dh.create<PenaltyInfo>();

  penaltyInfo->cancelRefundAppl() = Penalties::APPLIES;
  penaltyInfo->penaltyRefund() = Penalties::APPLIES;
  penaltyInfo->penaltyCancel() = Penalties::APPLIES;
  penaltyInfo->volAppl() = Penalties::APPLIES;
  penaltyInfo->penaltyReissue() = Penalties::APPLIES;
  penaltyInfo->penaltyNoReissue() = Penalties::APPLIES;

  return penaltyInfo;
}

const VoluntaryChangesInfoW* createDummyVoluntaryChangesInfo(DataHandle& dh)
{
  VoluntaryChangesInfoW* voluntaryChangesInfoW = dh.create<VoluntaryChangesInfoW>();
  VoluntaryChangesInfo* voluntaryChangesInfo = dh.create<VoluntaryChangesInfo>();

  voluntaryChangesInfoW->orig() = voluntaryChangesInfo;

  voluntaryChangesInfo->feeAppl() = ReissuePenaltyCalculator::HIGHEST_OF_CHANGED_FC;

  return voluntaryChangesInfoW;
}

const VoluntaryRefundsInfo* createDummyVoluntaryRefundsInfo(DataHandle& dh)
{
  VoluntaryRefundsInfo* voluntaryRefundsInfo = dh.create<VoluntaryRefundsInfo>();

  return voluntaryRefundsInfo;
}

} /* DummyFareCreator namespace */
} /* tse namespace */
