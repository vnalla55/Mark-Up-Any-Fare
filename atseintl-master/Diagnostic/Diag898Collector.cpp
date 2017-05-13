//----------------------------------------------------------------------------
//  File:        Diag898Collector.cpp
//  Authors:
//  Created:
//
//  Description: Diagnostic 898- MMGR S8 Branded Fares - programs, fares, services
//  Updates:
//
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
//----------------------------------------------------------------------------
#include "Diagnostic/Diag898Collector.h"

#include "BrandedFares/BSDiagnostics.h"
#include "BrandedFares/MarketCriteria.h"
#include "BrandedFares/MarketResponse.h"
#include "BrandedFares/MarketRule.h"
#include "BrandedFares/RuleExecution.h"
#include "BrandedFares/S8BrandingResponseParser.h"
#include "BrandedFares/S8BrandingSecurity.h"
#include "DataModel/PricingTrx.h"


#include <iomanip>
#include <iostream>

namespace tse
{

void
Diag898Collector::printDiagnostic(
    const std::map<int, std::vector<FareMarket*> >& marketIDFareMarketMap)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << "*************** BRANDED FARES 898 ANALYSIS ***************\n";

  if (!_trx)
    return printDataNotFound();

  typedef PricingTrx::BrandedMarketMap::value_type BrandedMarketMapValueType;
  const PricingTrx* trx = dynamic_cast<const PricingTrx*>(_trx);

  if (!trx)
    return printDataNotFound();

  if (trx->brandedMarketMap().empty())
    return printDataNotFound();

  bool hasData = false;

  for (const BrandedMarketMapValueType& brandedMarketMapValue : trx->brandedMarketMap())
  {
    for (const MarketResponse* marketResponse : brandedMarketMapValue.second)
    {

      if (!matchFareMarket(marketResponse))
        continue;

      processMarketResponse(marketResponse, marketIDFareMarketMap, hasData);
    }
  }

  if (!hasData)
    printDataNotFound();
}

void
Diag898Collector::processMarketResponse(
    const MarketResponse* marketResponse,
    const std::map<int, std::vector<FareMarket*> >& marketIDFareMarketMap,
    bool& hasData)
{
  if (!marketResponse->bsDiagnostics())
    return;

  printS8FareMarket(marketResponse);

  if (isDdInfo())
    printS8DetailHeader();
  else
    printS8CommonHeader();

  if (marketResponse->bsDiagnostics())
  {
    for (const RuleExecution* ruleExecution : marketResponse->bsDiagnostics()->ruleExecution())
    {

      if (!shouldDisplay(ruleExecution->governingCarrier()))
        continue;

      if (!shouldDisplay(ruleExecution->programCode()))
        continue;

      if (!shouldDisplay(ruleExecution->status()))
        continue;

      if (!hasData)
        hasData = true;

      if (isDdInfo())
      {
        printDetailRuleExecutionContent(ruleExecution, marketResponse, marketIDFareMarketMap);
        printSeparator();
      }
      else
      {
        printRuleExecutionContent(ruleExecution, marketResponse, marketIDFareMarketMap);
      }
    }
  }
}

void
Diag898Collector::printS8FareMarket(const MarketResponse* marketResponse)
{
  if (!marketResponse->marketCriteria())
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << "------------ FARE MARKET : ";
  dc << marketResponse->marketCriteria()->departureAirportCode() << " - "
     << marketResponse->marketCriteria()->arrivalAirportCode();
  dc << "   CXR - " << marketResponse->carrier();
  dc << " ----------\n";
}

void
Diag898Collector::printS8CommonHeader()
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc << "V CXR   PROGRAMID      PROGRAM     PAX  STATUS\n";
}

void
Diag898Collector::printS8DetailHeader()
{
  DiagCollector& dc = (DiagCollector&)*this;
  dc << "--------------- BRANDED FARES 898 DETAILED INFO ----------\n";
}

