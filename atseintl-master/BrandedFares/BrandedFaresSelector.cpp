//-------------------------------------------------------------------
//
//  File:        S8BrandedFaresSelector.cpp
//  Created:     2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
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

#include "BrandedFares/BrandedFaresSelector.h"

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/MarketCriteria.h"
#include "BrandedFares/MarketResponse.h"
#include "BrandedFares/S8BrandedFaresSelector.h"
#include "Common/BrandingUtil.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Common/Vendor.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/RexExchangeTrx.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/SvcFeesFareIdInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag889Collector.h"
#include "Diagnostic/Diag894Collector.h"
#include "Rules/RuleUtil.h"


namespace tse
{
FALLBACK_DECL(fallbackSkipDirectionalityCheckInFQ)
FALLBACK_DECL(fallbackProperDirectionalityValidationInFQ)

Logger
BrandedFaresSelector::_logger("atseintl.BrandedFares.BrandedFaresSelector");

namespace
{
ConfigurableValue<bool>
ignoreYyFaresInBranding("S8_BRAND_SVC", "IGNORE_YY_FARES_IN_BRANDING", false);
}

// This method supposed to be called for FQ only.
// This method goes through vector of PaxTypeFare and validates them again Table 189.
// Pricing and Shopping should not call this method.

void
BrandedFaresSelector::validate(std::vector<PaxTypeFare*>& fares)
{
  if (_trx.getTrxType() != PricingTrx::FAREDISPLAY_TRX)
    return;

  BrandedFareDiagnostics diagnostics(_trx, _diag889);

  diagnostics.printT189Banner();

  if (_trx.brandedMarketMap().empty())
  {
    diagnostics.printDataNotFound();
    return;
  }

  bool hasPrintedPaxTypeFareCommonHeader = false;

  for (PaxTypeFare* fare : fares)
  {

    if (fare->fare() == nullptr)
    {
      LOG4CXX_ERROR(_logger, "S8BrandedFaresSelector::validate PaxTypeFare Fare is NULL");
      continue;
    }

    if (!matchFareMarketForDiagParam(*fare->fareMarket()))
    {
      diagnostics.printFareMarketNotMatched(*fare->fareMarket());
      return;
    }

    if (fare->fareMarket()->marketIDVec().empty())
    {
      LOG4CXX_ERROR(_logger, "S8BrandedFaresSelector::validate marketIDVec is empty");
      diagnostics.printT189NotExist(*fare->fareMarket());
      return;
    }

    if (!isT189exist(fare->fareMarket()->marketIDVec().front()))
    {
      diagnostics.printT189NotExist(*fare->fareMarket());
      return;
    }

    if (!diagnostics.isForFareClassCode(fare->fareClass()))
      continue;

    diagnostics.printHeaderInfoSeparator(fare->fareMarket(), hasPrintedPaxTypeFareCommonHeader);
    validatePaxTypeFare(fare, fare->fareMarket()->marketIDVec().front(), diagnostics);
  }
}

// This method supposed to be called by Pricing and Shopping only.
// This method goes through all PaxTypeFare in the FareMarket and validates them again Table 189.

void
BrandedFaresSelector::validate(FareMarket& fareMarket)
{
  BrandedFareDiagnostics diagnostics(_trx, _diag889);
  if (!matchFareMarketForDiagParam(fareMarket))
  {
    return;
  }

  if (fareMarket.allPaxTypeFare().empty())
  {
    diagnostics.printT189Banner();
    diagnostics.printNoFaresFound(fareMarket);
    return;
  }

  if (_trx.brandedMarketMap().empty())
  {
    diagnostics.printT189Banner();
    diagnostics.printDataNotFound(fareMarket);
    return;
  }

  if (!diagnostics.isForCarrier(fareMarket.governingCarrier()))
  {
    diagnostics.printT189Banner();
    diagnostics.printCarrierNotMatched(fareMarket);
    return;
  }

  if (fareMarket.brandProgramIndexVec().empty())
  {
    diagnostics.printT189Banner();
    diagnostics.printT189NotExist(fareMarket);
    return;
  }

  if (_trx.getTnShoppingBrandingMode() == TnShoppingBrandingMode::SINGLE_BRAND)
  {
    // Because trx _brandProgramVec is filled with brands data our "generic"
    // code expect that all fares will also contain brands information.
    // We have to fake it for now, as we don't use it anyway until post pricing.
    for (PaxTypeFare* fare : fareMarket.allPaxTypeFare())
    {
      if (LIKELY(fare->fare() != nullptr))
      {
        fare->getMutableBrandStatusVec().assign(
            fare->fareMarket()->brandProgramIndexVec().size(),
            std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));
      }
    }
    return;
  }

