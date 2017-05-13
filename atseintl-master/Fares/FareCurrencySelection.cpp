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
#include "Fares/FareCurrencySelection.h"

#include "Common/ErrorResponseException.h"
#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Currency.h"
#include "DBAccess/CurrencySelection.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Fares/CurrencySelectionValidator.h"
#include "Rules/RuleConst.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <vector>

#include <time.h>

namespace tse
{
static Logger
logger("atseintl.Fares.FareCurrencySelection");

const CurrencyCode
FareCurrencySelection::USD("USD");
const CurrencyCode
FareCurrencySelection::CAD("CAD");

typedef std::set<CurrencyCode>::iterator CurCacheI;

class FareCurrencySelection::UniqueCurrencies
    : public std::unary_function<const FareMarketCurrencyKey&, bool>
{
public:
  UniqueCurrencies(const FareMarketCurrencyKey& key) : _key(key) {}

  bool operator()(const FareMarketCurrencyKey& key) const
  {
    if (UNLIKELY(_key.directionality() == BOTH))
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

}; // lint !e1509

class hasMatchingKey
{
public:
  hasMatchingKey(const FareMarketCurrencyKey& key) : _key(key) {}

  // This should only be used if there is one currency per
  // passenger type in the market
  //
  bool operator()(const FareMarketCurrencyKey& key) const
  {
    return (_key.paxTypeCode() == key.paxTypeCode() &&
            _key.directionality() == key.directionality());
  }

private:
  const FareMarketCurrencyKey& _key;
};

//--------------------------------------------------------------
//
//   @method FareCurrencySelection
//
//--------------------------------------------------------------
tse::FareCurrencySelection::FareCurrencySelection() : _diag(nullptr) {}

//---------------------------------------------------------------
//
//   @method ~FareCurrencySelection
//
//---------------------------------------------------------------
tse::FareCurrencySelection::~FareCurrencySelection() {}

//--------------------------------------------------------------------------------------------
//
//   @method selectPrimeCurrency
//
//   Description: Performs validation on each foreign domestic fare component
//                to determine the currency for fare selection.
//
//   @param PricingTrx         - transaction object
//   @param FareMarket         - fare market
//   @param GeoTravelType      - travel type of itinerary
//   @param bool               - determine out bound currency only - true, else false
//
//   @return bool  - true/false, false is failure
//
//--------------------------------------------------------------------------------------------
bool
tse::FareCurrencySelection::selectPrimeCurrency(PricingTrx& trx,
                                                FareMarket& fareMarket,
                                                Itin& itin,
                                                bool determineOutBoundOnly)
{
  CurrencyCode fareCompPrimeCurrency;
  CurrencyCode fareQuoteOverrideCurrency;
  NationCode nationCode;
  NationCode blankNation;
  bool validRC = false;
  bool getNationRC = false;
  bool paxTypeSearchFailed = false;
  CurrencyCode firstCurrency;
  bool nationNotFound = false;
  bool foundInBoundCurrency = false;
  bool foundOutBoundCurrency = false;
  bool isInternational = false;

  LOG4CXX_DEBUG(logger, "Entered selectPrimeCurrency for Foreign Domestic/International");

  GeoTravelType& itinTravelType = itin.geoTravelType();

  if (itinTravelType == GeoTravelType::International)
    isInternational = true;

  DateTime& ticketingDate = trx.getRequest()->ticketingDT();

  LOG4CXX_DEBUG(logger, "Retrieved Fare Market");
  LOG4CXX_DEBUG(logger, "Faremarket GEO Travel Type: " << static_cast<int>(fareMarket.geoTravelType()));
  LOG4CXX_DEBUG(logger, "Faremarket Origin: " << fareMarket.origin()->loc());
  LOG4CXX_DEBUG(logger, "Faremarket Destination: " << fareMarket.destination()->loc());

  const Customer* agentTJR = trx.getRequest()->ticketingAgent()->agentTJR();
  const std::string cruisePfaCurrency = ((agentTJR != nullptr) ? agentTJR->cruisePfaCur() : "");

  determineCurrenciesInFareMarket(fareMarket, cruisePfaCurrency, trx, itinTravelType);

  createDiag(trx, fareMarket, itinTravelType, SELECT_PRIME_FUNC);

  std::vector<NationCode> nations;
  nations.push_back(fareMarket.origin()->loc());
  nations.push_back(fareMarket.destination()->loc());

  PaxTypeBucketVec& paxTypeCortegeVec = fareMarket.paxTypeCortege();

  unsigned int paxTypeCortegeVecSize = paxTypeCortegeVec.size();

  LOG4CXX_DEBUG(logger, "Size of Pax Type Cortege vector: " << paxTypeCortegeVecSize);

  for (unsigned int paxTypeCortegeCntr = 0; paxTypeCortegeCntr < paxTypeCortegeVecSize;
       paxTypeCortegeCntr++)
  {
    PaxTypeBucket& paxTypeCortege = paxTypeCortegeVec[paxTypeCortegeCntr];

    const tse::PaxType* requestedPaxType = paxTypeCortege.requestedPaxType();
    const tse::PaxTypeCode paxType = requestedPaxType->paxType();

    LOG4CXX_DEBUG(logger, "Requested pax type = " << paxType);

    if (UNLIKELY(_diag))
    {
      (*_diag) << " ----------------------------------------------------------- \n"
               << " PAXTYPECORTEGE REQUESTED PAXTYPE: " << paxType << " CUR: " << cruisePfaCurrency
               << "\n";
    }

    if (UNLIKELY(!cruisePfaCurrency.empty() &&
                 ((paxType == JCB) || (paxType == PFA) || (paxType == CBC) || (paxType == CBI))))
    {
      // Cruise PFA Currency does not go through currency validation.
      diagResult(paxTypeCortege);
      continue;
    }

    // Determine currency for Fare Market Origin and Destination
    //
    for (int i = 0; i < 2; i++)
    {
      const tse::Directionality direction = (i == 0 ? FROM : TO);

      getNationRC = getNationCode(trx, nations[i], ticketingDate, nationCode);

      diagNationCode(nationCode, getNationRC);

      if (UNLIKELY(!getNationRC))
        continue;
      else
        getNationRC = false;

      // Always use FROM direction for origin and TO for destination
      //
      FareMarketCurrencyKey ccKey(paxType, direction);

      int numCurrencies = count_if(
          fareMarket.currencies().begin(), fareMarket.currencies().end(), UniqueCurrencies(ccKey));

      if (numCurrencies == 0)
      {
        ccKey.paxTypeCode() = EMPTY_PAXTYPE_CODE;
        numCurrencies = count_if(fareMarket.currencies().begin(),
                                 fareMarket.currencies().end(),
                                 UniqueCurrencies(ccKey));
      }

      /*
               CurrencyIter it  = fareMarket.currencies().begin();
               CurrencyIter end = fareMarket.currencies().end();

               for (; it != end; it++)
               {
                  const FareMarketCurrencyKey& ccKey = *it;
                  LOG4CXX_DEBUG(logger, "Currency Cache pax type is: " << ccKey.paxTypeCode());
                  LOG4CXX_DEBUG(logger, "Currency Cache directionality is: " <<
         ccKey.directionality());
                  LOG4CXX_DEBUG(logger, "Currency Cache currency is: " << ccKey.currencyCode());
               }
      */

      LOG4CXX_DEBUG(logger, "Number of currencies in market : " << numCurrencies);

      diagCurr(fareMarket, numCurrencies);

      if (numCurrencies > 1)
      {
        if (itinTravelType == GeoTravelType::ForeignDomestic)
        {
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
                                          false);

          // If Passenger Type validation failed or Nation not in Currency Selection table
          // Then use a blank nation
          //
          if (paxTypeSearchFailed || nationNotFound)
          {
            LOG4CXX_DEBUG(logger, "Pax type search failed,searching on blank nation");

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
                                            false);

            paxTypeSearchFailed = false;
            nationNotFound = false;
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
                                        false);
            if (!validRC)
            {
              LOG4CXX_DEBUG(logger, "Nations validations failed");
            }
          }

          LOG4CXX_DEBUG(logger,
                        "Foreign Domestic validation returned : " << validRC << " currency = "
                                                                  << fareCompPrimeCurrency);

          if (paxTypeCortegeCntr > 0)
          {
            if (!(validRC = isCurrencySameAsPrevious(
                      direction, fareCompPrimeCurrency, paxTypeCortegeVec)))
            {
              LOG4CXX_DEBUG(logger, "Unable To  Price Issue Separate Tickets");
            }
          }

          LOG4CXX_DEBUG(logger, "SelectPrimeForeignDomesticCurrency returned: " << validRC);
        }
        else if (itinTravelType == GeoTravelType::International)
        {
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
                                          true);

          // If Passenger Type validation failed or Nation not in Currency Selection table
          // Then use a blank nation
          //
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
                                            true);
            paxTypeSearchFailed = false;
            nationNotFound = false;
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
                                        true);
            if (!validRC)
              LOG4CXX_DEBUG(logger, "Nations validations failed");
          }

          LOG4CXX_DEBUG(logger,
                        "International Validation returned : " << validRC << " currency = "
                                                               << fareCompPrimeCurrency);
        }

      } // number of currencies in market > 1

      if (validRC)
      {
        LOG4CXX_DEBUG(logger,
                      "Validation succeeded : " << validRC << " fare component prime currency = "
                                                << fareCompPrimeCurrency);

        addCurrencyToFareMarket(direction, paxTypeCortege, fareCompPrimeCurrency);

        if (direction == FROM)
          foundOutBoundCurrency = true;
        else if (direction == TO)
          foundInBoundCurrency = true;

        fareCompPrimeCurrency.clear();

        validRC = false;
      }
      else
      {
        LOG4CXX_DEBUG(logger, "Adding currency: " << fareCompPrimeCurrency);

        if (numCurrencies == 1)
        {
          CurrencyCacheI iter = find_if(fareMarket.currencies().begin(),
                                        fareMarket.currencies().end(),
                                        hasMatchingKey(ccKey));

          if (LIKELY(iter != fareMarket.currencies().end()))
            fareCompPrimeCurrency = (*iter).currencyCode();

          addCurrencyToFareMarket(direction, paxTypeCortege, fareCompPrimeCurrency);

          if (direction == FROM)
            foundOutBoundCurrency = true;
          else if (LIKELY(direction == TO))
            foundInBoundCurrency = true;

          fareCompPrimeCurrency.clear();
        }
        else
        {
          const PaxTypeFare* tmpPaxTypeFare = nullptr;

          tmpPaxTypeFare = findMatchingPaxTypeFare(
              fareMarket, direction, nationCode, isInternational, trx.ticketingDate());

          if (tmpPaxTypeFare)
          {
            addCurrencyToFareMarket(tmpPaxTypeFare->fare()->directionality(),
                                    paxTypeCortege,
                                    tmpPaxTypeFare->fare()->currency());

            if (direction == FROM)
              foundOutBoundCurrency = true;
            else if (direction == TO)
              foundInBoundCurrency = true;
          }
        }
      }

      doDiag(foundOutBoundCurrency, foundInBoundCurrency);

      diagNUCRate(trx, paxTypeCortege, foundOutBoundCurrency, foundInBoundCurrency);

      if (foundInBoundCurrency && foundOutBoundCurrency)
      {
        foundInBoundCurrency = false;
        foundOutBoundCurrency = false;
        break;
      }
      else if (itin.tripCharacteristics().isSet(Itin::OneWay) && (itin.travelSeg().size() == 1))
      {
        foundInBoundCurrency = true;
        foundOutBoundCurrency = true;
        break;
      }

    } // for origin nation and dest nation

  } // for all PaxTypeBucket's

  if (itinTravelType == GeoTravelType::International)
  {
    nationCode.clear();

    if (fareMarket.direction() == FMDirection::OUTBOUND)
    {
      if (LIKELY(fareMarket.origin()))
        nationCode = fareMarket.origin()->nation();
    }
    else if (fareMarket.direction() == FMDirection::INBOUND)
    {
      if (fareMarket.destination())
        nationCode = fareMarket.destination()->nation();
    }
    else if (fareMarket.direction() == FMDirection::UNKNOWN)
    {
      if (fareMarket.origin())
        nationCode = fareMarket.origin()->nation();
      else if (fareMarket.destination())
        nationCode = fareMarket.destination()->nation();
    }

    if (LIKELY(!nationCode.empty()))
    {
      _csValidator.getAseanCurrencies(trx, nationCode, fareMarket);
      diagAsean(fareMarket);
    }
  }

  if (foundInBoundCurrency && foundOutBoundCurrency)
    validRC = true;

  LOG4CXX_DEBUG(logger, "Leaving selectPrimeCurrency for Foreign Domestic/International");
  endDiag(SELECT_PRIME_FUNC, validRC);

  return validRC;
}

