//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
#include "BookingCode/FareDisplayBookingCode.h"

#include "BookingCode/FareDisplayBookingCodeException.h"
#include "Common/ErrorResponseException.h"
#include "Common/Logger.h"
#include "Common/PaxTypeFareRuleDataCast.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/DiscountInfo.h"
#include "Diagnostic/Diagnostic.h"
#include "FareDisplay/InclusionCodeConsts.h"

#include <iomanip>
#include <iostream>
#include <vector>

namespace tse
{
const std::string FareDisplayBookingCode::DOLLARBLANK = "$ ";
const std::string FareDisplayBookingCode::TWO_BLANKS = "  ";
const std::string FareDisplayBookingCode::DOLLAR = "$";

//---------------------------------------------------------------------
//  This is a base class for the Booking Code display  process.
//---------------------------------------------------------------------

log4cxx::LoggerPtr
FareDisplayBookingCode::_logger(
    log4cxx::Logger::getLogger("atseintl.BookingCode.FareDisplayBookingCode"));

//---------------------------------------------------------------
//  The Reservation Booking Designation (RBD) validation process
//  invokes by this method.
//---------------------------------------------------------------
void
FareDisplayBookingCode::getBookingCode(FareDisplayTrx& trx,
                                       PaxTypeFare& ptf,
                                       BookingCode& bookingCode)
{
  LOG4CXX_DEBUG(_logger,
                "Entered FareDisplayBookingCode::getBookingCode() - fbc:" << ptf.fareClass());
  bookingCode = TWO_BLANKS;
  std::vector<BookingCode> bkgCodes;
  _trx = &trx;
  TravelSegPtrVecI iterTvl = ptf.fareMarket()->travelSeg().begin();
  AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);
  setAirSeg(&airSeg);

  // Check industry fare
  if (ptf.fare()->isIndustry() || ptf.fare()->carrier() == INDUSTRY_CARRIER)
  {
    // Check input YY/**/ALL carriers
    if ((trx.getOptions()->isAllCarriers() || (isYYCarrierRequest(trx.preferredCarriers()))) &&
        ptf.fareMarket()->governingCarrier() == INDUSTRY_CARRIER && isNotRBReady())
    {
      bookingCode = DOLLARBLANK;
    }
    else if (ptf.fcasCarrierApplTblItemNo() != 0 && validateIndustryFareBkgCode(ptf, bkgCodes))
    {

      setRBBookingCodes(bkgCodes);
      bookingCode = bkgCodes.front();
      ptf.s8BFBookingCode() = bookingCode;
      if (bkgCodes.size() > 1)
      {
        updateBookingCode(bookingCode);
      }
    }
    else
    {
      // Process record 6 convention 1
      if (!validateRecord6Conv1(ptf, airSeg, bkgCodes, *iterTvl))
      {
        bookingCode = TWO_BLANKS;
      }
      else
      {
        bookingCode = bkgCodes.front();
        ptf.s8BFBookingCode() = bookingCode;
        if (bkgCodes.size() > 1)
        {
          updateBookingCode(bookingCode);
        }
      }
    }
  }
  // Check Published, Constructed, FareByRule and Discouted fares
  else
  {
    validateFareBkgCode(ptf, bkgCodes);

    if (bkgCodes.empty())
    {
      bookingCode = TWO_BLANKS;
    }
    else
    {
      bookingCode = bkgCodes.front();
      ptf.s8BFBookingCode() = bookingCode;
      if (bkgCodes.size() > 1)
      {
        updateBookingCode(bookingCode);
      }
    }
  }

  LOG4CXX_DEBUG(_logger,
                "Leaving FareDisplayBookingCode::getBookingCode()" << bookingCode
                                                                   << " fcc=" << ptf.fareClass());
}

//---------------------------------------------------------------
// validateT990
// This method is called to validate the input specified carrier
// in the Carrier Application Table of YY fare.
//---------------------------------------------------------------
bool
FareDisplayBookingCode::validateIndustryFareBkgCode(PaxTypeFare& ptf,
                                                    std::vector<BookingCode>& bkgCodes)
{
  LOG4CXX_DEBUG(_logger, "Entering FareDisplayBookingCode::validateIndustryFareBkgCode()");

  uint16_t carrierApplTblItemNo = ptf.fcasCarrierApplTblItemNo();
  if (carrierApplTblItemNo == 0)
  {
    return false;
  }

  const std::vector<CarrierApplicationInfo*>& carrierApplList =
      getCarrierApplication(ptf.vendor(), carrierApplTblItemNo);
  if (carrierApplList.empty())
  {
    return false;
  }

  FareMarket* mkt = ptf.fareMarket();
  CarrierCode& airCxr = mkt->governingCarrier();
  if (findCXR(airCxr, carrierApplList))
  {
    // Get Rec1b booking codes
    return getPrimeBookingCode(ptf, bkgCodes);
  }
  // go with Rec6Conv1
  return false;
}

