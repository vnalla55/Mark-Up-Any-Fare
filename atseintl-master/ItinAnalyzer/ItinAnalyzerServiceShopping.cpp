///-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "Common/Assert.h"
#include "Common/ClassOfService.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/GoverningCarrier.h"
#include "Common/ItinUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/RBDByCabinUtil.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/TSELatencyData.h"
#include "Common/ValidatingCxrUtil.h"
#include "DataModel/Diversity.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/InterlineTicketCarrierData.h"
#include "DBAccess/Cabin.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag892Collector.h"
#include "Diagnostic/Diag900Collector.h"
#include "Diagnostic/Diag922Collector.h"
#include "ItinAnalyzer/BrandedFaresDataRetriever.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"
#include "ItinAnalyzer/ItinAnalyzerServiceWrapperSOL.h"
#include "ItinAnalyzer/ItinAnalyzerUtils.h"
#include "Limitations/LimitationOnIndirectTravel.h"

#include <algorithm>

namespace tse
{
FALLBACK_DECL(fallbackStopOverLegByCityCode);
FALLBACK_DECL(fallbackDJCBannerFix);
FALLBACK_DECL(fallbackFareSelction2016);

namespace
{
ConfigurableValue<bool>
collect("BOUND_FARE", "COLLECT", false);
ConfigurableValue<bool>
validate("BOUND_FARE", "VALIDATE", false);
ConfigurableValue<bool>
validateBookingCode("BOUND_FARE", "VALIDATEBOOKINGCODE", false);
ConfigurableValue<bool>
createFareClassAppInfo("BOUND_FARE", "CREATEFARECLASSAPPINFO", false);
ConfigurableValue<bool>
useWebFareFlag("BOUND_FARE", "USEWEBFAREFLAG", false);
ConfigurableValue<bool>
useBoundRoutings("BOUND_FARE", "USEBOUNDROUTINGS", false);
ConfigurableValue<double>
sopQualityCoeff("SHOPPING_OPT", "SOP_QUALITY_COEFFICIENT", 0.0);
ConfigurableValue<int>
asoThruMileagePercent("SHOPPING_OPT", "ASO_THRU_MILEAGE_PERCENTAGE", 40);
ConfigurableValue<int>
asoJcbThreshold("SHOPPING_OPT", "ASO_JCB_THRESHOLD");
ConfigurableValue<int>
asoThreshold("SHOPPING_OPT", "ASO_THRESHOLD");
ConfigurableValue<int>
highestMileagePercent("SHOPPING_OPT", "HIGHEST_MILEAGE_PERCENTAGE");
}
namespace
{
bool
isCabinClassInvalid(const ShoppingTrx::SchedulingOption& sop)
{
  return (false == sop.cabinClassValid());
}

bool
isCabinClassValid(const ShoppingTrx::SchedulingOption& sop)
{
  return (true == sop.cabinClassValid());
}

std::string
generateMctErrorMessage(const std::vector<uint16_t>& problematicLegs)
{
  std::ostringstream message;
  message << "MIN CONNECTION TIME NOT MET BEFORE LEG";

  if (problematicLegs.size() > 1)
    message << "S";

  bool commaNeeded = false;
  for (auto internalLegId : problematicLegs)
  {
    if (commaNeeded)
      message << ",";

    commaNeeded = true;
    message << " " << (internalLegId + 1);
  }
  return message.str();
}

} // anon ns

// Functor for erasing ClassOfService elements
class FilterClassOfServiceByPriceCabin : public std::unary_function<ClassOfService*, bool>
{
private:
  CabinType _priceCabin;

public:
  FilterClassOfServiceByPriceCabin(CabinType& priceCabin) : _priceCabin(priceCabin) {}

  bool operator()(const ClassOfService* x)
  {
    if (UNLIKELY(x == nullptr))
    {
      return (false);
    }

    if (UNLIKELY(x->cabin().isUndefinedClass()))
    {
      return (false);
    }

    if (x->cabin() > _priceCabin)
    {
      return (true);
    }

    return (false);
  }
};

bool
ItinAnalyzerService::process(ShoppingTrx& trx)
{
  if (PricingTrx::ESV_TRX == trx.getTrxType())
  {
    return processESV(trx);
  }

  TSELatencyData metrics(trx, "ITIN PROCESS");

  LOG4CXX_INFO(_logger, "ItinAnalyzerService::process(ShoppingTrx)");
  if (trx.isAltDates())
  {
    checkValidateAltDays(trx);

    PricingTrx::AltDatePairs& altDatePairs = trx.altDatePairs();
    if (altDatePairs.size() == 0)
    {
      throw ErrorResponseException(ErrorResponseException::EMPTY_ALT_DATES_PAIRS_MAP);
      return false;
    }
  }
  // Test restricted currency in currency override transaction
  checkRestrictedCurrencyNation(trx);

  // See if they are disabled transactionally
  bool cfgASOLegsOn = _enableASOLegs;

  if ((*trx.paxType().begin())->paxType().equalToConst("JCB"))
  {
    _useASOThreshold = asoJcbThreshold.getValue();
  }
  else
  {
    _useASOThreshold = asoThreshold.getValue();
  }

  if (trx.noASOLegs() || int(trx.legs().size()) > _useASOThreshold || trx.isAltDates())
  {
    cfgASOLegsOn = false;
  }

  // Clear the itin vector so that when the journey
  // itin is pushed in generateJourneyItin, it is the one
  // and only itin in the transaction
  trx.itin().clear();

  // Generate the journey itin
  generateJourneyItin(trx);

  setFurthestPoint(trx, trx.journeyItin());

  trx.setTravelDate(trx.journeyItin()->travelDate());
  trx.bookingDate() = TseUtil::getBookingDate(trx.journeyItin()->travelSeg());

  // Set itinerary origination and calculations currencies
  ItinUtil::setItinCurrencies(*(trx.journeyItin()), trx.ticketingDate());

  // Select the ticketing carrier for the transaction
  selectTicketingCarrier(trx);

  // Set retransit points
  setRetransits(trx);

  // check if minConnection time is met for any possible itin
  std::vector<uint16_t> problematicLegs;
  if (!ShoppingUtil::isMinConnectionTimeMet(trx.legs(), trx.getOptions(),
                                            trx.getRequest()->isReturnIllogicalFlights(),
                                            problematicLegs))
  {
    throw ErrorResponseException(ErrorResponseException::NO_VALID_FLIGHT_DATE_FOUND,
                                 generateMctErrorMessage(problematicLegs));
  }

  // Validates the flight cabin codes
  if (!trx.getRequest()->isBrandedFaresRequest() && !trx.diversity().isExchangeForAirlines())
  {
    validateFlightCabin(trx);
  }
  else
  {
    setIndicesToSchedulingOption(trx);
  }

  if (Diagnostic908 == trx.diagnostic().diagnosticType() && !trx.diagnostic().processAllServices())
  {
    return true;
  }

  // Classify the legs according to surface
  // sector rules
  classifyLegSurfaceSectorTypes(trx);

  if (cfgASOLegsOn)
  {
    // Generate the combinations of legs that make
    // up the stop over legs
    generateStopOverLegs(trx);
  }

  // Check limitations using travelSeg from journey itin
  std::vector<Itin*>::const_iterator itinItem = trx.itin().begin();
  LimitationOnIndirectTravel limits(trx, **itinItem);
  limits.validateJourney();

  // Determine the governing carrier and retrieve the fare
  // market data handles
  getGovCxrAndFareMkt(trx);

  // Group the schedules
  groupSchedules(trx);

  if (cfgASOLegsOn)
  {
    // Estimate the governing carrier for the stop over legs
    estimateGovCxrForStopOverLegs(trx);

    // For any across stop over leg that also crosses a surface sector,
    // add a proper arunk travel seg span to its itinerary
    addAcrossStopOverLegArunkSpans(trx);

    // For all across stop over legs, generate cxr combination
    // maps that feed the bit index iterator
    generateAcrossStopOverLegCxrMaps(trx);
  }

  // Set inbound/outbound
  setInboundOutbound(trx);

  // Set currency override
  setCurrencyOverride(trx);

  // Set sortTaxByOrigCity flag
  setSortTaxByOrigCity(trx);

  // Order the segments by their itin containers
  ShoppingUtil::orderSegsByItin(trx);

  setItinRounding(trx);

  setShoppingFareMarketInfo(trx);

  if (trx.isSumOfLocalsProcessingEnabled())
  {
    calculateFlightTimeMinutes(trx);
    calculateMileage(trx);
    ItinAnalyzerServiceWrapperSOL wrapper(trx, *this);
    wrapper.doSumOfLocals();
  }
  else
  {
    if (trx.isIataFareSelectionApplicable())
      itinanalyzerutils::createSopUsages(trx);
  }

  collectMultiAirport(trx);

  if (trx.getRequest()->isBrandedFaresRequest())
  {
    BrandedFaresDataRetriever bfdr(trx, BrandRetrievalMode::PER_O_AND_D);
    if (!bfdr.process())
      LOG4CXX_ERROR(_logger, "No branded fares for any itins.");
    if (trx.getRequest()->isAllFlightsData() || trx.getRequest()->isCheapestWithLegParityPath())
    {
      Diag892Collector* diag892 = createDiag<Diag892Collector>(trx);
      itinanalyzerutils::filterBrandsForIS(trx, diag892);
    }
  }

  return true;
}

bool
ItinAnalyzerService::processESV(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "ITIN PROCESS ESV");

  LOG4CXX_INFO(_logger, "ItinAnalyzerService::processESV(ShoppingTrx&)");

  // Test restricted currency in currency override transaction
  checkRestrictedCurrencyNation(trx);

  // Clear the itin vector
  trx.itin().clear();

  // Generate the journey itin
  generateJourneyItin(trx);

  setFurthestPoint(trx, trx.journeyItin());

  trx.setTravelDate(trx.journeyItin()->travelDate());
  trx.bookingDate() = TseUtil::getBookingDate(trx.journeyItin()->travelSeg());

  // Set itinerary origination and calculations currencies
  ItinUtil::setItinCurrencies(*(trx.journeyItin()), trx.ticketingDate());

  // Select the ticketing carrier for the transaction
  selectTicketingCarrier(trx);

  // Set retransit points
  setRetransits(trx);

  // Validates the flight cabin codes
  validateFlightCabinESV(trx);

  if (trx.getOptions()->validateTicketingAgreement())
  {
    validateInterlineTicketCarrierESV(trx);
  }

  // Classify the legs according to surface sector rules
  classifyLegSurfaceSectorTypes(trx);

  // Check limitations using travelSeg from journey itin
  std::vector<Itin*>::const_iterator itinItem = trx.itin().begin();

  LimitationOnIndirectTravel limits(trx, **itinItem);
  limits.validateJourney();

  // Group the schedules
  groupSchedulesESV(trx);

  // Build local markets
  buildLocalMarkets(trx);

  // Set currency override
  setCurrencyOverride(trx);

  // Set sortTaxByOrigCity flag
  setSortTaxByOrigCity(trx);

  // Set the validating carrier
  setValidatingCarrier(trx);

  // Order the segments by their itin containers
  ShoppingUtil::orderSegsByItin(trx);

  setItinRounding(trx);

  calculateFlightTimeMinutes(trx);

  // Read bound fare configuration
  trx.setCollectBoundFares(collect.getValue());
  trx.setValidateUsingBindings(validate.getValue());
  trx.setValidateBookingCodeBF(validateBookingCode.getValue());
  trx.setCreateFareClassAppInfoBF(createFareClassAppInfo.getValue());
  trx.setUseWebFareFlagBF(useWebFareFlag.getValue());
  trx.setUseBoundRoutings(useBoundRoutings.getValue());
  return true;
}

void
ItinAnalyzerService::addScheduleGroupMapEntryESV(ShoppingTrx::Leg& curLeg,
                                                 ShoppingTrx::SchedulingOption& curSop,
                                                 ItinIndex::ItinCellInfo& itinCellInfo)
{
  // Create carrier key
  ItinIndex::Key cxrKey;
  ShoppingUtil::createCxrKey(curSop.governingCarrier(), cxrKey);

  // Create schedule key
  ItinIndex::Key scheduleKey;
  ShoppingUtil::createScheduleKey(curSop.itin(), scheduleKey);

  // Add entry to the group
  curLeg.carrierIndex().addItinCell(curSop.itin(), itinCellInfo, cxrKey, scheduleKey);
}

//---------------------------------------------------------------------------
// groupSchedulesByGovCxrAndConxnPoint()
// In/Out: curMap : Schedule group map for current leg
// In/Out: curLeg : Current leg
// This method takes a map and a leg and groups the itineraries stored in the
// leg by governing carrier first, then connection point secondly.
// Note this method can only be called after the governing carrier has been
// determined for every itinerary in the transaction.
//---------------------------------------------------------------------------
bool
ItinAnalyzerService::groupSchedulesByGovCxrAndConxnPoint(ShoppingTrx::Leg& curLeg, ShoppingTrx& trx)

{
  ItinIndex& curCarrierIndex = curLeg.carrierIndex();
  uint32_t sopIndex = 0;
  uint32_t sopMatched = 0;
  bool rt = false;
  bool mapEntryFailed = false;

  // get the requested cxr for flight finder
  CarrierCode requestedCxr;
  if (PricingTrx::FF_TRX == trx.getTrxType())
  {
    FlightFinderTrx& ffTrx = dynamic_cast<FlightFinderTrx&>(trx);
    size_t segIndex = ffTrx.journeyItin()->travelSeg().size() - 1;
    AirSeg* journeySeg = dynamic_cast<AirSeg*>(ffTrx.journeyItin()->travelSeg()[segIndex]);
    requestedCxr = journeySeg->carrier();

    if (curLeg.sop().empty())
    {
      addDummySop(trx, curLeg, *journeySeg);
    }
  }

  std::map<const Itin*, double> sopScoreMap;

  std::vector<ShoppingTrx::SchedulingOption>::iterator itinIter = curLeg.sop().begin();

  std::vector<ShoppingTrx::SchedulingOption>::iterator endItinIter = curLeg.sop().end();
  for (; itinIter != endItinIter; ++itinIter)
  {
    ItinIndex::ItinCellInfo itinCellInfo;
    Itin*& curItin = itinIter->itin();
    itinCellInfo.setPrimarySector(curItin->fareMarket().front()->primarySector());

    // if FF & BFF and the gov cxr did not match the input carrier then don't create it
    if (UNLIKELY(PricingTrx::FF_TRX == trx.getTrxType()))
    {
      // loop to find the matched governingCarrier

      std::vector<FareMarket*>::iterator fareMarket = curItin->fareMarket().begin();
      std::vector<FareMarket*>::iterator fareMarketEnd = curItin->fareMarket().end();

      for (; fareMarket != fareMarketEnd; ++fareMarket)
      {
        if ((*fareMarket)->governingCarrier() == requestedCxr)
        {
          // swap with the first fareMarket
          std::swap(*fareMarket, curItin->fareMarket().front());

          break;
        }
      }

      if (fareMarket == fareMarketEnd)
      {
        // Fake flight when carrier index is still empty and this is last processed SOP
        if ((sopMatched == 0) && (boost::next(itinIter) == endItinIter) && !trx.isAltDates())
        {
          // Keep this flight and set input carrier
          curItin->fareMarket().front()->governingCarrier() = requestedCxr;
          itinCellInfo.flags() |= ItinIndex::ITININDEXCELLINFO_FAKEDFLIGHT;
        }
        else
        {
          ++sopIndex;
          continue;
        }
      }
      else
      {
        if (itinIter->cabinClassValid() && !curItin->travelSeg().empty())
        {
          sopMatched++;
        }
      }
    }

    if (!(itinIter->cabinClassValid()))
    {
      ++sopIndex;
      continue;
    }

    // Get the travel segments for this schedule
    std::vector<TravelSeg*>& travelSegs = curItin->travelSeg();

    // If there are no travel segments for this schedule, we are not going to
    // group it with the others
    if (UNLIKELY(travelSegs.empty()))
    {
      ++sopIndex;
      continue;
    }

    // Create the itin cell info block and populate the necessary fields
    // for this point in the processing
    itinCellInfo.sopIndex() = sopIndex;
    itinCellInfo.itinColumnIndex() = sopIndex;
    itinCellInfo.globalDirection() = itinIter->globalDirection();
    itinCellInfo.combineSameCxr() = itinIter->combineSameCxr();

    // Add map entry and check for failure
    if (UNLIKELY(!addScheduleGroupMapEntry(curCarrierIndex, curItin, itinCellInfo)))
    {
      mapEntryFailed = true;
      break;
    }

    ++sopIndex;
  }

  // Set success flag based on map entry failure status
  if (!mapEntryFailed)
  {
    rt = true;
  }

  if (PricingTrx::IS_TRX == trx.getTrxType() && curLeg.preferredCabinClass().isEconomyClass())
  {
    int requestedNumberOfSeats = PaxTypeUtil::totalNumSeats(trx);
    double point = 0.0;
    int numberOfClassesToAdd = 0;

    ItinUtil::readConfigDataForSOPScore(point, numberOfClassesToAdd);

    if (numberOfClassesToAdd > 0)
    {
      SOPScoreComparator comparator(point,
                                    numberOfClassesToAdd,
                                    requestedNumberOfSeats,
                                    &sopScoreMap,
                                    curLeg,
                                    sopQualityCoeff.getValue());
      sortItinsBasedOnSOPScore(curCarrierIndex, comparator);
    }
  }
  return (rt);
}

void
ItinAnalyzerService::sortItinsBasedOnSOPScore(ItinIndex& curCarrierIndex,
                                              SOPScoreComparator& comparator)
{
  // go through  ItinMatrix
  ItinIndex::ItinMatrixIterator iMIter;

  for (iMIter = curCarrierIndex.root().begin(); iMIter != curCarrierIndex.root().end(); iMIter++)
  {
    ItinIndex::ItinRowIterator iRowIter;
    for (iRowIter = iMIter->second.begin(); iRowIter != iMIter->second.end(); iRowIter++)
    {
      std::sort(iRowIter->second.begin(), iRowIter->second.end(), comparator);
    }
  }

  // go through ItinRowCellMap
  ItinIndex::ItinRowCellMapIterator iRCMIt;
  for (iRCMIt = curCarrierIndex.rowCellMap().begin(); iRCMIt != curCarrierIndex.rowCellMap().end();
       iRCMIt++)
  {
    std::sort(iRCMIt->second.begin(), iRCMIt->second.end(), comparator);
  }
}

void
ItinAnalyzerService::getOtherGovCxrFMs(ShoppingTrx& trx,
                                       ItinIndex::ItinRow& itinRow,
                                       ItinIndex::ItinCell* directItinCell)
{
  Itin* directItin = directItinCell->second;
  if (!directItin)
    return;

  const FareMarket* itinFM = directItin->fareMarket().front();
  if (itinFM == nullptr)
    return;

  std::set<CarrierCode> allGovCxr;
  allGovCxr.insert(itinFM->governingCarrier());

  ItinIndex::ItinRow::iterator i = itinRow.begin();
  ItinIndex::ItinRow::iterator iEnd = itinRow.end();

  for (; i != iEnd; ++i)
  {
    ItinIndex::ItinColumn& itinColumn = i->second;
    for (ItinIndex::ItinCell& itinCell : itinColumn)
    {
      if (&itinCell == directItinCell)
        continue;

      if (!itinCell.second)
        continue;

      Itin& itin = *itinCell.second;
      for (FareMarket* fm : itin.fareMarket())
        if (allGovCxr.insert(fm->governingCarrier()).second)
          directItin->fareMarket().push_back(fm);
    }
  }
}

void
ItinAnalyzerService::addOtherGoverningCarriersToDirectFlight(ShoppingTrx& trx,
                                                             ShoppingTrx::Leg& curLeg)
{
  for (ItinIndex::ItinMatrix::value_type& carrierEntry : curLeg.carrierIndex().root())
  {
    ItinIndex::ItinCell* directItinCell = ShoppingUtil::retrieveDirectItin(
        curLeg.carrierIndex(), carrierEntry.first, ItinIndex::CHECK_NOTHING);

    if (!directItinCell)
      continue;

    getOtherGovCxrFMs(trx, carrierEntry.second, directItinCell);
  }
}

