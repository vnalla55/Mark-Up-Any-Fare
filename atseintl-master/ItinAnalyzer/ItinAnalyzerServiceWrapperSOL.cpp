#include "ItinAnalyzer/ItinAnalyzerServiceWrapperSOL.h"

#include "Common/Assert.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/GoverningCarrier.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TFPUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Diversity.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "Diagnostic/DCFactory.h"
#include "ItinAnalyzer/CodeShareValidatorSOL.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"


#include <algorithm>
#include <iostream>

#include <tr1/unordered_map>

namespace tse
{
FIXEDFALLBACK_DECL(fallbackFMDirectionSetting)
FALLBACK_DECL(fallbackExcludeCodeSharingFix);
FALLBACK_DECL(fallbackFareSelction2016);
FALLBACK_DECL(unskippableCarriersDueToFullyOpen);

namespace
{
static Logger
logger("atseintl.ItinAnalyzer.ItinAnalyzerServiceWrapperSOL");

ConfigurableValue<bool>
excludeLocalFmForArea1_2("SHOPPING_DIVERSITY", "EXCLUDE_LOCALFM_FOR_AREA_1_2", false);
ConfigurableValue<bool>
excludeLocalFmForArea1_3("SHOPPING_DIVERSITY", "EXCLUDE_LOCALFM_FOR_AREA_1_3", false);
ConfigurableValue<bool>
excludeLocalFmForNationUscaArea1("SHOPPING_DIVERSITY",
                                 "EXCLUDE_LOCALFM_FOR_NATION_USCA_AREA_1",
                                 false);
ConfigurableValue<bool>
excludeLocalFmForNationUscaChina("SHOPPING_DIVERSITY",
                                 "EXCLUDE_LOCALFM_FOR_NATION_USCA_CHINA",
                                 false);

ConfigurableValue<bool>
excludeLocalFmForSubArea21Area3("SHOPPING_DIVERSITY",
                                "EXCLUDE_LOCALFM_FOR_SUBAREA_21_AREA_3",
                                false);
ConfigurableValue<bool>
excludeLocalFmForSubArea21SubArea22_23("SHOPPING_DIVERSITY",
                                       "EXCLUDE_LOCALFM_FOR_SUBAREA_21_SUBAREA_22_23",
                                       false);
ConfigurableValue<bool>
excludeLocalFmForSubArea21International("SHOPPING_DIVERSITY",
                                        "EXCLUDE_LOCALFM_FOR_SUBAREA_21_INTERNATIONAL",
                                        false);
ConfigurableValue<bool>
excludeLocalFmForForeignDomestic(
    "SHOPPING_DIVERSITY",
    "EXCLUDE_LOCALFM_FOR_FOREIGN_DOMESTIC_WITH_INTERNATIONAL_CONNECTION",
    false);

ConfigurableValue<ConfigSet<CarrierCode>>
unskippableCarriersOnArea1_Area2(
    "SHOPPING_DIVERSITY", "UNSKIPPABLE_CARRIERS_ON_AREA1_AREA2");

ConfigurableValue<ConfigSet<CarrierCode>>
unskippableCarriersOnSubarea21_Subarea22Or23("SHOPPING_DIVERSITY",
                                             "UNSKIPPABLE_CARRIERS_ON_SUBAREA21_SUBAREA22_OR_23");

ConfigurableValue<ConfigSet<CarrierCode>>
unskippableFlightUSArea220Carriers("SHOPPING_DIVERSITY", "UNSKIPPABLE_FLIGHT_US_AREA220_CARRIERS");

ConfigurableValue<ConfigSet<CarrierCode>>
unskippableCarriersOnNationUSCA_Area1("SHOPPING_DIVERSITY",
                                      "UNSKIPPABLE_CARRIERS_ON_NATION_USCA_AREA1");

ConfigurableValue<ConfigSet<CarrierCode>>
unskippableCarriersOnNationUSCA_China("SHOPPING_DIVERSITY",
                                      "UNSKIPPABLE_CARRIERS_ON_NATION_USCA_CHINA");

ConfigurableValue<ConfigSet<CarrierCode>>
unskippableCarriersOnArea21_Area3(
    "SHOPPING_DIVERSITY", "UNSKIPPABLE_CARRIERS_ON_AREA21_AREA3");

ConfigurableValue<ConfigSet<CarrierCode>>
unskippableCarriersDueToFullyOpen(
    "SHOPPING_DIVERSITY", "UNSKIPPABLE_CARRIERS_DUE_TO_FULLY_OPEN");

std::pair<SOLFareMarketMap::iterator, bool>
insertFMIntoMap(SOLFareMarketMap& map, FareMarket* fm)
{
  SOLFareMarketKey key(
      fm->boardMultiCity(), fm->offMultiCity(), fm->governingCarrier(), fm->getFmTypeSol());
  return map.insert(std::make_pair(key, fm));
}

FareMarket*
getFMFromFMMap(SOLFareMarketMap& map, FareMarket* fm)
{
  SOLFareMarketKey key(
      fm->boardMultiCity(), fm->offMultiCity(), fm->governingCarrier(), fm->getFmTypeSol());
  SOLFareMarketMap::iterator it = map.find(key);
  return (it == map.end() ? nullptr : it->second);
}
} // namespace

ItinAnalyzerServiceWrapperSOL::ItinAnalyzerServiceWrapperSOL(ShoppingTrx& trx,
                                                             ItinAnalyzerService& itinAnalyzer)
  : _trx(trx),
    _itinAnalyzer(itinAnalyzer),
    _diag922Data(trx),
    _foreignDomesticSkipConfigured(excludeLocalFmForForeignDomestic.getValue()),
    _area12SOLSkipConfigured(excludeLocalFmForArea1_2.getValue()),
    _area13SOLSkipConfigured(excludeLocalFmForArea1_3.getValue()),
    _nationUSCAArea1SOLSkipConfigured(excludeLocalFmForNationUscaArea1.getValue()),
    _nationUSCAChinaSOLSkipConfigured(excludeLocalFmForNationUscaChina.getValue()),
    _subarea21Area3SOLSkipConfigured(excludeLocalFmForSubArea21Area3.getValue()),
    _subarea21SubArea22Or23SOLSkipConfigured(excludeLocalFmForSubArea21SubArea22_23.getValue()),
    _subarea21InternationalSOLSkipConfigured(excludeLocalFmForSubArea21International.getValue())
{
}

bool
ItinAnalyzerServiceWrapperSOL::hasFares(const Itin& itin, const FareMarket& fareMarket)
{
  LOG4CXX_DEBUG(logger, "HasFares called");
  const LocCode& origin = fareMarket.origin()->loc();
  const LocCode& destination = fareMarket.destination()->loc();
  const LocCode& boardMultiCity = fareMarket.boardMultiCity();
  const LocCode& offMultiCity = fareMarket.offMultiCity();
  bool diffBoardPoint = (!boardMultiCity.empty()) && (boardMultiCity != origin);
  bool diffOffPoint = (!offMultiCity.empty()) && (offMultiCity != destination);

  if (_trx.isAltDates())
  {
    DatePair dateRange = ShoppingAltDateUtil::getTravelDateRange(_trx);

    if (_trx.dataHandle()
            .getFaresByMarketCxr(origin,
                                 destination,
                                 fareMarket.governingCarrier(),
                                 dateRange.first,
                                 dateRange.second)
            .size() > 0)
      return true;
    if (diffBoardPoint &&
        _trx.dataHandle()
                .getFaresByMarketCxr(boardMultiCity,
                                     destination,
                                     fareMarket.governingCarrier(),
                                     dateRange.first,
                                     dateRange.second)
                .size() > 0)
      return true;
    if (diffOffPoint &&
        _trx.dataHandle()
                .getFaresByMarketCxr(origin,
                                     offMultiCity,
                                     fareMarket.governingCarrier(),
                                     dateRange.first,
                                     dateRange.second)
                .size() > 0)
      return true;
    if (diffBoardPoint && diffOffPoint &&
        _trx.dataHandle()
                .getFaresByMarketCxr(boardMultiCity,
                                     offMultiCity,
                                     fareMarket.governingCarrier(),
                                     dateRange.first,
                                     dateRange.second)
                .size() > 0)
      return true;
  }
  else
  {
    DateTime travelDate = ItinUtil::getTravelDate(itin.travelSeg());
    if (_trx.dataHandle()
            .getFaresByMarketCxr(origin, destination, fareMarket.governingCarrier(), travelDate)
            .size() > 0)
      return true;
    if (diffBoardPoint &&
        _trx.dataHandle()
                .getFaresByMarketCxr(
                     boardMultiCity, destination, fareMarket.governingCarrier(), travelDate)
                .size() > 0)
      return true;
    if (diffOffPoint &&
        _trx.dataHandle()
                .getFaresByMarketCxr(
                     origin, offMultiCity, fareMarket.governingCarrier(), travelDate)
                .size() > 0)
      return true;
    if (diffBoardPoint && diffOffPoint &&
        _trx.dataHandle()
                .getFaresByMarketCxr(
                     boardMultiCity, offMultiCity, fareMarket.governingCarrier(), travelDate)
                .size() > 0)
      return true;
  }

  return false;
}

void
ItinAnalyzerServiceWrapperSOL::doSumOfLocals()
{
  _cxrOverride = _trx.getRequest()->cxrOverride();

  Itin& journeyItin = *_trx.journeyItin();

  if (_foreignDomesticSkipConfigured)
  {
    _isSkipForeignDomestic = isForeignDomesticJourneyItin(&journeyItin);
  }

  SolRestriction solRestriction;
  setUpSolRestriction(&journeyItin, solRestriction);

  std::map<ItinIndex::Key, CarrierCode> carrierMap;
  std::vector<FareMarket*> allFareMarkets;
  int legIdx(1);
  for (ShoppingTrx::Leg& curLeg : _trx.legs())
  {
    if (curLeg.stopOverLegFlag())
      continue;

    SOLFareMarketMap uniqueFareMarkets;
    Itin2SopInfoMap itinOrigSopInfoMap = itinanalyzerutils::getSopInfoMap(journeyItin, curLeg);

    ItinIndex::ItinMatrixIterator matrixIt(curLeg.carrierIndex().root().begin());
    for (; matrixIt != curLeg.carrierIndex().root().end(); ++matrixIt)
    {
      const uint32_t carrierKey(matrixIt->first);
      Itin* firstItin = itinanalyzerutils::getFirstItin(matrixIt->second);
      CarrierCode govCxr = itinanalyzerutils::getGovCarrier(firstItin);
      int numAllSOPs = itinanalyzerutils::getNumSOPs(matrixIt->second);

      FareMarket* thruFM = nullptr;

      GovCxrGroupParameters groupParameters(carrierKey, govCxr, numAllSOPs);

      carrierMap.insert(std::make_pair(carrierKey, govCxr));

      // reuse all thru fare markets
      if (LIKELY(firstItin && !firstItin->fareMarket().empty()))
      {
        thruFM = firstItin->fareMarket().front();
        for (FareMarket* fareMarket : firstItin->fareMarket())
        {
          fareMarket->setFmTypeSol(FareMarket::SOL_FM_THRU);
          insertFMIntoMap(uniqueFareMarkets, fareMarket);

          if (!_solTuning.isThruMileageInitialized())
            _solTuning.initializeThruMilage(fareMarket);

          if (!fallback::fixed::fallbackFMDirectionSetting())
            itinanalyzerutils::determineFMDirection(fareMarket, LocCode());
        }

        // initialize thru fm(s)
        for (FareMarket* fareMarket : firstItin->fareMarket())
        {
          ItinIndex::ItinIndexIterator rowIt(curLeg.carrierIndex().beginRow(carrierKey));
          for (; rowIt != curLeg.carrierIndex().endRow(); ++rowIt)
          {
            Itin* curItin(rowIt->second);
            if (skipDomesticTransborderThroughFareMarket(fareMarket, curItin))
              continue;

            Itin2SopInfoMap::const_iterator it(itinOrigSopInfoMap.find(curItin));

            SopInfo origSopInfo =
                (it != itinOrigSopInfoMap.end() ? it->second : SopInfo(-1, false));

            FareMarket* fmToUpdate = fareMarket;
            if (UNLIKELY(_trx.isIataFareSelectionApplicable()))
            {
              fmToUpdate = getFMFromFMMap(uniqueFareMarkets, fareMarket);
              if (!fmToUpdate)
                fmToUpdate = fareMarket;
            }

            itinanalyzerutils::initializeSopUsages(_trx,
                                                   fmToUpdate,
                                                   curItin,
                                                   curItin->travelSeg(),
                                                   0,
                                                   origSopInfo.first,
                                                   rowIt.bitIndex(),
                                                   groupParameters);
          }
        }

        firstItin->fareMarket().clear();
      }

      bool isSuppressedSOLIfThruExists = false;

      _isThisCarrierUnskippable = solRestriction.unskippableCarriers->has(govCxr);

      if (solRestriction.skipByArea && (!_isThisCarrierUnskippable) && thruFM)
      {
        isSuppressedSOLIfThruExists = hasFares(*firstItin, *thruFM);
      }

      // Don't create local FMs if the thru FM is online and fully open
      const bool isFullyOpen = !_isThisCarrierUnskippable &&
                               isFullyOpenAndOnline(curLeg.carrierIndex(), govCxr, carrierKey);

      ItinIndex::ItinIndexIterator rowIt(curLeg.carrierIndex().beginRow(carrierKey));
      for (; rowIt != curLeg.carrierIndex().endRow(); ++rowIt)
      {
        Itin* curItin(rowIt->second);
        curItin->fareMarket().clear();

        if (LIKELY(filterGovCxrOverride(curItin)))
        {
          Itin2SopInfoMap::const_iterator it(itinOrigSopInfoMap.find(curItin));
          SopInfo origSopInfo = (it != itinOrigSopInfoMap.end() ? it->second : SopInfo(-1, false));
          getGovCxrAndAllFareMkt(curItin,
                                 uniqueFareMarkets,
                                 legIdx,
                                 origSopInfo,
                                 rowIt.bitIndex(),
                                 groupParameters,
                                 isFullyOpen,
                                 isSuppressedSOLIfThruExists);
        }
      }
      // put all unique fare markets into first itin for governing carrier
      if (LIKELY(firstItin != nullptr))
      {
        fillCxrFareMarkets(*firstItin, matrixIt->second);
      }
    }

    // put all unique fare markets for current Leg into trx
    for (SOLFareMarketMap::value_type& pr : uniqueFareMarkets)
      allFareMarkets.push_back(pr.second);
    ++legIdx;
  }

  _trx.fareMarket().swap(allFareMarkets);

  _trx.setThroughFarePrecedencePossible(
      TFPUtil::isThroughFarePrecedencePossibleAtGeography(_trx.journeyItin()->travelSeg()));
  if (_trx.isThroughFarePrecedencePossible())
  {
    for (FareMarket* fm : _trx.fareMarket())
      fm->setThroughFarePrecedenceNGS(TFPUtil::isThroughFarePrecedenceNeededNGS(_trx, *fm));
  }

  DiagCollector* diag942 = nullptr;
  if (_trx.diagnostic().diagnosticType() == Diagnostic942)
  {
    diag942 = DCFactory::instance()->create(_trx);
    diag942->enable(Diagnostic942);
  }
  _trx.diversity().initialize(diag942, _trx, carrierMap);
  if (diag942)
    diag942->flushMsg();

  itinanalyzerutils::printDiagnostic922(_trx, carrierMap);
  _diag922Data.printSkippedFareMarkets();
}

// If a request contains governing carrier override, this method will determine if
// the candidate itin is valid for the override. For all other conditions, this
// method is a pass through filter
bool
ItinAnalyzerServiceWrapperSOL::filterGovCxrOverride(const Itin* itin)
{
  if (LIKELY(_cxrOverride == BLANK_CODE))
    return true;

  const std::vector<TravelSeg*>& trvSegs(itin->travelSeg());
  return isOverrideCxrExistInTrvSegs(trvSegs);
}
namespace
{
bool
canCreateLocalFM(const std::vector<TravelSeg*>(&fmPairTS)[2])
{
  for (const std::vector<TravelSeg*>& travelSegs : fmPairTS)
  {
    if (travelSegs.size() == 1)
    {
      const TravelSeg* tvlSeg = travelSegs.front();
      if (tvlSeg->boardMultiCity() == tvlSeg->offMultiCity() || tvlSeg->isArunk())
        return false;
    }
  }
  return true;
}
}

void
ItinAnalyzerServiceWrapperSOL::getGovCxrAndAllFareMkt(Itin* itin,
                                                      SOLFareMarketMap& uniqueFareMarkets,
                                                      int legIdx,
                                                      SopInfo origSopInfo,
                                                      int bitIndex,
                                                      GovCxrGroupParameters& params,
                                                      bool isFullyOpenAndOnline,
                                                      bool isSkipByAreaValidation)
{
  TSELatencyData metrics(_trx, "ITIN GOVCXR AND ALL FMKT");
  std::vector<TravelSeg*>& trvSegs(itin->travelSeg());

  // address thru fare market
  std::vector<FareMarket*> fareMarkets = getGovCxrAndAllFareMktold(
      itin, 0, trvSegs, uniqueFareMarkets, legIdx, origSopInfo.first, bitIndex, params);

  for (FareMarket* fm : fareMarkets)
  {
    itin->fareMarket().push_back(fm);

    if (origSopInfo.second)
      _trx.setCustomSolutionFM(fm);
    if (UNLIKELY(ShoppingUtil::isSpanishDiscountApplicableForShopping(_trx, itin)))
    {
      _trx.setSpanishDiscountFM(fm);
    }
  }

  FareMarket* thruMarket = nullptr;
  if (!fareMarkets.empty())
    thruMarket = fareMarkets.front();

  bool domestic = false;

  if (UNLIKELY(_foreignDomesticSkipConfigured))
  {
    domestic = (thruMarket ? (thruMarket->geoTravelType() == GeoTravelType::Domestic) : false);
  }
  else
  {
    // Include Foreign domestic as domestic
    domestic = (thruMarket ? (thruMarket->geoTravelType() == GeoTravelType::Domestic ||
                              thruMarket->geoTravelType() == GeoTravelType::ForeignDomestic)
                           : false);
  }

  // address local fare markets
  if (_trx.getRequest()->isSuppressedSumOfLocals())
    return;

  // JIRA SCI-245: Add check for skipping local FM creation because of exclude code share sop.
  //              the code share checks should be done for only international travel
  bool isCreateLocalFMCheckNeeded = (!fallback::fallbackExcludeCodeSharingFix(&_trx))
                                        ? (itin->geoTravelType() == GeoTravelType::International)
                                        : true;

  CodeShareValidatorSOL codeShareValidator(
      _trx, itin, legIdx, (thruMarket ? thruMarket->geoTravelType() : GeoTravelType::UnknownGeoTravelType));

  if (isCreateLocalFMCheckNeeded && !codeShareValidator.shouldCreateLocalFareMarkets() &&
      !_isThisCarrierUnskippable)
  {
    const char* reason = "skipped local fare market creation because of EXCLUDE_CODE_SHARE_SOP";
    if (codeShareValidator.isGovernigSegmentFound())
      reason = "skipped local fare market creation because of EXCLUDE_CODE_SHARE_SOP [no primary "
               "sector]";

    _diag922Data.reportFareMktToDiag(itin, trvSegs, legIdx, params, reason);
    return;
  }

  for (std::vector<TravelSeg*>::iterator itBegin(trvSegs.begin()),
       it(itBegin + 1),
       itEnd(trvSegs.end());
       it < itEnd;
       ++it)
  {
    std::vector<TravelSeg*> fmPairTS[2];
    fmPairTS[0].assign(itBegin, it), fmPairTS[1].assign(it, itEnd);

    if (isCreateLocalFMCheckNeeded &&
        (codeShareValidator.shouldSkipLocalFareMarketPair(fmPairTS[0], fmPairTS[1])) &&
        (!_isThisCarrierUnskippable))
    {
      _diag922Data.reportFareMktToDiag(
          itin, fmPairTS[0], legIdx, params, "skipped via EXCLUDE_CODE_SHARE_SOP");
      _diag922Data.reportFareMktToDiag(
          itin, fmPairTS[1], legIdx, params, "skipped via EXCLUDE_CODE_SHARE_SOP");
      continue;
    }
    if (!canCreateLocalFM(fmPairTS))
      continue;

    const int startSegOffs[2] = {0, static_cast<int>(fmPairTS[0].size())};

    for (int i = 0; i < 2; ++i)
    {
      const std::vector<TravelSeg*>& fmTS = fmPairTS[i];
      TSE_ASSERT(!fmTS.empty());

      const bool keepFlag = (domestic || shouldKeepLocalFareMarket(fmTS,
                                                                   fmPairTS[0],
                                                                   fmPairTS[1],
                                                                   itin,
                                                                   legIdx,
                                                                   isFullyOpenAndOnline,
                                                                   isSkipByAreaValidation,
                                                                   params));

      if (keepFlag)
      {
        std::vector<FareMarket*> localFareMarkets = getGovCxrAndAllFareMktold(itin,
                                                                              startSegOffs[i],
                                                                              fmTS,
                                                                              uniqueFareMarkets,
                                                                              legIdx,
                                                                              origSopInfo.first,
                                                                              bitIndex,
                                                                              params);

        for (FareMarket* fareMarket : localFareMarkets)
        {
          if (isCreateLocalFMCheckNeeded &&
              (codeShareValidator.hasCodeSharedGoverningSegment(fareMarket)) &&
              (!_isThisCarrierUnskippable))
          {
            _diag922Data.reportFareMktToDiag(
                itin, fmTS, legIdx, params, "skipped via EXCLUDE_CODE_SHARE_SOP");
          }
          else
          {
            itin->fareMarket().push_back(fareMarket);

            if (UNLIKELY(origSopInfo.second))
              _trx.setCustomSolutionFM(fareMarket);
            if (UNLIKELY(ShoppingUtil::isSpanishDiscountApplicableForShopping(_trx, itin)))
              _trx.setSpanishDiscountFM(fareMarket);
          }
        }
      }
    } // for (i)
  } // for (it)
}

// For a pair of travel segments, validate if both set of segments contains the override carrier.
// Pass through if the request doesn't contain override carrier
bool
ItinAnalyzerServiceWrapperSOL::isPairValidForGovCxrOverride(const std::vector<TravelSeg*>& fm1ts,
                                                            const std::vector<TravelSeg*>& fm2ts)
{
  if (LIKELY(_cxrOverride == BLANK_CODE))
  {
    // If carrier override is not set, pass the filter
    return true;
  }

  return (isOverrideCxrExistInTrvSegs(fm1ts) && isOverrideCxrExistInTrvSegs(fm2ts));
}

// Check if the override carrier exists in the vector of travel segments
// Return true if at least one of the segment matches with the override key
bool
ItinAnalyzerServiceWrapperSOL::isOverrideCxrExistInTrvSegs(const std::vector<TravelSeg*>& trvSegs)
{
  if (trvSegs.empty())
  {
    // Has to be non empty vector
    return false;
  }

  for (std::vector<TravelSeg*>::const_iterator itBegin(trvSegs.begin()),
       it(itBegin),
       itEnd(trvSegs.end());
       it < itEnd;
       ++it)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*it);
    if (airSeg && airSeg->carrier() == _cxrOverride)
    {
      return true;
    }
  }
  return false;
}