//---------------------------------------------------------------
// Validte input carrier against the Carrier Application Info table 990
//---------------------------------------------------------------
bool
FareDisplayBookingCode::findCXR(const CarrierCode& cxr,
                                const std::vector<CarrierApplicationInfo*>& carrierApplList)
{
  LOG4CXX_DEBUG(_logger, "Entering FareDisplayBookingCode::findCXR()");
  CarrierApplicationInfoListPtrIC iterCxr = carrierApplList.begin();

  for (; iterCxr != carrierApplList.end(); iterCxr++)
  {
    if ((cxr == (*iterCxr)->carrier()) && ((*iterCxr)->applInd() != 'X'))
    {
      setCarrierMatchedTable990();
      return true; // proceed to Prime RBD
    }
  }

  iterCxr = carrierApplList.begin();
  for (; iterCxr != carrierApplList.end(); iterCxr++)
  {
    if ((*iterCxr)->applInd() == 'X') // any "negative" item carrier in the table
    {
      if (cxr == (*iterCxr)->carrier())
        return false; // proceed to Rec6 Conv 1
    }
  }

  iterCxr = carrierApplList.begin();
  for (; iterCxr != carrierApplList.end(); iterCxr++)
  {
    if ((*iterCxr)->applInd() == '$') // any item carrier in the table
    {
      return true; // proceed to Prime RBD
    }
  }
  return false;
}

//------------------------------------------------------------------------
// Invoke the record 6 Convention 1 validate method and call LocalMarket
//------------------------------------------------------------------------
bool
FareDisplayBookingCode::validateRecord6Conv1(PaxTypeFare& paxTypeFare,
                                             AirSeg* airSeg,
                                             std::vector<BookingCode>& bkgCodes,
                                             TravelSeg* seg)
{
  // Validate Convention 1
  LOG4CXX_DEBUG(_logger, "Entering FareDisplayBookingCode::validateRecord6Conv1()");
  bool validReturnType = validateConvention1(paxTypeFare, paxTypeFare.vendor(), airSeg, bkgCodes);
  if (validReturnType && !bkgCodes.empty())
    return true;
  else
  {
    //    if (!paxTypeFare.getPrimeBookingCode(bkgCodes) || bkgCodes.empty())
    if (!getPrimeBookingCode(paxTypeFare, bkgCodes) || bkgCodes.empty())
    {
      return false;
    }
    if (isRBReady() && isGetBookingCodesEmpty() && !bkgCodes.empty())
    {
      setRBBookingCodes(bkgCodes);
    }
    return true;
  }
}

//-------------------------------------------------------------
// Validate BookingCode for the Particular Travel Sector
//-------------------------------------------------------------
bool
FareDisplayBookingCode::validateSectorPrimeRBDRec1(AirSeg* travelSeg,
                                                   std::vector<BookingCode>& bkgCodes,
                                                   bool dispRec1)
{
  LOG4CXX_DEBUG(_logger, "Entering FareDisplayBookingCode::validateSectorPrimeRBDRec1()");
  if (bkgCodes.empty()) // No booking codes in Rec 1
  {
    return false;
  }
  return true;
}

//---------------------------------------------------------------------
// Retrieve the Convention object and invoke the Record6 Convention 2
// or 1 validate method.
//---------------------------------------------------------------------

bool
FareDisplayBookingCode::validateConvention1(PaxTypeFare& paxTypeFare,
                                            VendorCode vendorA,
                                            AirSeg* airSeg,
                                            std::vector<BookingCode>& bkgCodes)
{
  LOG4CXX_DEBUG(_logger, "Entering FareDisplayBookingCode::validateConvention1()");
  bool returnType = false;
  if (airSeg == nullptr)
  {
    return returnType;
  }

  // Convention 1 does not have the Tariff and Rule as the Primary keys
  if (vendorA.equalToConst("SITA"))
  {
    vendorA = "ATP";
  }

  bool convention2 = false;
  return getBookingCodeException(paxTypeFare, vendorA, airSeg, bkgCodes, convention2);
}