void
Diag898Collector::printSeparator()
{
  DiagCollector& dc = (DiagCollector&)*this;

  dc << "-----------------------------------------------------------\n";
}

void
Diag898Collector::printRuleExecutionContent(
    const RuleExecution* ruleExecution,
    const MarketResponse* marketResponse,
    const std::map<int, std::vector<FareMarket*> >& marketIDFareMarketMap)
{

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  VendorCode vendor = ruleExecution->vendorCode();

  displayVendor(vendor);

  std::string tmp = ruleExecution->programCode();
  boost::to_upper(tmp);

  if (!ruleExecution->governingCarrier().empty())
    dc << std::setw(3) << ruleExecution->governingCarrier().front()->_patternValue;
  else
    dc << std::setw(3) << "   ";

  dc << "  ";

  dc << std::setw(15) << ruleExecution->ruleID() << "  " << std::setw(10) << tmp;

  if (!ruleExecution->passengerType().empty())
    dc << std::setw(3) << ruleExecution->passengerType().front()->_patternValue;
  else
    dc << std::setw(3) << "   ";

  std::string statusStr;
  getStatusStr(ruleExecution->status(), statusStr);
  setStatusStr(ruleExecution, marketResponse, statusStr);
  dc << "  " << std::setw(18) << statusStr << "\n";
}

void
Diag898Collector::printDetailRuleExecutionContent(
    const RuleExecution* ruleExecution,
    const MarketResponse* marketResponse,
    const std::map<int, std::vector<FareMarket*> >& marketIDFareMarketMap)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);

  std::string tmp = ruleExecution->programCode();
  boost::to_upper(tmp);

  dc << " PROGRAM  : " << std::setw(10) << tmp << " CARRIER : ";
  for (const PatternInfo* patternInfo : ruleExecution->governingCarrier())
    dc << std::setw(3) << patternInfo->_patternValue;

  dc << "\n";

  dc << " PROGRAMID: " << std::setw(15) << ruleExecution->ruleID() << "\n";

  dc << " EFF DATE : " << std::setw(10)
     << ruleExecution->salesDateStart().dateToString(YYYYMMDD, "-") << "   ";
  dc << " TRVL DATE START : " << std::setw(10)
     << ruleExecution->travelDateStart().dateToString(YYYYMMDD, "-") << "\n";

  dc << " DISC DATE: " << std::setw(10) << ruleExecution->salesDateEnd().dateToString(YYYYMMDD, "-")
     << "   ";
  dc << " TRVL DATE STOP  : " << std::setw(10)
     << ruleExecution->travelDateEnd().dateToString(YYYYMMDD, "-") << "\n";

  dc << " PAX TYPE : ";
  for (const PatternInfo* patternInfo : ruleExecution->passengerType())
    dc << std::setw(3) << patternInfo->_patternValue;

  dc << "\n";

  bool firstMarket = true;

  dc << " MARKET   : ";
  for (const MarketRuleInfo* marketRuleInfo : ruleExecution->marketRules())
    printMarketRule(marketRuleInfo->_marketRule, firstMarket);

  dc << "\n";

  dc << " ACC CODE : ";
  for (const PatternInfo* patternInfo : ruleExecution->accountCodes())
    dc << std::setw(21) << patternInfo->_patternValue;

  dc << "\n";

  dc << " PCC      : ";
  for (const PatternInfo* patternInfo : ruleExecution->pseudoCityCodes())
    dc << std::setw(5) << patternInfo->_patternValue;

  dc << "\n";

  dc << "**SECURITY: **"
     << "\n";
  if (ruleExecution->s8BrandingSecurity())
    printS8BrandingSecurity(ruleExecution->s8BrandingSecurity());
  dc << "\n**\n";

  std::string statusStr;
  getStatusStr(ruleExecution->status(), statusStr);
  setStatusStr(ruleExecution, marketResponse, statusStr);
  dc << " STATUS   : ";
  dc << std::setw(18) << statusStr << "\n";
}

