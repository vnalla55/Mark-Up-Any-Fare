//-------------------------------------------------------------------
//
//  File:        GatewayPair.cpp
//  Created:     May 14, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Abstract class to represent common data and methods
//               to build single- and double- ended constructed fares:
//               1. [Gateway,            Construction Point]
//               2. [Construction Point, Gateway           ]
//               3. [Construction Point, Construction Point]
//
//  Copyright Sabre 2004
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

#include "AddonConstruction/GatewayPair.h"

#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/CombFareClassMap.h"
#include "AddonConstruction/ConstructedFare.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/ConstructionVendor.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"
#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TSELatencyData.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/FareInfo.h"
#include "Diagnostic/Diag252Collector.h"
#include "Diagnostic/Diag254Collector.h"
#include "Diagnostic/Diag255Collector.h"
#include "Util/BranchPrediction.h"

namespace tse
{
FALLBACK_DECL(removeDynamicCastForAddonConstruction);

using namespace std;

static Logger
logger("atseintl.AddonConstruction.GatewayPair");

namespace
{
ConfigurableValue<ConfigSet<CarrierCode>>
matchCurrencyCarriers("ADDON_CONSTRUCTION", "MATCH_CURR_CARRIERS");

template <class Vector>
void
clearVector(Vector& v)
{
  Vector().swap(v);
}

struct AddonFareCmp
{
  bool operator()(const AddonFareInfo* a1, const AddonFareInfo* a2) const
  {
    if (a1 == a2)
      return false;

    if (a1->fareClass() < a2->fareClass())
      return true;
    if (a2->fareClass() < a1->fareClass())
      return false;

    if (a1->addonTariff() < a2->addonTariff())
      return true;
    if (a2->addonTariff() < a1->addonTariff())
      return false;

    if (a1->owrt() < a2->owrt())
      return true;
    if (a2->owrt() < a1->owrt())
      return false;

    if (a1->routing() < a2->routing())
      return true;
    if (a2->routing() < a1->routing())
      return false;

    if (a1->effDate() < a2->effDate())
      return true;
    if (a2->effDate() < a1->effDate())
      return false;

    if (a1->expireDate() < a2->expireDate())
      return true;
    if (a2->expireDate() < a1->expireDate())
      return false;

    if (a1->arbZone() < a2->arbZone())
      return true;
    if (a2->arbZone() < a1->arbZone())
      return false;

    if (a1->footNote1() < a2->footNote1())
      return true;
    if (a2->footNote1() < a1->footNote1())
      return false;

    if (a1->footNote2() < a2->footNote2())
      return true;
    if (a2->footNote2() < a1->footNote2())
      return false;

    return false;
  }
};
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

void
GatewayPair::initialize(ConstructionJob* cj, ConstructionVendor* vendor)

{
  _cJob = cj;
  _vendor = vendor;
  _date = DateTime::localTime();
  _specFares = nullptr;
  _specFaresFromCache = false;
}

void
GatewayPair::initialize(ConstructionJob* cj,
                        ConstructionVendor* vendor,
                        const LocCode& aGateway1,
                        const LocCode& aGateway2,
                        bool isGw1CP,
                        bool isGw2CP,
                        unsigned int aGw1FirstFare,
                        unsigned int aGw1FareCount,
                        unsigned int aGw2FirstFare,
                        unsigned int aGw2FareCount)

{
  _cJob = cj;
  _vendor = vendor;
  _gateway1 = aGateway1;
  _gateway2 = aGateway2;
  _needsReconstruction = false;
  _isGw1ConstructPoint = isGw1CP;
  _isGw2ConstructPoint = isGw2CP;
  _gw1FirstFare = aGw1FirstFare;
  _gw1FareCount = aGw1FareCount;
  _gw2FirstFare = aGw2FirstFare;
  _gw2FareCount = aGw2FareCount;
  _date = DateTime::localTime();
  _specFares = nullptr;
  _specFaresFromCache = false;
};

void
GatewayPair::clear()
{
  _cJob = nullptr;
  _vendor = nullptr;

  _gw1FirstFare = 0;
  _gw1FareCount = 0;

  _gw2FirstFare = 0;
  _gw2FareCount = 0;

  clearVector(_constructedFares);
  _specFares = nullptr;
  _specFaresFromCache = false;
}

void
GatewayPair::prepareData()
{
  if (_specFares)
    return;
  if (UNLIKELY((_isGw1ConstructPoint && _gw1FareCount == 0) || (_isGw2ConstructPoint && _gw2FareCount == 0)))
    return;

  // retrieve published fareInfo records for governing carrier

  if (_isGw1ConstructPoint)
    _multiCity1 = LocUtil::getMultiTransportCity(
        _gateway1, _cJob->carrier(), GeoTravelType::International, _cJob->travelDate());
  else
    _multiCity1 = _cJob->boardMultiCity();

  if (_isGw2ConstructPoint)
    _multiCity2 = LocUtil::getMultiTransportCity(
        _gateway2, _cJob->carrier(), GeoTravelType::International, _cJob->travelDate());
  else
    _multiCity2 = _cJob->offMultiCity();

  getSpecifiedFares(_specFares);
}

void
GatewayPair::process(CacheConstructedFareInfoVec& response)
{
  prepareData();

  if (!_specFares)
    return;
  if (_specFares->empty())
  {
    cacheSpecFares(_specFares);
    return;
  }

  Diag255Collector* diag255{nullptr};
  if (!fallback::removeDynamicCastForAddonConstruction(&_cJob->trx()))
  {
    diag255 = _cJob->diagnostic<Diag255Collector>();
  }
  else
  {
    diag255 = _cJob->diag255();
  }

  if (UNLIKELY(diag255))
    diag255->writeGWHeader(*_vendor, *this);

  // match each SpecifiedFare with origin & destination add-on

  matchCFsAndAddons();
  cacheSpecFares(_specFares);

  // the final match

  finalMatch();

  // define market1 & market2 for fare. reverse fare if necessary

  defineOriginDestination();

  // populate the response vector with the results

  populateResponse(response);

  if (UNLIKELY(diag255))
    diag255->writeGWFooter();
}


void
GatewayPair::cacheSpecFares(SpecifiedFareList* fares)
{
  // No specified fare caching for exchange-type transactions as they feature
  // two ticketing dates (historical and present) and given fare set 
  // can be valid for one date and invalid for the other.
  if (!_specFaresFromCache &&
      _cJob->trx().excTrxType() == PricingTrx::NOT_EXC_TRX)
  {
    _cJob->specifiedCache()->add(_gateway1, _gateway2, _cJob->carrier(), _vendor->vendor(), fares);
  }
}


void
GatewayPair::dummyData(GatewayPair& obj)
{
  obj._gateway1 = "DFWXX";
  obj._multiCity1 = "HOUXX";
  obj._gateway2 = "NYCYY";
  obj._multiCity2 = "SFOYY";
  obj._date = DateTime::localTime();
  obj._needsReconstruction = false;
  obj._isGw1ConstructPoint = false;
  obj._isGw2ConstructPoint = true;
  obj._gw1FirstFare = 500;
  obj._gw1FareCount = 3;
  obj._gw2FirstFare = 1000;
  obj._gw2FareCount = 5;
  obj._specFares = nullptr;
  obj._specFaresFromCache = false;
}

void
GatewayPair::getSpecifiedFares(SpecifiedFareList*& specFares)
{
  TSELatencyData metrics(_cJob->trx(), "AC RETRIEVE SPECIFIED FARES");

  LOG4CXX_INFO(logger, "Entered GatewayPair::getSpecifiedFares(...)");

  // if cached, get entire list along with matched addon fare classes
  if ((specFares = _cJob->specifiedCache()->get(_gateway1, _gateway2, _cJob->carrier(), _vendor->vendor())))
  {
    _specFaresFromCache = true;
    return;
  }

  _specFaresFromCache = false;
  _cJob->trxDataHandle().get(specFares);

  Diag252Collector* diag252{nullptr};
  if (!fallback::removeDynamicCastForAddonConstruction(&_cJob->trx()))
  {
    diag252 = _cJob->diagnostic<Diag252Collector>();
  }
  else
  {
    diag252 = _cJob->diag252();
  }

  if (UNLIKELY(diag252))
    diag252->writeSpecFareDiagHeader(*this);

  retrieveSpecifiedFares(_gateway1, _gateway2, " fares retrived between gateways ", *specFares);

  // Multi-airport city

  if (_multiCity1 != _gateway1)
    retrieveSpecifiedFares(
        _multiCity1, _gateway2, " fares retrived between MultiAirportCity-gateway ", *specFares);

  if (_multiCity2 != _gateway2)
    retrieveSpecifiedFares(
        _gateway1, _multiCity2, " fares retrived between gateway-MultiAirportCity ", *specFares);

  if (_multiCity1 != _gateway1 && _multiCity2 != _gateway2)
    retrieveSpecifiedFares(
        _multiCity1, _multiCity2, " fares retrived between MultiAirportCities ", *specFares);

  if (UNLIKELY(diag252))
  {
    diag252->writeSpecifiedFares(*this, *specFares);
    diag252->writeSpecFareDiagFooter(*this);
  }
}

void
GatewayPair::retrieveSpecifiedFares(const LocCode& loc1,
                                    const LocCode& loc2,
                                    const char* logMsg,
                                    SpecifiedFareList& specFares)
{
  // Specified fares are to be shared across addon construction threads -
  // that's why transaction-scoped dataHandle has to be used here.
  const std::vector<const FareInfo*>& fareInfoDb =
      _cJob->trxDataHandle().getFaresByMarketCxr(loc1, loc2,
                                                 _cJob->carrier(),
                                                 _vendor->vendor(),
                                                 _cJob->ticketingDate());

  unsigned int numRejectedSITAFares = 0;

  Diag252Collector* diag252{nullptr};
  if (!fallback::removeDynamicCastForAddonConstruction(&_cJob->trx()))
  {
    diag252 = _cJob->diagnostic<Diag252Collector>();
  }
  else
  {
    diag252 = _cJob->diag252();
  }

  LOG4CXX_INFO(logger, fareInfoDb.size() << logMsg << loc1 << "-" << loc2);

  if (!fareInfoDb.empty())
  {
    if (_cJob->isSita() || _cJob->isSMF())
    {
      // SITA and SMF fares with ConstructionInd == 'N'
      // are not eligible for construction
      for (auto fi: fareInfoDb)
      {
        if (fi->constructionInd() != 'N')
          specFares.emplace_back(fi);
        else
        {
          numRejectedSITAFares++;

          if (UNLIKELY(diag252))
            diag252->writeSpecifiedFare(*this, *fi, Diag252Collector::CP_SITA_NOT_FOR_CONSTRUCTION);
        }
      }

      if (numRejectedSITAFares > 0)
        LOG4CXX_INFO(logger,
                     numRejectedSITAFares << " SITA fares rejected for " << loc1 << "-" << loc2);
    }
    else
      std::transform(fareInfoDb.begin(), fareInfoDb.end(),
                     std::back_inserter(specFares),
                     [](const FareInfo* f){return SpecifiedFare(f);});
  }
}


ConstructedFare*
GatewayPair::getConstructedFare(const FareInfo* specFare, bool oppositeSpecified)
{
  ConstructedFare* constrFare(getConstructedFare());

  if (UNLIKELY(_cJob->trx().getOptions()->isRtw()))
    const_cast<FareInfo*>(specFare)->resetToOriginalAmount();

  constrFare->initialize(_cJob, *specFare, *this, oppositeSpecified);

  return constrFare;
}


void
GatewayPair::matchCFsAndAddons()
{
  Diag255Collector* diag255{nullptr};
  if (!fallback::removeDynamicCastForAddonConstruction(&_cJob->trx()))
  {
    diag255 = _cJob->diagnostic<Diag255Collector>();
  }
  else
  {
    diag255 = _cJob->diag255();
  }

  AddonFareCortegeVec& origAddons(_vendor->addonFares(CP_ORIGIN));
  AddonFareCortegeVec& destAddons(_vendor->addonFares(CP_DESTINATION));
  AddonFareCortegeVec::iterator firstAddonL(origAddons.begin()), firstAddonR(destAddons.begin());
  AddonFareCortegeVec::iterator endOfAddonsL(origAddons.end()), endOfAddonsR(destAddons.end());

  if (_isGw1ConstructPoint)
  {
    firstAddonL += _gw1FirstFare;
    endOfAddonsL = firstAddonL + _gw1FareCount;
  }
  if (_isGw2ConstructPoint)
  {
    firstAddonR += _gw2FirstFare;
    endOfAddonsR = firstAddonR + _gw2FareCount;
  }

  const bool preferMatchingCurrency = matchCurrencyCarriers.getValue().has(_cJob->carrier());
  std::set<std::pair<CurrencyCode, const AddonFareInfo*> >* addonsValidForCurrencyL(nullptr);
  std::set<std::pair<CurrencyCode, const AddonFareInfo*> >* addonsValidForCurrencyR(nullptr);

  // identify all currencies of the specified fares
  if (UNLIKELY(preferMatchingCurrency))
  {
    _cJob->dataHandle().get(addonsValidForCurrencyL);
    _cJob->dataHandle().get(addonsValidForCurrencyR);
    std::set<CurrencyCode> currencies;
    for (const auto& fare : *_specFares)
      currencies.insert(fare._specFare->currency());

    getAddonsForCurrency(currencies, firstAddonL, endOfAddonsL, *addonsValidForCurrencyL);
    getAddonsForCurrency(currencies, firstAddonR, endOfAddonsR, *addonsValidForCurrencyR);
  }

  // loop via all ConstructedFare's

  CombFareClassMap* cmap(_vendor->getCombFareClassMap());

  AddonFareCortegeVec::iterator roundTripMayNotBeHalvedAddonBoundL(firstAddonL);
  if (_isGw1ConstructPoint)
    roundTripMayNotBeHalvedAddonBoundL = partition(firstAddonL, endOfAddonsL);
  AddonFareCortegeVec::iterator roundTripMayNotBeHalvedAddonBoundR(firstAddonR);
  if (_isGw2ConstructPoint)
    roundTripMayNotBeHalvedAddonBoundR = partition(firstAddonR, endOfAddonsR);

  for (auto & specFare: *_specFares)
  {
    if (cmap != nullptr)
    {
      cmap->setTariff(specFare._specFare->fareTariff());
    }

    // loop via each add-on

    std::pair<AddonFareCortegeVec::iterator, AddonFareCortegeVec::iterator> addonIt, addonIt1;
    if (_isGw1ConstructPoint)
    {
      addonIt = getOwrtMatchingAddonRange(
          *specFare._specFare, firstAddonL, endOfAddonsL, roundTripMayNotBeHalvedAddonBoundL);
      if (_isGw2ConstructPoint)
        addonIt1 = getOwrtMatchingAddonRange(
            *specFare._specFare, firstAddonR, endOfAddonsR, roundTripMayNotBeHalvedAddonBoundR);
    }
    else //(_isGw2ConstructPoint)
    {
      addonIt = getOwrtMatchingAddonRange(
          *specFare._specFare, firstAddonR, endOfAddonsR, roundTripMayNotBeHalvedAddonBoundR);
    }

    bool oppositeSpecified = ((specFare._specFare->market1() != _gateway1) &&
                              (specFare._specFare->market1() != _multiCity1));

    AddonFareCortegeVec::iterator iAddon = addonIt.first;
    for (; iAddon != addonIt.second; ++iAddon)
    {
      if (UNLIKELY(preferMatchingCurrency))
      {
        if ((_isGw1ConstructPoint && addonsValidForCurrencyL->find(
                std::make_pair(specFare._specFare->currency(), (*iAddon)->addonFare())) ==
            addonsValidForCurrencyL->end()) ||
            (!_isGw1ConstructPoint && addonsValidForCurrencyR->find(
                std::make_pair(specFare._specFare->currency(), (*iAddon)->addonFare())) ==
            addonsValidForCurrencyR->end()))
          continue;
      }
      AtpcoFareDateInterval dateInterval;
      FareMatchCode matchResult = matchAddonAndSpecified(**iAddon,
                                                         specFare,
                                                         oppositeSpecified,
                                                         _isGw1ConstructPoint,
                                                         dateInterval);

      if (UNLIKELY(diag255))
        diag255->writeFarePair(*(*iAddon)->addonFare(), *specFare._specFare, matchResult);

      if (matchResult == FM_GOOD_MATCH && _isGw1ConstructPoint && _isGw2ConstructPoint)
      {
        AddonFareCortegeVec::iterator iAddon1 = addonIt1.first;
        for (; iAddon1 != addonIt1.second; ++iAddon1)
        {
          if (UNLIKELY(preferMatchingCurrency))
          {
            if (addonsValidForCurrencyR->find(
                    std::make_pair(specFare._specFare->currency(), (*iAddon1)->addonFare())) ==
                addonsValidForCurrencyR->end())
            {
              continue;
            }
          }
          AtpcoFareDateInterval dateInterval1;
          FareMatchCode matchResult1 = matchAddonAndSpecified(**iAddon1,
                                                             specFare,
                                                             oppositeSpecified,
                                                             false/*destination addon*/,
                                                             dateInterval1);
          if (matchResult1 == FM_GOOD_MATCH)
          {
            ConstructedFare* newCF(getConstructedFare(specFare._specFare, oppositeSpecified));
            newCF->setAddon(*iAddon, true/*origin addon*/, dateInterval);
            newCF->setAddon(*iAddon1, false/*destination addon*/, dateInterval1);
            _constructedFares.push_back(newCF);
          }
        }
      }
      else if (matchResult == FM_GOOD_MATCH)
      {
        ConstructedFare* newCF(getConstructedFare(specFare._specFare, oppositeSpecified));
        newCF->setAddon(*iAddon, _isGw1ConstructPoint, dateInterval);
        _constructedFares.push_back(newCF);
      }
    }
  }
}

void
GatewayPair::adjustPrevValues()
{
  ConstructedFareList::const_iterator i = _constructedFares.begin();
  ConstructedFareList::const_iterator ie = _constructedFares.end();

  for (; i != ie; ++i)
    (*i)->adjustPrevValues();
}

void
GatewayPair::defineOriginDestination()
{
  ConstructedFareList::const_iterator i = _constructedFares.begin();
  ConstructedFareList::const_iterator ie = _constructedFares.end();

  for (; i != ie; ++i)
  {
    if ((*i)->isValid())
      (*i)->defineOriginDestination();
  }
}

void
GatewayPair::populateResponse(CacheConstructedFareInfoVec& response)
{
  Diag254Collector* diag254{nullptr};
  if (!fallback::removeDynamicCastForAddonConstruction(&_cJob->trx()))
  {
    diag254 = _cJob->diagnostic<Diag254Collector>();
  }
  else
  {
    diag254 = _cJob->diag254();
  }

  ConstructedFareList::const_iterator i = _constructedFares.begin();
  ConstructedFareList::const_iterator ie = _constructedFares.end();

  for (; i != ie; ++i)
  {
    if (!(*i)->isValid())
      continue;

    if (UNLIKELY(diag254))
      diag254->writeConstructedFare(**i);

    std::shared_ptr<ConstructedFareInfo> cfi((*i)->cloneToConstructedFareInfo());

    response.push_back(cfi);
  }

  if (UNLIKELY(diag254))
    diag254->writeFooter();
}

AddonFareCortegeVec::iterator
GatewayPair::partition(AddonFareCortegeVec::iterator firstAddon,
                       AddonFareCortegeVec::iterator endOfAddons)
{
  return endOfAddons;
}

std::pair<AddonFareCortegeVec::iterator, AddonFareCortegeVec::iterator>
GatewayPair::getOwrtMatchingAddonRange(
    const FareInfo& sfi,
    AddonFareCortegeVec::iterator firstAddon,
    AddonFareCortegeVec::iterator endOfAddons,
    AddonFareCortegeVec::iterator roundTripMayNotBeHalvedAddonBound)
{

  return std::make_pair(firstAddon, endOfAddons);
}


void
GatewayPair::getAddonsForCurrency(const std::set<CurrencyCode>& currencies,
                                  AddonFareCortegeVec::iterator firstAddon,
                                  AddonFareCortegeVec::iterator endOfAddons,
                                  std::set<std::pair<CurrencyCode,
                                                     const AddonFareInfo*> >& addonsValidForCurrency)
{
  for (const auto& currency : currencies)
  {
    std::multiset<const AddonFareInfo*, AddonFareCmp> fareInfos;
    for (AddonFareCortegeVec::iterator i = firstAddon; i != endOfAddons; i++)
    {
      const AddonFareInfo* afi = (*i)->addonFare();
      auto const& foundRange = fareInfos.equal_range(afi);

      if (foundRange.first == foundRange.second)
      {
        fareInfos.insert(afi);
      }
      else
      {
        const AddonFareInfo* firstFound = *(foundRange.first);
        if (firstFound == afi)
          continue;
        if (firstFound->cur() == currency)
        {
          if (afi->cur() == currency)
          {
            fareInfos.insert(afi);
            LOG4CXX_ERROR(logger, "Check duplicate Addons");
          }
        }
        else
        {
          if (afi->cur() == currency)
          {
            LOG4CXX_INFO(logger,
                         std::distance(foundRange.first, foundRange.second)
                             << " addon(s) eliminated by currency check");
            fareInfos.erase(foundRange.first, foundRange.second);
          }
          fareInfos.insert(afi);
        }
      }
    }
    for (const auto fareInfo : fareInfos)
      addonsValidForCurrency.insert(std::make_pair(currency, fareInfo));
  }
}

#else

void
GatewayPair::initialize(ConstructionJob* cj, ConstructionVendor* vendor)

{
  _cJob = cj;
  _vendor = vendor;
}

void
GatewayPair::initialize(ConstructionJob* cj,
                        ConstructionVendor* vendor,
                        const LocCode& aGateway1,
                        const LocCode& aGateway2,
                        bool isGw1CP,
                        bool isGw2CP,
                        unsigned int aGw1FirstFare,
                        unsigned int aGw1FareCount,
                        unsigned int aGw2FirstFare,
                        unsigned int aGw2FareCount)

{
  _cJob = cj;
  _vendor = vendor;
  _gateway1 = aGateway1;
  _gateway2 = aGateway2;
  _needsReconstruction = false;
  _isGw1ConstructPoint = isGw1CP;
  _isGw2ConstructPoint = isGw2CP;
  _gw1FirstFare = aGw1FirstFare;
  _gw1FareCount = aGw1FareCount;
  _gw2FirstFare = aGw2FirstFare;
  _gw2FareCount = aGw2FareCount;
};

void
GatewayPair::clear()
{
  _cJob = nullptr;
  _vendor = nullptr;

  _gw1FirstFare = 0;
  _gw1FareCount = 0;

  _gw2FirstFare = 0;
  _gw2FareCount = 0;

  clearVector(_constructedFares);
  clearVector(_specFares);
}
void
GatewayPair::prepareData()
{
  if (!_specFares.empty())
  {
    return;
  }
  if (UNLIKELY((_isGw1ConstructPoint && _gw1FareCount == 0) || (_isGw2ConstructPoint && _gw2FareCount == 0)))
    return;

  // retrieve published fareInfo records for governing carrier

  if (_isGw1ConstructPoint)
    _multiCity1 = LocUtil::getMultiTransportCity(
        _gateway1, _cJob->carrier(), GeoTravelType::International, _cJob->travelDate());
  else
    _multiCity1 = _cJob->boardMultiCity();

  if (_isGw2ConstructPoint)
    _multiCity2 = LocUtil::getMultiTransportCity(
        _gateway2, _cJob->carrier(), GeoTravelType::International, _cJob->travelDate());
  else
    _multiCity2 = _cJob->offMultiCity();

  getSpecifiedFares(_specFares);
}

void
GatewayPair::process(CacheConstructedFareInfoVec& response)
{
  prepareData();

  if (_specFares.empty())
    return;

  // loop through all retrieved fareInfos and
  // create initial vector of ConstructedFare's

  createConstructedFares(_specFares);

  Diag255Collector* diag255{nullptr};
  if (!fallback::removeDynamicCastForAddonConstruction(&_cJob->trx()))
  {
    diag255 = _cJob->diagnostic<Diag255Collector>();
  }
  else
  {
    diag255 = _cJob->diag255();
  }

  if (UNLIKELY(diag255))
    diag255->writeGWHeader(*_vendor, *this);

  // match each ConstructedFare with origin add-on

  if (_isGw1ConstructPoint)
    matchCFsAndAddons(true);

  // match each ConstructedFare with destination add-on

  if (_isGw2ConstructPoint)
  {
    if (_isGw1ConstructPoint)
      adjustPrevValues();

    matchCFsAndAddons(false);
  }

  // the final match

  finalMatch();

  // define market1 & market2 for fare. reverse fare if necessary

  defineOriginDestination();

  // populate the response vector with the results

  populateResponse(response);

  if (UNLIKELY(diag255))
    diag255->writeGWFooter();
}

void
GatewayPair::dummyData(GatewayPair& obj)
{
  obj._gateway1 = "DFWXX";
  obj._multiCity1 = "HOUXX";
  obj._gateway2 = "NYCYY";
  obj._multiCity2 = "SFOYY";
  obj._needsReconstruction = false;
  obj._isGw1ConstructPoint = false;
  obj._isGw2ConstructPoint = true;
  obj._gw1FirstFare = 500;
  obj._gw1FareCount = 3;
  obj._gw2FirstFare = 1000;
  obj._gw2FareCount = 5;
}

unsigned int
GatewayPair::getSpecifiedFares(std::vector<const FareInfo*>& specFares)
{
  TSELatencyData metrics(_cJob->trx(), "AC RETRIEVE SPECIFIED FARES");

  LOG4CXX_INFO(logger, "Entered GatewayPair::getSpecifiedFares(...)");

  Diag252Collector* diag252{nullptr};
  if (!fallback::removeDynamicCastForAddonConstruction(&_cJob->trx()))
  {
    diag252 = _cJob->diagnostic<Diag252Collector>();
  }
  else
  {
    diag252 = _cJob->diag252();
  }
  if (UNLIKELY(diag252))
    diag252->writeSpecFareDiagHeader(*this);

  retrieveSpecifiedFares(_gateway1, _gateway2, " fares retrived between gateways ", specFares);

  // Multi-airport city

  if (_multiCity1 != _gateway1)
    retrieveSpecifiedFares(
        _multiCity1, _gateway2, " fares retrived between MultiAirportCity-gateway ", specFares);

  if (_multiCity2 != _gateway2)
    retrieveSpecifiedFares(
        _gateway1, _multiCity2, " fares retrived between gateway-MultiAirportCity ", specFares);

  if (_multiCity1 != _gateway1 && _multiCity2 != _gateway2)
    retrieveSpecifiedFares(
        _multiCity1, _multiCity2, " fares retrived between MultiAirportCities ", specFares);

  if (UNLIKELY(diag252))
  {
    diag252->writeSpecifiedFares(*this, specFares);
    diag252->writeSpecFareDiagFooter(*this);
  }

  return specFares.size();
}

unsigned int
GatewayPair::retrieveSpecifiedFares(const LocCode& loc1,
                                    const LocCode& loc2,
                                    const char* logMsg,
                                    std::vector<const FareInfo*>& specFares)
{
  const std::vector<const FareInfo*>& fareInfoDb =
      _cJob->dataHandle().getFaresByMarketCxr(loc1, loc2, _cJob->carrier(), _vendor->vendor());

  unsigned int numFares = fareInfoDb.size();
  unsigned int numRejectedSITAFares = 0;

  Diag252Collector* diag252{nullptr};
  if (!fallback::removeDynamicCastForAddonConstruction(&_cJob->trx()))
  {
    diag252 = _cJob->diagnostic<Diag252Collector>();
  }
  else
  {
    diag252 = _cJob->diag252();
  }

  LOG4CXX_INFO(logger, numFares << logMsg << loc1 << "-" << loc2);

  if (!fareInfoDb.empty())
  {
    if (_cJob->isSita() || _cJob->isSMF())
    {
      // SITA and SMF fares with ConstructionInd == 'N'
      // are not eligible for construction

      std::vector<const FareInfo*>::const_iterator i = fareInfoDb.begin();
      std::vector<const FareInfo*>::const_iterator ie = fareInfoDb.end();

      for (; i != ie; ++i)
      {
        const FareInfo* fi = *i;

        if (fi->constructionInd() != 'N')
          specFares.push_back(fi);
        else
        {
          numFares--;
          numRejectedSITAFares++;

          if (UNLIKELY(diag252))
            diag252->writeSpecifiedFare(*this, *fi, Diag252Collector::CP_SITA_NOT_FOR_CONSTRUCTION);
        }
      }

      if (numRejectedSITAFares > 0)
        LOG4CXX_INFO(logger,
                     numRejectedSITAFares << " SITA fares rejected for " << loc1 << "-" << loc2);
    }
    else
      copy(fareInfoDb.begin(), fareInfoDb.end(), back_inserter(specFares));
  }

  return numFares;
}

void
GatewayPair::createConstructedFares(std::vector<const FareInfo*>& specFares)
{
  ConstructedFare* cf;

  std::vector<const FareInfo*>::const_iterator i = specFares.begin();
  std::vector<const FareInfo*>::const_iterator ie = specFares.end();
  for (; i != ie; ++i)
  {
    if (UNLIKELY(_cJob->trx().getOptions()->isRtw()))
      const_cast<FareInfo*>(*i)->resetToOriginalAmount();

    cf = getConstructedFare();

    bool oppositeSpecified = ((*i)->market1() != _gateway1) && ((*i)->market1() != _multiCity1);
    cf->initialize(_cJob, **i, *this, oppositeSpecified);

    _constructedFares.push_back(cf);
  }
}

void
GatewayPair::matchCFsAndAddons(bool matchOriginAddons)
{
  Diag255Collector* diag255{nullptr};
  if (!fallback::removeDynamicCastForAddonConstruction(&_cJob->trx()))
  {
    diag255 = _cJob->diagnostic<Diag255Collector>();
  }
  else
  {
    diag255 = _cJob->diag255();
  }

  AddonFareCortegeVec::iterator firstAddon;
  AddonFareCortegeVec::iterator endOfAddons;

  if (matchOriginAddons)
  {
    firstAddon = _vendor->addonFares(CP_ORIGIN).begin() + _gw1FirstFare;
    endOfAddons = firstAddon + _gw1FareCount;
  }
  else
  {
    firstAddon = _vendor->addonFares(CP_DESTINATION).begin() + _gw2FirstFare;
    endOfAddons = firstAddon + _gw2FareCount;
  }

  const bool preferMatchingCurrency = matchCurrencyCarriers.getValue().has(_cJob->carrier());
  std::set<std::pair<CurrencyCode, const AddonFareInfo*> > addonsValidForCurrency;

  // identify all currencies of the specified fares
  if (UNLIKELY(preferMatchingCurrency))
  {
    std::set<CurrencyCode> currencies;
    for (const ConstructedFare* fare : _constructedFares)
      currencies.insert(fare->specifiedFare()->currency());

    for (const auto currency : currencies)
    {
      std::multiset<const AddonFareInfo*, AddonFareCmp> fareInfos;
      for (AddonFareCortegeVec::iterator i = firstAddon; i != endOfAddons; i++)
      {
        const AddonFareInfo* afi = (*i)->addonFare();
        std::pair<std::multiset<const AddonFareInfo*, AddonFareCmp>::iterator,
                  std::multiset<const AddonFareInfo*, AddonFareCmp>::iterator> foundRange =
            fareInfos.equal_range(afi);

        if (foundRange.first == foundRange.second)
        {
          fareInfos.insert(afi);
        }
        else
        {
          const AddonFareInfo* firstFound = *(foundRange.first);
          if (firstFound == afi)
            continue;
          if (firstFound->cur() == currency)
          {
            if (afi->cur() == currency)
            {
              fareInfos.insert(afi);
              LOG4CXX_ERROR(logger, "Check duplicate Addons");
            }
          }
          else
          {
            if (afi->cur() == currency)
            {
              LOG4CXX_INFO(logger,
                           std::distance(foundRange.first, foundRange.second)
                               << " addon(s) eliminated by currency check");
              fareInfos.erase(foundRange.first, foundRange.second);
            }
            fareInfos.insert(afi);
          }
        }
      }
      for (const auto fareInfo : fareInfos)
        addonsValidForCurrency.insert(std::make_pair(currency, fareInfo));
    }
  }

  // loop via all ConstructedFare's

  // we push back new elements to _constructedFares at the end
  // of this loop, therefore we can't use iterators to loop via
  // _constructedFares vector. we use operator [] instead.

  // theoretically this should slow down the application a bit...
  // if somebody can measure that couple of nanoseconds please let
  // me know...

  CombFareClassMap* cmap(_vendor->getCombFareClassMap());

  int iCF = 0;
  int cfiListSize = _constructedFares.size();

  AddonFareCortegeVec::iterator firstAddonPartition = firstAddon;
  AddonFareCortegeVec::iterator endOfAddonsPartition = endOfAddons;

  AddonFareCortegeVec::iterator roundTripMayNotBeHalvedAddonBound =
      partition(firstAddon, endOfAddons);

  for (; iCF < cfiListSize; iCF++)
  {
    ConstructedFare* cfi = _constructedFares[iCF];

    if (cmap != nullptr)
    {
      cmap->setTariff(cfi->specifiedFare()->fareTariff());
    }
    // for destination add-on skip double-ended constructed
    // fares if origin add-on is missing

    if (!matchOriginAddons && (cfi->isDoubleEnded()) && (cfi->origAddon() == nullptr))
    {
      continue;
    }

    // loop via each add-on

    std::pair<AddonFareCortegeVec::iterator, AddonFareCortegeVec::iterator> addonIt =
        getAddonIterators(
            *cfi->specifiedFare(), firstAddon, endOfAddons, roundTripMayNotBeHalvedAddonBound);

    AddonFareCortegeVec::iterator iAddon = addonIt.first;

    for (; iAddon != addonIt.second; ++iAddon)
    {
      if (UNLIKELY(preferMatchingCurrency))
      {
        if (addonsValidForCurrency.find(
                std::make_pair(cfi->specifiedFare()->currency(), (*iAddon)->addonFare())) ==
            addonsValidForCurrency.end())
          continue;
      }
      FareMatchCode matchResult = matchAddonAndSpecified(**iAddon, *cfi, matchOriginAddons);

      if (UNLIKELY(diag255))
        diag255->writeFarePair(*(*iAddon)->addonFare(), *cfi->specifiedFare(), matchResult);
    }
  }
}

FareMatchCode
GatewayPair::matchAddonAndSpecified(AddonFareCortege& addonFare,
                                    ConstructedFare& constrFare,
                                    const bool isOriginAddon)
{

  vector<DateIntervalBase*> constrFareIntervals;

  FareMatchCode matchResult =
      matchAddonAndSpecified(addonFare, constrFare, isOriginAddon, constrFareIntervals);

  if (matchResult == FM_GOOD_MATCH)
  {
    ConstructedFare* newCF;

    vector<DateIntervalBase*>::const_iterator i = constrFareIntervals.begin();

    for (; i != constrFareIntervals.end(); ++i)
    {
      if (!constrFare.origOrDestAddon(isOriginAddon))
        newCF = &constrFare;
      else
      {
        newCF = constrFare.clone(_cJob->dataHandle());

        _constructedFares.push_back(newCF);
      }

      newCF->setAddon(&addonFare, isOriginAddon, **i);
    }
  }

  return matchResult;
}

void
GatewayPair::adjustPrevValues()
{
  ConstructedFareList::const_iterator i = _constructedFares.begin();
  ConstructedFareList::const_iterator ie = _constructedFares.end();

  for (; i != ie; ++i)
    (*i)->adjustPrevValues();
}

void
GatewayPair::defineOriginDestination()
{
  ConstructedFareList::const_iterator i = _constructedFares.begin();
  ConstructedFareList::const_iterator ie = _constructedFares.end();

  for (; i != ie; ++i)
  {
    if ((*i)->isValid())
      (*i)->defineOriginDestination();
  }
}

void
GatewayPair::populateResponse(CacheConstructedFareInfoVec& response)
{
  Diag254Collector* diag254{nullptr};
  if (!fallback::removeDynamicCastForAddonConstruction(&_cJob->trx()))
  {
    diag254 = _cJob->diagnostic<Diag254Collector>();
  }
  else
  {
    diag254 = _cJob->diag254();
  }

  ConstructedFareList::const_iterator i = _constructedFares.begin();
  ConstructedFareList::const_iterator ie = _constructedFares.end();

  for (; i != ie; ++i)
  {
    if (!(*i)->isValid())
      continue;

    if (UNLIKELY(diag254))
      diag254->writeConstructedFare(**i);

    std::shared_ptr<ConstructedFareInfo> cfi((*i)->cloneToConstructedFareInfo());

    response.push_back(cfi);
  }

  if (UNLIKELY(diag254))
    diag254->writeFooter();
}

AddonFareCortegeVec::iterator
GatewayPair::partition(const AddonFareCortegeVec::iterator& firstAddon,
                       const AddonFareCortegeVec::iterator& endOfAddons)
{
  return endOfAddons;
}

std::pair<AddonFareCortegeVec::iterator, AddonFareCortegeVec::iterator>
GatewayPair::getAddonIterators(
    const FareInfo& sfi,
    const AddonFareCortegeVec::iterator& firstAddon,
    const AddonFareCortegeVec::iterator& endOfAddons,
    const AddonFareCortegeVec::iterator& roundTripMayNotBeHalvedAddonBound)
{

  return std::make_pair(firstAddon, endOfAddons);
}

#endif


void
GatewayPair::flattenize(Flattenizable::Archive& archive)
{
  // _cJob -- transient data member only
  // _vendor -- transient data member only
  FLATTENIZE(archive, _gateway1);
  FLATTENIZE(archive, _multiCity1);
  FLATTENIZE(archive, _gateway2);
  FLATTENIZE(archive, _multiCity2);
  FLATTENIZE(archive, _needsReconstruction);
  FLATTENIZE(archive, _isGw1ConstructPoint);
  FLATTENIZE(archive, _isGw2ConstructPoint);
  FLATTENIZE(archive, _gw1FirstFare);
  FLATTENIZE(archive, _gw1FareCount);
  FLATTENIZE(archive, _gw2FirstFare);
  FLATTENIZE(archive, _gw2FareCount);
  // _constructedFares -- transient data member only
  // _specFares -- transient data member only
  // _specFaresFromCache -- transient data member only
}

bool
GatewayPair::
operator==(const GatewayPair& rhs) const
{
  return ((_gateway1 == rhs._gateway1) && (_multiCity1 == rhs._multiCity1) &&
          (_gateway2 == rhs._gateway2) && (_multiCity2 == rhs._multiCity2) &&
          // _date is irrelevant in comparison
          (_needsReconstruction == rhs._needsReconstruction) &&
          (_isGw1ConstructPoint == rhs._isGw1ConstructPoint) &&
          (_isGw2ConstructPoint == rhs._isGw2ConstructPoint) &&
          (_gw1FirstFare == rhs._gw1FirstFare) && (_gw1FareCount == rhs._gw1FareCount) &&
          (_gw2FirstFare == rhs._gw2FirstFare) && (_gw2FareCount == rhs._gw2FareCount));
          // _specFares and _specFaresFromCache are irrelevant in comparison
}

}