  DCFactory* diagFactory = DCFactory::instance();
  Diag894Collector* diag894 = dynamic_cast<Diag894Collector*>(diagFactory->create(_trx));
  if (UNLIKELY(diag894 != nullptr))
    diag894->enable(Diagnostic894);

  diagnostics.printT189Banner();
  bool hasPrintedPaxTypeFareCommonHeader = false;
  for (PaxTypeFare* fare : fareMarket.allPaxTypeFare())
  {
    if (fare->fare() == nullptr)
    {
      LOG4CXX_ERROR(_logger, "S8BrandedFaresSelector::validate FareMarket Fare is NULL");
      continue;
    }

    if (!diagnostics.isForFareClassCode(fare->fareClass()))
      continue;

    diagnostics.printHeaderInfoSeparator(fare->fareMarket(), hasPrintedPaxTypeFareCommonHeader);
    displayBrandProgramMapSize(fare, fareMarket, diagnostics);

    processPaxTypeFare(fare, diagnostics, diag894);
  }

  if (UNLIKELY(diag894 != nullptr))
    diag894->flushMsg();
}

void
BrandedFaresSelector::brandFares(std::set<PaxTypeFare*>& faresToBrand)
{
  if (_trx.getTnShoppingBrandingMode() == TnShoppingBrandingMode::SINGLE_BRAND)
  {
    DCFactory* diagFactory = DCFactory::instance();
    Diag894Collector* diag894 = dynamic_cast<Diag894Collector*>(diagFactory->create(_trx));
    if (UNLIKELY(diag894 != nullptr))
      diag894->enable(Diagnostic894);
    BrandedFareDiagnostics diagnostics(_trx, _diag889);
    diagnostics.printT189Banner();
    bool hasPrintedPaxTypeFareCommonHeader = false;
    for (PaxTypeFare* fare : faresToBrand)
    {
      if (fare->fare() == nullptr)
      {
        LOG4CXX_ERROR(_logger, "BrandedFaresSelector::validate FareMarket Fare is NULL");
        continue;
      }
      TSE_ASSERT(fare->fareMarket() != nullptr);
      diagnostics.printHeaderInfoSeparator(fare->fareMarket(), hasPrintedPaxTypeFareCommonHeader);
      displayBrandProgramMapSize(fare, *(fare->fareMarket()), diagnostics);
      processPaxTypeFare(fare, diagnostics, diag894);
    }
    if (UNLIKELY(diag894 != nullptr))
      diag894->flushMsg();
  }
}

bool
BrandedFaresSelector::validatePaxTypeFare(PaxTypeFare* paxTypeFare, int marketID, BrandedFareDiagnostics& diagnostics)
{

  diagnostics.printPaxTypeFare(paxTypeFare);

  PricingTrx::BrandedMarketMap::iterator marketResponseMapIter =
      _trx.brandedMarketMap().find(marketID);
  if (marketResponseMapIter == _trx.brandedMarketMap().end())
  {
    return false;
  }

  uint16_t brandSize = _trx.brandProgramVec().size();
  paxTypeFare->getMutableBrandStatusVec().assign(brandSize,
      std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));

  std::vector<MarketResponse*>& marketResp = marketResponseMapIter->second;

  for (auto mR : marketResp)
  {
    if (mR == nullptr)
    {
      LOG4CXX_ERROR(_logger, "BrandedFaresSelector::validatePaxTypeFare MarketResponse is NULL");
      continue;
    }

    if (!diagnostics.isForCarrier(mR->carrier()))
      continue;

    if (mR->brandPrograms().empty())
    {
      diagnostics.printBrandProgramNotFound();
      continue;
    }

    if (getBrandData(paxTypeFare, mR->brandPrograms(), diagnostics))
    {
      return true;
    }
  }

  return false;
}

bool
BrandedFaresSelector::isFMforExchange(const FareMarket& fareMarket) const
{
  return fareMarket.fareCompInfo() != nullptr;
}

BrandCode
BrandedFaresSelector::getBrandFromExcFM(const PaxTypeFare& ptf) const
{
  return ptf.fareMarket()->travelSeg().front()->getBrandCode();
}