void
ItinAnalyzerServiceWrapperSOL::initializeFareMarket(FareMarket*& fm,
                                                    Itin* itin,
                                                    SOLFareMarketMap& uniqueFareMarkets,
                                                    const std::vector<TravelSeg*>& fmts,
                                                    int startSeg,
                                                    int origSopId,
                                                    int bitIndex,
                                                    GovCxrGroupParameters& params)
{
  std::pair<SOLFareMarketMap::const_iterator, bool> pr = insertFMIntoMap(uniqueFareMarkets, fm);

  if (!pr.second) // Already existing
    fm = pr.first->second;
  else // New fare market
  {
    if (UNLIKELY(!_trx.isNotExchangeTrx()))
      fm->setFCChangeStatus(-1 /*itin.pointOfChgSegOrder()*/);

    // We only keep 1 FM for the same O&D&GOV CXR
    fm->travelSeg().clear();

    AirSeg* airSeg(nullptr);
    _trx.dataHandle().get(airSeg);
    airSeg->segmentType() = Open;
    airSeg->carrier() = fm->governingCarrier();
    airSeg->departureDT() = fmts.front()->departureDT();
    airSeg->arrivalDT() = fmts.back()->arrivalDT();
    airSeg->origin() = fmts.front()->origin();
    airSeg->destination() = fmts.back()->destination();
    airSeg->origAirport() = fmts.front()->origAirport();
    airSeg->destAirport() = fmts.back()->destAirport();
    airSeg->boardMultiCity() = fmts.front()->boardMultiCity();
    airSeg->offMultiCity() = fmts.back()->offMultiCity();
    airSeg->bookingDT() = _trx.getRequest()->ticketingDT();
    airSeg->resStatus() = CONFIRM_RES_STATUS; // in Shopping all segments have confirm status

    fm->travelSeg().push_back(airSeg);
  }

  itinanalyzerutils::initializeSopUsages(
      _trx, fm, itin, fmts, startSeg, origSopId, bitIndex, params);
}

