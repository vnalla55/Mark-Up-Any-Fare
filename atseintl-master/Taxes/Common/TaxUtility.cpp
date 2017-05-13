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
//----------------------------------------------------------------------------

#include <algorithm>
#include <vector>

#include "Common/BSRCollectionResults.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/FallbackUtil.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/LocUtil.h"
#include "Common/NUCCollectionResults.h"
#include "Common/TseUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/TaxAkHiFactor.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxSpecConfigReg.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/CalculationDetails.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Taxes/LegacyTaxes/TaxOnTax.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"
#include "DBAccess/DiscountInfo.h"

namespace tse
{

FALLBACK_DECL(taxFixActualPaxType);

namespace taxUtil
{
const char* TAX_CODE_BB3("BB3");
const char* TAX_CODE_MX("MX");
const char* TAX_CODE_RC("RC");
const char* TAX_CODE_YS("YS");
const char* TAX_CODE_UO("UO");
const char* TAX_CODE_XG("XG");
const char* TAX_CODE_NZ("NZ");

enum LocCategory
checkLocCategory(const Loc& loc)
{
  if (LocUtil::isHawaii(loc))
    return HAWAII;

  if (UNLIKELY(LocUtil::isAlaska(loc)))
    return ALASKA;

  if (LocUtil::isUS(loc) && (!LocUtil::isUSTerritoryOnly(loc)))
    return US;

  return OTHER;
}

bool
soldInUS(PricingTrx& trx)
{
  const Loc* pointOfSaleLocation = TrxUtil::saleLoc(trx);
  return LocUtil::isUS(*pointOfSaleLocation) && (!LocUtil::isUSTerritoryOnly(*pointOfSaleLocation));
}

bool
isBufferZone(const Loc& loc)
{
  return (LocUtil::isCanada(loc) || LocUtil::isMexico(loc)) && loc.bufferZoneInd();
}

bool
isStopOver(TravelSeg* currentSeg, TravelSeg* previousSeg)
{
  AirSeg* airSeg = dynamic_cast<AirSeg*>(currentSeg);
  if (!airSeg)
    return true;
  airSeg = dynamic_cast<AirSeg*>(previousSeg);
  if (!airSeg)
    return true;

  if (previousSeg == nullptr || currentSeg == nullptr)
    return true;

  if (previousSeg->isForcedConx())
    return false;

  if (currentSeg->isStopOver(previousSeg, GeoTravelType::International, TravelSeg::TAXES) ||
      previousSeg->isForcedStopOver())
    return true;
  else
    return false;
}

void
findFareBreaks(std::map<uint16_t, FareUsage*>& fareBreaks, const FarePath& farePath)
{
  std::vector<PricingUnit*>::const_iterator pricingUnitI;
  std::vector<FareUsage*>::iterator fareUsageI;

  for (pricingUnitI = farePath.pricingUnit().begin(); pricingUnitI != farePath.pricingUnit().end();
       pricingUnitI++)
  {
    for (fareUsageI = (*pricingUnitI)->fareUsage().begin();
         fareUsageI != (*pricingUnitI)->fareUsage().end();
         fareUsageI++)
    {
      TravelSeg* travelSegBack = (*fareUsageI)->travelSeg().back();
      fareBreaks.insert(std::pair<uint16_t, FareUsage*>(
          farePath.itin()->segmentOrder(travelSegBack) - 1, *fareUsageI));
    }
  }
}

// ----------------------------------------------------------------------------
// Description:  Is hidden stop on airSeg in loc
// ----------------------------------------------------------------------------
bool
hasHiddenStopInLoc(const AirSeg* airSeg, const LocCode& loc)
{
  const std::vector<const Loc*> hiddenStops = airSeg->hiddenStops();

  return std::any_of(hiddenStops.cbegin(),
                     hiddenStops.cend(),
                     [&loc](const Loc* hiddenStop)
                     { return hiddenStop->loc() == loc; });
}

bool
isTransitSeq(const TaxCodeReg& taxCodeReg)
{

  return !taxCodeReg.restrictionTransit().empty() &&
         taxCodeReg.restrictionTransit().front().transitTaxonly() == YES;
}

bool
isTaxOnOC(PricingTrx& trx, TaxCodeReg& taxCodeReg)
{
  utc::TaxBase taxBase(trx, taxCodeReg);
  return taxBase.isTaxOnOC();
}

bool
isUS(const Loc& loc)
{
  return (LocUtil::isUS(loc) && (!LocUtil::isUSTerritoryOnly(loc)));
}

bool
isSurfaceSegmentAFactor(const TaxResponse& taxResponse, const uint16_t& startSeg)
{
  TaxLocIterator locIt;
  locIt.setSkipHidden(true);
  locIt.initialize(*(taxResponse.farePath()));

  locIt.toSegmentNo(startSeg);

  bool prevSurfaceOK = false;

  while (locIt.hasPrevious())
  {
    bool isCurrentUS = isUS(*(locIt.loc()));
    if (!locIt.isPrevSegAirSeg())
    {
      locIt.previous();
      if (isCurrentUS != isUS(*(locIt.loc())))
      {
        if (isCurrentUS)
          prevSurfaceOK = true;
        break;
      }
    }
    else
    {
      locIt.previous();
    }
  }

  if (locIt.hasPrevious() && !prevSurfaceOK)
    return false;

  locIt.toSegmentNo(startSeg);

  while (locIt.hasNext())
  {
    bool isCurrentUS = isUS(*(locIt.loc()));
    if (!locIt.isNextSegAirSeg())
    {
      locIt.next();
      if (isCurrentUS != isUS(*(locIt.loc())))
      {
        if (isCurrentUS && !prevSurfaceOK)
          return true;

        return false;
      }
    }
    else
    {
      locIt.next();
    }
  }

  if (!locIt.hasNext() && prevSurfaceOK)
    return true;

  return false;
}

bool
isMostDistantUS(PricingTrx& trx, TaxResponse& taxResponse)
{
  const Loc* locFrom;
  const Loc* locToO = nullptr;
  const Loc* locToX = nullptr;
  NationCode nationFrom;
  TaxLocIterator locIt;
  locIt.setSkipHidden(true);
  locIt.initialize(*(taxResponse.farePath()));

  locIt.toBack();
  if (isUS(*(locIt.loc())))
    return true;
  locIt.toFront();
  if (isUS(*(locIt.loc())))
    return true;

  bool mirrorImage = false;

  for (; !locIt.isEnd(); locIt.next())
  {
    if (!isUS(*(locIt.loc())))
      nationFrom = locIt.loc()->nation();
    else
      break;
  }

  for (; !locIt.isEnd() && isUS(*(locIt.loc())); locIt.next())
    ;

  if (!locIt.isEnd())
    if (locIt.loc()->nation() == nationFrom)
      mirrorImage = true;

  locIt.toFront();

  locFrom = locIt.loc();
  uint32_t milesO = 0;
  uint32_t milesX = 0;
  std::vector<TravelSeg*> tsv;

  while (locIt.hasNext())
  {
    locIt.next();

    tsv.push_back(locIt.prevSeg());

    const uint32_t curMiles = calculateMiles(trx, taxResponse, *locFrom, *(locIt.loc()), tsv);

    if (trx.diagnostic().diagnosticType() == Diagnostic818)
    {
      std::ostringstream stream;
      stream << curMiles;
      trx.diagnostic().insertDiagMsg(stream.str());
    }

    if (locIt.isStop())
    {
      if (curMiles > milesO)
      {
        locToO = locIt.loc();
        milesO = curMiles;
      }
    }
    else
    {
      if (curMiles > milesX)
      {
        locToX = locIt.loc();
        milesX = curMiles;
      }
    }
  }

  if (locToO)
  {
    if (isUS(*locToO))
    {
      if (trx.diagnostic().diagnosticType() == Diagnostic818)
        trx.diagnostic().insertDiagMsg("\nUS MOST DISTANT\n");
      return true;
    }
    else if (!mirrorImage || (locToX && !isUS(*locToX)) || milesX <= milesO)
    {
      if (trx.diagnostic().diagnosticType() == Diagnostic818)
        trx.diagnostic().insertDiagMsg("\nUS NOT MOST DISTANT\n");
      return false;
    }
  }

  if (mirrorImage)
    return true;

  if (locToX)
  {
    if (isUS(*locToX))
    {
      if (trx.diagnostic().diagnosticType() == Diagnostic818)
        trx.diagnostic().insertDiagMsg("\nUS MOST DISTANT\n");
      return true;
    }
    else
    {
      if (trx.diagnostic().diagnosticType() == Diagnostic818)
        trx.diagnostic().insertDiagMsg("\nUS NOT MOST DISTANT\n");
      return false;
    }
  }

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
    trx.diagnostic().insertDiagMsg("\nUS NOT MOST DISTANT\n");

  return false;
}

bool
isMostDistantUSOld(PricingTrx& trx, TaxResponse& taxResponse)
{
  const Loc* locFrom;
  const Loc* locTo = nullptr;
  TaxLocIterator locIt;
  locIt.setSkipHidden(true);
  locIt.initialize(*(taxResponse.farePath()));

  locIt.toBack();
  if (isUS(*(locIt.loc())))
    return true;
  locIt.toFront();
  if (isUS(*(locIt.loc())))
    return true;

  locFrom = locIt.loc();
  uint32_t miles = 0;
  std::vector<TravelSeg*> tsv;

  while (locIt.hasNext())
  {
    locIt.next();
    tsv.push_back(locIt.prevSeg());

    const uint32_t curMiles = calculateMiles(trx, taxResponse, *locFrom, *(locIt.loc()), tsv);

    if (trx.diagnostic().diagnosticType() == Diagnostic818)
    {
      std::ostringstream stream;
      stream << curMiles;
      trx.diagnostic().insertDiagMsg(stream.str());
    }

    if (curMiles > miles)
    {
      locTo = locIt.loc();
      miles = curMiles;
    }
  }

  if (locTo && isUS(*locTo))
  {
    if (trx.diagnostic().diagnosticType() == Diagnostic818)
      trx.diagnostic().insertDiagMsg("\nUS MOST DISTANT\n");

    return true;
  }

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
    trx.diagnostic().insertDiagMsg("\nUS NOT MOST DISTANT\n");

  return false;
}

MoneyAmount
locateAkFactor(PricingTrx& trx, const Loc* zoneLoc, const LocCode& locCode)
{
  const std::vector<TaxAkHiFactor*>& taxakHiVect =
      trx.dataHandle().getTaxAkHiFactor(locCode, trx.getRequest()->ticketingDT());

  if (!taxakHiVect.empty())
  {
    const TaxAkHiFactor* taxakHiFactor = taxakHiVect.front();
    switch (zoneLoc->alaskazone())
    {
    case 'A':
      return taxakHiFactor->zoneAPercent();

    case 'B':
      return taxakHiFactor->zoneBPercent();

    case 'C':
      return taxakHiFactor->zoneCPercent();

    case 'D':
      return taxakHiFactor->zoneDPercent();

    default:
      break;
    }
  }

  return 0.0;
}

MoneyAmount
locateHiFactor(PricingTrx& trx, const LocCode& locCode)
{
  const std::vector<TaxAkHiFactor*>& taxakHiVect =
      trx.dataHandle().getTaxAkHiFactor(locCode, trx.getRequest()->ticketingDT());
  if (!taxakHiVect.empty())
  {
    const TaxAkHiFactor* taxakHiFactor = taxakHiVect.front();
    return taxakHiFactor->hawaiiPercent();
  }

  return 0.0;
}

uint32_t
calculateMiles(PricingTrx& trx,
               TaxResponse& taxResponse,
               const Loc& market1,
               const Loc& market2,
               std::vector<TravelSeg*>& tsv)
{
  GlobalDirection gd = GlobalDirection::XX;
  DateTime travelDate = taxResponse.farePath()->itin()->travelDate();
  GlobalDirectionFinderV2Adapter::getGlobalDirection(&trx, travelDate, tsv, gd);

  LocCode city1 = market1.city();
  if (city1.empty())
    city1 = market1.loc();

  LocCode city2 = market2.city();
  if (city2.empty())
    city2 = market2.loc();

  if (city1 == city2)
    return 0;

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
  {
    std::ostringstream stream;
    stream << std::endl << city1 << " " << city2 << " ";
    trx.diagnostic().insertDiagMsg(stream.str());
  }

  DataHandle dataHandle;
  const Mileage* mileage(nullptr);
  mileage = dataHandle.getMileage(city1, city2, TPM, gd, travelDate);
  if (mileage)
  {
    return mileage->mileage();
  }
  else
  {
    mileage = dataHandle.getMileage(city1, city2, MPM, gd, travelDate);
    if (mileage)
    {
      return TseUtil::getTPMFromMPM(mileage->mileage());
    }
  }

  const std::vector<Mileage*>& l = dataHandle.getMileage(city1, city2, travelDate, TPM);
  if (l.empty())
  {
    if (trx.diagnostic().diagnosticType() == Diagnostic818)
      trx.diagnostic().insertDiagMsg("GC ");

    return TseUtil::greatCircleMiles(market1, market2);
  }

  if (trx.diagnostic().diagnosticType() == Diagnostic818)
    trx.diagnostic().insertDiagMsg("NO GD ");

  std::vector<Mileage*>::const_iterator i = l.begin();
  uint32_t mils = (*i)->mileage();
  i++;
  for (; i != l.end(); i++)
    if ((*i)->mileage() < mils)
      mils = (*i)->mileage();

  return mils;
}

bool doUsTaxesApplyOnYQYR(const PricingTrx& trx, const FarePath& farePath)
{
  if(!TrxUtil::areUSTaxesOnYQYREnabled(trx))
    return false;

  for (PricingUnit* pu : farePath.pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      const PaxTypeFare* ptf(fu->paxTypeFare());

      if (!ptf->isFareByRule()) //no cat25
        return false;

      if (ptf->fbrApp().tktDesignator().empty() && ptf->fareByRuleInfo().tktDesignator().empty())
        return false;

      if (fu->totalFareAmount() - fu->surchargeAmt() > EPSILON)
        return false;
    }
  }
  return true;
}

bool doesUS2Apply(uint8_t startIndex, uint8_t endIndex, const PricingTrx& trx, const TaxResponse& taxResponse, const TaxCodeReg& taxCodeReg)
{
  if (taxResponse.farePath()->getTotalNUCAmount() > EPSILON ||
      trx.getRequest()->equivAmountOverride() > EPSILON)
      return true;

    CalculationDetails calculationDetails;
    TaxSplitDetails taxSplitDetails;
    TaxOnTax tOnT(calculationDetails, taxSplitDetails);
    tOnT.setRequireTaxOnTaxSeqMatch(false);
    tOnT.setIndexRange(std::make_pair(startIndex, endIndex));

    std::vector<TaxItem *> taxOnTaxItems;

    MoneyAmount amt = tOnT.calculateTaxFromTaxItem(taxResponse,
                                                   taxCodeReg.taxOnTaxCode(),
                                                   taxCodeReg.nation(),
                                                   taxOnTaxItems,
                                                   false);
    if (amt == 0)
      return false;

    return true;
}

bool
isAnciliaryItin(const PricingTrx& pricingTrx, const Itin& itin)
{
  const TaxTrx* taxTrx = dynamic_cast<const TaxTrx*>(&pricingTrx);

  if (UNLIKELY(taxTrx))
  {
    return ((taxTrx->requestType() == tse::OTA_REQUEST) && (!itin.anciliaryServiceCode().empty()));
  }

  return false;
}

const PaxType*
findActualPaxType(PricingTrx& trx, const FarePath* farePath, const uint16_t startIndex)
{
  TravelSeg* segOnIndex = farePath->itin()->travelSeg()[startIndex];
  const PaxType* requestedPaxType = farePath->paxType();

  for (const PricingUnit* pu : farePath->pricingUnit())
    for (const FareUsage* fu : pu->fareUsage())
      for (const TravelSeg* ts : fu->travelSeg())
      {
        if(ts == segOnIndex)
        {
          if (fu->paxTypeFare()->actualPaxType()->paxType().equalToConst("ADT") && fu->paxTypeFare()->isDiscounted())
          {
            try
            {
              if (fu->paxTypeFare()->discountInfo().category() == 19)
                return requestedPaxType;
            }
            catch (...) {}
          }

          if (requestedPaxType->paxType() == fu->paxTypeFare()->actualPaxType()->paxType() &&
              !fallback::taxFixActualPaxType(&trx))
            return requestedPaxType;

          return fu->paxTypeFare()->actualPaxType();
        }
     }
  return requestedPaxType;
}

MoneyAmount
convertCurrency(PricingTrx& trx,
                MoneyAmount moneyAmount,
                const CurrencyCode& paymentCurrency,
                const CurrencyCode& calculationCurrency,
                const CurrencyCode& baseFareCurrency,
                CurrencyConversionRequest::ApplicationType applType,
                bool useInternationalRounding)
{
  CurrencyConversionFacade ccFacade;
  BSRCollectionResults resultBsr;
  NUCCollectionResults resultNuc;
  resultNuc.collect() = true;
  CurrencyCollectionResults *result = nullptr;

  Money targetMoney(paymentCurrency);
  targetMoney.value() = 0;

  bool bDiag = (trx.diagnostic().diagnosticType() == Diagnostic818) &&
      trx.diagnostic().diagParamMapItemPresent("DC");

  if (calculationCurrency != baseFareCurrency)
  {
    Money targetMoneyOrigination(baseFareCurrency);
    targetMoneyOrigination.value() = 0;

    Money sourceMoneyCalculation(moneyAmount, calculationCurrency);

    if (bDiag)
      result = targetMoneyOrigination.isNuc() || sourceMoneyCalculation.isNuc() ?
          static_cast<CurrencyCollectionResults *>(&resultNuc) :
          static_cast<CurrencyCollectionResults *>(&resultBsr);

    ccFacade.convert(targetMoneyOrigination,
                     sourceMoneyCalculation,
                     trx,
                     useInternationalRounding,
                     applType,
                     false,
                     result);

    if (bDiag)
      addConversionDetailsToDiag(trx, sourceMoneyCalculation, targetMoneyOrigination,
                                 resultBsr, resultNuc);

    moneyAmount = targetMoneyOrigination.value();
  }

  if (baseFareCurrency!= paymentCurrency)
  {
    ccFacade.setRoundFare(true);

    Money sourceMoney(moneyAmount, baseFareCurrency);

    if (bDiag)
      result = targetMoney.isNuc() || sourceMoney.isNuc() ?
          static_cast<CurrencyCollectionResults *>(&resultNuc) :
          static_cast<CurrencyCollectionResults *>(&resultBsr);

    ccFacade.convert(targetMoney,
                     sourceMoney,
                     trx,
                     useInternationalRounding,
                     applType,
                     false,
                     result);

    if (bDiag)
      addConversionDetailsToDiag(trx, sourceMoney, targetMoney, resultBsr, resultNuc);

    moneyAmount = targetMoney.value();
  }

  return moneyAmount;
}

void
addConversionDetailsToDiag(PricingTrx& trx,
                           const Money& sourceMoney,
                           const Money& targetMoney,
                           const BSRCollectionResults& resultBsr,
                           const NUCCollectionResults& resultNuc)
{
  std::ostringstream stream;
  stream.setf(std::ios::fixed, std::ios::floatfield);
  stream.precision(4);

  stream << "CONVERT " << sourceMoney.value() << sourceMoney.code()
      << " TO " << targetMoney.value() << targetMoney.code() << " RATE:"
      << (targetMoney.isNuc() || sourceMoney.isNuc() ?
         resultNuc.exchangeRate() : resultBsr.exchangeRate1())
      << std::endl;
  trx.diagnostic().insertDiagMsg(stream.str());
}

const std::vector<PaxTypeFare*>*
locatePaxTypeFare(const FareMarket* fareMarketReTrx,
                  const PaxTypeCode& paxTypeCode)
{
  for (const PaxTypeBucket& paxTypeCortege : fareMarketReTrx->paxTypeCortege())
  {
    if (paxTypeCortege.requestedPaxType()->paxType() == paxTypeCode)
      return &(paxTypeCortege.paxTypeFare());
  }

  return nullptr;
}

bool
isYQorYR(const TaxCode& code)
{
  return code.length() >= 2 && code[0] == 'Y'
      && (code[1] == 'Q' || code[1] == 'R');
}

bool
isGstTax(const TaxCode& code)
{
  return ((strncmp(code.c_str(), TAX_CODE_BB3, 3) == 0) ||
      (strncmp(code.c_str(), TAX_CODE_MX, 2) == 0) ||
      (strncmp(code.c_str(), TAX_CODE_NZ, 2) == 0) ||
      (strncmp(code.c_str(), TAX_CODE_RC, 2) == 0) ||
      (strncmp(code.c_str(), TAX_CODE_YS, 2) == 0) ||
      (strncmp(code.c_str(), TAX_CODE_UO, 2) == 0) ||
      (strncmp(code.c_str(), TAX_CODE_XG, 2) == 0));
}

} // namespace taxUtil
}