bool
ItinAnalyzerService::ensureGroupHasDirectFlight(ShoppingTrx& trx, ShoppingTrx::Leg& curLeg)
{
  TSELatencyData metrics(trx, "ITIN ENSURE DIRECTFLIGHT");

  PricingTrx& pricingTrx = static_cast<PricingTrx&>(trx);

  // Prepare to iterate over the carrier index for this leg
  ItinIndex& curIndex = curLeg.carrierIndex();
  ItinIndex::ItinMatrixIterator idxIter = curIndex.root().begin();
  ItinIndex::ItinMatrixIterator idxEndIter = curIndex.root().end();

  for (; idxIter != idxEndIter; ++idxIter)
  {
    // Look up number of direct flights for this governing cxr
    ItinIndex::ItinRow& curRow = idxIter->second;
    ItinIndex::Key directFlightKey;
    ShoppingUtil::createScheduleKey(directFlightKey);
    uint32_t directColumnCnt = curRow.count(directFlightKey);

    // If there is at least one direct itin for this carrier,
    // skip to the next carrier
    if ((directColumnCnt > 0) && (trx.getTrxType() != PricingTrx::ESV_TRX))
    {
      continue;
    }

    // Get the first full itin of the grouping for this carrier.
    // Since there is no direct flight yet, this will be the most

    //"direct" flight we can find
    ItinIndex::ItinColumn& firstColumnInRow = curRow.begin()->second;

    if (firstColumnInRow.empty())
    {
      continue;
    }

    // Insert the itin row pair (key, ItinColumn) that will represent
    // the direct flight itinerary for this carrier
    curRow.insert(ItinIndex::ItinRowPair(directFlightKey, ItinIndex::ItinColumn()));

    // Now we need to retrieve the newly added direct itin column
    // ItinIndex::ItinColumn& newColumn = curRow[directFlightKey];

    // Get data associated with first itin column
    ItinIndex::ItinCell& firstItinCell = firstColumnInRow.front();

    // Get the itinerary from this cell
    Itin*& firstItin = firstItinCell.second;

    // Get the governing carrier code
    const CarrierCode& govCxrCode = firstItin->fareMarket().front()->governingCarrier();

    // Find the segment with this carrier code
    std::vector<TravelSeg*>::iterator tSegIter = firstItin->travelSeg().begin();
    std::vector<TravelSeg*>::iterator tSegEIter = firstItin->travelSeg().end();
    AirSeg* airSegFound = nullptr;
    for (; tSegIter != tSegEIter; ++tSegIter)
    {
      AirSeg* curSeg = dynamic_cast<AirSeg*>(*(tSegIter));
      if ((curSeg != nullptr) && (curSeg->carrier() == govCxrCode))
      {
        airSegFound = curSeg;
      }
    }
    if (!airSegFound)
    {
      throw ErrorResponseException(ErrorResponseException::NO_VALID_FLIGHT_DATE_FOUND,
                                   "NO VALID FLIGHT/DATE FOUND");
    }

    if (!airSegFound)
    {
      throw ErrorResponseException(ErrorResponseException::NO_VALID_FLIGHT_DATE_FOUND,
                                   "NO VALID FLIGHT/DATE FOUND");
      return false;
    }

    const FlightNumber& flNumber = airSegFound->flightNumber();
    const CarrierCode& opCxr = airSegFound->operatingCarrierCode();
    const FlightNumber& opFlNumber = airSegFound->operatingFlightNumber();

    // Get the air segments representing the start and end of this itinerary
    const AirSeg& orig = *(firstItin->travelSeg().front()->toAirSeg());
    const AirSeg& dest = *(firstItin->travelSeg().back()->toAirSeg());

    // Generate a single air segment to represent the entire itinerary
    AirSeg* newAirSeg = generateNewAirSeg(trx, orig, dest, govCxrCode, flNumber, opCxr, opFlNumber);

    // Generate a new itinerary with the single air segment, ie, a direct itinerary
    Itin* newItin = generateNewItinInLeg(trx, curLeg, newAirSeg);

    newItin->setTravelDate(trx.journeyItin()->travelDate());

    // Get the gov cxr and fare market data handle
    getGovCxrAndFareMkt(trx, *newItin, false);

    // Get the global direction for this new itinerary

    // Create the itin cell info structure
    ItinIndex::ItinCellInfo itinCellInfo;
    itinCellInfo.sopIndex() = curLeg.sop().size();
    itinCellInfo.combineSameCxr() = firstItinCell.first.combineSameCxr();
    GlobalDirection& gDir = itinCellInfo.globalDirection();
    if (trx.isAltDates())
    {
      uint32_t legIndex = (&curLeg == &*(trx.legs().begin())) ? 0 : 1;
      DateTime travelDate;
      DatePair travelDatePair;
      if (ShoppingAltDateUtil::getDatePair(trx.altDatePairs(),
                                           newItin->travelSeg().front()->departureDT(),
                                           legIndex,
                                           travelDatePair))
      {
        travelDate = travelDatePair.first;
      }
      else
      {
        travelDate = trx.altDatePairs().begin()->first.first; // default to the first datepair
      }
      GlobalDirectionFinderV2Adapter::getGlobalDirection(
          &pricingTrx, travelDate, newItin->travelSeg(), gDir);
    }
    else
    {
      GlobalDirectionFinderV2Adapter::getGlobalDirection(
          &pricingTrx, newItin->travelDate(), newItin->travelSeg(), gDir);
    }
    // Set the global direction in the fare market
    newItin->fareMarket().front()->setGlobalDirection(gDir);

    // Make sure the cell info flag is set to show that this is a
    // fake direct flight
    itinCellInfo.flags() |= ItinIndex::ITININDEXCELLINFO_FAKEDIRECTFLIGHT;

    // Add the itin cell to the itin index
    curIndex.addItinCell(newItin, itinCellInfo, idxIter->first, directFlightKey);

    // If it's ESV request clear Fare markets it would be created later
    if (trx.getTrxType() == PricingTrx::ESV_TRX)
    {
      newItin->fareMarket().clear();
    }

    // Add this direct itinerary to the leg sop vector
    curLeg.addSop(ShoppingTrx::SchedulingOption(newItin, 0 /*fake either way*/));
    ShoppingTrx::SchedulingOption& newSop = curLeg.sop().back();
    newSop.globalDirection() = itinCellInfo.globalDirection();
    newSop.cabinClassValid() = false;
    newSop.setDummy(true);
    newSop.governingCarrier() = newItin->fareMarket().front()->governingCarrier();
    newSop.sopId() = curLeg.sop().size() - 1;
  }

  return (true);
}

bool
ItinAnalyzerService::createDummyItinForEveryGovCxr(ShoppingTrx& trx, ShoppingTrx::Leg& curLeg)
{
  TSELatencyData metrics(trx, "ITIN CREATE DUMMY ITIN FOR EVERY GOV CXR");

  LOG4CXX_DEBUG(
      _logger,
      "ItinAnalyzerService::createDummyItinForEveryGovCxr(ShoppingTrx&, ShoppingTrx::Leg&)");

  // Go thorough carriers from carrier index
  ItinIndex::ItinMatrixIterator matrixIter;

  for (matrixIter = curLeg.carrierIndex().root().begin();
       matrixIter != curLeg.carrierIndex().root().end();
       ++matrixIter)
  {
    // Get carrier row
    ItinIndex::ItinRow& curRow = matrixIter->second;

    // Get first itin column from a carrier row
    ItinIndex::ItinColumn& firstColumnInRow = curRow.begin()->second;

    // Get data associated with first itin column
    ItinIndex::ItinCell& firstItinCell = firstColumnInRow.front();
    ItinIndex::ItinCellInfo& ici = firstItinCell.first;

    // Get the governing carrier code
    const CarrierCode& govCxrCode = (curLeg.sop()[ici.sopIndex()]).governingCarrier();

    // Insert the itin row pair (key, ItinColumn) that will represent the
    // direct flight itinerary for this carrier
    ItinIndex::Key directFlightKey;
    ShoppingUtil::createScheduleKey(directFlightKey);

    curRow.insert(ItinIndex::ItinRowPair(directFlightKey, ItinIndex::ItinColumn()));

    // Create dummy air segment
    AirSeg* newAirSeg;
    trx.dataHandle().get(newAirSeg);
    newAirSeg->carrier() = govCxrCode;

    // Create dummy itin
    Itin* newItin;
    trx.dataHandle().get(newItin);
    newItin->travelSeg().push_back(newAirSeg);

    // Create the itin cell info structure
    ItinIndex::ItinCellInfo itinCellInfo;
    itinCellInfo.sopIndex() = curLeg.sop().size();
    itinCellInfo.flags() |= ItinIndex::ITININDEXCELLINFO_FAKEDIRECTFLIGHT;

    // Add the itin cell to the itin index
    curLeg.carrierIndex().addItinCell(newItin, itinCellInfo, matrixIter->first, directFlightKey);

    // Add this direct itinerary to the leg sop vector
    curLeg.sop().push_back(ShoppingTrx::SchedulingOption(newItin, true));
    ShoppingTrx::SchedulingOption& newSop = curLeg.sop().back();
    newSop.setDummy(true);
  }

  return true;
}

//----------------------------------------------------------------------------
// generateJourneyItin()
// Generate the journey itinerary that encompasses all of the legs
// contained in the shopping trx, excluding the across-stopover legs.
//----------------------------------------------------------------------------
bool
ItinAnalyzerService::generateJourneyItin(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "ITIN GENJOURNEY");
  std::vector<ShoppingTrx::Leg>& legs = trx.legs();

  if (legs.empty())
  {
    return (false);
  }

  LOG4CXX_DEBUG(_logger, "generateJourneyItin(ShoppingTrx)...");

  // Create leg iterators
  std::vector<ShoppingTrx::Leg>::iterator legIter = legs.begin();
  std::vector<ShoppingTrx::Leg>::iterator legEndIter = legs.end();

  // Allocate journey itin
  Itin*& journeyItin = trx.journeyItin();
  trx.dataHandle().get(journeyItin);
  trx.itin().insert(trx.itin().begin(), journeyItin);
  Itin& jrnItinRef = *journeyItin;
  std::vector<TravelSeg*>& jrnItinTSegs = jrnItinRef.travelSeg();

  // set up simple round trip
  jrnItinRef.simpleTrip() = trx.isSimpleTrip();
  // Loop through the legs
  int16_t cnt = 1;
  for (; legIter != legEndIter; ++legIter)
  {
    ShoppingTrx::Leg& curLeg = *legIter;

    if (curLeg.stopOverLegFlag())
    {
      continue;
    }

    std::pair<int16_t, int16_t>& journeyIndexPair = curLeg.journeyIndexPair();

    // Setup data
    ShoppingTrx::SchedulingOption& frontSop = curLeg.sop().front();
    Itin& sopItin = *(frontSop.itin());

    // Get the sop itin front air seg
    AirSeg& sopItinFront = dynamic_cast<AirSeg&>(*(sopItin.travelSeg().front()));

    // Get the sop itin back air seg
    AirSeg& sopItinBack = dynamic_cast<AirSeg&>(*(sopItin.travelSeg().back()));

    // Generate the new segment
    AirSeg* newSeg = generateNewAirSegMin(trx, sopItinFront, sopItinBack);

    // Set the new segment pnrSegment values
    newSeg->pnrSegment() = cnt;

    // Set the segment to open
    newSeg->segmentType() = Open;

    if (IS_DEBUG_ENABLED(_logger))
    {
      LOG4CXX_DEBUG(_logger,
                    "-- journey itin segment[" << cnt++ << "](" << newSeg->pnrSegment()
                                               << ") = " << newSeg->origin()->loc() << "("
                                               << newSeg->departureDT() << ") -> "
                                               << newSeg->destination()->loc() << "("
                                               << newSeg->arrivalDT().toSimpleString() << ")");
    }

    // Push the new air segment onto the journey itin
    jrnItinTSegs.push_back(newSeg);
    FareMarket* journeyFM;
    trx.dataHandle().get(journeyFM); // lint !e530
    jrnItinRef.fareMarket().push_back(journeyFM);
    journeyFM->travelSeg().push_back(newSeg);
    journeyFM->origin() = newSeg->origin();
    journeyFM->destination() = newSeg->destination();
    journeyFM->boardMultiCity() = newSeg->boardMultiCity();
    journeyFM->offMultiCity() = newSeg->offMultiCity();
    journeyFM->travelDate() = jrnItinRef.travelDate();

    // Set the journey index pair
    journeyIndexPair.first = cnt;
    journeyIndexPair.second = cnt;
    // make the journey itins for alternate dates
    for (ShoppingTrx::AltDatePairs::iterator i = trx.altDatePairs().begin();
         i != trx.altDatePairs().end();
         ++i)
    {
      ShoppingTrx::AltDateInfo& info = *(i->second);
      if (info.journeyItin == nullptr)
      {
        info.journeyItin = trx.dataHandle().create<Itin>();
      }

      AirSeg* newSeg = generateNewAirSegMin(trx, sopItinFront, sopItinBack);
      newSeg->departureDT() = (legIter == legs.begin()) ? i->first.first : i->first.second;
      newSeg->pssDepartureDate() = newSeg->departureDT().dateToSqlString();
      newSeg->pnrSegment() = cnt;
      newSeg->segmentType() = Open;

      info.journeyItin->travelSeg().push_back(newSeg);

      DatePair*& myPair = info.journeyItin->datePair();
      trx.dataHandle().get(myPair);
      *myPair = i->first;

      // info.journeyItin->datePair() = myPair;
      FareMarket* journeyFM;
      trx.dataHandle().get(journeyFM); // lint !e530
      info.journeyItin->fareMarket().push_back(journeyFM);
      journeyFM->travelSeg().push_back(newSeg);
      journeyFM->origin() = newSeg->origin();
      journeyFM->destination() = newSeg->destination();
      journeyFM->boardMultiCity() = newSeg->boardMultiCity();
      journeyFM->offMultiCity() = newSeg->offMultiCity();
      journeyFM->travelDate() = jrnItinRef.travelDate();

      // add durationAltDatePairs into PricingTrx
      uint64_t duration =
          (myPair->second.get64BitRepDateOnly()) - (myPair->first.get64BitRepDateOnly());

      std::map<uint64_t, PricingTrx::AltDatePairs>::iterator p;
      p = trx.durationAltDatePairs().find(duration);

      if (p == trx.durationAltDatePairs().end())
      {
        PricingTrx::AltDatePairs datePairsMap;
        datePairsMap.insert(*i);
        std::pair<uint64_t, PricingTrx::AltDatePairs> datePairItem(duration, datePairsMap);
        trx.durationAltDatePairs().insert(datePairItem);
      }
      else
      {
        p->second.insert(*i);
      }
    }
  }
  // set main duration in pricingTrx
  std::map<uint64_t, PricingTrx::AltDatePairs>::const_iterator iter =
      trx.durationAltDatePairs().begin();
  std::map<uint64_t, PricingTrx::AltDatePairs>::const_iterator iterEnd =
      trx.durationAltDatePairs().end();
  uint64_t mainDuration = 0;
  size_t altDatePairSize = 0;
  for (; iter != iterEnd; iter++)
  {
    if (altDatePairSize < iter->second.size())
    {
      altDatePairSize = iter->second.size();
      mainDuration = iter->first;
    }
  }
  trx.mainDuration() = mainDuration;
  // Generate the travel boundaries for the journey itin
  Boundary tvlBoundary = _tvlSegAnalysis.selectTravelBoundary(jrnItinTSegs);
  ItinUtil::setGeoTravelType(_tvlSegAnalysis, tvlBoundary, jrnItinRef);

  // Set misc journey itin data
  jrnItinRef.setTravelDate(TseUtil::getTravelDate(jrnItinTSegs));
  jrnItinRef.bookingDate() = TseUtil::getBookingDate(jrnItinTSegs);

  for (ShoppingTrx::AltDatePairs::iterator i = trx.altDatePairs().begin();
       i != trx.altDatePairs().end();
       ++i)
  {
    ShoppingTrx::AltDateInfo& info = *(i->second);
    Itin& itin = *info.journeyItin;
    ItinUtil::setGeoTravelType(_tvlSegAnalysis, tvlBoundary, itin);
    itin.setTravelDate(TseUtil::getTravelDate(itin.travelSeg()));
    itin.bookingDate() = TseUtil::getBookingDate(itin.travelSeg());
  }

  return (true);
}

void
ItinAnalyzerService::classifyLegSurfaceSectorTypes(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "ITIN CLASSIFY SURFSECT");
  LOG4CXX_DEBUG(_logger, "ItinAnalyzerService::classifyLegSurfaceSectorTypes(ShoppingTrx)...");
  std::vector<ShoppingTrx::Leg>& legs = trx.legs();
  if (legs.empty() || legs.size() == 0)
  {
    LOG4CXX_INFO(_logger, "Transaction contains no legs");
    // Cannot have a surface sector when there are no legs
    return;
  }
  uint32_t legsSize = legs.size();

  if (legsSize == 1)
  {
    LOG4CXX_INFO(_logger, "There cannot be a surface sector with only one leg.");
    // Cannot have a surface sector when there is only one leg
    return;
  }

  // Store surface leg indices
  IndexPairVector surfaceLegIndices;

  // Classify all of the legs in the transaction
  for (size_t j = 0; j < legs.size() - 1; j++)
  {
    ShoppingTrx::Leg& curLeg = legs[j];
    ShoppingTrx::Leg& nextLeg = legs[j + 1];
    ShoppingTrx::Leg::SurfaceSectorLegType& curLegType = curLeg.surfaceSectorLegType();
    ShoppingTrx::Leg::SurfaceSectorLegType& nextLegType = nextLeg.surfaceSectorLegType();

    ShoppingTrx::SchedulingOption& curLegSop = curLeg.sop()[0];
    ShoppingTrx::SchedulingOption& nextLegSop = nextLeg.sop()[0];

    const Itin* curLegItin = curLegSop.itin();
    const Itin* nextLegItin = nextLegSop.itin();

    const TravelSeg* curLegDest = curLegItin->travelSeg().back();
    const TravelSeg* nextLegOrig = nextLegItin->travelSeg().front();

    //----------------------------------------------------------
    //-- A surface sector exists when the following occurs:
    //--   o  LegA comes before LegB in the trip
    //     o  If LegA's destination off point is
    //         NOT equal to LegB's origin board point
    //----------------------------------------------------------
    const LocCode& curLegDestOff = curLegDest->offMultiCity();
    const LocCode& nextLegOrigBrd = nextLegOrig->boardMultiCity();
    if (curLegDestOff != nextLegOrigBrd)
    {
      surfaceLegIndices.push_back(IndexPair(j, j + 1));

      // Depending on a leg's current surface sector type, a new type
      // will be determined
      LOG4CXX_DEBUG(_logger,
                    "- Found surface sector between leg "
                        << j << "and leg " << j + 1 << " -- curLegDestOff = " << curLegDestOff
                        << " -- nextLegOrigBrd = " << nextLegOrigBrd);

      if (curLegType == ShoppingTrx::Leg::SurfaceSectorLegNone)
      {
        curLegType = ShoppingTrx::Leg::SurfaceSectorBeginAtLegDest;
        LOG4CXX_DEBUG(_logger, "-- Setting curLegType = SurfaceSectorBeginAtLegDest");
      }
      else
      {
        curLegType = ShoppingTrx::Leg::SurfaceSectorEndAtLegOrigBeginAtLegDest;
        LOG4CXX_DEBUG(_logger, "-- Setting curLegType = SurfaceSectorEndAtLegOrigBeginAtLegDest");
      }

      if (nextLegType == ShoppingTrx::Leg::SurfaceSectorLegNone)
      {
        nextLegType = ShoppingTrx::Leg::SurfaceSectorEndAtLegOrig;
        LOG4CXX_DEBUG(_logger, "-- Setting nextLegType = SurfaceSectorEndAtLegOrig");
      }
      else
      {
        nextLegType = ShoppingTrx::Leg::SurfaceSectorEndAtLegOrigBeginAtLegDest;
        LOG4CXX_DEBUG(_logger, "-- Setting nextLegTlype = SurfaceSectorEndAtLegOrigBeginAtLegDest");
      }
    }
  }
}

//----------------------------------------------------------------------------
// groupSchedules()
// Group all of the schedules in the transaction
//----------------------------------------------------------------------------

bool
ItinAnalyzerService::groupSchedules(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "ITIN GROUP SCHEDULES");
  std::vector<ShoppingTrx::Leg>::iterator legIter = trx.legs().begin();
  std::vector<ShoppingTrx::Leg>::iterator endLegIter = trx.legs().end();
  // bool rt = false;
  // bool groupingFailed = false;

  for (; legIter != endLegIter; legIter++)
  {
    ShoppingTrx::Leg& curLeg = *legIter;

    if (!groupSchedulesByGovCxrAndConxnPoint(curLeg, trx))
    {
      // groupingFailed = true;
      return (false);
    }

    // Make sure each governing carrier in a leg has at least
    // one direct flight schedule -- no need to check if
    // across stop over legs have direct flights
    if (curLeg.stopOverLegFlag())
    {
      continue;
    }

    if (!ensureGroupHasDirectFlight(trx, curLeg))
    {
      // groupingFailed = true;
      return (false);
    }

    if (trx.isIataFareSelectionApplicable())
      addOtherGoverningCarriersToDirectFlight(trx, curLeg);
  }

  return (true);
}

bool
ItinAnalyzerService::groupSchedulesESV(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "ITIN GROUP SCHEDULES ESV");

  LOG4CXX_DEBUG(_logger, "ItinAnalyzerService::groupSchedulesESV(ShoppingTrx&)");

  // Go thorough all legs
  std::vector<ShoppingTrx::Leg>::iterator legIter;

  for (legIter = trx.legs().begin(); legIter != trx.legs().end(); ++legIter)
  {
    ShoppingTrx::Leg& curLeg = (*legIter);
    uint32_t sopIndex = 0;

    // Go thorough all scheduling options
    std::vector<ShoppingTrx::SchedulingOption>::iterator sopIter;

    for (sopIter = curLeg.sop().begin(); sopIter != curLeg.sop().end(); ++sopIter, ++sopIndex)
    {
      ShoppingTrx::SchedulingOption& curSop = (*sopIter);
      curSop.sopId() = sopIndex;

      Itin*& curItin = curSop.itin();

      if (nullptr == curItin)
      {
        LOG4CXX_ERROR(_logger,
                      "ItinAnalyzerService::groupSchedulesESV - Itinerary object is NULL.");
        continue;
      }

      if (curItin->travelSeg().empty())
      {
        LOG4CXX_ERROR(_logger,
                      "ItinAnalyzerService::groupSchedulesESV - Travel segments vector is empty.");
        continue;
      }

      curItin->setTravelDate(trx.journeyItin()->travelDate());

      // We don't need to determine governing carrier for domestic request
      // instead of it we will group itineraries by carrier taken from
      // first travel segment of each itinerary
      TravelSeg* travelSeg = curSop.itin()->travelSeg()[0];
      const AirSeg& airSegment = dynamic_cast<const AirSeg&>(*(travelSeg));
      curSop.governingCarrier() = airSegment.carrier();

      // Create the itin cell info block and populate the necessary fields
      // for this point in the processing
      ItinIndex::ItinCellInfo itinCellInfo;
      itinCellInfo.sopIndex() = sopIndex;
      itinCellInfo.itinColumnIndex() = sopIndex;

      // Add entry to map
      addScheduleGroupMapEntryESV(curLeg, curSop, itinCellInfo);
    }

    // Create dummy itinerary for every governing carrier
    createDummyItinForEveryGovCxr(trx, curLeg);
  }

  return true;
}