std::vector<FareMarket*>
ItinAnalyzerServiceWrapperSOL::getGovCxrAndAllFareMktold(Itin* itin,
                                                         int startSeg,
                                                         const std::vector<TravelSeg*>& fmts,
                                                         SOLFareMarketMap& uniqueFareMarkets,
                                                         int legIdx,
                                                         int origSopId,
                                                         int bitIndex,
                                                         GovCxrGroupParameters& params)
{
  std::vector<FareMarket*> ret;

  const bool isThruFM = (itin->travelSeg().size() == fmts.size());
  FareMarket* fareMarket = nullptr;
  fareMarket = itinanalyzerutils::buildFareMkt(_trx, itin, fmts, legIdx, params);

  // NOTE: this is fine as long as we have only 1 break point (so there are only 2 FMs per leg
  // possible)
  if (!isThruFM)
    fareMarket->setSolComponentDirection(startSeg == 0 ? FareMarket::SOL_COMPONENT_ORIGIN
                                                       : FareMarket::SOL_COMPONENT_DESTINATION);

  if (isThruFM)
  {
    // Thru FM should already be initialized
    FareMarket* existingFareMarket = getFMFromFMMap(uniqueFareMarkets, fareMarket);
    if (existingFareMarket)
    {
      if (!existingFareMarket->getApplicableSOPs())
        return ret;

      ret.push_back(existingFareMarket);

      FareMarket* highestTPMFareMarket = checkDualGoverningCarrier(existingFareMarket, fmts, legIdx, origSopId);
      if (UNLIKELY(highestTPMFareMarket))
      {
        initializeFareMarket(highestTPMFareMarket,
                             itin,
                             uniqueFareMarkets,
                             fmts,
                             startSeg,
                             origSopId,
                             bitIndex,
                             params);
        ret.push_back(highestTPMFareMarket);
      }

      return ret;
    }
  }

  // Check to see if the governing carrier string is empty
  if (fareMarket->governingCarrier().empty())
  {
    // If no governing carrier was found, use special processing logic
    GoverningCarrier govCxr(&_trx);
    govCxr.getGovCxrSpecialCases(*fareMarket);
  }

  const bool keepFM = shouldKeepFareMarket(fareMarket, itin);
  if (!keepFM)
    return ret;
  initializeFareMarket(
      fareMarket, itin, uniqueFareMarkets, fmts, startSeg, origSopId, bitIndex, params);

  ret.push_back(fareMarket);

  FareMarket* highestTPMFareMarket = checkDualGoverningCarrier(fareMarket, fmts, legIdx, origSopId, true);
  if (UNLIKELY(highestTPMFareMarket))
  {
    initializeFareMarket(
        highestTPMFareMarket, itin, uniqueFareMarkets, fmts, startSeg, origSopId, bitIndex, params);
    ret.push_back(highestTPMFareMarket);
  }

  return ret;
}