//-----------------------------------------------------------
// The RBD validation process for all Fares, except Industry.
//-----------------------------------------------------------
bool
FareDisplayBookingCode::validateFareBkgCode(PaxTypeFare& paxTypeFare,
                                            std::vector<BookingCode>& bkgCodes)
{
  LOG4CXX_DEBUG(_logger, "Entering FareDisplayBookingCode::validateFareBkgCode()");
  getPrimeBookingCode(paxTypeFare, bkgCodes);
  bool returnBkgCode = false;
  long itemNumber = paxTypeFare.fcasBookingCodeTblItemNo(); //  Table 999
  bool fbrActive = false;
  if (paxTypeFare.isFareByRule())
  {
    fbrActive = true;
  }

  setRBBookingCodes(bkgCodes);

  //  Cat 25
  const FBRPaxTypeFareRuleData* fbrPTFare = nullptr;
  if (fbrActive)
  {
    fbrPTFare = paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);
    if (fbrPTFare == nullptr)
    {
      return returnBkgCode;
    }
    if (!paxTypeFare.vendor().equalToConst("ATP") && !paxTypeFare.vendor().equalToConst("SITA") &&
        itemNumber)
    {
      itemNumber = 0;
    }
  }

  if (itemNumber != 0)
  {
    returnBkgCode = validateBookingCodeTblItemNo(paxTypeFare, bkgCodes, fbrActive);
  }
  else
  {
    // No REC1 Table 999 item number exists, check for Domestic
    if (paxTypeFare.fareMarket()->travelBoundary().isSet(FMTravelBoundary::TravelWithinUSCA))
    {
      returnBkgCode = validateDomestic(paxTypeFare, bkgCodes);
      return returnBkgCode;
    }
    // Validate International Market
    if (fbrPTFare != nullptr)
    {
      returnBkgCode = validateFBRFare(paxTypeFare, bkgCodes, false);
    }
    else
    {
      // Record 6 Convention 2 validation for the Fare Market
      TravelSegPtrVecI iterTvl = paxTypeFare.fareMarket()->travelSeg().begin();
      AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);
      setAirSeg(&airSeg);

      if (airSeg == nullptr || (!validateConvention2(paxTypeFare, paxTypeFare, airSeg, bkgCodes)))
      {
        // International RBD validation = apply PrimeRBD Rec1, Rec6 Conv1, local
        // market validations for the international FareMarket
        returnBkgCode = validateRBDInternational(paxTypeFare, bkgCodes);
      }
    }
  }
  return returnBkgCode;
}

//--------------------------------------------------------------
// TONY
//--------------------------------------------------------------
bool
FareDisplayBookingCode::validateDomestic(PaxTypeFare& paxTfare, std::vector<BookingCode>& bkgCodes)
{
  LOG4CXX_DEBUG(_logger, "Entering FareDisplayBookingCode::validateDomestic()");
  bool validReturnType = true; // defult setup

  // Apply Prime RBD from the Rec 1 for the Domestic FareMarket
  if (validatePrimeRBDRecord1Domestic(paxTfare, bkgCodes))
  {
    validReturnType = true;
  }
  else
  {
    // if command pricing e.g WPQY26 then PASS the fare
    if (paxTfare.validForCmdPricing(false))
    {
      validReturnType = true;
    }
    else
    {
      validReturnType = false;
    }
  }

  return validReturnType;
}

//-----------------------------------------------------------------
// Validate Booking code in the travel segment against Prime RBD in
// the Record1.  Loop over the valid booking codes in the Record 1
// and determine if the flight class of service matches one of them.
//-----------------------------------------------------------------
bool
FareDisplayBookingCode::validatePrimeRBDRecord1Domestic(PaxTypeFare& paxTypeFare,
                                                        std::vector<BookingCode>& bkgCodes)
{
  LOG4CXX_DEBUG(_logger, "Entering FareDisplayBookingCode::validatePrimeRBDRecord1Domestic()");
  const FBRPaxTypeFareRuleData* fbrPTFare =
      paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE); // Cat 25 pointer
  const CategoryRuleInfo* fbrRuleInfo = nullptr; // Rec 2
  bool cat25Rec2Carrier = true; // Cat25 Rec2 Cxr does match Marketing Cxr
  //  paxTypeFare.getPrimeBookingCode(bkgCodes);    // gets Bkgs Codes from Rec 1B
  getPrimeBookingCode(paxTypeFare, bkgCodes); // gets Bkgs Codes from Rec 1B
  if (fbrPTFare == nullptr) // If it's not a CAT25 Fare,
  {
    if (bkgCodes.empty()) // and, no Prime RBD exists -> return
    {
      return false; // No book codes in Rec 1
    }
  }
  setRBBookingCodes(bkgCodes);

  bool dispRec1 = true;

  // Loop all Travel segments for this PaxTypeFare
  TravelSegPtrVecI iterTvl = paxTypeFare.fareMarket()->travelSeg().begin();

  AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);
  setAirSeg(&airSeg);

  if (fbrPTFare != nullptr) // Cat 25 Fare ?
  {
    // If Cat25 fare, Rec 2 Carrier code must match the marketing carrier
    fbrRuleInfo = fbrPTFare->categoryRuleInfo(); // Rec 2 FBR

    if (airSeg->carrier() != fbrRuleInfo->carrierCode())
    {
      cat25Rec2Carrier = false;
    }

    // If it's a CAT25 Fare and CXR's are match, and, no Prime RBD (booking codes) exists
    if ((validateRBDforDiscountFBRFare(paxTypeFare, bkgCodes, dispRec1)) == RET_CONTINUE)
    {
      return false;
    }
  } // FBR PaxTypeFare

  if (!cat25Rec2Carrier) // Cat25 Rec2 Carrier code does not match the Marketing carrier
  {
    return false;
  }
  return true;
}