//PaxTypeFare validation called for FareQuota requests
bool
BrandedFaresSelector::getBrandData(PaxTypeFare* paxTypeFare,
                                   const std::vector<BrandProgram*>& brandPrograms, BrandedFareDiagnostics& diagnostics)
{
  std::vector<BrandProgram*>::const_iterator bPr = brandPrograms.begin(); // S8 vector
  bool needBrandProgramSeparator = false;
  bool rc = false;

  diagnostics.printBrandProgramSize(brandPrograms.size());

  for (; bPr != brandPrograms.end(); ++bPr)
  {
    BrandProgram* brandPr = *bPr;

    if (brandPr == nullptr)
    {
      LOG4CXX_ERROR(_logger, "BrandedFaresSelector::getBrandData BrandProgram is NULL");
      continue;
    }

    if (!diagnostics.isForVendor(brandPr->vendorCode()) ||
        !diagnostics.isForProgramName(brandPr->programCode()))
      continue;

    diagnostics.printProgram(brandPr, needBrandProgramSeparator);

    if (brandPr->brandsData().empty())
    {
      diagnostics.printBrandNotFound();
      continue;
    }

    if (!matchGlobalDirection(*brandPr, *paxTypeFare))
    {
      diagnostics.printProgramFailGlobalDirection(*brandPr, *paxTypeFare);
      continue;
    }

    Direction programDirection = Direction::BOTHWAYS;
    if (!fallback::fallbackProperDirectionalityValidationInFQ(&_trx))
    {
      brandPr->calculateDirectionality(*(paxTypeFare->fareMarket()),
                                       programDirection,
                                       nullptr);
    }

    if (fallback::fallbackSkipDirectionalityCheckInFQ(&_trx))
    {
      if (!matchDirectionality(*brandPr, *paxTypeFare, programDirection))
      {
        diagnostics.printProgramFailDirectionality(*brandPr, *paxTypeFare, programDirection);
        continue;
      }
    }

    BrandProgram::BrandsData::const_iterator brI = brandPr->brandsData().begin();
    bool needBrandSeparator = false;

    diagnostics.printBrandSize(brandPr->brandsData().size());

    for (; brI != brandPr->brandsData().end(); ++brI)
    {
      BrandInfo* brand = *brI;

      if (brand == nullptr)
      {
        LOG4CXX_ERROR(_logger, "BrandedFaresSelector::getBrandData Brand is NULL");
        continue;
      }

      if (!diagnostics.isForBrandId(brand->brandCode()))
        continue;

      diagnostics.printDetailBrand(brand);

      int brandIndex =
          ShoppingUtil::getRequestedBrandIndex(_trx, brandPr->programID(), brand->brandCode());

      PaxTypeFare::BrandStatus brandStatus = validateFare(paxTypeFare,
                                                          brandPr,
                                                          brand,
                                                          needBrandSeparator,
                                                          diagnostics,
                                                          brandIndex);
      if (brandStatus == PaxTypeFare::BS_HARD_PASS)
      {
        // TODO(andrzej.fediuk) FQ: this code is called only for FQ requests, so no checks needed
        // just return true
        rc = true;
        if (!(_trx.getTrxType() == PricingTrx::MIP_TRX || _trx.getTrxType() == PricingTrx::IS_TRX))
        {
          return rc;
        }
      }
    }
  }
  return rc;
}

bool
BrandedFaresSelector::matchGlobalDirection(const BrandProgram& brandPr, const PaxTypeFare& paxTF)
{
  if (brandPr.globalDirection() == GlobalDirection::NO_DIR)
    return true;

  const std::string* prBrGlobalDir = globalDirectionToStr(brandPr.globalDirection());
  if (prBrGlobalDir == nullptr)
    return false;
  if (!(*prBrGlobalDir).empty() && paxTF.globalDirection() != GlobalDirection::ZZ &&
      paxTF.globalDirection() != brandPr.globalDirection())
    return false;

  return true;
}