bool
ItinAnalyzerServiceWrapperSOL::shouldKeepFareMarket(const FareMarket* fareMarket, const Itin* itin)
{
  const bool logDiagnostic = (Diagnostic922 == _trx.diagnostic().diagnosticType());
  std::pair<bool, std::string> skipCheck(_solTuning.skipByConnectingCityMileage(
      *fareMarket, fareMarket->travelSeg(), itin, logDiagnostic));

  if (_isThisCarrierUnskippable)
  {
    return true;
  }

  if (skipCheck.first)
  {
    _diag922Data.reportFareMktToDiag(fareMarket, skipCheck.second, true);
    return false;
  }

  return true;
}

bool
ItinAnalyzerServiceWrapperSOL::shouldKeepLocalFareMarket(const std::vector<TravelSeg*>& fmToCheck,
                                                         const std::vector<TravelSeg*>& ctxPairTS1,
                                                         const std::vector<TravelSeg*>& ctxPairTS2,
                                                         const Itin* itin,
                                                         int legIdx,
                                                         bool isFullyOpenAndOnline,
                                                         bool isSkipByAreaValidation,
                                                         const GovCxrGroupParameters& params)
{
  // Those checks can be arbitrary order, but stacked this way
  // to keep diag 922 output on skipped fare markets consistent in time line
  const char* skipReason = nullptr;
  if (isFullyOpenAndOnline || !isPairValidForGovCxrOverride(ctxPairTS1, ctxPairTS2))
  {
    skipReason = "skipped for fully open";
  }
  else if (isSkipByAreaValidation)
  {
    skipReason = "skipped by origin/destination area validation";
  }
  else if ((_solTuning.skipLocalFMByLegOndMileage(/*legIdx to base offset 0*/ legIdx - 1, itin)) &&
           (!_isThisCarrierUnskippable))
  {
    _trx.setSolNoLocalInd(true);
    skipReason = "skipped by leg origin and destination mileage";
  }
  else if (UNLIKELY(_isSkipForeignDomestic))
  {
    // If foreign domestic itin have international connection, skip creating SOL FM
    for (const TravelSeg* tvlSeg : itin->travelSeg())
    {
      if (tvlSeg->isInternationalSegment())
      {
        skipReason = "skipped by foreign domestic with international connection validation";
        break;
      }
    }
  }

  if (skipReason)
  {
    _diag922Data.reportFareMktToDiag(itin, fmToCheck, legIdx, params, skipReason);
  }

  return (skipReason == nullptr);
}