// -------------------------------------------------------------
// Prime RBD does not exist in the Cat25 Fare Resulting Fare
// -------------------------------------------------------------
FareDisplayBookingCode::ValidateReturnType
FareDisplayBookingCode::validateRBDforDiscountFBRFare(PaxTypeFare& paxTypeFare,
                                                      std::vector<BookingCode>& bkgCodes,
                                                      bool& dispRec1)
{
  LOG4CXX_DEBUG(_logger, "Entering FareDisplayBookingCode::validateRBDforDiscountFBRFare()");
  ValidateReturnType validReturnType = RET_PASS; // default

  // Cat25 & Cat 19
  uint16_t cat19 = 19;

  const FBRPaxTypeFareRuleData* fbrPTFare =
      paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE); // Cat 25 pointer
  if (fbrPTFare == nullptr)
  {
    return (validReturnType = RET_FAIL);
  }
  // Discounted FBR fare pointer
  const PaxTypeFareRuleData* diskPTfare = paxTypeFare.paxTypeFareRuleData(cat19); // Cat 19 pointer

  // If it's a CAT25 Fare and CXR's are match, and, no Prime RBD exists

  if (!fbrPTFare->isSpecifiedFare() && !paxTypeFare.isDiscounted())
  {
    fbrPTFare->getBaseFarePrimeBookingCode(bkgCodes); // Base Fare
    dispRec1 = false; // do not display Rec1 RBD in the validateSectorPrimeRBDRec1 method
  }
  else if (fbrPTFare->isSpecifiedFare() && !paxTypeFare.isDiscounted())
  {
    validReturnType = RET_CONTINUE;
  }
  else if (paxTypeFare.isDiscounted())
  {
    if (diskPTfare == nullptr)
    {
      validReturnType = RET_CONTINUE;
    }
    else

    {
      const DiscountInfo* discountInfo =
          dynamic_cast<const DiscountInfo*>(diskPTfare->ruleItemInfo());
      if (discountInfo != nullptr)
      {
        if (discountInfo->bookingCode().empty())
        {
          if (fbrPTFare->isSpecifiedFare())
          {
            validReturnType = RET_CONTINUE;
          }
          else
          {
            fbrPTFare->getBaseFarePrimeBookingCode(bkgCodes); // Prime RBD Base Fare
          }
        }
        else
        {
          bkgCodes.push_back(discountInfo->bookingCode());
        }
      }
    } // No RBD exists in the Discounted 19-22 Fare
  } // Discounted PaxTypeFare

  // Set FBR bookingCodes for RB rule text
  setRBBookingCodes(bkgCodes);

  return validReturnType;
}

//----------------------------------------------------------------------
//   Table 999 item number exists in the FareClassAppSegInfo (Record 1B)
//   TONY
//----------------------------------------------------------------------
bool
FareDisplayBookingCode::validateBookingCodeTblItemNo(PaxTypeFare& paxTypeFare,
                                                     std::vector<BookingCode>& bkgCodes,
                                                     bool fbrActive)
{
  LOG4CXX_DEBUG(_logger, "Entering FareDisplayBookingCode::validateBookingCodeTblItemNo()");
  bool validReturnType = true;

  //  NoMatch means - nomatch is found in the DB or/and cache
  if (validateT999(paxTypeFare, bkgCodes))
  {
    return validReturnType;
  } //   Table  999 data are not-Match or are not foun in the Data Base

  // Two ways of validation will be applied depends on Domestic or International
  // First, check for Domestic
  if (paxTypeFare.fareMarket()->travelBoundary().isSet(
          FMTravelBoundary::TravelWithinUSCA)) // It is Domestic fare component
  {
    validReturnType = validateDomestic(paxTypeFare, bkgCodes);
  }
  else
  {
    // International RBD validation = apply PrimeRBD Rec 1, Rec6 Conv1
    // validations for the international FareMarket
    validReturnType = RET_PASS;

    if (fbrActive) // FBR fare
    {
      validReturnType = validateFBRFare(paxTypeFare, bkgCodes, fbrActive);
    }
    else
    {
      validReturnType = validateRBDInternational(paxTypeFare, bkgCodes);
    }
  }
  return validReturnType;
}