//----------------------------------------------------------------------------
// getGovCxrAndFareMkt()
//  Determine the governing carrier and retrieve the fare
//  market from the DataHandle. This does not retrieve fares,
//  fare retrieval occurs ONLY in the FareCollectorOrchestrator,
//  which occurs AFTER we leave the ItinAnalyzer.   It should
//  be referred to as allocate fare market object. Build fare market
//  suggests a fare retrieval operation, which does not ever
//  occur in the ItinAnalyzer.  This applies for both pricing and
//  shopping
//----------------------------------------------------------------------------
bool
ItinAnalyzerService::getGovCxrAndFareMkt(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "ITIN SHOP GOVCXR AND FMKT");
  std::vector<ShoppingTrx::Leg>::iterator legIter = trx.legs().begin();
  std::vector<ShoppingTrx::Leg>::iterator endLegIter = trx.legs().end();

  PricingTrx& pricingTrx = static_cast<PricingTrx&>(trx);
  pricingTrx.setTravelDate(trx.journeyItin()->travelDate());
  GoverningCarrier govCxr(&trx);

  for (uint32_t legIndex = 0; legIter != endLegIter; ++legIter, ++legIndex)
  {
    // Resolve the current leg
    ShoppingTrx::Leg& curLeg = *legIter;

    std::vector<ShoppingTrx::SchedulingOption>::iterator itinIter = curLeg.sop().begin();
    std::vector<ShoppingTrx::SchedulingOption>::iterator endItinIter = curLeg.sop().end();
    bool inboudSopSimpleTrip = false;
    if (trx.isSimpleTrip() && legIndex == 1)
    {
      inboudSopSimpleTrip = true;
    }
    for (/*itinIndex=0*/; itinIter != endItinIter; ++itinIter /*, ++itinIndex*/)
    {
      // Resolve the current itinerary
      Itin*& curItin = itinIter->itin();

      if (UNLIKELY(curItin == nullptr))
      {
        continue;
      }

      curItin->setTravelDate(trx.journeyItin()->travelDate());
      if (inboudSopSimpleTrip)
      {
        curItin->simpleTrip() = true;
        curItin->inboundSop() = true;
      }
      // Get the governing carrier and the fare market handle
      // without setting the inbound/outbound indicators
      // The false parameter is used to designate this desired functionality
      getGovCxrAndFareMkt(trx, *curItin, false, itinIter->isCustomSop(), legIndex);

      // Get the governing carrier for this itinerary

      FareMarket* fm = curItin->fareMarket().front();

      CarrierCode highestGC = govCxr.getHighestTPMCarrier(fm->travelSeg(),
                                                          fm->direction(),
                                                          fm->primarySector());
      if(!fallback::fallbackFareSelction2016(&trx))
      {
        if (!highestGC.empty() && fm->governingCarrier()!=highestGC)
        {
          itinanalyzerutils::setSopWithHighestTPM(trx, legIndex, itinIter->originalSopId());
        }
      }
      itinanalyzerutils::setGovCxrForSubIata21(trx, *fm, govCxr, highestMileagePercent.getValue());

      itinIter->governingCarrier() = curItin->fareMarket().front()->governingCarrier();


      // Get the global direction for this itinerary
      GlobalDirection& gDir = itinIter->globalDirection();
      if (trx.isAltDates())
      {
        DateTime travelDate;
        DatePair travelDatePair;
        if (ShoppingAltDateUtil::getDatePair(trx.altDatePairs(),
                                             curItin->travelSeg().front()->departureDT(),
                                             legIndex,
                                             travelDatePair))
        {
          travelDate = travelDatePair.first;
        }
        else
        {
          itinIter->cabinClassValid() = false; // to make this sop invalid
          continue;
        }
        GlobalDirectionFinderV2Adapter::getGlobalDirection(
            &pricingTrx, travelDate, curItin->travelSeg(), gDir);
      }
      else
      {
        GlobalDirectionFinderV2Adapter::getGlobalDirection(
            &pricingTrx, curItin->travelDate(), curItin->travelSeg(), gDir);
      }
      curItin->fareMarket().front()->setGlobalDirection(gDir);
      curItin->fareMarket().front()->setCombineSameCxr(itinIter->combineSameCxr());
    }
  }

  return (true);
}

//----------------------------------------------------------------------------
// Build a vector containing the direct itins from each leg
//----------------------------------------------------------------------------
void
ItinAnalyzerService::buildLegDirectItins(ShoppingTrx& trx,
                                         const uint32_t& legId,
                                         std::vector<Itin*>& dItins)
{
  TSELatencyData metrics(trx, "ITIN BLDLEGDIRECTITINS");
  // Get the carrier index for this leg
  ShoppingTrx::Leg& curLeg = trx.legs()[legId];
  ItinIndex& curGrp = curLeg.carrierIndex();

  // Acquire carrier index iterators
  ItinIndex::ItinMatrixIterator rtIter = curGrp.root().begin();
  ItinIndex::ItinMatrixIterator rtEndIter = curGrp.root().end();
  ItinIndex::ItinCell* curItinCell;

  dItins.reserve(curGrp.root().size());

  // Get direct itineraries
  for (; rtIter != rtEndIter; ++rtIter)
  {
    // Get the key
    const ItinIndex::Key& cCode = static_cast<const ItinIndex::Key&>(rtIter->first);

    // Get the direct itin for this carrier
    curItinCell = ShoppingUtil::retrieveDirectItin(curGrp, cCode, ItinIndex::CHECK_NOTHING);

    if (LIKELY(curItinCell))
    {
      Itin* curItin = curItinCell->second;
      dItins.push_back(curItin);
    }
  }
}

//----------------------------------------------------------------------------
// Unary function for filtering stop over leg pair combinations
//----------------------------------------------------------------------------
class FilterStopOverPairsByIndex : public std::unary_function<IndexPair, bool>
{
private:
  const IndexVector& _indexDiff;

public:
  FilterStopOverPairsByIndex(const IndexVector& indexDiff) : _indexDiff(indexDiff) {}

  bool operator()(IndexPair& x)
  {
    IndexVectorConstIterator iter = _indexDiff.begin();
    IndexVectorConstIterator endIter = _indexDiff.end();

    for (; iter != endIter; ++iter)
    {
      if (abs(static_cast<int>(static_cast<int>(x.first) - static_cast<int>(x.second))) ==
          static_cast<int>(*iter))
      {
        return (true);
      }
    }

    return (false);
  }
}; // lint !e1510

//----------------------------------------------------------------------------
// Binary function for determining the ordering between two ClassOfService
// objects based on booking code value
//----------------------------------------------------------------------------
class SortClassOfServiceVector : public std::binary_function<ClassOfService*, ClassOfService*, bool>
{
public:
  bool operator()(const ClassOfService* x, const ClassOfService* y)
  {
    return (x->bookingCode() < y->bookingCode());
  }
};

//----------------------------------------------------------------------------
// Binary function for determining uniqueness between two ClassOfService
// objects based on booking code value
//----------------------------------------------------------------------------
class UniqueClassOfServiceVectorFilter
    : public std::binary_function<ClassOfService*, ClassOfService*, bool>
{
public:
  bool operator()(const ClassOfService* x, const ClassOfService* y)
  {
    return (x->bookingCode() == y->bookingCode());
  }
};

//----------------------------------------------------------------------------
// Debug function for printing log stop over indices
//----------------------------------------------------------------------------
void
ItinAnalyzerService::logStopOverIndex(const ShoppingTrx& trx, const IndexPairVector& index)
{
  if (!IS_DEBUG_ENABLED(_logger))
  {
    return;
  }
#ifndef DEBUG_LOG_DISABLED
  IndexPairVectorConstIterator buckIter = index.begin();
  IndexPairVectorConstIterator buckEndIter = index.end();
  const std::vector<ShoppingTrx::Leg>& legs = trx.legs();
  int i = 0;

  LOG4CXX_DEBUG(_logger, "---------------------------------------------");
  LOG4CXX_DEBUG(_logger, "- Leg Combinations:");
  for (; buckIter != buckEndIter; ++buckIter)
  {
    const IndexPair& curPair = *buckIter;
    const Itin* firstItin = legs[curPair.first].sop().front().itin();
    const Itin* secondItin = legs[curPair.second].sop().back().itin();
    const TravelSeg* firstTSeg = firstItin->travelSeg().front();
    const TravelSeg* secondTSeg = secondItin->travelSeg().back();

    LOG4CXX_DEBUG(_logger,
                  "-- Combination["
                      << i++ << "] = (" << firstTSeg->origin()->loc()
                      << ")(B:" << firstTSeg->boardMultiCity() << ",O:" << firstTSeg->offMultiCity()
                      << ")" << curPair.first << " -> (" << secondTSeg->destination()->loc()
                      << ")(B:" << secondTSeg->boardMultiCity()
                      << ",O:" << secondTSeg->offMultiCity() << ")" << curPair.second);
  }
  LOG4CXX_DEBUG(_logger, "---------------------------------------------");
#endif
}

//----------------------------------------------------------------------------
// Debug function for printing out leg data
//----------------------------------------------------------------------------
void
ItinAnalyzerService::logLegData(const ShoppingTrx& trx)
{
  if (!IS_DEBUG_ENABLED(_logger))
  {
    return;
  }

  std::vector<ShoppingTrx::Leg>::const_iterator legIter = trx.legs().begin();
  std::vector<ShoppingTrx::Leg>::const_iterator legEndIter = trx.legs().end();
  LOG4CXX_DEBUG(_logger, "---------------------------------------------");
  LOG4CXX_DEBUG(_logger, "- Legs:");
  uint32_t i = 0;
  for (; legIter != legEndIter; ++legIter)
  {
    logLegData(trx, i, *legIter);
    ++i;
  }

  LOG4CXX_DEBUG(_logger, "---------------------------------------------");
}

void
ItinAnalyzerService::logLegData(const ShoppingTrx& trx,
                                uint32_t& id,
                                const ShoppingTrx::Leg& curLeg)
{
  if (!IS_DEBUG_ENABLED(_logger))
  {
    return;
  }
#ifndef DEBUG_LOG_DISABLED
  const Itin* firstItin = curLeg.sop().front().itin();
  const Itin* secondItin = curLeg.sop().back().itin();
  const TravelSeg* firstTSeg = firstItin->travelSeg().front();
  const TravelSeg* secondTSeg = secondItin->travelSeg().back();
#endif
  LOG4CXX_DEBUG(_logger,
                "-- Leg[" << id << "] = (" << firstTSeg->origin()->loc() << ")(B:"
                          << firstTSeg->boardMultiCity() << ",O:" << firstTSeg->offMultiCity()
                          << ") -> (" << secondTSeg->destination()->loc() << ")(B:"
                          << secondTSeg->boardMultiCity() << ",O:" << secondTSeg->offMultiCity()
                          << ")(" << curLeg.carrierIndex().root().size() << ")");
  if (curLeg.stopOverLegFlag())
  {
    LOG4CXX_DEBUG(_logger, "--- #STOP-OVER LEG# ---");
    LOG4CXX_DEBUG(_logger, "--- Legs crossed over: Size " << curLeg.jumpedLegIndices().size());
    std::ostringstream oStr;
    std::copy(curLeg.jumpedLegIndices().begin(),
              curLeg.jumpedLegIndices().end(),
              std::ostream_iterator<uint32_t>(oStr, " "));
    LOG4CXX_DEBUG(_logger, "Jumped leg indices = " << oStr.str());
    oStr.flush();

    LOG4CXX_DEBUG(_logger,
                  "--- SOver Combination: " << curLeg.stopOverLegCombination().first << ","
                                            << curLeg.stopOverLegCombination().second);
    LOG4CXX_DEBUG(_logger, "--- adoptedLegRefIndx: " << curLeg.adoptedCrossedLegRefIndex());
  }
}

void
ItinAnalyzerService::logSegmentData(const AirSeg& aSeg)
{
  if (LIKELY(!IS_DEBUG_ENABLED(_logger)))
  {
    return;
  }

  LOG4CXX_DEBUG(_logger, "Air Segment Data:");
  LOG4CXX_DEBUG(_logger, "- origAirport = " << aSeg.origAirport());
  LOG4CXX_DEBUG(_logger, "- departureDT = " << aSeg.departureDT().toSimpleString());
  LOG4CXX_DEBUG(_logger, "- origin      = " << aSeg.origin()->loc());
  LOG4CXX_DEBUG(_logger, "- destAirport = " << aSeg.destAirport());
  LOG4CXX_DEBUG(_logger, "- arrivalDT   = " << aSeg.arrivalDT().toSimpleString());
  LOG4CXX_DEBUG(_logger, "- destination = " << aSeg.destination()->loc());
  LOG4CXX_DEBUG(_logger, "- carrier     = " << aSeg.carrier());
  LOG4CXX_DEBUG(_logger, "- boardCity   = " << aSeg.boardMultiCity());
  LOG4CXX_DEBUG(_logger, "- offCity     = " << aSeg.offMultiCity());
  LOG4CXX_DEBUG(_logger, "- flightNum   = " << aSeg.flightNumber());
  LOG4CXX_DEBUG(_logger, "- opCarrier   = " << aSeg.operatingCarrierCode());
  LOG4CXX_DEBUG(_logger, "- opFlightNum = " << aSeg.operatingFlightNumber());
  LOG4CXX_DEBUG(_logger,
                "- segmentType = " << ((aSeg.segmentType() == Arunk) ? "ARUNK" : "NON-ARUNK"));
}

//----------------------------------------------------------------------------
// Primary function for generating stop over legs
//----------------------------------------------------------------------------
void
ItinAnalyzerService::generateStopOverLegs(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "ITIN GEN ASOLEGS");
  std::vector<ShoppingTrx::Leg>& legs = trx.legs();

  if (legs.size() < MIN_NUMBER_LEGS_FOR_STOPOVER)
  {
    return;
  }

  // Output the information pertaining to the original input legs
  if (IS_DEBUG_ENABLED(_logger))
  {
    LOG4CXX_DEBUG(_logger, "Original legs:");
    logLegData(trx);
  }

  // Get necessary data
  ShoppingTrx::Leg& firstLeg = legs.front();
  ShoppingTrx::Leg& lastLeg = legs.back();
  ShoppingTrx::SchedulingOption& firstSOP = firstLeg.sop().front();
  ShoppingTrx::SchedulingOption& lastSOP = lastLeg.sop().front();
  Itin*& firstItin = firstSOP.itin();
  Itin*& lastItin = lastSOP.itin();

  // Get the end location and the start location
  // Determine if this is a round trip or not
  bool roundTrip = false;
  if (fallback::fallbackStopOverLegByCityCode(&trx))
  {
    if (firstItin->travelSeg().front()->origin()->loc() ==
        lastItin->travelSeg().back()->destination()->loc())
      roundTrip = true;
  }
  else
  {
    if (firstItin->travelSeg().front()->boardMultiCity() ==
        lastItin->travelSeg().back()->offMultiCity())
      roundTrip = true;
  }

  if (roundTrip && legs.size() == MIN_NUMBER_LEGS_FOR_STOPOVER)
  {
    return;
  }

  // Create the combination vector
  IndexPairVector combinations;

  // Generate the combination bucket
  generateStopOverLegCombinationPairs(trx, combinations, roundTrip);

  // Prune invalid combinations
  pruneStopOverLegCombinationPairs(trx, combinations, roundTrip);

  // Generate the stop over legs based on the pruned results
  generateStopOverLegsFromCombinationPairs(trx, combinations);

  if (IS_DEBUG_ENABLED(_logger))
  {
    LOG4CXX_DEBUG(_logger, "Original legs + stopover legs:");
    logLegData(trx);
  }
}

//----------------------------------------------------------------------------
// Generates the pairs of indices used for stop over legs
//----------------------------------------------------------------------------
void
ItinAnalyzerService::generateStopOverLegCombinationPairs(const ShoppingTrx& trx,
                                                         IndexPairVector& combinations,
                                                         const bool& isRoundTrip)

{
  // Get leg reference
  const std::vector<ShoppingTrx::Leg>& legs = trx.legs();

  // Create offset value based on isRoundTrip boolean
  uint32_t roundTripOffset = static_cast<uint32_t>((isRoundTrip) ? 1 : 0);

  // Create the new leg index limits
  uint32_t indexOneLimit = legs.size() - roundTripOffset;
  uint32_t indexTwoLimit = legs.size();

  // Loop and generate all possible combinations
  for (uint32_t indexOne = 0; indexOne < indexOneLimit; indexOne++)
  {
    for (uint32_t indexTwo = indexOne + roundTripOffset; indexTwo < indexTwoLimit; indexTwo++)
    {
      // Add the current pair to the pair vector
      combinations.push_back(IndexPair(indexOne, indexTwo));
    }
  }
}

void
ItinAnalyzerService::preparePruningRemovalList(const ShoppingTrx& trx,
                                               IndexPairVector& combinations,
                                               IndexPairVector& removeList)
{
  IndexPairVectorIterator buckIter = combinations.begin();
  IndexPairVectorIterator buckEndIter = combinations.end();
  const std::vector<ShoppingTrx::Leg>& legs = trx.legs();

  // Remove combinations that have equal Board/Board points and
  // equal Off/Off points OR combinations that have equal
  // Board/Off points and equal Off/Board points
  // Also remove combinations where the first segment's board point
  // is equal to the second segments off point
  // Perform this logic and add the candidates to the removeList
  for (; buckIter != buckEndIter; ++buckIter)
  {
    IndexPair& curPair = *buckIter;
    const ShoppingTrx::Leg& legOne = legs[curPair.first];
    const ShoppingTrx::Leg& legTwo = legs[curPair.second];
    const Itin* itinOne = legs[curPair.first].sop().front().itin();
    const Itin* itinTwo = legs[curPair.second].sop().front().itin();

    const TravelSeg* itinOneTSeg = itinOne->travelSeg().front();
    const TravelSeg* itinTwoTSeg = itinTwo->travelSeg().back();
    const LocCode& itinOneBrd = itinOneTSeg->boardMultiCity();
    const LocCode& itinOneOff = itinOneTSeg->offMultiCity();
    const LocCode& itinTwoBrd = itinTwoTSeg->boardMultiCity();
    const LocCode& itinTwoOff = itinTwoTSeg->offMultiCity();
    const LocCode& itinOneOrigin = itinOneTSeg->origin()->loc();
    const LocCode& itinOneDestin = itinOneTSeg->destination()->loc();
    const LocCode& itinTwoOrigin = itinTwoTSeg->origin()->loc();
    const LocCode& itinTwoDestin = itinTwoTSeg->destination()->loc();
    // const ShoppingTrx::Leg::SurfaceSectorLegType& curLegSSType =
    //  legOne.surfaceSectorLegType();
    // const ShoppingTrx::Leg::SurfaceSectorLegType& nexLegSSType =
    //  legTwo.surfaceSectorLegType();

    //--1 BRD --- 1 OFF - 2 BRD --- 2 OFF--//
    if ((legOne.surfaceSectorLegType() != ShoppingTrx::Leg::SurfaceSectorLegNone) ||
        (legTwo.surfaceSectorLegType() != ShoppingTrx::Leg::SurfaceSectorLegNone))
    {
      LOG4CXX_DEBUG(_logger,
                    "Not fully filtering combination [" << curPair.first << "," << curPair.second
                                                        << "]");
      if (itinOneOrigin == itinTwoOrigin && itinOneDestin == itinTwoDestin)
      {
        removeList.push_back(curPair);
      }
      else if (itinOneOrigin == itinTwoDestin)
      {
        removeList.push_back(curPair);
      }
      else
      {
        addExistingLegCombinationsToRemovalList(
            legs, itinOneOrigin, itinTwoDestin, curPair, removeList);
      }
      continue;
    }

    // Do not allow across stop over legs which has org-dest TPM less than nn% of travel TPM
    // nn is the percentage value set in configurable file
    // Ex. A-B-C if TPM from A-C is less than nn% of TPM from A-B plus B-C, then remove this ASO leg
    // A-C
    //
    uint32_t travelMiles = 0;
    uint32_t origDestMiles = 0;

    const DateTime* travelDate = nullptr;

    if (itinOne->travelDate().isEmptyDate())
    {
      travelDate = &(itinOne->travelSeg().front()->departureDT());
    }
    else
    {
      travelDate = &(itinOne->travelDate());
    }

    GlobalDirection gd;
    PricingTrx& pricingTrx = const_cast<PricingTrx&>(static_cast<const PricingTrx&>(trx));

    //---
    // Find origin to destination miles
    //---
    const Loc& loc1 = *(itinOneTSeg)->origin();
    const Loc& loc4 = *(itinTwoTSeg)->destination();
    std::vector<TravelSeg*> tempTravelSegs;
    tempTravelSegs.insert(
        tempTravelSegs.begin(), itinOne->travelSeg().begin(), itinOne->travelSeg().end());
    tempTravelSegs.insert(
        tempTravelSegs.end(), itinTwo->travelSeg().begin(), itinTwo->travelSeg().end());

    GlobalDirectionFinderV2Adapter::getGlobalDirection(
        &pricingTrx, *travelDate, tempTravelSegs, gd);

    origDestMiles = LocUtil::getTPM(loc1, loc4, gd, *travelDate, pricingTrx.dataHandle());

    //---
    // Find miles for each inside leg and sum them
    //---
    const Loc& loc2 = *(itinOneTSeg)->destination();
    const Loc& loc3 = *(itinTwoTSeg)->origin();

    GlobalDirectionFinderV2Adapter::getGlobalDirection(
        &pricingTrx, *travelDate, itinOne->travelSeg(), gd);

    travelMiles += LocUtil::getTPM(loc1, loc2, gd, *travelDate, pricingTrx.dataHandle());

    GlobalDirectionFinderV2Adapter::getGlobalDirection(
        &pricingTrx, *travelDate, itinTwo->travelSeg(), gd);

    travelMiles += LocUtil::getTPM(loc3, loc4, gd, *travelDate, pricingTrx.dataHandle());

    // Determine if this leg can be removed
    if (origDestMiles < (asoThruMileagePercent.getValue() * travelMiles) / 100)
    {
      removeList.push_back(curPair);
      continue;
    }

    // If the board and off points are similar in either case, add
    // this pair to the removal vector
    if (((itinOneBrd == itinTwoBrd) && (itinOneOff == itinTwoOff)) ||
        ((itinOneBrd == itinTwoOff) && (itinOneOff == itinTwoBrd)) || (itinOneBrd == itinTwoOff))
    {
      removeList.push_back(curPair);
    }
    // Otherwise if the origin/destination of the first travel seg is
    // equal to the origin/destination of the second travel seg, add
    // this pair to the removal vector
    else if ((itinOneOrigin == itinTwoOrigin) && (itinOneDestin == itinTwoDestin))
    {
      removeList.push_back(curPair);
    }
    else
    {
      addExistingLegCombinationsToRemovalList(
          legs, itinOneOrigin, itinTwoDestin, curPair, removeList);
    }
  }
}

