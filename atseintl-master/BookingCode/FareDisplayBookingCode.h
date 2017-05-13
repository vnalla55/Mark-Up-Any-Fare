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
#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"

#include <log4cxx/helpers/objectptr.h>

#include <set>
#include <vector>

namespace log4cxx
{
class Logger;
using LoggerPtr = helpers::ObjectPtrT<Logger>;
}

namespace tse
{

class FareDisplayTrx;
class PaxTypeFare;
class PaxTypeFareRuleData;
class FBRPaxTypeFareRuleData;
class AirSeg;
class TravelSeg;
class CarrierApplicationInfo;

//------------------------------------------------------------------
// This is a base class for the Booking Code Display process.
//------------------------------------------------------------------
class FareDisplayBookingCode
{
  friend class FareDisplayBookingCodeTest;

public:
  virtual ~FareDisplayBookingCode() = default;

  static const std::string DOLLARBLANK;
  static const std::string TWO_BLANKS;
  static const std::string DOLLAR;

  /*---------------------------------------------------------------------------
   * Booking Code validation Return Type
   *-------------------------------------------------------------------------*/
  enum ValidateReturnType
  { RET_PASS = 1,
    RET_FAIL,
    RET_MIXED,
    RET_NO_MATCH,
    RET_CONTINUE };

  /*---------------------------------------------------------------------------
   * Return Types for the Travel Segment after Booking code validated
   *-------------------------------------------------------------------------*/
  enum StatusReturnType
  { PASS = 1,
    FAIL_STATUS_RETURN_TYPE,
    MIXED,
    SOME_PASS_SOME_FAIL_NO_MATCH_NOT_PROCESSED,
    NONE_PASS_SOME_FAIL_NO_MATCH_NOT_PROCESSED,
    SOME_PASS_NONE_FAIL_NO_MATCH_NOT_PROCESSED,
    NO_MATCH_NOT_PROCESSED,
    NEED_REVAL };

  /*---------------------------------------------------------------------------
   * Return Types for the Travel Segment after Booking code validated
   *-------------------------------------------------------------------------*/
  enum setStatus
  { INIT = '0',
    REC1 = '1',
    CONV_2 = '2' };

  enum StatusType
  { STATUS_RULE1 = 1,
    STATUS_RULE2,
    STATUS_FLOW_FOR_LOCAL_JRNY_CXR };
  enum FareType
  { PUBLISHED_FARE = 1,
    FARE_BY_RULE_FARE };

  void getBookingCode(FareDisplayTrx& trx, PaxTypeFare& ptf, BookingCode& bookingCode);

  // YY Fares, find carrier item in Carrier Application Table (Table990)
  // for positive, negative ot any ($$) status.
  bool findCXR(const CarrierCode& cxr, const std::vector<CarrierApplicationInfo*>& carrierApplList);

protected:
  virtual bool isRBReady();
  virtual bool isNotRBReady();
  virtual bool isSecondary();
  virtual bool isGetBookingCodesEmpty();
  virtual bool getBookingCodeException(PaxTypeFare& paxTypeFare,
                                       VendorCode vendorA,
                                       AirSeg* airSeg,
                                       std::vector<BookingCode>& bkgCodes,
                                       bool convention);
  virtual void setAirSeg(AirSeg** as);
  virtual void setCarrierMatchedTable990();
  virtual void setRBBookingCodes(std::vector<BookingCode>& bkgCodes);

  virtual bool validateT999(PaxTypeFare& paxTfare, std::vector<BookingCode>& bkgCodes);

  bool validateIndustryFareBkgCode(PaxTypeFare& ptf, std::vector<BookingCode>& bkgCodes);

  //  Record6 Convention1, calls LocalMarket
  bool validateRecord6Conv1(PaxTypeFare& paxTfare,
                            AirSeg* airSeg,
                            std::vector<BookingCode>& bkgCodes,
                            TravelSeg* seg);

  bool setPaxTypeFareBkgCodeStatus(PaxTypeFare& paxTypeFare);

  bool validateConvention1(PaxTypeFare& paxTypeFare,
                           VendorCode vendorA,
                           AirSeg* airSeg,
                           std::vector<BookingCode>& bkgCodes);

  bool
  validateSectorPrimeRBDRec1(AirSeg* airSeg, std::vector<BookingCode>& bkgCodes, bool dispRec1);

  bool validateLocalMarket(PaxTypeFare& paxTfare, AirSeg& airSeg, TravelSeg* seg);

  // The RBD validation process for all Fares, except Industry.
  bool validateFareBkgCode(PaxTypeFare& paxTypeFare, std::vector<BookingCode>& bkgCodes);

  // Table 999 item number exists in the FareClassAppSegInfo (Record 1B)
  // validate it
  bool validateBookingCodeTblItemNo(PaxTypeFare& paxTypeFare,
                                    std::vector<BookingCode>& bkgCodes,
                                    bool fbrActive);

  bool validateDomestic(PaxTypeFare& paxTypeFare, std::vector<BookingCode>& bkgCodes);

  // Validate FBR Fare
  bool
  validateFBRFare(PaxTypeFare& paxTypeFare, std::vector<BookingCode>& bkgCodes, bool indicator);

  // Validate FBR Fare
  bool validateFBR_RBDInternational(PaxTypeFare& paxTypeFare,
                                    std::vector<BookingCode>& bkgCodes,
                                    bool indicator);

  // Retrieve the Convention object and invoke the Record6 Convention 2
  //  method for the FBR fare.
  bool validateConvention2(PaxTypeFare& paxTypeFare,
                           PaxTypeFare& paxTypeBaseFare,
                           AirSeg* airSeg,
                           std::vector<BookingCode>& bkgCodes);

  bool validateRBDInternational(PaxTypeFare& paxTypeFare, std::vector<BookingCode>& bkgCodes);

  bool validatePrimeRBDRecord1Domestic(PaxTypeFare& paxTfare, std::vector<BookingCode>& bkgCodes);

  // Prime RBD does not exist in the Cat25 Fare Resulting Fare
  // Process if it's Discounted FBR
  ValidateReturnType validateRBDforDiscountFBRFare(PaxTypeFare& paxTfare,
                                                   std::vector<BookingCode>& bookingCodeVec,
                                                   bool& rec1);

  bool validateFBRConvention1(PaxTypeFare& paxTypeFare,
                              AirSeg* airSeg,
                              const FBRPaxTypeFareRuleData& fbrPTFBaseFare,
                              std::vector<BookingCode>& bkgCodes);

  bool
  finalValidateFBR_RBDInternational(PaxTypeFare& paxTypeFare, std::vector<BookingCode>& bkgCodes);

  void updateBookingCode(BookingCode& bookingCode);

  bool isYYCarrierRequest(std::set<CarrierCode>& rqstedCxrs);

  bool getPrimeBookingCode(PaxTypeFare& paxTypeFare, std::vector<BookingCode>& bkgCodes);

  virtual const std::vector<CarrierApplicationInfo*>&
  getCarrierApplication(const VendorCode& vendor, int carrierApplTblItemNo) const;

  // Attributes:
  FareDisplayTrx* _trx = nullptr;
  static log4cxx::LoggerPtr _logger;
};

using CarrierApplicationInfoListPtr = std::vector<CarrierApplicationInfo*>;
using CarrierApplicationInfoListPtrI = std::vector<CarrierApplicationInfo*>::iterator;
using CarrierApplicationInfoListPtrIC = std::vector<CarrierApplicationInfo*>::const_iterator;

using TravelSegPtrVecIC = std::vector<tse::TravelSeg*>::const_iterator;

} // end tse namespace