// -----------------------------------------------------------------
// Retrieve the Convention object and invoke the Record6
// Convention 2 validate method for the FBR fare.
// TONY
// -----------------------------------------------------------------
bool
FareDisplayBookingCode::validateConvention2(PaxTypeFare& paxTypeFare,
                                            PaxTypeFare& basePaxTypeFare,
                                            AirSeg* airSeg,
                                            std::vector<BookingCode>& bkgCodes)
{
  LOG4CXX_DEBUG(_logger, "Entering FareDisplayBookingCode::validateConvention2()");
  // bool returnType = true;           // default setup

  if (airSeg == nullptr)
  {
    return false;
  }
  VendorCode vendor = const_cast<VendorCode&>(basePaxTypeFare.vendor());
  bool convention2 = true;
  // SPR105891 - we pass FBR fare needed in validation
  return getBookingCodeException(paxTypeFare, vendor, airSeg, bkgCodes, convention2);
}

// -----------------------------------------------------------------------
// International Non-FBR RBD validation = apply PrimeRBD Rec 1, Rec6 Conv1
// validations for the international FareMarket
// TONY
// -----------------------------------------------------------------------
bool
FareDisplayBookingCode::validateRBDInternational(PaxTypeFare& paxTypeFare,
                                                 std::vector<BookingCode>& bkgCodes)
{
  bool validReturnType = true;
  if (isSecondary())
  {
    FareMarket* mkt = paxTypeFare.fareMarket();
    TravelSegPtrVecI iterTvl = mkt->travelSeg().begin();
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);
    setAirSeg(&airSeg);
    if (airSeg->carrier() == paxTypeFare.carrier())
      getPrimeBookingCode(paxTypeFare, bkgCodes); // gets Bkgs Codes from Rec 1B
    else
      validateConvention1(paxTypeFare, paxTypeFare.vendor(), airSeg, bkgCodes);
  }
  else
  {
    // preparation to get Booking codes from the Record 1
    //  paxTypeFare.getPrimeBookingCode(bkgCodes); // gets Bkgs Codes from Rec 1B
    getPrimeBookingCode(paxTypeFare, bkgCodes); // gets Bkgs Codes from Rec 1B
  }
  setRBBookingCodes(bkgCodes);
  return validReturnType;
}

//-----------------------------------------------------
// Validate FBR Fare
//-----------------------------------------------------
bool
FareDisplayBookingCode::validateFBRFare(PaxTypeFare& paxTypeFare,
                                        std::vector<BookingCode>& bkgCodes,
                                        bool fbrActive)
{

  LOG4CXX_DEBUG(_logger, "Entering FareDisplayBookingCode::validateFBRFare()");
  //   FBRPaxTypeFareRuleData* fbrData = paxTypeFare.getFbrRuleData(uint16_t (25));
  const FBRPaxTypeFareRuleData* fbrData = paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);
  if (fbrData == nullptr || !fbrData->getBaseFarePrimeBookingCode(bkgCodes))
  {
    bkgCodes.push_back("  ");
  }
  bool validReturnType = true;
  validReturnType = validateFBR_RBDInternational(paxTypeFare, bkgCodes, fbrActive);

  if (!validReturnType)
  {
    if (fbrData != nullptr)
    {
      fbrData->getBaseFarePrimeBookingCode(bkgCodes);
      setRBBookingCodes(bkgCodes);
      if (!fbrData->isSpecifiedFare()) // FBR calculated
      {
        // Check base fare record 1's booking code table item no
        PaxTypeFare* pBaseF = fbrData->baseFare();
        if (pBaseF != nullptr) // Base Fare present
        {
          long itemNumber = pBaseF->fcasBookingCodeTblItemNo(); // "TABLE 999 ITEM NUMBER - "
                                                                // // default Table999 item Number

          if (!(paxTypeFare.vendor() == ATPCO_VENDOR_CODE ||
                paxTypeFare.vendor() == SITA_VENDOR_CODE))
          {
            const FareByRuleItemInfo* fbrItemInfo =
                dynamic_cast<const FareByRuleItemInfo*>(fbrData->ruleItemInfo());
            if (fbrItemInfo != nullptr && fbrItemInfo->primeSector() == 'X')
            {
              itemNumber = 0;
            }
          }

          if (itemNumber != 0)
          {
            // if Table999 item presents in the Base Fare Record 1B
            // validReturnType = validateBookingCodeTblItemNo( *pBaseF, bkgCodes, fbrActive );
            validReturnType = validateT999(*pBaseF, bkgCodes);
            if (validReturnType)
              return validReturnType;
          }
          // No REC1 table 999 item number exists, check for domestic
          if (paxTypeFare.fareMarket()->travelBoundary().isSet(
                  FMTravelBoundary::TravelWithinUSCA)) // It is Domestic fare component
          {
            validReturnType = validateDomestic(paxTypeFare, bkgCodes);
            return validReturnType;
          }
          // Record 6 Convention 2 validation for the Fare Market
          TravelSegPtrVecI iterTvl = paxTypeFare.fareMarket()->travelSeg().begin();
          AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);
          setAirSeg(&airSeg);
          if (airSeg == nullptr ||
              !validateConvention2(paxTypeFare, *(fbrData->baseFare()), airSeg, bkgCodes))
          {
            validReturnType = finalValidateFBR_RBDInternational(paxTypeFare, bkgCodes);
          }
        }
      }
    }
  }
  return validReturnType;
}