//--------------------------------------------------------------------------------------------
//
//   @method selectAlternateCurrency
//
//   Description: Performs validations on each alternate currency pricing entry
//                for an international fare component to determine the currency for fare
//                selection.
//
//   @param PaxTypeFare        - Fare market data
//   @param NationCode         - code representing nation , used to lookup,
//                               currency information in Nations table.
//
//   @return bool  - true/false, false is failure
//
//
//--------------------------------------------------------------------------------------------
bool
tse::FareCurrencySelection::selectAlternateCurrency(PricingTrx& trx,
                                                    FareMarket& fareMarket,
                                                    CurrencyCode& alternateCurrency,
                                                    GeoTravelType& itinTravelType)
{
  createDiag(trx, fareMarket, itinTravelType, SELECT_ALTERNATE_FUNC);

  CurrencyCode fareCompPrimeCurrency;
  NationCode nationCode;
  NationCode blankNation;
  bool isInternational = false;
  bool validRC = false;
  bool validOverride = true;
  bool foundInBoundCurrency = false;
  bool foundOutBoundCurrency = false;
  bool paxTypeSearchFailed = false;
  bool nationNotFound = false;

  LOG4CXX_DEBUG(logger, "Entered selectAlternateCurrency for Foreign Domestic/International");

  DateTime& ticketingDate = trx.getRequest()->ticketingDT();

  LOG4CXX_DEBUG(logger, "Retrieved Fare Market");
  LOG4CXX_DEBUG(logger, "Faremarket GEO Travel Type: " << static_cast<int>(fareMarket.geoTravelType()));
  LOG4CXX_DEBUG(logger, "Faremarket Origin: " << fareMarket.origin()->loc());
  LOG4CXX_DEBUG(logger, "Faremarket Destination: " << fareMarket.destination()->loc());

  PaxTypeBucketVec& paxTypeCortegeVec = fareMarket.paxTypeCortege();

  unsigned int paxTypeCortegeVecSize = paxTypeCortegeVec.size();
  std::vector<NationCode> nations;
  nations.push_back(fareMarket.origin()->loc());
  nations.push_back(fareMarket.destination()->loc());

  for (unsigned int paxTypeCortegeCntr = 0; paxTypeCortegeCntr < paxTypeCortegeVecSize;
       paxTypeCortegeCntr++)
  {
    PaxTypeBucket& paxTypeCortege = paxTypeCortegeVec[paxTypeCortegeCntr];

    for (unsigned int i = 0; i < 2; i++)
    {
      const tse::Directionality direction = (i == 0 ? FROM : TO);

      // getNationRC =
      getNationCode(trx, nations[i], ticketingDate, nationCode);

      if (itinTravelType == GeoTravelType::International)
        isInternational = true;

      validRC = _csValidator.validate(trx,
                                      paxTypeCortege,
                                      fareMarket,
                                      nationCode,
                                      ticketingDate,
                                      alternateCurrency,
                                      fareCompPrimeCurrency,
                                      paxTypeSearchFailed,
                                      nationNotFound,
                                      validOverride,
                                      isInternational);

      // If Passenger Type validation failed or Nation not in Currency Selection table
      // Then use a blank nation
      //
      if (paxTypeSearchFailed || nationNotFound)
      {
        LOG4CXX_DEBUG(logger, "Pax type search failed,searching on blank nation");
        validRC = _csValidator.validate(trx,
                                        paxTypeCortege,
                                        fareMarket,
                                        blankNation,
                                        ticketingDate,
                                        alternateCurrency,
                                        fareCompPrimeCurrency,
                                        paxTypeSearchFailed,
                                        nationNotFound,
                                        validOverride,
                                        isInternational);

        paxTypeSearchFailed = false;
        nationNotFound = false;
      }

      if (!validRC)
      {
        ErrorResponseException ex(tse::ErrorResponseException::FARE_CURRENCY_OVERRIDE_NOT_VALID);
        throw ex;
      }

      if (paxTypeCortegeCntr > 0)
      {
        if (!(validRC =
                  isCurrencySameAsPrevious(direction, fareCompPrimeCurrency, paxTypeCortegeVec)))
        {
          LOG4CXX_DEBUG(logger, "Unable To  Price Issue Separate Tickets");
        }
      }

      if (validRC)
      {
        addCurrencyToFareMarket(direction, paxTypeCortege, alternateCurrency);

        if (direction == FROM)
          foundOutBoundCurrency = true;
        else if (direction == TO)
          foundInBoundCurrency = true;

        validRC = false;
      }
      else
      {
        if (validOverride)
        {
          const PaxTypeFare* tmpPaxTypeFare = nullptr;

          tmpPaxTypeFare = findMatchingPaxTypeFare(
              fareMarket, direction, nationCode, isInternational, trx.ticketingDate());

          if (tmpPaxTypeFare)
          {
            addCurrencyToFareMarket(direction, paxTypeCortege, tmpPaxTypeFare->fare()->currency());
            if (direction == FROM)
              foundOutBoundCurrency = true;
            else if (direction == TO)
              foundInBoundCurrency = true;
          }
        }
        else
        {
          ErrorResponseException ex(tse::ErrorResponseException::FARE_CURRENCY_OVERRIDE_NOT_VALID);
          throw ex;
        }
      }

      doDiag(foundOutBoundCurrency, foundInBoundCurrency);

      diagNUCRate(trx, paxTypeCortege, foundOutBoundCurrency, foundInBoundCurrency);

      if (foundInBoundCurrency && foundOutBoundCurrency)
      {
        foundInBoundCurrency = false;
        foundOutBoundCurrency = false;
        break;
      }

    } // for each faremarket origin and destination

  } // for all PaxTypeBucket's

  if (foundInBoundCurrency && foundOutBoundCurrency)
    validRC = true;

  LOG4CXX_DEBUG(logger, "Leaving selectAlternateCurrency for Foreign Domestic/International");
  endDiag(SELECT_ALTERNATE_FUNC, validRC);
  return validRC;
}

