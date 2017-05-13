//
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#include "Fares/FDFareCurrencySelection.h"

#include "Common/ErrorResponseException.h"
#include "Common/FareDisplayUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/Currency.h"
#include "DBAccess/CurrencySelection.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "Fares/CurrencySelectionValidator.h"
#include "Rules/RuleConst.h"

#include <iostream>
#include <set>
#include <vector>

#include <time.h>

namespace tse
{

typedef std::vector<tse::Itin*> ItinVec;
typedef ItinVec::const_iterator ItinVecI;

static Logger
logger("atseintl.Fares.FDFareCurrencySelection");

typedef std::pair<CurrencyCode, int> CCPair;
typedef std::set<CurrencyCode>::iterator CurCacheI;

class FDFareCurrencySelection::UniqueCurrencies
    : public std::unary_function<const FareMarketCurrencyKey&, bool>
{
public:
  UniqueCurrencies(const FareMarketCurrencyKey& key) : _key(key) {}

  bool operator()(const FareMarketCurrencyKey& key) const
  {
    if (_key.directionality() == BOTH)
    {
      if (_key.paxTypeCode() == key.paxTypeCode() && _key.currencyCode() != key.currencyCode())
      {
        return true;
      }
    }
    else
    {
      if (_key.paxTypeCode() == key.paxTypeCode() &&
          _key.directionality() == key.directionality() &&
          _key.currencyCode() != key.currencyCode())
      {
        return true;
      }
    }

    return false;
  }

  const FareMarketCurrencyKey& _key;
};

class hasMatchingFDKey
{
public:
  hasMatchingFDKey(const FareMarketCurrencyKey& key) : _key(key) {}

  // This should only be used if there is one currency per
  // passenger type in the market
  //
  bool operator()(const FareMarketCurrencyKey& key) const
  {
    return (_key.paxTypeCode() == key.paxTypeCode() &&
            _key.directionality() == key.directionality() &&
            _key.currencyCode() == key.currencyCode());
  }

private:
  const FareMarketCurrencyKey& _key;
};

class FDFareCurrencySelection::CTRWCurrencies
    : public std::unary_function<const CTRWCurrencyKey&, bool>
{
public:
  CTRWCurrencies(const CTRWCurrencyKey& key) : _key(key) {}

  bool operator()(const FareMarketCurrencyKey& key) const
  {

    if (_key.paxTypeCode() == key.paxTypeCode() && _key.currencyCode() != key.currencyCode())
    {
      return true;
    }

    return false;
  }

  const CTRWCurrencyKey& _key;
};

class hasMatchingCTRWKey
{
public:
  hasMatchingCTRWKey(const CTRWCurrencyKey& key) : _key(key) {}