// ----------------------------------------------------------------
// Retrieve the ExceptT999 object from the objectManager and invoke
// the object's validate method.
// ----------------------------------------------------------------
bool
FareDisplayBookingCode::validateT999(PaxTypeFare& paxTypeFare, std::vector<BookingCode>& bkgCodes)
{
  LOG4CXX_DEBUG(_logger, "Entering FareDisplayBookingCode::validateT999()");
  FareDisplayBookingCodeException fdbce(_trx, &paxTypeFare); // BookingCodeExcept object
  return fdbce.getBookingCodeException(bkgCodes);
}

//--------------------------------------------------------------------
// International RBD validation = apply PrimeRBD Rec 1 only (needs for
// FBR fares) validations for the international FareMarket
//--------------------------------------------------------------------
bool
FareDisplayBookingCode::validateFBR_RBDInternational(PaxTypeFare& paxTypeFare,
                                                     std::vector<BookingCode>& bkgCodes,
                                                     bool bkgCodeItem)
{
  LOG4CXX_DEBUG(_logger, "Entering FareDisplayBookingCode::validateFBR_RBDInternational()");
  bool validReturnType = false; // default

  bool dispRec1 = true; // default, display Rec1 RBD codes in the validateSectorPrimeRBDRec1 method

  // Cat 25 data
  int cat19 = 19;
  FBRPaxTypeFareRuleData* fbrData = paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);
  const PaxTypeFareRuleData* fbrPTFare =
      paxTypeFare.paxTypeFareRuleData(RuleConst::FARE_BY_RULE); // Cat 25 pointer
  const CategoryRuleInfo* fbrRuleInfo = nullptr; // Initialize Pointer Rec 2
  fbrRuleInfo = fbrPTFare->categoryRuleInfo(); // Retrieve Rec 2 FBR

  if (fbrData == nullptr)
  {
    return false;
  }

  // Discounted FBR fare pointer
  const PaxTypeFareRuleData* diskPTfare = paxTypeFare.paxTypeFareRuleData(cat19); // Cat 19 pointer

  //  paxTypeFare.getPrimeBookingCode(bkgCodes); // gets Bkgs Codes from Rec 1B
  getPrimeBookingCode(paxTypeFare, bkgCodes); // gets Bkgs Codes from Rec 1B
  setRBBookingCodes(bkgCodes);
  // Get statuses for each segment in the Fare Market
  TravelSegPtrVecI iterTvl = paxTypeFare.fareMarket()->travelSeg().begin();

  AirSeg* airSeg = dynamic_cast<AirSeg*>(*iterTvl);
  setAirSeg(&airSeg);

  // SPR 106773 - setAirSeg get AirSeg for secoundary market, passed by
  // RBData in FareDisplayBookingCodeRB.

  // Carrier code is not set in travel segment, when geting primary
  // booking code for FareDisplay. We use governing Carrier from market

  CarrierCode cxr = airSeg->carrier();
  if (cxr.empty())
    cxr = paxTypeFare.fareMarket()->governingCarrier();

  if (cxr == fbrRuleInfo->carrierCode())
  {
    if (bkgCodeItem) // BookingCodeTableItemNumber exists (logic)
    {
      if (paxTypeFare.isDiscounted())
      {
        if (diskPTfare == nullptr)
        {
          return false;
        }
        else
        {
          const RuleItemInfo* ruleItemInfo = diskPTfare->ruleItemInfo();
          const DiscountInfo* discountInfo = dynamic_cast<const DiscountInfo*>(ruleItemInfo);
          if (discountInfo != nullptr)
          {
            if (discountInfo->bookingCode().empty())
            {
              if (bkgCodes.empty() && // CAT25 Resulting Fare Prime RBD
                  (!fbrData->isSpecifiedFare()))
              {
                const PaxTypeFare* pBaseF = fbrData->baseFare();
                pBaseF->getPrimeBookingCode(bkgCodes);
              }
            }
            else
            {
              bkgCodes.clear();
              bkgCodes.push_back(discountInfo->bookingCode());
            }
          }
        } // No RBD exists in the Discounted 19-22 Fare
      } // CAT19-22 fare
      else // resulting fare is No CAT19-22 fare
      {
        if (bkgCodes.empty())
        {
          if (bkgCodeItem) // BookingCodeTableItemNumber exists (logic)
          {
            if (!fbrData->isSpecifiedFare())
            {
              const PaxTypeFare* pBaseF = fbrData->baseFare();
              pBaseF->getPrimeBookingCode(bkgCodes);
            }
          }
        } //  No PRIME RBD in CAT 25 Resulting Fare
      } //  collect PRIME RBD
    } //  T999 presents
    else //  CAT25 Resulting fare does not have T999
    {
      if (paxTypeFare.isDiscounted())
      {
        if (diskPTfare == nullptr)
        {
          return false;
        }
        else
        {
          const RuleItemInfo* ruleItemInfo = diskPTfare->ruleItemInfo();
          const DiscountInfo* discountInfo = dynamic_cast<const DiscountInfo*>(ruleItemInfo);
          if (discountInfo != nullptr)
          {
            if (!discountInfo->bookingCode().empty())
            {
              bkgCodes.clear();
              bkgCodes.push_back(discountInfo->bookingCode());
            }
          }
        } // No RBD exists in the Discounted 19-22 Fare
      } // CAT19-22 fare
      else // resulting fare is No CAT19-22 fare
      {
        if (bkgCodes.empty())
          return false; // continue with base fare
      }
      if (!(paxTypeFare.vendor() == ATPCO_VENDOR_CODE || paxTypeFare.vendor() == SITA_VENDOR_CODE))
      {
        const FareByRuleItemInfo* fbrItemInfo =
            dynamic_cast<const FareByRuleItemInfo*>(fbrData->ruleItemInfo());
        if (fbrItemInfo != nullptr && fbrItemInfo->primeSector() == 'X')
        {
          const FBRPaxTypeFareRuleData* fbrPTFBaseFare = PTFRuleData::toFBRPaxTypeFare(fbrPTFare);
          if (fbrPTFBaseFare && !fbrPTFBaseFare->isSpecifiedFare())
          {
            validateConvention2(paxTypeFare, paxTypeFare, airSeg, bkgCodes);
          }
        }
      }
    }
    if (validateSectorPrimeRBDRec1(airSeg, bkgCodes, dispRec1))
    {
      setRBBookingCodes(bkgCodes);
      return true;
    }
    else
    {
      return false;
    }
  }
  else // different carrier
  {
    if (!bkgCodeItem) // BookingCodeTableItemNumber does not exist (logic)
    {
      if (!fbrData->isSpecifiedFare())
        return false;
    }
    validReturnType = validateFBRConvention1(paxTypeFare, airSeg, *fbrData, bkgCodes);
  }
  return validReturnType;
}