//--------------------------------------------------------------------------------------------
//
//   @method addCurrencyToFareMarket
//
//   Description: Adds a Currency to either the inbound or outbound currency on
//                PaxTypeBucket.
//
//   @param Direction        -  either TO/FROM
//   @param PaxTypeBucket   -  contains PaxTypeFares and inbound/outbound
//                                 CurrencyCodes.
//   @param CurrencyCode     -  currency code
//
//   @return void
//--------------------------------------------------------------------------------------------
void
tse::FareCurrencySelection::addCurrencyToFareMarket(Directionality direction,
                                                    PaxTypeBucket& paxTypeCortege,
                                                    const CurrencyCode& currencyCode,
                                                    bool isTransBorder)
{
  LOG4CXX_DEBUG(logger, "going to add Currency " << currencyCode);

  if (isTransBorder)
  {
    LOG4CXX_DEBUG(logger, "Adding Transborder Currency: " << currencyCode);

    paxTypeCortege.outboundCurrency() = currencyCode;
    paxTypeCortege.inboundCurrency() = currencyCode;
  }
  else
  {
    if (direction == FROM)
    {
      paxTypeCortege.outboundCurrency() = currencyCode;
      LOG4CXX_DEBUG(logger,
                    "Added Currency " << paxTypeCortege.outboundCurrency()
                                      << " to Outbound Currency");
    }
    else if (direction == TO)
    {
      paxTypeCortege.inboundCurrency() = currencyCode;
      LOG4CXX_DEBUG(
          logger, "Added Currency " << paxTypeCortege.inboundCurrency() << " to Inbound Currency");
    }
    else if (direction == BOTH)
    {
      LOG4CXX_DEBUG(logger, "Adding Foreign Domestic Currency: " << currencyCode);

      paxTypeCortege.outboundCurrency() = currencyCode;
      paxTypeCortege.inboundCurrency() = currencyCode;
    }
  }
  diagResult(paxTypeCortege);
}

//--------------------------------------------------------------------------------------------
//
//   @method getNationCode
//
//   Description: Retrieves Nation code for a Loc
//
//   @param PricingTrx         - Transaction object
//   @param LocCode            - market
//   @param DateTime           -  travel date
//   @param NationCode         - return parameter ode representing nation , used to lookup,
//                               currency information in Nations table.
//
//   @return bool  - true  - operation completed successfully, false failure
//
//--------------------------------------------------------------------------------------------
bool
tse::FareCurrencySelection::getNationCode(PricingTrx& trx,
                                          const LocCode market,
                                          DateTime& date,
                                          NationCode& nationCode)
{
  const tse::Loc* loc = trx.dataHandle().getLoc(market, date);

  if (LIKELY(loc))
  {
    nationCode = loc->nation();
    LOG4CXX_DEBUG(logger, "Nation code for market is: " << nationCode);
    return true;
  }
  else
  {
    LOG4CXX_DEBUG(logger, "Failed to retrieve nation");
    return false;
  }
}
//--------------------------------------------------------------------------------------------
//
//   @method isCurrencySameAsPrevious
//
//   Description: Checks to see for foreign domestic itineraries that the current
//                currency is the same as the previous. The first
//                fare component determines the currency.
//
//   @param Directionality     - Used to determine which currency code to compare
//                               with. The PaxTypeBucket class stores inbound
//                               and outbound currencies.
//
//   @param CurrencyCode       - currency to compare with previous
//
//   @param PaxTypeBucketVec  - Used to select first fare component information.
//
//   @return bool  - true  - operation completed successfully, false failure
//
//   @return bool  - true  - operation completed successfully, false failure
//
//--------------------------------------------------------------------------------------------
bool
tse::FareCurrencySelection::isCurrencySameAsPrevious(Directionality direction,
                                                     CurrencyCode& currency,
                                                     PaxTypeBucketVec& paxTypeCortegeVec)
{

  if (direction == FROM)
  {
    if (currency != paxTypeCortegeVec[0].outboundCurrency())
    {
      diagMsg(OUTBOUND_CURR_DIFF);
      return false;
    }
  }
  else if (direction == TO)
  {
    if (currency != paxTypeCortegeVec[0].inboundCurrency())
    {
      diagMsg(INBOUND_CURR_DIFF);
      return false;
    }
  }

  return true;
}

//--------------------------------------------------------------------------------------------
//
//   @method selectUSCACurrency
//
//   Description: Selects the currency code for US and CA trans border travel.
//
//   @param PricingTrx                - transaction
//   @param NationCode         - code representing origin nation.
//   @param FareMarketVec      - vector of FareMarkets.
//
//   @return bool  - true/false, false is failure
//
//--------------------------------------------------------------------------------------------
void
tse::FareCurrencySelection::selectUSCACurrency(PricingTrx& trx,
                                               NationCode& originNation,
                                               FareMarket& fareMarket)