void
ItinAnalyzerService::addExistingLegCombinationsToRemovalList(
    const std::vector<ShoppingTrx::Leg>& legs,
    const LocCode& itinOneOrigin,
    const LocCode& itinTwoDestin,
    IndexPair& curPair,
    IndexPairVector& removeList)
{
  // Check to see if the pair combinations have similarities to the existing legs
  std::vector<ShoppingTrx::Leg>::const_iterator legSimIter = legs.begin();
  std::vector<ShoppingTrx::Leg>::const_iterator legSimEndIter = legs.end();
  bool matchLeg = false;

  for (; legSimIter != legSimEndIter; ++legSimIter)
  {
    const ShoppingTrx::Leg& curLeg = static_cast<const ShoppingTrx::Leg&>(*legSimIter);
    const std::vector<TravelSeg*>& curLegTSeg = curLeg.sop().front().itin()->travelSeg();
    const LocCode& curLegOrigin = curLegTSeg.front()->origin()->loc();
    const LocCode& curLegDestin = curLegTSeg.back()->destination()->loc();

    matchLeg = false;

    if ((itinOneOrigin == curLegOrigin) && (itinTwoDestin == curLegDestin))
    {
      matchLeg = true;
      break;
    }
  }

  if (matchLeg)
  {
    removeList.push_back(curPair);
  }
}

//----------------------------------------------------------------------------
// Prunes the indices using travel segment / itin data
//----------------------------------------------------------------------------
void
ItinAnalyzerService::pruneStopOverLegCombinationPairs(const ShoppingTrx& trx,
                                                      IndexPairVector& combinations,
                                                      const bool& isRoundTrip)
{
  if (combinations.empty())
  {
    return;
  }

  // uint32_t startSize = combinations.size();
  // Get legs reference
  const std::vector<ShoppingTrx::Leg>& legs = trx.legs();
  uint32_t legSz = legs.size();

  // Print the vector to see what has been removed
  if (IS_DEBUG_ENABLED(_logger))
  {
    logStopOverIndex(trx, combinations);
  }

  // Create index removal vector
  IndexVector indexRemovals;

  // Only index differences of zero will be removed
  indexRemovals.push_back(0);

  //### ASO Optimization # 1 ###//
  // Do not allow across stop over legs to exceed crossing more
  // than two legs (i.e. index differences of 2 or more)
  // We'll add removals for indices up to 10 just in case
  // we have a 10 leg request ;)
  // We need at least three legs in the request to have
  // a difference of two in the jumped leg indices
  if (legSz >= 3)
  {
    for (uint32_t n = 2; n <= legSz; n++)
    {
      indexRemovals.push_back(n);
    }
  }
  //###

  // Erase pairs from the combination vector using a filter functor
  FilterStopOverPairsByIndex fSOPIdxFunctor(indexRemovals);
  combinations.erase(std::remove_if(combinations.begin(), combinations.end(), fSOPIdxFunctor),
                     combinations.end());

  // Setup the vector and combination iterators
  IndexPairVector removeList;
  preparePruningRemovalList(trx, combinations, removeList);

  // Prepare an iterator to process the remove vector
  IndexPairVectorIterator rLIter = removeList.begin();
  IndexPairVectorIterator rLEndIter = removeList.end();

  // Process the remove vector with the remove_if bind2nd equal_to functor
  for (; rLIter != rLEndIter; ++rLIter)
  {
    combinations.erase(std::remove_if(combinations.begin(),
                                      combinations.end(),
                                      std::bind2nd(std::equal_to<IndexPair>(), *rLIter)),
                       combinations.end());
  }

  // uint32_t endSize = combinations.size();

  // Print the vector to see what has been removed
  if (IS_DEBUG_ENABLED(_logger))
  {
    logStopOverIndex(trx, combinations);
  }
}
//----------------------------------------------------------------------------
// Copies class of service data from src to dest
//----------------------------------------------------------------------------
void
ItinAnalyzerService::copyClassOfServiceData(ShoppingTrx& trx, const AirSeg& src, AirSeg& dest)
{
  TSELatencyData(trx, "ITIN COPYCOSDATA");
  if (UNLIKELY(src.segmentType() == Arunk))
  {
    return;
  }
  const std::vector<ClassOfService*>& cosV = src.classOfService();
  std::vector<ClassOfService*>::const_iterator cosIter = cosV.begin();
  std::vector<ClassOfService*>::const_iterator cosEIter = cosV.end();
  std::vector<ClassOfService*>& destCosV = dest.classOfService();
  uint32_t dCosVSz = destCosV.size();

  // Reserve the destination vector, taking into account if there are
  // already elements present
  destCosV.reserve((destCosV.empty() ? 0 : dCosVSz) + cosV.size());

  // Iterate through the src class of service data
  for (; cosIter != cosEIter; ++cosIter)
  {
    ClassOfService* newCos;
    trx.dataHandle().get(newCos); // lint !e530
    ClassOfService* curCos = *cosIter;
    newCos->bookingCode() = curCos->bookingCode();
    newCos->numSeats() = curCos->numSeats();
    newCos->cabin() = curCos->cabin();
    destCosV.push_back(newCos);
  }
}
//----------------------------------------------------------------------------
// Copies necessary data in air segment generation
//----------------------------------------------------------------------------
AirSeg*
ItinAnalyzerService::copyDefaultAirSegData(ShoppingTrx& trx, const AirSeg& orig, const AirSeg& dest)
{
  TSELatencyData metrics(trx, "ITIN COPYSEGDATA");
  AirSeg* seg;
  trx.dataHandle().get(seg); // lint !e530
  AirSeg& segRef = *seg;

  copyClassOfServiceData(trx, orig, segRef);

  copyClassOfServiceData(trx, dest, segRef);
  std::vector<ClassOfService*>& csvRef = segRef.classOfService();

  std::sort(csvRef.begin(), csvRef.end(), SortClassOfServiceVector());
  csvRef.erase(std::unique(csvRef.begin(), csvRef.end(), UniqueClassOfServiceVectorFilter()),
               csvRef.end());

  // TODO:: Optimize this copy operation
  // This would be much faster as a memcpy, need to determine
  // fields that are string objects or other dynamically

  // allocated structures -- they still have to be copied
  // on a per field basis -- however the others can be memcpy'd across
  // much more efficiently - I doubt the compiler would
  // optimize this much
  segRef.segmentType() = orig.segmentType();
  segRef.pssBookingDate() = orig.pssBookingDate();
  segRef.pssBookingTime() = orig.pssBookingTime();
  segRef.bookingDT() = orig.bookingDT();
  segRef.setBookingCode(orig.getBookingCode());
  segRef.bookedCabin() = orig.bookedCabin();
  segRef.resStatus() = orig.resStatus();
  segRef.fareBasisCode() = orig.fareBasisCode();
  segRef.fareCalcFareAmt() = orig.fareCalcFareAmt();
  segRef.forcedConx() = orig.forcedConx();
  segRef.forcedStopOver() = orig.forcedStopOver();
  segRef.eticket() = orig.eticket();
  segRef.considerOnlyCabin() = orig.considerOnlyCabin();
  segRef.forcedFareBrk() = orig.forcedFareBrk();
  segRef.forcedNoFareBrk() = orig.forcedNoFareBrk();
  segRef.geoTravelType() = orig.geoTravelType();
  segRef.stopOver() = orig.stopOver();
  segRef.equipmentType() = orig.equipmentType();
  segRef.furthestPoint() = orig.furthestPoint();
  segRef.retransited() = orig.retransited();
  segRef.origAirport() = orig.origAirport();
  segRef.departureDT() = orig.departureDT();
  segRef.pssDepartureDate() = orig.pssDepartureDate();
  segRef.pssDepartureTime() = orig.pssDepartureTime();
  segRef.origin() = orig.origin();
  segRef.destAirport() = dest.destAirport();
  segRef.arrivalDT() = dest.arrivalDT();
  segRef.pssArrivalDate() = dest.pssArrivalDate();
  segRef.pssArrivalTime() = dest.pssArrivalTime();
  segRef.destination() = dest.destination();
  segRef.boardMultiCity() = orig.boardMultiCity();
  segRef.offMultiCity() = dest.offMultiCity();
  segRef.carrier() = orig.carrier();
  segRef.flightNumber() = orig.flightNumber();
  segRef.validatedBookingCode() = orig.validatedBookingCode();
  segRef.setOperatingCarrierCode(orig.operatingCarrierCode());
  segRef.operatingFlightNumber() = orig.operatingFlightNumber();

  return (seg);
}

//----------------------------------------------------------------------------
// Generates an airsegment that encompasses the starting
// point of the orig AirSeg and the ending point of the
// dest AirSeg
//----------------------------------------------------------------------------
AirSeg*
ItinAnalyzerService::generateNewAirSegMin(ShoppingTrx& trx, const AirSeg& orig, const AirSeg& dest)
{
  AirSeg* seg;
  trx.dataHandle().get(seg); // lint !e530
  AirSeg& segRef = *seg;

  segRef.origAirport() = orig.origAirport();
  segRef.departureDT() = orig.departureDT();
  segRef.bookingDT() = orig.bookingDT();
  segRef.origin() = orig.origin();
  segRef.destAirport() = dest.destAirport();
  segRef.arrivalDT() = dest.arrivalDT();
  segRef.destination() = dest.destination();
  segRef.boardMultiCity() = orig.boardMultiCity();
  segRef.offMultiCity() = dest.offMultiCity();

  segRef.resStatus() = "OK";
  segRef.pssDepartureDate() = orig.departureDT().dateToSqlString();

  logSegmentData(segRef);

  return (seg);
}

//----------------------------------------------------------------------------
// Generates a new AirSeg* object based on the origin of the
// the first AirSeg (orig) and the destination of the second
// AirSeg (dest)
// You can also use this function to make a copy of the airsegment
// by passing the same airseg into orig and dest
//----------------------------------------------------------------------------
AirSeg*
ItinAnalyzerService::generateNewAirSeg(ShoppingTrx& trx, const AirSeg& orig, const AirSeg& dest)
{
  AirSeg* newSegment = copyDefaultAirSegData(trx, orig, dest);

  logSegmentData(*newSegment);

  return (newSegment);
}

//----------------------------------------------------------------------------
// Generates a arunk segment between the origin and destination
// This technically "spans" the gap between the origin segment and
// the destination segment
//----------------------------------------------------------------------------
AirSeg*
ItinAnalyzerService::generateNewAirSegArunkSpan(ShoppingTrx& trx,
                                                const AirSeg& orig,
                                                const AirSeg& dest)
{
  AirSeg* newSegment;
  trx.dataHandle().get(newSegment); // lint !e530
  AirSeg& segRef = *newSegment;

  // Origin of arunk segment is the end of the orig segment
  segRef.origAirport() = orig.destAirport();
  segRef.departureDT() = orig.arrivalDT();
  segRef.bookingDT() = trx.transactionStartTime();
  segRef.origin() = orig.destination();

  // Destination of the arunk segment is the start of the dest segment
  segRef.destAirport() = dest.origAirport();
  segRef.arrivalDT() = dest.departureDT();
  segRef.destination() = dest.origin();

  // City assignments follow the same pattern as the orig and dest
  // directly above in assigning the board point to the end of the orig
  // and the off point to the beginning of the dest
  segRef.boardMultiCity() = orig.offMultiCity();
  segRef.offMultiCity() = dest.boardMultiCity();

  // Set type to arunk
  segRef.segmentType() = Arunk;

  logSegmentData(segRef);

  return (newSegment);
}

//----------------------------------------------------------------------------
// Generates a new AirSeg* object based on the origin of the
// the first AirSeg (orig) and the destination of the second
// AirSeg (dest)
// You can also use this function to make a copy of the airsegment
// by passing the same airseg into orig and dest
//----------------------------------------------------------------------------
AirSeg*
ItinAnalyzerService::generateNewAirSeg(ShoppingTrx& trx,
                                       const AirSeg& orig,
                                       const AirSeg& dest,
                                       const CarrierCode& cxr,
                                       const FlightNumber& flightNum,
                                       const CarrierCode& opCxr,
                                       const FlightNumber& opFlightNum)
{
  AirSeg* newSegment = copyDefaultAirSegData(trx, orig, dest);
  AirSeg& newSeg = *newSegment;

  newSeg.carrier() = cxr;
  newSeg.flightNumber() = flightNum;
  newSeg.setOperatingCarrierCode(opCxr);
  newSeg.operatingFlightNumber() = opFlightNum;

  logSegmentData(newSeg);

  return (newSegment);
}

//----------------------------------------------------------------------------
// Generates a new Itin object based on the AirSeg object
//----------------------------------------------------------------------------
Itin*
ItinAnalyzerService::generateNewItinInLeg(ShoppingTrx& trx, ShoppingTrx::Leg& leg, AirSeg*& airSeg)
{
  Itin* newItin;
  trx.dataHandle().get(newItin); // lint !e530

  // Setup itin data and its relation with a leg, an air segment,
  // and a shopping transaction
  newItin->travelSeg().push_back(airSeg);
  return (newItin);
}

//----------------------------------------------------------------------------
// Generates a new Itin object based on the vector of AirSeg objects
//----------------------------------------------------------------------------
Itin*
ItinAnalyzerService::generateNewItinInLeg(ShoppingTrx& trx,
                                          ShoppingTrx::Leg& leg,
                                          std::vector<AirSeg*>& airSegs)
{
  Itin* newItin;
  trx.dataHandle().get(newItin); // lint !e530
  std::vector<AirSeg*>::iterator aSIter = airSegs.begin();
  std::vector<AirSeg*>::iterator aSEndIter = airSegs.end();
  newItin->travelSeg().reserve(airSegs.size());

  for (; aSIter != aSEndIter; ++aSIter)
  {
    AirSeg*& curAS = static_cast<AirSeg*&>(*aSIter);
    newItin->travelSeg().push_back(curAS);
  }

  // Setup itin data
  leg.addSop(ShoppingTrx::SchedulingOption(newItin, leg.sop().size() + 1));
  ShoppingTrx::SchedulingOption& newSop = leg.sop().back();
  newSop.cabinClassValid() = false;

  return (newItin);
}

//----------------------------------------------------------------------------
// Generates a new itinerary based on an air segment
//----------------------------------------------------------------------------
Itin*
ItinAnalyzerService::generateNewItin(ShoppingTrx& trx,
                                     std::vector<AirSeg*>& airSegs,
                                     FareMarket* newFM)
{
  Itin* newItin;
  trx.dataHandle().get(newItin); // lint !e530

  std::vector<AirSeg*>::iterator aSIter = airSegs.begin();
  std::vector<AirSeg*>::iterator aSEndIter = airSegs.end();
  newItin->fareMarket().push_back(newFM);
  newItin->travelSeg().reserve(airSegs.size());
  newFM->travelSeg().reserve(airSegs.size());
  for (; aSIter != aSEndIter; ++aSIter)
  {
    AirSeg*& curAirSeg = static_cast<AirSeg*&>(*aSIter);
    newItin->travelSeg().push_back(curAirSeg);
    newFM->travelSeg().push_back(curAirSeg);
  }

  return (newItin);
}

//----------------------------------------------------------------------------
// Generates the stop over legs and adds them to the transaction
//----------------------------------------------------------------------------
void
ItinAnalyzerService::generateStopOverLegsFromCombinationPairs(ShoppingTrx& trx,
                                                              const IndexPairVector& combinations)

{
  TSELatencyData metrics(trx, "ITIN GEN-ASOLEGS");

  if (combinations.empty())
  {
    return;
  }

  // Prepare the iterators and vector references for leg generation
  IndexPairVector::const_iterator comIter = combinations.begin();
  IndexPairVector::const_iterator comEndIter = combinations.end();

  // Add new legs
  for (; comIter != comEndIter; ++comIter)
  {
    const IndexPair& curPair = *comIter;
    std::vector<ShoppingTrx::Leg>& legs = trx.legs();

    // Check for surface sector cases
    ShoppingTrx::Leg& legOne = legs[curPair.first];
    ShoppingTrx::Leg& legTwo = legs[curPair.second];
    ShoppingTrx::Leg::SurfaceSectorLegType& legOneSSType = legOne.surfaceSectorLegType();
    ShoppingTrx::Leg::SurfaceSectorLegType& legTwoSSType = legTwo.surfaceSectorLegType();

    if (legOneSSType == ShoppingTrx::Leg::SurfaceSectorLegNone &&
        legTwoSSType == ShoppingTrx::Leg::SurfaceSectorLegNone)
    {
      generateAcrossStopOverLeg(trx, curPair);
    }
    else
    {
      generateAcrossStopOverLegSurface(trx, curPair);
    }
  }
}
//----------------------------------------------------------------------------
// Normal across stop over leg generation function(overloaded)
//----------------------------------------------------------------------------
void
ItinAnalyzerService::generateAcrossStopOverLeg(ShoppingTrx& trx, const IndexPair& curPair)
{
  TSELatencyData metrics(trx, "ITIN GEN-NORM ASOLEG");
  std::vector<ShoppingTrx::Leg>& legs = trx.legs();

  // Create the new leg
  legs.push_back(ShoppingTrx::Leg());
  ShoppingTrx::Leg& newLeg = legs.back();
  IndexVector& jumpedLegIdx = newLeg.jumpedLegIndices();
  jumpedLegIdx.reserve(curPair.second - curPair.first + 1);

  // Create jumped leg indices
  bool isCus = false;
  for (uint32_t i = curPair.first; i <= curPair.second; i++)
  {
    jumpedLegIdx.push_back(i);
    isCus = (isCus || legs[i].isCustomLeg());
  }

  // Create new air segs based on the legs we are crossing
  std::vector<AirSeg*> aSONewAirSegs;
  IndexVectorIterator jLIter = jumpedLegIdx.begin();
  IndexVectorIterator jLEIter = jumpedLegIdx.end();

  for (; jLIter != jLEIter; ++jLIter)
  {
    ShoppingTrx::Leg& curLeg = legs[*jLIter];
    Itin*& curLegItin = curLeg.sop().front().itin();
    const AirSeg& frontSeg = dynamic_cast<const AirSeg&>(*(curLegItin->travelSeg().front()));
    const AirSeg& backSeg = dynamic_cast<const AirSeg&>(*(curLegItin->travelSeg().back()));

    // We must use custom field data to generate proper air segments
    AirSeg* newSeg = generateNewAirSeg(trx,
                                       frontSeg,
                                       backSeg,
                                       frontSeg.carrier(),
                                       frontSeg.flightNumber(),
                                       frontSeg.operatingCarrierCode(),
                                       frontSeg.operatingFlightNumber());
    aSONewAirSegs.push_back(newSeg);
  }

  // Create the new itin that represents the flight for the
  // stop-over leg
  generateNewItinInLeg(trx, newLeg, aSONewAirSegs);

  // Set the new leg data
  newLeg.preferredCabinClass() =
      legs[curPair.first].preferredCabinClass() > legs[curPair.second].preferredCabinClass()
          ? legs[curPair.first].preferredCabinClass()
          : legs[curPair.second].preferredCabinClass();

  newLeg.stopOverLegFlag() = true;
  newLeg.stopOverLegCombination() = curPair;
  newLeg.setCustomLeg(isCus);
} // lint !e529
//----------------------------------------------------------------------------
// Extended surface sector across stopover leg generation function(overloaded)
// TODO - Refactor method into smaller pieces - GJL
//----------------------------------------------------------------------------
void
ItinAnalyzerService::generateAcrossStopOverLegSurface(ShoppingTrx& trx, const IndexPair& curPair)
{
  TSELatencyData metrics(trx, "ITIN GEN-SURF ASOLEG");

  std::vector<ShoppingTrx::Leg>& legs = trx.legs();

  // Create the new leg
  legs.push_back(ShoppingTrx::Leg());
  ShoppingTrx::Leg& newLeg = legs.back();
  IndexVector& jumpedLegIndices = newLeg.jumpedLegIndices();
  // Multiply by two in case there are surface sectors between every leg
  jumpedLegIndices.reserve((curPair.second - curPair.first + 1) << 1);

  // Create jumped leg indices
  for (uint32_t i = curPair.first; i <= curPair.second; i++)
  {
    jumpedLegIndices.push_back(i);
    ShoppingTrx::Leg::SurfaceSectorLegType& ssLegType = legs[i].surfaceSectorLegType();
    if (ssLegType == ShoppingTrx::Leg::SurfaceSectorLegNone)
    {
      continue;
    }

    IndexPair iPair;

    if (ssLegType != ShoppingTrx::Leg::SurfaceSectorEndAtLegOrig)
    {
      iPair.first = i;
      iPair.second = ASOLEG_SURFACE_SECTOR_ID;
      newLeg.surfaceSectorMap()[i] = iPair;
    }
  }

  // Create new air segs based on the legs we are crossing
  std::vector<AirSeg*> aSONewAirSegs;
  aSONewAirSegs.clear();
  // uint32_t jLIdx = 0;
  uint32_t jLSize = jumpedLegIndices.size();

  // Create the itin for this particular pair using the proper
  // eSPair flags
  for (uint32_t i = 0; i < jLSize; i++)
  {
    // const AirSeg* prevBackSeg = NULL;
    // ShoppingTrx::Leg* prevLeg = NULL;
    // Itin* prevLegItin = NULL;
    AirSeg* newSeg;
    uint32_t& curJumpedLegIndex = jumpedLegIndices[i];
    ShoppingTrx::Leg& curLeg = legs[curJumpedLegIndex];
    Itin*& curLegItin = curLeg.sop().front().itin();
    const AirSeg& curFrontSeg = dynamic_cast<const AirSeg&>(*(curLegItin->travelSeg().front()));
    const AirSeg& curBackSeg = dynamic_cast<const AirSeg&>(*(curLegItin->travelSeg().back()));

    // We must use custom field data to generate proper air segments
    newSeg = generateNewAirSeg(trx,
                               curFrontSeg,
                               curBackSeg,
                               curFrontSeg.carrier(),
                               curFrontSeg.flightNumber(),
                               curFrontSeg.operatingCarrierCode(),
                               curFrontSeg.operatingFlightNumber());
    aSONewAirSegs.push_back(newSeg);
  }

  // Create the new itin that represents the flight for the
  // stop-over leg
  // Itin* newItin =
  generateNewItinInLeg(trx, newLeg, aSONewAirSegs);

  // Set the new leg data
  newLeg.preferredCabinClass() = legs[curPair.first].preferredCabinClass();
  newLeg.stopOverLegFlag() = true;
  newLeg.stopOverLegCombination() = curPair;
}

