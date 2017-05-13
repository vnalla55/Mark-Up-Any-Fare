//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Diagnostic/Diag666Collector.h"

#include "Common/GoverningCarrier.h"
#include "Common/SpanishLargeFamilyUtil.h"
#include "Common/SpanishResidentFaresEnhancementUtil.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/Itin.h"
#include "DBAccess/DataHandle.h"
#include "Diagnostic/DiagnosticTable.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"
#include "Pricing/YFlexiValidator.h"

namespace tse
{
Diag666Collector&
Diag666Collector::
operator<<(const Agent& agent)
{
  printHeader("AGENT'S INFO");
  printValue("NATION", agent.agentLocation()->nation());
  printValue("APPLICABLE FOR DISCOUNT", SRFEUtil::isApplicableForPOS(agent));
  return *this;
}

void
Diag666Collector::printPassengerValidation(LocCode residency)
{
  printHeader("PASSENGER'S INFO");
  StateCode residencyState;
  *this << "RESIDENCY: " << residency << std::endl;
  bool applicable = SRFEUtil::isPassengerApplicable(residency, residencyState);
  if (applicable)
    *this << "STATE: " << residencyState << std::endl;
  printValue("APPLICABLE FOR DISCOUNT", applicable);
}

namespace
{
void
printSingleItinerary(Diag666Collector& dc, const PricingTrx& trx, const Itin* itin)
{
  dc.printHeader("ITINERARY");
  if (trx.isMip())
    dc << "ITIN NO: " << itin->itinNum() << '\n';
  dc << "FLIGHTS:" << '\n';

  const auto& segments = itin->travelSeg();
  for (auto seg = segments.cbegin(); seg != segments.cend(); ++seg)
  {
    if (!(*seg)->isAir())
      continue;

    const auto* orig = (*seg)->origin();
    const auto* dest = (*seg)->destination();
    dc << "  " << orig->loc() << "(" << orig->nation() << ")-" << dest->loc() << "("
       << dest->nation() << ")";

    auto nextTS = std::find_if(std::next(seg),
                               segments.cend(),
                               [](const auto* ts)
                               { return ts->isAir(); });

    if (nextTS != segments.cend())
    {
      dc << " CONX: " << DateTime::diffTime((*nextTS)->departureDT(), (*seg)->arrivalDT()) / 60;
      if ((*seg)->destination() != (*nextTS)->origin())
      {
        const auto* nextOrig = (*nextTS)->origin();
        dc << "\n  " << dest->loc() << "(" << dest->nation() << ")-" << nextOrig->loc() << "("
           << nextOrig->nation() << ")"
           << " ARUNK";
      }
    }
    dc << '\n';
  }

  dc << "APPLICABLE FOR DISCOUNT: " << (SRFEUtil::isItinApplicable(*itin) ? "Y" : "N") << '\n';
}

void
printPricingUnit(Diag666Collector& dc, const PUPath& puPath)
{
  dc.printLine("PU:");
  StateCode state;
  for (const PU* pu : puPath.puPath())
  {
    dc << "  " << DiagnosticUtil::pricingUnitTypeToShortString(pu->puType()) << ":";
    for (const MergedFareMarket* mfm : pu->fareMarket())
    {
      if (!mfm->mergedFareMarket().empty())
      {
        const FareMarket* fm = mfm->mergedFareMarket()[0];
        dc << " " << fm->origin()->loc();
        state = SRFEUtil::mapCityToStateItin(fm->origin()->loc());
        if (!state.empty())
          dc << "(" << state << ")";
        dc << "-" << fm->governingCarrier() << "-" << fm->destination()->loc();
        state = SRFEUtil::mapCityToStateItin(fm->destination()->loc());
        if (!state.empty())
          dc << "(" << state << ")";
      }
    }
    dc << "\n";
  }
}

void
printCarriersActivation(Diag666Collector& dc, PricingTrx& trx, const FareMarketPath& fmp)
{
  dc.printLine("CXR ACTIVATION:");
  for (const CarrierCode& carrier : SRFEUtil::getGoverningCarriers(fmp))
    dc.printValue("  " + carrier, trx.isCustomerActivatedByFlag("SRF", &carrier));
}

bool
hasItinWithNumberFromDiagArg(const PricingTrx& trx, const Itin& itin, const std::string value)
{
  if (!trx.diagnostic().diagParamMapItem(value).empty() &&
      trx.diagnostic().diagParamMapItem(value) != std::to_string(itin.itinNum()))
  {
    return std::any_of(itin.getSimilarItins().cbegin(),
                       itin.getSimilarItins().cend(),
                        [&](const auto similars) {
      return trx.diagnostic().diagParamMapItem(value) == std::to_string(similars.itin->itinNum());
    });
  }
  return true;
}

void
printItinNumbers(Diag666Collector& dc, const Itin& itin)
{
  std::vector<IntIndex> similarItinNums;
  for (const auto similars : itin.getSimilarItins())
    similarItinNums.push_back(similars.itin->itinNum());

  if (itin.itinNum() != INVALID_INT_INDEX)
  {
    dc.printValue("ITIN NO", itin.itinNum());
    dc.printValues("SIMILAR ITINS NO", similarItinNums);
  }
}

} // namespace

void
Diag666Collector::printItinValidation(const PricingTrx& trx)
{
  for (const auto* itin : trx.itin())
  {
    if (nullptr == itin)
      continue;

    printSingleItinerary(*this, trx, itin);
    for (const auto similars : itin->getSimilarItins())
      printSingleItinerary(*this, trx, similars.itin);
  }
}

void
Diag666Collector::printFarePathSolutions(const FarePath& farePath,
                                         const PricingTrx& trx,
                                         PUPath& puPath)
{
  if (!trx.diagnostic().diagParamMapItemPresent("FP"))
    return;

  const Itin& itin = *farePath.itin();

  if (!hasItinWithNumberFromDiagArg(trx, itin, "FP"))
    return;

  printHeader("FARE PATH SOLUTION");
  printItinNumbers(*this, itin);

  const uint32_t originDestinationColumnWidth = 7;
  const uint32_t amountColumnWidth = 11;
  const uint32_t carrierColumnWidth = 3;
  const uint32_t percentColumnWidth = 4;

  DiagnosticTable diagTable(trx);
  diagTable.addColumn("ORI-DES", originDestinationColumnWidth)
           .addColumn("VAL CXR", carrierColumnWidth)
           .addColumn("GOV CXR", carrierColumnWidth)
           .addColumn("FARE AMOUNT", amountColumnWidth).setRightAlign()
           .addColumn("SFR AMOUNT", amountColumnWidth).setRightAlign()
           .addColumn("LF DISC", percentColumnWidth).setRightAlign()
           .addColumn("TOT DISC", percentColumnWidth).setRightAlign()
           .addColumn("DISC AMOUNT", amountColumnWidth).setRightAlign();

  const CarrierCode validatingCarrier = farePath.getValidatingCarrier();

  std::vector<std::string> fareBasisCodes;

  for (const PricingUnit* pu : farePath.pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      const PaxTypeFare& paxTypeFare = *fu->paxTypeFare();
      fareBasisCodes.push_back(paxTypeFare.createFareBasis(nullptr));

      const FareMarket& fm = *paxTypeFare.fareMarket();


      const bool shouldReverseOrigDest = paxTypeFare.directionality() == Directionality::TO;
      if (shouldReverseOrigDest)
        diagTable << paxTypeFare.destination() >> "-" >> paxTypeFare.origin();
      else
        diagTable << paxTypeFare.origin() >> "-" >> paxTypeFare.destination();

      MoneyAmount discountAmount  = fu->getSpanishResidentDiscountAmt();
      MoneyAmount totalFareAmount = paxTypeFare.totalFareAmount();
      MoneyAmount spanishAmount   = puPath.findSpanishResidentAmount(fm,
                                                                     fm.governingCarrier(),
                                                                     validatingCarrier);

      const CurrencyCode calculationCurrency = puPath.itin()->calculationCurrency();
      CurrencyCode displayCurrency = calculationCurrency;
      if (trx.diagnostic().diagParamMapItemPresent("CU"))
      {
        displayCurrency = trx.diagnostic().diagParamMapItem("CU");

        auto convert = [&](MoneyAmount& amount) {
          amount = PricingUtil::convertCurrency(trx, amount, displayCurrency, calculationCurrency);
        };

        convert(discountAmount);
        convert(totalFareAmount);
        convert(spanishAmount);
      }

      diagTable << validatingCarrier
                << fm.governingCarrier()
                << totalFareAmount >> " " >> displayCurrency
                << spanishAmount >> " " >> displayCurrency
                << SLFUtil::getDiscountPercent(*trx.getOptions()) >> "%"
                << 100 * SRFEUtil::SPANISH_DISCOUNT + SLFUtil::getDiscountPercent(*trx.getOptions()) >> "%"
                << discountAmount >> " " >> displayCurrency;
      diagTable.nextRow();
    }
  }
  *this << diagTable.toString();
  printValues("FARE BASIS CODES", fareBasisCodes);
}