// should be removed with fallbackBrandDirectionality
bool
BrandedFaresSelector::matchDirectionalityShopping(const LocCode& originLoc,
                                                    const PaxTypeFare& paxTF)
{
  const FareMarket& fm = *(paxTF.fareMarket());

  // return true even if only one brand market's direction matches the fare's direction.
  for (const int& marketId : fm.marketIDVec())
  {
    PricingTrx::BrandedMarketMap::const_iterator i = _trx.brandedMarketMap().find(marketId);
    TSE_ASSERT(i != _trx.brandedMarketMap().end());

    const std::vector<MarketResponse*>& mkts = i->second;

    for (MarketResponse* mkt : mkts)
    {
      if (paxTF.directionality() == FROM &&
          mkt->marketCriteria()->departureAirportCode() == originLoc)
        return true;

      if (paxTF.directionality() == TO && mkt->marketCriteria()->arrivalAirportCode() == originLoc)
        return true;
    }
  }

  return false;
}

bool
BrandedFaresSelector::matchDirectionality(const BrandProgram& brandPr, const PaxTypeFare& paxTF,
  Direction programDirection)
{
  if (BrandingUtil::isDirectionalityToBeUsed(_trx))
    return paxTF.isFareAndProgramDirectionConsistent(programDirection);
  else
  {
    if (paxTF.directionality() == BOTH || brandPr.originLoc().empty())
      return true;

    if (_trx.getTrxType() == PricingTrx::MIP_TRX || _trx.getTrxType() == PricingTrx::IS_TRX)
      return matchDirectionalityShopping(brandPr.originLoc(), paxTF);
  }

  if (paxTF.directionality() == FROM &&
      paxTF.fareMarket()->travelSeg().front()->origAirport() != brandPr.originLoc())
    return false;

  if (paxTF.directionality() == TO &&
      paxTF.fareMarket()->travelSeg().back()->destAirport() != brandPr.originLoc())
    return false;

  return true;
}

PaxTypeFare::BrandStatus
BrandedFaresSelector::validateFare(PaxTypeFare* paxTypeFare,
                                     BrandProgram* brandPr,
                                     BrandInfo* brand,
                                     bool& needBrandSeparator,
                                     BrandedFareDiagnostics& diagnostics,
                                     int brandProgramIndex,
                                     bool skipHardPassValidation)
{
  PaxTypeFare::BrandStatus status =
    _brandSelectorFactory.getValidator(brandPr->dataSource()).validateFare(
        paxTypeFare, brandPr, brand, needBrandSeparator, diagnostics, skipHardPassValidation);
  if(status != PaxTypeFare::BS_FAIL)
  {
    paxTypeFare->getMutableBrandStatusVec()[brandProgramIndex].first = status;
    //brand direction (.second) is updated outside this function, based on
    //returned value and directionality of program and fare

    if (status == PaxTypeFare::BS_HARD_PASS)
    {
      FareDisplayInfo* fdInfo = paxTypeFare->fareDisplayInfo();
      if (fdInfo && brandPr && brand)
      {
        fdInfo->setProgramBrand(make_pair(brandPr->programCode(), brand->brandCode()));
        fdInfo->setBrandProgramPair(std::make_pair(brandPr, brand));
      }
    }
  }
  return status;
}

bool
BrandedFaresSelector::matchFareMarketForDiagParam(const FareMarket& fareMarket)
{
  const std::string& diagFareMarket = _trx.diagnostic().diagParamMapItem(Diagnostic::FARE_MARKET);

  if (_diag889 && !diagFareMarket.empty())
  {
    LocCode boardCity = diagFareMarket.substr(0, 3);
    LocCode offCity = diagFareMarket.substr(3, 3);

    if (((fareMarket.origin()->loc() != boardCity) && (fareMarket.boardMultiCity() != boardCity)) ||
        ((fareMarket.destination()->loc() != offCity) && (fareMarket.offMultiCity() != offCity)))
      return false;
  }

  return true;
}

bool
BrandedFaresSelector::isT189exist(int marketID)
{
  PricingTrx::BrandedMarketMap::iterator marketResponseMapBeg =
      _trx.brandedMarketMap().find(marketID);
  return (marketResponseMapBeg != _trx.brandedMarketMap().end());
}

void
BrandedFaresSelector::displayBrandProgramMapSize(const PaxTypeFare* paxTypeFare, const FareMarket& fareMarket,
                                                 BrandedFareDiagnostics& diagnostics) const
{
  diagnostics.printPaxTypeFare(paxTypeFare);
  if (diagnostics.isDiag889Enabled() && diagnostics.isDDInfo())
  {
    std::map<BrandProgram*, std::vector<BrandInfo*>, BrandProgramComparator> brandProgramMap;
    createBrandProgramMap(fareMarket, brandProgramMap);
    diagnostics.printBrandProgramSize(brandProgramMap.size());
  }
}