bool
ItinAnalyzerServiceWrapperSOL::skipDomesticTransborderThroughFareMarket(
    const FareMarket* fareMarket, const Itin* itin)
{
  // skip domestic-transborder through fare markets
  if (GeoTravelType::Transborder == fareMarket->geoTravelType() || GeoTravelType::Domestic == fareMarket->geoTravelType())
  {
    CarrierCode firstCarrier;
    for (const TravelSeg* tvlSeg : itin->travelSeg())
    {
      if (LIKELY(tvlSeg->isAir()))
      {
        const AirSeg* airSeg(static_cast<const AirSeg*>(tvlSeg));
        if (firstCarrier.empty())
        {
          firstCarrier = airSeg->carrier();
        }
        if (firstCarrier != airSeg->carrier())
        {
          _diag922Data.reportFareMktToDiag(fareMarket, "domestic-transborder through fare market");
          return true;
        }
      }
    }
  }
  return false;
}

void
ItinAnalyzerServiceWrapperSOL::setUpSolRestriction(const Itin* journeyItin, SolRestriction& context)
    const
{
  // Area of origin and destination
  const Loc* origin = journeyItin->travelSeg().front()->origin();
  const Loc* destination = journeyItin->travelSeg().front()->destination();
  const IATAAreaCode& originIATA = origin->area();
  const IATAAreaCode& destinationIATA = destination->area();
  const IATASubAreaCode& originSubArea = origin->subarea();
  const IATASubAreaCode& destinationSubArea = destination->subarea();

  if ((LocUtil::isUS(*origin) && LocUtil::isMiddleEast(*destination)) ||
      (LocUtil::isUS(*destination) && LocUtil::isMiddleEast(*origin)))
  {
    context.unskippableCarriers = &unskippableFlightUSArea220Carriers.getValue();
    context.skipByArea = true;
    return;
  }
  if (_area12SOLSkipConfigured)
  {
    if (((IATA_AREA1 == originIATA) && (IATA_AREA2 == destinationIATA)) ||
        ((IATA_AREA2 == originIATA) && (IATA_AREA1 == destinationIATA)))
    {
      context.unskippableCarriers = &unskippableCarriersOnArea1_Area2.getValue();
      context.skipByArea = true;
      return;
    }
  }

  if (_nationUSCAChinaSOLSkipConfigured)
  {
    // This was required by JIRA SCI-907.
    if ((LocUtil::isDomesticUSCA(*origin) && LocUtil::isChina(*destination)) ||
        (LocUtil::isDomesticUSCA(*destination) && LocUtil::isChina(*origin)))
    {
      context.unskippableCarriers = &unskippableCarriersOnNationUSCA_China.getValue();
      context.skipByArea = true;
      return;
    }
  }

  if (_area13SOLSkipConfigured)
  {
    if (((IATA_AREA1 == originIATA) && (IATA_AREA3 == destinationIATA)) ||
        ((IATA_AREA3 == originIATA) && (IATA_AREA1 == destinationIATA)))
    {
      context.skipByArea = true;
      return;
    }
  }
  if (_nationUSCAArea1SOLSkipConfigured)
  {
    if ((LocUtil::isDomesticUSCA(*origin) && (IATA_AREA1 == destinationIATA) &&
         !LocUtil::isDomesticUSCA(*destination)) ||
        (LocUtil::isDomesticUSCA(*destination) && (IATA_AREA1 == originIATA) &&
         !LocUtil::isDomesticUSCA(*origin)))
    {
      context.unskippableCarriers = &unskippableCarriersOnNationUSCA_Area1.getValue();
      context.skipByArea = true;
      return;
    }
  }

  if (_subarea21Area3SOLSkipConfigured)
  {
    if (((IATA_SUB_AREA_21() == originSubArea) && (IATA_AREA3 == destinationIATA)) ||
        ((IATA_AREA3 == originIATA) && (IATA_SUB_AREA_21() == destinationSubArea)))
    {
      context.unskippableCarriers = &unskippableCarriersOnArea21_Area3.getValue();
      context.skipByArea = true;
      return;
    }
  }
  if (_subarea21SubArea22Or23SOLSkipConfigured)
  {
    if (((IATA_SUB_AREA_21() == originSubArea) &&
         (IATA_SUB_AREA_22() == destinationSubArea || IATA_SUB_AREA_23() == destinationSubArea)) ||
        ((IATA_SUB_AREA_22() == originSubArea || IATA_SUB_AREA_23() == originSubArea) &&
         (IATA_SUB_AREA_21() == destinationSubArea)))
    {
      context.unskippableCarriers = &unskippableCarriersOnSubarea21_Subarea22Or23.getValue();
      context.skipByArea = true;
      return;
    }
  }
  if (_subarea21InternationalSOLSkipConfigured)
  {
    if ((IATA_SUB_AREA_21() == originSubArea) && (IATA_SUB_AREA_21() == destinationSubArea) &&
        (origin->nation() != destination->nation()))
    {
      context.skipByArea = true;
      return;
    }
  }
}

