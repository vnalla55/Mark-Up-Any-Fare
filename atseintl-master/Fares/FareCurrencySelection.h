//----------------------------------------------------------------------------
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
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Fares/CurrencySelectionValidator.h"


#include <string>
#include <vector>

namespace tse
{
class CurrencySelection;
class DataHandle;
class DateTime;
class DiagCollector;
class ErrorResponseException;
class FareMarket;
class Itin;
class Nation;
class PaxType;
class vector;

typedef std::vector<tse::FareMarket*> FareMarketVec;
typedef FareMarketVec::const_iterator FareMarketVecI;
typedef std::vector<tse::PaxTypeBucket> PaxTypeBucketVec;
typedef PaxTypeBucketVec::iterator PaxTypeBucketVecI;

/**
*   @class FareCurrencySelection
*
*   Description:
*   FareCurrencySelection is responsible for determining the currency that
*   will be used for fare selection if multiple currencies exist for a given
*   fare component.
*
*/
class FareCurrencySelection
{
public:
  /**
  *
  *   @method FareCurrencySelection
  *
  */
  FareCurrencySelection();

  /**
  *
  *   @method ~FareCurrencySelection
  *
  */
  virtual ~FareCurrencySelection();

  /**
  *
  *   @method selectPrimeCurrency
  *
  *   Description: Performs validations on each domestic or international
  *                fare component of the fare market to determine the currency for fare selection.
  *
  *   @param PricingTrx         - transaction object
  *   @param FareMarket         - fare market
  *   @param GeoTravelType      - travel type of itinerary
  *   @param bool               - determine outbound currency only - true, else false
  *
  *   @return bool  - true  - operation completed successfully, false failure
  *
  */
  virtual bool selectPrimeCurrency(PricingTrx& trx,
                                   FareMarket& fareMarket,
                                   Itin& itin,
                                   bool determineOutBoundOnly = false);

  /**
  *
  *   @method selectAlternateCurrency
  *
  *   Description: Performs validations on each alternate currency pricing entry
  *                for an international or domestic fare component to determine the
  *                currency for fare selection.
  *
  *   @param PricingTrx                - transaction object
  *   @param FareMarket         - fare market
  *   @param CurrencyCode       - alternate currency code
  *   @param GeoTravelType      - travel type of itinerary
  *
  *   @return bool  - true  - operation completed successfully, false failure
  *
  */
  virtual bool selectAlternateCurrency(PricingTrx& trx,
                                       FareMarket& fareMarket,
                                       CurrencyCode& alternateCurrency,
                                       GeoTravelType& itinTravelType);

  /**
  *
  *   @method selectUSCACurrency
  *
  *   Description: Performs validations on each US/Canadian itinerary
  *                determine the currency for fare selection.
  *
  *   @param PricingTrx         - trx
  *   @param NationCode         - code representing nation , used to lookup,
  *                               currency information in Nations table.
  *   @param FareMarket         -  fare market
  *
  *   @return bool  - true  - operation completed successfully, false failure
  *
  */
  virtual void
  selectUSCACurrency(PricingTrx& trx, NationCode& originNation, FareMarket& fareMarket);

  /**
  *
  *   @method determineCurrenciesInMarket
  *
  *   Description: Determines number of currencies per pax type and direction
  *                for this fare market.
  *
  *   @param FareMarket     -  fare market
  *
  *   @return void
  *
  */
  void determineCurrenciesInFareMarket(FareMarket& fareMarket,
                                       const std::string& cruisePfaCurrency,
                                       PricingTrx& trx,
                                       GeoTravelType& itinTravelType,
                                       bool skipIndustryFare = false,
                                       bool privateFaresRequested = false);

  class UniqueCurrencies;

  /**
  *   @method getNationCode
  *
  *   Description: Retrieves Nation code for a Loc
  *
  *   @param PricingTrx         - Transaction object
  *   @param LocCode            - market
  *   @param DateTime           -  travel date
  *   @param NationCode         - return parameter ode representing nation , used to lookup,
  *                               currency information in Nations table.
  *
  *   @return bool  - true  - operation completed successfully, false failure
  */

  static bool
  getNationCode(PricingTrx& trx, const LocCode market, DateTime& date, NationCode& nationCode);

protected:
  /**
  *
  *   @method addCurrencyToFareMarket
  *
  *   Description: Adds a Currency to either the inbound or outbound currency on
  *                PaxTypeBucket.
  *
  *   @param Direction        -  either TO/FROM
  *   @param PaxTypeBucket   -  contains PaxTypeFares and inbound/outbound
  *                              CurrencyCodes.
  *   @param CurrencyCode     -  currency code
  *   @param bool             -  whether or not directionality is reversed
  *
  *   @return void
  *
  */
  void addCurrencyToFareMarket(Directionality direction,
                               PaxTypeBucket& paxTypeCortege,
                               const CurrencyCode& currencyCode,
                               bool isTransBorder = false);