void
Diag898Collector::printS8BrandingSecurity(const S8BrandingSecurity* s8BrandingSecurity)
{

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  std::string statusStr;
  for (const SecurityInfo* securityInfo : s8BrandingSecurity->securityInfoVec())
  {
    getStatusStr(securityInfo->_status, statusStr);

    dc << "  " << std::setw(11) << securityInfo->_securityName << " : ";
    if (securityInfo->_securityName == "VIEW BOOK")
      dc << std::setw(20) << (securityInfo->_securityValue == "true" ? "YES" : "NO") << " ";
    else
    {
      std::string tmp = securityInfo->_securityValue;
      boost::to_upper(tmp);
      dc << std::setw(20) << tmp << " ";
    }

    dc << std::setw(18) << statusStr << "\n";
  }
}

void
Diag898Collector::printMarketRule(const MarketRule* marketRule, bool& firstMarket)
{
  DiagCollector& dc = (DiagCollector&)*this;

  if (firstMarket)
  {
    firstMarket = false;
    dc << " ORGN LOC: " << std::setw(8) << marketRule->originLoc();
  }
  else
  {
    printBlankLine();
    dc << "             ORGN LOC: " << std::setw(8) << marketRule->originLoc();
  }

  dc << "  LOC TYPE : " << std::setw(2) << marketRule->originLocType() << "\n";
  dc << "             DEST LOC: " << std::setw(8) << marketRule->destinationLoc() << " "
     << " LOC TYPE : " << std::setw(2) << marketRule->destinationLocType() << "\n";

  dc << "             DIRECTION: " << std::setw(8);

  if (marketRule->direction() == "OT")
    dc << "OUTBOUND";
  else if (marketRule->direction() == "IN")
    dc << "INBOUND";
  else if (marketRule->direction() == "OI")
    dc << "        ";
  else
    dc << marketRule->direction();

  dc << " GLOBAL DIR : ";
  const std::string* prBrGlobalDir = globalDirectionToStr(marketRule->globalDirection());
  if (prBrGlobalDir != nullptr)
    dc << std::setw(3) << *prBrGlobalDir;
  dc << "\n";
}

void
Diag898Collector::printBlankLine() const
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;

  dc << "                                                           \n";
}

void
Diag898Collector::displayVendor(const VendorCode& vendor, bool isDetailDisp)
{
  DiagCollector& dc = (DiagCollector&)*this;
  if ("ATP" == vendor)
  {
    if (isDetailDisp)
      dc << " A";
    else
      dc << "A  ";
  }
  else if ("MMGR" == vendor)
  {
    if (isDetailDisp)
      dc << " M";
    else
      dc << "M  ";
  }
  else if ("USOC" == vendor)
  {
    if (isDetailDisp)
      dc << " C";
    else
      dc << "C  ";
  }
  else if (isDetailDisp)
    dc << "  ";
  else
    dc << "   ";
}

void
Diag898Collector::getStatusStr(StatusS8 status, std::string& statusStr) const
{
  switch (status)
  {
  case PASS_S8:
    statusStr = "PASS";
    break;
  case FAIL_S8_PASSENGER_TYPE:
    statusStr = "FAIL PSGR TYPE";
    break;
  case FAIL_S8_ACCOUNT_CODE:
    statusStr = "FAIL ACC CODE";
    break;
  case FAIL_S8_MARKET:
    statusStr = "FAIL MARKET";
    break;
  case FAIL_S8_PCC:
    statusStr = "FAIL PCC";
    break;
  case FAIL_S8_CARRIER:
    statusStr = "FAIL CARRIER";
    break;
  case FAIL_S8_SALES_DATE:
    statusStr = "FAIL SALES DATE";
    break;
  case FAIL_S8_TRAVEL_DATE:
    statusStr = "FAIL TRAVEL DATE";
    break;
  case FAIL_S8_IATA_NUMBER:
    statusStr = "FAIL IATA NUM";
    break;
  case FAIL_S8_CARRIER_GDS:
    statusStr = "FAIL CARRIER GDS";
    break;
  case FAIL_S8_AGENT_LOCATION:
    statusStr = "FAIL AGENT LOC";
    break;
  case FAIL_S8_DEPARTMENT_CODE:
    statusStr = "FAIL DEPT CODE";
    break;
  case FAIL_S8_OFFICE_DESIGNATOR:
    statusStr = "FAIL OFFICE DESIG";
    break;
  case FAIL_S8_SECURITY:
    statusStr = "FAIL SECURITY";
    break;
  case FAIL_S8_VIEW_BOOK_TICKET:
    statusStr = "FAIL VIEW BOOK";
    break;
  }
}