{
  GeoTravelType itinTravelType = GeoTravelType::Domestic;
  createDiag(trx, fareMarket, itinTravelType, SELECT_USCA_FUNC);
  CurrencyCode fareCompPrimeCurrency;

  LOG4CXX_DEBUG(logger, "Entered selectUSCACurrency");

  if (originNation == UNITED_STATES)
    fareCompPrimeCurrency = FareCurrencySelection::USD;
  else if (originNation == CANADA)
    fareCompPrimeCurrency = FareCurrencySelection::CAD;

  LOG4CXX_DEBUG(logger, "Faremarket GEO Travel Type: " << static_cast<int>(fareMarket.geoTravelType()));
  LOG4CXX_DEBUG(logger, "Faremarket Origin: " << originNation);

  PaxTypeBucketVec& paxTypeCortegeVec = fareMarket.paxTypeCortege();

  unsigned int paxTypeCortegeVecSize = paxTypeCortegeVec.size();

  const Customer* agentTJR = trx.getRequest()->ticketingAgent()->agentTJR();
  const std::string& cruisePfaCurrency = ((agentTJR != nullptr) ? agentTJR->cruisePfaCur() : "");

  for (unsigned int paxTypeCortegeCntr = 0; paxTypeCortegeCntr < paxTypeCortegeVecSize;
       paxTypeCortegeCntr++)
  {
    PaxTypeBucket& paxTypeCortege = paxTypeCortegeVec[paxTypeCortegeCntr];
    PaxTypeCode& paxCode = paxTypeCortege.requestedPaxType()->paxType();

    if (UNLIKELY(_diag))
    {
      (*_diag) << " ----------------------------------------------------------- \n"
               << " PAXTYPECORTEGE REQUESTED PAXTYPE: " << paxCode << " CUR: " << cruisePfaCurrency
               << "\n";
    }

    // Check PFA Cruise Line Agency Currency
    if (!cruisePfaCurrency.empty() && ((paxCode == PFA) || (paxCode == CBC) || (paxCode == CBI)))
    {
      paxTypeCortege.outboundCurrency() = cruisePfaCurrency;
      paxTypeCortege.inboundCurrency() = cruisePfaCurrency;

      FareMarketCurrencyKey key1(paxCode, FROM, cruisePfaCurrency);
      fareMarket.currencies().insert(key1);

      FareMarketCurrencyKey key2(paxCode, TO, cruisePfaCurrency);
      fareMarket.currencies().insert(key2);

      diagResult(paxTypeCortege);

      continue;
    }

    PaxTypeFarePtrVec paxTypeFareVec = paxTypeCortege.paxTypeFare();

    unsigned int paxVecSize = paxTypeFareVec.size();

    LOG4CXX_DEBUG(logger, "number of pax type fares: " << paxVecSize);

    for (unsigned int j = 0; j < paxVecSize; j++)
    {
      const PaxTypeFare* paxFare = paxTypeFareVec[j];

      if (!paxFare->isValid())
        continue;

      diagPTF(trx, paxFare, fareMarket);
      diagNationCode(originNation, true);

      if (paxFare->isFareByRule())
      {
        const FBRPaxTypeFareRuleData* fbrPaxTypeFare =
            paxFare->getFbrRuleData(RuleConst::FARE_BY_RULE);

        if (fbrPaxTypeFare && fbrPaxTypeFare->isSpecifiedFare())
        {
          continue;
        }
      }

      const tse::Directionality direction = paxFare->fare()->directionality();

      if (paxFare->isTransborder())
      {

        addCurrencyToFareMarket(direction, paxTypeCortege, fareCompPrimeCurrency, true);
        break;
      }
      else
      {
        // If not Canadian domestic the default is US
        //
        if (LocUtil::isCanada(*(paxFare->fareMarket()->origin())) &&
            LocUtil::isCanada(*(paxFare->fareMarket()->destination())))
        {

          addCurrencyToFareMarket(direction, paxTypeCortege, FareCurrencySelection::CAD, true);
          break;
        }
      }
    }
  }

  LOG4CXX_INFO(logger, "Leaving selectUSCACurrency");
  endDiag(SELECT_USCA_FUNC, true);
  return;
}

//--------------------------------------------------------------------------------------------
//
//   @method nationValidations
//
//   Description: If multiple currencies still exist after all of the validations have been
//                performed then this method is invoked. It will use one of the currencies in
//                the Nations table for comparison or USD or currency of 1st published fare
//                that has the same currency code as the fareCompPrimeCur field in the
//                Currency Selection table.
//
//                Validation overview:
//
//                1. If International itin compare to Bank Rate Currency
//                   else go to step 2 first
//                2. Compare to Pricing currency
//                3. If no match to 2, compare to National currency
//                4. If no match to 3, compare to  "USD"
//                5. If no match to 4, compare to 1st published fare
//                   using currency from Currency Selection table.
//
//   @param PaxTypeBucket   -  contains inbound/outbound currencies
//   @param PaxTypeFare      -  pax fare
//   @param NationCode       -  code for this nation
//   @param DateTime      -  travel date
//   @param CurrencyCode     -  first currency code in Currency Selection table
//   @param CurrencyCode     -  return validated fare component prime currency
//
//   @return bool  - true  - they match , else false.
//
//
//
//   @return bool  - true/false, false is failure
//
//--------------------------------------------------------------------------------------------
bool
tse::FareCurrencySelection::nationValidations(PaxTypeBucket& paxTypeCortege,
                                              FareMarket& fareMarket,
                                              const Directionality& direction,
                                              NationCode& nationCode,
                                              DateTime& ticketingDate,
                                              CurrencyCode& firstCurrency,
                                              CurrencyCode& fareCompCurrency,
                                              bool isInternational)
{
  CurrencyCode usd("USD");
  DataHandle dataHandle(ticketingDate);
  RecordScope recordScope = DOMESTIC;

  LOG4CXX_INFO(logger, "Entered   FareCurrencySelection::nationValidations");

  LOG4CXX_DEBUG(logger, "Ticketing date is " << ticketingDate.toSimpleString());

  const tse::Nation* nation = dataHandle.getNation(nationCode, ticketingDate);

  diagNation(nation, ENTER_NATION, firstCurrency);

  if (!nation)
  {
    LOG4CXX_DEBUG(logger, "Unable to retrieve Nation from cache for nation: " << nationCode);
    diagNation(nation, NATION_NOT_FOUND, firstCurrency);
    return false;
  }
  else
  {
    if (isInternational)
    {
      if (!nation->conversionCur().empty())
      {
        fareCompCurrency = nation->conversionCur();
        diagNation(nation, USE_CONVERSION_CURRENCY, firstCurrency);
        return true;
      }
      diagNation(nation, CONVERSION_CURR_NOMATCH, firstCurrency);
    }

    if (!nation->alternateCur().empty())
    {
      LOG4CXX_DEBUG(logger,
                    "pax fare currency matched nation alternate currency "
                        << nation->alternateCur());

      fareCompCurrency = nation->alternateCur();
      diagNation(nation, USE_NATION_ALTERNATE_CURRENCY, firstCurrency);
      return true;
    }
    else if (!nation->primeCur().empty())
    {
      LOG4CXX_DEBUG(logger,
                    "pax fare currency matched nation prime currency " << nation->primeCur());

      fareCompCurrency = nation->primeCur();
      diagNation(nation, USE_NATION_PRIME_CURRENCY, firstCurrency);
      return true;
    }
    else if (findMatchingUSDFare(fareMarket))
    {
      fareCompCurrency = usd;
      diagNation(nation, USE_USD, firstCurrency);
      return true;
    }
    else
    {
      const PaxTypeFare* nationPaxFare = findMatchingPaxTypeFare(
          fareMarket, direction, nationCode, isInternational, ticketingDate);

      if (!nationPaxFare)
        return false;

      if (isInternational)
        recordScope = INTERNATIONAL;

      if (_csValidator.isCurrencyPublished(recordScope, fareMarket, firstCurrency, ticketingDate))
      {
        fareCompCurrency = firstCurrency;
        diagNation(nation, USE_FIRST_CURRENCY, firstCurrency);
        return true;
      }
      diagNation(nation, EXIT_NATION, firstCurrency);
    }
  }

  LOG4CXX_INFO(logger, "Leaving   FareCurrencySelection::nationValidations");

  return false;
}