//----------------------------------------------------------------------------
// Primary governing carrier estimation logic function
//----------------------------------------------------------------------------
// TODO: Clean this function up and split it
void
ItinAnalyzerService::estimateGovCxrForStopOverLegs(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "ITIN ESTGOVCXR ASOLEGS");
  // Get the legs needed for analysis -- only across stop over legs
  std::vector<ShoppingTrx::Leg*> aCSOLegs;
  std::vector<ShoppingTrx::Leg>::iterator legIter = trx.legs().begin();
  std::vector<ShoppingTrx::Leg>::iterator legEIter = trx.legs().end();

  for (; legIter != legEIter; ++legIter)
  {
    ShoppingTrx::Leg& curLeg = static_cast<ShoppingTrx::Leg&>(*legIter);
    if (!curLeg.stopOverLegFlag())
    {
      continue;
    }

    aCSOLegs.push_back(&curLeg);
  }

  if (aCSOLegs.empty())
  {
    return;
  }

  std::vector<ShoppingTrx::Leg*>::iterator aCSOLegIter = aCSOLegs.begin();
  std::vector<ShoppingTrx::Leg*>::iterator aCSOLegEIter = aCSOLegs.end();

  for (; aCSOLegIter != aCSOLegEIter; ++aCSOLegIter)
  {
    ShoppingTrx::Leg*& curAL = static_cast<ShoppingTrx::Leg*&>(*aCSOLegIter);

    // Get the itin from this stop over leg
    Itin* curALItin = curAL->sop().front().itin();

    // Get the faremarket from this itin
    FareMarket* curALFM = curALItin->fareMarket().front();

    std::map<uint32_t, FareMarket*> tBoundaryMap;
    genTravelBoundMapForGovCxrEst(trx, curAL->jumpedLegIndices(), tBoundaryMap);

    if (tBoundaryMap.empty() || tBoundaryMap.size() == 0)
    {
      LOG4CXX_WARN(_logger, "- Travel boundary map is empty");
      continue;
    }

    IndexVector tBoundCounts;
    genTravelBoundSumsForGovCxrEst(trx, tBoundaryMap, tBoundCounts);

    if (tBoundCounts.empty() || tBoundCounts.size() == 0)
    {
      continue;
    }

    IndexPair maxCntIdxPair;
    IndexVector repeatedCounts;
    genTravelBoundMaxCountsForGovCxrEst(trx, tBoundCounts, maxCntIdxPair, repeatedCounts);

    uint32_t idxChosen = maxCntIdxPair.second;

    if ((!(repeatedCounts.empty())) && (repeatedCounts.size() > 0))
    {
      // Drill further down than just travel boundary bit sets
      // Examine the following: (ordered from most significant to least)
      // - detailed sub-IATA check
      // - national border check
      estimateGovCxrDetailedCheck(trx, tBoundaryMap, maxCntIdxPair, repeatedCounts, idxChosen);
    }

    // Select the leg with the most international travel
    uint32_t adoptedLegId = curAL->jumpedLegIndices()[idxChosen];
    ShoppingTrx::Leg& chosenLeg = trx.legs()[adoptedLegId];
    ItinIndex& chosenLegIdx = chosenLeg.carrierIndex();
    curAL->adoptedCrossedLegRefIndex() = adoptedLegId;

    // Adopt governing carriers from the chosen leg in combination
    // with copies of the itin currently stored in the carrier index
    // in the across stop over leg
    ItinIndex::ItinMatrixIterator iMIter = chosenLegIdx.root().begin();
    ItinIndex::ItinMatrixIterator iMEIter = chosenLegIdx.root().end();
    curAL->sop().clear();
    curAL->carrierIndex().root().clear();

    // TODO: Split this section into another method
    // Go through the itin index and pick out the governing carriers
    for (; iMIter != iMEIter; ++iMIter)
    {
      ItinIndex::ItinMatrixPair iMPair = *iMIter;
      ItinIndex::ItinCell* curCell =
          chosenLegIdx.retrieveTopItinCell(iMPair.first, ItinIndex::CHECK_FAKEDIRECTFLIGHT);

      if (!curCell)
      {
        continue;
      }

      // Create the fare market for this new across stop over leg governing carrier
      FareMarket* newFM;
      trx.dataHandle().get(newFM); // lint !e530
      CarrierCode& govCxr = curCell->second->fareMarket().front()->governingCarrier();
      cloneFareMarket(*curALFM, *newFM, govCxr);
      newFM->setCombineSameCxr(curCell->first.combineSameCxr());
      newFM->travelSeg().clear();

      Itin* newItin;
      std::vector<AirSeg*> newAirSegs;
      newAirSegs.clear();

      std::vector<TravelSeg*>::const_iterator itinIter = curALItin->travelSeg().begin();
      std::vector<TravelSeg*>::const_iterator itinEIter = curALItin->travelSeg().end();

      int tSegIdx = 0;
      IndexVector& jumpedLegIndices = curAL->jumpedLegIndices();
      for (; itinIter != itinEIter; ++itinIter)
      {
        AirSeg* newAirSeg;
        const AirSeg& curAirSeg = dynamic_cast<const AirSeg&>(**itinIter);
        uint32_t& jLegIdx = jumpedLegIndices[tSegIdx];

        if (jLegIdx != adoptedLegId)
        {
          newAirSeg = generateNewAirSeg(trx, curAirSeg, curAirSeg);
        }
        // Otherwise, generate a similar air seg with several
        // different field values
        else
        {
          newAirSeg = generateNewAirSeg(trx,
                                        curAirSeg,
                                        curAirSeg,
                                        govCxr,
                                        curAirSeg.flightNumber(),
                                        curAirSeg.operatingCarrierCode(),
                                        curAirSeg.operatingFlightNumber());
        }

        newAirSegs.push_back(newAirSeg);
        ++tSegIdx;
      }

      newItin = generateNewItin(trx, newAirSegs, newFM);
      ItinIndex::ItinCellInfo newItinCellInfo;
      newItinCellInfo.globalDirection() = newFM->getGlobalDirection();
      newItinCellInfo.combineSameCxr() = newFM->combineSameCxr();

      // Push the sop onto the sop vector and add the main itin to the
      // carrier index of the across stop over leg
      ItinIndex::Key cxrKey;
      ItinIndex::Key columnKey;
      cxrKey = iMPair.first;
      ShoppingUtil::createKey(ITININDEX_DEFAULT, columnKey);
      curAL->addSop(ShoppingTrx::SchedulingOption(newItin, curAL->sop().size() + 1));
      curAL->carrierIndex().addItinCell(newItin, newItinCellInfo, cxrKey, columnKey);
      ShoppingTrx::SchedulingOption& newSop = curAL->sop().back();
      newSop.cabinClassValid() = false;
    }
  }
}

void
ItinAnalyzerService::genTravelBoundMapForGovCxrEst(
    ShoppingTrx& trx,
    const IndexVector& jumpedLegIndices,
    std::map<uint32_t, FareMarket*>& travelBoundaryMap)
{
  TSELatencyData metrics(trx, "ITIN GENTVLBOUNDMAP ESTGOVCXR");
  // uint32_t numLegsJumped = jumpedLegIndices.size();
  IndexVector::const_iterator cJLIter = jumpedLegIndices.begin();
  IndexVector::const_iterator cJLEIter = jumpedLegIndices.end();

  // Clear the map
  travelBoundaryMap.clear();

  // For each leg we are jumping across, retrieve the travel
  // boundary
  for (uint32_t cnt = 0; cJLIter != cJLEIter; ++cJLIter, ++cnt)
  {
    const uint32_t& jLegIdx = *cJLIter;
    if (jLegIdx == ASOLEG_SURFACE_SECTOR_ID)
    {
      continue;
    }

    ShoppingTrx::Leg& curLeg = trx.legs()[jLegIdx];
    ItinIndex& cxrIdx = curLeg.carrierIndex();

    if (cxrIdx.root().empty() || cxrIdx.root().size() == 0)
    {
      continue;
    }

    ItinIndex::ItinMatrixIterator firstMatIter = cxrIdx.root().begin();
    if (firstMatIter == cxrIdx.root().end())
    {
      continue;
    }

    ItinIndex::ItinRowIterator firstRowIter = firstMatIter->second.begin();
    if (firstRowIter == firstMatIter->second.end())
    {
      continue;
    }

    ItinIndex::ItinColumn& firstColumn = static_cast<ItinIndex::ItinColumn&>(firstRowIter->second);
    if (firstColumn.empty() || firstColumn.size() == 0)
    {
      continue;
    }

    ItinIndex::ItinCell& firstCell = static_cast<ItinIndex::ItinCell&>(firstColumn.front());
    Itin*& firstItin = static_cast<Itin*&>(firstCell.second);
    if (firstItin == nullptr)
    {
      continue;
    }

    FareMarket*& firstFM = static_cast<FareMarket*&>(firstItin->fareMarket().front());

    // Set map entry
    travelBoundaryMap[cnt] = firstFM;
  }
}

void
ItinAnalyzerService::genTravelBoundSumsForGovCxrEst(
    ShoppingTrx& trx,
    const std::map<uint32_t, FareMarket*>& travelBoundaryMap,
    IndexVector& travelBoundSummations)
{
  TSELatencyData metrics(trx, "ITIN GETTVLBOUNDSUMS ESTGOVCXR");
  std::map<uint32_t, FareMarket*>::const_iterator tBIter = travelBoundaryMap.begin();
  std::map<uint32_t, FareMarket*>::const_iterator tBEIter = travelBoundaryMap.end();
  travelBoundSummations.clear();
  travelBoundSummations.reserve(travelBoundaryMap.size());

  // By utilizing the travel boundary bit set, a count can be
  // computed from each one of the sets with a cost factor.
  // The set with the highest sum is going to determine which
  // leg we adopt for the across stop over leg.
  for (; tBIter != tBEIter; ++tBIter)
  {
    const FareMarket* bSRef = tBIter->second;
    uint32_t bSSum = 0;

    FMTravelBoundary tBoundType = FMTravelBoundary::TravelWithinUSCA;
    bSSum +=
        static_cast<uint32_t>((bSRef->travelBoundary().isSet(tBoundType)) ? INTL_TRAVCOST_USCA : 0);

    tBoundType = FMTravelBoundary::TravelWithinSameCountryExceptUSCA;
    bSSum += static_cast<uint32_t>(
        (bSRef->travelBoundary().isSet(tBoundType)) ? INTL_TRAVCOST_NONUSCA : 0);

    tBoundType = FMTravelBoundary::TravelWithinOneIATA;
    bSSum += static_cast<uint32_t>(
        (bSRef->travelBoundary().isSet(tBoundType)) ? INTL_TRAVCOST_ONEIATA : 0);

    tBoundType = FMTravelBoundary::TravelWithinTwoIATA;
    bSSum += static_cast<uint32_t>(
        (bSRef->travelBoundary().isSet(tBoundType)) ? INTL_TRAVCOST_TWOIATA : 0);

    tBoundType = FMTravelBoundary::TravelWithinAllIATA;
    bSSum += static_cast<uint32_t>(
        (bSRef->travelBoundary().isSet(tBoundType)) ? INTL_TRAVCOST_ALLIATA : 0);

    tBoundType = FMTravelBoundary::TravelWithinSubIATA11;
    bSSum += static_cast<uint32_t>(
        (bSRef->travelBoundary().isSet(tBoundType)) ? INTL_TRAVCOST_SUBIATA11 : 0);

    tBoundType = FMTravelBoundary::TravelWithinSubIATA21;
    bSSum += static_cast<uint32_t>(
        (bSRef->travelBoundary().isSet(tBoundType)) ? INTL_TRAVCOST_SUBIATA21 : 0);

    tBoundType = FMTravelBoundary::TravelWithinSameSubIATAExcept21And11;
    bSSum += static_cast<uint32_t>(
        (bSRef->travelBoundary().isSet(tBoundType)) ? INTL_TRAVCOST_SUBIATA : 0);

    travelBoundSummations.push_back(bSSum);
  }
}

void
ItinAnalyzerService::genTravelBoundMaxCountsForGovCxrEst(
    ShoppingTrx& trx,
    const std::vector<uint32_t>& travelBoundSummations,
    IndexPair& maxCountIndex,
    IndexVector& repeatedMaxCountIndices)
{
  TSELatencyData metrics(trx, "ITIN GETTVLBOUNDCNTS ESTGOVCXR");
  // Select the travel boundary that exhibits the most
  // international service.  These travel boundaries
  // map directly to the legs that we are crossing over.
  // If a tie occurs across all, the first leg is always chosen.
  // Utilizing the numeric nature of the travel boundary bit set
  // to determine which carrier provides the most international
  // service.

  IndexVector::const_iterator tBCIter = travelBoundSummations.begin();
  IndexVector::const_iterator tBCEIter = travelBoundSummations.end();

  // Compute the max element
  IndexVector::const_iterator maxElem = std::max_element(tBCIter, tBCEIter);

  // Get the values
  maxCountIndex.first = *maxElem;
  maxCountIndex.second = (maxElem - travelBoundSummations.begin());

  // Next we must see if there is a tie on the max value
  tBCIter = travelBoundSummations.begin();
  tBCEIter = travelBoundSummations.end();
  repeatedMaxCountIndices.clear();
  repeatedMaxCountIndices.reserve(travelBoundSummations.size());

  if (std::count(tBCIter, tBCEIter, maxCountIndex.first) > 1)
  {
    for (uint32_t idx = 0; tBCIter != tBCEIter; ++tBCIter, ++idx)
    {
      const uint32_t& curCnt = *tBCIter;
      if (curCnt == maxCountIndex.first)
      {
        repeatedMaxCountIndices.push_back(idx);
      }
    }
  }
}

void
ItinAnalyzerService::estimateGovCxrDetailedCheck(
    ShoppingTrx& trx,
    const std::map<uint32_t, FareMarket*>& travelBoundaryMap,
    const IndexPair& maxCountIndex,
    const IndexVector& repeatedMaxCountIndices,
    uint32_t& govCxrIdxChosen)
{
  TSELatencyData metrics(trx, "ITIN ESTGOVCXR DETAILCHECK");
  if (repeatedMaxCountIndices.empty() || repeatedMaxCountIndices.size() <= 1)
  {
    govCxrIdxChosen = maxCountIndex.second;
    return;
  }

  std::vector<const FareMarket*> maxCountFareMarkets;
  IndexVector::const_iterator rMCIter = repeatedMaxCountIndices.begin();
  IndexVector::const_iterator rMCEndIter = repeatedMaxCountIndices.end();

  // Generate a vector of fare markets referred to by the max
  // count repeated indices
  for (; rMCIter != rMCEndIter; ++rMCIter)

  {
    const uint32_t& curIdx = static_cast<const uint32_t&>(*rMCIter);
    FareMarket* curIdxFM = (travelBoundaryMap.find(curIdx))->second;
    maxCountFareMarkets.push_back(curIdxFM);
  }

  // Iterate over the fare markets and perform a scoring of iata changes, sub iata
  // changes and national changes
  std::vector<const FareMarket*>::iterator mCFMIter = maxCountFareMarkets.begin();
  std::vector<const FareMarket*>::iterator mCFMEndIter = maxCountFareMarkets.end();
  IndexVector detailedFMCosts;
  uint32_t maxFMCost = 0;
  uint32_t maxFMCostIdx = 9999;
  bool rptMaxVals = false;

  detailedFMCosts.reserve(maxCountFareMarkets.size());
  for (uint32_t idx = 0; mCFMIter != mCFMEndIter; ++mCFMIter, ++idx)
  {
    uint32_t fMCost = 0;
    const FareMarket* curFM = static_cast<const FareMarket*>(*mCFMIter);
    const Loc* curFMOrigin = curFM->origin();
    const Loc* curFMDestination = curFM->destination();

    // Check iata changes first
    const IATAAreaCode& originIATA = curFMOrigin->area();
    const IATAAreaCode& destinationIATA = curFMDestination->area();

    if (originIATA != destinationIATA)
    {
      fMCost += INTL_DETAILTRAVCOST_IATA;
    }

    // Check sub iata changes second
    const IATASubAreaCode& originSubIATA = curFMOrigin->subarea();
    const IATASubAreaCode& destinationSubIATA = curFMDestination->subarea();

    if (originSubIATA != destinationSubIATA)
    {
      fMCost += INTL_DETAILTRAVCOST_SUBIATA;
    }

    // Check nation changes last
    const NationCode& originNation = curFMOrigin->nation();
    const NationCode& destinationNation = curFMDestination->nation();

    if (originNation != destinationNation)
    {
      fMCost += INTL_DETAILTRAVCOST_NATION;
    }

    if (IS_DEBUG_ENABLED(_logger))
    {
      LOG4CXX_DEBUG(_logger, "************************************");
      LOG4CXX_DEBUG(_logger,
                    "[" << idx << "] examining fare market from " << curFMOrigin << " -> "
                        << curFMDestination);
      LOG4CXX_DEBUG(_logger, "************************************");
      LOG4CXX_DEBUG(_logger, "*- IATA    " << originIATA << " -> " << destinationIATA);
      LOG4CXX_DEBUG(_logger, "*- SubIATA " << originSubIATA << " -> " << destinationSubIATA);
      LOG4CXX_DEBUG(_logger, "*- Nation  " << originNation << " -> " << destinationNation);
      LOG4CXX_DEBUG(_logger, "*-- Cost = " << fMCost);
    }

    if (fMCost > maxFMCost)
    {
      maxFMCost = fMCost;
      maxFMCostIdx = idx;
      rptMaxVals = false;
    }
    else if (fMCost == maxFMCost)
    {
      rptMaxVals = true;
    }

    detailedFMCosts.push_back(fMCost);
  }

  govCxrIdxChosen =
      ((rptMaxVals) ? repeatedMaxCountIndices[0] : repeatedMaxCountIndices[maxFMCostIdx]);
}

