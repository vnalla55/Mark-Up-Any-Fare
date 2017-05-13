//----------------------------------------------------------------------------
//
//  File:        PaxTypeUtil.h
//  Created:     4/12/2004
//  Authors:     JY
//
//  Description: Common functions required for ATSE shopping/pricing.
//
//  Updates:
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
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PosPaxType.h"

#include <boost/regex.hpp>
#include <set>
#include <vector>

namespace tse
{

class PaxType;
class PaxTypeFare;
class DataHandle;
class PricingTrx;
class ShoppingTrx;
class FareDisplayTrx;
class MaxPenaltyInfo;

enum PaxTypeStatus
{ PAX_TYPE_STATUS_UNKNOWN = 0,
  PAX_TYPE_STATUS_ADULT,
  PAX_TYPE_STATUS_CHILD,
  PAX_TYPE_STATUS_INFANT };

class PaxTypeUtil
{
public:
  static bool initialize(PricingTrx& trx,
                         PaxType& requestedPaxType,
                         PaxTypeCode& paxTypeCode,
                         uint16_t number,
                         uint16_t age,
                         StateCode& stateCode,
                         uint16_t inputOrder);

  static void initialize(FareDisplayTrx& trx);

  static bool addPaxTypeToMap(PricingTrx& trx,
                              std::map<CarrierCode, std::vector<PaxType*>*>& paxTypeMap,
                              const PaxTypeCode& paxTypeCode,
                              const uint16_t number,
                              const uint16_t age,
                              const StateCode& stateCode,
                              const VendorCode& vendor,
                              const CarrierCode& carrier,
                              MaxPenaltyInfo* maxPenaltyInfo);

  static const PaxType* isAnActualPaxInTrx(PricingTrx& trx,
                                           const CarrierCode& carrier,
                                           const PaxTypeCode& paxTypeCode,
                                           const uint16_t minAge,
                                           const uint16_t maxAge);

  static const PaxType* isAnActualPaxInTrxImpl(const PricingTrx& trx,
                                               const CarrierCode& carrier,
                                               const PaxTypeCode& paxTypeCode);

  static const PaxType*
  isAnActualPaxInTrx(PricingTrx& trx, const CarrierCode& carrier, const PaxTypeCode& paxTypeCode);

  static bool isPaxInTrx(const PricingTrx& trx, const std::vector<PaxTypeCode>& paxTypeCodes);

  static bool isAnActualPax(const PaxType& paxType,
                            const CarrierCode& carrier,
                            const PaxTypeCode& paxTypeCode,
                            const uint16_t minAge = 0,
                            const uint16_t maxAge = 0);

  static const PaxType* isAdultInTrx(PricingTrx& trx);

  static std::vector<PaxTypeCode> retrievePaxTypes(const PricingTrx& trx);

  static bool isAdult(const PaxTypeCode& paxTypeCode, const VendorCode& vendor);
  static bool isAdult(PricingTrx& trx, const PaxTypeCode& paxTypeCode, const VendorCode& vendor);
  static bool isAdult(DataHandle& dh, const PaxTypeCode& paxTypeCode, const VendorCode& vendor);

  static bool isChild(const PaxTypeCode& paxTypeCode, const VendorCode& vendor);
  static bool isChildFD(const PaxTypeCode& paxTypeCode, const VendorCode& vendor);
  static bool isChild(PricingTrx& trx, const PaxTypeCode& paxTypeCode, const VendorCode& vendor);
  static bool isChild(DataHandle& dh, const PaxTypeCode& paxTypeCode, const VendorCode& vendor);
  static bool isChildFD(DataHandle& dh, const PaxTypeCode& paxTypeCode, const VendorCode& vendor);

  static bool isInfant(const PaxTypeCode& paxTypeCode, const VendorCode& vendor);
  static bool isInfantFD(const PaxTypeCode& paxTypeCode, const VendorCode& vendor);
  static bool isInfant(PricingTrx& trx, const PaxTypeCode& paxTypeCode, const VendorCode& vendor);
  static bool isInfant(DataHandle& dh, const PaxTypeCode& paxTypeCode, const VendorCode& vendor);
  static bool isInfantFD(DataHandle& dh, const PaxTypeCode& paxTypeCode, const VendorCode& vendor);

  static PaxTypeStatus paxTypeStatus(const PaxTypeCode& paxTypeCode, const VendorCode& vendor);
  static PaxTypeStatus nextPaxTypeStatus(PaxTypeStatus paxTypeStatus);

  static bool findMatch(std::vector<PaxType*> const& paxTypeVec, const PaxTypeCode& paxTypeCode);

  static uint16_t totalNumSeats(const PricingTrx& trx);

  static uint16_t numSeatsForFare(PricingTrx& trx, const PaxTypeFare& paxTypeFare);
  static uint16_t numSeatsForFareWithoutIgnoreAvail(PricingTrx& trx,
                                                    const PaxTypeFare& paxTypeFare);
  static uint16_t numSeatsForExcludedPaxTypes(const ShoppingTrx& trx);

  static bool
  isATPCoPaxRollupMatch(PricingTrx& trx, const PaxTypeCode& paxChk, const PaxTypeCode& paxRef);

  // Add Actual Negotiated Pax types for Equinox
  //
  static void createLowFareSearchNegPaxTypes(PricingTrx& trx, PaxType& requestedPaxType);

  static void createFareTypePricingPaxTypes(PricingTrx& trx, const std::set<PaxTypeCode>& psgTypes);

  static void getVendorCode(DataHandle& dataHandle, PaxType& requestedPaxType);

  static bool sabreVendorPaxType(PricingTrx& trx,
                                 const PaxType& requestedPaxType,
                                 const PaxTypeFare& paxTypeFare);

  static bool isAdultOrAssociatedType(const PaxTypeCode& paxType);

  static bool isNegotiatedOrAssociatedType(const PaxTypeCode& paxType);

  static bool isAdultOrNegotiatedOrAssociatedType(const PaxTypeCode& paxType);

  static bool getActualPaxInTrx(const PricingTrx& trx,
                                const CarrierCode& carrier,
                                const PaxTypeCode& paxTypeCode,
                                std::vector<PaxType*>& matchedPaxVec);

  static bool definedPaxAge(const PricingTrx& trx);

  static bool hasNotAwardPaxType(const std::vector<PaxType*>& paxTypeVec);

  static bool isSpanishPaxType(const PaxTypeCode& paxType);
  static bool isOnlyOnePassenger(const PricingTrx& trx);


  static bool isPaxWithAge(const PaxTypeCode& paxType);
  static bool isPaxWithSpecifiedAge(const PaxTypeCode& paxType);
  static PaxTypeCode getPaxWithUnspecifiedAge(char firstLetter);
  static bool extractAgeFromPaxType(PaxTypeCode& ptc, uint16_t& age);

  static uint16_t getDifferenceInYears(const DateTime& from, const DateTime& to);

  static void parsePassengerWithAge(PaxType& paxType);

  static const boost::regex PAX_WITH_SPECIFIED_AGE;
  static const boost::regex PAX_WITH_UNSPECIFIED_AGE;

private:
  static VendorCode getVendorCode(DataHandle& dataHandle, const PaxTypeCode& paxTypeCode);
  static bool containsAge(const PaxTypeCode& ptc);
  static PaxTypeCode removeAge(const PaxTypeCode& ptc);
};

} // end tse namespace

