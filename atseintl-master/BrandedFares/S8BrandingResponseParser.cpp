//-------------------------------------------------------------------
//
//  File:        S8BrandingResponseParser.cpp
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

#include "BrandedFares/S8BrandingResponseParser.h"

#include "BrandedFares/BrandFeatureItem.h"
#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandingResponse.h"
#include "BrandedFares/BrandingSchemaNames.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/BSDiagnostics.h"
#include "BrandedFares/MarketCriteria.h"
#include "BrandedFares/MarketResponse.h"
#include "BrandedFares/MarketRule.h"
#include "BrandedFares/RequestSource.h"
#include "BrandedFares/RuleExecution.h"
#include "BrandedFares/S8BrandingSecurity.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "Xform/CustomXMLParser/IParser.h"
#include "Xform/CustomXMLParser/IXMLSchema.h"
#include "Xform/CustomXMLParser/IXMLUtils.h"

#include <boost/tokenizer.hpp>

#define CARRIER_GDS_NAME "CARRIER GDS"
#define DEPARTMENT_CODE_NAME "DEPT CODE"
#define OFFICEDESIGNATOR_NAME "OFFICE DESIG"
#define MARKET_LOCATION_NAME "MARKET LOC"
#define IATA_NUMBER_NAME "IATA NUM"
#define IS_VIEW_BOOK_TICKET_NAME "VIEW BOOK"

namespace
{
ILookupMap _elemLookupMap, _attrLookupMap;
}

bool
init(IXMLUtils::initLookupMaps(tse::brands::_BrandingElementNames,
                               tse::brands::_NumberOfElementNames_,
                               _elemLookupMap,
                               tse::brands::_BrandingAttributeNames,
                               tse::brands::_NumberOfAttributeNames_,
                               _attrLookupMap));