  /**
  *   @method isCurrencySameAsPrevious
  *
  *   Description: Checks to see for foreign domestic itineraries that the current
  *                fare component currency is the same as the previous. The first
  *                fare component determines the currency.
  *
  *   @param Directionality     - Used to determine which currency code to compare
  *                               with. The PaxTypeBucket class stores inbound
  *                               and outbound currencies.
  *
  *   @param CurrencyCode       - currency to compare with previous
  *
  *   @param PaxTypeBucketVec  - Used to select first fare component information.
  *
  *   @return bool  - true  - operation completed successfully, false failure
  */
  bool isCurrencySameAsPrevious(Directionality direction,
                                CurrencyCode& currency,
                                PaxTypeBucketVec& paxTypeCortegeVec);

  /**
  *
  *   @method nationValidations
  *
  *   Description: If multiple currencies still exist after all of the validations have been
  *                performed then this method is invoked. It will use one of the currencies in
  *                the Nations table for comparison or USD or currency of 1st published fare
  *                that has the same currency code as the fareCompPrimeCur field in the
  *                Currency Selection table.
  *
  *                Validation overview:
  *
  *                1. If International itin compare to Bank Rate Currency
  *                   else go to step 2 first
  *                2. Compare to Pricing currency
  *                3. If no match to 2, compare to National currency
  *                4. If no match to 3, compare to  "USD"
  *                5. If no match to 4, compare to 1st published fare
  *                   using currency from Currency Selection table.
  *
  *   @param PaxTypeBucket   -  contains inbound/outbound currencies
  *   @param NationCode       -  code for this nation
  *   @param DateTime         -  travel date
  *   @param CurrencyCode     -  first currency code in Currency Selection table
  *   @param CurrencyCode     -  return validated fare component prime currency
  *
  *   @return bool  - true  - they match , else false.
  *
  */
  bool nationValidations(PaxTypeBucket& paxTypeCortege,
                         FareMarket& fareMarket,
                         const Directionality& direction,
                         NationCode& nation,
                         DateTime& travelDate,
                         CurrencyCode& firstCurrency,
                         CurrencyCode& fareCompCurrency,
                         bool isInternational = false);

  /**
  *
  *   @method findMatchingPaxTypeFare
  *
  *   Description: If the directionality is reversed it finds the pax  type fare
  *
  *   @param FareMarket      - fare market
  *   @param Directionality  - direction
  *   @param NationCode      - nation
  *   @param bool            - isInternational
  *
  *   @return PaxTypeFare * - pointer to Pax type Fare
  *
  */
  const PaxTypeFare* findMatchingPaxTypeFare(FareMarket& fareMarket,
                                             const Directionality& direction,
                                             NationCode& nationCode,
                                             bool& isInternational,
                                             DateTime& ticketingDate);

  /**
  *
  *   @method findMatchingUSDFare
  *
  *   Description: Looks for a USD fare in current fare market
  *
  *   @param FareMarket      - fare market
  *   @param Directionality  - direction
  *
  *   @return bool - true, found a USD fare else false
  *
  */
  bool findMatchingUSDFare(FareMarket& fareMarket);

  /**
  *
  *   @method getNationCurrency - retrieves either the national or pricing currency
  *
  *   @param NationCode   - nation
  *   @param CurrencyCode - return parameter of nation prime currency
  *   @param bool         - isInternational
  *
  *   @return bool - true - nation currency retrieved , else false
  */
  static bool getNationCurrency(NationCode& nation,
                                CurrencyCode& nationCurrency,
                                bool isInternational,
                                DateTime& ticketingDate);

  tse::CurrencySelectionValidator _csValidator;

private:
  enum DiagMsgs
  {
    ENTER_NATION = 1,
    NATION_NOT_FOUND,
    MATCHING_FARE_FOUND,
    MATCHING_FARE_NOT_FOUND,
    USE_CONVERSION_CURRENCY,
    CONVERSION_CURR_NOMATCH,
    USE_NATION_ALTERNATE_CURRENCY,
    USE_NATION_PRIME_CURRENCY,
    USE_USD,
    USE_FIRST_CURRENCY,
    EXIT_NATION,
    OUTBOUND_CURR_DIFF,
    INBOUND_CURR_DIFF,
    TRANSBORDER_OUT,
    TRANSBORDER_IN,
    SELECT_PRIME_FUNC,
    SELECT_ALTERNATE_FUNC,
    SELECT_USCA_FUNC
  };

  void createDiag(PricingTrx& trx, FareMarket& fm, GeoTravelType& itinTravelType, DiagMsgs diagMsg);

  void diagPTF(PricingTrx& trx, const PaxTypeFare* paxFare, FareMarket& fm);

  void diagCurr(FareMarket& fm, int uniqueCurr);

  void diagNationCode(NationCode& nationCode, bool nationCodeFound);

  void diagNation(const tse::Nation* nation, DiagMsgs diagMsg, CurrencyCode& firstCurrency);

  void doDiag(bool foundOutBoundCurrency, bool foundInBoundCurreIncy);

  void diagMsg(DiagMsgs diagMsg);

  void diagAsean(FareMarket& fm);

  void diagResult(PaxTypeBucket& paxTypeCortege);

  void diagNUCRate(PricingTrx& trx,
                   PaxTypeBucket& paxTypeCortege,
                   bool foundOutBoundCurrency,
                   bool foundInBoundCurrency);

  void endDiag(DiagMsgs diagMsg, bool validRC);

  static const CurrencyCode USD;
  static const CurrencyCode CAD;

  // tse::CurrencySelectionValidator _csValidator;
  DiagCollector* _diag;
};

} /* end namespace tse */