void
ItinAnalyzerService::addAcrossStopOverLegArunkSpans(ShoppingTrx& trx)
{
  TSELatencyData(trx, "ITIN ADDASO ARUNKSPANS");
  std::vector<ShoppingTrx::Leg>& legs = trx.legs();
  std::vector<ShoppingTrx::Leg>::iterator lBegin = legs.begin();
  std::vector<ShoppingTrx::Leg>::iterator lEnd = legs.end();
  for (uint32_t legId = 0; lBegin != lEnd; ++lBegin, legId++)
  {
    ShoppingTrx::Leg& curLeg = *lBegin;
    if (!curLeg.stopOverLegFlag())
    {
      continue;
    }

    std::map<uint32_t, IndexPair>& ssMap = curLeg.surfaceSectorMap();

    if (ssMap.empty())
    {
      continue;
    }

    IndexVector& jumpedLegIndices = curLeg.jumpedLegIndices();
    std::map<uint32_t, IndexPair>::iterator ssMapIter = ssMap.begin();
    std::map<uint32_t, IndexPair>::iterator ssMapEIter = ssMap.end();
    for (; ssMapIter != ssMapEIter; ++ssMapIter)
    {
      // Get the current mapped leg key index
      const std::pair<uint32_t, IndexPair>& curMapEntry = *ssMapIter; // lint !e1561
      const uint32_t& key = curMapEntry.first;
      const IndexPair& val = curMapEntry.second;

      // Find the jumped leg index that matches the mapped leg key index
      IndexVectorIterator iVIter = jumpedLegIndices.begin();
      IndexVectorIterator iVEIter = jumpedLegIndices.end();
      // bool pushedFirst = false;

      for (; iVIter != iVEIter; ++iVIter)
      {
        uint32_t& curI = *iVIter;
        if (curI == key)
        {
          // If the jumped leg index comes first, insert the surface
          // sector leg indicator after the jumped leg index
          if (val.first == key)
          {
            // don't add arunks at the end of an ASO
            if (iVIter + 1 != iVEIter)
            {
              jumpedLegIndices.insert(iVIter + 1, ASOLEG_SURFACE_SECTOR_ID);
            }
          }
          // Otherwise, the surface sector leg indicator comes before the
          // jumped leg index
          else
          {
            // don't add arunks at the beginning of an ASO
            if (iVIter != jumpedLegIndices.begin())
            {
              jumpedLegIndices.insert(iVIter, ASOLEG_SURFACE_SECTOR_ID);
            }
          }
          break;
        }
      }
    }

    if (IS_DEBUG_ENABLED(_logger))
    {
      std::ostringstream oStr;
      std::copy(jumpedLegIndices.begin(),
                jumpedLegIndices.end(),
                std::ostream_iterator<uint32_t>(oStr, " "));
      LOG4CXX_DEBUG(_logger, "Jumped leg indices = " << oStr.str());
    }

    // Build leg direct itins
    std::vector<Itin*> dItins;
    buildLegDirectItins(trx, legId, dItins);

    // Update the itinerary to reflect the new jumped leg index data
    IndexVectorIterator iVIter = jumpedLegIndices.begin();
    IndexVectorIterator iVEIter = jumpedLegIndices.end();
    // uint32_t idx = 0;
    uint32_t offset = 0;

    for (uint32_t j = 0; iVIter != iVEIter; ++iVIter, j++)
    {
      uint32_t& curIV = *iVIter;

      if (curIV != ASOLEG_SURFACE_SECTOR_ID)
      {
        // idx++;
        continue;
      }

      // Compute real indices (before and after)
      uint32_t realIndexBefore = j - 1 - offset;
      uint32_t realIndexAfter = j - offset;

      if (IS_DEBUG_ENABLED(_logger))
      {
        LOG4CXX_DEBUG(_logger, "Index           = " << j);
        LOG4CXX_DEBUG(_logger, "RealIndexBefore = " << realIndexBefore);
        LOG4CXX_DEBUG(_logger, "RealIndexAfter  = " << realIndexAfter);
      }

      // Acquire before and after segs from the itinerary
      Itin& firstDItin = *(dItins[0]);
      std::vector<TravelSeg*>& iSegs = firstDItin.travelSeg();

      TSE_ASSERT(realIndexBefore < iSegs.size()); // lint !e574
      TSE_ASSERT(realIndexAfter < iSegs.size()); // lint !e574
      const AirSeg& befSeg = dynamic_cast<const AirSeg&>(*(iSegs[realIndexBefore]));
      const AirSeg& aftSeg = dynamic_cast<const AirSeg&>(*(iSegs[realIndexAfter]));

      if (IS_DEBUG_ENABLED(_logger))
      {
        LOG4CXX_DEBUG(_logger,
                      "Before Seg = " << befSeg.origin()->loc() << "-"
                                      << befSeg.destination()->loc());
        LOG4CXX_DEBUG(_logger,
                      "After Seg  = " << aftSeg.origin()->loc() << "-"
                                      << aftSeg.destination()->loc());
      }

      // Create surface sector span segment
      AirSeg* surfSpanSegment = generateNewAirSegArunkSpan(trx, befSeg, aftSeg);
      std::vector<Itin*>::iterator dIter = dItins.begin();
      std::vector<Itin*>::iterator dEIter = dItins.end();

      for (; dIter != dEIter; ++dIter)
      {
        Itin& curItin = **dIter;
        std::vector<TravelSeg*>& curItinSegs = curItin.travelSeg();
        std::vector<TravelSeg*>& curFMSegs = curItin.fareMarket().front()->travelSeg();

        curItinSegs.insert(curItinSegs.begin() + realIndexAfter, surfSpanSegment);
        curFMSegs.insert(curFMSegs.begin() + realIndexAfter, surfSpanSegment);
      }

      offset++;
    }
  }
}
//----------------------------------------------------------------------------
// Generate across stop over carrier maps
//----------------------------------------------------------------------------
void
ItinAnalyzerService::generateAcrossStopOverLegCxrMaps(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "ITIN GEN-ASOLEG-CXRMAPS");
  std::vector<ShoppingTrx::Leg>& legs = trx.legs();
  std::vector<ShoppingTrx::Leg>::iterator lBegin = legs.begin();
  std::vector<ShoppingTrx::Leg>::iterator lEnd = legs.end();
  for (; lBegin != lEnd; ++lBegin)
  {
    ShoppingTrx::Leg& curLeg = *lBegin;
    if (!curLeg.stopOverLegFlag())
    {
      continue;
    }

    curLeg.generateAcrossStopOverCombinations(trx);
  }
}

void
ItinAnalyzerService::setInboundOutbound(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "ITIN SHOP SET INBND/OUTBND");
  std::vector<ShoppingTrx::Leg>::iterator legIter = trx.legs().begin();
  std::vector<ShoppingTrx::Leg>::iterator legEndIter = trx.legs().end();

  // Iterate through the legs
  std::vector<Itin*> directItinV;
  Itin& journeyItin = *(trx.journeyItin());
  std::vector<FareMarket*> journeyFMVector = journeyItin.fareMarket();
  std::vector<FareMarket*>::iterator jFMIter = journeyFMVector.begin();
  std::vector<FareMarket*>::iterator jFMEIter = journeyFMVector.end();

  for (; jFMIter != jFMEIter; ++jFMIter)
  {
    FareMarket& curJFM = *(*jFMIter);

    // Set the inbound/outbound indicators utilizing the journey itin
    // to drive the pricing logic determination
    setInboundOutbound(trx, journeyItin, curJFM);
  }

  for (uint32_t legId = 0; legIter != legEndIter; ++legIter, ++legId)
  {
    ShoppingTrx::Leg& curLeg = static_cast<ShoppingTrx::Leg&>(*legIter);
    directItinV.clear();
    buildLegDirectItins(trx, legId, directItinV);

    if (directItinV.empty())
    {
      continue;
    }

    Itin& firstItin = *(directItinV.front());
    FareMarket& firstFM = *(firstItin.fareMarket().front());

    // Set the leg direction
    FareMarket* jFMarket = nullptr;

    // For non-across stop over legs, use the direction
    // determined within the pricing logic calls with
    // the journey itin
    if (!curLeg.stopOverLegFlag())
    {
      jFMarket = journeyItin.fareMarket()[legId];
      firstFM.direction() = jFMarket->direction();
    }
    else
    {
      // If we are at the front of the stop over leg, take
      // the first fare market direction from the journey itin

      if (curLeg.jumpedLegIndices().front() == 0)
      {
        jFMarket = journeyItin.fareMarket()[0];
        firstFM.direction() = jFMarket->direction();
      }
      else
      {
        bool foundOutbound = false;
        bool foundInbound = false;
        bool foundUnknown = false;
        IndexVectorIterator iVIter = curLeg.jumpedLegIndices().begin();
        IndexVectorIterator iVEIter = curLeg.jumpedLegIndices().end();
        for (; iVIter != iVEIter; ++iVIter)
        {
          // skip across stopover legs
          if (*iVIter == ASOLEG_SURFACE_SECTOR_ID)
          {
            continue;
          }

          TSE_ASSERT(*iVIter < journeyItin.fareMarket().size()); // lint !e574

          FareMarket& curM = *(journeyItin.fareMarket()[*iVIter]);

          // If we find an outbound fare market in the journey itin
          // set the outbound indicator flag
          if (!foundOutbound && curM.direction() == FMDirection::OUTBOUND)
          {
            foundOutbound = true;
            continue;
          }

          // If we find an inbound fare market in the journey itin
          // set the inbound indicator flag
          if (!foundInbound && curM.direction() == FMDirection::INBOUND)
          {
            foundInbound = true;
            continue;
          }

          // If we find an unknown directional fare market in the journey itin,
          // set the indicator and immediately exit the loop as this modifies
          // the end logic for determining an across stop over leg's directionality
          if (!foundUnknown && curM.direction() == FMDirection::UNKNOWN)
          {
            foundUnknown = true;
            break;
          }
        }

        // If any unknown fare markets were found in the journey, the
        // across stop over leg must assume the same directionality status
        // as it jumps across a leg with unknown directionality
        if (foundUnknown)
        {
          firstFM.direction() = FMDirection::UNKNOWN;
        }
        else
        {
          // If both directions are found, we must set unknown as the direction
          // due to the mutually exclusive conflict
          if (foundOutbound && foundInbound)
          {
            firstFM.direction() = FMDirection::UNKNOWN;
          }
          else
          {
            firstFM.direction() = (foundOutbound) ? FMDirection::OUTBOUND : FMDirection::INBOUND;
          }
        }
      }
    }
    curLeg.directionalIndicator() = firstFM.direction();

    if (directItinV.size() == 1)
    {
      continue;
    }

    // Set all direct itins contained within the carrier index
    // to have the proper directionality
    std::vector<Itin*>::iterator dVIter = directItinV.begin() + 1;
    std::vector<Itin*>::iterator dVEIter = directItinV.end();
    for (; dVIter != dVEIter; ++dVIter)
    {
      Itin& dVItin = **dVIter;
      FareMarket& dVFM = (*(dVItin.fareMarket().front()));
      dVFM.direction() = firstFM.direction();
    }
  }

  // Cleanup journey itin
  journeyItin.fareMarket().clear();
}

std::string
ItinAnalyzerService::printNoPreferredClass(CabinType& preferredCabinClass, ShoppingTrx::Leg& curLeg)
    const
{
  std::string error = "NO " + preferredCabinClass.printName() + " AVAILABLE ";
  error += curLeg.sop()[0].itin()->travelSeg().front()->origin()->loc();
  error += "-";
  error += curLeg.sop()[0].itin()->travelSeg().back()->destination()->loc();

  return error;
}

//----------------------------------------------------------------------------
// validateFlightCabin()
// validate booking code for each flight with requested cabin.
// pass itin if :
// 1. requested cabin is offered and available.
// 2. requested cabin is not offered. the first lower level is offered and
//    available.
// Then remove all class of service of cabin less than the cabin that is offered
// and available.
// fail itin if :
// 1. requested cabin is offered but not available.
// 2. requested cabin is not offered. First lower level cabin that is offered

//    but not available.
//----------------------------------------------------------------------------
bool
ItinAnalyzerService::validateFlightCabin(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "ITIN VALIDATE FLCABIN");

  bool skipFirstLeg = false;
  bool expandedJumpCabinLogic = trx.getRequest()->getJumpCabinLogic() != JumpCabinLogic::ENABLED;

  FlightFinderTrx* ffTrx = dynamic_cast<FlightFinderTrx*>(&trx);
  if (ffTrx != nullptr && ffTrx->isBffReq() &&
      (ffTrx->bffStep() == FlightFinderTrx::STEP_4 || ffTrx->bffStep() == FlightFinderTrx::STEP_6))
  {
    skipFirstLeg = true;
  }

  //------------------------------------------------------------//
  // find total number of seat needed for all passenger types   //
  //------------------------------------------------------------//
  std::vector<PaxType*>::iterator paxTypeIter = trx.paxType().begin();
  std::vector<PaxType*>::iterator endPaxTypeIter = trx.paxType().end();
  uint16_t totalSeat = PaxTypeUtil::totalNumSeats(trx);

  std::vector<ShoppingTrx::Leg>::iterator legIter = trx.legs().begin();
  std::vector<ShoppingTrx::Leg>::iterator endLegIter = trx.legs().end();

  std::vector<ShoppingTrx::SchedulingOption*> sopVec, sopMixedVec;

  legIter = (skipFirstLeg) ? ++legIter : legIter;

  //-------------------------------//
  // Loop through all of the legs  //
  //-------------------------------//
  for (; legIter != endLegIter; ++legIter)
  {
    ShoppingTrx::Leg& curLeg = *legIter;

    CabinType preferredCabinClass = curLeg.preferredCabinClass();
    bool jumpDownAllowed = true;
    const bool totallyDisableJump =
        (trx.getRequest()->getJumpCabinLogic() == JumpCabinLogic::DISABLED);
    bool lastLeg = (legIter == endLegIter - 1);
    sopVec.clear();
    sopMixedVec.clear();

    std::map<CabinType, bool> legCabinsOffered = CabinType::createEmptyCabinBoolMap();
    std::map<CabinType, bool> legCabinsAvailable = CabinType::createEmptyCabinBoolMap();

    // For each leg, loop through the itineraries
    std::vector<ShoppingTrx::SchedulingOption>::iterator sopIter = curLeg.sop().begin();
    std::vector<ShoppingTrx::SchedulingOption>::iterator endSopIter = curLeg.sop().end();

    for (; sopIter != endSopIter; ++sopIter)
    {
      if (UNLIKELY(sopIter->getDummy()))
      {
        continue;
      }

      if (UNLIKELY(PricingTrx::FF_TRX == trx.getTrxType()))
      {
        ShoppingUtil::prepareFFClassOfService(*sopIter);
      }

      validateSOPPremiumFlightCabin(trx,
                                    preferredCabinClass,
                                    totalSeat,
                                    lastLeg,
                                    *sopIter,
                                    sopVec,
                                    sopMixedVec,
                                    jumpDownAllowed,
                                    legCabinsOffered,
                                    legCabinsAvailable);
    } // for sopIter

    // if jump down is allowed then try to jump down with the mix cabin with the requested cabin
    // fist.
    // if mixed cabin is not available then try to jump down to lower class totally.

    if (!fallback::fallbackDJCBannerFix(&trx) && TrxUtil::isJumpCabinLogicDisableCompletely(trx))
    {
      if (totallyDisableJump && jumpDownAllowed)
      {
        std::string errMsg(printNoPreferredClass(preferredCabinClass, curLeg));
        VALIDATE_OR_THROW(!sopMixedVec.empty(), INVALID_INPUT, errMsg);
      }
    }

    if (TrxUtil::isJumpCabinLogicDisableCompletely(trx))
      jumpDownAllowed &= (!totallyDisableJump);

    if (jumpDownAllowed)
    {
      if (expandedJumpCabinLogic && sopMixedVec.empty())
      {
        std::string errMsg(printNoPreferredClass(preferredCabinClass, curLeg));
        VALIDATE_OR_THROW(!sopMixedVec.empty(), INVALID_INPUT, errMsg);
      }

      if (sopMixedVec.empty() == false && sopVec.empty() == false)
      {
        std::vector<ShoppingTrx::SchedulingOption*>::iterator sopPtrIter = sopVec.begin();

        std::vector<ShoppingTrx::SchedulingOption*>::iterator endSopPtrIter = sopVec.end();
        for (; sopPtrIter != endSopPtrIter; ++sopPtrIter)
        {
          (*sopPtrIter)->cabinClassValid() = false;
        }
      }
      else if (sopVec.empty() == false)
      {
        validateJumpDown(
            trx, preferredCabinClass, sopVec, legCabinsOffered, legCabinsAvailable, curLeg);
      }
    }
    else
    {
      if (sopVec.empty() == false)
      {
        std::vector<ShoppingTrx::SchedulingOption*>::iterator sopPtrIter = sopVec.begin();
        std::vector<ShoppingTrx::SchedulingOption*>::iterator endSopPtrIter = sopVec.end();
        for (; sopPtrIter != endSopPtrIter; ++sopPtrIter)
        {
          (*sopPtrIter)->cabinClassValid() = false;
        }
      }
      // tempolary assume area2 to allow mixed class option to price per meeting 01/30
      // more requeirement will be provided. follow up with Marybeth

      bool allowMixed(!totallyDisableJump);

      if (!TrxUtil::isJumpCabinLogicDisableCompletely(trx))
        allowMixed = true;

      if (UNLIKELY(!allowMixed))
      {
        std::vector<ShoppingTrx::SchedulingOption*>::iterator sopPtrIter = sopMixedVec.begin();
        std::vector<ShoppingTrx::SchedulingOption*>::iterator endSopPtrIter = sopMixedVec.end();
        for (; sopPtrIter != endSopPtrIter; ++sopPtrIter)
        {
          (*sopPtrIter)->cabinClassValid() = false;
        }
      }
    }

    if (trx.diagnostic().diagnosticType() != Diagnostic908 || trx.diagnostic().processAllServices())
    {
      // now remove any items from this leg that have an invalid cabin
      // class. We don't need them anymore.

      // adjust the count of SOPs by counting the number of SOPs we
      // will be erasing
      curLeg.requestSops() -=
          std::count_if(curLeg.sop().begin(), curLeg.sop().end(), isCabinClassInvalid);

      // we need to loop over the SOPs, erasing each one with an invalid
      // cabin class. When we do this, we need to adjust the SOP ID mapping
      // in the trx, to make sure we give a good response at the end
      std::vector<ShoppingTrx::SchedulingOption>::iterator item = curLeg.sop().end();

      while ((item = std::find_if(curLeg.sop().begin(), curLeg.sop().end(), isCabinClassInvalid)) !=
             curLeg.sop().end())
      {
        const size_t leg = legIter - trx.legs().begin();
        const size_t index = item - curLeg.sop().begin();
        curLeg.sop().erase(item);

        // the map maps request ID to index, so we want to erase the item
        // with the index being removed. We also want to decrement all
        // indexes that are greater than the index being removed.
        for (std::map<uint32_t, uint32_t>::iterator i = trx.schedulingOptionIndices()[leg].begin();
             i != trx.schedulingOptionIndices()[leg].end();)
        {
          if (i->second == index)
          {
            trx.schedulingOptionIndices()[leg].erase(i++);
            continue;
          }
          else if (i->second > index)
          {
            i->second--;
          }

          ++i;
        }
      }
    }

    validateSpecialProcessingStatus(trx, curLeg);

    if (trx.awardRequest())
    {
      if (curLeg.sop().empty())
      {
        throw ErrorResponseException(ErrorResponseException::SOLD_OUT);
      }
    }

    std::ostringstream error;
    if (curLeg.sop().empty() == true)
    {
      error << "NO AVAILABLE FLIGHT SCHEDULES";
    }

    if ((ffTrx != nullptr) && (ffTrx->isFFReq() || ffTrx->avlInS1S3Request()))
    {
      continue;
    }

    VALIDATE_OR_THROW(curLeg.sop().empty() == false, INVALID_INPUT, error.str());

  } // for Legs

  setIndicesToSchedulingOption(trx);
  return (true);
}

void
ItinAnalyzerService::validateSpecialProcessingStatus(ShoppingTrx& trx,
                                                     const ShoppingTrx::Leg& curLeg)
{
  if ((false == curLeg.isCustomLeg()) || (trx.getNumOfCustomSolutions() == 0))
  {
    // We don't need to validate if it's not a custom leg or
    // the trx doesn't require any custom solutions
    return;
  }

  for (uint16_t i = 0; i < curLeg.sop().size(); i++)
  {
    if (curLeg.sop()[i].isCustomSop())
    {
      // We have at least one custom SOP in this custom leg. This Leg is valid for
      // special processing.
      return;
    }
  }

  // If there is no custom sop left in a custom Leg, we can't proceed
  // with custom solution. So, we will deactivate custom solution logic
  // by setting number of custom solutions to zero.
  trx.setNumOfCustomSolutions(0);
}