bool
ItinAnalyzerServiceWrapperSOL::isForeignDomesticJourneyItin(const Itin* journeyItin) const
{
  const Loc* origin = journeyItin->travelSeg().front()->origin();
  const Loc* destination = journeyItin->travelSeg().front()->destination();

  return ((origin->nation() == destination->nation()) && (NATION_US != origin->nation()));
}

bool
ItinAnalyzerServiceWrapperSOL::isOnlineThroughFareMarket(const Itin* itin) const
{
  CarrierCode firstCarrier;
  size_t segCount = 0;

  for (const TravelSeg* tvlSeg : itin->travelSeg())
  {
    const AirSeg* const as = tvlSeg->toAirSeg();

    if (UNLIKELY(!as))
      continue;

    ++segCount;

    if (firstCarrier.empty())
      firstCarrier = as->marketingCarrierCode();
    else if (firstCarrier != as->marketingCarrierCode())
      return false;
  }

  return segCount > 1;
}

bool
ItinAnalyzerServiceWrapperSOL::isFullyOpenThroughAvl(const Itin* itin) const
{
  PricingTrx::ClassOfServiceKey cosKey;
  cosKey.reserve(itin->travelSeg().size());

  // Build the key using all related travelSeg of the itin
  for (TravelSeg* ts : itin->travelSeg())
  {
    const AirSeg* const as = ts->toAirSeg();
    if (LIKELY(as && !as->isFake()))
      cosKey.push_back(ts);
  }

  if (UNLIKELY(cosKey.size() <= 1))
  {
    // We don't need to process if there is no connection
    return false;
  }

  const auto availIt = _trx.availabilityMap().find(ShoppingUtil::buildAvlKey(cosKey));
  if (UNLIKELY(availIt == _trx.availabilityMap().end()))
    return false;

  const auto& sopCOS = *availIt->second;
  return std::all_of(sopCOS.begin(), sopCOS.end(), [&](const auto& fliCOS)
  {
    return std::all_of(fliCOS.begin(), fliCOS.end(), [&](const ClassOfService* cos)
    {
      return cos->numSeats() != 0 && cos->numSeats() == fliCOS.front()->numSeats();
    });
  });
}

