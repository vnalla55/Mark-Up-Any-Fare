//----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <vector>

namespace tse
{
class CurrencySelection;
class CurSelectionGovCxr;
class CurSelectionPaxType;
class DataHandle;
class DateTime;
class DiagCollector;
class FareMarket;
class Itin;
class PaxType;
class PaxTypeBucket;
class PaxTypeFare;
class PricingTrx;

/**
*   @class CurrencySelectionValidator
*
*   Description:
*   CurrencySelectionValidator performs the validations on a fare component
*   to determine the currency for that fare component.
*
*/
class CurrencySelectionValidator final
{
public:
  static constexpr char CS_YES = 'Y';
  static constexpr char CS_NO = 'N';
  static constexpr char CS_BLANK = ' ';
  static constexpr char CS_DOMESTIC = 'D';
  static constexpr char CS_INTERNATIONAL = 'I';

  using CurSelectionGovCxrList = const std::vector<CurSelectionGovCxr*>;
  using CurSelectionPaxTypeList = const std::vector<CurSelectionPaxType*>;

  using CurSelectionGovCxrVec = const std::vector<CarrierCode>;
  using CurSelectionPaxTypeVec = const std::vector<PaxTypeCode>;

  /**
  *
  *   @method validate
  *
  *   Description: Performs validations on each prime domestic/international fare component of the
  *                fare market to determine the currency for fare selection.
  *
  *   @param PricingTrx                - transaction object
  *   @param PaxTypeBucket     - container class for paxTypefares
  *   @param PaxTypeFare        - passenger fare
  *   @param NationCode         - code representing nation , used to lookup,
  *                               currency information in Nations table.
  *   @param DateTime           - ticket date
  *   @param CurrencyCode       - validated fare component prime currency
  *   @param CurrencyCode       - first currency code in Currency Selection table
  *
  *   @return bool  - true  -   - valid currency was determined else false
  *
  */
  bool validate(PricingTrx& trx,
                PaxTypeBucket& paxTypeCortege,
                FareMarket& fareMarket,
                Itin& itin,
                NationCode& nationCode,
                DateTime& ticketDate,
                CurrencyCode& fareCompPrimeCurrency,
                CurrencyCode& fareQuoteOverrideCurrency,
                CurrencyCode& firstCurrency,
                bool& paxTypeSearchFailed,
                bool& nationNotFound,
                bool isInternational);

  /**
  *
  *   @method validate
  *
  *   Description: Performs validations on each alternate domestic/international
  *                fare component of the fare market to determine the currency for
  *                fare selection. Alternate Currency must be published and not
  *                restrictestricted.
  *
  *   @param PricingTrx         - transaction object
  *   @param PaxTypeBucket     - container class for paxTypefares
  *   @param PaxTypeFare        - passenger fare
  *   @param NationCode         - code representing nation , used to lookup,
  *                               currency information in Nations table.
  *   @param DateTime           - ticket date
  *   @param CurrencyCode       - alternate currency code
  *   @param CurrencyCode       - validated fare component prime currency
  *
  *   @return bool  - true  -   - valid currency was determined else false
  *
  */
  bool validate(PricingTrx& trx,
                PaxTypeBucket& paxTypeCortege,
                FareMarket& fareMarket,
                NationCode& nationCode,
                DateTime& ticketDate,
                CurrencyCode& alternateCurrency,
                CurrencyCode& fareCompPrimeCurrency,
                bool& paxTypeSearchFailed,
                bool& nationNotFound,
                bool& validOverride,
                bool isInternational);

  /**
  *
  *   @method isValid
  *
  *   Description: Validates the fare component what is in the Currency Selection
  *                table.
  *
  *   @param NationCode         -  nation
  *   @param PricingTrx         - transaction object
  *   @param PaxTypeBucket     - container class for paxTypefares
  *   @param CurrencySelection  -  pointer to current Currency Selection record
  *   @param PaxTypeFare        -  paxTypeFare
  *   @param bool               -  passenger type search failed - return parameter
  *   @param bool               -  whether or not this is international
  *
  *   @return bool  - true  - fare component is valid  else false.
  *
  */
  bool isValid(const NationCode& nation,
               PricingTrx& trx,
               PaxTypeBucket& paxTypeCortege,
               const CurrencySelection* cs,
               FareMarket& fareMarket,
               bool& paxTypeSearchFailed,
               bool isInternational);