//------------------------------------------------------------------------
// FareByRule Fare Record 6 Convention 1 validation for the Travel Segment
//------------------------------------------------------------------------
bool
FareDisplayBookingCode::validateFBRConvention1(PaxTypeFare& paxTypeFare,
                                               AirSeg* airSeg,
                                               const FBRPaxTypeFareRuleData& fbrPTFBaseFare,
                                               std::vector<BookingCode>& bkgCodes)
{
  LOG4CXX_DEBUG(_logger, "Entering FareDisplayBookingCode::validateFBRConvention1()");
  bool validReturnType = true;

  if (paxTypeFare.vendor() != "ATP" && paxTypeFare.vendor() != "SITA" // vendor = FMS
      &&
      (&fbrPTFBaseFare)->baseFare() != nullptr)
  {
    validReturnType =
        validateConvention1(paxTypeFare, (fbrPTFBaseFare.baseFare())->vendor(), airSeg, bkgCodes);
  }
  else
  {
    validReturnType = validateConvention1(paxTypeFare, paxTypeFare.vendor(), airSeg, bkgCodes);
  }

  return validReturnType;
}

//-----------------------------------------------------------------
//  Update BookingCode
//-----------------------------------------------------------------
void
FareDisplayBookingCode::updateBookingCode(BookingCode& bookingCode)
{
  if (bookingCode[1] == 'N')
  {
    bookingCode = DOLLARBLANK;
  }
  else
  {
    bookingCode += DOLLAR;
  }
}


//--------------------------------------------------------------------
// International FBR RBD validation = apply PrimeRBD Rec 1, Rec6 Conv1
// validations for the international FareMarket
//--------------------------------------------------------------------
bool
FareDisplayBookingCode::finalValidateFBR_RBDInternational(PaxTypeFare& paxTypeFare,
                                                          std::vector<BookingCode>& bkgCodes)
{
  // Cat 25 data
  const FBRPaxTypeFareRuleData* fbrData = paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);

  return (fbrData && fbrData->getBaseFarePrimeBookingCode(bkgCodes));
}

