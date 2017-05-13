#include "Diagnostic/Diag202Collector.h"

#include "Common/Rec2Filter.h"
#include "Common/Rec2Selector.h"
#include "Common/LocUtil.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag225Collector.h"

#include <string>

namespace tse
{
const std::string Diag202Collector::FBR = "FBR";
const std::string Diag202Collector::FNT = "FNT";
const std::string Diag202Collector::GFR = "GFR";

Diag202Collector*
Diag202Collector::makeMe(PricingTrx& trx,
                         const std::string* r2id,
                         const FareMarket* fareMarket,
                         const PaxTypeFare* ptf)
{
  if (LIKELY(trx.diagnostic().diagnosticType() != Diagnostic202))
    return nullptr;

  if (r2id && trx.diagnostic().diagParamMapItemPresent(Diagnostic::RULE_NUMBER) &&
      !trx.diagnostic().diagParamIsSet(Diagnostic::RULE_NUMBER, *r2id))
    return nullptr;

  if (fareMarket && !trx.diagnostic().shouldDisplay(*fareMarket))
    return nullptr;

  if (ptf && !trx.diagnostic().shouldDisplay(*ptf))
    return nullptr;

  DCFactory* factory = DCFactory::instance();
  Diag202Collector* dc = static_cast<Diag202Collector*>(factory->create(trx));
  dc->enable(Diagnostic202);
  return dc;
}

namespace
{
template <class Rec2Type>
void
printR2Details(Diag202Collector& dc,
               const Rec2Type* rule,
               const std::string& failCode)
{
  dc << " VCTR:";
  dc << rule->vendorCode();
  dc << "/" << rule->carrierCode();
  dc << "/" << rule->tariffNumber();
  dc << "/" << rule->ruleNumber();
  dc << " SEQ:" << rule->sequenceNumber();
  dc << " CAT:" << rule->categoryNumber();
  dc << " FAIL:" << failCode << std::endl;

  dc << "  EFF: " << rule->effDate().toIsoExtendedString()
     << " DISC: " << rule->discDate().toIsoExtendedString()
     << " EXPIRE: " << rule->expireDate().toIsoExtendedString() << std::endl;

  dc << "  LOC1: " << rule->loc1().loc() << " " << rule->loc1().locType()
     << " LOC2: " << rule->loc2().loc() << " " << rule->loc2().locType()
     << " JOINT CXR TBL: " << rule->jointCarrierTblItemNo() << std::endl;
}

void
printMatchLocations(Diag202Collector& dc, Rec2Filter::LocFilter& f, const FareByRuleCtrlInfo& r2)
{
  dc << "   " << Diag202Collector::FBR << " " << Diag202Collector::GFR
     << " NUMBER:" << r2.generalRule() << Diag202Collector::GFR
     << " TARIFF:" << r2.generalRuleTariff() << " SEGCOUNT:" << r2.segCnt()
     << " MATCH LOC:" << !f(&r2) << " SWAPPED:" << f._isLocationSwapped << std::endl;
}

void
printMatchLocations(Diag202Collector& dc, Rec2Filter::LocFilter& f, const FootNoteCtrlInfo& r2)
{
  dc << "   " << Diag202Collector::FNT << " " << r2.footNote() << " CAT:" << r2.category()
     << " SEGCOUNT:" << r2.segcount() << std::endl;

  dc << "   GEO STRAIGHT MATCH ORIGIN:" << f.isInLoc(*(f._fareMarket.origin()), r2.loc1(), r2)
     << " DESTINATION:" << f.isInLoc(*(f._fareMarket.destination()), r2.loc2(), r2) << std::endl;

  dc << "    GEO SWAPPED MATCH ORIGIN:" << f.isInLoc(*(f._fareMarket.origin()), r2.loc2(), r2)
     << " DESTINATION:" << f.isInLoc(*(f._fareMarket.destination()), r2.loc1(), r2)
     << " SWAPPED:" << f._isLocationSwapped << std::endl;
}

void
printMatchLocations(Diag202Collector& dc, Rec2Filter::LocFilter& f, const GeneralFareRuleInfo& r2)
{
  dc << "   " << Diag202Collector::GFR << " FARE CLASS:" << r2.fareClass()
     << " SEGCOUNT:" << r2.segcount() << " SWAPPED:" << f._isLocationSwapped << std::endl;
}

template <class Rec2Type>
bool
anyMatchedHas(const PricingTrx& trx,
              const std::string& filter,
              const std::vector<std::pair<const Rec2Type*, GeoMatchResult>>& resultVec)
{
  for (const std::pair<const Rec2Type*, GeoMatchResult>& r2p : resultVec)
  {
    if (filter == Diagnostic::SEQ_NUMBER &&
        trx.diagnostic().diagParamIsSet(Diagnostic::SEQ_NUMBER,
                                        std::to_string(r2p.first->sequenceNumber())))
      return true;

    if (filter == Diagnostic::SPECIFIC_CATEGORY &&
        trx.diagnostic().diagParamIsSet(Diagnostic::SPECIFIC_CATEGORY,
                                        std::to_string(r2p.first->categoryNumber())))
      return true;
  }
  return false;
}

const std::vector<FareByRuleCtrlInfo*>&
getFullCandidatesList(PricingTrx& trx, const FareByRuleCtrlInfo& fbrApp)
{
  return trx.dataHandle().getAllFareByRuleCtrl(
      fbrApp.vendorCode(), fbrApp.carrierCode(), fbrApp.tariffNumber(), fbrApp.ruleNumber());
}

const std::vector<FootNoteCtrlInfo*>&
getFullCandidatesList(PricingTrx& trx, const FootNoteCtrlInfo& gfr)
{
  return trx.dataHandle().getAllFootNoteCtrl(gfr.vendorCode(),
                                             gfr.carrierCode(),
                                             gfr.tariffNumber(),
                                             gfr.footNote(),
                                             gfr.categoryNumber());
}

const std::vector<GeneralFareRuleInfo*>&
getFullCandidatesList(PricingTrx& trx, const GeneralFareRuleInfo& gfr)
{
  return trx.dataHandle().getAllGeneralFareRule(gfr.vendorCode(),
                                                gfr.carrierCode(),
                                                gfr.tariffNumber(),
                                                gfr.ruleNumber(),
                                                gfr.categoryNumber());
}
}

template <class Rec2Type>
void
Diag202Collector::printR2sMatchDetails(
    const std::string& r2Type,
    PricingTrx& trx,
    const std::vector<Rec2Type*>& r2vec,
    const std::vector<std::pair<const Rec2Type*, GeoMatchResult>>& resultVec,
    const FareMarket& fareMarket,
    const CarrierCode& cxr,
    const DateTime& travelDate,
    const PaxTypeFare* ptf)
{
  Diag202Collector& dc = *this;

  if (r2vec.empty() || !dc.isActive())
    return;

  if (trx.diagnostic().diagParamMapItemPresent(Diagnostic::SEQ_NUMBER) &&
      !anyMatchedHas(trx, Diagnostic::SEQ_NUMBER, resultVec))
    return;

  if (trx.diagnostic().diagParamMapItemPresent(Diagnostic::SPECIFIC_CATEGORY) &&
      !anyMatchedHas(trx, Diagnostic::SPECIFIC_CATEGORY, resultVec))
    return;

  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << std::endl;
  dc << "*** " << r2Type << " MATCHING FOR " << cxr << " ***" << std::endl;
  dc << "MARKET:";
  dc << fareMarket.boardMultiCity();
  dc << "-";
  dc << fareMarket.governingCarrier();
  dc << "-";
  dc << fareMarket.offMultiCity();
  dc << " " << DiagnosticUtil::geoTravelTypeToString(fareMarket.geoTravelType()) << std::endl;

  if (ptf)
    dc << "FARE:" << ptf->createFareBasis(&trx) << " REVERSED:" << ptf->isReversed() << std::endl;

  dc << "TRAVEL:" << travelDate.toIsoExtendedString();
  dc << " TICKET:" << trx.dataHandle().ticketDate().toIsoExtendedString();
  dc << std::endl;

  // below guy to show how matching actually go
  Rec2Filter::CompoundFilter<Rec2Type> filter(trx, fareMarket, travelDate);

  if (!resultVec.empty())
    dc << "--- MATCHED RECORD 2 ---" << std::endl;

  for (const std::pair<const Rec2Type*, GeoMatchResult>& r2p : resultVec)
  {
    filter._locFilter(r2p.first);
    printR2Details(dc, r2p.first, Diag225Collector::R2_PASS);
    printMatchLocations(*this, filter._locFilter, *r2p.first);
    dc << " SWAPPED:" << r2p.second << std::endl;
  }

  if (trx.diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, "ALL"))
  {
    // little trick here: use first to get them all, since part of them was already discarded by
    // previous
    // filtering with the same key
    const std::vector<Rec2Type*>& allCandidates = getFullCandidatesList(trx, *r2vec.front());

    dc << "--- RECORD 2 CANDIDATES ---" << std::endl;

    for (const Rec2Type* r2 : allCandidates)
    {
      std::string fail = Diag225Collector::R2_PASS;

      if (filter._dateFilter(r2))
        fail = Diag225Collector::R2_FAIL_EFF_DISC_DATE;
      else if (filter._inhibitFilter(r2))
        fail = "INHIBIT";
      else if (filter._locFilter(r2))
        fail = Diag225Collector::R2_FAIL_GEO;
      else if (filter._cxrFilter(r2))
        fail = "JOINT CXR";

      printR2Details(dc, r2, fail);
      printMatchLocations(*this, filter._locFilter, *r2);
    }

    dc << "***   " << allCandidates.size() << " CANDIDATES TOTAL   ***" << std::endl;
  }

