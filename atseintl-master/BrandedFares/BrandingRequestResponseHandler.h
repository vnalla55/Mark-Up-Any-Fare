//-------------------------------------------------------------------
//
//  File:        BrandingRequestResponseHandler.h
//  Created:     March 2013
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

#include "BrandedFares/MarketRequest.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/XMLConstruct.h"

#include <boost/noncopyable.hpp>

namespace tse
{
class PricingTrx;
class FareMarket;
class RequestSource;
class MarketRequest;
class BrandingCriteria;
class BrandedFare;
class DateTime;
class Diag888Collector;
class Diag890Collector;
class Diag891Collector;
class Diag898Collector;
class S8BrandingResponseParser;

namespace xray
{
class JsonMessage;
}

class BrandingRequestResponseHandler final : boost::noncopyable
{
  friend class BrandingRequestResponseHandlerTest;

public:
  // Possible client IDs: PBB, AVL, FQ, SHP, MKT
  BrandingRequestResponseHandler(PricingTrx& trx) : _trx(trx) {}

  bool getBrandedFares();

  int buildMarketRequest(const LocCode& depAirportCode,
                         const LocCode& arrivalAirportCode,
                         const DateTime& travelDate,
                         const std::vector<PaxTypeCode>& paxTypes,
                         const MarketRequest::CarrierList& carriers,
                         GlobalDirection gd,
                         AlphaCode direction);

  int buildMarketRequest(const LocCode& depAirportCode,
                         const LocCode& arrivalAirportCode,
                         const DateTime& travelDate,
                         const std::vector<PaxTypeCode>& paxTypes,
                         const MarketRequest::CarrierList& carriers,
                         const std::vector<FareMarket*>& fareMarketVec,
                         GlobalDirection gd,
                         AlphaCode direction);

  void brandedFaresDiagnostic888();
  const std::string& xmlData() const { return _xmlData; }
  bool& getOnlyXmlData() { return _getOnlyXmlData; }
  const bool& getOnlyXmlData() const { return _getOnlyXmlData; }
  StatusBrandingService getStatusBrandingService() const { return _status; }
  void setStatusBrandingService(StatusBrandingService status) { _status = status; }

  // accessors
  RequestSource*& requestSource() { return _requestSource; }
  const RequestSource* requestSource() const { return _requestSource; }

  BrandingCriteria*& brandingCriteria() { return _brandingCriteria; }
  const BrandingCriteria* brandingCriteria() const { return _brandingCriteria; }

  std::map<int, std::vector<FareMarket*> >& marketIDFareMarketMap()
  {
    return _marketIDFareMarketMap;
  }
  const std::map<int, std::vector<FareMarket*> >& marketIDFareMarketMap() const
  {
    return _marketIDFareMarketMap;
  }

  void setClientId(const ClientID& id) { _clientID = id; }
  void setActionCode(const ActionCode& action) { _action = action; }

private:
  void createRequest(XMLConstruct& construct);
  void buildRequestSource(XMLConstruct& construct);
  void addMarketRequestInfo(XMLConstruct& construct, const MarketRequest* marketReq);
  void addPassengerTypeInfo(XMLConstruct& construct, const std::vector<PaxTypeCode>& paxTypes);
  void addCarrierInfo(XMLConstruct& construct, const MarketRequest::CarrierList& carrierList);
  void addAccountCodeInfo(XMLConstruct& construct, const std::vector<std::string>& accountCodes);
  void addXray(XMLConstruct& construct, xray::JsonMessage* jsonMessage);

  void printS8BrandedFaresData(FareMarket& fareMarket);
  const std::vector<BrandedFare*>&
  getBrandedFaresData(const VendorCode& vn, const CarrierCode& cxr);

  bool createDiag();
  bool createDiag890();
  bool createDiag891();
  bool createDiag898();
  bool isError(const S8BrandingResponseParser& s8BrandingResponseParser);
  void populateFareMarketMarketID(const std::vector<FareMarket*>& fareMarketVec, int marketID);

  void printBrandedDataError();

  void printBrandedMarketMap();
  std::string callBrandingService(std::string& request);

protected:
  PricingTrx& _trx;
  RequestSource* _requestSource = nullptr;
  BrandingCriteria* _brandingCriteria = nullptr;
  ClientID _clientID;
  ActionCode _action;
  std::string _xmlData;
  int _marketID = 0;
  bool _getOnlyXmlData = false;
  Diag888Collector* _diag888 = nullptr;
  Diag890Collector* _diag890 = nullptr;
  Diag891Collector* _diag891 = nullptr;
  Diag898Collector* _diag898 = nullptr;
  std::map<int, std::vector<FareMarket*> > _marketIDFareMarketMap;
  StatusBrandingService _status = StatusBrandingService::NO_BS_ERROR;
};
} // tse
