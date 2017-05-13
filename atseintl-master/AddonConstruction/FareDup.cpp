//-------------------------------------------------------------------
//
//  File:        FareDup.cpp
//  Created:     Jul 9, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Misc. util functions to remove constructed fare
//               duplicates
//
//  Copyright Sabre 2005
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

#include "AddonConstruction/FareDup.h"

#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/ConstructedFare.h"
#include "AddonConstruction/ConstructedFareInfoResponse.h"
#include "AddonConstruction/ConstructionJob.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TSELatencyData.h"
#include "DBAccess/AddonFareInfo.h"
#include "Diagnostic/Diag257Collector.h"
#include "Util/Algorithm/Comparison.h"
#include "Util/BranchPrediction.h"

namespace tse
{
FALLBACK_DECL(removeDynamicCastForAddonConstruction);

static Logger
logger("atseintl.AddonConstruction.FareDup");

/////////////////////////////////////////////////////////////////////
// main interface

FareDup::ComparisonFailureCode
FareDup::isEqual(const ConstructedFareInfo& cfi1, const ConstructedFareInfo& cfi2)
{
  // compare FareInfos

  ComparisonFailureCode cmpResult = compareFareInfo(cfi1.fareInfo(), cfi2.fareInfo());

  if (UNLIKELY(cmpResult != EQUAL))
    return cmpResult;

  // compare directionality

  if (UNLIKELY(cfi1.fareInfo().directionality() != cfi2.fareInfo().directionality()))
    return DIRECTIONALITY_NE;

  // compare routings

  cmpResult = compareRoutings(cfi1, cfi2);

  if (cmpResult != EQUAL)
    return cmpResult;

  // compare footnotes

  cmpResult = compareFootnotes(cfi1, cfi2);

  return cmpResult;
}

FareDup::ComparisonFailureCode
FareDup::isEqualWithAddons(const ConstructedFareInfo& cfi1, const ConstructedFareInfo& cfi2)
{
  const ComparisonFailureCode code = isEqual(cfi1, cfi2);

  if (code != EQUAL)
    return code;

  const bool origAddonsEqual =
      std::abs(cfi1.origAddonAmount() - cfi2.origAddonAmount()) < EPSILON &&
      alg::members_equal(cfi1,
                         cfi2,
                         &ConstructedFareInfo::origAddonFareClass,
                         &ConstructedFareInfo::origAddonTariff,
                         &ConstructedFareInfo::origAddonCurrency,
                         &ConstructedFareInfo::origAddonRouting,
                         &ConstructedFareInfo::origAddonCurrency,
                         &ConstructedFareInfo::origAddonOWRT);

  if (!origAddonsEqual)
    return ORIGADDON_NE;

  const bool destAddonsEqual =
      std::abs(cfi1.destAddonAmount() - cfi2.destAddonAmount()) < EPSILON &&
      alg::members_equal(cfi1,
                         cfi2,
                         &ConstructedFareInfo::destAddonFareClass,
                         &ConstructedFareInfo::destAddonTariff,
                         &ConstructedFareInfo::destAddonCurrency,
                         &ConstructedFareInfo::destAddonRouting,
                         &ConstructedFareInfo::destAddonCurrency,
                         &ConstructedFareInfo::destAddonOWRT);

  if (!destAddonsEqual)
    return DESTADDON_NE;

  return EQUAL;
}

FareDup::ComparisonFailureCode
FareDup::isEqual(const ConstructedFareInfo& cfi, const Fare& fare)
{
  ComparisonFailureCode cmpResult = compareFareInfo(cfi.fareInfo(), *(fare.fareInfo()));

  if (cmpResult != EQUAL)
    return cmpResult;

  return isEqualSimple(cfi, fare);
}

FareDup::ComparisonFailureCode
FareDup::isEqualSimple(const ConstructedFareInfo& cfi, const Fare& fare)
{
  if (cfi.fareInfo().directionality() != fare.fareInfo()->directionality())
    return DIRECTIONALITY_NE;

  ComparisonFailureCode cmpResult = compareRoutings(cfi, fare);

  if (cmpResult != EQUAL)
    return cmpResult;

  cmpResult = compareFootnotes(cfi, fare);

  return cmpResult;
}

void
FareDup::addFIToResponse(ConstructionJob& cj, ConstructedFareInfo* cfi)
{
  TSELatencyData metrics(cj.trx(), "AC DUPLICATE ELIMINATION");

  const bool singleOverDouble = cj.singleOverDouble();
  const bool fareHasFareClassPriority = !cj.isSita();

  const bool isHistorical = cj.isHistorical();

  const boost::gregorian::date ticketingDate(cj.ticketingDate().date());
  const boost::gregorian::date travelDate(cj.travelDate().date());

  // loop via vector of constructed fares to see if fares are equal
  // if not then push back the fare to the end of the vector
  //        else apply special logic to eliminate duplicate fare

  FareDup::DupResolution dupResolution = KEEP_GOING;

  const auto cfSetIter = cj.response().responseHashSet().find(cfi);

  if (cfSetIter != cj.response().responseHashSet().end())
  {
    Diag257Collector* diag257{nullptr};
    if (!fallback::removeDynamicCastForAddonConstruction(&cj.trx()))
    {
      diag257 = cj.diagnostic<Diag257Collector>();
    }
    else
    {
      diag257 = cj.diag257();
    }

    ConstructedFareInfo* i = *cfSetIter;

    // fares are equal. carrier preferences tell us what to do.
    // but in any event duplicate fare should not be pushed back.

    if (fareHasFareClassPriority)
      dupResolution = resolveFareClassPriority(*i, *cfi, diag257);

    if (dupResolution == KEEP_GOING && singleOverDouble)
      dupResolution = singleVsDouble(*i, *cfi, diag257);

    // check special conditions (createDate/expireDate) for
    // historical add-on construction.

    if (UNLIKELY(isHistorical && dupResolution != KEEP_GOING))
      checkHistoricalIntervals(dupResolution, *i, *cfi, ticketingDate, travelDate, diag257);

    if (dupResolution == KEEP_GOING)
      dupResolution = keepLowestFare(*i, *cfi, diag257);

    // keep or replace existed fare

    if (dupResolution == KEEP_EXISTED)
      return; // do nothing. we don't need new fare

    if (LIKELY(dupResolution == REPLACE_EXISTED))
    {
      // we DO NEED new fare. BUT it should not be
      // pushed back. the current fare should be
      // replaced with new instead

      cj.response().responseHashSet().erase(cfSetIter);
      cj.response().responseHashSet().insert(cfi);
      return;
    }
  }

  if (LIKELY(dupResolution == KEEP_GOING))
    cj.response().responseHashSet().insert(cfi);
}

FareDup::DupResolution
FareDup::resolveFareClassPriority(const ConstructedFareInfo& cfi1,
                                  const ConstructedFareInfo& cfi2,
                                  Diag257Collector* diag257)
{
  // compare fare classes priorities. if they are not equal
  // fare with highest fare classes priority should be kept
  // REGARDLES of price.

  if (cfi1.gateway1() != cfi2.gateway1() || cfi1.gateway2() != cfi2.gateway2())
    return KEEP_GOING;

  if (cfi1.atpFareClassPriority() > cfi2.atpFareClassPriority())
  {
    if (UNLIKELY(diag257 != nullptr))
      diag257->writeDupDetail(cfi1, cfi2, Diag257Collector::DRR_FARE_CLASS_PRIORITY);

    return KEEP_EXISTED;
  }

  if (cfi1.atpFareClassPriority() < cfi2.atpFareClassPriority())
  {
    if (UNLIKELY(diag257 != nullptr))
      diag257->writeDupDetail(cfi2, cfi1, Diag257Collector::DRR_FARE_CLASS_PRIORITY);

    return REPLACE_EXISTED;
  }

  return KEEP_GOING;
}

FareDup::DupResolution
FareDup::singleVsDouble(const ConstructedFareInfo& cfi1,
                        const ConstructedFareInfo& cfi2,
                        Diag257Collector* diag257)
{
  // check for single-ended fares vs. double-ended fare.
  // single-ended fare should be kept REGARDLES of price

  if (cfi1.constructionType() != ConstructedFareInfo::DOUBLE_ENDED &&
      cfi2.constructionType() == ConstructedFareInfo::DOUBLE_ENDED)
  {
    if (UNLIKELY(diag257 != nullptr))
      diag257->writeDupDetail(cfi1, cfi2, Diag257Collector::DRR_SINGLE_OVER_DBL);

    return KEEP_EXISTED;
  }

  if (UNLIKELY(cfi1.constructionType() == ConstructedFareInfo::DOUBLE_ENDED &&
      cfi2.constructionType() != ConstructedFareInfo::DOUBLE_ENDED))
  {
    if (UNLIKELY(diag257 != nullptr))
      diag257->writeDupDetail(cfi2, cfi1, Diag257Collector::DRR_SINGLE_OVER_DBL);

    return REPLACE_EXISTED;
  }

  return KEEP_GOING;
}

void
FareDup::checkHistoricalIntervals(DupResolution& dupResolution,
                                  const ConstructedFareInfo& cfi1,
                                  const ConstructedFareInfo& cfi2,
                                  const boost::gregorian::date& ticketingDate,
                                  const boost::gregorian::date& travelDate,
                                  Diag257Collector* diag257)
{
  // check to see if ticketingDate == createDate for high priority fare
  // if so then special logic for historical construction applays
  // and we need to keep lowest (not high priority!) fare

  if (dupResolution == KEEP_EXISTED)
  {
    // cfi1 is high priority fare

    if (ticketingDate == cfi1.fareInfo().createDate().date() ||
        ticketingDate == cfi1.fareInfo().effDate().date())
      dupResolution = KEEP_GOING;

    else if (travelDate == cfi1.fareInfo().expireDate().date() ||
             travelDate == cfi1.fareInfo().discDate().date())
      dupResolution = KEEP_GOING;
  }

  else if (dupResolution == REPLACE_EXISTED)
  {
    // cfi2 is high priority fare

    if (ticketingDate == cfi2.fareInfo().createDate().date() ||
        ticketingDate == cfi2.fareInfo().effDate().date())
      dupResolution = KEEP_GOING;

    else if (travelDate == cfi2.fareInfo().expireDate().date() ||
             travelDate == cfi2.fareInfo().discDate().date())
      dupResolution = KEEP_GOING;
  }
}

FareDup::DupResolution
FareDup::keepLowestFare(const ConstructedFareInfo& cfi1,
                        const ConstructedFareInfo& cfi2,
                        Diag257Collector* diag257)
{
  // define fare with the lowest fare amount and keept it

  MoneyAmount diff = cfi1.constructedNucAmount() - cfi2.constructedNucAmount();

  if (diff > EPSILON)
  {
    if (UNLIKELY(diag257 != nullptr))
      diag257->writeDupDetail(cfi2, cfi1, Diag257Collector::DRR_LESS_FARE_AMOUNT);

    return REPLACE_EXISTED;
  }

  if (UNLIKELY(diag257 != nullptr))
    diag257->writeDupDetail(cfi1, cfi2, Diag257Collector::DRR_LESS_OR_EQ_FARE_AMOUNT);

  return KEEP_EXISTED;
}

/////////////////////////////////////////////////////////////////////
// compare FareInfo

FareDup::ComparisonFailureCode
FareDup::compareFareInfo(const FareInfo& fi1, const FareInfo& fi2)
{
  if (fi1.fareTariff() != fi2.fareTariff())
    return FARE_TARIFF_NE;

  if (fi1.owrt() != fi2.owrt())
    return OWRT_NE;

  if (UNLIKELY(fi1.globalDirection() != fi2.globalDirection()))
    return GLOBAL_DIR_NE;

  if (fi1.fareClass() != fi2.fareClass())
    return FARE_CLASS_NE;

  if (UNLIKELY(fi1.vendor() != fi2.vendor()))
    return VENDORS_NE;

  if (UNLIKELY(fi1.carrier() != fi2.carrier()))
    return CARRIERS_NE;

  if (fi1.currency() != fi2.currency())
    return CURRENCY_NE;

  if (UNLIKELY(fi1.ruleNumber() != fi2.ruleNumber()))
    return RULE_NUMBER_NE;

  if (UNLIKELY(fi1.market1() != fi2.market1() || fi1.market2() != fi2.market2()))
    return MARKETS_NE;

  return EQUAL;
}

/////////////////////////////////////////////////////////////////////
// routings

FareDup::ComparisonFailureCode
FareDup::compareRoutings(const ConstructedFare& cf1, const ConstructedFare& cf2)
{
  // create array of routing numbers for cf1

  unsigned int cf1RSize = 0;
  RoutingNumber cf1R[3];

  populateRoutingArray(cf1, cf1R, cf1RSize);

  // create array of routing numbers for cf2

  unsigned int cf2RSize = 0;
  RoutingNumber cf2R[3];

  populateRoutingArray(cf2, cf2R, cf2RSize);

  // compare arrays

  return compareRoutingArrays(cf1R, cf1RSize, cf2R, cf2RSize);
}

FareDup::ComparisonFailureCode
FareDup::compareRoutings(const ConstructedFareInfo& cfi1, const ConstructedFareInfo& cfi2)
{
  // create array of routing numbers for cfi1

  unsigned int cfi1RSize = 0;
  RoutingNumber cfi1R[3];

  populateRoutingArray(cfi1, cfi1R, cfi1RSize);

  // create array of routing numbers for cfi2

  unsigned int cfi2RSize = 0;
  RoutingNumber cfi2R[3];

  populateRoutingArray(cfi2, cfi2R, cfi2RSize);

  // compare arrays

  return compareRoutingArrays(cfi1R, cfi1RSize, cfi2R, cfi2RSize);
}

FareDup::ComparisonFailureCode
FareDup::compareRoutings(const ConstructedFareInfo& cfi, const Fare& fare)
{
  // create array of routing numbers for cfi1

  unsigned int cfi1RSize = 0;
  RoutingNumber cfi1R[3];

  populateRoutingArray(cfi, cfi1R, cfi1RSize);

  // create array of routing numbers for published fare

  unsigned int f2RSize = 1;
  RoutingNumber f2R[3];
  f2R[0] = fare.routingNumber();

  // compare arrays

  return compareRoutingArrays(cfi1R, cfi1RSize, f2R, f2RSize);
}

void
FareDup::populateRoutingArray(const ConstructedFare& cf, RoutingNumber r[], unsigned int& rSize)
{
  rSize = 0;

  if (cf.origAddon() != nullptr)
    r[rSize++] = cf.origAddon()->addonFare()->routing();

  r[rSize++] = cf.specifiedFare()->routingNumber();

  if (cf.destAddon() != nullptr)
    r[rSize++] = cf.destAddon()->addonFare()->routing();
}

void
FareDup::populateRoutingArray(const ConstructedFareInfo& cfi,
                              RoutingNumber r[],
                              unsigned int& rSize)
{
  rSize = 0;

  if (cfi.constructionType() == ConstructedFareInfo::SINGLE_ORIGIN ||
      cfi.constructionType() == ConstructedFareInfo::DOUBLE_ENDED)
    r[rSize++] = cfi.origAddonRouting();

  r[rSize++] = cfi.fareInfo().routingNumber();

  if (cfi.constructionType() == ConstructedFareInfo::SINGLE_DESTINATION ||
      cfi.constructionType() == ConstructedFareInfo::DOUBLE_ENDED)
    r[rSize++] = cfi.destAddonRouting();
}

FareDup::ComparisonFailureCode
FareDup::compareRoutingArrays(const RoutingNumber r1[],
                              const unsigned int r1Size,
                              const RoutingNumber r2[],
                              const unsigned int r2Size)
{
  // compare arrays

  unsigned int i;
  if (r1Size == r2Size)
  {
    // ConstructedFareData's are equal if routing numbers and
    // order of their appearence are the same

    for (i = 0; i < r1Size; i++)
      if (r1[i] != r2[i])
        return ROUTINGS_NE;
  }
  else
  {
    // ConstructedFareData's are equal
    // if all routing numbers are the same

    RoutingNumber rn = r1[0];

    for (i = 1; i < r1Size; i++)
      if (rn != r1[i])
        return ROUTINGS_NE;

    for (i = 0; i < r2Size; i++)
      if (rn != r2[i])
        return ROUTINGS_NE;
  }

  return EQUAL;
}

/////////////////////////////////////////////////////////////////////
// footnotes

FareDup::ComparisonFailureCode
FareDup::compareFootnotes(const ConstructedFare& cf1, const ConstructedFare& cf2)
{
  // create a vector of unique pairs
  // tariff/footnote for cf1

  FootnoteTariffVec cf1Footnotes;
  buildFootnoteTariffs(cf1, cf1Footnotes);

  // create a vector of unique pairs
  // tariff/footnote for cf2

  FootnoteTariffVec cf2Footnotes;
  buildFootnoteTariffs(cf2, cf2Footnotes);

  // compare vectors

  if (compareFootnoteTariffs(cf1Footnotes, cf2Footnotes))
    return EQUAL;

  return FOOTNOTES_NE;
}

FareDup::ComparisonFailureCode
FareDup::compareFootnotes(const ConstructedFareInfo& cfi1, const ConstructedFareInfo& cfi2)
{
  // create a vector of unique pairs
  // tariff/footnote for cfi1

  FootnoteTariffVec cfi1Footnotes;
  buildFootnoteTariffs(cfi1, cfi1Footnotes);

  // create a vector of unique pairs
  // tariff/footnote for cfi2

  FootnoteTariffVec cfi2Footnotes;
  buildFootnoteTariffs(cfi2, cfi2Footnotes);

  // compare vectors

  if (compareFootnoteTariffs(cfi1Footnotes, cfi2Footnotes))
    return EQUAL;

  return FOOTNOTES_NE;
}

FareDup::ComparisonFailureCode
FareDup::compareFootnotes(const ConstructedFareInfo& cfi, const Fare& fare)
{
  // create a vector of unique pairs
  // tariff/footnote for cfi

  FootnoteTariffVec cfiFootnotes;
  buildFootnoteTariffs(cfi, cfiFootnotes);

  // create a vector of unique pairs
  // tariff/footnote for fare.
  // in common case, fare can be published or constructed.

  FootnoteTariffVec fareFootnotes;

  const FareInfo* fi2 = fare.fareInfo();
  const ConstructedFareInfo* cfi2 = fare.constructedFareInfo();

  if (UNLIKELY(cfi2 != nullptr))
    buildFootnoteTariffs(*cfi2, fareFootnotes);
  else
    buildFootnoteTariffs(*fi2, fareFootnotes);

  // compare vectors

  if (compareFootnoteTariffs(cfiFootnotes, fareFootnotes))
    return EQUAL;

  return FOOTNOTES_NE;
}

bool
FareDup::buildFootnoteTariffs(const ConstructedFare& cf, FootnoteTariffVec& fT)
{
  const FareInfo& sfi = *(cf.specifiedFare());
  bool skipFT = (sfi.vendor() != SITA_VENDOR_CODE);

  // origin add-on footnotes

  if (cf.origAddon() != nullptr)
  {
    const AddonFareInfo& oa = *(cf.origAddon()->addonFare());

    addFootnoteTariff(sfi.fareTariff(), oa.footNote1(), skipFT, fT);

    addFootnoteTariff(sfi.fareTariff(), oa.footNote2(), skipFT, fT);
  }

  // specified fare footnotes

  buildFootnoteTariffs(sfi, fT);

  // origin add-on footnotes

  if (cf.destAddon() != nullptr)
  {
    const AddonFareInfo& da = *(cf.destAddon()->addonFare());

    addFootnoteTariff(sfi.fareTariff(), da.footNote1(), skipFT, fT);

    addFootnoteTariff(sfi.fareTariff(), da.footNote2(), skipFT, fT);
  }

  return true;
}

bool
FareDup::buildFootnoteTariffs(const ConstructedFareInfo& cfi, FootnoteTariffVec& fT)
{
  const FareInfo& fi = cfi.fareInfo();
  bool skipFT = (fi.vendor() != SITA_VENDOR_CODE);

  // origin add-on footnotes

  addFootnoteTariff(fi.fareTariff(), cfi.origAddonFootNote1(), skipFT, fT);

  addFootnoteTariff(fi.fareTariff(), cfi.origAddonFootNote2(), skipFT, fT);

  // specified fare footnotes

  buildFootnoteTariffs(fi, fT);

  // origin add-on footnotes

  addFootnoteTariff(fi.fareTariff(), cfi.destAddonFootNote1(), skipFT, fT);

  addFootnoteTariff(fi.fareTariff(), cfi.destAddonFootNote2(), skipFT, fT);

  return true;
}

bool
FareDup::buildFootnoteTariffs(const FareInfo& fi, FootnoteTariffVec& fT)
{
  addFootnoteTariff(fi.fareTariff(), fi.footNote1(), false, fT);

  addFootnoteTariff(fi.fareTariff(), fi.footNote2(), false, fT);

  return true;
}

bool
FareDup::compareFootnoteTariffs(const FootnoteTariffVec& fT1, const FootnoteTariffVec& fT2)
{
  // vectors are equal if both contain the same set of
  // footnote/tariff pairs. order of pairs doesn't important

  if (fT1.size() != fT2.size())
    return false;

  for (const auto& elem : fT1)
  {
    if (!findFootnoteTariff(elem, fT2))
      return false;
  }

  return true;
}

void
FareDup::addFootnoteTariff(const TariffNumber& tn,
                           const Footnote& ft,
                           const bool skipFT,
                           FootnoteTariffVec& ftVec)
{
  if (ft.empty())
    return;

  if (ft[0] >= '0' && ft[0] <= '9')
  {
    // first symbol is number, i.e. one and only one footnote
    // is present. it could be 2 symbol footnote but we don't care

    addFootnoteTariff(FootnoteTariff(tn, ft), skipFT, ftVec);
    return;
  }

  // foonote ft[0]

  addFootnoteTariff(FootnoteTariff(tn, &ft[0]), skipFT, ftVec);

  // footnote ft[1]

  if (ft.size() > 1)
    addFootnoteTariff(FootnoteTariff(tn, &ft[1]), skipFT, ftVec);
}

void
FareDup::addFootnoteTariff(const FootnoteTariff& ft, const bool skipFT, FootnoteTariffVec& ftVec)
{
  static const std::string FOOTNOTE_TO = "T";
  static const std::string FOOTNOTE_FROM = "F";

  if (skipFT && (ft.second == FOOTNOTE_TO || ft.second == FOOTNOTE_FROM))
    return;

  if (!findFootnoteTariff(ft, ftVec))
    ftVec.push_back(ft);
}

bool
FareDup::findFootnoteTariff(const FootnoteTariff& ft, const FootnoteTariffVec& ftVec)
{
  FootnoteTariffVecCI i = ftVec.begin();
  for (; i != ftVec.end(); ++i)
  {
    if ((*i) == ft)
      return true;
  }

  return false;
}

} // ns tse