  /**
  *
  *   @method isRestricted
  *
  *   Description: Determines whether or not this alternate currency is restricted.
  *
  *   @param CurrencySelection  -  pointer to current Currency Selection record
  *   @param CurrencyCode       - validated fare component prime currency
  *
  *   @return bool  - true  - currency is restricted, else false - no
  *                           restrictions.
  *
  */
  bool isRestricted(const CurrencySelection* cs,
                    CurrencyCode& fareCompPrimeCurrency,
                    const CurrencyCode& alternateCurrency);
  /**
  *
  *   @method isGoverningCarrierValid
  *
  *   Description: Validates the fare component governing carrier to
  *                what is in the Currency Selection table.
  *
  *   @param NationCode         -   nation
  *   @param CurrencySelection  -  pointer to current Currency Selection record
  *   @param PaxTypeFare        -   paxTypeFare
  *   @param Indicator          -   governing carrier exception
  *
  *   @return bool  - true  - governing carrier on fare component is valid,
  *                           else false.
  *
  */
  bool isGoverningCarrierValid(const NationCode& nation,
                               const CurrencySelection* cs,
                               FareMarket& fareMarket,
                               Indicator& govCarrierException);

  /**
  *
  *   @method validateGovCxrExceptions
  *
  *   Description: If the exception indicator is turned on in
  *                the CurrencySelection table then this method
  *                will be invoked to validate the exceptional
  *                conditions.
  *
  *   @param CurSelectionGovCxrVec   -   vector of Governing Carriers
  *   @param CarrierCode             -   Fare Component Governing Carrier Code
  *
  *   @return bool  - true  - the fare component Governing Carrier is not an exception
  *                           else false.
  *
  */
  bool validateGovCxrExceptions(CurSelectionGovCxrVec& curSelectionGovCxr,
                                const CarrierCode& govCarrier);

  /**
  *
  *   @method validateGovCxrNonExceptions
  *
  *   Description: If the exception indicator is not turned on in
  *                the CurrencySelection table then this method
  *                will be invoked to validate the vector of
  *                valid carriers .
  *
  *   @param CurSelectionGovCxrVec   -   vector of Governing Carriers
  *   @param CarrierCode             -   Fare Component Governing Carrier Code
  *   @param int                     -   sequence number of CurrencySelection table
  *
  *   @return bool  - true  - the fare component Governing Carrier is valid
  *                           else false.
  *
  */
  bool validateGovCxrNonExceptions(CurSelectionGovCxrVec& curSelectionGovCxr,
                                   const CarrierCode& govCarrier);

  /**
  *
  *   @method isPassengerTypeValid
  *
  *   Description: Validates the fare component passenger type to
  *                what is in the Currency Selection table.
  *
  *   @param NationCode         -   nation
  *   @param CurrencySelection  -  pointer to current Currency Selection record
  *   @param PaxTypeFare        -   paxTypeFare
  *   @param Indicator          -   paxtype  exception
  *
  *   @return bool  - true  - governing carrier on fare component is valid,
  *                           else false.
  *
  */
  bool isPassengerTypeValid(const NationCode& nation,
                            const CurrencySelection* cs,
                            const PaxType* paxType,
                            Indicator& paxTypeException);

  /**
  *
  *   @method validatePaxTypeExceptions
  *
  *   Description: If the exception indicator is turned on in
  *                the CurrencySelection table then this method
  *                will be invoked to validate the exceptional
  *                conditions.
  *
  *   @param CurSelectionPaxTypeVec   -   vector of Passenger Types
  *   @param PaxTypeCode              -   Fare Component Passenger Type Code
  *
  *   @return bool  - true  - the fare component Passenger Type is not an exception
  *                           else false.
  *
  */
  bool validatePaxTypeExceptions(CurSelectionPaxTypeVec& curSelectionPaxType,
                                 const PaxTypeCode& paxType);

  /**
  *
  *   @method validatePaxTypeNonExceptions
  *
  *   Description: If the exception indicator is not turned on in
  *                the CurrencySelection table then this method
  *                will be invoked to validate the vector of
  *                valid carriers.
  *
  *   @param CurSelectionPaxTypeList  -   vector of Passenger Types
  *   @param PaxTypeCode              -   Fare Component Passenger Type Code
  *
  *   @return bool  - true  - the fare component Governing Carrier is valid
  *                           else false.
  *
  */
  bool validatePaxTypeNonExceptions(CurSelectionPaxTypeVec& curSelectionPaxType,
                                    const PaxTypeCode& paxType);