bool
ItinAnalyzerServiceWrapperSOL::isFullyOpenAndOnline(ItinIndex& cxrIndex,
                                                    const CarrierCode govCxr,
                                                    const uint32_t cxrKey) const
{
  const bool isDisabled = unskippableCarriersDueToFullyOpen.getValue().has(govCxr) ||
                          unskippableCarriersDueToFullyOpen.getValue().has(ANY_CARRIER);

  if (isDisabled)
    return false;

  for (auto itinCell = cxrIndex.beginRow(cxrKey), itinCellEnd = cxrIndex.endRow();
       itinCell != itinCellEnd;
       ++itinCell)
  {
    // Don't create local FMs if the thru FM is online and fully open
    if (isOnlineThroughFareMarket(itinCell->second) && isFullyOpenThroughAvl(itinCell->second))
      return true;
  }
  return false;
}

void
ItinAnalyzerServiceWrapperSOL::fillCxrFareMarkets(Itin& itin, ItinIndex::ItinRow& row) const
{
  SOLFareMarketMap cxrUniqueFMs;
  for (FareMarket* fm : itin.fareMarket())
    insertFMIntoMap(cxrUniqueFMs, fm);

  for (const ItinIndex::ItinRow::value_type& columns : row)
    for (const ItinIndex::ItinColumn::value_type& cell : columns.second)
      for (FareMarket* fm : cell.second->fareMarket())
        if (insertFMIntoMap(cxrUniqueFMs, fm).second)
          itin.fareMarket().push_back(fm);
}