namespace tse
{
FALLBACK_DECL(fallbackUseOnlyFirstProgram)

using namespace brands;

static Logger
logger("atseintl.BrandedFares.S8BrandingResponseParser");

void
S8BrandingResponseParser::parse(const std::string& content)
{
  IValueString attrValueArray[_NumberOfAttributeNames_];
  int attrRefArray[_NumberOfAttributeNames_];
  IXMLSchema schema(
      _elemLookupMap, _attrLookupMap, _NumberOfAttributeNames_, attrValueArray, attrRefArray, true);
  IParser parser(content, *this, schema);
  parser.parse();
}

bool
S8BrandingResponseParser::startElement(const IKeyString&, const IAttributes&)
{
  return false;
}

bool
S8BrandingResponseParser::endElement(const IKeyString&)
{
  return false;
}

void
S8BrandingResponseParser::characters(const char* value, size_t length)
{
  _value.assign(value, length);
}

bool
S8BrandingResponseParser::startElement(int idx, const IAttributes& attrs)
{

  switch (idx)
  {
  case _ResponseSource:
    _trx.dataHandle().get(_requestSource);
    onStartResponseSource(attrs);
    break;
  case _MarketResponse:
    _marketResponseOpen = true;
    _trx.dataHandle().get(_marketResponse);
    attrs.get(_marketID, _marketResponse->setMarketID(), 0);
    break;
  case _MarketCriteria:
    _trx.dataHandle().get(_marketResponse->marketCriteria());
    onStartMarketCriteria(attrs);
    break;
  case _Message:
    if (_marketResponseOpen)
      attrs.get(_messageText, _marketResponse->errMessage());
    else
      onStartBrandingResponse(attrs);
    break;
  case _BrandProgram:
    _brandProgramOpen = true;
    _trx.dataHandle().get(_brandProgram);
    {
      std::string value;
      attrs.get(_programID, value);
      _brandProgram->programID() = value;
      std::string dataSourceValue;
      attrs.get(_dataSource, dataSourceValue);
      if (dataSourceValue == "CBS")
        _brandProgram->dataSource() = BRAND_SOURCE_CBAS;
      else
        _brandProgram->dataSource() = BRAND_SOURCE_S8;
    }
    break;
  case _Brand:
    _trx.dataHandle().get(_brand);
    break;
  case _Ancillary:
    onStartAncillary(attrs);
    break;
  case _BrandingResponse:
    _trx.dataHandle().get(_brandingResponse);
    break;
  case _Diagnostics:
    _trx.dataHandle().get(_marketResponse->bsDiagnostics());
    break;
  case _RuleExecution:
    _ruleExecutionOpen = true;
    _trx.dataHandle().get(_ruleExecution);
    {
      std::string value;
      attrs.get(_ruleId, value);
      _ruleExecution->ruleID() = value;
    }
    for (auto& elem : _securityStatus)
      elem = SS_NONE;

    break;
  case _Condition:
    onStartCondition(attrs);
    break;

  }

  startElementCbas(idx, attrs);

  return true;
}

void
S8BrandingResponseParser::onStartResponseSource(const IAttributes& attrs)
{
  std::string distributionChannel;
  attrs.get(_distributionChannel, distributionChannel);
  std::string pseudoCityCode;
  attrs.get(_pseudoCityCode, pseudoCityCode);
  std::string iataNumber;
  attrs.get(_iataNumber, iataNumber);
  std::string clientID;
  attrs.get(_clientID, clientID);
  _requestSource->clientID() = clientID;
  std::string requestType;
  attrs.get(_requestType, requestType);
  std::string requestingCarrierGDS;
  attrs.get(_requestingCarrierGDS, requestingCarrierGDS);
  std::string geoLocation;
  attrs.get(_geoLocation, geoLocation);
  std::string departmentCode;
  attrs.get(_departmentCode, departmentCode);
  std::string officeDesignator;
  attrs.get(_officeDesignator, officeDesignator);
}

void
S8BrandingResponseParser::onStartMarketCriteria(const IAttributes& attrs)
{
  std::string value;
  attrs.get(_direction, value);
  _marketResponse->marketCriteria()->direction() = value;

  attrs.get(_globalIndicator, value);
  GlobalDirection dst;
  bool rc = strToGlobalDirection(dst, value.c_str());
  if (rc)
  {
    _marketResponse->marketCriteria()->globalDirection() = dst;
  }
}

void
S8BrandingResponseParser::onStartBrandingResponse(const IAttributes& attrs)
{
  if (_brandingResponse == nullptr)
  {
    LOG4CXX_ERROR(logger,
                  "S8BrandingResponseParser::onStartBrandingResponse _brandingResponse is NULL");
    return;
  }

  attrs.get(_messageCode, _brandingResponse->messageCode());
  attrs.get(_failCode, _brandingResponse->failCode());
  attrs.get(_messageText, _brandingResponse->messageText());
}

void
S8BrandingResponseParser::onStartAncillary(const IAttributes& attrs)
{
  _trx.dataHandle().get(_featureItem);
  _featureItem->setSequenceNumber(attrs.get(_sequenceNumber, _featureItem->getSequenceNumber()));
  _featureItem->setSubCode(attrs.get(_subCode, std::string()));
  _featureItem->setServiceType(attrs.get(_service, _featureItem->getServiceType()));
  _featureItem->setApplication(attrs.get(_application, _featureItem->getApplication()));
  // setCommercialName() will be called in onEndAncillary()
}

void
S8BrandingResponseParser::onStartCondition(const IAttributes& attrs)
{
  std::string pattern;
  attrs.get(_pattern, pattern);
  size_t pos(0);
  _s8PatternType = S8_PT_NONE;
  static const std::string MARKET_MATCH(
      "marketLocationHelper.isMarketMatch(origin, destination, direction, rqGlobalIndicator,");
  static const size_t MARKET_MATCH_SZ(MARKET_MATCH.size());
  static const std::string DATE_RANGE("DateHelper.isDateInRange");
  static const size_t DATE_RANGE_SZ(DATE_RANGE.size());
  static const std::string PASSENGER_TYPE(
      "PassengerTypeHelper.isPassengerTypeValid(passengerTypes,");
  static const size_t PASSENGER_TYPE_SZ(PASSENGER_TYPE.size());
  static const std::string ACCOUNT_CODES("accountCodes)");
  static const std::string CONTAINS_AN_ELEMENT_FROM("CollectionHelper.containsAnElementFrom");
  static const size_t CONTAINS_AN_ELEMENT_FROM_SZ(CONTAINS_AN_ELEMENT_FROM.size());
  static const std::string PCC_IN("pcc in (");
  static const size_t PCC_IN_SZ(PCC_IN.size());
  static const std::string CARRIER_EQ("carrier == ");
  static const size_t CARRIER_EQ_SZ(CARRIER_EQ.size());
  static const std::string CARRIER_GDS("carrierGds in (");
  static const size_t CARRIER_GDS_SZ(CARRIER_GDS.size());
  static const std::string DEPARTMENT_CODE("departmentCode in (");
  static const size_t DEPARTMENT_CODE_SZ(DEPARTMENT_CODE.size());
  static const std::string OFFICEDESIGNATOR("officeDesignator in (");
  static const size_t OFFICEDESIGNATOR_SZ(OFFICEDESIGNATOR.size());
  static const std::string MARKET_LOCATION("locationHelper.isMarketInLocation(geoLocation,");
  static const size_t MARKET_LOCATION_SZ(MARKET_LOCATION.size());
  static const std::string IATA_NUMBER("iataNumber in (");
  static const size_t IATA_NUMBER_SZ(IATA_NUMBER.size());
  static const std::string IATA_NUMBER_NULL("iataNumber == null");
  static const std::string IS_VIEW_BOOK_TICKET("isViewBookTicket == ");
  static const size_t IS_VIEW_BOOK_TICKET_SZ(IS_VIEW_BOOK_TICKET.size());

  std::string patternValue;

  if ((pos = pattern.find(MARKET_MATCH)) != std::string::npos)
  {
    _s8PatternType = S8_PT_MARKET;
    patternValue = pattern.substr(pos + MARKET_MATCH_SZ);
    parseMarketMatch(patternValue);
  }
  else if ((pos = pattern.find(DATE_RANGE)) != std::string::npos)
  {
    patternValue = pattern.substr(pos + DATE_RANGE_SZ);
    parseDateRange(patternValue);
  }
  else if ((pos = pattern.find(PASSENGER_TYPE)) != std::string::npos)
  {
    _s8PatternType = S8_PT_PASSENGER_TYPE;
    patternValue = pattern.substr(pos + PASSENGER_TYPE_SZ);
    parsePassengerType(patternValue);
  }
  else if ((pattern.find(ACCOUNT_CODES) != std::string::npos) &&
           ((pos = pattern.find(CONTAINS_AN_ELEMENT_FROM)) != std::string::npos))
  {
    _s8PatternType = S8_PT_ACCOUNT_CODE;
    patternValue = pattern.substr(pos + CONTAINS_AN_ELEMENT_FROM_SZ);
    parseAccountCodes(patternValue);
  }
  else if ((pos = pattern.find(PCC_IN)) != std::string::npos)
  {
    _s8PatternType = S8_PT_PCC;
    patternValue = pattern.substr(pos + PCC_IN_SZ);
    parsePCC(patternValue);
  }
  else if ((pos = pattern.find(CARRIER_EQ)) != std::string::npos)
  {
    _s8PatternType = S8_PT_CARRIER;
    patternValue = pattern.substr(pos + CARRIER_EQ_SZ);
    _governingCarrierVec.push_back(patternValue.substr(1, patternValue.length() - 2));
  }
  else if ((pos = pattern.find(CARRIER_GDS)) != std::string::npos)
  {
    patternValue = pattern.substr(pos + CARRIER_GDS_SZ);
    _securityName = CARRIER_GDS_NAME;
    _securityValue = patternValue.substr(1, patternValue.length() - 3);
  }
  else if ((pos = pattern.find(DEPARTMENT_CODE)) != std::string::npos)
  {
    patternValue = pattern.substr(pos + DEPARTMENT_CODE_SZ);
    _securityName = DEPARTMENT_CODE_NAME;
    _securityValue = patternValue.substr(1, patternValue.length() - 3);
  }
  else if ((pos = pattern.find(OFFICEDESIGNATOR)) != std::string::npos)
  {
    patternValue = pattern.substr(pos + OFFICEDESIGNATOR_SZ);
    _securityName = OFFICEDESIGNATOR_NAME;
    _securityValue = patternValue.substr(1, patternValue.length() - 3);
  }
  else if ((pos = pattern.find(MARKET_LOCATION)) != std::string::npos)
  {
    patternValue = pattern.substr(pos + MARKET_LOCATION_SZ);
    _securityName = MARKET_LOCATION_NAME;
    parseMarketLocation(patternValue);
  }
  else if ((pos = pattern.find(IATA_NUMBER)) != std::string::npos)
  {
    patternValue = pattern.substr(pos + IATA_NUMBER_SZ);
    parseIataNumbers(patternValue);
  }
  else if ((pos = pattern.find(IATA_NUMBER_NULL)) != std::string::npos)
  {
    _securityName = IATA_NUMBER_NAME;
  }
  else if ((pos = pattern.find(IS_VIEW_BOOK_TICKET)) != std::string::npos)
  {
    patternValue = pattern.substr(pos + IS_VIEW_BOOK_TICKET_SZ);
    _securityName = IS_VIEW_BOOK_TICKET_NAME;
    _securityValue = patternValue;
  }
}

void
S8BrandingResponseParser::parseMarketMatch(const std::string& patternValue)
{
  _trx.dataHandle().get(_marketRule);

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> separator(", )");
  tokenizer tokens(patternValue, separator);
  int count = 1;
  std::string finalTmp;

  for (const std::string& tmp : tokens)
  {
    finalTmp = "";
    if (count == 1)
      _marketRule->originLoc() = tmp.substr(1, tmp.length() - 2);
    else if (count == 2)
    {
      finalTmp = tmp.substr(1, tmp.length() - 2);
      if (!finalTmp.empty())
        _marketRule->originLocType() = finalTmp.at(0);
    }
    else if (count == 3)
      _marketRule->destinationLoc() = tmp.substr(1, tmp.length() - 2);
    else if (count == 4)
    {
      finalTmp = tmp.substr(1, tmp.length() - 2);
      if (!finalTmp.empty())
        _marketRule->destinationLocType() = finalTmp.at(0);
    }
    else if (count == 5)
      _marketRule->direction() = tmp.substr(1, tmp.length() - 2);
    else if (count == 6)
    {
      finalTmp = tmp.substr(1, tmp.length() - 2);
      if (!finalTmp.empty())
      {
        GlobalDirection dst;
        bool rc = strToGlobalDirection(dst, finalTmp.c_str());
        if (rc)
          _marketRule->globalDirection() = dst;
      }
    }
    else if (count == 7)
      _marketRule->carrier() = tmp.substr(1, tmp.length() - 2);

    count++;
  }

  _marketRuleVec.push_back(_marketRule);
}

void
S8BrandingResponseParser::parseDateRange(const std::string& patternValue)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> separator(", ()");
  tokenizer tokens(patternValue, separator);
  int count = 1;
  std::string type;
  std::string day;
  std::string month;
  std::string year;
  for (const std::string& tmp : tokens)
  {
    if (count == 1)
      type = tmp;
    else if (count == 2)
      day = tmp.substr(1, tmp.length() - 2);
    else if (count == 3)
      month = tmp.substr(1, tmp.length() - 2);
    else if (count == 4)
    {
      year = tmp.substr(1, tmp.length() - 2);
      DateTime dateTime(year + "/" + month + "/" + day, 0);
      if (type == "travelDate")
      {
        _s8PatternType = S8_PT_TRAVEL_DATE;
        _ruleExecution->travelDateStart() = dateTime;
      }
      else if (type == "salesDate")
      {
        _s8PatternType = S8_PT_SALES_DATE;
        _ruleExecution->salesDateStart() = dateTime;
      }
    }
    else if (count == 5)
      day = tmp.substr(1, tmp.length() - 2);
    else if (count == 6)
      month = tmp.substr(1, tmp.length() - 2);
    else if (count == 7)
    {
      year = tmp.substr(1, tmp.length() - 2);
      DateTime dateTime(year + "/" + month + "/" + day, 0);
      if (type == "travelDate")
        _ruleExecution->travelDateEnd() = dateTime;
      if (type == "salesDate")
        _ruleExecution->salesDateEnd() = dateTime;
    }

    count++;
  }

