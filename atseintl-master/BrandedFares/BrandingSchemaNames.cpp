//-------------------------------------------------------------------
//
//  File:        BrandingSchemaNames.cpp
//  Created:     2013
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
namespace tse
{
namespace brands
{
const char* _BrandingElementNames[] = {
  "GetAirlineBrandsRS",   "BrandingResponse",     "ResponseSource",     "BrandingResults",
  "MarketResponse",       "ProgramIdList",        "ProgramID",          "Carrier",
  "MarketCriteria",       "DepartureAirportCode", "ArrivalAirportCode", "PassengerTypes",
  "Type",                 "Message",              "CarrierBrandsData",  "BrandProgram",
  "Vendor",               "Sequence",             "ProgramCode",        "ProgramName",
  "ProgramDescription",   "PassengerType",        "BrandsData",         "Brand",
  "Code",                 "Name",                 "Tier",               "PrimaryFareIDTable",
  "SecondaryFareIDTable", "AccountCodeList",      "AccountCode",        "GlobalIndicator",
  "Direction",            "EffectiveDate",        "DiscontinueDate",    "OriginLoc",
  "Diagnostics",          "RuleExecution",        "Condition",          "SystemCode",
  "CarrierFlightItemNum", "Text",                 "BookingCodeList",    "SecondaryBookingCodeList",
  "FareBasisCode",        "AncillaryList",        "Ancillary"
};

const char* _BrandingAttributeNames[] = {
  "distributionChannel",  "pseudoCityCode",  "iataNumber",     "clientID",         "requestType",
  "requestingCarrierGDS", "geoLocation",     "departmentCode", "officeDesignator", "marketID",
  "direction",            "globalIndicator", "airlineCode",    "failCode",         "messageCode",
  "messageText",          "programID",       "pattern",        "ruleId",           "dataSource",
  "includeInd",           "Sequence",        "SubCode",        "Service",          "Application"
};
}
}
