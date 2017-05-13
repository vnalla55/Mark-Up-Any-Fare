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
#include "DataModel/CTRWCurrencyKey.h"
#include "Fares/CurrencySelectionValidator.h"
#include "Fares/FareCurrencySelection.h"


namespace tse
{
class FareDisplayTrx;
class FareMarket;
class CurrencySelection;
class PricingTrx;
class Itin;
class vector;
class set;
class PaxType;

typedef std::vector<tse::FareMarket*> FareMarketVec;
typedef FareMarketVec::const_iterator FareMarketVecI;
typedef std::vector<PaxTypeBucket> PaxTypeBucketVec;
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
class FDFareCurrencySelection : public FareCurrencySelection
{
public:
  FDFareCurrencySelection();

  virtual ~FDFareCurrencySelection();

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
  virtual bool selectPrimeCurrency(FareDisplayTrx& trx, FareMarket& fareMarket, Itin& itin);

  class UniqueCurrencies;
  class CTRWCurrencies;

  static bool getDisplayCurrency(FareDisplayTrx& trx, CurrencyCode& displayCurrency);
  static bool checkFareQuoteOverride(FareDisplayTrx& trx, CurrencyCode& displayCurrency);
  static bool validFaresInCortege(PaxTypeBucket& paxTypeCortege);
  static bool getSellingCurrency(FareDisplayTrx& trx, CurrencyCode& displayCurrency);

protected:
  /**
  *
  *   @method getNationCurrencyFD - retrieves either the national or pricing currency
  *
  *   @param NationCode   - nation
  *   @param CurrencyCode - return parameter of nation prime currency
  *   @param bool         - isInternational
  *
  *   @return bool - true - nation currency retrieved , else false
  */
  static bool getNationCurrencyFD(NationCode& nation,
                                  CurrencyCode& nationCurrency,
                                  bool isInternational,
                                  DateTime& ticketingDate);

private:
  tse::CurrencySelectionValidator _csValidator;

  const PaxTypeFare* findMatchingPaxTypeFare(PaxTypeBucket& paxTypeCortege,
                                             NationCode& nationCode,
                                             bool privateFaresRequested);

  void findPaxTypeFareCurrency(FareMarket& fareMarket,
                               const Directionality& direction,
                               NationCode& nationCode,
                               PaxTypeBucket& paxTypeCortege,
                               bool privateFaresRequested);

  void setFQOverride(FareDisplayTrx& trx,
                     CurrencyCode& fqCurrencyOverride,
                     CurrencyCode& fareCompPrimeCurrency,
                     PaxTypeBucket& paxTypeCortege,
                     const PaxTypeCode& paxType,
                     const Directionality& direction,
                     FareMarket& fareMarket,
                     NationCode& nationCode);

  bool faresThisCurrency(FareDisplayTrx& trx,
                         CurrencyCode& currency,
                         const PaxTypeCode& paxType,
                         const Directionality& direction,
                         FareMarket& fareMarket);
};

} /* end namespace tse */