bool
Diag898Collector::isDdInfo() const
{
  if (!_trx)
    return false;
  return _trx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO";
}

bool
Diag898Collector::shouldDisplay(const std::vector<PatternInfo*>& cxrVec) const
{
  if (_trx)
  {
    std::string filter = _trx->diagnostic().diagParamMapItem(Diagnostic::DIAG_CARRIER);

    if (!filter.empty())
    {
      for (const PatternInfo* patternInfo : cxrVec)
      {
        if (patternInfo->_patternValue == filter)
          return true;
      }

      return false;
    }
  }
  return true;
}

bool
Diag898Collector::shouldDisplay(const ProgramCode& programCode) const
{
  if (_trx)
  {
    std::string filter = _trx->diagnostic().diagParamMapItem(Diagnostic::PROGRAM_NAME);
    if (!filter.empty() && programCode.find(filter) == std::string::npos)
      return false;
  }
  return true;
}

bool
Diag898Collector::matchFareMarket(const MarketResponse* marketResponse) const
{

  if (!_trx || !marketResponse->marketCriteria())
    return true;

  const std::string& diagFareMarket = _trx->diagnostic().diagParamMapItem(Diagnostic::FARE_MARKET);

  if (!diagFareMarket.empty())
  {
    LocCode boardCity = diagFareMarket.substr(0, 3);
    LocCode offCity = diagFareMarket.substr(3, 3);

    if (marketResponse->marketCriteria()->departureAirportCode() != boardCity ||
        marketResponse->marketCriteria()->arrivalAirportCode() != offCity)
      return false;
  }

  return true;
}

bool
Diag898Collector::shouldDisplay(const StatusS8& status) const
{
  if (_trx)
  {
    std::string filter = _trx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL);
    if (filter == "PASSED" && status != PASS_S8)
      return false;
  }
  return true;
}

void
Diag898Collector::printDataNotFound() const
{

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "BRANDED DATA NOT FOUND\n";
}

void
Diag898Collector::printBrandedDataError() const
{

  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "*************** BRANDED FARES 898 ANALYSIS ***************\n";
  dc << "BRANDED DATA NOT FOUND\n";
}

bool
Diag898Collector::matchGlobalDirection(const MarketCriteria* marketCriteria,
                                       const MarketRule* marketRule) const
{
  if (!marketCriteria || !marketRule)
    return true;

  if (marketCriteria->globalDirection() == GlobalDirection::NO_DIR)
    return true;

  const std::string* prBrGlobalDir = globalDirectionToStr(marketCriteria->globalDirection());
  if (prBrGlobalDir == nullptr)
    return false;

  if (!(*prBrGlobalDir).empty() && marketRule->globalDirection() != GlobalDirection::NO_DIR &&
      marketRule->globalDirection() != marketCriteria->globalDirection())
    return false;

  return true;
}