  /**
  *
  *   @method isPointOfSaleValid
  *
  *   Description: Validates whether the point of sale is inside or outside
  *                of the fare component origin nation unless a sales override
  *                qualifier is specified.
  *
  *   @param PricingTrx                - transaction object
  *   @param CurrencySelection* - pointer to CurrencySelection record
  *
  *   @return bool - true - point of sale is valid, else false
  *
  */
  bool isPointOfSaleValid(PricingTrx& trx, const CurrencySelection* curSelectionRec);

  /**
  *
  *   @method isPointOfIssueValid
  *
  *   Description: Validates whether the ticketing point of issue is inside or outside
  *                of the fare component origin nation unless a ticketing
  *                override qualifer is specified.
  *
  *   @param PricingTrx                - transaction object
  *   @param CurrencySelection* - pointer to CurrencySelection record
  *
  *   @return bool - true - point of issue is valid, else false
  *
  */
  bool isPointOfIssueValid(PricingTrx& trx, const CurrencySelection* curSelectionRec);

  /**
  *
  *   @method isCurrencyPublished
  *
  *   Description: Determines if currency is a currency for a published fare.
  *
  *   @param RecordScope      -  used by DBAccess to control search
  *   @param FareMarket       -  fare market
  *   @param CurrencyCode     -  currency code to validate
  *   @param DateTime         -  ticketing date date
  *
  *   @return bool            - true currency determined, else false
  *
  */
  bool isCurrencyPublished(RecordScope RecordScope,
                           FareMarket& fareMarket,
                           const CurrencyCode& currency,
                           const DateTime& ticketingDate);

  /**
  *
  *   @method validatePublishedCurrency
  *
  *   Description: Retrieves fares for these markets and carrier and checks to see
  *                if the currency is published
  *
  *   @param RecordScope      -  used by DBAccess to control search
  *   @param CurrencyCode     -  currency code to validate
  *   @param DateTime         -  ticketing date
  *
  *   @return bool            - true currency determined, else false
  *
  */
  bool validatePublishedCurrency(RecordScope RecordScope,
                                 const LocCode& market1,
                                 const LocCode& market2,
                                 const CarrierCode& cxr,
                                 const CurrencyCode& currency,
                                 const DateTime& ticketingDate);

  /**
  *
  *   @method getCurrencySelection
  *
  *   Description: Retrieves a vector of CurrencySelection records using the nation.
  *                We have to do 2 searches. If the nation is not found then search
  *                for a blank(default) nation.
  *
  *   @param PricingTrx       -  transaction object
  *   @param NationCode       -  nation
  *   @param DateTime         -  travel date
  *
  *   @return std::vector       -  vector of Currency Selection records
  *
  */
  const std::vector<CurrencySelection*>& getCurrencySelection(PricingTrx& trx,
                                                              NationCode& nation,
                                                              const DateTime& date,
                                                              bool& useDefaultNation);

  /**
  *
  *
  *   @method getAseanCurrencies
  *
  *   Description: Retrieves a vector of Asean currencies.
  *
  *   @param PricingTrx     -  transaction object
  *   @param NationCode     -  nation
  *   @param FareMarket     -  fare market
  *
  *   @return void
  */
  void getAseanCurrencies(PricingTrx& trx, NationCode& nationCode, FareMarket& fareMarket);

  void setDiag(DiagCollector* diag212);

private:
  enum DiagMsgs
  {
    JOURNEY_RESTRICTION_FAIL = 1,
    FARE_COMP_RESTRICTION_FAIL,
    GOVERNING_CARRIER_FAIL,
    PAXTYPE_FAIL,
    POINT_OF_ISSUE_FAIL,
    PRIME_CURRNCY_EMPTY,
    CURRENCY_REC_PASS,
    ALT_CURR_NOT_PUBLISHED,
    RESTRICTED_FAIL,
    RESTRICTED_PASS
  };

  void diagStart(uint16_t currVecsize, NationCode& nationCode);

  void diagStartAlt(CurrencyCode& alternateCurrency, NationCode& nationCode);
  void diagAlt(uint16_t currVecsize);

  void diagCurrRec(const CurrencySelection* csRec);
  void diagAltCurrRec(const CurrencySelection* csRec);
  void diagMsg(DiagMsgs diagMsg);
  void diagEnd(bool determinedCurr);

  DiagCollector* _diag = nullptr; // diag 212
};
}