  dc.flushMsg();
}

// eat it linker! Om nom nom!
template void
Diag202Collector::printR2sMatchDetails<FareByRuleCtrlInfo>(
    const std::string& r2Type,
    PricingTrx& trx,
    const std::vector<FareByRuleCtrlInfo*>& r2vec,
    const std::vector<std::pair<const FareByRuleCtrlInfo*, GeoMatchResult>>& resultVec,
    const FareMarket& fareMarket,
    const CarrierCode& cxr,
    const DateTime& travelDate,
    const PaxTypeFare* ptf);

template void
Diag202Collector::printR2sMatchDetails<FootNoteCtrlInfo>(
    const std::string& r2Type,
    PricingTrx& trx,
    const std::vector<FootNoteCtrlInfo*>& r2vec,
    const std::vector<std::pair<const FootNoteCtrlInfo*, GeoMatchResult>>& resultVec,
    const FareMarket& fareMarket,
    const CarrierCode& cxr,
    const DateTime& travelDate,
    const PaxTypeFare* ptf);

template void
Diag202Collector::printR2sMatchDetails<GeneralFareRuleInfo>(
    const std::string& r2Type,
    PricingTrx& trx,
    const std::vector<GeneralFareRuleInfo*>& r2vec,
    const std::vector<std::pair<const GeneralFareRuleInfo*, GeoMatchResult>>& resultVec,
    const FareMarket& fareMarket,
    const CarrierCode& cxr,
    const DateTime& travelDate,
    const PaxTypeFare* ptf);
}