  bool operator()(const FareMarketCurrencyKey& key) const
  {
    return (_key.paxTypeCode() == key.paxTypeCode() && _key.currencyCode() == key.currencyCode());
  }

private:
  const CTRWCurrencyKey& _key;
};

//--------------------------------------------------------------
//
//   @method FDFareCurrencySelection
//
//--------------------------------------------------------------
tse::FDFareCurrencySelection::FDFareCurrencySelection() {}

//---------------------------------------------------------------
//
//   @method ~FDFareCurrencySelection
//
//---------------------------------------------------------------
tse::FDFareCurrencySelection::~FDFareCurrencySelection() {}

//--------------------------------------------------------------------------------------------
//
//   @method selectPrimeCurrency
//
//   Description: Performs validation on each fare component to determine the currency for
//                fare selection, unless there is a single currency, then fares are
//                selected in that currency.
//
//   @param FareDisplayTrx     - transaction object
//   @param FareMarket         - fare market
//   @param GeoTravelType      - travel type of itinerary
//   @param bool               - determine out bound currency only - true, else false
//
//   @return bool  - true/false, false is failure
//
//--------------------------------------------------------------------------------------------
bool
tse::FDFareCurrencySelection::selectPrimeCurrency(FareDisplayTrx& trx,
                                                  FareMarket& fareMarket,
                                                  Itin& itin)
{

  LOG4CXX_DEBUG(
      logger,
      "Entered FDFDFareCurrencySelection::selectPrimeCurrency for Foreign Domestic/International");

  PaxTypeBucketVecI paxTypeCortegeVecIter;
  PaxTypeBucketVecI paxTypeCortegeVecIterEnd;
  CurrencyCode fareCompPrimeCurrency;
  CurrencyCode fareQuoteOverrideCurrency;
  CurrencyCode firstCurrency;
  NationCode nationCode;
  NationCode blankNation;
  bool validRC = false;
  bool paxTypeSearchFailed = false;
  bool nationNotFound = false;
  bool foundOutBoundCurrency = false;
  GeoTravelType& itinTravelType = itin.geoTravelType();
  bool isInternational = (itin.geoTravelType() == GeoTravelType::International);

  DateTime& ticketingDate = trx.getRequest()->ticketingDT();
  const Customer* agentTJR = trx.getRequest()->ticketingAgent()->agentTJR();
  const std::string cruisePfaCurrency = ((agentTJR != nullptr) ? agentTJR->cruisePfaCur() : "");
  const NationCode& nation = fareMarket.origin()->loc();
  getNationCode(trx, nation, ticketingDate, nationCode);
  const tse::Directionality direction = FROM;

  bool privateFaresRequested = trx.getOptions()->isPrivateFares();
  determineCurrenciesInFareMarket(
      fareMarket, cruisePfaCurrency, trx, itinTravelType, true, privateFaresRequested);

  PaxTypeBucketVec& paxTypeCortegeVec = fareMarket.paxTypeCortege();
  paxTypeCortegeVecIter = fareMarket.paxTypeCortege().begin();
  paxTypeCortegeVecIterEnd = fareMarket.paxTypeCortege().end();

  size_t paxTypeCortegeVecSize = paxTypeCortegeVec.size();

  // Set the Outbound Currency for each PaxTypeBucket
  for (uint16_t paxTypeCortegeCntr = 0; paxTypeCortegeCntr < paxTypeCortegeVecSize;
       paxTypeCortegeCntr++)
  {
    fareQuoteOverrideCurrency.clear();
    fareCompPrimeCurrency.clear();

    PaxTypeBucket& paxTypeCortege = paxTypeCortegeVec[paxTypeCortegeCntr];
    const tse::PaxType* requestedPaxType = paxTypeCortege.requestedPaxType();
    const tse::PaxTypeCode paxType = requestedPaxType->paxType();

    if (paxTypeCortege.paxTypeFare().empty())
    {
      continue;
    }

    if (!cruisePfaCurrency.empty() && ((paxType == PFA) || (paxType == JCB)))
    {
      // Cruise PFA Currency does not go through currency validation.
      addCurrencyToFareMarket(direction, paxTypeCortege, cruisePfaCurrency);
      continue;
    }

    validRC = _csValidator.validate(trx,
                                    paxTypeCortege,
                                    fareMarket,
                                    itin,
                                    nationCode,
                                    ticketingDate,
                                    fareCompPrimeCurrency,
                                    fareQuoteOverrideCurrency,
                                    firstCurrency,
                                    paxTypeSearchFailed,
                                    nationNotFound,
                                    isInternational);

    // If Passenger Type validation failed or Nation not in Currency Selection table
    // Then use a blank nation
    if (paxTypeSearchFailed || nationNotFound)
    {
      validRC = _csValidator.validate(trx,
                                      paxTypeCortege,
                                      fareMarket,
                                      itin,
                                      blankNation,
                                      ticketingDate,
                                      fareCompPrimeCurrency,
                                      fareQuoteOverrideCurrency,
                                      firstCurrency,
                                      paxTypeSearchFailed,
                                      nationNotFound,
                                      isInternational);
    }

    if (!validRC)
    {
      validRC = nationValidations(paxTypeCortege,
                                  fareMarket,
                                  direction,
                                  nationCode,
                                  ticketingDate,
                                  firstCurrency,
                                  fareCompPrimeCurrency,
                                  isInternational);
    }

    //-------------------------------------------------------------
    // Check Result of Prime Currency Selection for PaxTypeBucket
    //-------------------------------------------------------------
    // bool privateFaresRequested = trx.getOptions()->isPrivateFares();
    if (validRC)
    {
      foundOutBoundCurrency = true;

      if (!fareQuoteOverrideCurrency.empty())
      {
        setFQOverride(trx,
                      fareQuoteOverrideCurrency,
                      fareCompPrimeCurrency,
                      paxTypeCortege,
                      paxType,
                      direction,
                      fareMarket,
                      nationCode);
      }
      else
      {
        if (faresThisCurrency(trx, fareCompPrimeCurrency, paxType, direction, fareMarket))
        {
          addCurrencyToFareMarket(direction, paxTypeCortege, fareCompPrimeCurrency);
        }
        else
        {
          findPaxTypeFareCurrency(
              fareMarket, direction, nationCode, paxTypeCortege, privateFaresRequested);
        }
      }
    }

    // No Prime Currency Selected.  Find a currency from a PaxTypeFare
    else
    {
      findPaxTypeFareCurrency(
          fareMarket, direction, nationCode, paxTypeCortege, privateFaresRequested);
    }

  } // for all paxTypeCorteges

  // Return true if the outbound currency was selected for at least one of the paxTypeCorteges
  if (foundOutBoundCurrency)
  {
    validRC = true;
  }

  LOG4CXX_DEBUG(
      logger,
      "Leaving FDFDFareCurrencySelection::selectPrimeCurrency for Foreign Domestic/International");

  return validRC;
}

//------------------------------------------------------------------------------
//
//   @method findMatchingPaxTypeFare
//
//   Description: Looks for a fare in fare market with matching direction
//                and currency within the given paxTypeCortege.
//
//
//  @return PaxTypeFare * - pointer to Pax type Fare
//
//------------------------------------------------------------------------------
const PaxTypeFare*
FDFareCurrencySelection::findMatchingPaxTypeFare(PaxTypeBucket& paxTypeCortege,
                                                 NationCode& nationCode,
                                                 bool privateFaresRequested)
{
  LOG4CXX_DEBUG(logger, "Entered findMatchingPaxTypeFare");

  const PaxTypeFare* nationPaxFare = nullptr;

  PaxTypeFarePtrVec& paxTypeFareVec = paxTypeCortege.paxTypeFare();
  uint16_t paxVecSize = paxTypeFareVec.size();

  for (uint16_t i = 0; i < paxVecSize; i++)
  {
    PaxTypeFare* tmpPaxFare = paxTypeFareVec[i];

    if (!tmpPaxFare->isValid() || tmpPaxFare->carrier() == INDUSTRY_CARRIER)
    {
      continue;
    }

    if (privateFaresRequested && tmpPaxFare->tcrTariffCat() != RuleConst::PRIVATE_TARIFF)
    {
      continue;
    }

    if ((tmpPaxFare->fare()->directionality() == BOTH))
    {
      nationPaxFare = tmpPaxFare;
      break;
    }
    else if ((tmpPaxFare->fare()->directionality() == FROM))
    {
      nationPaxFare = tmpPaxFare;
      break;
    }
  }

  LOG4CXX_DEBUG(logger, "Leaving findMatchingPaxTypeFare");
  return nationPaxFare;
}

//------------------------------------------------------------------------------
//
//   @method findPaxTypeFareCurrency
//
//   Description: findCurrency to set for PaxTypeBucket
//
//------------------------------------------------------------------------------
void
FDFareCurrencySelection::findPaxTypeFareCurrency(FareMarket& fareMarket,
                                                 const Directionality& direction,
                                                 NationCode& nationCode,
                                                 PaxTypeBucket& paxTypeCortege,
                                                 bool privateFaresRequested)
{

  LOG4CXX_DEBUG(logger, "Entered findPaxTypeFareCurrency");

  const PaxTypeFare* tmpPaxTypeFare = nullptr;

  tmpPaxTypeFare = findMatchingPaxTypeFare(paxTypeCortege, nationCode, privateFaresRequested);

  if (tmpPaxTypeFare)
  {
    addCurrencyToFareMarket(tmpPaxTypeFare->fare()->directionality(),
                            paxTypeCortege,
                            tmpPaxTypeFare->fare()->currency());
  }

  LOG4CXX_DEBUG(logger, "Leaving findPaxTypeFareCurrency");
}

//------------------------------------------------------------------------------
//
//   @method setFQOverride
//
//   Description: checkFQOverride field from Currency Selection Table
//
//------------------------------------------------------------------------------
void
FDFareCurrencySelection::setFQOverride(FareDisplayTrx& trx,
                                       CurrencyCode& fqCurrencyOverride,
                                       CurrencyCode& fareCompPrimeCurrency,
                                       PaxTypeBucket& paxTypeCortege,
                                       const PaxTypeCode& paxType,
                                       const Directionality& direction,
                                       FareMarket& fareMarket,
                                       NationCode& nationCode)
{

  LOG4CXX_DEBUG(logger, "setFQOverride");

  if (faresThisCurrency(trx, fqCurrencyOverride, paxType, direction, fareMarket))
  {
    fareCompPrimeCurrency = fqCurrencyOverride;
  }
  else
  {
    bool privateFaresRequested = trx.getOptions()->isPrivateFares();
    const PaxTypeFare* tmpPaxTypeFare =
        findMatchingPaxTypeFare(paxTypeCortege, nationCode, privateFaresRequested);

    if (tmpPaxTypeFare)
    {
      fareCompPrimeCurrency = tmpPaxTypeFare->fare()->currency();
    }
  }

  // Set currency override to the FQ Override
  if (trx.getOptions()->currencyOverride().empty() && fareCompPrimeCurrency == fqCurrencyOverride)
  {
    paxTypeCortege.fareQuoteOverrideCurrency() = fqCurrencyOverride;
  }

  addCurrencyToFareMarket(direction, paxTypeCortege, fareCompPrimeCurrency);

  LOG4CXX_DEBUG(logger, "Leaving setFQOverride");
}

//------------------------------------------------------------------------------
//
//   @method setFQOverride
//
//   Description: faresThisCurrency   Determine whether fares exist in the
//                                    given currency based on the key.
//
//                                    Don't check directionality for Circle Trip
//                                    or Round the World Fares (Same City Fares)
//
//------------------------------------------------------------------------------
bool
FDFareCurrencySelection::faresThisCurrency(FareDisplayTrx& trx,
                                           CurrencyCode& currency,
                                           const PaxTypeCode& paxType,
                                           const Directionality& direction,
                                           FareMarket& fareMarket)
{

  if (trx.isSameCityPairRqst())
  {
    CTRWCurrencyKey ccKey(paxType, currency);

    CurrencyCacheI iter = find_if(
        fareMarket.currencies().begin(), fareMarket.currencies().end(), hasMatchingCTRWKey(ccKey));

    if (iter != fareMarket.currencies().end())
    {
      return true;
    }
  }
  else
  {
    FareMarketCurrencyKey ccKey2(paxType, direction, currency);

    CurrencyCacheI iter = find_if(
        fareMarket.currencies().begin(), fareMarket.currencies().end(), hasMatchingFDKey(ccKey2));
    if (iter != fareMarket.currencies().end())
    {
      return true;
    }
  }

  return false;
}

//---------------------------------------------------------------------------
// Determine Display Currency
//---------------------------------------------------------------------------
bool
FDFareCurrencySelection::getDisplayCurrency(FareDisplayTrx& trx, CurrencyCode& displayCurrency)
{

  displayCurrency = US_DOLLARS;

  if (trx.getRequest() == nullptr || trx.getOptions() == nullptr)
  {
    return false;
  }

  if (!trx.getOptions()->currencyOverride().empty())
  {
    displayCurrency = trx.getOptions()->currencyOverride();
    return true;
  }
  else if (checkFareQuoteOverride(trx, displayCurrency))
  {
    return true;
  }

  if (!trx.getOptions()->alternateCurrency().empty())
  {
    displayCurrency = trx.getOptions()->alternateCurrency();
    return true;
  }

  if (trx.getOptions()->isSellingCurrency())
  {
    getSellingCurrency(trx, displayCurrency);
    return true;
  }

  if (!trx.getRequest()->ticketingAgent()->currencyCodeAgent().empty())
  {
    displayCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();
  }

  return true;
}

//---------------------------------------------------------------------------
// Apply the FareQuote override as the display currency only when it exists
// for all paxTypes requested.
//---------------------------------------------------------------------------
bool
FDFareCurrencySelection::checkFareQuoteOverride(FareDisplayTrx& trx, CurrencyCode& displayCurrency)
{

  CurrencyCode overrideCurrency;
  bool currencyOverrideMissing = false;

  // If the Fare Quote Override Currency exists for each paxTypeCortege that has valid fares,
  // in each FareMarket, then apply it as the display currency.  Otherwise, ignore it.

  Itin* itin = trx.itin().front();
  std::vector<FareMarket*>::const_iterator fmItr = itin->fareMarket().begin();
  std::vector<FareMarket*>::const_iterator fmEnd = itin->fareMarket().end();

  for (; fmItr != fmEnd; fmItr++)
  {
    FareMarket& fareMarket = **fmItr;

    std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();
    std::vector<PaxTypeBucket>::iterator paxTypeCortI = paxTypeCortegeVec.begin();
    std::vector<PaxTypeBucket>::iterator paxTypeCortEnd = paxTypeCortegeVec.end();

    for (; paxTypeCortI < paxTypeCortEnd; paxTypeCortI++)
    {
      PaxTypeBucket& paxTypeCortege = (*paxTypeCortI);
      if (validFaresInCortege(paxTypeCortege) && paxTypeCortege.fareQuoteOverrideCurrency().empty())
      {
        currencyOverrideMissing = true;
        continue;
      }
      else if (validFaresInCortege(paxTypeCortege) &&
               !paxTypeCortege.fareQuoteOverrideCurrency().empty())
      {
        overrideCurrency = paxTypeCortege.fareQuoteOverrideCurrency();
      }
    }
  }

  if (!overrideCurrency.empty() && !currencyOverrideMissing)
  {
    displayCurrency = overrideCurrency;
    return true;
  }

  return false;
}

//---------------------------------------------------------------------------
// Apply the FareQuote override as the display currency only when it exists
// for all paxTypes requested.
//---------------------------------------------------------------------------
bool
FDFareCurrencySelection::validFaresInCortege(PaxTypeBucket& paxTypeCortege)
{
  bool foundValidFare = false;

  PaxTypeFarePtrVec& paxTypeFareVec = paxTypeCortege.paxTypeFare();
  std::vector<tse::PaxTypeFare*>::const_iterator paxTypeFareI = paxTypeFareVec.begin();
  std::vector<tse::PaxTypeFare*>::const_iterator paxTypeFareEnd = paxTypeFareVec.end();

  for (; paxTypeFareI < paxTypeFareEnd; paxTypeFareI++)
  {
    PaxTypeFare& paxTypeFare = **paxTypeFareI;

    if (paxTypeFare.isValid())
    {
      // Found at least 1 valid fare in this cortege
      foundValidFare = true;
      break;
    }
  }

  if (foundValidFare)
  {
    return true;
  }

  return false;
}

//--------------------------------
// Get the first valid PaxTypeFare
//--------------------------------
bool
validPaxTypeFare(PaxTypeFare* paxTypeFare)
{
  return paxTypeFare->isValid();
}

//---------------------------------------------------------------------------
// Get the Selling Currency
//---------------------------------------------------------------------------
bool
FDFareCurrencySelection::getSellingCurrency(FareDisplayTrx& trx, CurrencyCode& displayCurrency)
{

  Itin* itin = trx.itin().front();
  FareMarket* fareMarket = itin->fareMarket().front();

  if (!trx.multipleCurrencies())
  {
    std::vector<FareMarket*>::const_iterator fmItr = itin->fareMarket().begin();
    std::vector<FareMarket*>::const_iterator fmEnd = itin->fareMarket().end();
    for (; fmItr != fmEnd; fmItr++)
    {
      FareMarket& fareMarket = **fmItr;
      if (!fareMarket.allPaxTypeFare().empty())
      {
        std::vector<PaxTypeFare*>::const_iterator i =
            std::find_if(fareMarket.allPaxTypeFare().begin(),
                         fareMarket.allPaxTypeFare().end(),
                         validPaxTypeFare);

        if (i != fareMarket.allPaxTypeFare().end())
        {
          displayCurrency = (*i)->currency();
          return true;
        }
      }
    }
  }

  // Multiple Currencies exist - select display currency
  DateTime& ticketingDate = trx.getRequest()->ticketingDT();
  NationCode nationCode;
  NationCode blankNation;
  CurrencyCode nationCurrency;
  const NationCode& nation = fareMarket->origin()->loc();
  getNationCode(trx, nation, ticketingDate, nationCode);
  bool isInternational = (itin->geoTravelType() == GeoTravelType::International);

  bool validRC = getNationCurrencyFD(nationCode, nationCurrency, isInternational, ticketingDate);

  if (validRC)
  {
    displayCurrency = nationCurrency;
    return true;
  }

  validRC = getNationCurrencyFD(blankNation, nationCurrency, isInternational, ticketingDate);

  if (validRC)
  {
    displayCurrency = nationCurrency;
    return true;
  }

  return false;
}

//---------------------------------------------------------------------
//
//   @method getNationCurrencyFD
//
//   @param NationCode   - nation
//   @param CurrencyCode - return parameter of nation prime currency
//
//   @return bool - true - nation currency retrieved , else false
//---------------------------------------------------------------------
bool
FDFareCurrencySelection::getNationCurrencyFD(NationCode& nationCode,
                                             CurrencyCode& nationCurrency,
                                             bool isInternational,
                                             DateTime& ticketingDate)
{
  DataHandle dataHandle(ticketingDate);
  const tse::Nation* nation = dataHandle.getNation(nationCode, ticketingDate);
  if (!nation)
  {
    LOG4CXX_DEBUG(logger, "Unable to retrieve Nation from cache " << nationCode);
    return false;
  }

  if (isInternational)
  {
    // 1. Pricing Currency
    // 2. Bank Rate Currency
    // 3. National Currency
    if (!nation->alternateCur().empty())
    {
      LOG4CXX_DEBUG(logger, "Using pricing currency: " << nation->alternateCur());
      nationCurrency = nation->alternateCur();
      return true;
    }
    if (!nation->conversionCur().empty())
    {
      LOG4CXX_DEBUG(logger, "Using conversion currency: " << nation->conversionCur());
      nationCurrency = nation->conversionCur();
      return true;
    }
    if (!nation->primeCur().empty())
    {
      LOG4CXX_DEBUG(logger, "Using national currency: " << nation->primeCur());
      nationCurrency = nation->primeCur();
      return true;
    }
  }
  else
  {
    // 1. Pricing Currency
    // 2. National Currency
    // 3. Bank Rate Currency
    if (!nation->alternateCur().empty())
    {
      LOG4CXX_DEBUG(logger, "Using pricing currency: " << nation->alternateCur());
      nationCurrency = nation->alternateCur();
      return true;
    }
    if (!nation->primeCur().empty())
    {
      LOG4CXX_DEBUG(logger, "Using national currency: " << nation->primeCur());
      nationCurrency = nation->primeCur();
      return true;
    }
    if (!nation->conversionCur().empty())
    {
      LOG4CXX_DEBUG(logger, "Using conversion currency: " << nation->conversionCur());
      nationCurrency = nation->conversionCur();
      return true;
    }
  }
  return false;
}

} // tse