namespace {
  //TODO(andrzej.fediuk) DIR: After directionality is tested and accepted move this
  //to BrandProgram class.
  void storeDirectionInformation(BrandProgram& program,
    const std::vector<TravelSeg*>& travelSegments, const Direction programDirection)
  {
    BrandProgram::LegToDirectionMap& legMap = program.getMutableLegToDirectionMap();
    std::set<BrandProgram::OnDPair>& ondpairs = program.getMutableAvailabilityOnOnD();
    for (const TravelSeg* travelSeg: travelSegments)
    {
      const AirSeg* airSeg = travelSeg->toAirSeg();
      if (airSeg == nullptr)
        continue;
      int16_t legId = travelSeg->legId();
      if ((legMap.find(legId) != legMap.end()) && (legMap.at(legId) != programDirection))
        legMap[legId] = Direction::BOTHWAYS;
      else
        legMap[legId] = programDirection;
      ondpairs.insert(std::make_pair(travelSeg->boardMultiCity(), travelSeg->offMultiCity()));
    }
  }
}

//PaxTypeFare validation called for Shopping and Pricing requests
bool
BrandedFaresSelector::processPaxTypeFare(PaxTypeFare* paxTypeFare,
  BrandedFareDiagnostics& diagnostics, Diag894Collector* diag894)
{
  bool gotHardPass = false;
  bool needBrandProgramSeparator = false;
  bool needBrandSeparator = false;
  bool useDiag894 = false;

  const bool isSoftPassDisabled = _trx.isSoftPassDisabled();

  if (UNLIKELY(diag894 != nullptr))
  {
    if (diag894->isActive())
      useDiag894 = true;
  }

  uint16_t fmBrandSize = paxTypeFare->fareMarket()->brandProgramIndexVec().size();
  paxTypeFare->getMutableBrandStatusVec().assign(fmBrandSize,
      std::make_pair(PaxTypeFare::BS_FAIL, Direction::BOTHWAYS));

  const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(_trx);

  if (paxTypeFare->carrier().equalToConst("YY") && _ignoreYyFares)
  {
    diagnostics.printYyFareIgnored();
    return false;
  }

  if (UNLIKELY(useDiag894 && useDirectionality))
  {
    if (!diag894->isValidForPaxTypeFare(_trx, *paxTypeFare))
      useDiag894 = false;
    else
      diag894->printPaxTypeFarInfo(*paxTypeFare);
  }

  //program directionality to avoid recalculations
  std::map<ProgramID, std::pair<bool, Direction>> programsDirection;

  //hard pass status for each direction (original, reversed)
  std::pair<bool, bool> brandDirectionHardPass = std::make_pair(false, false);

  //soft pass status (Pricing only)
  bool anySoftPass = false;

  for (uint16_t brandProgramIndex = 0; brandProgramIndex < fmBrandSize; brandProgramIndex++)
  {
    bool useDiag894ForBrand = false;
    std::size_t brandProgramVecIndex = paxTypeFare->fareMarket()->brandProgramIndexVec()[brandProgramIndex];

    if(!(brandProgramVecIndex < _trx.brandProgramVec().size()))
    {
      LOG4CXX_ERROR(_logger,
                    "S8BrandedFaresSelector::processPaxTypeFare BrandProgramIndex out of range");
      return gotHardPass;
    }
    const QualifiedBrand& brandProgramPair = _trx.brandProgramVec()[brandProgramVecIndex];

    if(isFMforExchange(*paxTypeFare->fareMarket()) &&
       brandProgramPair.second->brandCode() != getBrandFromExcFM(*paxTypeFare))
    {
      if (_trx.getTrxType() == PricingTrx::MIP_TRX)
      {
        RexExchangeTrx* rexTrx = static_cast<RexExchangeTrx*>(&_trx);
        if (rexTrx->trxPhase() != RexPricingTrx::PRICE_NEWITIN_PHASE &&
            !rexTrx->getKeepOriginal())
        {
          continue;
        }
      }
      else if(isFMforExchange(*paxTypeFare->fareMarket()) &&
              brandProgramPair.second->brandCode() != getBrandFromExcFM(*paxTypeFare))
      {
        continue;
      }
    }

    if(!diagnostics.isForVendor(brandProgramPair.first->vendorCode()) ||
       !diagnostics.isForProgramName(brandProgramPair.first->programCode()))
      continue;

    diagnostics.printProgram(brandProgramPair.first, needBrandProgramSeparator);

    if (brandProgramPair.first->brandsData().empty())
    {
      diagnostics.printBrandNotFound();
      continue;
    }

    diagnostics.printCurrentBrand(brandProgramPair.second, brandProgramPair.first->brandsData().size());

    if(!diagnostics.isForBrandId(brandProgramPair.second->brandCode()))
      continue;

    diagnostics.printDetailBrand(brandProgramPair.second);
    if (!matchGlobalDirection(*(brandProgramPair.first), *paxTypeFare))
    {
      diagnostics.printProgramFailGlobalDirection(*(brandProgramPair.first), *paxTypeFare);
      continue;
    }

    Direction programDirection = Direction::BOTHWAYS;
    bool directionalityCalculated = true;
    if (useDirectionality)
    {
      TSE_ASSERT(paxTypeFare->fareMarket() != 0);
      std::map<ProgramID, std::pair<bool, Direction>>::iterator currentProgramDirection =
          programsDirection.find(brandProgramPair.first->programID());
      if (currentProgramDirection != programsDirection.end())
      {
        directionalityCalculated = currentProgramDirection->second.first;
        programDirection = currentProgramDirection->second.second;;
      }
      else
      {
        directionalityCalculated =
            brandProgramPair.first->calculateDirectionality(
                                                  *(paxTypeFare->fareMarket()),
                                                  programDirection,
                                                  (useDiag894 ? diag894 : nullptr));
        programsDirection[brandProgramPair.first->programID()] =
            std::make_pair(directionalityCalculated, programDirection);
      }

      if (!directionalityCalculated)
        continue;

      paxTypeFare->getMutableBrandStatusVec()[brandProgramIndex].second = programDirection;

      if (UNLIKELY(useDiag894))
      {
        const std::string& diagBrandCode = diag894->getBrandCode();
        if ((diagBrandCode == "ALL") || (brandProgramPair.second->brandCode() == diagBrandCode))
        {
          useDiag894ForBrand = true;
          diag894->printQualifiedBrandInfo(brandProgramVecIndex, brandProgramPair);
        }
      }

      storeDirectionInformation(*(brandProgramPair.first),
                                paxTypeFare->fareMarket()->travelSeg(),
                                programDirection);
    }

    if (!matchDirectionality(*(brandProgramPair.first), *paxTypeFare, programDirection))
    {
      if (UNLIKELY(useDiag894 && useDiag894ForBrand && useDirectionality))
        diag894->printProgramToFareMarketDirectionMatch(false);
      diagnostics.printProgramFailDirectionality(
          *(brandProgramPair.first), *paxTypeFare, programDirection);
      continue;
    }
    else if (UNLIKELY(useDiag894 && useDiag894ForBrand && useDirectionality))
      diag894->printProgramToFareMarketDirectionMatch(true);

    if (useDirectionality)
    {
      gotHardPass = (programDirection == Direction::REVERSED ?
          brandDirectionHardPass.second : brandDirectionHardPass.first);
    }
    else
      gotHardPass = (brandDirectionHardPass.first || brandDirectionHardPass.second);

    if (useDirectionality && gotHardPass && isSoftPassDisabled)
    {
      if (UNLIKELY(useDiag894ForBrand))
        diag894->printFareValidationStatusHotHardPass();
      continue;
    }

    PaxTypeFare::BrandStatus status = validateFare(paxTypeFare,
                                                   brandProgramPair.first,
                                                   brandProgramPair.second,
                                                   needBrandSeparator,
                                                   diagnostics,
                                                   brandProgramIndex,
                                                   gotHardPass);
    if (status != PaxTypeFare::BS_FAIL)
    {
      if (UNLIKELY(useDiag894 && useDiag894ForBrand && useDirectionality))
        diag894->printFareValidationStatus(status);
      if (useDirectionality)
      {
        if (programDirection == Direction::BOTHWAYS ||
            programDirection == Direction::ORIGINAL)
        {
          if (status == PaxTypeFare::BS_HARD_PASS)
            brandDirectionHardPass.first = true;
        }
        if (programDirection == Direction::BOTHWAYS ||
            programDirection == Direction::REVERSED)
        {
          if (status == PaxTypeFare::BS_HARD_PASS)
            brandDirectionHardPass.second = true;
        }
      }
      else
      {
        // when brand directionality is not used first hard pass should trigger
        // validating rest of the brands as soft passes
        if (status == PaxTypeFare::BS_HARD_PASS)
        {
          brandDirectionHardPass.first = true;
          brandDirectionHardPass.second = true;
        }
      }
      if (status == PaxTypeFare::BS_SOFT_PASS)
        anySoftPass = true;

      //if BFA/EXPEDIA and soft passes disabled end after first valid brand found
      if (isSoftPassDisabled)
      {
        if (status == PaxTypeFare::BS_SOFT_PASS)
        {
          paxTypeFare->getMutableBrandStatusVec()[brandProgramIndex].first = PaxTypeFare::BS_FAIL;
          if (UNLIKELY(useDiag894 && useDiag894ForBrand && useDirectionality))
            diag894->printFareValidationStatus(PaxTypeFare::BS_FAIL);
        }

        if (brandDirectionHardPass.first && brandDirectionHardPass.second)
          return true;
      }

      if (_trx.getTrxType() != PricingTrx::MIP_TRX
          && _trx.getTrxType() != PricingTrx::IS_TRX
          && !_trx.activationFlags().isSearchForBrandsPricing())
      {
        //TODO(andrzej.fediuk) is other TRX type possible here?
        //?: REPRICING_TRX, ESV_TRX, FF_TRX or RESHOP_TRX
        //no: FAREDISPLAY_TRX (never executes this code)
        if (_trx.getTrxType() != PricingTrx::PRICING_TRX)
          return true;
      }
    }
    else if (UNLIKELY(useDiag894 && useDiag894ForBrand && useDirectionality))
      diag894->printFareValidationStatus(PaxTypeFare::BS_FAIL);
  }

  if (UNLIKELY(useDiag894 && useDirectionality))
    diag894->lineSkip(0);
  if (brandDirectionHardPass.first || brandDirectionHardPass.second)
    return true;

  if (!isSoftPassDisabled &&
      (_trx.getTrxType() == PricingTrx::PRICING_TRX ||
       _trx.getTrxType() == PricingTrx::MIP_TRX) &&
      anySoftPass)
    return true;

  return false;
}