bool
FareDisplayBookingCode::getPrimeBookingCode(PaxTypeFare& paxTypeFare,
                                            std::vector<BookingCode>& bookingCodeVec)
{

  bookingCodeVec.clear();
  if (paxTypeFare.isFareClassAppSegMissing())
  {
    return false;
  }

  LOG4CXX_DEBUG(_logger,
                "getPrimeBookingCode() - cxrReq: "
                    << paxTypeFare.fareMarket()->governingCarrier() << " " << paxTypeFare.carrier()
                    << " cxrApplTblItemNo: " << paxTypeFare.fcasCarrierApplTblItemNo());
  if (paxTypeFare.fare()->isIndustry() &&
      paxTypeFare.fareMarket()->governingCarrier() != INDUSTRY_CARRIER &&
      paxTypeFare.fcasCarrierApplTblItemNo() == 0)
  {
    LOG4CXX_DEBUG(_logger,
                  "getPrimeBookingCode() - cxrReq: " << paxTypeFare.fareMarket()->governingCarrier()
                                                     << " " << paxTypeFare.carrier()
                                                     << " cxrApplTblItemNo: "
                                                     << paxTypeFare.fcasCarrierApplTblItemNo());
    return false;
  }

  if ((_trx->getRequest()->passengerTypes().size() > 1 ||
       _trx->getRequest()->requestedInclusionCode() == ALL_FARES ||
       !paxTypeFare.fareClassAppSegInfo()->_paxType.empty()) &&
      paxTypeFare.fcasCarrierApplTblItemNo() == 0 && paxTypeFare.fcasiSegments().size() == 1)
  {
    const FareClassAppSegInfo* fcasi = paxTypeFare.fareClassAppSegInfo();
    for (uint16_t i = 0; i < 8; ++i)
    {
      if (!fcasi->_bookingCode[i].empty())
      {
        bookingCodeVec.push_back(fcasi->_bookingCode[i]);
      }
      else
      {
        break;
      }
    }
    return (!bookingCodeVec.empty());
  }
  FareClassAppSegInfoList::const_iterator iter = paxTypeFare.fcasiSegments().begin();
  FareClassAppSegInfoList::const_iterator iterEnd = paxTypeFare.fcasiSegments().end();
  CarrierCode& cxr = paxTypeFare.fareMarket()->governingCarrier();
  for (; iter != iterEnd; ++iter)
  {
    if ((*iter)->_carrierApplTblItemNo != 0)
    {
      const std::vector<CarrierApplicationInfo*>& carrierApplList =
          getCarrierApplication(paxTypeFare.vendor(), (*iter)->_carrierApplTblItemNo);
      if (!carrierApplList.empty() && !findCXR(cxr, carrierApplList))

        continue;
    }
    if (!(*iter)->_paxType.empty() && paxTypeFare.fareClassAppSegInfo() != (*iter))
    {
      continue;
    }

    for (uint16_t i = 0; i < 8; ++i)
    {
      if (!(*iter)->_bookingCode[i].empty())
      {
        bookingCodeVec.push_back((*iter)->_bookingCode[i]);
      }
      else
      {
        break;
      }
    }
  }

  if (bookingCodeVec.empty())
  {
    const FareClassAppSegInfo* fcasi = paxTypeFare.fareClassAppSegInfo();
    for (uint16_t i = 0; i < 8; ++i)
    {
      if (!fcasi->_bookingCode[i].empty())
      {
        bookingCodeVec.push_back(fcasi->_bookingCode[i]);
      }
      else
      {
        break;
      }
    }
  }
  return (!bookingCodeVec.empty());
}

bool
FareDisplayBookingCode::isYYCarrierRequest(std::set<CarrierCode>& rqstedCxrs)
{
  std::set<CarrierCode>::iterator i(rqstedCxrs.begin()), end(rqstedCxrs.end());
  i = std::find(rqstedCxrs.begin(), rqstedCxrs.end(), INDUSTRY_CARRIER);
  return (i != end);
}

bool
FareDisplayBookingCode::isRBReady()
{
  return false;
}

bool
FareDisplayBookingCode::isSecondary()
{
  return false;
}

void
FareDisplayBookingCode::setAirSeg(AirSeg** as)
{
  return;
}

bool
FareDisplayBookingCode::isNotRBReady()
{
  return true;
}

void
FareDisplayBookingCode::setCarrierMatchedTable990()
{
  return;
}

bool
FareDisplayBookingCode::isGetBookingCodesEmpty()
{
  return false;
}

bool
FareDisplayBookingCode::getBookingCodeException(PaxTypeFare& paxTypeFare,
                                                VendorCode vendorA,
                                                AirSeg* airSeg,
                                                std::vector<BookingCode>& bkgCodes,
                                                bool convention)
{
  FareDisplayBookingCodeException fdbce(_trx, &paxTypeFare);
  return fdbce.getBookingCodeException(vendorA, bkgCodes, airSeg, convention);
}

void
FareDisplayBookingCode::setRBBookingCodes(std::vector<BookingCode>& bkgCodes)
{
  return;
}
const std::vector<CarrierApplicationInfo*>&
FareDisplayBookingCode::getCarrierApplication(const VendorCode& vendor, int carrierApplTblItemNo)
    const
{
  return _trx->dataHandle().getCarrierApplication(vendor, carrierApplTblItemNo);
}

} // namespace tse