FareMarket*
ItinAnalyzerServiceWrapperSOL::checkDualGoverningCarrier(FareMarket* fareMarket,
                                                         const std::vector<TravelSeg*>& fmts,
                                                         int legIdx,
                                                         int origSopId,
                                                         bool isLocal)
{
  if (fmts.size() <= 1)
    return nullptr;

  if (!fallback::fallbackFareSelction2016(&_trx) &&
      (_trx.getTrxType() == PricingTrx::IS_TRX))
    return checkDualGoverningCarrierForIS(fareMarket, fmts, legIdx, origSopId);

  const bool isSubIata21 = fareMarket->travelBoundary().isSet(FMTravelBoundary::TravelWithinSubIATA21);
  if (UNLIKELY(_trx.isIataFareSelectionApplicable()))
  {
    if (TrxUtil::isFareSelectionForSubIata21Only(_trx) && !isSubIata21)
      return nullptr;
  }
  else if (!isSubIata21) // Pre-FareSelection only cares about sub iata 21 region
    return nullptr;

  GoverningCarrier govCxr(&_trx);
  bool useNewIataFareSelectionProcess =
      _trx.isIataFareSelectionApplicable() && !TrxUtil::isFareSelectionForSubIata21Only(_trx);

  CarrierCode dualGovCxr =
      useNewIataFareSelectionProcess
          ? govCxr.getHighestTPMCarrier(fmts, fareMarket->direction(), fareMarket->primarySector())
          : govCxr.getHighestTPMCarrierOld(fmts);

  if ((dualGovCxr != BAD_CARRIER) && (!dualGovCxr.empty()) &&
      (fareMarket->governingCarrier() != dualGovCxr))
  {
    fareMarket->setDualGoverningFlag(true);

    if (_trx.isIataFareSelectionApplicable())
    {
      if (isLocal && TrxUtil::isFareSelectionForNGSThruFMOnly(_trx))
        return nullptr; // Do not create local FMs if tuning is active.

      FareMarket* highestTPMFareMarket = nullptr;
      _trx.dataHandle().get(highestTPMFareMarket);

      _itinAnalyzer.cloneFareMarket(*fareMarket, *highestTPMFareMarket, dualGovCxr);
      return highestTPMFareMarket;
    }
  }

  return nullptr;
}

FareMarket*
ItinAnalyzerServiceWrapperSOL::checkDualGoverningCarrierForIS(FareMarket* fareMarket,
                                                              const std::vector<TravelSeg*>& fmts,
                                                              int legIdx,
                                                              int origSopId)
{
  const bool isSubIata21 = fareMarket->travelBoundary().isSet(FMTravelBoundary::TravelWithinSubIATA21);
  if (!isSubIata21) // Pre-FareSelection only cares about sub iata 21 region
    return nullptr;

  GoverningCarrier govCxr(&_trx);
  bool useNewIataFareSelectionProcess = !TrxUtil::isFareSelectionForSubIata21Only(_trx);

  CarrierCode dualGovCxr =
      useNewIataFareSelectionProcess
          ? govCxr.getHighestTPMCarrier(fmts, fareMarket->direction(), fareMarket->primarySector())
          : govCxr.getHighestTPMCarrierOld(fmts);

  if ((dualGovCxr != BAD_CARRIER) && (!dualGovCxr.empty()) &&
      (fareMarket->governingCarrier() != dualGovCxr))
  {
     itinanalyzerutils::setSopWithHighestTPM(_trx, legIdx, origSopId);
  }

  return nullptr;
}

} // tse
