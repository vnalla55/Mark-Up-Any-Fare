//----------------------------------------------------------------------------
//  File:           PfcDisplayRequest.h
//  Description:    PfcDisplayRequest header file for ATSE International Project
//  Created:        04/01/2008
//  Authors:        Piotr Lach
//
//
//  Updates:
//
//  Copyright Sabre 2008
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

#include "Common/DateTime.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PricingRequest.h"

#include <string>
#include <tuple>
#include <vector>

namespace tse
{

class PfcDisplayRequest : public PricingRequest
{
public:
  typedef std::vector<LocCode> Markets;

  enum PosInSegment
  {
    SEGMENT_NUMBER = 0,
    DEPARTURE_AIRPORT = 1,
    ARIVAL_AIRPORT = 2,
    CARRIER_CODE = 3,
    DEPARTURE_DATE = 4,
    FLIGHT_NUMBER = 5
  };

  typedef std::tuple<uint32_t, // Segment Number
                     LocCode, // Departure Airport
                     LocCode, // Arival Airport
                     CarrierCode, // Carrier Code
                     DateTime, // Departure Date
                     FlightNumber> // Flight Number
      Segment;
  typedef std::vector<Segment> Segments;
  typedef float Percentage;

  enum CmdType
  {
    ERROR = 0, // Wrong entry
    HELP = 1, // PFC Help entry type
    PXC = 2, // Passenger Facility Charge (PFC) entry type
    PXE = 3, // Essential Air Service Record (EAS) entry type
    PXM = 4, // Co - terminal Record (CTR) entry type
    PXT = 5, // Collection Method Record (CMR) entry type
    PXQ = 6, // Equipment Type Exemption Record (ETER) entry type
    PXA = 7, // Carrier Absorption (CAP/XCAP) entry type
    PXH = 8 // Historical PFC entry type
  };

  PfcDisplayRequest();

  DateTime& overrideDate() { return _overrideDate; }
  const DateTime& overrideDate() const { return _overrideDate; }

  CmdType& cmdType() { return _cmdType; }
  const CmdType& cmdType() const { return _cmdType; }

  Markets& markets() { return _markets; }
  const Markets& markets() const { return _markets; }

  CarrierCode& carrier1() { return _carrier1; }
  const CarrierCode& carrier1() const { return _carrier1; }

  CarrierCode& carrier2() { return _carrier2; }
  const CarrierCode& carrier2() const { return _carrier2; }

  Percent& percentageRate() { return _pecentageRate; }
  const Percent& percentageRate() const { return _pecentageRate; }

  uint32_t& absorptionRecordNumber() { return _absorptionRecord; }
  const uint32_t& absorptionRecordNumber() const { return _absorptionRecord; }

  Segments& segments() { return _segments; }
  const Segments& segments() const { return _segments; }

  bool& isPNR() { return _isPNR; }
  const bool isPNR() const { return _isPNR; }

private:
  DateTime _overrideDate;
  CmdType _cmdType;
  Markets _markets;
  CarrierCode _carrier1;
  CarrierCode _carrier2;
  Percent _pecentageRate;
  uint32_t _absorptionRecord;
  Segments _segments;
  bool _isPNR;

  PfcDisplayRequest(const PfcDisplayRequest&);
  PfcDisplayRequest& operator=(const PfcDisplayRequest&);
};

inline PfcDisplayRequest::PfcDisplayRequest()
  : _overrideDate(DateTime::emptyDate()),
    _cmdType(ERROR),
    _pecentageRate(0.0),
    _absorptionRecord(0),
    _isPNR(false)
{
}

} // namespace tse
