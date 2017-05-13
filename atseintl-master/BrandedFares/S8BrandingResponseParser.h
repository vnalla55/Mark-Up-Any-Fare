//-------------------------------------------------------------------
//
//  File:        S8BrandingResponseParser.h
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

#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Xform/CustomXMLParser/IAttributes.h"
#include "Xform/CustomXMLParser/IBaseHandler.h"

namespace tse
{

class PricingTrx;
class MarketResponse;
class MarketCriteria;
class BrandProgram;
class BrandInfo;
class BrandFeatureItem;
class RequestSource;
class BrandingResponse;
class RuleExecution;
class MarketRule;
class FareMarket;
class S8BrandingSecurity;
class SecurityInfo;

struct PatternInfo
{
  PatternInfo() : _status(PASS_S8) {}

  std::string _patternValue;
  StatusS8 _status;
};

struct MarketRuleInfo
{
  MarketRuleInfo() : _marketRule(nullptr), _status(PASS_S8) {}

  MarketRule* _marketRule;
  StatusS8 _status;
};

class S8BrandingResponseParser : public IBaseHandler
{

  friend class S8BrandingResponseParserTest;
  using BrandProgramId = std::pair<std::string, BrandSource>;

  class BrandProgramIdComparator
  {
  public:
    bool operator()(const BrandProgramId& element1, const BrandProgramId& element2) const
    {
      if(element1.first != element2.first)
        return element1.first < element2.first;
      return element1.second < element2.second;
    }
  };
  using ProgramIdMap = std::map<BrandProgramId, BrandProgram*, BrandProgramIdComparator>;

public:
  enum S8PatternType
  { S8_PT_NONE = 0,
    S8_PT_PASSENGER_TYPE,
    S8_PT_ACCOUNT_CODE,
    S8_PT_MARKET,
    S8_PT_PCC,
    S8_PT_CARRIER,
    S8_PT_SALES_DATE,
    S8_PT_TRAVEL_DATE };

  enum BSSecurityType
  { CARRIER_GDS = 0,
    DEPT_CODE,
    OFFICE_DESIG,
    MARKET_LOC,
    IATA_NUM,
    VIEW_BOOK,
    NUM_BS_SECURITY_TYPE };

  enum BSSecurityStatus
  { SS_NONE = -1,
    SS_FAIL,
    SS_PASS };

  S8BrandingResponseParser(PricingTrx& trx) : _trx(trx) {}

  void parse(const std::string& content);
  void process(std::map<int, std::vector<FareMarket*> >& marketIDFareMarketMap) const;
  void print() const;
  const RequestSource* requestSource() const { return _requestSource; }
  const BrandingResponse* brandingResponse() const { return _brandingResponse; }
  bool isError() const;

private:
  virtual bool startElement(const IKeyString&, const IAttributes&) override;
  virtual bool endElement(const IKeyString&) override;
  void checkPatternFailure();
  void checkSecurityFailure();
  void populateIataNumbersSecurity(const std::string& condition);
  void addPatternInfo(std::vector<std::string>& patternValueVec,
                      const std::string& condition,
                      std::vector<PatternInfo*>& ruleExecutionPatternVec);
  void setPatternStatus(StatusS8& status);
  void checkFailure(const std::vector<PatternInfo*>& patternValueVec, StatusS8 status);

  StatusS8 getSecurityStatus();
  void setStatus();
  void addMarketRuleInfo(std::vector<MarketRule*>& marketRuleVec,
                         const std::string& condition,
                         std::vector<MarketRuleInfo*>& marketRuleInfoVec);
  void checkMarketFailure(const std::vector<MarketRuleInfo*>& marketRuleInfoVec, StatusS8 status);
  virtual void characters(const char* value, size_t length) override;
  virtual bool startElement(int idx, const IAttributes& attrs) override;
  virtual bool endElement(int idx) override;

  void onStartResponseSource(const IAttributes& attrs);
  void onStartMarketCriteria(const IAttributes& attrs);
  void onStartBrandingResponse(const IAttributes& attrs);
  void onStartAncillary(const IAttributes& attrs);
  void onStartCondition(const IAttributes& attrs);
  void parseMarketMatch(const std::string& patternValue);
  void parseDateRange(const std::string& patternValue);
  void parsePassengerType(const std::string& patternValue);
  void parseAccountCodes(const std::string& patternValue);
  void parsePCC(const std::string& patternValue);
  void parseMarketLocation(const std::string& patternValue);
  void parseIataNumbers(const std::string& patternValue);
  void onEndBrandProgram();
  void onEndMarketResponse();
  void onEndAncillary();
  void onEndGetAirlineBrandsRS();
  void
  populateBrandProgramVector(const MarketResponse* marketResponse,
                             std::map<int, std::vector<FareMarket*> >& marketIDFareMarketMap) const;
  void populateFareMarketVector(const MarketResponse* marketResponse,
                                std::map<int, std::vector<FareMarket*> >& marketIDFareMarketMap,
                                int index) const;
  void populateFareMarket(FareMarket* fareMarket, int index) const;
  void checkAndPopulatePassengerType(const MarketResponse* marketResponse) const;
  void populateFareIDdata(const MarketResponse* marketResponse) const;
  void printBrandProgramMap() const;
  void printMarketResponse(const MarketResponse* marketResponse) const;
  void printProgramID(const std::vector<ProgramID>& programIDList) const;
  void printMarketCriteria(const MarketCriteria* marketCriteria) const;
  void printBrandProgram(const BrandProgram* brandProgram) const;
  void printBrand(const BrandInfo* brand) const;
  void printBrandingResponse() const;
  void printMarketRule(const MarketRule* marketRule) const;
  void printRuleExecution(const RuleExecution* ruleExecution) const;
  void printS8BrandingSecurity(const S8BrandingSecurity* s8BrandingSecurity) const;
  void getStatusStr(StatusS8 status, std::string& statusStr) const;
  void processPassSecurity(const SecurityInfo* securityInfo);
  void processFailSecurity(const SecurityInfo* securityInfo);
  void parseCode();
  void parseFareBasisCode();
  void startElementCbas(int idx, const IAttributes& attrs);
  void endElementCbas(int idx);
  void printCbasBrand(const BrandInfo* brand) const;
  void printDataSource(const MarketResponse* marketResponse) const;

  PricingTrx& _trx;
  IValueString _value;
  MarketResponse* _marketResponse = nullptr;
  BrandProgram* _brandProgram = nullptr;
  BrandInfo* _brand = nullptr;
  BrandFeatureItem* _featureItem = nullptr;
  RequestSource* _requestSource = nullptr;
  BrandingResponse* _brandingResponse = nullptr;
  bool _marketResponseOpen = false;
  bool _ruleExecutionOpen = false;
  bool _brandProgramOpen = false;
  RuleExecution* _ruleExecution = nullptr;
  std::string _securityName;
  std::string _securityValue;
  std::vector<std::string> _iataNumbers;
  std::vector<std::string> _accountCodeVec;
  std::vector<std::string> _pseudoCityCodeVec;
  std::vector<std::string> _passengerTypeVec;
  std::vector<std::string> _governingCarrierVec;
  S8PatternType _s8PatternType = S8PatternType::S8_PT_NONE;
  BSSecurityStatus _securityStatus[NUM_BS_SECURITY_TYPE];
  ProgramIdMap _brandProgramMap;
  MarketRule* _marketRule = nullptr;
  std::vector<MarketRule*> _marketRuleVec;
  bool _bookingCodeListOpen = false;
  bool _secondaryBookingCodeListOpen = false;
  bool _includeFareBasisCode = true;
};
} // tse