  std::string dateStr = year + "/" + month + "/" + day;
  DateTime dateTime(dateStr, 0);
}

void
S8BrandingResponseParser::parsePassengerType(const std::string& patternValue)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> separator(", )");
  tokenizer tokens(patternValue, separator);

  for (const std::string& tmp : tokens)
  {
    _passengerTypeVec.push_back(tmp.substr(1, tmp.length() - 2));
    break;
  }
}

void
S8BrandingResponseParser::parseAccountCodes(const std::string& patternValue)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> separator(", ()''");
  tokenizer tokens(patternValue, separator);

  for (const std::string& tmp : tokens)
  {
    if (tmp != "accountCodes")
      _accountCodeVec.push_back(tmp);
  }
}

void
S8BrandingResponseParser::parsePCC(const std::string& patternValue)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> separator(", )");
  tokenizer tokens(patternValue, separator);

  for (const std::string& tmp : tokens)
    _pseudoCityCodeVec.push_back(tmp.substr(1, tmp.length() - 2));
}

void
S8BrandingResponseParser::parseMarketLocation(const std::string& patternValue)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> separator(", )");
  tokenizer tokens(patternValue, separator);
  int count = 1;

  for (const std::string& tmp : tokens)
  {
    if (count == 1)
      _securityValue = tmp.substr(1, tmp.length() - 2);
    else if (count == 2)
    {
      std::string finalTmp = tmp.substr(1, tmp.length() - 2);
      if (!finalTmp.empty())
      {
        _securityValue += "|";
        _securityValue += finalTmp;
      }
      break;
    }

    count++;
  }
}

void
S8BrandingResponseParser::parseIataNumbers(const std::string& patternValue)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> separator(", ()''");
  tokenizer tokens(patternValue, separator);

  for (const std::string& tmp : tokens)
    _iataNumbers.push_back(tmp);
}

bool
S8BrandingResponseParser::endElement(int idx)
{

  switch (idx)
  {
  case _ProgramID:
    _marketResponse->programIDList().push_back(ProgramID(_value.c_str(), _value.length()));
    break;
  case _Carrier:
    _marketResponse->carrier().assign(_value.c_str(), _value.length());
    break;
  case _DepartureAirportCode:
    _marketResponse->marketCriteria()->departureAirportCode().assign(_value.c_str(),
                                                                     _value.length());
    break;
  case _ArrivalAirportCode:
    _marketResponse->marketCriteria()->arrivalAirportCode().assign(_value.c_str(), _value.length());
    break;
  case _Type:
    if (_brandProgramOpen)
      _brandProgram->passengerType().push_back(PaxTypeCode(_value.c_str(), _value.length()));
    else
    {
      PaxTypeCode tmpPaxTypeCode(_value.c_str(), _value.length());
      if (std::find(_marketResponse->marketCriteria()->paxType().begin(),
                    _marketResponse->marketCriteria()->paxType().end(),
                    tmpPaxTypeCode) == _marketResponse->marketCriteria()->paxType().end())
        _marketResponse->marketCriteria()->paxType().push_back(tmpPaxTypeCode);
    }
    break;
  case _BrandProgram:
    onEndBrandProgram();
    break;
  case _Ancillary:
    onEndAncillary();
    break;
  case _Vendor:
    if (_ruleExecutionOpen)
      _ruleExecution->vendorCode().assign(_value.c_str(), _value.length());
    else if(_brandProgram)
      _brandProgram->vendorCode().assign(_value.c_str(), _value.length());
    break;
  case _Sequence:
    _brandProgram->sequenceNo() = atoi(_value.c_str());
    break;
  case _ProgramCode:
    if (_ruleExecutionOpen)
      _ruleExecution->programCode().assign(_value.c_str(), _value.length());
    else
      _brandProgram->programCode().assign(_value.c_str(), _value.length());
    break;
  case _ProgramName:
    _brandProgram->programName().assign(_value.c_str(), _value.length());
    break;
  case _ProgramDescription:
    _brandProgram->programDescription().assign(_value.c_str(), _value.length());
    break;
  case _PassengerType:
    _brandProgram->passengerType().push_back(PaxTypeCode(_value.c_str(), _value.length()));
    break;
  case _Brand:
    _brandProgram->brandsData().push_back(_brand);
    break;
  case _Code:
    parseCode();
    break;
  case _Name:
    _brand->brandName().assign(_value.c_str(), _value.length());
    break;
  case _Tier:
    _brand->tier() = atoi(_value.c_str());
    break;
  case _PrimaryFareIDTable:
    _brand->primaryFareIdTable() = atoll(_value.c_str());
    break;
  case _SecondaryFareIDTable:
    _brand->secondaryFareIdTable() = atoll(_value.c_str());
    break;
  case _AccountCode:
    _brandProgram->accountCodes().push_back(std::string(_value.c_str(), _value.length()));
    break;
  case _MarketResponse:
    onEndMarketResponse();
    break;
  case _GetAirlineBrandsRS:
    onEndGetAirlineBrandsRS();
    break;
  case _GlobalIndicator:
  {
    std::string tmp;
    tmp.assign(_value.c_str(), _value.length());
    GlobalDirection dst;
    if (strToGlobalDirection(dst, tmp))
    {
      _brandProgram->globalDirection() = dst;
    }
  }
  break;
  case _Direction:
    _brandProgram->direction().assign(_value.c_str(), _value.length());
    break;
  case _EffectiveDate:
  {
    std::string dateStr(_value.c_str(), _value.length());
    ;
    _brandProgram->effectiveDate() = DateTime(dateStr, 0);
  }
  break;
  case _DiscontinueDate:
  {
    std::string dateStr(_value.c_str(), _value.length());
    ;
    _brandProgram->discontinueDate() = DateTime(dateStr, 0);
  }
  break;

  case _OriginLoc:
    _brandProgram->originLoc().assign(_value.c_str(), _value.length());
    break;

  case _Condition:
  {
    std::string condition;
    condition.assign(_value.c_str(), _value.length());
    if (_securityName.length())
    {

      if (_ruleExecution->s8BrandingSecurity() == nullptr)
        _trx.dataHandle().get(_ruleExecution->s8BrandingSecurity());
      StatusS8 status = PASS_S8;
      if (condition == "false")
        status = getSecurityStatus();

      SecurityInfo* securityInfo = nullptr;
      _trx.dataHandle().get(securityInfo);

      securityInfo->_securityName = _securityName;
      securityInfo->_securityValue = _securityValue;
      securityInfo->_status = status;
      _ruleExecution->s8BrandingSecurity()->securityInfoVec().push_back(securityInfo);
      _securityName = "";
      _securityValue = "";
    }
    else if (!_iataNumbers.empty())
      populateIataNumbersSecurity(condition);
    else if (!_accountCodeVec.empty())
      addPatternInfo(_accountCodeVec, condition, _ruleExecution->accountCodes());
    else if (!_pseudoCityCodeVec.empty())
      addPatternInfo(_pseudoCityCodeVec, condition, _ruleExecution->pseudoCityCodes());
    else if (!_passengerTypeVec.empty())
      addPatternInfo(_passengerTypeVec, condition, _ruleExecution->passengerType());
    else if (!_governingCarrierVec.empty())
      addPatternInfo(_governingCarrierVec, condition, _ruleExecution->governingCarrier());
    else if (!_marketRuleVec.empty())
      addMarketRuleInfo(_marketRuleVec, condition, _ruleExecution->marketRules());
    else if (condition == "false" && _s8PatternType != S8_PT_NONE)
    {
      setStatus();
      _s8PatternType = S8_PT_NONE;
    }
  }
  break;
  case _RuleExecution:
    _ruleExecutionOpen = false;
    _marketResponse->bsDiagnostics()->ruleExecution().push_back(_ruleExecution);
    checkPatternFailure();
    checkSecurityFailure();
    break;

  case _SystemCode:
  {
    std::string tmp;
    tmp.assign(_value.c_str(), _value.length());
    if (!tmp.empty())
      _brandProgram->systemCode() = tmp.at(0);
  }
  break;
  case _CarrierFlightItemNum:
    _brand->setCarrierFlightItemNum(atoi(_value.c_str()));
    break;

  }

  endElementCbas(idx);

  _value.clear();
  return true;
}

