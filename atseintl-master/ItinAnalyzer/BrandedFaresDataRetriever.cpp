//----------------------------------------------------------------------------
//  File: BrandedFaresDataRetriever.h
//
//  Author: Michal Mlynek
//
//  Created:      07/01/2013
//
//  Description: For each fare market computes a ODC ( origin/destination/carrier)
//               tuple and creates a map : ODC -> vactor<FareMarket*>.
//               Then builds, using this map entries, requests to Pricing
//
//  Copyright Sabre 2013
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#include "ItinAnalyzer/BrandedFaresDataRetriever.h"

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/S8BrandedFaresSelector.h"
#include "Common/BrandingUtil.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/ItinUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/ExcItin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "Diagnostic/BrandedDiagnosticUtil.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag892Collector.h"
#include "Diagnostic/Diag894Collector.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "ItinAnalyzer/ItinAnalyzerUtils.h"

namespace tse
{
FALLBACK_DECL(fallbackBrandedServiceInterface);
FALLBACK_DECL(fallbackFixBrandsOriginBasedPricing)
FALLBACK_DECL(fallbackSendDirectionToMM)

namespace
{
ConfigurableValue<bool>
shouldYYFMsBeIgnored("S8_BRAND_SVC", "IGNORE_YY_FMS_IN_BRANDING", false);
}

Logger
BrandedFaresDataRetriever::_logger("atseintl.ItinAnalyzer.ItinAnalyzerService");
std::string BrandedFaresDataRetriever::_ignoredYYFareMarketsMsg =
    "\nYY FARE MARKETS NOT SENT TO BRANDING.\nIGNORE_YY_FMS_IN_BRANDING SET TO TRUE IN CONFIG "
    "FILE\n\n";

BrandedFaresDataRetriever::BrandedFaresDataRetriever(PricingTrx& trx,
                                                     BrandRetrievalMode mode,
                                                     Diag892Collector* diag892,
                                                     Diag894Collector* diag894)
  : _trx(trx),
    _brandRetrievalMode(mode),
    _diag892(diag892),
    _diag894(diag894),
    _diag901(trx),
    _requestResponseHandler(trx)
{
  _isMip = (trx.getTrxType() == PricingTrx::MIP_TRX);
  _isCatchAllBucket = (trx.getRequest()->isCatchAllBucketRequest());
  _paxTypes = PaxTypeUtil::retrievePaxTypes(trx);
  _tripType = IAIbfUtils::calculateTripType(trx);
  // At the time of writing this project there were no YY fares filed in S8
  // so it was useless to use FMs with YY as governing carrier in brand retrieval
  _shouldYYFMsBeIgnored = shouldYYFMsBeIgnored.getValue();
}

BrandedFaresDataRetriever::~BrandedFaresDataRetriever()
{
  flushDiagnostics();
}

void
BrandedFaresDataRetriever::flushBrandedDiagnostics(DiagCollector* diag)
{
  BrandedDiagnosticUtil::displayBrandRetrievalMode(*diag, _brandRetrievalMode);
  BrandedDiagnosticUtil::displayTripType(*diag, _tripType);
  if (_shouldYYFMsBeIgnored)
    *diag << _ignoredYYFareMarketsMsg;
  displayFMsAndODCMatching();
  diag->flushMsg();
}

void
BrandedFaresDataRetriever::flushDiagnostics()
{
  if (UNLIKELY(_diag892))
    flushBrandedDiagnostics(_diag892);

  if (UNLIKELY(_diag894))
    flushBrandedDiagnostics(_diag894);

  if (_diag901.isActive())
  {
    if (_shouldYYFMsBeIgnored)
      _diag901 << _ignoredYYFareMarketsMsg;
    _diag901.flush(ITIN_ANALYZER);
  }
}

// Function returns the status of transaction ( success vs failure )
bool
BrandedFaresDataRetriever::process()
{
  TSELatencyData metrics(_trx, "S8 BRAND PROCESS");

  if (!buildODCWithFareMarketsMap())
    return false;

  if (!retrieveBrandedFares())
    return false;

  if (!fallback::fallbackFixBrandsOriginBasedPricing(&_trx) &&
      _trx.getRequest()->originBasedRTPricing())
    fillDummyFMWithBrands();

  if (!_isMip && !_trx.activationFlags().isSearchForBrandsPricing())
    return true;

  bool processIfNoBrands = _isCatchAllBucket || _trx.isBrandsForTnShopping() ||
                           _trx.activationFlags().isSearchForBrandsPricing();

  if (_trx.brandProgramVec().empty() && !processIfNoBrands)
    return false;

  return true;
}

void
BrandedFaresDataRetriever::fillDummyFMWithBrands()
{
  if (!_trx.getRequest()->originBasedRTPricing())
    return;

  if (_trx.brandProgramVec().empty())
    return;

  // initialize brand index vector with all possible brands
  std::vector<int> allBrandsVector(_trx.brandProgramVec().size());
  for (size_t index = 0; index < allBrandsVector.size(); ++index)
    allBrandsVector[index] = index;

  for (Itin* itin : _trx.itin())
  {
    for (FareMarket* fm : itin->fareMarket())
    {
      // assign all brands vector to all dummy fare markets
      if (fm->useDummyFare())
        fm->brandProgramIndexVec() = allBrandsVector;
    }
  }
}

bool
BrandedFaresDataRetriever::buildODCWithFareMarketsMap()
{
  if (_isMip || _trx.getTrxType() == PricingTrx::PRICING_TRX)
    buildODCWithFareMarketsMapMIP();
  else
    buildODCWithFareMarketsMapIS();

  return !_fMsForBranding.empty();
}

void
BrandedFaresDataRetriever::buildODCWithFareMarketsMapMIP()
{
  if (_trx.itin().empty() || _trx.fareMarket().empty())
    return;

  std::set<FareMarket*> processedFMs;

  const bool isOriginBasedRTPricing = !fallback::fallbackFixBrandsOriginBasedPricing(&_trx) &&
    _trx.getRequest()->originBasedRTPricing();
  for (Itin* itin : _trx.itin())
  {
    itinanalyzerutils::setItinLegs(itin);

    for (FareMarket* fm : itin->fareMarket())
    {
      if (fm->travelSeg().empty() || (processedFMs.find(fm) != processedFMs.end()))
        continue;

      if (_shouldYYFMsBeIgnored && fm->governingCarrier().equalToConst("YY"))
        continue;

      if (isOriginBasedRTPricing && fm->useDummyFare())
        continue;

      IAIbfUtils::findAllOdDataForMarketPricing(itin, fm, _tripType, _brandRetrievalMode, _trx,
          [&](IAIbfUtils::OdcTuple& odcTuple){ _fMsForBranding[odcTuple].push_back(fm); });

      processedFMs.insert(fm);
    }
  }

  RexBaseTrx* rexTrx = dynamic_cast<RexBaseTrx*>(&_trx);

  if (rexTrx)
  {
    ExcItin* excItin = rexTrx->exchangeItin().front();
    itinanalyzerutils::setItinLegs(excItin);

    for (FareMarket* fm : excItin->fareMarket())
    {
      IAIbfUtils::findAllOdDataForMarketPricing(excItin, fm, _tripType, _brandRetrievalMode, _trx,
          [&](IAIbfUtils::OdcTuple& odcTuple){ _fMsForBranding[odcTuple].push_back(fm); });
    }
  }
}

void
BrandedFaresDataRetriever::buildODCWithFareMarketsMapIS()
{
  ShoppingTrx& trx = dynamic_cast<ShoppingTrx&>(_trx);

  for (unsigned legIndex = 0; legIndex < trx.legs().size(); ++legIndex)
  {
    const ShoppingTrx::Leg& leg = trx.legs()[legIndex];

    const ItinIndex& cxrIdx = leg.carrierIndex();
    const ItinIndex::ItinMatrix& itinMatrix = cxrIdx.root();
    // for each carrier in ItinMatrix on this leg
    for (const auto& itinMatrixIt : itinMatrix)
    {
      const ItinIndex::Key& carrierKey = itinMatrixIt.first;
      // retrieve the first itin. in IS there all fare markets are either leg's thru markets
      // or are within a leg local markets in NGS). In both cases O&D for brand retrieval will match leg.
      const ItinIndex::ItinCell* itinCell =
          ShoppingUtil::retrieveDirectItin(cxrIdx, carrierKey, ItinIndex::CHECK_NOTHING);

      if (!itinCell || !itinCell->second)
        continue;

      const Itin& itin = *itinCell->second;

      if (itin.fareMarket().empty())
        continue;

      // We're taking govCarrier from the first fm as this whole iteration is for one carrier key
      const CarrierCode& govCarrier = itin.fareMarket().front()->governingCarrier();

      if (_shouldYYFMsBeIgnored && govCarrier.equalToConst("YY"))
        continue;

      OdDataForFareMarket fmOdData;
      IAIbfUtils::fillOdDataForFareMarketShopping(fmOdData, itin, *(trx.journeyItin()));

      std::vector<OdcTuple> odcTupleVec;
      IAIbfUtils::fillOdcTupleVec(
          odcTupleVec, fmOdData, govCarrier, _tripType, _brandRetrievalMode);

      for (FareMarket* fm : itin.fareMarket())
        insertOdcTupleVecIntoMap(odcTupleVec, fm);
    }
  }
}

void
BrandedFaresDataRetriever::insertOdcTupleVecIntoMap(const std::vector<OdcTuple>& odcTupleVec,
                                                    FareMarket* fm)
{
  for (const OdcTuple& odcTuple : odcTupleVec)
  {
    _fMsForBranding[odcTuple].push_back(fm);
  }
}

bool
BrandedFaresDataRetriever::retrieveBrandedFares()
{
  if (fallback::fallbackBrandedServiceInterface(&_trx))
    _requestResponseHandler.setClientId(BR_CLIENT_SHOPPING);
  else
  {
    _requestResponseHandler.setClientId(BR_CLIENT_MIP);
    _requestResponseHandler.setActionCode(BR_ACTION_SHOPPING);
  }

  AlphaCode direction = "";
  if (!fallback::fallbackSendDirectionToMM(&_trx))
    direction = "OT";

  for (const FMsForBrandingPair& item : _fMsForBranding)
  {

    _requestResponseHandler.buildMarketRequest(
        item.first.origin,
        item.first.destination,
        _trx.travelDate(),
        _paxTypes,
        std::vector<CarrierCode>(1, item.first.governingCarrier),
        item.second,
        GlobalDirection::NO_DIR,
        direction);

    if (_diag901.isActive()) // Diag 901 is IS only
    {
      _diag901.collectDataSentToPricing(item.first, item.second.size(), _paxTypes);
    }
  }

  return _requestResponseHandler.getBrandedFares();
}

void
BrandedFaresDataRetriever::displayFMsAndODCMatching()
{
  if (LIKELY(!_diag892 && !_diag894))
    return;

  bool showOnd = false;
  DiagCollector* diag = nullptr;

  if (_diag894)
  {
    showOnd = true;
    diag = _diag894;
  }
  else
  {
    diag = _diag892;
  }
  BrandedDiagnosticUtil::displayAllBrandIndices(*diag, _trx.brandProgramVec(), showOnd);
  if (_diag892)
  {
    *diag << "\n*** PROCESSING BRAND PARITY AND FILTERING ***\n\n"
          << "FARE MARKET BRANDS BEFORE PROCESSING ALL ITINS:\n";
  }
  OdcsForBranding invertedMap;
  IAIbfUtils::invertOdcToFmMap(_fMsForBranding, invertedMap);
  BrandedDiagnosticUtil::displayFareMarketsWithBrands(*diag, _trx.fareMarket(),
                                                      _trx.brandProgramVec(), &invertedMap,
                                                      showOnd);
}
}
