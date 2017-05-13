//-------------------------------------------------------------------
//
//  File:        BrandingSchemaNames.h
//  Created:     April 2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
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
namespace tse
{
namespace brands
{
enum _ElementNameIdx_
{ _GetAirlineBrandsRS,
  _BrandingResponse,
  _ResponseSource,
  _BrandingResults,
  _MarketResponse,
  _ProgramIdList,
  _ProgramID,
  _Carrier,
  _MarketCriteria,
  _DepartureAirportCode,
  _ArrivalAirportCode,
  _PassengerTypes,
  _Type,
  _Message,
  _CarrierBrandsData,
  _BrandProgram,
  _Vendor,
  _Sequence,
  _ProgramCode,
  _ProgramName,
  _ProgramDescription,
  _PassengerType,
  _BrandsData,
  _Brand,
  _Code,
  _Name,
  _Tier,
  _PrimaryFareIDTable,
  _SecondaryFareIDTable,
  _AccountCodeList,
  _AccountCode,
  _GlobalIndicator,
  _Direction,
  _EffectiveDate,
  _DiscontinueDate,
  _OriginLoc,
  _Diagnostics,
  _RuleExecution,
  _Condition,
  _SystemCode,
  _CarrierFlightItemNum,
  _Text,
  _BookingCodeList,
  _SecondaryBookingCodeList,
  _FareBasisCode,
  _AncillaryList,
  _Ancillary,
  _NumberOfElementNames_ };

enum _AttributeNameIdx_
{ _distributionChannel,
  _pseudoCityCode,
  _iataNumber,
  _clientID,
  _requestType,
  _requestingCarrierGDS,
  _geoLocation,
  _departmentCode,
  _officeDesignator,
  _marketID,
  _direction,
  _globalIndicator,
  _airlineCode,
  _failCode,
  _messageCode,
  _messageText,
  _programID,
  _pattern,
  _ruleId,
  _dataSource,
  _includeInd,
  _sequenceNumber,
  _subCode,
  _service,
  _application,
  _NumberOfAttributeNames_ };

extern const char* _BrandingElementNames[];
extern const char* _BrandingAttributeNames[];
}
}