bool
Diag898Collector::matchMarket(const MarketCriteria* marketCriteria, const MarketRule* marketRule)
    const
{

  if (!_trx || !marketCriteria || !marketRule)
    return true;

  if (marketRule->originLocType() == 'M' && marketRule->destinationLocType() == 'M' &&
      matchAirport(marketCriteria, marketRule))
    return true;

  const Loc* origin =
      _trx->dataHandle().getLoc(marketCriteria->departureAirportCode(), _trx->ticketingDate());
  const Loc* destination =
      _trx->dataHandle().getLoc(marketCriteria->arrivalAirportCode(), _trx->ticketingDate());
  if (!origin || !destination)
    return true;

  GeoTravelType geoTvlType = GeoTravelType::International;
  if (LocUtil::isDomestic(*origin, *destination))
    geoTvlType = GeoTravelType::Domestic;

  return matchLocation(marketCriteria, marketRule, origin, destination, geoTvlType);
}

bool
Diag898Collector::isLocInBetween(const LocCode& originLoc,
                                 const LocCode& destinationLoc,
                                 const LocTypeCode& originLocType,
                                 const LocTypeCode& destinationLocType,
                                 const Loc& origin,
                                 const Loc& destination,
                                 const VendorCode& vendor,
                                 const GeoTravelType geoTvlType,
                                 const CarrierCode& carrier) const
{
  if (LocUtil::isInLoc(origin,
                       originLocType,
                       originLoc,
                       vendor,
                       RESERVED,
                       LocUtil::OTHER,
                       geoTvlType,
                       carrier,
                       _trx->ticketingDate()) &&
      LocUtil::isInLoc(destination,
                       destinationLocType,
                       destinationLoc,
                       vendor,
                       RESERVED,
                       LocUtil::OTHER,
                       geoTvlType,
                       carrier,
                       _trx->ticketingDate()))
    return true;
  else if (LocUtil::isInLoc(destination,
                            originLocType,
                            originLoc,
                            vendor,
                            RESERVED,
                            LocUtil::OTHER,
                            geoTvlType,
                            carrier,
                            _trx->ticketingDate()) &&
           LocUtil::isInLoc(origin,
                            destinationLocType,
                            destinationLoc,
                            vendor,
                            RESERVED,
                            LocUtil::OTHER,
                            geoTvlType,
                            carrier,
                            _trx->ticketingDate()))
    return true;

  return false;
}

bool
Diag898Collector::isZoneInBetween(const LocCode& originLoc,
                                  const LocCode& destinationLoc,
                                  const LocTypeCode& originLocType,
                                  const LocTypeCode& destinationLocType,
                                  const Loc& origin,
                                  const Loc& destination,
                                  const VendorCode& vendor,
                                  const GeoTravelType geoTvlType,
                                  const CarrierCode& carrier) const
{
  if (LocUtil::isInZone(origin,
                        vendor,
                        originLoc,
                        TAX_ZONE,
                        LocUtil::OTHER,
                        geoTvlType,
                        carrier,
                        _trx->ticketingDate()) &&
      LocUtil::isInZone(destination,
                        vendor,
                        destinationLoc,
                        TAX_ZONE,
                        LocUtil::OTHER,
                        geoTvlType,
                        carrier,
                        _trx->ticketingDate()))
    return true;
  else if (LocUtil::isInZone(destination,
                             vendor,
                             originLoc,
                             TAX_ZONE,
                             LocUtil::OTHER,
                             geoTvlType,
                             carrier,
                             _trx->ticketingDate()) &&
           LocUtil::isInZone(origin,
                             vendor,
                             destinationLoc,
                             TAX_ZONE,
                             LocUtil::OTHER,
                             geoTvlType,
                             carrier,
                             _trx->ticketingDate()))
    return true;

  return false;
}