// This method loops through BrandProgram Index from fareMarket
// Then gets BrandProgram Pair from PricingTrx using this index
// If this BrandProgram (key) does not exist in BrandProgram Map then we insert this BrandProgram
// (key) and vector of Brands in BrandProgram Map
// Otherwise we insert this
void
BrandedFaresSelector::createBrandProgramMap(const FareMarket& fareMarket, std::map<BrandProgram*, std::vector<BrandInfo*>, BrandProgramComparator>& brandProgramMap) const
{
  for (std::size_t brandProgramIndex : fareMarket.brandProgramIndexVec())
  {
    if(brandProgramIndex >= _trx.brandProgramVec().size())
    {
      LOG4CXX_ERROR(_logger,
                    "S8BrandedFaresSelector::createBrandProgramMap BrandProgramIndex out of range");
      return;
    }

    const QualifiedBrand& brandProgramPair = _trx.brandProgramVec()[brandProgramIndex];

    std::map<BrandProgram*, std::vector<BrandInfo*>, BrandProgramComparator>::iterator it =
        brandProgramMap.find(brandProgramPair.first);
    if (it == brandProgramMap.end())
    {
      std::vector<BrandInfo*> brandVector;
      brandVector.push_back(brandProgramPair.second);
      brandProgramMap.insert(std::make_pair(brandProgramPair.first, brandVector));
    }
    else
    {
      it->second.push_back(brandProgramPair.second);
    }
  }
}

const BrandedFaresValidator&
BrandedFareValidatorFactory::getValidator(BrandSource brandSource) const
{
  if(brandSource == BRAND_SOURCE_CBAS)
    return _cbasBrandedFareValidator;
  return _s8BrandedFareValidator;
}

BrandedFaresSelector::BrandedFaresSelector(PricingTrx& trx,
                                           const BrandedFareValidatorFactory& brandSelectorFactory)
  : _trx(trx),
    _diag889(nullptr),
    _ignoreYyFares(ignoreYyFaresInBranding.getValue()),
    _brandSelectorFactory(brandSelectorFactory)
{
}

} // namespace