void
Diag666Collector::printSpanishResidentAmount(PricingTrx& trx, PUPath& puPath)
{
  PUPath tmpPuPath;
  tmpPuPath.puPath().assign(puPath.puPath().begin(), puPath.puPath().end());
  tmpPuPath.itin() = puPath.itin();

  YFlexiValidator yFlexiValidator(trx, tmpPuPath, tmpPuPath.itin()->originationCurrency());
  yFlexiValidator.updDiscAmountBoundary();

  if(puPath.getSpanishResidentAmount().size() != tmpPuPath.getSpanishResidentAmount().size())
    return;
  auto pos = puPath.getSpanishResidentAmount().begin();

  for(const auto& values : tmpPuPath.getSpanishResidentAmount())
  {
    std::stringstream line;

    line << std::get<0>(values.first) << "-" << std::get<1>(values.first) << "  GOV: "
        << std::get<2>(values.first) << "  VAL: " << std::get<3>(values.first) << "  AMT: ";
    std::string value;
    line >> value;
    line << puPath.itin()->originationCurrency() << values.second << " "
        << puPath.itin()->calculationCurrency() << pos->second;

    printValue(value, line.str());
    ++pos;
  }
}

void
Diag666Collector::printSolutionPatternInfo(PricingTrx& trx,
                                           PUPath& puPath,
                                           const Itin& itin)
{
  if (!trx.diagnostic().diagParamMapItemPresent("SP"))
    return;

  if (!hasItinWithNumberFromDiagArg(trx, itin, "SP"))
    return;

  printHeader("SOLUTION PATTERN");
  printItinNumbers(*this, itin);

  printPricingUnit(*this, puPath);

  printCarriersActivation(*this, trx, *puPath.fareMarketPath());

  printValue("SURFACES", SRFEUtil::hasSpanishGovArunk(*puPath.fareMarketPath()));
  printValue("LONG CONNECTIONS", SRFEUtil::hasLongConnection(*puPath.fareMarketPath()));
  printValue("ORG/DST VALID", SRFEUtil::hasValidResidency(trx,
                                                          *puPath.fareMarketPath(),
                                                          trx.residencyState()));

  printValue("APPLICABLE FOR DISCOUNT", SRFEUtil::isSolutionPatternApplicable(trx, *puPath.fareMarketPath()));

  printValue("SPANISH REFERENCE DATA", puPath.isSRFApplicable());

  if (puPath.isSRFApplicable())
    printSpanishResidentAmount(trx, puPath);
}

} // tse