//------------------------------------------------------------------------------
//
//   @method findMatchingUSDFare
//
//   Description: Looks for a USD fare in fare market
//
//  @return bool - true, found USD fare in fare market else false
//
//------------------------------------------------------------------------------
bool
FareCurrencySelection::findMatchingUSDFare(FareMarket& fareMarket)
{
  bool foundUSDCurrency = false;
  CurrencyCode usd("USD");

  LOG4CXX_DEBUG(logger, "Entered findMatchingUSDFare");

  CurrencyIter currencyIter = fareMarket.currencies().begin();
  CurrencyIter currencyIterEnd = fareMarket.currencies().end();

  for (; currencyIter != currencyIterEnd; currencyIter++)
  {
    const FareMarketCurrencyKey& fmCurrencyKey = *currencyIter;

    if (fmCurrencyKey.currencyCode() == usd)
    {
      foundUSDCurrency = true;
      break;
    }
  }

  LOG4CXX_DEBUG(logger, "Leaving findMatchingUSDFare");

  return foundUSDCurrency;
}
//------------------------------------------------------------------------------
//
//   @method findMatchingPaxTypeFare
//
//   Description: Looks for a fare in fare market with matching direction
//                and currency.
//
//
//  @return PaxTypeFare * - pointer to Pax type Fare
//
//------------------------------------------------------------------------------
const PaxTypeFare*
FareCurrencySelection::findMatchingPaxTypeFare(FareMarket& fareMarket,
                                               const Directionality& direction,
                                               NationCode& nationCode,
                                               bool& isInternational,
                                               DateTime& ticketingDate)
{
  const PaxTypeFare* nationPaxFare = nullptr;
  bool foundCurrency = false;

  LOG4CXX_DEBUG(logger, "Entered findMatchingPaxTypeFare");
  LOG4CXX_DEBUG(logger, "Directionality is " << direction);

  PaxTypeBucketVec& paxTypeCortegeVec = fareMarket.paxTypeCortege();
  unsigned int paxTypeCortegeVecSize = paxTypeCortegeVec.size();

  LOG4CXX_DEBUG(logger, "Size of Pax Type Cortege vector: " << paxTypeCortegeVecSize);

  CurrencyCode nationCurrency;

  bool retVal = getNationCurrency(nationCode, nationCurrency, isInternational, ticketingDate);

  if (UNLIKELY(!retVal))
    return nationPaxFare;

  for (unsigned int paxTypeCortegeCntr = 0; paxTypeCortegeCntr < paxTypeCortegeVecSize;
       paxTypeCortegeCntr++)
  {
    PaxTypeBucket& paxTypeCortege = paxTypeCortegeVec[paxTypeCortegeCntr];

    PaxTypeFarePtrVec& paxTypeFareVec = paxTypeCortege.paxTypeFare();

    unsigned int paxVecSize = paxTypeFareVec.size();

    for (unsigned int i = 0; i < paxVecSize; i++)
    {
      PaxTypeFare* tmpPaxFare = paxTypeFareVec[i];

      if (!tmpPaxFare->isValid())
        continue;

      if (UNLIKELY(tmpPaxFare->isFareByRule() && tmpPaxFare->isSpecifiedFare()))
      {
        continue;
      }

      const tse::CurrencyCode& currency = tmpPaxFare->fare()->currency();

      if (UNLIKELY((tmpPaxFare->fare()->directionality() == BOTH) && (nationCurrency == currency)))
      {
        nationPaxFare = tmpPaxFare;
        foundCurrency = true;
        LOG4CXX_DEBUG(logger, "Fare Currency: " << nationPaxFare->fare()->currency());
        break;
      }
      else if ((tmpPaxFare->fare()->directionality() == direction) && (nationCurrency == currency))
      {
        nationPaxFare = tmpPaxFare;
        foundCurrency = true;
        LOG4CXX_DEBUG(logger, "Fare Currency: " << nationPaxFare->fare()->currency());
        break;
      }
    }

    if (foundCurrency)
      break;
  }

  LOG4CXX_DEBUG(logger, "Leaving findMatchingPaxTypeFare");

  return nationPaxFare;
}