void
S8BrandingResponseParser::checkPatternFailure()
{
  checkFailure(_ruleExecution->accountCodes(), FAIL_S8_ACCOUNT_CODE);
  checkFailure(_ruleExecution->pseudoCityCodes(), FAIL_S8_PCC);
  checkFailure(_ruleExecution->passengerType(), FAIL_S8_PASSENGER_TYPE);
  checkFailure(_ruleExecution->governingCarrier(), FAIL_S8_CARRIER);
  checkMarketFailure(_ruleExecution->marketRules(), FAIL_S8_MARKET);
}

void
S8BrandingResponseParser::checkSecurityFailure()
{
  if (_ruleExecution->status() == PASS_S8 && _ruleExecution->s8BrandingSecurity())
  {
    for (const SecurityInfo* securityInfo : _ruleExecution->s8BrandingSecurity()->securityInfoVec())
    {
      if (securityInfo->_status == PASS_S8)
        processPassSecurity(securityInfo);
      else
        processFailSecurity(securityInfo);
    }

    for (const auto& elem : _securityStatus)
    {
      if (elem == SS_FAIL)
      {
        _ruleExecution->status() = FAIL_S8_SECURITY;
        break;
      }
    }
  }
}

void
S8BrandingResponseParser::processPassSecurity(const SecurityInfo* securityInfo)
{

  if (securityInfo->_securityName == CARRIER_GDS_NAME)
    _securityStatus[CARRIER_GDS] = SS_PASS;
  else if (securityInfo->_securityName == DEPARTMENT_CODE_NAME)
    _securityStatus[DEPT_CODE] = SS_PASS;
  else if (securityInfo->_securityName == OFFICEDESIGNATOR_NAME)
    _securityStatus[OFFICE_DESIG] = SS_PASS;
  else if (securityInfo->_securityName == MARKET_LOCATION_NAME)
    _securityStatus[MARKET_LOC] = SS_PASS;
  else if (securityInfo->_securityName == IATA_NUMBER_NAME)
    _securityStatus[IATA_NUM] = SS_PASS;
  else if (securityInfo->_securityName == IS_VIEW_BOOK_TICKET_NAME)
    _securityStatus[VIEW_BOOK] = SS_PASS;
}

void
S8BrandingResponseParser::processFailSecurity(const SecurityInfo* securityInfo)
{

  if (securityInfo->_securityName == CARRIER_GDS_NAME && _securityStatus[CARRIER_GDS] != SS_PASS)
    _securityStatus[CARRIER_GDS] = SS_FAIL;
  else if (securityInfo->_securityName == DEPARTMENT_CODE_NAME &&
           _securityStatus[DEPT_CODE] != SS_PASS)
    _securityStatus[DEPT_CODE] = SS_FAIL;
  else if (securityInfo->_securityName == OFFICEDESIGNATOR_NAME &&
           _securityStatus[OFFICE_DESIG] != SS_PASS)
    _securityStatus[OFFICE_DESIG] = SS_FAIL;
  else if (securityInfo->_securityName == MARKET_LOCATION_NAME &&
           _securityStatus[MARKET_LOC] != SS_PASS)
    _securityStatus[MARKET_LOC] = SS_FAIL;
  else if (securityInfo->_securityName == IATA_NUMBER_NAME && _securityStatus[IATA_NUM] != SS_PASS)
    _securityStatus[IATA_NUM] = SS_FAIL;
  else if (securityInfo->_securityName == IS_VIEW_BOOK_TICKET_NAME &&
           _securityStatus[VIEW_BOOK] != SS_PASS)
    _securityStatus[VIEW_BOOK] = SS_FAIL;
}

void
S8BrandingResponseParser::populateIataNumbersSecurity(const std::string& condition)
{
  if (_iataNumbers.empty())
    return;
  if (_ruleExecution->s8BrandingSecurity() == nullptr)
    _trx.dataHandle().get(_ruleExecution->s8BrandingSecurity());

  StatusS8 status = PASS_S8;
  if (condition == "false")
    status = getSecurityStatus();
  SecurityInfo* securityInfo = nullptr;

  for (const std::string& tmp : _iataNumbers)
  {
    _trx.dataHandle().get(securityInfo);
    securityInfo->_securityName = IATA_NUMBER_NAME;
    securityInfo->_securityValue = tmp;
    securityInfo->_status = status;
    _ruleExecution->s8BrandingSecurity()->securityInfoVec().push_back(securityInfo);
  }

  _iataNumbers.clear();
}

void
S8BrandingResponseParser::addPatternInfo(std::vector<std::string>& patternValueVec,
                                         const std::string& condition,
                                         std::vector<PatternInfo*>& ruleExecutionPatternVec)
{
  if (patternValueVec.empty())
    return;

  StatusS8 status = PASS_S8;

  if (condition == "false")
    setPatternStatus(status);

  PatternInfo* patternInfo = nullptr;

  for (const std::string& tmp : patternValueVec)
  {
    _trx.dataHandle().get(patternInfo);
    patternInfo->_patternValue = tmp;
    patternInfo->_status = status;
    ruleExecutionPatternVec.push_back(patternInfo);
  }

  patternValueVec.clear();
}