bool
Diag898Collector::matchLocation(const MarketCriteria* marketCriteria,
                                const MarketRule* marketRule,
                                const Loc* origin,
                                const Loc* destination,
                                const GeoTravelType geoTvlType) const
{

  VendorCode vendor = EMPTY_VENDOR;

  if (marketRule->direction() == "OI")
  {

    if (marketRule->originLocType() == LOCTYPE_USER &&
        marketRule->destinationLocType() == LOCTYPE_USER)
    {
      return isZoneInBetween(marketRule->originLoc(),
                             marketRule->destinationLoc(),
                             marketRule->originLocType(),
                             marketRule->destinationLocType(),
                             *origin,
                             *destination,
                             vendor,
                             geoTvlType,
                             marketRule->carrier());
    }
    else if (marketRule->originLocType() != LOCTYPE_USER &&
             marketRule->destinationLocType() != LOCTYPE_USER)
    {
      return isLocInBetween(marketRule->originLoc(),
                            marketRule->destinationLoc(),
                            marketRule->originLocType(),
                            marketRule->destinationLocType(),
                            *origin,
                            *destination,
                            vendor,
                            geoTvlType,
                            marketRule->carrier());
    }
  }
  else if (marketRule->direction() == "OT")
  {

    if (marketRule->originLocType() == LOCTYPE_USER &&
        marketRule->destinationLocType() == LOCTYPE_USER)
    {
      return LocUtil::isInZone(*origin,
                               vendor,
                               marketRule->originLoc(),
                               TAX_ZONE,
                               LocUtil::OTHER,
                               geoTvlType,
                               marketRule->carrier(),
                               _trx->ticketingDate()) &&
             LocUtil::isInZone(*destination,
                               vendor,
                               marketRule->destinationLoc(),
                               TAX_ZONE,
                               LocUtil::OTHER,
                               geoTvlType,
                               marketRule->carrier(),
                               _trx->ticketingDate());
    }
    else if (marketRule->originLocType() != LOCTYPE_USER &&
             marketRule->destinationLocType() != LOCTYPE_USER)
    {

      return LocUtil::isInLoc(*origin,
                              marketRule->originLocType(),
                              marketRule->originLoc(),
                              vendor,
                              RESERVED,
                              LocUtil::OTHER,
                              geoTvlType,
                              marketRule->carrier(),
                              _trx->ticketingDate()) &&
             LocUtil::isInLoc(*destination,
                              marketRule->destinationLocType(),
                              marketRule->destinationLoc(),
                              vendor,
                              RESERVED,
                              LocUtil::OTHER,
                              geoTvlType,
                              marketRule->carrier(),
                              _trx->ticketingDate());
    }
  }
  else if (marketRule->direction() == "IN")
  {

    if (marketRule->originLocType() == LOCTYPE_USER &&
        marketRule->destinationLocType() == LOCTYPE_USER)
    {

      return LocUtil::isInZone(*destination,
                               vendor,
                               marketRule->originLoc(),
                               TAX_ZONE,
                               LocUtil::OTHER,
                               geoTvlType,
                               marketRule->carrier(),
                               _trx->ticketingDate()) &&
             LocUtil::isInZone(*origin,
                               vendor,
                               marketRule->destinationLoc(),
                               TAX_ZONE,
                               LocUtil::OTHER,
                               geoTvlType,
                               marketRule->carrier(),
                               _trx->ticketingDate());
    }
    else if (marketRule->originLocType() != LOCTYPE_USER &&
             marketRule->destinationLocType() != LOCTYPE_USER)
    {

      return LocUtil::isInLoc(*destination,
                              marketRule->originLocType(),
                              marketRule->originLoc(),
                              vendor,
                              RESERVED,
                              LocUtil::OTHER,
                              geoTvlType,
                              marketRule->carrier(),
                              _trx->ticketingDate()) &&
             LocUtil::isInLoc(*origin,
                              marketRule->destinationLocType(),
                              marketRule->destinationLoc(),
                              vendor,
                              RESERVED,
                              LocUtil::OTHER,
                              geoTvlType,
                              marketRule->carrier(),
                              _trx->ticketingDate());
    }
  }

  return false;
}

