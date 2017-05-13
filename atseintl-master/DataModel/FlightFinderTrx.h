//-------------------------------------------------------------------
//
//  File:        FlightFinderTrx.h
//  Created:     October 23, 2007
//  Design:      Kitima Chunpleng
//  Authors:
//
//  Description: Flight Finder Transaction object
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
//-------------------------------------------------------------------

#pragma once

#include "Common/ErrorResponseException.h"
#include "Common/SmallBitSet.h"
#include "DataModel/ShoppingTrx.h"

#include <map>
#include <string>

namespace tse
{

class FlightFinderTrx : public ShoppingTrx
{
public:
  enum LegsFaresValidationStatus
  {
    LEG1_HAS_VLD_FARE = 0x01,
    LEG2_HAS_VLD_FARE = 0x02,
    ALL_LEGS_HAS_VLD_FARE = 0x04,
  };
  enum BffStep
  {
    STEP_NONE = 0,
    STEP_1,
    STEP_2,
    STEP_3,
    STEP_4,
    STEP_5,
    STEP_6
  };
  typedef struct
  {
    std::string fareBasiscode;
    MoneyAmount fareAmount;
    CurrencyCode currencyCode;
  } FareBasisCodeInfo;

  typedef struct
  {
    BookingCode bkgCode;
    uint16_t numSeats;
  } BookingCodeData;

  typedef struct
  {
    //<vect per PTF <vect per Segment> >
    std::vector<std::vector<BookingCodeData> > bkgCodeDataVect;
    std::vector<PaxTypeFare*> paxTypeFareVect;
    uint32_t sopIndex;
  } SopInfo;

  struct FlightDataInfo
  {
    FlightDataInfo() : onlyApplicabilityFound(false) {};

    std::vector<SopInfo*> flightList;
    std::vector<PaxTypeFare*> altDatesPaxTypeFareVect;
    bool onlyApplicabilityFound;
  };

  typedef struct
  {
    uint8_t flightBitStatus;
    std::vector<PaxTypeFare*> paxTypeFareVect;
    std::vector<PaxTypeFare*> inboundPaxTypeFareVect;
  } FlightBitInfo;

  typedef std::map<DateTime, FlightDataInfo*> InboundDateFlightMap;

  typedef struct
  {
    FlightDataInfo flightInfo;
    InboundDateFlightMap iBDateFlightMap;
  } OutBoundDateInfo;

  typedef std::map<DateTime, OutBoundDateInfo*> OutBoundDateFlightMap;
  typedef OutBoundDateFlightMap::iterator OutBoundDateFlightMapI;
  typedef OutBoundDateFlightMap::const_iterator OutBoundDateFlightMapIC;
  typedef SmallBitSet<uint8_t, LegsFaresValidationStatus> LegsStatus;

  typedef struct ErrorInfo
  {
    ErrorInfo() : errorCode(ErrorResponseException::NO_ERROR), message("") {}

    ErrorResponseException::ErrorResponseCode errorCode;
    std::string message;

    operator bool() { return !message.empty(); }

  } ErrorInfo;

  virtual bool process(Service& srv) override { return srv.process(*this); }

  OutBoundDateFlightMap& outboundDateflightMap() { return (_outboundDateflightMap); }

  std::vector<FareBasisCodeInfo>& fareBasisCodesFF() { return (_fareBasisCodesFF); }

  const std::vector<FareBasisCodeInfo>& fareBasisCodesFF() const { return (_fareBasisCodesFF); }

  LegsStatus& legsStatus() { return _legsStatus; }
  const LegsStatus& legsStatus() const { return _legsStatus; }

  bool& ignoreDiagReq() { return _ignoreDiagReq; }
  bool ignoreDiagReq() const { return _ignoreDiagReq; }

  ErrorInfo& reportError() { return _reportError; }
  const ErrorInfo& reportError() const { return _reportError; }

  std::vector<PaxTypeCode>& reqPaxType() { return _paxType; }
  const std::vector<PaxTypeCode>& reqPaxType() const { return _paxType; }
  bool isPaxTypeReq(const PaxTypeCode& psgType) const
  {
    return find(_paxType.begin(), _paxType.end(), psgType) != _paxType.end();
  }

  // BFF function
  std::string& owrt() { return _owrt; }
  const std::string& owrt() const { return _owrt; }

  BffStep& bffStep() { return _step; }
  BffStep bffStep() const { return _step; }

  size_t& numDaysFwd() { return _numDaysFwd; }
  size_t numDaysFwd() const { return _numDaysFwd; }

  DateTime& departureDT() { return _departureDT; }
  const DateTime& departureDT() const { return _departureDT; }

  bool& avlInS1S3Request() { return _avlInS1S3Request; }
  const bool& avlInS1S3Request() const { return _avlInS1S3Request; }

  bool isOW() const { return _owrt == "O"; }
  bool isRT() const { return _owrt == "R"; }
  bool isBffReq() const { return !owrt().empty() && bffStep() != STEP_NONE; }
  bool isFFReq() const { return !isBffReq(); }

protected:
  OutBoundDateFlightMap _outboundDateflightMap;
  std::vector<FareBasisCodeInfo> _fareBasisCodesFF;

  // BFF additional data
  std::string _owrt;
  BffStep _step = BffStep::STEP_NONE;
  size_t _numDaysFwd = 0;
  DateTime _departureDT;
  bool _avlInS1S3Request = false;

private:
  LegsStatus _legsStatus;

  // Set when client want to see diagnostic for altdate request
  bool _ignoreDiagReq = false;
  std::vector<PaxTypeCode> _paxType;
  ErrorInfo _reportError;
};
} // tse namespace