void
S8BrandingResponseParser::setPatternStatus(StatusS8& status)
{
  switch (_s8PatternType)
  {
  case S8_PT_PASSENGER_TYPE:
    status = FAIL_S8_PASSENGER_TYPE;
    break;
  case S8_PT_ACCOUNT_CODE:
    status = FAIL_S8_ACCOUNT_CODE;
    break;
  case S8_PT_MARKET:
    status = FAIL_S8_MARKET;
    break;
  case S8_PT_PCC:
    status = FAIL_S8_PCC;
    break;
  case S8_PT_CARRIER:
    status = FAIL_S8_CARRIER;
    break;
  case S8_PT_SALES_DATE:
    status = FAIL_S8_SALES_DATE;
    break;
  case S8_PT_TRAVEL_DATE:
    status = FAIL_S8_TRAVEL_DATE;
    break;
  default:
    break;
  }
}

void
S8BrandingResponseParser::checkFailure(const std::vector<PatternInfo*>& patternValueVec,
                                       StatusS8 status)
{

  if (patternValueVec.empty())
    return;

  if (_ruleExecution->status() == PASS_S8)
  {
    for (const PatternInfo* patternInfo : patternValueVec)
    {
      if (patternInfo->_status == PASS_S8)
        return;
    }
    _ruleExecution->status() = status;
  }
}

StatusS8
S8BrandingResponseParser::getSecurityStatus()
{
  StatusS8 status = PASS_S8;

  if (_securityName == CARRIER_GDS_NAME)
    status = FAIL_S8_CARRIER_GDS;
  else if (_securityName == DEPARTMENT_CODE_NAME)
    status = FAIL_S8_DEPARTMENT_CODE;
  else if (_securityName == OFFICEDESIGNATOR_NAME)
    status = FAIL_S8_OFFICE_DESIGNATOR;
  else if (_securityName == MARKET_LOCATION_NAME)
    status = FAIL_S8_AGENT_LOCATION;
  else if (_securityName == IATA_NUMBER_NAME)
    status = FAIL_S8_IATA_NUMBER;
  else if (_securityName == IS_VIEW_BOOK_TICKET_NAME)
    status = FAIL_S8_VIEW_BOOK_TICKET;

  return status;
}

void
S8BrandingResponseParser::setStatus()
{
  switch (_s8PatternType)
  {
  case S8_PT_PASSENGER_TYPE:
    _ruleExecution->status() = FAIL_S8_PASSENGER_TYPE;
    break;
  case S8_PT_ACCOUNT_CODE:
    _ruleExecution->status() = FAIL_S8_ACCOUNT_CODE;
    break;
  case S8_PT_MARKET:
    _ruleExecution->status() = FAIL_S8_MARKET;
    break;
  case S8_PT_PCC:
    _ruleExecution->status() = FAIL_S8_PCC;
    break;
  case S8_PT_CARRIER:
    _ruleExecution->status() = FAIL_S8_CARRIER;
    break;
  case S8_PT_SALES_DATE:
    _ruleExecution->status() = FAIL_S8_SALES_DATE;
    break;
  case S8_PT_TRAVEL_DATE:
    _ruleExecution->status() = FAIL_S8_TRAVEL_DATE;
    break;
  default:
    break;
  }
}

void
S8BrandingResponseParser::addMarketRuleInfo(std::vector<MarketRule*>& marketRuleVec,
                                            const std::string& condition,
                                            std::vector<MarketRuleInfo*>& marketRuleInfoVec)
{
  if (marketRuleVec.empty())
    return;

  StatusS8 status = PASS_S8;

  if (condition == "false")
    setPatternStatus(status);

  MarketRuleInfo* marketRuleInfo = nullptr;

  for (MarketRule* marketRule : marketRuleVec)
  {
    _trx.dataHandle().get(marketRuleInfo);
    marketRuleInfo->_marketRule = marketRule;
    marketRuleInfo->_status = status;
    marketRuleInfoVec.push_back(marketRuleInfo);
  }

  marketRuleVec.clear();
}

void
S8BrandingResponseParser::checkMarketFailure(const std::vector<MarketRuleInfo*>& marketRuleInfoVec,
                                             StatusS8 status)
{

  if (marketRuleInfoVec.empty())
    return;

  if (_ruleExecution->status() == PASS_S8)
  {
    for (const MarketRuleInfo* marketRuleInfo : marketRuleInfoVec)
    {
      if (marketRuleInfo->_status == PASS_S8)
        return;
    }
    _ruleExecution->status() = status;
  }
}

void
S8BrandingResponseParser::onEndBrandProgram()
{
  _brandProgramOpen = false;
  if (_brandProgramMap.find(std::make_pair(_brandProgram->programID(), _brandProgram->dataSource())) == _brandProgramMap.end())
  {
    _brandProgramMap.insert(ProgramIdMap::value_type(std::make_pair(_brandProgram->programID(), _brandProgram->dataSource()), _brandProgram));
  }
}

void
S8BrandingResponseParser::onEndAncillary()
{
  if (_brand == nullptr)
  {
    LOG4CXX_ERROR(logger,
                  "S8BrandingResponseParser::onEndAncillary _brand is NULL");
    return;
  }

  if (_featureItem == nullptr)
  {
    LOG4CXX_ERROR(logger,
                  "S8BrandingResponseParser::onEndAncillary _featureItem is NULL");
    return;
  }

  _featureItem->setCommercialName(std::string(_value.c_str(), _value.length()));

  _brand->getMutableFeatureItems().push_back(_featureItem);
  _featureItem = nullptr;
}

void
S8BrandingResponseParser::onEndMarketResponse()
{
  _marketResponseOpen = false;
  PricingTrx::BrandedMarketMap::iterator marketResponseMapBeg =
      _trx.brandedMarketMap().find(_marketResponse->getMarketID());
  if (marketResponseMapBeg == _trx.brandedMarketMap().end())
  {
    std::vector<MarketResponse*> marketResponseVector;
    marketResponseVector.push_back(_marketResponse);
    _trx.brandedMarketMap().insert(PricingTrx::BrandedMarketMap::value_type(
        _marketResponse->getMarketID(), marketResponseVector));
  }
  else
  {
    marketResponseMapBeg->second.push_back(_marketResponse);
  }
}

void
S8BrandingResponseParser::onEndGetAirlineBrandsRS()
{

  if (_trx.brandedMarketMap().empty())
  {
    return;
  }

  std::map<int, std::vector<MarketResponse*> >::const_iterator marketResponseMapBeg =
      _trx.brandedMarketMap().begin();

  for (; marketResponseMapBeg != _trx.brandedMarketMap().end(); marketResponseMapBeg++)
  {
    // LOG4CXX_DEBUG(logger, "S8BrandingResponseParser::onEndGetAirlineBrandsRS MarketID " <<
    // marketResponseMapBeg->first);
    std::vector<MarketResponse*> marketResponseVector = marketResponseMapBeg->second;

    std::vector<MarketResponse*>::const_iterator marketResponseBeg = marketResponseVector.begin();

    for (; marketResponseBeg != marketResponseVector.end(); marketResponseBeg++)
    {

      std::vector<ProgramID>::const_iterator programIDBeg =
          (*marketResponseBeg)->programIDList().begin();
      std::vector<ProgramID>::const_iterator programIDEnd =
          (*marketResponseBeg)->programIDList().end();

      for (; programIDBeg != programIDEnd; programIDBeg++)
      {
        // LOG4CXX_DEBUG(logger, "S8BrandingResponseParser::onEndGetAirlineBrandsRS programID " <<
        // (*programIDBeg));
        ProgramIdMap::const_iterator brandProgramBeg =
            _brandProgramMap.find(std::make_pair((*programIDBeg), (*marketResponseBeg)->dataSource()));

        if (brandProgramBeg != _brandProgramMap.end())
        {
          BrandProgram* program = brandProgramBeg->second;
          MarketCriteria* marketCriteria = (*marketResponseBeg)->marketCriteria();
          program->originsRequested().insert(marketCriteria->departureAirportCode());
          program->destinationsRequested().insert(marketCriteria->arrivalAirportCode());

          (*marketResponseBeg)->brandPrograms().push_back((*brandProgramBeg).second);
        }
      }
    }
  }
}