void
ItinAnalyzerService::validateJumpDown(ShoppingTrx& trx,
                                      CabinType preferredCabinClass,
                                      std::vector<ShoppingTrx::SchedulingOption*>& sopVec,
                                      std::map<CabinType, bool>& legCabinOffered,
                                      std::map<CabinType, bool>& legCabinAvailable,
                                      ShoppingTrx::Leg& curLeg)
{
  bool nextCabinOfferButNotAvailable = false;
  CabinType priceCabin = CabinType::addOneLevelToCabinType(preferredCabinClass);
  CabinType economyCabin;
  economyCabin.setEconomyClass();
  for (; priceCabin <= economyCabin; priceCabin = CabinType::addOneLevelToCabinType(priceCabin))
  {
    if (legCabinOffered[priceCabin])
    {
      if (!legCabinAvailable[priceCabin])
      {
        nextCabinOfferButNotAvailable = true;
      }
      break;
    }
  }

  if (!nextCabinOfferButNotAvailable && (preferredCabinClass < economyCabin))
  {
    curLeg.preferredCabinClass() = priceCabin;
  }

  std::vector<ShoppingTrx::SchedulingOption*>::iterator sopPtrIter = sopVec.begin();
  std::vector<ShoppingTrx::SchedulingOption*>::iterator endSopPtrIter = sopVec.end();
  for (; sopPtrIter != endSopPtrIter; ++sopPtrIter)
  {
    if (nextCabinOfferButNotAvailable || // fail all sops
        getSopCabin(**sopPtrIter) != priceCabin)
    {
      (*sopPtrIter)->cabinClassValid() = false;
    }
  }
}

CabinType
ItinAnalyzerService::getSopCabin(ShoppingTrx::SchedulingOption& sop)
{
  CabinType sopCabin;
  sopCabin.setUndefinedClass();
  Itin*& curItin = sop.itin();
  if (curItin == nullptr)
  {
    return sopCabin;
  }
  std::vector<TravelSeg*>::const_iterator trvSegIter;
  for (trvSegIter = curItin->travelSeg().begin(); trvSegIter != curItin->travelSeg().end();
       ++trvSegIter)
  {
    if (sopCabin.isUndefinedClass() || sopCabin > (*trvSegIter)->bookedCabin())
    {
      sopCabin = (*trvSegIter)->bookedCabin();
    }
  }
  return sopCabin;
}

void
ItinAnalyzerService::validateSOPPremiumFlightCabin(
    ShoppingTrx& trx,
    CabinType preferredCabinClass,
    int totalSeat,
    bool lastLeg,
    ShoppingTrx::SchedulingOption& sop,
    std::vector<ShoppingTrx::SchedulingOption*>& sopVec,
    std::vector<ShoppingTrx::SchedulingOption*>& sopMixedVec,
    bool& jumpDownAllowed,
    std::map<CabinType, bool>& legCabinOffered,
    std::map<CabinType, bool>& legCabinAvailable)
{
  bool requestedCabinSopFound = false;
  bool nonRequestedCabinSopFound = false;

  validateSOPPremiumTravSegVector(trx,
                                  sop,
                                  preferredCabinClass,
                                  totalSeat,
                                  lastLeg,
                                  requestedCabinSopFound,
                                  nonRequestedCabinSopFound,
                                  legCabinOffered,
                                  legCabinAvailable);

  // requested cabin is found  then not allowed to jump down
  if (jumpDownAllowed && requestedCabinSopFound && !nonRequestedCabinSopFound)
  {
    jumpDownAllowed = false;
  }

  // do not know yet whether jump down allow or not. Keep sop in sopVec.
  if (nonRequestedCabinSopFound && requestedCabinSopFound) // mixed cabin found
  {
    sopMixedVec.push_back(&sop);
  }
  if (nonRequestedCabinSopFound && !requestedCabinSopFound) // all lower class cabin
  {
    sopVec.push_back(&sop);
  }
} // validateSOPPremiumFlightCabin

void
ItinAnalyzerService::validateSOPPremiumTravSegVector(ShoppingTrx& trx,
                                                     ShoppingTrx::SchedulingOption& sop,
                                                     CabinType preferredCabinClass,
                                                     int totalSeat,
                                                     bool lastLeg,
                                                     bool& requestedCabinSopFound,
                                                     bool& nonRequestedCabinSopFound,
                                                     std::map<CabinType, bool>& legCabinsOffered,
                                                     std::map<CabinType, bool>& legCabinsAvailable)
{
  Itin*& curItin = sop.itin();
  if (UNLIKELY(curItin == nullptr))
  {
    return;
  }

  if (lastLeg && !curItin->travelSeg().empty())
  {
    curItin->travelSeg().back()->stopOver() = false;
  }

  //------------------------------//
  // Loop through all travelSegs  //
  //------------------------------//
  std::vector<TravelSeg*>::const_iterator trvSegIter;
  CabinType lowestSOPCabinType;
  lowestSOPCabinType.setUndefinedClass();
  bool lowestSOPCabinTypeAvailable = true;
  // 1st Loop through the travel segments to find whether the preferred cabin is offered and
  // available or not

  for (trvSegIter = curItin->travelSeg().begin(); trvSegIter != curItin->travelSeg().end();
       ++trvSegIter)
  {
    bool requestedCabinFound = false;
    std::map<CabinType, bool> cabinsOffered = CabinType::createEmptyCabinBoolMap();
    std::map<CabinType, bool> cabinsAvailable = CabinType::createEmptyCabinBoolMap();

    AirSeg* curAirSeg = dynamic_cast<AirSeg*>(*trvSegIter);
    if (UNLIKELY(curAirSeg == nullptr))
    {
      continue;
    }

    std::vector<ClassOfService*>& classOfService = curAirSeg->classOfService();

    validateSOPPremiumCOSVector(trx,
                                totalSeat,
                                curAirSeg,
                                classOfService,
                                cabinsOffered,
                                cabinsAvailable,
                                requestedCabinFound,
                                preferredCabinClass);

    if (requestedCabinFound)
    {
      requestedCabinSopFound = true;
    }
    else
    {
      nonRequestedCabinSopFound = true;
    }

    // this travel seg is either offer and available or not offer
    // check to remove unneeded cabin
    // nothing to remove if price cabin is economy
    CabinType priceCabin;
    CabinType economyCabin;
    economyCabin.setEconomyClass();
    priceCabin.setEconomyClass();

    CabinType bestSuitableOfferedClass = CabinType::addOneLevelToCabinType(economyCabin);

    // find the most suitable class offered
    for (CabinType i = preferredCabinClass; i <= economyCabin;
         i = CabinType::addOneLevelToCabinType(i))
    {
      if (cabinsOffered[i])
      {
        bestSuitableOfferedClass = i;
        break;
      }
    }
    if (lowestSOPCabinType.isUndefinedClass() || lowestSOPCabinType > bestSuitableOfferedClass)
    {
      lowestSOPCabinType = bestSuitableOfferedClass;
    }

    if ((bestSuitableOfferedClass <= economyCabin) && !cabinsAvailable[bestSuitableOfferedClass])
    {
      lowestSOPCabinTypeAvailable = false;
      sop.cabinClassValid() = false;
      break;
    }
    else
    {
      priceCabin = bestSuitableOfferedClass;
    }

    if (!requestedCabinFound &&
        (curAirSeg->getBookingCode().empty() || curAirSeg->getBookingCode() == "0"))
    {
      setupDefaultBookingCodeAndCabin(sop, totalSeat, curAirSeg, classOfService);
    }
    else
    {
      FilterClassOfServiceByPriceCabin cosFilter(priceCabin);

      classOfService.erase(std::remove_if(classOfService.begin(), classOfService.end(), cosFilter),
                           classOfService.end());
    }

  } // for trvlSegs
  legCabinsOffered[lowestSOPCabinType] = true;
  if (lowestSOPCabinTypeAvailable)
  {
    legCabinsAvailable[lowestSOPCabinType] = lowestSOPCabinTypeAvailable;
  }
}

void
ItinAnalyzerService::setupDefaultBookingCodeAndCabin(ShoppingTrx::SchedulingOption& sop,
                                                     int totalSeat,
                                                     AirSeg* curAirSeg,
                                                     std::vector<ClassOfService*>& classOfService)
{
  std::vector<ClassOfService*>::reverse_iterator cosIter;
  std::vector<ClassOfService*>::reverse_iterator cosEndIter = classOfService.rend();

  for (cosIter = classOfService.rbegin(); cosIter != cosEndIter; ++cosIter)
  {
    ClassOfService* cos = *cosIter;

    if (!cos || cos->cabin().isUndefinedClass())
    {
      continue;
    }

    if (cos->numSeats() >= totalSeat)
    {
      curAirSeg->bookedCabin() = cos->cabin();
      curAirSeg->setBookingCode(cos->bookingCode());
      break;
    }

    if (cosIter == cosEndIter - 1)
    {
      sop.cabinClassValid() = false;
    }
  }
}

void
ItinAnalyzerService::validateSOPPremiumCOSVector(ShoppingTrx& trx,
                                                 int totalSeat,
                                                 AirSeg* curAirSeg,
                                                 std::vector<ClassOfService*>& classOfService,
                                                 std::map<CabinType, bool>& cabinsOffered,
                                                 std::map<CabinType, bool>& cabinsAvailable,
                                                 bool& requestedCabinFound,
                                                 CabinType preferredClass)
{
  // Loop through class of service for each travel seg
  std::vector<ClassOfService*>::iterator cosIter;
  ClassOfService* cos;

  for (cosIter = classOfService.begin(); cosIter != classOfService.end(); ++cosIter)
  {
    cos = *cosIter;

    // Make sure cos itself is not null
    if (UNLIKELY(!cos))
    {
      continue;
    }

    if (UNLIKELY(cos->cabin().isUndefinedClass()))
    {
      const Cabin* aCabin = nullptr;
      if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx))
      {
        RBDByCabinUtil rbdCabin(trx, ITIN_SHP_SVC);
        aCabin = rbdCabin.getCabinByRBD(curAirSeg->carrier(), cos->bookingCode(), *curAirSeg);
      }
      else
      {
        aCabin = trx.dataHandle().getCabin(
            curAirSeg->carrier(), cos->bookingCode(), curAirSeg->departureDT());
      }
      if (aCabin != nullptr)
      {
        cos->cabin() = aCabin->cabin();
      }
    }
    if (cos->cabin() == preferredClass)
    {
      requestedCabinFound = true;
      cabinsOffered[cos->cabin()] = true;

      if (cos->numSeats() >= totalSeat)
      {
        cabinsAvailable[cos->cabin()] = true;
        curAirSeg->bookedCabin() = cos->cabin();
        curAirSeg->setBookingCode(cos->bookingCode());
        break;
      }
    }

    else
    {
      if ((cos->cabin() > preferredClass) &&
          (!cabinsOffered[cos->cabin()] || !cabinsAvailable[cos->cabin()]))
      {
        cabinsOffered[cos->cabin()] = true;
        if (cos->numSeats() >= totalSeat)
        {
          cabinsAvailable[cos->cabin()] = true;
          bool moreSuitableClassAlreadyFound = false;
          for (CabinType i = CabinType::addOneLevelToCabinType(preferredClass); i < cos->cabin();
               i = CabinType::addOneLevelToCabinType(i))
          {
            if (cabinsOffered[i])
            {
              moreSuitableClassAlreadyFound = true;
              break;
            }
          }

          if (!moreSuitableClassAlreadyFound)
          {
            curAirSeg->bookedCabin() = cos->cabin();
            curAirSeg->setBookingCode(cos->bookingCode());
          }
        }
      }
    } // else
  } // for cosIter
}

bool
ItinAnalyzerService::validateFlightCabinESV(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "ITIN VALIDATE FLIGHT CABIN ESV");

  LOG4CXX_DEBUG(_logger, "ItinAnalyzerService::validateFlightCabinESV(ShoppingTrx&)");

  // Go thorough all legs
  std::vector<ShoppingTrx::Leg>::iterator legIter;

  for (legIter = trx.legs().begin(); legIter != trx.legs().end(); ++legIter)
  {
    ShoppingTrx::Leg& leg = (*legIter);

    // Check if scheduling options vector is not empty
    if (true == leg.sop().empty())
    {
      LOG4CXX_ERROR(
          _logger,
          "ItinAnalyzerService::validateFlightCabinESV - Scheduling options vector is empty");
      return false;
    }

    std::vector<ShoppingTrx::SchedulingOption>::iterator sopIter;

    // Go thorough all scheduling options
    for (sopIter = leg.sop().begin(); sopIter != leg.sop().end(); ++sopIter)
    {
      ShoppingTrx::SchedulingOption& sop = (*sopIter);

      sop.cabinClassValid() = true;

      Itin* itin = sop.itin();

      // Check if Itinerary object is not NULL
      if (nullptr == itin)
      {
        LOG4CXX_ERROR(_logger,
                      "ItinAnalyzerService::validateFlightCabinESV - Itinerary object is NULL");
        continue;
      }

      std::vector<TravelSeg*>::iterator segIter;

      for (segIter = itin->travelSeg().begin(); segIter != itin->travelSeg().end(); ++segIter)
      {
        TravelSeg* travelSeg = (*segIter);
        AirSeg& airSegment = dynamic_cast<AirSeg&>(*(travelSeg));

        bool avlFound = false;
        std::vector<uint32_t>::iterator avlIter;

        for (avlIter = airSegment.validAvailabilityIds().begin();
             avlIter != airSegment.validAvailabilityIds().end();
             ++avlIter)
        {
          uint32_t avlId = (*avlIter);

          if (sop.availabilityIds().end() !=
              std::find(sop.availabilityIds().begin(), sop.availabilityIds().end(), avlId))
          {
            avlFound = true;
            break;
          }
        }

        if (false == avlFound)
        {
          sop.cabinClassValid() = false;
          break;
        }
      }
    }

    bool removeInvalidSops = true;

    if (Diagnostic900 == trx.diagnostic().diagnosticType())
    {
      Diag900Collector* diag900Collector =
          dynamic_cast<Diag900Collector*>(DCFactory::instance()->create(trx));

      const std::string& displayRemovedSops =
          diag900Collector->rootDiag()->diagParamMapItem("RMSOPS");

      if ("Y" == displayRemovedSops)
      {
        removeInvalidSops = false;
      }
    }

    if (true == removeInvalidSops)
    {
      // Remove all scheduling options with not complete availability
      leg.sop().erase(remove_if(leg.sop().begin(), leg.sop().end(), isCabinClassInvalid),
                      leg.sop().end());
    }
    else
    {
      // Remove all scheduling options with complete availability
      // (in order to display removed sops in 900 diag)
      leg.sop().erase(remove_if(leg.sop().begin(), leg.sop().end(), isCabinClassValid),
                      leg.sop().end());
    }
  }

  return true;
}

void
ItinAnalyzerService::setShoppingFareMarketInfo(ShoppingTrx& trx)
{
  std::vector<ShoppingTrx::Leg>& sLV = trx.legs();

  if (sLV.empty())
    return;

  std::vector<CarrierCode> validatingCxrs;
  if (trx.getTrxType() == PricingTrx::FF_TRX)
    ValidatingCxrUtil::getAllValidatingCarriersForFlightFinderTrx(trx, validatingCxrs);

  std::vector<ShoppingTrx::Leg>::iterator sLVIter = sLV.begin();
  std::vector<ShoppingTrx::Leg>::iterator sLVEndIter = sLV.end();

  for (uint16_t legIndex = 0; sLVIter != sLVEndIter; ++sLVIter, ++legIndex)
  {
    ShoppingTrx::Leg& curLeg = (*sLVIter);
    ItinIndex& sIG = curLeg.carrierIndex();

    ItinIndex::ItinMatrixIterator iMIter = sIG.root().begin();
    ItinIndex::ItinMatrixIterator iMEndIter = sIG.root().end();
    for (; iMIter != iMEndIter; ++iMIter)
    {
      ItinIndex::ItinCell* curCell =
          ShoppingUtil::retrieveDirectItin(trx, legIndex, iMIter->first, ItinIndex::CHECK_NOTHING);
      if (!curCell)
        continue;

      Itin* thruFareDirectItin = curCell->second;
      if (thruFareDirectItin == nullptr)
        continue;

      for (FareMarket* fM : thruFareDirectItin->fareMarket())
      {
        if (LIKELY(fM))
        {
          fM->legIndex() = legIndex;

          if (trx.getTrxType() == PricingTrx::FF_TRX)
            fM->validatingCarriers() = validatingCxrs;

          if (LIKELY(!trx.isIataFareSelectionApplicable()))
            break; // pre fare-selection only processes first FM
        }
      }
    }

    // For each leg, loop through the itineraries
    std::vector<ShoppingTrx::SchedulingOption>::iterator sopIter = curLeg.sop().begin();
    std::vector<ShoppingTrx::SchedulingOption>::iterator endSopIter = curLeg.sop().end();
    const int64_t DomesticConnectionTime = 4 * 60 * 60; // 4 hours

    for (; sopIter != endSopIter; ++sopIter)
    {
      Itin*& itin = sopIter->itin();
      if (itin->travelSeg().size() <= 1)
        continue;

      if (itin->travelSeg().front()->departureDT().date() <
          itin->travelSeg().back()->departureDT().date()) // date change
      {
        curLeg.dateChange() = true;
      }

      TravelSegPtrVec itinTvlSegs = itin->travelSeg();
      TravelSegPtrVecI nextTvlIt = itinTvlSegs.begin();
      TravelSegPtrVecI it = nextTvlIt++;

      for (; nextTvlIt != itinTvlSegs.end(); ++nextTvlIt, ++it)
      {
        if (((*it)->geoTravelType() == GeoTravelType::Domestic ||
             (*it)->geoTravelType() == GeoTravelType::Transborder) &&
            ((*nextTvlIt)->geoTravelType() == GeoTravelType::Domestic ||
             (*nextTvlIt)->geoTravelType() == GeoTravelType::Transborder))
        {
          const int64_t diff = DateTime::diffTime((*nextTvlIt)->departureDT(), (*it)->arrivalDT());
          if (diff > DomesticConnectionTime)
          {
            sopIter->domesticCnxTimeMoreThan4() = true;
            break;
          }
        }
      }
    }
  }
}

void
ItinAnalyzerService::generateAltDatesMap(ShoppingTrx& trx) const
{
  // make an algorithm which iterates over SOPs and generates
  // all possible date pairs and puts them in trx.altDatePairs()

  typedef ShoppingTrx::AltDateInfo AltDateInfo;

  std::set<DateTime> possibleDepartureDates;
  std::set<DateTime> possibleArrivalDates;

  for (size_t i = 0; i != trx.legs()[0].sop().size(); ++i)
  {
    TravelSeg* airSegPtr1 = trx.legs()[0].sop()[i].itin()->travelSeg().front();
    const TravelSeg& aSeg1 = *airSegPtr1;
    const DateTime& depDT1 = aSeg1.departureDT();
    DateTime dateOnly(depDT1.year(), depDT1.month(), depDT1.day());
    possibleDepartureDates.insert(dateOnly);
  }

  if (trx.legs().size() < 2)
  {
    possibleArrivalDates.insert(DateTime::emptyDate());
  }

  else
  {
    for (size_t i = 0; i != trx.legs()[1].sop().size(); ++i)
    {
      TravelSeg* airSegPtr2 = trx.legs()[1].sop()[i].itin()->travelSeg().front();
      const TravelSeg& aSeg2 = *airSegPtr2;
      const DateTime& depDT2 = aSeg2.departureDT();
      DateTime dateOnly(depDT2.year(), depDT2.month(), depDT2.day());
      possibleArrivalDates.insert(dateOnly);
    }
  }

  for (std::set<DateTime>::const_iterator i = possibleDepartureDates.begin();
       i != possibleDepartureDates.end();
       ++i)
  {
    for (std::set<DateTime>::const_iterator j = possibleArrivalDates.begin();
         j != possibleArrivalDates.end();
         ++j)
    {
      DatePair datePair(*i, *j);
      PricingTrx::AltDateInfo* altDateInfo = nullptr;
      trx.dataHandle().get(altDateInfo);
      std::pair<DatePair, AltDateInfo*> mapItem(datePair, altDateInfo);
      trx.altDatePairs().insert(mapItem);
    }
  }

  const DateTime traveDate = trx.getTravelDate();
  const DateTime returnDate = trx.getReturnDate();
  const DateTime emptyDate(DateTime::emptyDate());

  if (traveDate != emptyDate)
  {
    DatePair datePairPDT(traveDate, returnDate);

    std::map<DatePair, AltDateInfo*>& altDateMap = trx.altDatePairs();

    std::map<DatePair, AltDateInfo*>::iterator altDateMapIt = altDateMap.find(datePairPDT);

    if (altDateMapIt != altDateMap.end())
    {
      altDateMap.erase(altDateMapIt);
    }
  }
}

void
ItinAnalyzerService::checkValidateAltDays(ShoppingTrx& trx) const
{
  generateAltDatesMap(trx);
  typedef ShoppingTrx::AltDateInfo AltDateInfo;

  for (std::map<DatePair, AltDateInfo*>::iterator i = trx.altDatePairs().begin();
       i != trx.altDatePairs().end();)
  {
    DatePair myPair = i->first;
    bool bIsValid = ShoppingAltDateUtil::validateAltDatePair(trx, myPair);
    if (!bIsValid)
    {
      trx.altDatePairs().erase(i++);
    }
    else
    {
      ++i;
    }
  }
}