bool
Diag898Collector::matchAirport(const MarketCriteria* marketCriteria, const MarketRule* marketRule)
    const
{

  if (marketRule->direction() == "OI")
  {
    if ((marketRule->originLoc() == marketCriteria->departureAirportCode() &&
         marketRule->destinationLoc() == marketCriteria->arrivalAirportCode()) ||
        (marketRule->destinationLoc() == marketCriteria->departureAirportCode() &&
         marketRule->originLoc() == marketCriteria->arrivalAirportCode()))
      return true;
  }
  else if (marketRule->direction() == "OT")
  {
    if (marketRule->originLoc() == marketCriteria->departureAirportCode() &&
        marketRule->destinationLoc() == marketCriteria->arrivalAirportCode())
      return true;
  }
  else if (marketRule->direction() == "IN")
  {
    if (marketRule->destinationLoc() == marketCriteria->departureAirportCode() &&
        marketRule->originLoc() == marketCriteria->arrivalAirportCode())
      return true;
  }

  return false;
}

void
Diag898Collector::setStatusForMarketFailure(const MarketCriteria* marketCriteria,
                                            const MarketRule* marketRule,
                                            std::string& statusStr)
{

  if (!marketCriteria || !marketRule)
    return;

  if (!matchGlobalDirection(marketCriteria, marketRule))
    statusStr = "FAIL GLOBAL DIRECTION";
  else if (!matchGeo(marketCriteria, marketRule))
    statusStr = "FAIL GEOGRAPHY";
  else if (!matchMarket(marketCriteria, marketRule))
    statusStr = "FAIL DIRECTION";
}

void
Diag898Collector::setStatusPassSeclected(const RuleExecution* ruleExecution,
                                         const MarketResponse* marketResponse,
                                         std::string& statusStr)
{
  if (std::find(marketResponse->programIDList().begin(),
                marketResponse->programIDList().end(),
                ruleExecution->ruleID()) != marketResponse->programIDList().end())
    statusStr = "PASS SELECTED";
}

void
Diag898Collector::setStatusStr(const RuleExecution* ruleExecution,
                               const MarketResponse* marketResponse,
                               std::string& statusStr)
{
  if (ruleExecution->status() == PASS_S8)
    setStatusPassSeclected(ruleExecution, marketResponse, statusStr);
  else if (ruleExecution->status() == FAIL_S8_MARKET)
  {
    for (const MarketRuleInfo* marketRuleInfo : ruleExecution->marketRules())
      setStatusForMarketFailure(
          marketResponse->marketCriteria(), marketRuleInfo->_marketRule, statusStr);
  }
}

bool
Diag898Collector::matchGeo(const MarketCriteria* marketCriteria, const MarketRule* marketRule) const
{

  if (!_trx || !marketCriteria || !marketRule)
    return true;

  if (marketRule->originLocType() == 'M' && marketRule->destinationLocType() == 'M' &&
      matchGeoAirport(marketCriteria, marketRule))
    return true;

  const Loc* origin =
      _trx->dataHandle().getLoc(marketCriteria->departureAirportCode(), _trx->ticketingDate());
  const Loc* destination =
      _trx->dataHandle().getLoc(marketCriteria->arrivalAirportCode(), _trx->ticketingDate());

  if (!origin || !destination)
    return true;

  GeoTravelType geoTvlType = GeoTravelType::International;
  if (LocUtil::isDomestic(*origin, *destination))
    geoTvlType = GeoTravelType::Domestic;

  return matchGeoLocation(marketCriteria, marketRule, origin, destination, geoTvlType);
}