namespace
{

struct compareBrandProgramSequenceNumber
{

  bool operator()(const BrandProgram* brandProgram1, const BrandProgram* brandProgram2) const
  {
    if (!brandProgram1 || !brandProgram2)
      return false;

    return brandProgram1->sequenceNo() < brandProgram2->sequenceNo();
  }
};

class BrandProgramMatched : std::unary_function<const QualifiedBrand&, bool>
{
  const BrandProgram* _brp;
  const BrandInfo* _br;

public:
  BrandProgramMatched(QualifiedBrand prevBrandProgram)
    : _brp(prevBrandProgram.first), _br(prevBrandProgram.second)
  {
  }

  bool operator()(const QualifiedBrand& nextBrandProgram) const
  {

    if (nextBrandProgram.first->programID() == _brp->programID() &&
        nextBrandProgram.second->brandCode() == _br->brandCode())
    {
      return true;
    }

    return false;
  }
};
}

void
S8BrandingResponseParser::process(std::map<int, std::vector<FareMarket*> >& marketIDFareMarketMap)
    const
{

  if (_trx.brandedMarketMap().empty())
  {
    return;
  }

  std::map<int, std::vector<MarketResponse*> >::const_iterator marketResponseMapBeg =
      _trx.brandedMarketMap().begin();

  for (; marketResponseMapBeg != _trx.brandedMarketMap().end(); marketResponseMapBeg++)
  {
    std::vector<MarketResponse*> marketResponseVector = marketResponseMapBeg->second;

    std::vector<MarketResponse*>::const_iterator marketResponseBeg = marketResponseVector.begin();

    for (; marketResponseBeg != marketResponseVector.end(); marketResponseBeg++)
    {
      std::sort((*marketResponseBeg)->brandPrograms().begin(),
                (*marketResponseBeg)->brandPrograms().end(),
                compareBrandProgramSequenceNumber());
      checkAndPopulatePassengerType((*marketResponseBeg));
      populateFareIDdata((*marketResponseBeg));
      populateBrandProgramVector((*marketResponseBeg), marketIDFareMarketMap);
    }
  }
}

// This method populates vector of Pair of BrandProgram and Brand in PricingTrx
// Also this populates vector of BrandProgram index in FareMarket

void
S8BrandingResponseParser::populateBrandProgramVector(
    const MarketResponse* marketResponse,
    std::map<int, std::vector<FareMarket*> >& marketIDFareMarketMap) const
{
  for (BrandProgram* brandPr : marketResponse->brandPrograms())
  {
    if (brandPr == nullptr)
    {
      LOG4CXX_ERROR(logger,
                    "S8BrandingResponseParser::populateBrandProgramVector BrandProgram is NULL");
      continue;
    }

    for (BrandInfo* brand : brandPr->brandsData())
    {
      if (brand == nullptr)
      {
        LOG4CXX_ERROR(logger, "S8BrandingResponseParser::populateBrandProgramVector Brand is NULL");
        continue;
      }

      std::vector<QualifiedBrand>::iterator it =
          std::find_if(_trx.brandProgramVec().begin(),
                       _trx.brandProgramVec().end(),
                       BrandProgramMatched(std::make_pair(brandPr, brand)));

      // If we found this Pair of BrandProgram and Brand in PricingTrx then no need to insert this
      // pair in PricingTrx
      if (it != _trx.brandProgramVec().end())
      {
        populateFareMarketVector(marketResponse,
                                 marketIDFareMarketMap,
                                 std::distance(_trx.brandProgramVec().begin(), it));
        continue;
      }

      _trx.brandProgramVec().push_back(std::make_pair(brandPr, brand));
      populateFareMarketVector(
          marketResponse, marketIDFareMarketMap, _trx.brandProgramVec().size() - 1);
    }

    //remove the whole loop when removing fallback, get only data from first (not null) element
    if (!fallback::fallbackUseOnlyFirstProgram(&_trx))
      break; //break the loop after first program parsed
  }
}

// This method finds vector of FareMarket from Map of key as MarketID and value as vector of
// FareMarket
// And then calls populateFareMarket for each FareMarket from FareMarket vector

void
S8BrandingResponseParser::populateFareMarketVector(
    const MarketResponse* marketResponse,
    std::map<int, std::vector<FareMarket*> >& marketIDFareMarketMap,
    int index) const
{
  if (marketIDFareMarketMap.empty())
  {
    return;
  }

  std::map<int, std::vector<FareMarket*> >::iterator marketIDFareMarketMapBeg =
      marketIDFareMarketMap.find(marketResponse->getMarketID());
  if (marketIDFareMarketMapBeg == marketIDFareMarketMap.end())
  {
    return;
  }

  for (FareMarket* fareMarket : marketIDFareMarketMapBeg->second)
    populateFareMarket(fareMarket, index);
}

// This method populates vector of BrandProgram index in FareMarket

void
S8BrandingResponseParser::populateFareMarket(FareMarket* fareMarket, int index) const
{
  std::vector<int>::const_iterator it = std::find(
      fareMarket->brandProgramIndexVec().begin(), fareMarket->brandProgramIndexVec().end(), index);
  // If we found this index in vector of BrandProgram index in FareMarket then no need to insert
  // this index in FareMarket

  if (it != fareMarket->brandProgramIndexVec().end())
  {
    return;
  }

  fareMarket->brandProgramIndexVec().push_back(index);
}

void
S8BrandingResponseParser::checkAndPopulatePassengerType(const MarketResponse* marketResponse) const
{

  MarketResponse::CarrierBrandData brandPrograms = marketResponse->brandPrograms();

  MarketResponse::CarrierBrandData::const_iterator carrierBrandDataBeg = brandPrograms.begin();
  for (; carrierBrandDataBeg != brandPrograms.end(); carrierBrandDataBeg++)
    if ((*carrierBrandDataBeg)->passengerType().empty())
      (*carrierBrandDataBeg)->passengerType() = marketResponse->marketCriteria()->paxType();
}

void
S8BrandingResponseParser::populateFareIDdata(const MarketResponse* marketResponse) const
{

  MarketResponse::CarrierBrandData brandPrograms = marketResponse->brandPrograms();

  MarketResponse::CarrierBrandData::const_iterator carrierBrandDataBeg = brandPrograms.begin();
  for (; carrierBrandDataBeg != brandPrograms.end(); carrierBrandDataBeg++)
  {

    BrandProgram::BrandsData::const_iterator brandsDataBeg =
        (*carrierBrandDataBeg)->brandsData().begin();
    for (; brandsDataBeg != (*carrierBrandDataBeg)->brandsData().end(); brandsDataBeg++)
    {
      (*brandsDataBeg)->collectPrimaryFareIDdata(
          _trx, (*brandsDataBeg)->primaryFareIdTable(), (*carrierBrandDataBeg)->vendorCode());
      (*brandsDataBeg)->collectSecondaryFareIDdata(
          _trx, (*brandsDataBeg)->secondaryFareIdTable(), (*carrierBrandDataBeg)->vendorCode());
    }
  }
}