void
ItinAnalyzerService::buildLocalMarkets(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "ITIN BUILD LOCAL MARKETS");

  LOG4CXX_DEBUG(_logger, "ItinAnalyzerService::buildLocalMarkets(ShoppingTrx&)");

  // Check if legs vector is not empty
  if (true == trx.legs().empty())
  {
    LOG4CXX_ERROR(_logger, "ItinAnalyzerService::buildLocalMarkets - Legs vector is empty");
    return;
  }

  // Clear fare market vector for current transaction
  trx.fareMarket().clear();

  // Go thorough all legs
  for (uint32_t legId = 0; legId != trx.legs().size(); legId++)
  {
    ShoppingTrx::Leg& curLeg = trx.legs()[legId];

    // Check if scheduling options vector is not empty
    if (true == curLeg.sop().empty())
    {
      if (Diagnostic900 == trx.diagnostic().diagnosticType())
      {
        continue;
      }

      std::string error = "";
      std::stringstream s;
      s << (legId + 1);

      error = "NO VALID FLIGHTS FOUND FOR LEG: ";
      error += s.str();

      VALIDATE_OR_THROW(false, INVALID_INPUT, error);
    }

    std::vector<ShoppingTrx::SchedulingOption>::iterator sopIter;

    // Go thorough all scheduling options
    for (sopIter = curLeg.sop().begin(); sopIter != curLeg.sop().end(); sopIter++)
    {
      // Check if it's not a dummy sop
      if (false == sopIter->getDummy())
      {
        Itin* curItin = sopIter->itin();

        // Check if Itinerary object is not NULL
        if (nullptr == curItin)
        {
          LOG4CXX_ERROR(_logger,
                        "ItinAnalyzerService::buildLocalMarkets - Itinerary object is NULL");
          continue;
        }

        if (sopIter->governingCarrier().empty())
        {
          LOG4CXX_ERROR(_logger,
                        "ItinAnalyzerService::buildLocalMarkets - Empty governinig "
                        "carrier in itinerary object");
          continue;
        }

        // Create carrier key for governing carrier from itinerary
        ItinIndex::Key carrierKey;
        ShoppingUtil::createCxrKey(sopIter->governingCarrier(), carrierKey);

        ItinIndex& curCarrierIndex = curLeg.carrierIndex();

        Itin* dummyItin =
            ShoppingUtil::getDummyItineraryFromCarrierIndex(curCarrierIndex, carrierKey);

        // Check if dummy itinerary object is not NULL
        if (nullptr == dummyItin)
        {
          LOG4CXX_ERROR(_logger,
                        "ItinAnalyzerService::buildLocalMarkets - Dummy itinerary object is NULL");
          continue;
        }

        // If processing itin is online set correct online carrier
        curItin->onlineCarrier() =
            getFareMarketCarrier(trx, curItin->travelSeg().begin(), curItin->travelSeg().end() - 1);

        // Clear fareMarket vector for current itinerary (we will fill
        // this vector with all posible local markets)
        curItin->fareMarket().clear();

        buildMarketsForItinerary(trx, curItin, dummyItin, legId);
      }
    }
  }
}

void
ItinAnalyzerService::buildMarketsForItinerary(ShoppingTrx& trx,
                                              Itin* curItin,
                                              Itin* dummyItin,
                                              uint32_t legId)
{
  TSELatencyData metrics(trx, "ITIN BUILD MARKETS FOR ITINERARY");

  LOG4CXX_DEBUG(
      _logger,
      "ItinAnalyzerService::buildMarketsForItinerary(ShoppingTrx&, Itin*, Itin*, uint32_t)");

  std::vector<TravelSeg*>& trvSegs = curItin->travelSeg();

  // Check if travel segments vector is not empty
  if (true == trvSegs.empty())
  {
    LOG4CXX_ERROR(
        _logger, "ItinAnalyzerService::buildMarketsForItinerary - Travel segments vector is empty");
    return;
  }

  int maxSegCountInFare = 3;

  std::vector<TravelSeg*>::iterator segIterOrig;
  std::vector<TravelSeg*>::iterator segIterDest;

  // Combine all segments in order to find all possible local markets
  for (segIterOrig = trvSegs.begin(); segIterOrig != trvSegs.end(); segIterOrig++)
  {
    int segCount = 0;

    for (segIterDest = trvSegs.begin(); segIterDest != trvSegs.end(); segIterDest++)
    {
      if (segIterOrig <= segIterDest)
      {
        segCount++;

        // Build fare market only when we've got up to 3 segments for
        // fare market
        if (segCount > maxSegCountInFare)
        {
          break;
        }

        // Build fare markets only when we've got the same carrier on
        // every travle segment
        CarrierCode carrierCode = getFareMarketCarrier(trx, segIterOrig, segIterDest);

        // If carrier code is empty it means that there are different
        // carriers in travel segments for processing fare market
        if ("" == carrierCode)
        {
          break;
        }

        // Create empty FareMarket object
        FareMarket* fareMarket;
        trx.dataHandle().get(fareMarket);

        // Fill fareMarket object with data from current segment
        // and current itinerary
        fareMarket->origin() = ((TravelSeg*)*segIterOrig)->origin();
        fareMarket->destination() = ((TravelSeg*)*segIterDest)->destination();
        fareMarket->boardMultiCity() = ((TravelSeg*)*segIterOrig)->boardMultiCity();
        fareMarket->offMultiCity() = ((TravelSeg*)*segIterDest)->offMultiCity();
        fareMarket->travelDate() = curItin->travelDate();

        // Add all needed segments to apropriate fare market
        std::vector<TravelSeg*>::iterator segIter;

        for (segIter = segIterOrig; segIter <= segIterDest; segIter++)
        {
          fareMarket->travelSeg().push_back(*segIter);
        }

        // Set travel boundary and geo travel type for current fare
        // market
        Boundary tvlBoundary = _tvlSegAnalysis.selectTravelBoundary(fareMarket->travelSeg());
        ItinUtil::setGeoTravelType(tvlBoundary, *fareMarket);

        // Set fare market direction to outbound for fare markets from
        // first leg and to inbound for fare markets from second leg
        fareMarket->direction() = (0 == legId) ? FMDirection::OUTBOUND : FMDirection::INBOUND;

        // Set apropriate carrier for fare market
        fareMarket->governingCarrier() = carrierCode;

        // Set flag for OND serialization
        if ((segIterOrig == trvSegs.begin()) && (segIterDest == trvSegs.end() - 1))
          fareMarket->setEsvThtuMarket(true);

        addFareMarket(
            trx, fareMarket, trx.fareMarket(), dummyItin->fareMarket(), curItin->fareMarket());
      }
    }
  }
}

CarrierCode
ItinAnalyzerService::getFareMarketCarrier(ShoppingTrx& trx,
                                          std::vector<TravelSeg*>::iterator segIterOrig,
                                          std::vector<TravelSeg*>::iterator segIterDest)
{
  TSELatencyData metrics(trx, "ITIN GET FARE MARKET CARRIER");

  LOG4CXX_DEBUG(_logger,
                "ItinAnalyzerService::getFareMarketCarrier(ShoppingTrx&, "
                "std::vector<TravelSeg*>::iterator, std::vector<TravelSeg*>::iterator)");

  CarrierCode carrierCode = "";

  std::vector<TravelSeg*>::iterator segIter;

  for (segIter = segIterOrig; segIter <= segIterDest; segIter++)
  {
    TravelSeg* travelSeg = (*segIter);

    const AirSeg& airSegment = dynamic_cast<const AirSeg&>(*(travelSeg));

    if ("" == carrierCode)
    {
      carrierCode = airSegment.carrier();
    }
    else
    {
      if (carrierCode != airSegment.carrier())
      {
        carrierCode = "";
        return carrierCode;
      }
    }
  }

  return carrierCode;
}

FareMarket*
ItinAnalyzerService::getExistingFareMarket(ShoppingTrx& trx,
                                           FareMarket& fareMarket,
                                           std::vector<FareMarket*>& fareMarkets)
{
  TSELatencyData metrics(trx, "ITIN GET EXISTING FARE MARKET");

  LOG4CXX_DEBUG(_logger,
                "ItinAnalyzerService::getExistingFareMarket(ShoppingTrx&, FareMarket&, "
                "std::vector<FareMarket*>&)");

  // Go thorough all fare markets in fare market vector if current fare market
  // already exist in fare markets vector return pinter to existing one,
  // otherwise return NULL
  for (std::vector<FareMarket*>::const_iterator fareMarketsIter = fareMarkets.begin();
       fareMarketsIter != fareMarkets.end();
       fareMarketsIter++)
  {
    if ((*fareMarketsIter)->origin() != fareMarket.origin())
      continue;
    if ((*fareMarketsIter)->destination() != fareMarket.destination())
      continue;
    if ((*fareMarketsIter)->travelDate().date() != fareMarket.travelDate().date())
      continue;
    if ((*fareMarketsIter)->governingCarrier() != fareMarket.governingCarrier())
      continue;
    if ((*fareMarketsIter)->direction() != fareMarket.direction())
      continue;

    LOG4CXX_INFO(_logger,
                 "FareMarket: " << fareMarket.origin()->loc() << "-"
                                << fareMarket.destination()->loc() << " already exists, REUSING");
    LOG4CXX_DEBUG(_logger,
                  "  fareMarket == existing fareMarket: " << (**fareMarketsIter == fareMarket));

    return (*fareMarketsIter);
  }

  return nullptr;
}

void
ItinAnalyzerService::addFareMarket(ShoppingTrx& trx,
                                   FareMarket* fareMarket,
                                   std::vector<FareMarket*>& trxFareMarkets,
                                   std::vector<FareMarket*>& carrierFareMarkets,
                                   std::vector<FareMarket*>& itineraryFareMarkets)
{
  TSELatencyData metrics(trx, "ITIN ADD FARE MARKET");

  LOG4CXX_DEBUG(_logger,
                "ItinAnalyzerService::addFareMarket(ShoppingTrx&, FareMarket* , "
                "std::vector<FareMarket*>&, std::vector<FareMarket*>&, "
                "std::vector<FareMarket*>&)");

  FareMarket* existingTrxFareMarket = nullptr;
  FareMarket* existingCarrierFareMarket = nullptr;
  FareMarket* existingItineraryFareMarket = nullptr;

  // Check if new fare market exist in trx fare markets vector
  if (nullptr == (existingTrxFareMarket = getExistingFareMarket(trx, *fareMarket, trxFareMarkets)))
  {
    // If it doesn't exist add it to current itinerary, carrier index and
    // trx fare markets vectors
    itineraryFareMarkets.push_back(fareMarket);
    carrierFareMarkets.push_back(fareMarket);
    trxFareMarkets.push_back(fareMarket);
    fareMarket->ruleApplicationDate() = DateTime::emptyDate();
  }
  else
  {
    // If it exist in trx fare markets vector check if it exist also in
    // carrier index fare markets vector
    if (nullptr ==
        (existingCarrierFareMarket = getExistingFareMarket(trx, *fareMarket, carrierFareMarkets)))
    {
      // If it doesn't exist add pointer to existing one to current
      // itinerary and carier index fare markets vectors
      itineraryFareMarkets.push_back(existingTrxFareMarket);
      carrierFareMarkets.push_back(existingTrxFareMarket);
    }
    else
    {
      // If it exist in trx and carrier index fare markets vectors check
      // if it exist also in current itinerary fare markets vector
      if (nullptr == (existingItineraryFareMarket =
                          getExistingFareMarket(trx, *fareMarket, itineraryFareMarkets)))
      {
        // If it doesn't exist add pointer to existing one to current
        // itinerary fare markets vector
        itineraryFareMarkets.push_back(existingCarrierFareMarket);
      }
    }
  }
}

void
ItinAnalyzerService::addDummySop(ShoppingTrx& trx,
                                 ShoppingTrx::Leg& curLeg,
                                 const AirSeg& journeySeg)
{
  LocCode boardCity = journeySeg.origAirport();
  LocCode offCity = journeySeg.destAirport();
  DateTime depDate = journeySeg.departureDT();
  CarrierCode cxr = journeySeg.carrier();
  int16_t pnrSegment = 1;
  uint32_t sopIndex = 0;

  Itin* itin;
  trx.dataHandle().get(itin);

  ShoppingAltDateUtil::generateJourneySegAndFM(
      trx.dataHandle(), *itin, depDate, boardCity, offCity, cxr, pnrSegment);
  itin->fareMarket().front()->travelDate() = trx.journeyItin()->travelDate();

  curLeg.sop().push_back(ShoppingTrx::SchedulingOption(itin, sopIndex));
}

namespace
{
class InterlineCarriersValid
{
public:
  InterlineCarriersValid(ShoppingTrx& trx,
                         InterlineTicketCarrier* interlineTicketCarrier,
                         bool removeInvalidInterline)
    : _trx(trx),
      _interlineTicketCarrier(interlineTicketCarrier),
      _removeInvalidInterline(removeInvalidInterline)
  {
  }

  bool operator()(const ShoppingTrx::SchedulingOption& sop) const
  {
    bool result = false;

    if (sop.itin()->travelSeg().empty())
    {
      return _removeInvalidInterline ? !result : result;
    }

    AirSeg* airSeg = dynamic_cast<AirSeg*>(sop.itin()->travelSeg()[0]);

    if (airSeg == nullptr)
    {
      return _removeInvalidInterline ? !result : result;
    }

    CarrierCode& validatingCarrier = airSeg->carrier();
    std::set<CarrierCode> cxrs;

    ShoppingUtil::buildCarriersSet(sop.itin(), cxrs);

    result = ShoppingUtil::checkCarriersSet(_trx, validatingCarrier, _interlineTicketCarrier, cxrs);

    return _removeInvalidInterline ? !result : result;
  }

private:
  ShoppingTrx& _trx;
  InterlineTicketCarrier* _interlineTicketCarrier;
  bool _removeInvalidInterline;
};

} // anonymous namespace

void
ItinAnalyzerService::validateInterlineTicketCarrierESV(ShoppingTrx& trx)
{
  TSELatencyData metrics(trx, "ITIN VALIDATE INTERLINE TICKET CARRIER ESV");

  LOG4CXX_DEBUG(_logger, "ItinAnalyzerService::validateInterlineTicketCarrierESV(ShoppingTrx&)");

  bool removeInvalidInterline = true;

  if (Diagnostic900 == trx.diagnostic().diagnosticType())
  {
    Diag900Collector* diag900Collector =
        dynamic_cast<Diag900Collector*>(DCFactory::instance()->create(trx));

    const std::string& displayRemovedSops =
        diag900Collector->rootDiag()->diagParamMapItem("INVALID_INTERLINE");

    if ("Y" == displayRemovedSops)
    {
      removeInvalidInterline = false;
    }
  }

  if (trx.legs().empty())
  {
    return;
  }

  ShoppingTrx::Leg& outboundLeg = trx.legs()[0];
  InterlineTicketCarrier interlineTicketCarrier;

  outboundLeg.sop().erase(
      remove_if(outboundLeg.sop().begin(),
                outboundLeg.sop().end(),
                InterlineCarriersValid(trx, &interlineTicketCarrier, removeInvalidInterline)),
      outboundLeg.sop().end());
}

void
ItinAnalyzerService::collectMultiAirport(ShoppingTrx& trx)
{
  Airports originAirports;
  Airports destinationAirports;

  std::set<FareMarket*> processedFMs;
  for (unsigned legIndex = 0; legIndex < trx.legs().size(); ++legIndex)
  {
    const ShoppingTrx::Leg& leg = trx.legs()[legIndex];
    const ItinIndex::ItinMatrix& itinMatrix = leg.carrierIndex().root();
    for (ItinIndex::ItinMatrix::const_iterator itinMatrixIt = itinMatrix.begin();
         itinMatrixIt != itinMatrix.end();
         ++itinMatrixIt)
    {
      const ItinIndex::Key& carrierKey = itinMatrixIt->first;
      ItinIndex::ItinCell* itinCell =
          ShoppingUtil::retrieveDirectItin(trx, legIndex, carrierKey, ItinIndex::CHECK_NOTHING);
      if (!itinCell || !itinCell->second)
      {
        continue;
      }

      Itin& itin = *itinCell->second;
      for (std::vector<FareMarket*>::const_iterator fareMarketIt = itin.fareMarket().begin();
           fareMarketIt != itin.fareMarket().end();
           ++fareMarketIt, originAirports.clear(), destinationAirports.clear())
      {
        FareMarket& fareMarket = **fareMarketIt;

        if (UNLIKELY(trx.isIataFareSelectionApplicable() &&
                      !processedFMs.insert(&fareMarket).second))
          continue; // already processed

        if (FareMarket::MultiAirportInfo* multiAirportInfo = fareMarket.getMultiAirportInfo())
        {
          collectMultiAirportForFareMarket(trx,
                                           legIndex,
                                           carrierKey,
                                           fareMarket,
                                           multiAirportInfo->origin(),
                                           multiAirportInfo->destination());
        }
        else
        {
          collectMultiAirportForFareMarket(
              trx, legIndex, carrierKey, fareMarket, originAirports, destinationAirports);
          if (originAirports.size() > 1 || destinationAirports.size() > 1)
          {
            FareMarket::MultiAirportInfo* multiAirportInfo =
                trx.dataHandle().create<FareMarket::MultiAirportInfo>();
            multiAirportInfo->origin() = originAirports;
            multiAirportInfo->destination() = destinationAirports;
            fareMarket.setMultiAirportInfo(multiAirportInfo);
          }
        }
      }
    }
  }

  printMultiAirport(trx);
}

void
ItinAnalyzerService::collectMultiAirportForFareMarket(ShoppingTrx& trx,
                                                      unsigned legIndex,
                                                      const ItinIndex::Key& carrierKey,
                                                      const FareMarket& fareMarket,
                                                      Airports& originAirports,
                                                      Airports& destinationAirports)
{
  const SOPUsages* sopUsages = nullptr;
  if (trx.isSumOfLocalsProcessingEnabled() && fareMarket.getApplicableSOPs())
  {
    ApplicableSOP::const_iterator applicableSOPIt =
        fareMarket.getApplicableSOPs()->find(carrierKey);
    if (LIKELY(applicableSOPIt != fareMarket.getApplicableSOPs()->end()))
    {
      sopUsages = &applicableSOPIt->second;
    }
  }

  ShoppingTrx::Leg& leg = trx.legs()[legIndex];
  ItinIndex::ItinIndexIterator itinIndexIt =
      leg.stopOverLegFlag() ? leg.carrierIndex().beginAcrossStopOverRow(trx, legIndex, carrierKey)
                            : leg.carrierIndex().beginRow(carrierKey);
  ItinIndex::ItinIndexIterator itinIndexEndIt = leg.stopOverLegFlag()
                                                    ? leg.carrierIndex().endAcrossStopOverRow()
                                                    : leg.carrierIndex().endRow();

  for (; itinIndexIt != itinIndexEndIt; ++itinIndexIt)
  {
    const Itin& itin = *itinIndexIt->second;
    const uint32_t bitIndex = itinIndexIt.bitIndex();

    const SOPUsage* sopUsage = sopUsages ? &sopUsages->at(bitIndex) : nullptr;
    if (sopUsage && !sopUsage->applicable_)
    {
      continue;
    }

    const TravelSeg* firstTravelSeg;
    const TravelSeg* lastTravelSeg;
    if (sopUsage && sopUsage->startSegment_ != -1 && sopUsage->endSegment_ != -1)
    {
      firstTravelSeg = itin.travelSeg().at(sopUsage->startSegment_);
      lastTravelSeg = itin.travelSeg().at(sopUsage->endSegment_);
    }
    else
    {
      firstTravelSeg = itin.travelSeg().front();
      lastTravelSeg = itin.travelSeg().back();
    }

    const LocCode& origin = firstTravelSeg->origin()->loc();
    const LocCode& destination = lastTravelSeg->destination()->loc();
    if (std::find(originAirports.begin(), originAirports.end(), origin) == originAirports.end())
    {
      originAirports.push_back(origin);
    }

    if (std::find(destinationAirports.begin(), destinationAirports.end(), destination) ==
        destinationAirports.end())
    {
      destinationAirports.push_back(destination);
    }
  }
}

void
ItinAnalyzerService::printMultiAirport(ShoppingTrx& trx)
{
  if (Diagnostic922 != trx.diagnostic().diagnosticType())
  {
    return;
  }

  Diag922Collector* diagnostic =
      dynamic_cast<Diag922Collector*>(DCFactory::instance()->create(trx));
  if (!diagnostic)
  {
    return;
  }

  diagnostic->enable(Diagnostic922);

  *diagnostic << "\n====== Fare Markets Multi-airports for Carriers =======\n\n";
  diagnostic->setf(std::ios::left, std::ios::adjustfield);
  diagnostic->printFareMarketHeader(trx);

  for (unsigned legIndex = 0; legIndex < trx.legs().size(); ++legIndex)
  {
    *diagnostic << "\n====== Leg " << (legIndex + 1) << " ======\n";

    const ShoppingTrx::Leg& leg = trx.legs()[legIndex];
    const ItinIndex::ItinMatrix& itinMatrix = leg.carrierIndex().root();
    for (ItinIndex::ItinMatrix::const_iterator itinMatrixIt = itinMatrix.begin();
         itinMatrixIt != itinMatrix.end();
         ++itinMatrixIt)
    {
      const ItinIndex::Key& key = itinMatrixIt->first;
      const ItinIndex::ItinCell* itinCell =
          ShoppingUtil::retrieveDirectItin(trx, legIndex, key, ItinIndex::CHECK_NOTHING);
      if (itinCell && itinCell->second)
      {
        const Itin& itin = *itinCell->second;
        for (std::vector<FareMarket*>::const_iterator fareMarketIt = itin.fareMarket().begin();
             fareMarketIt != itin.fareMarket().end();
             ++fareMarketIt)
        {
          const FareMarket& fareMarket = **fareMarketIt;
          const FareMarket::MultiAirportInfo* multiAirportInfo = fareMarket.getMultiAirportInfo();
          if (multiAirportInfo)
          {
            diagnostic->printFareMarket(trx, fareMarket);
            diagnostic->printFareMarketMultiAirPort(fareMarket);
            *diagnostic << "===========\n";
          }
        }
      }
    }
  }

  diagnostic->flushMsg();
}
} // tse
