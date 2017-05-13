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
#include "Fares/CurrencySelectionValidator.h"

#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CurrencySelection.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "Diagnostic/DiagCollector.h"

#include <iostream>
#include <vector>

#include <time.h>

namespace tse
{
static Logger
logger("atseintl.Fares.CurrencySelectionValidator");

///------------------------------------------------------------------------------
//
//   @method validate
//
//   Description: Performs validations on each Foreign domestic or International
//                fare component of the market to determine the currency for fare
//                selection.
//
//   @param PricingTrx         - transaction object
//   @param PaxTypeBucket     - container class for paxTypefares
//   @param PaxTypeFare        - passenger fare
//   @param NationCode         - code representing nation , used to lookup,
//                               currency information in Nations table.
//   @param DateTime           - ticket date
//   @param CurrencyCode       - validated fare component prime currency
//   @param bool               - true - return parameter used to indicate that
//                               the Passenger type validation failed. If true
//                               an additional search will be made to check for
//                               a Gov/Miliatary passenger type.
//
//   @return bool  - true  -   - valid currency was determined else false
//
//------------------------------------------------------------------------------
bool
CurrencySelectionValidator::validate(PricingTrx& trx,
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
                                     bool isInternational)
{
  bool determinedCurrency = false;
  bool useDefaultNation = false;
  static boost::mutex csTextMsgsFastMutex;

  LOG4CXX_INFO(logger,
               "Entered CurrencySelectionValidator::validate Foreign Domestic/International");

  const std::vector<CurrencySelection*>& currencySelection =
      getCurrencySelection(trx, nationCode, ticketDate, useDefaultNation);

  LOG4CXX_DEBUG(logger,
                "Size of CurrencySelection vector returned : " << currencySelection.size()
                                                               << " for nation: " << nationCode);

  diagStart(currencySelection.size(), nationCode);

  if (!currencySelection.empty())
  {
    std::vector<CurrencySelection*>::const_iterator curSelIter = currencySelection.begin();
    std::vector<CurrencySelection*>::const_iterator curSelIterEnd = currencySelection.end();

    firstCurrency = currencySelection[0]->fareCompPrimeCur();

    for (; curSelIter != curSelIterEnd; ++curSelIter)
    {
      const tse::CurrencySelection* curSelectionRec = *curSelIter;

      diagCurrRec(curSelectionRec);

      // Nation has already been implicitly validated
      // when the CurrencySelection records were retrieved

      if (!isValid(nationCode,
                   trx,
                   paxTypeCortege,
                   curSelectionRec,
                   fareMarket,
                   paxTypeSearchFailed,
                   isInternational))
        continue;

      if (!(curSelectionRec->fareCompPrimeCur().empty()))
      {
        LOG4CXX_DEBUG(logger,
                      "Currency Selection fare component prime currency is "
                          << curSelectionRec->fareCompPrimeCur());

        fareCompPrimeCurrency = curSelectionRec->fareCompPrimeCur();
        fareMarket.indirectEquivAmtCurrencyCode() = curSelectionRec->equivOverrideCur();
        paxTypeCortege.equivAmtOverrideCurrency() = curSelectionRec->equivOverrideCur();
        paxTypeSearchFailed = false;
        nationNotFound = false;
        determinedCurrency = true;

        if (!(curSelectionRec->fareQuoteOverrideCur().empty()))
        {
          fareQuoteOverrideCurrency = curSelectionRec->fareQuoteOverrideCur();
        }

        if (!curSelectionRec->txtSegs().empty())
        {

          std::vector<std::string>::const_iterator curSelTextIter =
              curSelectionRec->txtSegs().begin();
          std::vector<std::string>::const_iterator curSelTextIterEnd =
              curSelectionRec->txtSegs().end();

          if (_diag != nullptr)
          {
            *_diag << "TEXT MESSAGES: " << std::endl;
          }

          for (; curSelTextIter != curSelTextIterEnd; ++curSelTextIter)
          {
            const std::string& textMsg = *curSelTextIter;

            if (_diag)
              *_diag << textMsg << std::endl;

            boost::lock_guard<boost::mutex> g(csTextMsgsFastMutex);

            std::vector<std::string>::iterator iter =
                find(itin.csTextMessages().begin(), itin.csTextMessages().end(), textMsg);

            if (iter == itin.csTextMessages().end())
              itin.csTextMessages().push_back(textMsg);
          }
        }

        diagMsg(CURRENCY_REC_PASS);
        break;
      }
      else
        diagMsg(PRIME_CURRNCY_EMPTY);
    }
  }
  else
    nationNotFound = true;

  LOG4CXX_INFO(logger,
               "Leaving CurrencySelectionValidator::validate Foreign Domestic/International");

  diagEnd(determinedCurrency);
  return determinedCurrency;
}
///------------------------------------------------------------------------------
//
//   @method validate
//
//   Description: Performs validations on each alternate foreign domestic
//                or International fare component of the fare market to determine
//                the currency for fare selection.
//
//   @param PricingTrx         - transaction object
//   @param PaxTypeBucket     - container class for paxTypefares
//   @param PaxTypeFare        - passenger fare
//   @param NationCode         - code representing nation , used to lookup,
//                               currency information in Nations table.
//   @param DateTime           - ticket date
//   @param CurrencyCode       - alternate currency code
//   @param CurrencyCode       - validated fare component prime currency
//
//   @return bool  - true  -   - valid currency was determined else false
//
//------------------------------------------------------------------------------
bool
CurrencySelectionValidator::validate(PricingTrx& trx,
                                     PaxTypeBucket& paxTypeCortege,
                                     FareMarket& fareMarket,
                                     NationCode& nationCode,
                                     DateTime& ticketDate,
                                     CurrencyCode& alternateCurrency,
                                     CurrencyCode& fareCompPrimeCurrency,
                                     bool& paxTypeSearchFailed,
                                     bool& nationNotFound,
                                     bool& validOverride,
                                     bool isInternational)
{
  RecordScope recordScope = DOMESTIC;
  bool useDefaultNation = false;

  LOG4CXX_INFO(logger, "Entered CurrencySelectionValidator::validate - Alternate Currency ");
  diagStartAlt(alternateCurrency, nationCode);

  if (isInternational)
    recordScope = INTERNATIONAL;

  // Skip currency check for Fare Display entries
  if (trx.getTrxType() != PricingTrx::FAREDISPLAY_TRX)
  {
    // Check to see if currency is published for this market
    //
    if (!isCurrencyPublished(recordScope, fareMarket, alternateCurrency, trx.ticketingDate()))
    {
      validOverride = false;
      diagMsg(ALT_CURR_NOT_PUBLISHED);
      diagEnd(false);
      return false; // CURRENCY OVERRIDE NOT VALID
    }
  }

  else
  {
    const std::vector<CurrencySelection*>& currencySelection =
        getCurrencySelection(trx, nationCode, ticketDate, useDefaultNation);

    std::vector<CurrencySelection*>::const_iterator curSelIter = currencySelection.begin();
    std::vector<CurrencySelection*>::const_iterator curSelIterEnd = currencySelection.end();

    diagAlt(currencySelection.size());

    if (!currencySelection.empty())
    {
      for (; curSelIter != curSelIterEnd; ++curSelIter)
      {
        const tse::CurrencySelection* curSelectionRec = *curSelIter;
        diagAltCurrRec(curSelectionRec);

        LOG4CXX_DEBUG(logger, "Alternate Currency: " << alternateCurrency);
        LOG4CXX_DEBUG(logger, "Nation: " << nationCode);

        bool restrictedCurrency =
            isRestricted(curSelectionRec, fareCompPrimeCurrency, alternateCurrency);

        if (restrictedCurrency)
        {
          LOG4CXX_ERROR(logger, "Currency is Restricted: " << alternateCurrency);
          diagMsg(RESTRICTED_FAIL);
          validOverride = false;
          diagEnd(false);
          return false;
        }

      } // while more currency selection records
    }
    else
      nationNotFound = true;
  }

  LOG4CXX_INFO(logger, "Leaving  CurrencySelectionValidator::validate - alternate currency ");

  validOverride = true;
  fareCompPrimeCurrency = alternateCurrency;
  diagEnd(true);
  return true;
}

//--------------------------------------------------------------------------------------------
//
//
//   @method isRestricted
//
//   Description: Determines whether or not this alternate currency is restricted.
//
//   @param CurrencySelection  -  pointer to current Currency Selection record
//   @param CurrencyCode       - validated fare component prime currency
//
//   @return bool  - true  - currency is restricted, else false - no
//                           restrictions.
//
//   @return bool  - true  - fare component is valid  else false.
//
//--------------------------------------------------------------------------------------------
bool
CurrencySelectionValidator::isRestricted(const CurrencySelection* curSelectionRec,
                                         CurrencyCode& fareCompPrimeCurrency,
                                         const CurrencyCode& alternateCurrency)
{

  LOG4CXX_INFO(logger, "Entered   CurrencySelectionValidator::isRestricted");

  std::vector<CurrencyCode>::const_iterator resCurIter = curSelectionRec->restrictedCurs().begin();
  std::vector<CurrencyCode>::const_iterator resCurIterEnd = curSelectionRec->restrictedCurs().end();

  unsigned int numResCurrencies = curSelectionRec->restrictedCurs().size();

  if (curSelectionRec->restrCurExcept() == CS_YES)
  {
    if (numResCurrencies)
    {
      for (; resCurIter != resCurIterEnd; ++resCurIter)
      {
        const CurrencyCode& restrictedCur = *resCurIter;

        if (alternateCurrency == restrictedCur)
        {
          LOG4CXX_DEBUG(logger, "Restricted currency indicator is Y : currency is not restricted");
          fareCompPrimeCurrency = restrictedCur;
          return false;
        }
      } // while more restricted currencies
    }
    else
    {
      LOG4CXX_DEBUG(logger, "Restricted currency indicator is Y and child table is empty");
      return true; // everything is restricted
    }
  }
  else if (curSelectionRec->restrCurExcept() == CS_NO)
  {
    if (numResCurrencies)
    {
      for (; resCurIter != resCurIterEnd; ++resCurIter)
      {
        const CurrencyCode& restrictedCur = *resCurIter;

        if (alternateCurrency == restrictedCur)
        {
          LOG4CXX_DEBUG(logger, "Restricted currency indicator is N : currency is restricted");
          return true; // Currency is restricted
        }
      }
    }
  }

  LOG4CXX_INFO(logger, "Leaving   CurrencySelectionValidator::isRestricted");

  return false; // currency is not restricted
}

//--------------------------------------------------------------------------------------------
//
//   @method isValid
//
//   Description: Validates the fare component what is in the Currency Selection
//                table.
//
//   @param NationCode         -   nation
//   @param CurrencySelection  -   pointer to current Currency Selection record
//   @param PaxTypeFare        -   paxTypeFare
//   @param bool               -   passenger type search failed - return parameter
//   @param bool               -   isInternational
//
//   @return bool  - true  - fare component is valid  else false.
//
//--------------------------------------------------------------------------------------------
bool
CurrencySelectionValidator::isValid(const NationCode& nationCode,
                                    PricingTrx& trx,
                                    PaxTypeBucket& paxTypeCortege,
                                    const CurrencySelection* curSelectionRec,
                                    FareMarket& fareMarket,
                                    bool& paxTypeSearchFailed,
                                    bool isInternational)
{

  LOG4CXX_INFO(logger, "Entered   CurrencySelectionValidator::isValid");

  // Validate Journey restriction
  //
  if (isInternational)
  {
    if (!(curSelectionRec->journeyRestr() == CS_INTERNATIONAL))
    {
      LOG4CXX_DEBUG(logger, "Currency Selection journey restriction comparison failed");
      diagMsg(JOURNEY_RESTRICTION_FAIL);
      return false;
    }
  }
  else
  {
    if (!(curSelectionRec->journeyRestr() == CS_DOMESTIC))
    {
      LOG4CXX_DEBUG(logger, "Currency Selection journey restriction comparison failed");
      diagMsg(JOURNEY_RESTRICTION_FAIL);
      return false;
    }
  }

  LOG4CXX_DEBUG(logger, "Validated Journey Restriction");

  LOG4CXX_DEBUG(logger, "Validating Fare Component Restriction");

  // Validate Fare Component restriction
  //
  if ((curSelectionRec->farecomponentRestr() == CS_INTERNATIONAL))
  {
    if (!(fareMarket.geoTravelType() == GeoTravelType::International))
    {
      LOG4CXX_DEBUG(
          logger, "Currency Selection international fare component restriction comparison failed");
      diagMsg(FARE_COMP_RESTRICTION_FAIL);
      return false;
    }
  }
  else if ((curSelectionRec->farecomponentRestr() == CS_DOMESTIC))
  {
    if (!(fareMarket.geoTravelType() == GeoTravelType::ForeignDomestic) &&
        !(fareMarket.geoTravelType() == GeoTravelType::Domestic))
    {
      LOG4CXX_DEBUG(logger, "Currency Selection fare component restriction comparison failed");
      diagMsg(FARE_COMP_RESTRICTION_FAIL);
      return false;
    }
  }

  LOG4CXX_DEBUG(logger, "Validated Fare Component Restriction");

  Indicator govCarrierException = curSelectionRec->govCarrierExcept();

  if (!(isGoverningCarrierValid(nationCode, curSelectionRec, fareMarket, govCarrierException)))
  {
    LOG4CXX_DEBUG(logger, "Governing carrier validation failed");
    diagMsg(GOVERNING_CARRIER_FAIL);
    return false;
  }

  LOG4CXX_DEBUG(logger, "Governing carrier validation passed");

  Indicator paxTypeException = curSelectionRec->psgTypeExcept();
  const PaxType* paxType = paxTypeCortege.requestedPaxType();

  if (paxType)
  {
    if (!paxType->paxType().empty())
      LOG4CXX_DEBUG(logger, "Passenger Type: " << paxType->paxType());

    if (!(isPassengerTypeValid(nationCode, curSelectionRec, paxType, paxTypeException)))
    {
      LOG4CXX_DEBUG(logger, "Passenger type validation failed");
      paxTypeSearchFailed = true;
      diagMsg(PAXTYPE_FAIL);
      return false;
    }
  }
  else
  {
    LOG4CXX_DEBUG(logger, "Requested Passenger Type is null");
    diagMsg(PAXTYPE_FAIL);
    return false;
  }

  LOG4CXX_DEBUG(logger, "Passenger type validation passed");

  LOG4CXX_DEBUG(logger, "Validating Point of Sale");
  bool posRC = isPointOfSaleValid(trx, curSelectionRec);

  if (!posRC)
  {
    LOG4CXX_DEBUG(logger, "Validation of Point of Sale failed");
    return false;
  }

  LOG4CXX_DEBUG(logger, "Point of Sale validation passed");

  LOG4CXX_DEBUG(logger, "Validating Point of Issue");

  bool poiRC = isPointOfIssueValid(trx, curSelectionRec);

  if (!poiRC)
  {
    LOG4CXX_DEBUG(logger, "Validation of Point of Issue failed");
    diagMsg(POINT_OF_ISSUE_FAIL);
    return false;
  }

  LOG4CXX_DEBUG(logger, "Point of Issue validation passed");

  LOG4CXX_INFO(logger, "Leaving  CurrencySelectionValidator::isValid");

  return true;
}
//---------------------------------------------------------------------------------------------------
//
//   @method isGoverningCarrierValid
//
//   Description: Validates the fare component governing carrier to
//                what is in the Currency Selection table.
//
//   @param NationCode         -   nation
//   @param CurrencySelection  -   pointer to Currency Selection record
//   @param PaxTypeFare        -   paxTypeFare
//   @param Indicator          -   governing carrier exception
//
//   @return bool  - true  - governing carrier on fare component is valid,
//                           else false.
//
//---------------------------------------------------------------------------------------------------
bool
CurrencySelectionValidator::isGoverningCarrierValid(const NationCode& nation,
                                                    const CurrencySelection* cs,
                                                    FareMarket& fareMarket,
                                                    Indicator& govCarrierException)
{
  bool govCxrRC = false;

  LOG4CXX_DEBUG(logger, "Entered  CurrencySelectionValidator::isGoverningCarrierValid");

  if (govCarrierException == CS_YES)
  {
    // If the vector is empty then this is an error
    //
    if (!cs->govCarriers().empty())
      govCxrRC = validateGovCxrExceptions(cs->govCarriers(), fareMarket.governingCarrier());
    else
      LOG4CXX_DEBUG(logger, "Governing Carrier vector is empty");
  }
  else if (govCarrierException == CS_NO)
  {
    // If the vector is empty then all carriers are valid
    //
    if (cs->govCarriers().empty())
    {
      LOG4CXX_DEBUG(logger, "Exception is 'N' and Governing Carrier vector is empty");
      return true;
    }

    govCxrRC = validateGovCxrNonExceptions(cs->govCarriers(), fareMarket.governingCarrier());
  }

  LOG4CXX_DEBUG(logger, "Leaving  CurrencySelectionValidator::isGoverningCarrierValid");

  return govCxrRC;
}

//-----------------------------------------------------------------------------------------------------------
//
//   @method validateGovCxrExceptions
//
//   Description: If the exception indicator is turned on in
//                the CurrencySelection table then this method
//                will be invoked to validate the vector of
//                exceptional carriers(they are not valid).
//
//   @param CurSelectionGovCxrVec   -   vector of Governing Carriers
//   @param CarrierCode             -   Fare Component Governing Carrier Code
//
//   @return bool  - true  - the fare component Governing Carrier is valid
//                           else false.
//
//-----------------------------------------------------------------------------------------------------------
bool
CurrencySelectionValidator::validateGovCxrExceptions(CurSelectionGovCxrVec& curSelectionGovCxr,
                                                     const CarrierCode& govCarrier)
{
  LOG4CXX_DEBUG(logger, "Entered  CurrencySelectionValidator::validateGovCxrExceptions");

  std::vector<CarrierCode>::const_iterator curSelGovCxrIter = curSelectionGovCxr.begin();
  std::vector<CarrierCode>::const_iterator curSelGovCxrIterEnd = curSelectionGovCxr.end();

  for (; curSelGovCxrIter != curSelGovCxrIterEnd; ++curSelGovCxrIter)
  {
    const CarrierCode& govCarrierCode = *curSelGovCxrIter;

    if (govCarrierCode == govCarrier)
      return false;
  }

  LOG4CXX_DEBUG(logger, "Leaving  CurrencySelectionValidator::validateGovCxrExceptions");

  return true;
}

//-----------------------------------------------------------------------------------------------------------
//
//   @method validateGovCxrNonExceptions
//
//   Description: If the exception indicator is not turned on in
//                the CurrencySelection table then this method
//                will be invoked to validate the vector of
//                valid carriers.
//
//   @param CurSelectionGovCxrVec   -   vector of Governing Carriers
//   @param CarrierCode             -   Fare Component Governing Carrier Code
//
//   @return bool  - true  - the fare component Governing Carrier is valid
//                           else false.
//
//-----------------------------------------------------------------------------------------------------------
bool
CurrencySelectionValidator::validateGovCxrNonExceptions(CurSelectionGovCxrVec& curSelectionGovCxr,
                                                        const CarrierCode& govCarrier)
{
  LOG4CXX_DEBUG(logger, "Entered  CurrencySelectionValidator::validateGovCxrNonExceptions");

  std::vector<CarrierCode>::const_iterator curSelGovCxrIter = curSelectionGovCxr.begin();
  std::vector<CarrierCode>::const_iterator curSelGovCxrIterEnd = curSelectionGovCxr.end();

  if (curSelectionGovCxr.empty())
    return true;

  for (; curSelGovCxrIter != curSelGovCxrIterEnd; ++curSelGovCxrIter)
  {
    const CarrierCode& govCarrierCode = *curSelGovCxrIter;

    if (govCarrierCode == govCarrier)
      return true;
  }

  LOG4CXX_DEBUG(logger, "Leaving  CurrencySelectionValidator::validateGovCxrNonExceptions");

  return false;
}

//----------------------------------------------------------------------------------------------------
//
//   @method isPassengerTypeValid
//
//   Description: Validates the fare component passenger type to
//                what is in the Currency Selection table.
//
//   @param NationCode        -   nation
//   @param CurrencySelection - pointer to current Currency Selection record
//   @param PaxTypeFare       -   paxTypeFare
//   @param Indicator         -   governing carrier exception
//
//   @return bool  - true  - point of sale is valid, else false
//
//
//-----------------------------------------------------------------------------------------------
bool
CurrencySelectionValidator::isPassengerTypeValid(const NationCode& nation,
                                                 const CurrencySelection* cs,
                                                 const PaxType* paxType,
                                                 Indicator& paxTypeException)
{
  bool validRC = false;

  LOG4CXX_DEBUG(logger, "Entered  CurrencySelectionValidator::isPassengerTypeValid");

  LOG4CXX_DEBUG(
      logger, "size of currency selection passenger type vector: " << cs->passengerTypes().size());

  if (paxTypeException == CS_YES)
  {
    if (!cs->passengerTypes().empty())
      validRC = validatePaxTypeExceptions(cs->passengerTypes(), paxType->paxType());
    else
      LOG4CXX_DEBUG(logger, "Passenger Type List is empty");
  }
  else if (paxTypeException == CS_NO)
  {
    // All are valid if vector is empty
    //
    if (cs->passengerTypes().empty())
    {
      LOG4CXX_DEBUG(logger, "Exception is 'N' and Passenger Type List is empty");
      return true;
    }

    validRC = validatePaxTypeNonExceptions(cs->passengerTypes(), paxType->paxType());
  }

  LOG4CXX_DEBUG(logger, "Leaving  CurrencySelectionValidator::isPassengerTypeValid");

  return validRC;
}

//-----------------------------------------------------------------------------------------------------------
//
//   @method validatePaxTypeExceptions
//
//   Description: If the exception indicator is turned on in
//                the CurrencySelection table then this method
//                will be invoked to validate the vector of
//                exceptional passenger types(they are not valid).
//
//   @param CurSelectionPaxTypeVec   -   vector of Passenger Types
//   @param PaxTypeCode              -   Fare Component Passenger Type Code
//
//   @return bool  - true  - the fare component Passenger Type is valid
//                           else false.
//
//-----------------------------------------------------------------------------------------------------------
bool
CurrencySelectionValidator::validatePaxTypeExceptions(CurSelectionPaxTypeVec& curSelectionPaxType,
                                                      const PaxTypeCode& paxType)
{
  LOG4CXX_DEBUG(logger, "Entered  CurrencySelectionValidator::validatePaxTypeExceptions");

  if (!curSelectionPaxType.empty())
  {
    std::vector<PaxTypeCode>::const_iterator curSelPaxTypeIter = curSelectionPaxType.begin();
    std::vector<PaxTypeCode>::const_iterator curSelPaxTypeIterEnd = curSelectionPaxType.end();

    for (; curSelPaxTypeIter != curSelPaxTypeIterEnd; ++curSelPaxTypeIter)
    {
      const PaxTypeCode& paxTypeCode = *curSelPaxTypeIter;

      if (paxTypeCode == paxType)
      {
        LOG4CXX_DEBUG(logger, "Passenger types are same: " << paxTypeCode);
        return false;
      }
      else
      {
        if (!(paxTypeCode.empty()) && !(paxType.empty()))
        {
          LOG4CXX_DEBUG(logger, "Passenger types are not same: " << paxTypeCode);
          LOG4CXX_DEBUG(logger, "Fare Component Passenger type: " << paxType);
        }
      }
    }
  }

  LOG4CXX_DEBUG(logger, "Leaving  CurrencySelectionValidator::validatePaxTypeExceptions");

  // No exceptions found
  //
  return true;
}

//-----------------------------------------------------------------------------------------------------------
//
//   @method validatePaxTypeNonExceptions
//
//   Description: If the exception indicator is not turned on in
//                the CurrencySelection table then this method
//                will be invoked to validate the vector of
//                valid passenger types.
//
//   @param CurSelectionPaxTypeVec   -   vector of Passenger Types
//   @param PaxTypeCode              -   Fare Component Passenger Type Code
//
//   @return bool  - true  - the fare component Passenger Type is valid
//                           else false.
//
//-----------------------------------------------------------------------------------------------------------
bool
CurrencySelectionValidator::validatePaxTypeNonExceptions(
    CurSelectionPaxTypeVec& curSelectionPaxType, const PaxTypeCode& paxType)
{
  LOG4CXX_DEBUG(logger, "Entered  CurrencySelectionValidator::validatePaxTypeNonExceptions");

  std::vector<PaxTypeCode>::const_iterator curSelPaxTypeIter = curSelectionPaxType.begin();
  std::vector<PaxTypeCode>::const_iterator curSelPaxTypeIterEnd = curSelectionPaxType.end();

  if (curSelectionPaxType.empty())
    return true;

  for (; curSelPaxTypeIter != curSelPaxTypeIterEnd; ++curSelPaxTypeIter)
  {
    const PaxTypeCode& paxTypeCode = *curSelPaxTypeIter;

    if (paxTypeCode == paxType)
      return true;
  }

  LOG4CXX_DEBUG(logger, "Leaving  CurrencySelectionValidator::validatePaxTypeNonExceptions");

  // No valid passenger types found
  //
  return false;
}
//----------------------------------------------------------------------------------------------------
//
//   @method isPointOfSaleValid
//
//   Description: Validates whether the point of sale is inside or outside
//                of the fare component origin nation unless a sales override
//                qualifier is specified.
//
//   @param PricingTrx                - transaction object
//   @param CurrencySelection* - pointer to CurrencySelection record
//
//   @return bool - true - point of sale is valid, else false
//-----------------------------------------------------------------------------------------------
bool
CurrencySelectionValidator::isPointOfSaleValid(PricingTrx& trx,
                                               const CurrencySelection* curSelectionRec)
{
  LOG4CXX_INFO(logger, "Entered  CurrencySelectionValidator::isPointOfSaleValid");
  bool geoMatch = false;

  LocTypeCode posLocTypeCode = curSelectionRec->posLoc().locType();
  LocCode posLoc = curSelectionRec->posLoc().loc();
  Indicator posException = curSelectionRec->posexcept();

  if (!posLoc.empty())
  {
    if (!trx.getRequest()->salePointOverride().empty())
    {
      geoMatch = LocUtil::isInLoc(trx.getRequest()->salePointOverride(),
                                  posLocTypeCode,
                                  posLoc,
                                  Vendor::SABRE,
                                  MANUAL,
                                  GeoTravelType::International,
                                  LocUtil::OTHER,
                                  trx.getRequest()->ticketingDT());
    }
    else
    {
      geoMatch = LocUtil::isInLoc(*(trx.getRequest()->ticketingAgent()->agentLocation()),
                                  posLocTypeCode,
                                  posLoc,
                                  Vendor::SABRE,
                                  MANUAL,
                                  LocUtil::OTHER,
                                  GeoTravelType::International,
                                  EMPTY_STRING(),
                                  trx.getRequest()->ticketingDT());
    }

    if ((geoMatch && posException == CS_YES) || (!geoMatch && posException != CS_YES))
      return false;
  }

  LOG4CXX_INFO(logger, "Leaving  CurrencySelectionValidator::isPointOfSaleValid");

  return true;
}

//----------------------------------------------------------------------------------------------------
//
//   @method isPointOfIssueValid
//
//   Description: Validates whether the point of issue is inside or outside
//                of the fare component origin nation unless a sales ticketing
//                override qualifier is specified.
//
//   @param PricingTrx                - transaction object
//   @param CurrencySelection* - pointer to CurrencySelection record
//
//   @return bool - true - point of sale is valid, else false
//
//   @return bool  - true  - they match , else false.
//
//-----------------------------------------------------------------------------------------------
bool
CurrencySelectionValidator::isPointOfIssueValid(PricingTrx& trx,
                                                const CurrencySelection* curSelectionRec)
{
  LOG4CXX_INFO(logger, "Entered  CurrencySelectionValidator::isPointOfIssueValid");
  bool geoMatch = false;

  LocTypeCode poiLocTypeCode = curSelectionRec->poiLoc().locType();
  LocCode poiLoc = curSelectionRec->poiLoc().loc();
  Indicator poiException = curSelectionRec->poiExcept();

  if (!poiLoc.empty())
  {
    if (!trx.getRequest()->ticketPointOverride().empty())
    {
      geoMatch = LocUtil::isInLoc(trx.getRequest()->ticketPointOverride(),
                                  poiLocTypeCode,
                                  poiLoc,
                                  Vendor::SABRE,
                                  MANUAL,
                                  GeoTravelType::International,
                                  LocUtil::OTHER,
                                  trx.getRequest()->ticketingDT());
    }
    else
    {
      geoMatch = LocUtil::isInLoc(*(trx.getRequest()->ticketingAgent()->agentLocation()),
                                  poiLocTypeCode,
                                  poiLoc,
                                  Vendor::SABRE,
                                  MANUAL,
                                  LocUtil::OTHER,
                                  GeoTravelType::International,
                                  EMPTY_STRING(),
                                  trx.getRequest()->ticketingDT());
    }

    if ((geoMatch && poiException == CS_YES) || (!geoMatch && poiException != CS_YES))
      return false;
  }

  LOG4CXX_INFO(logger, "Leaving  CurrencySelectionValidator::isPointOfIssueValid");

  return true;
}

//--------------------------------------------------------------------------------------------
//
//   @method isCurrencyPublished
//
//
//   Description: Determines whether or not the currency from the Currency
//                is published or not.
//
//   @param RecordScope  - used by DBAccess to control search
//   @param FareMarket   - fare market
//   @param CurrencyCode - currency code to validate
//   @param DateTime     - ticketing  date
//
//   @return bool  - true/false, false is failure
//
//--------------------------------------------------------------------------------------------
bool
CurrencySelectionValidator::isCurrencyPublished(RecordScope recordScope,
                                                FareMarket& fareMarket,
                                                const CurrencyCode& currency,
                                                const DateTime& ticketingDate)
{
  DataHandle dataHandle(ticketingDate);

  LOG4CXX_DEBUG(logger, "Entered   CurrencySelectionValidator::isCurrencyPublished");

  LOG4CXX_DEBUG(logger, "Record Scope: " << recordScope);
  LOG4CXX_DEBUG(logger, "Currency : " << currency);
  LOG4CXX_DEBUG(logger, "Ticketing Date: " << ticketingDate.toSimpleString());

  LOG4CXX_DEBUG(logger, "Leaving   CurrencySelectionValidator::isCurrencyPublished");

  const tse::LocCode origin = LocUtil::getCity(*(fareMarket.origin()));
  const tse::LocCode destination = LocUtil::getCity(*(fareMarket.destination()));
  const tse::CarrierCode& carrier = fareMarket.governingCarrier();
  const LocCode& boardMultiCity = fareMarket.boardMultiCity();
  const LocCode& offMultiCity = fareMarket.offMultiCity();

  bool diffBoardPoint = (!boardMultiCity.empty()) && (boardMultiCity != origin);
  bool diffOffPoint = (!offMultiCity.empty()) && (offMultiCity != destination);

  bool publishedRC =
      validatePublishedCurrency(recordScope, origin, destination, carrier, currency, ticketingDate);
  if (publishedRC)
    return true;

  if (diffBoardPoint && diffOffPoint)
  {
    publishedRC = validatePublishedCurrency(
        recordScope, boardMultiCity, offMultiCity, carrier, currency, ticketingDate);
  }

  if (publishedRC)
    return true;

  if (diffBoardPoint)
  {
    publishedRC = validatePublishedCurrency(
        recordScope, boardMultiCity, destination, carrier, currency, ticketingDate);
  }

  if (publishedRC)
    return true;

  if (diffOffPoint)
  {
    publishedRC = validatePublishedCurrency(
        recordScope, origin, offMultiCity, carrier, currency, ticketingDate);
  }

  return publishedRC;
}

//--------------------------------------------------------------------------------------------
//
//   @method getCurrencySelection
//
//   Description: Retrieves a vector of CurrencySelection records using the nation.
//                We have to do 2 searches. If the nation is not found then search
//                for a blank(default) nation.
//
//   @param PricingTrx              -  transaction object
//   @param NationCode       -  nation
//   @param DateTime      -  ticket date
//
//   @return void
//--------------------------------------------------------------------------------------------
const std::vector<tse::CurrencySelection*>&
CurrencySelectionValidator::getCurrencySelection(PricingTrx& trx,
                                                 NationCode& nation,
                                                 const DateTime& ticketDate,
                                                 bool& useDefaultNation)
{

  LOG4CXX_INFO(logger, "Entered  CurrencySelectionValidator::getCurrencySelection");

  const std::vector<CurrencySelection*>& list1 =
      trx.dataHandle().getCurrencySelection(nation, ticketDate);

  LOG4CXX_INFO(logger, "Leaving  CurrencySelectionValidator::getCurrencySelection");
  return list1;
}

//--------------------------------------------------------------------------------------------
//
//   @method getAseanCurrencies
//
//   Description: Retrieves a vector of Asean currencies.
//
//   @param PricingTrx     -  transaction object
//   @param PaxTypeFare    -  fare
//   @param NationCode     -  nation
//
//   @return void
//--------------------------------------------------------------------------------------------
void
CurrencySelectionValidator::getAseanCurrencies(PricingTrx& trx,
                                               NationCode& nationCode,
                                               FareMarket& fareMarket)
{
  bool useDefaultNation = false;

  LOG4CXX_INFO(logger, "Entering  CurrencySelectionValidator::getAseanCurrencies");

  const FMDirection direction = fareMarket.direction();

  const DateTime& ticketingDate = trx.getRequest()->ticketingDT();

  const std::vector<CurrencySelection*>& currencySelection =
      getCurrencySelection(trx, nationCode, ticketingDate, useDefaultNation);

  LOG4CXX_DEBUG(logger,
                "Size of CurrencySelection vector returned : " << currencySelection.size()
                                                               << " for nation: " << nationCode);

  if (!currencySelection.empty())
  {
    std::vector<CurrencySelection*>::const_iterator curSelIter = currencySelection.begin();
    std::vector<CurrencySelection*>::const_iterator curSelIterEnd = currencySelection.end();

    for (; curSelIter != curSelIterEnd; ++curSelIter)
    {
      const tse::CurrencySelection* curSelectionRec = *curSelIter;

      if (!curSelectionRec->aseanCurs().empty())
      {
        std::vector<CurrencyCode>::const_iterator curSelAseanIter =
            curSelectionRec->aseanCurs().begin();
        std::vector<CurrencyCode>::const_iterator curSelAseanIterEnd =
            curSelectionRec->aseanCurs().end();

        for (; curSelAseanIter != curSelAseanIterEnd; ++curSelAseanIter)
        {
          const CurrencyCode& curSelAseanCur = *curSelAseanIter;

          std::vector<CurrencyCode>::iterator inIter =
              std::find(fareMarket.inBoundAseanCurrencies().begin(),
                        fareMarket.inBoundAseanCurrencies().end(),
                        curSelAseanCur);

          std::vector<CurrencyCode>::iterator outIter =
              std::find(fareMarket.outBoundAseanCurrencies().begin(),
                        fareMarket.outBoundAseanCurrencies().end(),
                        curSelAseanCur);
          if (direction == FMDirection::OUTBOUND)
          {

            if (outIter == fareMarket.outBoundAseanCurrencies().end())
              fareMarket.outBoundAseanCurrencies().push_back(curSelAseanCur);
          }
          else if (direction == FMDirection::INBOUND)
          {

            if (inIter == fareMarket.inBoundAseanCurrencies().end())
              fareMarket.inBoundAseanCurrencies().push_back(curSelAseanCur);
          }
          else if (direction == FMDirection::UNKNOWN)
          {
            if (outIter == fareMarket.outBoundAseanCurrencies().end())
              fareMarket.outBoundAseanCurrencies().push_back(curSelAseanCur);

            if (inIter == fareMarket.inBoundAseanCurrencies().end())
              fareMarket.inBoundAseanCurrencies().push_back(curSelAseanCur);
          }
        }
      }
    }
  }

  LOG4CXX_INFO(logger, "Leaving  CurrencySelectionValidator::getAseanCurrencies");
}

void
CurrencySelectionValidator::setDiag(DiagCollector* diag212)
{
  _diag = diag212;
}

void
CurrencySelectionValidator::diagStart(uint16_t currVecsize, NationCode& nationCode)
{
  if (LIKELY(_diag == nullptr))
    return;
  DiagCollector& diag = *(_diag);
  diag << " \n";
  diag << "   ENTER CURRENCY SELECTION VALIDATOR-NATION CODE: ";
  if (nationCode.empty())
    diag << "BLANK \n";
  else
    diag << nationCode << " \n";
  diag << "    CURRENCY SELECTION VECTOR SIZE: " << currVecsize << " \n";
}

void
CurrencySelectionValidator::diagStartAlt(CurrencyCode& alternateCurrency, NationCode& nationCode)
{
  if (_diag == nullptr)
    return;
  DiagCollector& diag = *(_diag);
  diag << " \n";
  diag << "   ENTER CURRENCY SELECTION VALIDATOR-NATION CODE: ";
  if (nationCode.empty())
    diag << "BLANK \n";
  else
    diag << nationCode << " \n";
  diag << "   ALTERNATE CURRENCY: " << alternateCurrency << " \n";
}

void
CurrencySelectionValidator::diagAlt(uint16_t currVecsize)
{
  if (_diag == nullptr)
    return;
  DiagCollector& diag = *(_diag);
  diag << "    CURRENCY SELECTION VECTOR SIZE: " << currVecsize << " \n";
}

void
CurrencySelectionValidator::diagEnd(bool determinedCurr)
{
  if (LIKELY(_diag == nullptr))
    return;
  DiagCollector& diag = *(_diag);
  diag << "   EXIT  CURRENCY SELECTION VALIDATOR: ";

  if (determinedCurr)
    diag << "TRUE";
  else
    diag << "FALSE";

  diag << " \n";
}

void
CurrencySelectionValidator::diagCurrRec(const CurrencySelection* csRec)
{
  if (LIKELY(_diag == nullptr))
    return;
  DiagCollector& diag = *(_diag);
  diag << " \n";
  diag << "    SEQ:" << csRec->seqNo() << " JRNY:" << csRec->journeyRestr()
       << " FC:" << csRec->farecomponentRestr() << " POS/EX:" << csRec->posLoc().locType() << "-"
       << csRec->posLoc().loc() << "/" << csRec->posexcept()
       << " CURR:" << csRec->fareCompPrimeCur() << " PAX EX:" << csRec->psgTypeExcept() << " \n";

  //   bool geoMatch = LocUtil::isInLoc(*(trx.getRequest()->ticketingAgent()->agentLocation()),

  diag << "    GOV CXR EX:" << csRec->govCarrierExcept() << "  CXRS:";
  uint16_t i = 0;
  if (!csRec->govCarriers().empty())
  {
    CurSelectionGovCxrVec& curSelectionGovCxr = csRec->govCarriers();
    std::vector<CarrierCode>::const_iterator curSelGovCxrIter = curSelectionGovCxr.begin();
    std::vector<CarrierCode>::const_iterator curSelGovCxrIterEnd = curSelectionGovCxr.end();

    for (; curSelGovCxrIter != curSelGovCxrIterEnd; ++curSelGovCxrIter)
    {
      const CarrierCode& govCarrierCode = *curSelGovCxrIter;
      diag << govCarrierCode << " ";
      ++i;
      if (i == 11)
      {
        diag << " \n";
        diag << "                      ";
        i = 0;
      }
    }
  }
  diag << " \n";

  diag << "    PAXTYPES:";
  CurSelectionPaxTypeVec& curSelectionPaxType = csRec->passengerTypes();

  if (!curSelectionPaxType.empty())
  {
    std::vector<PaxTypeCode>::const_iterator curSelPaxTypeIter = curSelectionPaxType.begin();
    std::vector<PaxTypeCode>::const_iterator curSelPaxTypeIterEnd = curSelectionPaxType.end();
    i = 0;
    for (; curSelPaxTypeIter != curSelPaxTypeIterEnd; ++curSelPaxTypeIter)
    {
      const PaxTypeCode& paxTypeCode = *curSelPaxTypeIter;
      diag << paxTypeCode << " ";
      ++i;
      if (i == 12)
      {
        diag << " \n";
        diag << "             ";
        i = 0;
      }
    }
  }

  diag << " \n";
}

void
CurrencySelectionValidator::diagAltCurrRec(const CurrencySelection* csRec)
{
  if (_diag == nullptr)
    return;
  DiagCollector& diag = *(_diag);
  unsigned int numResCurrencies = csRec->restrictedCurs().size();
  diag << " \n";
  diag << "    SEQ:" << csRec->seqNo() << " RESTRICTED CURR EXEMPT:" << csRec->restrCurExcept()
       << " SIZE:" << numResCurrencies << " \n";
  diag << "    RESTRICTED CURRENCIES: ";

  std::vector<CurrencyCode>::const_iterator resCurIter = csRec->restrictedCurs().begin();
  std::vector<CurrencyCode>::const_iterator resCurIterEnd = csRec->restrictedCurs().end();

  unsigned int rIndex = 0;

  for (; resCurIter != resCurIterEnd; ++resCurIter)
  {
    const CurrencyCode& restrictedCur = *resCurIter;
    diag << restrictedCur << " ";
    ++rIndex;
    if (rIndex == 9)
    {
      diag << " \n";
      diag << "                           ";
      rIndex = 0;
    }
  }
  diag << " \n";
}

void
CurrencySelectionValidator::diagMsg(DiagMsgs diagMsg)
{
  if (LIKELY(_diag == nullptr))
    return;
  DiagCollector& diag = *(_diag);

  switch (diagMsg)
  {
  case JOURNEY_RESTRICTION_FAIL:
  {
    diag << "    JOURNEY RESTRICTION FAILED-CONTINUE TO NEXT SEQUENCE \n";
    break;
  }
  case FARE_COMP_RESTRICTION_FAIL:
  {
    diag << "    FARE COMPONENT RESTRICTION FAILED-CONTINUE TO NEXT SEQUENCE \n";
    break;
  }
  case GOVERNING_CARRIER_FAIL:
  {
    diag << "    GOVERNING CARRIER CHECK FAILED-CONTINUE TO NEXT SEQUENCE \n";
    break;
  }
  case PAXTYPE_FAIL:
  {
    diag << "    PAX TYPE CHECK FAILED-CONTINUE TO NEXT SEQUENCE \n";
    break;
  }
  case POINT_OF_ISSUE_FAIL:
  {
    diag << "    POINT OF SALE CHECK FAILED-CONTINUE TO NEXT SEQUENCE \n";
    break;
  }
  case PRIME_CURRNCY_EMPTY:
  {
    diag << "    PRIMARY CURRENCY EMPTY-CONTINUE TO NEXT SEQUENCE \n";
    break;
  }
  case CURRENCY_REC_PASS:
  {
    diag << "    ITEM PASS-APPLY DATA \n";
    break;
  }
  case ALT_CURR_NOT_PUBLISHED:
  {
    diag << "    ALTERNATE CURRENCY IS NOT PUBLISHED IN THIS MARKET \n";
    break;
  }
  case RESTRICTED_FAIL:
  {
    diag << "    RESTRICTED CHECK FAIL-CONTINUE TO NEXT SEQUENCE \n";
    break;
  }
  case RESTRICTED_PASS:
  {
    diag << "    RESTRICTED CHECK PASS-APPLY DATA \n";
    break;
  }
  default:
  {
    diag << "    -CONTINUE TO NEXT SEQUENCE \n";
    break;
  }
  }
}

//--------------------------------------------------------------------------------------------
//
//   @method validatePublishedCurrency
//
//
//   Description: Determines whether or not the currency from the Currency
//                is published or not.
//
//   @param RecordScope  - used by DBAccess to control search
//   @param LocCode      - market1 code
//   @param LocCode      - market2 code
//   @param CarrierCode  - code for this carrier
//   @param CurrencyCode - currency code from currency selection table
//   @param DateTime  - ticket date
//
//   @return bool  - true/false, false is failure
//
//--------------------------------------------------------------------------------------------
bool
CurrencySelectionValidator::validatePublishedCurrency(RecordScope recordScope,
                                                      const LocCode& market1,
                                                      const LocCode& market2,
                                                      const CarrierCode& cxr,
                                                      const CurrencyCode& currency,
                                                      const DateTime& ticketingDate)
{

  LOG4CXX_INFO(
      logger,
      "Entered CurrencySelectionValidator::validatePublishedCurrency - Alternate Currency ");
  //   diagStartAlt(alternateCurrency, nationCode);

  DataHandle dataHandle(ticketingDate);

  LOG4CXX_DEBUG(logger, "Entered   CurrencySelectionValidator::validatePublishedCurrency");

  LOG4CXX_DEBUG(logger, "Record Scope: " << recordScope);
  LOG4CXX_DEBUG(logger, "Market 1 : " << market1);
  LOG4CXX_DEBUG(logger, "Market 2 : " << market2);
  LOG4CXX_DEBUG(logger, "Carrier Code : " << cxr);
  LOG4CXX_DEBUG(logger, "Currency : " << currency);
  LOG4CXX_DEBUG(logger, "Ticketing Date: " << ticketingDate.toSimpleString());

  const std::vector<const FareInfo*>& publishedFares =
      dataHandle.getFaresByMarketCxr(market1, market2, cxr, ticketingDate);

  std::vector<const FareInfo*>::const_iterator fareInfoI = publishedFares.begin();

  for (; fareInfoI != publishedFares.end(); fareInfoI++)
  {
    const FareInfo* fareInfo = *fareInfoI;
    LOG4CXX_DEBUG(logger, "FARE INFO CURRENCY: " << fareInfo->currency());

    if (fareInfo->currency() == currency)
    {
      LOG4CXX_INFO(logger,
                   "Currency " << currency << " is published for market1: " << market1
                               << " market2: " << market2 << " carrier: " << cxr);
      LOG4CXX_INFO(logger, "Leaving   CurrencySelectionValidator::validatePublishedCurrency");

      return true;
    }
  }

  return false;
}
}