void
S8BrandingResponseParser::print() const
{
  if (_trx.brandedMarketMap().empty())
  {
    printBrandingResponse();
    return;
  }

  printBrandProgramMap();

  std::map<int, std::vector<MarketResponse*> >::const_iterator marketResponseMapBeg =
      _trx.brandedMarketMap().begin();
  for (; marketResponseMapBeg != _trx.brandedMarketMap().end(); marketResponseMapBeg++)
  {
    LOG4CXX_DEBUG(logger,
                  "S8BrandingResponseParser::print MarketID " << marketResponseMapBeg->first);
    std::vector<MarketResponse*> marketResponseVector = marketResponseMapBeg->second;

    std::vector<MarketResponse*>::const_iterator marketResponseBeg = marketResponseVector.begin();

    for (; marketResponseBeg != marketResponseVector.end(); marketResponseBeg++)
      printMarketResponse((*marketResponseBeg));
  }

  printBrandingResponse();
}

void
S8BrandingResponseParser::printBrandProgramMap() const
{
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrandProgramMap INSIDE printBrandProgramMap");

  if (_brandProgramMap.empty())
  {
    return;
  }

  ProgramIdMap::const_iterator brandProgramBeg = _brandProgramMap.begin();
  for (; brandProgramBeg != _brandProgramMap.end(); brandProgramBeg++)
  {
    LOG4CXX_DEBUG(logger,
                  "S8BrandingResponseParser::printBrandProgramMap programID "
                      << brandProgramBeg->first.first);
    BrandProgram* brandProgram = brandProgramBeg->second;
    printBrandProgram(brandProgram);
  }
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrandProgramMap OUTSIDE printBrandProgramMap");
}

void
S8BrandingResponseParser::printMarketResponse(const MarketResponse* marketResponse) const
{

  printProgramID(marketResponse->programIDList());

  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printMarketResponse carrier "
                    << marketResponse->carrier());

  printMarketCriteria(marketResponse->marketCriteria());

  MarketResponse::CarrierBrandData brandPrograms = marketResponse->brandPrograms();

  MarketResponse::CarrierBrandData::const_iterator carrierBrandDataBeg = brandPrograms.begin();

  for (; carrierBrandDataBeg != brandPrograms.end(); carrierBrandDataBeg++)
  {
    LOG4CXX_DEBUG(
        logger,
        "S8BrandingResponseParser::printMarketResponse CALLING printBrandProgram for marketID "
            << marketResponse->getMarketID());
    printBrandProgram((*carrierBrandDataBeg));
  }

  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printMarketResponse errMessage "
                    << marketResponse->errMessage());

  if (marketResponse->bsDiagnostics())
  {
    LOG4CXX_DEBUG(logger,
                  "DEBUG S8BrandingResponseParser::printMarketResponse ruleExecution size = "
                      << marketResponse->bsDiagnostics()->ruleExecution().size());
    for (const RuleExecution* ruleExecution : marketResponse->bsDiagnostics()->ruleExecution())
      printRuleExecution(ruleExecution);
  }

  printDataSource(marketResponse);
}

void
S8BrandingResponseParser::printProgramID(const std::vector<ProgramID>& programIDList) const
{
  std::vector<ProgramID>::const_iterator programIDBeg = programIDList.begin();

  for (; programIDBeg != programIDList.end(); programIDBeg++)
  {
    LOG4CXX_DEBUG(logger, "S8BrandingResponseParser::printProgramID programID " << (*programIDBeg));
  }
}

void
S8BrandingResponseParser::printMarketCriteria(const MarketCriteria* marketCriteria) const
{
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printMarketCriteria direction "
                    << marketCriteria->direction());

  std::string globalDirectionStr;
  bool rc = globalDirectionToStr(globalDirectionStr, marketCriteria->globalDirection());
  if (rc)
  {
    LOG4CXX_DEBUG(logger,
                  "S8BrandingResponseParser::printMarketCriteria globalDirectionStr "
                      << globalDirectionStr);
  }

  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printMarketCriteria departureDate "
                    << marketCriteria->departureDate().dateToString(YYYYMMDD, "-").c_str());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printMarketCriteria departureAirportCode "
                    << marketCriteria->departureAirportCode());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printMarketCriteria arrivalAirportCode "
                    << marketCriteria->arrivalAirportCode());

  std::vector<PaxTypeCode> paxTypeCode = marketCriteria->paxType();

  std::vector<PaxTypeCode>::const_iterator paxTypeCodeBeg = paxTypeCode.begin();
  for (; paxTypeCodeBeg != paxTypeCode.end(); paxTypeCodeBeg++)
    LOG4CXX_DEBUG(logger,
                  "S8BrandingResponseParser::printMarketCriteria paxTypeCode "
                      << (*paxTypeCodeBeg).c_str());
}

void
S8BrandingResponseParser::printBrandProgram(const BrandProgram* brandProgram) const
{
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrandProgram programID "
                    << brandProgram->programID());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrandProgram programCode "
                    << brandProgram->programCode());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrandProgram programName "
                    << brandProgram->programName());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrandProgram programDescription "
                    << brandProgram->programDescription());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrandProgram vendorCode "
                    << brandProgram->vendorCode());

  std::vector<PaxTypeCode>::const_iterator paxTypeCodeBeg = brandProgram->passengerType().begin();
  for (; paxTypeCodeBeg != brandProgram->passengerType().end(); paxTypeCodeBeg++)
  {
    LOG4CXX_DEBUG(logger,
                  "S8BrandingResponseParser::printBrandProgram passengerType "
                      << (*paxTypeCodeBeg).c_str());
  }

  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrandProgram direction "
                    << brandProgram->direction());

  std::string globalDirectionStr;
  bool rc = globalDirectionToStr(globalDirectionStr, brandProgram->globalDirection());
  if (rc)
  {
    LOG4CXX_DEBUG(logger,
                  "S8BrandingResponseParser::printBrandProgram globalDirectionStr "
                      << globalDirectionStr);
  }

  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrandProgram sequenceNo "
                    << brandProgram->sequenceNo());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrandProgram effectiveDate "
                    << brandProgram->effectiveDate().dateToString(YYYYMMDD, "-").c_str());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrandProgram discontinueDate "
                    << brandProgram->discontinueDate().dateToString(YYYYMMDD, "-").c_str());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrandProgram originLoc "
                    << brandProgram->originLoc());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrandProgram systemCode "
                    << brandProgram->systemCode());
  BrandProgram::AccountCodes accountCodes = brandProgram->accountCodes();
  std::vector<std::string>::const_iterator accountCodesBeg = accountCodes.begin();
  for (; accountCodesBeg != accountCodes.end(); accountCodesBeg++)
  {
    LOG4CXX_DEBUG(logger,
                  "S8BrandingResponseParser::printBrandProgram accountCodes "
                      << (*accountCodesBeg).c_str());
  }

  BrandProgram::BrandsData brandsData = brandProgram->brandsData();
  BrandProgram::BrandsData::const_iterator brandsDataBeg = brandsData.begin();

  for (; brandsDataBeg != brandsData.end(); brandsDataBeg++)
    printBrand((*brandsDataBeg));
}

void
S8BrandingResponseParser::printBrand(const BrandInfo* brand) const
{
  LOG4CXX_DEBUG(logger, "S8BrandingResponseParser::printBrand brandCode " << brand->brandCode());
  LOG4CXX_DEBUG(logger, "S8BrandingResponseParser::printBrand brandName " << brand->brandName());
  LOG4CXX_DEBUG(logger, "S8BrandingResponseParser::printBrand tier " << brand->tier());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrand primaryFareIdTable "
                    << brand->primaryFareIdTable());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrand secondaryFareIdTable "
                    << brand->secondaryFareIdTable());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrand carrierFlightItemNum "
                    << brand->getCarrierFlightItemNum());

  printCbasBrand(brand);

}