bool
Diag898Collector::isGeoLocInBetween(const LocCode& originLoc,
                                    const LocCode& destinationLoc,
                                    const LocTypeCode& originLocType,
                                    const LocTypeCode& destinationLocType,
                                    const Loc& origin,
                                    const Loc& destination,
                                    const VendorCode& vendor,
                                    const GeoTravelType geoTvlType,
                                    const CarrierCode& carrier) const
{

  if ((LocUtil::isInLoc(origin,
                        originLocType,
                        originLoc,
                        vendor,
                        RESERVED,
                        LocUtil::OTHER,
                        geoTvlType,
                        carrier,
                        _trx->ticketingDate()) ||
       LocUtil::isInLoc(origin,
                        destinationLocType,
                        destinationLoc,
                        vendor,
                        RESERVED,
                        LocUtil::OTHER,
                        geoTvlType,
                        carrier,
                        _trx->ticketingDate())) &&
      (LocUtil::isInLoc(destination,
                        destinationLocType,
                        destinationLoc,
                        vendor,
                        RESERVED,
                        LocUtil::OTHER,
                        geoTvlType,
                        carrier,
                        _trx->ticketingDate()) ||
       LocUtil::isInLoc(destination,
                        originLocType,
                        originLoc,
                        vendor,
                        RESERVED,
                        LocUtil::OTHER,
                        geoTvlType,
                        carrier,
                        _trx->ticketingDate())))
    return true;

  return false;
}

bool
Diag898Collector::isGeoZoneInBetween(const LocCode& originLoc,
                                     const LocCode& destinationLoc,
                                     const LocTypeCode& originLocType,
                                     const LocTypeCode& destinationLocType,
                                     const Loc& origin,
                                     const Loc& destination,
                                     const VendorCode& vendor,
                                     const GeoTravelType geoTvlType,
                                     const CarrierCode& carrier) const
{
  if ((LocUtil::isInZone(origin,
                         vendor,
                         originLoc,
                         TAX_ZONE,
                         LocUtil::OTHER,
                         geoTvlType,
                         carrier,
                         _trx->ticketingDate()) ||
       LocUtil::isInZone(origin,
                         vendor,
                         destinationLoc,
                         TAX_ZONE,
                         LocUtil::OTHER,
                         geoTvlType,
                         carrier,
                         _trx->ticketingDate())) &&
      (LocUtil::isInZone(destination,
                         vendor,
                         destinationLoc,
                         TAX_ZONE,
                         LocUtil::OTHER,
                         geoTvlType,
                         carrier,
                         _trx->ticketingDate()) ||
       LocUtil::isInZone(destination,
                         vendor,
                         originLoc,
                         TAX_ZONE,
                         LocUtil::OTHER,
                         geoTvlType,
                         carrier,
                         _trx->ticketingDate())))
    return true;

  return false;
}

bool
Diag898Collector::matchGeoLocation(const MarketCriteria* marketCriteria,
                                   const MarketRule* marketRule,
                                   const Loc* origin,
                                   const Loc* destination,
                                   const GeoTravelType geoTvlType) const
{

  VendorCode vendor = EMPTY_VENDOR;

  if (marketRule->originLocType() == LOCTYPE_USER &&
      marketRule->destinationLocType() == LOCTYPE_USER)
    return isGeoZoneInBetween(marketRule->originLoc(),
                              marketRule->destinationLoc(),
                              marketRule->originLocType(),
                              marketRule->destinationLocType(),
                              *origin,
                              *destination,
                              vendor,
                              geoTvlType,
                              marketRule->carrier());
  else if (marketRule->originLocType() != LOCTYPE_USER &&
           marketRule->destinationLocType() != LOCTYPE_USER)
    return isGeoLocInBetween(marketRule->originLoc(),
                             marketRule->destinationLoc(),
                             marketRule->originLocType(),
                             marketRule->destinationLocType(),
                             *origin,
                             *destination,
                             vendor,
                             geoTvlType,
                             marketRule->carrier());

  return false;
}

bool
Diag898Collector::matchGeoAirport(const MarketCriteria* marketCriteria,
                                  const MarketRule* marketRule) const
{

  if ((marketRule->originLoc() == marketCriteria->departureAirportCode() ||
       marketRule->destinationLoc() == marketCriteria->departureAirportCode()) &&
      (marketRule->destinationLoc() == marketCriteria->arrivalAirportCode() ||
       marketRule->originLoc() == marketCriteria->arrivalAirportCode()))
    return true;

  return false;
}

} // namespace