//---------------------------------------------------------------------
//
//   @method getNationCurrency
//
//   @param NationCode   - nation
//   @param CurrencyCode - return parameter of nation prime currency
//
//   @return bool - true - nation currency retrieved , else false
//---------------------------------------------------------------------
bool
FareCurrencySelection::getNationCurrency(NationCode& nationCode,
                                         CurrencyCode& nationCurrency,
                                         bool isInternational,
                                         DateTime& ticketingDate)
{
  DataHandle dataHandle(ticketingDate);

  const tse::Nation* nation = dataHandle.getNation(nationCode, ticketingDate);

  if (UNLIKELY(!nation))
  {
    LOG4CXX_DEBUG(logger, "Unable to retrieve Nation from cache " << nationCode);
    return false;
  }

  if (isInternational)
  {
    if (!nation->conversionCur().empty())
    {
      LOG4CXX_DEBUG(logger, "Using conversion currency: " << nation->conversionCur());
      nationCurrency = nation->conversionCur();
      return true;
    }
  }

  if (LIKELY(!(nation->primeCur().empty())))
  {
    LOG4CXX_DEBUG(logger, "Using national currency: " << nation->primeCur());
    nationCurrency = nation->primeCur();
    return true;
  }

  if (!(nation->alternateCur().empty()))
  {
    LOG4CXX_DEBUG(logger, "Using pricing currency: " << nation->alternateCur());
    nationCurrency = nation->alternateCur();
    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------------------
//
//
//   @method determineCurrenciesInFareMarket
//
//   Description: Determine number of currencies for each faremarket
//
//   @param FareMarket -  FareMarket
//   @return void
//--------------------------------------------------------------------------------------------
void
FareCurrencySelection::determineCurrenciesInFareMarket(FareMarket& fareMarket,
                                                       const std::string& cruisePfaCurrency,
                                                       PricingTrx& trx,
                                                       GeoTravelType& itinTravelType,
                                                       bool skipIndustryFares,
                                                       bool privateFaresRequested)
{
  LOG4CXX_INFO(logger, "Entered  FareCurrencySelection::determineCurrenciesInFareMarket");

  PaxTypeBucketVec& paxTypeCortegeVec = fareMarket.paxTypeCortege();

  unsigned int paxTypeCortegeVecSize = paxTypeCortegeVec.size();

  LOG4CXX_DEBUG(logger, "Size of Pax Type Cortege vector: " << paxTypeCortegeVecSize);

  for (unsigned int paxTypeCortegeCntr = 0; paxTypeCortegeCntr < paxTypeCortegeVecSize;
       paxTypeCortegeCntr++)
  {
    PaxTypeBucket& paxTypeCortege = paxTypeCortegeVec[paxTypeCortegeCntr];
    const PaxTypeCode paxCodeReq = paxTypeCortege.requestedPaxType()->paxType();
    PaxTypeCode paxCode = paxCodeReq;

    // Check PFA Cruise Line Agency Currency
    if (UNLIKELY(!cruisePfaCurrency.empty() &&
                 ((paxCode == JCB) || (paxCode == PFA) || (paxCode == CBC) || (paxCode == CBI))))
    {
      FareMarketCurrencyKey key1(paxCode, FROM, cruisePfaCurrency);
      fareMarket.currencies().insert(key1);

      FareMarketCurrencyKey key2(paxCode, TO, cruisePfaCurrency);
      fareMarket.currencies().insert(key2);

      paxTypeCortege.outboundCurrency() = cruisePfaCurrency;
      paxTypeCortege.inboundCurrency() = cruisePfaCurrency;

      continue;
    }

    PaxTypeFarePtrVec paxTypeFareVec = paxTypeCortege.paxTypeFare();

    unsigned int paxVecSize = paxTypeFareVec.size();

    LOG4CXX_DEBUG(logger, "number of pax type fares: " << paxVecSize);
    LOG4CXX_DEBUG(logger, "Requested pax type = " << paxTypeCortege.requestedPaxType()->paxType());

    for (unsigned int j = 0; j < paxVecSize; j++)
    {
      PaxTypeFare* paxFare = paxTypeFareVec[j];

      if (!paxFare->isValidForPricing() ||
          (paxFare->carrier() == INDUSTRY_CARRIER && skipIndustryFares))
        continue;

      const tse::CurrencyCode& currencyCode = paxFare->fare()->currency();

      bool buildCurrencySet = false;
      bool isSpecifiedFare = false;
      paxCode = paxCodeReq;

      if (paxFare->isFareByRule())
      {
        const FBRPaxTypeFareRuleData* fbrPaxTypeFare =
            paxFare->getFbrRuleData(RuleConst::FARE_BY_RULE);

        if (fbrPaxTypeFare && fbrPaxTypeFare->isSpecifiedFare())
        {
          isSpecifiedFare = true;
        }
      }

      if (LIKELY(!isSpecifiedFare))
      {
        std::vector<PaxType*>::const_iterator paxTypeIter = paxTypeCortege.actualPaxType().begin();
        std::vector<PaxType*>::const_iterator paxTypeEnd = paxTypeCortege.actualPaxType().end();
        if (paxFare->fcasPaxType().empty())
        {
          if (paxTypeCortege.requestedPaxType()->paxType() == ADULT)
            buildCurrencySet = true;
        }
        else
        {

          for (; paxTypeIter != paxTypeEnd; ++paxTypeIter)
          {
            if ((*paxTypeIter)->paxType() == paxFare->fcasPaxType())
            {
              buildCurrencySet = true;
              break;
            }
          }
        }
        if ((paxFare->fcasPaxType().empty()) &&
            (paxTypeCortege.requestedPaxType()->paxType() != ADULT))
        {
          for (; paxTypeIter != paxTypeEnd; ++paxTypeIter)
          {
            if ((*paxTypeIter)->paxType() == ADULT)
            {
              buildCurrencySet = true;
              break;
            }
          }
        }

      } // if (!isSpecifiedFare)

      if (UNLIKELY(skipIndustryFares))
      {
        if (privateFaresRequested && paxFare->tcrTariffCat() != RuleConst::PRIVATE_TARIFF)
        {
          buildCurrencySet = false;
        }
      }

      if (LIKELY(buildCurrencySet))
      {
        if (paxFare->directionality() == BOTH)
        {
          FareMarketCurrencyKey key1(paxCode, FROM, currencyCode);
          fareMarket.currencies().insert(key1);

          FareMarketCurrencyKey key2(paxCode, TO, currencyCode);
          fareMarket.currencies().insert(key2);
        }
        else
        {
          FareMarketCurrencyKey key(paxCode, paxFare->directionality(), currencyCode);
          fareMarket.currencies().insert(key);
        }
      }

    } // for each pax type fare

  } // for all pax type corteges

  LOG4CXX_INFO(logger, "Leaving  FareCurrencySelection::determineCurrenciesInFareMarket");
}

//------------------------------------------------------------------------
void
FareCurrencySelection::createDiag(PricingTrx& trx,
                                  FareMarket& fm,
                                  GeoTravelType& itinTravelType,
                                  DiagMsgs diagMsg)
{
  DiagnosticTypes diagType = trx.diagnostic().diagnosticType();
  if (LIKELY(!trx.diagnostic().isActive() || diagType != Diagnostic212))
  {
    _diag = nullptr;
    return;
  }

  std::string specifiedFm("");
  std::string compFm = fm.boardMultiCity() + fm.offMultiCity();
  typedef std::map<std::string, std::string>::const_iterator DiagParamMapVecIC;
  DiagParamMapVecIC end = trx.diagnostic().diagParamMap().end();
  DiagParamMapVecIC begin = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_MARKET);
  if (begin != end)
  {
    size_t len = ((*begin).second).size();
    if (len != 0)
    {
      specifiedFm = ((*begin).second);
    }
    else
    {
      return;
    }

    if (specifiedFm != compFm)
    {
      return;
    }
  }

  _diag = DCFactory::instance()->create(trx);
  if (_diag != nullptr)
  {
    _diag->enable(Diagnostic212);
  }
  _csValidator.setDiag(_diag);
  DiagCollector& diag = *(_diag);

  switch (diagMsg)
  {
  case SELECT_PRIME_FUNC:
  {
    diag << "***** START DIAG 212 FUNCTION::SELECT-PRIME-CURRENCY ****** \n";
    break;
  }
  case SELECT_ALTERNATE_FUNC:
  {
    diag << "**** START DIAG 212 FUNCTION::SELECT-ALTERNATE-CURRENCY *** \n";
    break;
  }
  case SELECT_USCA_FUNC:
  {
    diag << "****** START DIAG 212 FUNCTION::SELECT-USCA-CURRENCY ****** \n";
    break;
  }
  default:
  {
    diag << "************* START DIAG 212 PER FARE MARKET *************** \n";
    break;
  }
  }

  diag << fm.boardMultiCity() << "-" << fm.offMultiCity();
  // Do Governing Carrier
  diag << "   GOV CXR: " << fm.governingCarrier();

  // Do Global Direction
  std::string gbd("UNK");
  globalDirectionToStr(gbd, fm.getGlobalDirection());
  diag << "    GLOBAL DIR: " << gbd;

  // Do Direction e.g. INBOUND/OUTBOUND
  std::string directnFm("UNKNOWN");
  switch (fm.direction())
  {
  case FMDirection::UNKNOWN:
    directnFm = "UNKNOWN";
    break;
  case FMDirection::INBOUND:
    directnFm = "INBOUND";
    break;
  case FMDirection::OUTBOUND:
    directnFm = "OUTBOUND";
    break;
  default:
    break;
  }
  diag << "   DIR: " << directnFm << " \n";

  diag.displayFareMarketItinNums("APPLIES TO ITINS           :", trx, fm);

  // Do Geo Travel type
  std::string gtt("UNKNOWN");
  switch (fm.geoTravelType())
  {
  case GeoTravelType::UnknownGeoTravelType:
    gtt = "UNKNOWN";
    break;
  case GeoTravelType::Domestic:
    gtt = "DOMESTIC";
    break;
  case GeoTravelType::International:
    gtt = "INTERNATIONAL";
    break;
  case GeoTravelType::Transborder:
    gtt = "TRANSBORDER";
    break;
  case GeoTravelType::ForeignDomestic:
    gtt = "FOREIGN DOMESTIC";
    break;
  default:
    break;
  }
  diag << "FARE MARKET GEO TRAVEL TYPE: " << gtt << " \n";

  // itinTravelType
  std::string itt("UNKNOWN");
  switch (itinTravelType)
  {
  case GeoTravelType::UnknownGeoTravelType:
    itt = "UNKNOWN";
    break;
  case GeoTravelType::Domestic:
    itt = "DOMESTIC";
    break;
  case GeoTravelType::International:
    itt = "INTERNATIONAL";
    break;
  case GeoTravelType::Transborder:
    itt = "TRANSBORDER";
    break;
  case GeoTravelType::ForeignDomestic:
    itt = "FOREIGN DOMESTIC";
    break;
  default:
    break;
  }
  diag << "ITINERARY GEO TRAVEL TYPE  : " << itt << " \n";

  if (trx.itin().size() > 0)
  {
    std::vector<Itin*>::iterator itinIter = trx.itin().begin();
    Itin& itin = *(*itinIter);

    diag << "ITINERARY ORIGINATION CURR : " << itin.originationCurrency() << " \n";
    diag << "ITINERARY CALCULATION CURR : " << itin.calculationCurrency() << " \n";
  }
  diag << "CACHE CURRENCIES: ";

  CurrencyIter itCur = fm.currencies().begin();
  CurrencyIter endCur = fm.currencies().end();
  std::string directn("UNKNOWN");
  int charcount = 0;
  for (; itCur != endCur; itCur++)
  {
    const FareMarketCurrencyKey& ccKey = *itCur;
    charcount += 9;
    switch (ccKey.directionality())
    {
    case 1:
      directn = "FROM";
      charcount += 4;
      break;
    case 2:
      directn = "TO";
      charcount += 2;
      break;
    case 3:
      directn = "BETWEEN";
      charcount += 7;
      break;
    case 4:
      directn = "WITHIN";
      charcount += 6;
      break;
    case 5:
      directn = "BOTH";
      charcount += 4;
      break;
    case 6:
      directn = "ORIGIN";
      charcount += 6;
      break;
    case 7:
      directn = "TERMINATE";
      charcount += 9;
      break;
    default:
      directn = "UNKNOWN";
      charcount += 7;
      break;
    }
    diag << ccKey.paxTypeCode() << "-" << directn << "-" << ccKey.currencyCode() << " ";
    if (charcount > 34)
    {
      diag << " \n";
      diag << "                  ";
      charcount = 0;
    }
  }
  diag << " \n";

  PaxTypeBucketVec& paxTypeCortegeVec = fm.paxTypeCortege();
  uint32_t paxTypeCortegeVecSize = paxTypeCortegeVec.size();

  diag << "NUMBER OF PAXTYPECORTEGES  : " << paxTypeCortegeVecSize;
  uint32_t totalPaxTypeFares = 0;
  for (uint32_t paxTypeCortegeCntr = 0; paxTypeCortegeCntr < paxTypeCortegeVecSize;
       paxTypeCortegeCntr++)
  {
    PaxTypeBucket& paxTypeCortege = paxTypeCortegeVec[paxTypeCortegeCntr];
    PaxTypeFarePtrVec paxTypeFareVec = paxTypeCortege.paxTypeFare();
    totalPaxTypeFares += paxTypeFareVec.size();
  }
  diag << "   TOTAL PAXTYPEFARES: " << totalPaxTypeFares << " \n";
  return;
}

//------------------------------------------------------------------------
void
FareCurrencySelection::diagPTF(PricingTrx& trx, const PaxTypeFare* paxFare, FareMarket& fm)
{
  if (LIKELY(_diag == nullptr))
    return;
  DiagCollector& diag = *(_diag);
  std::string specifiedFc("");
  std::string compFc = paxFare->createFareBasis(trx, false);
  typedef std::map<std::string, std::string>::const_iterator DiagParamMapVecIC;
  DiagParamMapVecIC end = trx.diagnostic().diagParamMap().end();
  DiagParamMapVecIC begin = trx.diagnostic().diagParamMap().find(Diagnostic::FARE_CLASS_CODE);
  if (begin != end)
  {
    size_t len = ((*begin).second).size();
    if (len != 0)
    {
      specifiedFc = ((*begin).second);
    }
    else
    {
      return;
    }

    if (specifiedFc != compFc)
    {
      return;
    }
  }
  diag << " ----------------------------------------------------------- \n";
  diag << " FARE PAXTYPE: " << paxFare->paxType()->paxType()
       << "   CXR: " << paxFare->fare()->carrier() << "   FARECLASS: " << compFc << " \n";

  diag << " FARE CURR: " << paxFare->fare()->currency()
       << "   MARKET1-2: " << paxFare->fare()->market1() << "-" << paxFare->fare()->market2();

  std::string directn("UNKNOWN");
  switch (paxFare->fare()->directionality())
  {
  case tse::FROM:
    directn = "FROM";
    break;
  case tse::TO:
    directn = "TO";
    break;
  case tse::BETWEEN:
    directn = "BETWEEN";
    break;
  case tse::WITHIN:
    directn = "WITHIN";
    break;
  case tse::BOTH:
    directn = "BOTH";
    break;
  case tse::ORIGIN:
    directn = "ORIGIN";
    break;
  case tse::TERMINATE:
    directn = "TERMINATE";
    break;
  default:
    break;
  }
  diag << "   DIRECTIONALITY: " << directn << " \n";

  if (paxFare->fare()->currency().empty())
  {
    diag << " CURRENCY CODE BLANK - CONTINUE TO NEXT FARE \n";
  }

  diag << " ----------------------------------------------------------- \n";

  return;
}

//------------------------------------------------------------------------
void
FareCurrencySelection::diagCurr(FareMarket& fm, int uniqueCurr)
{
  if (LIKELY(_diag == nullptr))
    return;

  DiagCollector& diag = *(_diag);
  diag << " NUM OF UNIQUE CURR: " << uniqueCurr << " \n";
  return;
}

//------------------------------------------------------------------------
void
FareCurrencySelection::diagNationCode(NationCode& nationCode, bool nationCodeFound)
{
  if (LIKELY(_diag == nullptr))
    return;

  DiagCollector& diag = *(_diag);
  diag << " NATION CODE: ";
  if (!nationCodeFound)
  {
    diag << " *ERROR* ";
  }
  else
  {
    diag << nationCode;
  }

  return;
}
//------------------------------------------------------------------------
void
FareCurrencySelection::diagNation(const tse::Nation* nation,
                                  DiagMsgs diagMsg,
                                  CurrencyCode& firstCurrency)
{
  if (_diag == nullptr)
    return;

  DiagCollector& diag = *(_diag);
  // CurrencyCode convCurr;
  // CurrencyCode altCurr;
  // CurrencyCode primeCurr;
  // CurrencyCode fareCurr;
  switch (diagMsg)
  {
  case ENTER_NATION:
  {
    diag << "   ENTER NATION VALIDATION-FIRSTCURRENCY: " << firstCurrency << " \n";
    break;
  }
  case NATION_NOT_FOUND:
  {
    diag << "    NATION NOT FOUND \n";
    diag << "   EXIT NATION VALIDATION-RETURN FALSE \n";
    break;
  }
  case USE_CONVERSION_CURRENCY:
  {
    diag << "    NATION CONVERSION CURRENCY MATCH \n";
    diag << "   EXIT NATION VALIDATION-RETURN TRUE \n";
    break;
  }
  case CONVERSION_CURR_NOMATCH:
  {
    diag << "    NATION CONVERSION CURRENCY NOMATCH \n";
    break;
  }
  case USE_NATION_ALTERNATE_CURRENCY:
  {
    diag << "    NATION ALTERNATE CURRENCY MATCH \n";
    diag << "   EXIT NATION VALIDATION-RETURN TRUE \n";
    break;
  }
  case USE_NATION_PRIME_CURRENCY:
  {
    diag << "    NATION ALTERNATE CURRENCY NOMATCH \n";
    diag << "    NATION PRIME CURRENCY MATCH \n";
    diag << "   EXIT NATION VALIDATION-RETURN TRUE \n";
    break;
  }
  case USE_USD:
  {
    diag << "    NATION ALTERNATE CURRENCY NOMATCH \n";
    diag << "    NATION PRIME CURRENCY NOMATCH \n";
    diag << "    USD MATCH WITH FARE CURRENCY \n";
    diag << "   EXIT NATION VALIDATION-RETURN TRUE \n";
    break;
  }
  case USE_FIRST_CURRENCY:
  {
    diag << "    NATION ALTERNATE CURRENCY NOMATCH \n";
    diag << "    NATION PRIME CURRENCY NOMATCH \n";
    diag << "    USD NOMATCH WITH FARE CURRENCY \n";
    diag << "    USE FIRSTCURRENCY-PUBLISHED CURRENCY \n";
    diag << "   EXIT NATION VALIDATION-RETURN TRUE \n";
    break;
  }
  case EXIT_NATION:
  {
    diag << "    NATION ALTERNATE CURRENCY NOMATCH \n";
    diag << "    NATION PRIME CURRENCY NOMATCH \n";
    diag << "    USD NOMATCH WITH FARE CURRENCY \n";
    diag << "    FIRSTCURRENCY IS NOT PUBLISHED CURRENCY \n";
    diag << "   EXIT NATION VALIDATION-RETURN FALSE \n";
    break;
  }
  default:
  {
    diag << "   EXIT NATION VALIDATION-RETURN FALSE \n";
    break;
  }
  }
  return;
}

//------------------------------------------------------------------------
void
FareCurrencySelection::diagMsg(DiagMsgs diagMsg)
{
  if (_diag == nullptr)
    return;
  DiagCollector& diag = *(_diag);
  switch (diagMsg)
  {
  case OUTBOUND_CURR_DIFF:
  {
    diag << " *OUTBOUND CURRENCY DIFFER FROM PREVIOUS PAXTYPRECORTEGE* \n";
    break;
  }
  case INBOUND_CURR_DIFF:
  {
    diag << " *INBOUND CURRENCY DIFFER FROM PREVIOUS PAXTYPRECORTEGE* \n";
    break;
  }
  case TRANSBORDER_OUT:
  {
    diag << " TRANSBORDER-FAREMARKET ORIGIN IN CANADA \n";
    diag << " PAXTYPECORTEGE OUTBOUND CURRENCY: CAD \n";
    diag << " CONTINUE TO NEXT PAXTYPEFARECORTEGE \n";
    break;
  }
  case TRANSBORDER_IN:
  {
    diag << " TRANSBORDER-FAREMARKET DESTINATION IN CANADA \n";
    diag << " PAXTYPECORTEGE INBOUND CURRENCY: CAD \n";
    diag << " CONTINUE TO NEXT PAXTYPEFARECORTEGE \n";
    break;
  }
  default:
  {
    break;
  }
  }
  return;
}
//------------------------------------------------------------------------
void
FareCurrencySelection::diagAsean(FareMarket& fm)
{
  if (LIKELY(_diag == nullptr))
    return;
  DiagCollector& diag = *(_diag);

  std::vector<tse::CurrencyCode> outAseanCur = fm.outBoundAseanCurrencies();
  std::vector<tse::CurrencyCode> inAseanCur = fm.inBoundAseanCurrencies();

  int outIndex = 0;
  int outSize = outAseanCur.size();
  int inIndex = 0;
  int inSize = inAseanCur.size();

  diag << " OUTBOUND ASEAN CURR: ";
  for (outIndex = 0; outIndex < outSize; outIndex++)
  {
    diag << outAseanCur[outIndex] << " ";
  }
  diag << " \n";

  diag << " INBOUND  ASEAN CURR: ";
  for (inIndex = 0; inIndex < inSize; inIndex++)
  {
    diag << inAseanCur[inIndex] << " ";
  }
  diag << " \n";

  return;
}
//------------------------------------------------------------------------
void
FareCurrencySelection::doDiag(bool foundOutBoundCurrency, bool foundInBoundCurrency)
{
  if (LIKELY(_diag == nullptr))
    return;
  DiagCollector& diag = *(_diag);
  std::string ob;
  std::string ib;
  if (foundOutBoundCurrency)
    ob = "TRUE";
  else
    ob = "FALSE";

  if (foundInBoundCurrency)
    ib = "TRUE";
  else
    ib = "FALSE";

  diag << " FOUND OUTBOUND CURRENCY: " << ob << " \n";
  diag << " FOUND INBOUND  CURRENCY: " << ib << " \n";

  return;
}

//------------------------------------------------------------------------
void
FareCurrencySelection::diagNUCRate(PricingTrx& trx,
                                   PaxTypeBucket& paxTypeCortege,
                                   bool foundOutBoundCurrency,
                                   bool foundInBoundCurrency)
{
  if (LIKELY(_diag == nullptr))
    return;

  DiagCollector& diag = *(_diag);

  if (trx.itin().size() > 0)
  {
    std::vector<Itin*>::iterator itinIter = trx.itin().begin();
    Itin& itin = *(*itinIter);

    if (itin.calculationCurrency() == NUC)
    {
      const std::vector<PaxTypeFare*>& paxFareVec = paxTypeCortege.paxTypeFare();
      std::vector<PaxTypeFare*>::const_iterator ptfIt = paxFareVec.begin();
      std::vector<PaxTypeFare*>::const_iterator ptfEnd = paxFareVec.end();

      if (foundOutBoundCurrency)
      {
        for (; ptfIt != ptfEnd; ++ptfIt)
        {
          PaxTypeFare& paxFare = **ptfIt;

          if ((paxFare.nucFareAmount()) && (paxFare.directionality() == FROM) &&
              (paxFare.currency() == paxTypeCortege.outboundCurrency()))
          {
            double outboundNucRate = paxFare.nucFareAmount() / paxFare.fareAmount();

            diag.setf(std::ios::fixed, std::ios::floatfield);
            diag << " OUTBOUND CURRENCY " << paxFare.currency()
                 << " TO NUC CONVERSION RATE: " << std::setprecision(6) << outboundNucRate << " \n";

            break;
          }
        }
      }

      if (foundInBoundCurrency)
      {
        for (; ptfIt != ptfEnd; ++ptfIt)
        {
          PaxTypeFare& paxFare = **ptfIt;

          if ((paxFare.nucFareAmount()) && (paxFare.directionality() == TO) &&
              (paxFare.currency() == paxTypeCortege.inboundCurrency()))
          {
            double inboundNucRate = paxFare.nucFareAmount() / paxFare.fareAmount();
            diag.setf(std::ios::fixed, std::ios::floatfield);
            diag << " INBOUND CURRENCY " << paxFare.currency()
                 << " TO NUC CONVERSION RATE: " << std::setprecision(6) << inboundNucRate << " \n";

            break;
          }
        }
      }
    }
  }

  if (foundOutBoundCurrency && foundInBoundCurrency)
    diag << " CONTINUE TO NEXT NATION \n";
  else
    diag << " CONTINUE TO NEXT PAXTYPEFARECORTEGE \n";

  return;
}

//------------------------------------------------------------------------
void
FareCurrencySelection::diagResult(PaxTypeBucket& paxTypeCortege)
{
  if (LIKELY(_diag == nullptr))
    return;
  DiagCollector& diag = *(_diag);

  diag << " \n";
  diag << " PAXTYPECORTEGE OUTBOUND CURRENCY: " << paxTypeCortege.outboundCurrency() << " \n";

  diag << " PAXTYPECORTEGE INBOUND  CURRENCY: " << paxTypeCortege.inboundCurrency() << " \n";

  return;
}

//------------------------------------------------------------------------
void
FareCurrencySelection::endDiag(DiagMsgs diagMsg, bool validRC)
{
  if (UNLIKELY(_diag != nullptr))
  {
    DiagCollector& diag = *(_diag);
    std::string msg;

    if (validRC)
      msg = "TRUE";
    else
      msg = "FALSE";

    if (diagMsg == SELECT_USCA_FUNC)
      msg = "VOID";

    switch (diagMsg)
    {
    case SELECT_PRIME_FUNC:
    {
      diag << "SELECT-PRIME-CURRENCY RETURN: " << msg << " \n";
      break;
    }
    case SELECT_ALTERNATE_FUNC:
    {
      diag << "SELECT-ALTERNATE-CURRENCY RETURN: " << msg << " \n";
      break;
    }
    case SELECT_USCA_FUNC:
    {
      diag << "SELECT-USCA-CURRENCY RETURN: " << msg << " \n";
      break;
    }
    default:
    {
      diag << "FARE-CURRENCY-SELECTION RETURN: " << msg << " \n";
      break;
    }
    }
    diag << "************** END DIAG 212 PER FARE MARKET **************** \n";
    diag << " \n";
    _diag->flushMsg();
    _diag->disable(Diagnostic212);
    _diag = nullptr;
  }
  return;
}
}