void
S8BrandingResponseParser::printBrandingResponse() const
{
  if (_brandingResponse == nullptr || _brandingResponse->messageText().empty())
    return;

  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrandingResponse messageCode "
                    << _brandingResponse->messageCode());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrandingResponse failCode "
                    << _brandingResponse->failCode());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printBrandingResponse messageText "
                    << _brandingResponse->messageText());
}

bool
S8BrandingResponseParser::isError() const
{
  return _brandingResponse && _brandingResponse->messageCode() == "Error" &&
         !_brandingResponse->messageText().empty();
}

void
S8BrandingResponseParser::printMarketRule(const MarketRule* marketRule) const
{
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printMarketRule originLoc " << marketRule->originLoc());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printMarketRule originLocType "
                    << marketRule->originLocType());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printMarketRule destinationLoc "
                    << marketRule->destinationLoc());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printMarketRule destinationLocType "
                    << marketRule->destinationLocType());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printMarketRule direction " << marketRule->direction());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printMarketRule globalDirection "
                    << marketRule->globalDirection());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printMarketRule carrier " << marketRule->carrier());
}

void
S8BrandingResponseParser::printRuleExecution(const RuleExecution* ruleExecution) const
{
#ifndef DEBUG_LOG_DISABLED
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printRuleExecution programCode "
                    << ruleExecution->programCode());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printRuleExecution ruleID " << ruleExecution->ruleID());
  for (const PatternInfo* patternInfo : ruleExecution->passengerType())
    LOG4CXX_DEBUG(logger,
                  "S8BrandingResponseParser::printRuleExecution passengerType "
                      << patternInfo->_patternValue);
  for (const PatternInfo* patternInfo : ruleExecution->accountCodes())
    LOG4CXX_DEBUG(logger,
                  "S8BrandingResponseParser::printRuleExecution accountCode "
                      << patternInfo->_patternValue);
  for (const MarketRuleInfo* marketRuleInfo : ruleExecution->marketRules())
    printMarketRule(marketRuleInfo->_marketRule);
  for (const PatternInfo* patternInfo : ruleExecution->pseudoCityCodes())
    LOG4CXX_DEBUG(logger,
                  "S8BrandingResponseParser::printRuleExecution pseudoCityCode "
                      << patternInfo->_patternValue);
  for (const PatternInfo* patternInfo : ruleExecution->governingCarrier())
    LOG4CXX_DEBUG(logger,
                  "S8BrandingResponseParser::printRuleExecution governingCarrier "
                      << patternInfo->_patternValue);
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printRuleExecution salesDateStart "
                    << ruleExecution->salesDateStart().dateToString(YYYYMMDD, "-").c_str());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printRuleExecution salesDateEnd "
                    << ruleExecution->salesDateEnd().dateToString(YYYYMMDD, "-").c_str());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printRuleExecution travelDateStart "
                    << ruleExecution->travelDateStart().dateToString(YYYYMMDD, "-").c_str());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printRuleExecution travelDateEnd "
                    << ruleExecution->travelDateEnd().dateToString(YYYYMMDD, "-").c_str());
  if (ruleExecution->s8BrandingSecurity())
    printS8BrandingSecurity(ruleExecution->s8BrandingSecurity());
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printRuleExecution status " << ruleExecution->status());
#endif
}

void
S8BrandingResponseParser::printS8BrandingSecurity(const S8BrandingSecurity* s8BrandingSecurity)
    const
{
  std::string statusStr;
  for (const SecurityInfo* securityInfo : s8BrandingSecurity->securityInfoVec())
  {
    getStatusStr(securityInfo->_status, statusStr);
    LOG4CXX_DEBUG(logger,
                  "S8BrandingResponseParser::printS8BrandingSecurity SecurityInfo _securityName = "
                      << securityInfo->_securityName << ", _securityValue = "
                      << securityInfo->_securityValue << ", _status = " << statusStr);
  }
}

void
S8BrandingResponseParser::getStatusStr(StatusS8 status, std::string& statusStr) const
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

void
S8BrandingResponseParser::parseCode()
{
  if(_bookingCodeListOpen)
  {
    _brand->primaryBookingCode().push_back(BookingCode(_value.c_str(), _value.length()));
  }
  else if(_secondaryBookingCodeListOpen)
  {
    _brand->secondaryBookingCode().push_back(BookingCode(_value.c_str(), _value.length()));
  }
  else
  {
    _brand->brandCode().assign(_value.c_str(), _value.length());
  }
}

void
S8BrandingResponseParser::parseFareBasisCode()
{
  if(_includeFareBasisCode)
    _brand->includedFareBasisCode().push_back(FareBasisCode(_value.c_str(), _value.length()));
  else
    _brand->excludedFareBasisCode().push_back(FareBasisCode(_value.c_str(), _value.length()));
}

void
S8BrandingResponseParser::startElementCbas(int idx, const IAttributes& attrs)
{
  switch (idx)
  {
  case _ProgramIdList:
    {
      std::string dataSourceValue;
      attrs.get(_dataSource, dataSourceValue);
      if (dataSourceValue == "CBS")
        _marketResponse->dataSource() = BRAND_SOURCE_CBAS;
      else
        _marketResponse->dataSource() = BRAND_SOURCE_S8;
    }
    break;
  case _FareBasisCode:
    {
      std::string includeIndValue;
      attrs.get(_includeInd, includeIndValue);
      if (includeIndValue == "true" || includeIndValue == "Y" || includeIndValue == "T")
        _includeFareBasisCode = true;
      else
        _includeFareBasisCode = false;
    }
    break;
  case _BookingCodeList:
    _bookingCodeListOpen = true;
    break;
  case _SecondaryBookingCodeList:
    _secondaryBookingCodeListOpen = true;
    break;
  }

}

void
S8BrandingResponseParser::endElementCbas(int idx)
{
  switch (idx)
  {
  case _Text:
    _brand->brandText().assign(_value.c_str(), _value.length());
    break;
  case _FareBasisCode:
    parseFareBasisCode();
    break;
  case _BookingCodeList:
    _bookingCodeListOpen = false;
    break;
  case _SecondaryBookingCodeList:
    _secondaryBookingCodeListOpen = false;
    break;
  }

}

void
S8BrandingResponseParser::printCbasBrand(const BrandInfo* brand) const
{
#ifndef DEBUG_LOG_DISABLED
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printCbasBrand brandText " << brand->brandText());
  for (const FareBasisCode& includedFareBasisCode : brand->includedFareBasisCode())
    LOG4CXX_DEBUG(logger,
                  "S8BrandingResponseParser::printCbasBrand includedFareBasisCode "
                      << includedFareBasisCode);
  for (const FareBasisCode& excludedFareBasisCode : brand->excludedFareBasisCode())
    LOG4CXX_DEBUG(logger,
                  "S8BrandingResponseParser::printCbasBrand excludedFareBasisCode "
                      << excludedFareBasisCode);
  for (const BookingCode& primaryBookingCode : brand->primaryBookingCode())
    LOG4CXX_DEBUG(logger,
                  "S8BrandingResponseParser::printCbasBrand primaryBookingCode "
                      << primaryBookingCode);
  for (const BookingCode& secondaryBookingCode : brand->secondaryBookingCode())
    LOG4CXX_DEBUG(logger,
                  "S8BrandingResponseParser::printCbasBrand secondaryBookingCode "
                      << secondaryBookingCode);
#endif
}

void
S8BrandingResponseParser::printDataSource(const MarketResponse* marketResponse) const
{
  LOG4CXX_DEBUG(logger,
                "S8BrandingResponseParser::printDataSource dataSource "
                    << marketResponse->dataSource());
}
} // tse
