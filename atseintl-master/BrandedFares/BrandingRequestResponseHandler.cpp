//-------------------------------------------------------------------
//
//  File:        BrandingRequestResponseHandler.cpp
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

#include "BrandedFares/BrandingRequestResponseHandler.h"

#include "BrandedFares/BrandingCriteria.h"
#include "BrandedFares/BrandingResponse.h"
#include "BrandedFares/MarketCriteria.h"
#include "BrandedFares/MarketRequest.h"
#include "BrandedFares/S8BrandingResponseParser.h"
#include "BrandingService/BrandingServiceCaller.h"
#include "Common/ConfigurableBrandingServiceCaller.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/BrandedFare.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag888Collector.h"
#include "Diagnostic/Diag890Collector.h"
#include "Diagnostic/Diag891Collector.h"
#include "Diagnostic/Diag893Collector.h"
#include "Diagnostic/Diag898Collector.h"


#define XML_NAMESPACE "http://stl.sabre.com/Merchandising/v1"
#define XML_NAMESPACE_XS "http://www.w3.org/2001/XMLSchema-instance"
#define XML_NAMESPACE_SCHEMA_LOCATION                                                              \
  "http://stl.sabre.com/Merchandising/v1 ../AirlineBranding_1_0_0.xsd"
#define ANCS_XML_NAMESPACE_SCHEMA_LOCATION "http://stl.sabre.com/Merchandising/v0"
#define BRANDINGREQUEST_VERSION "01"
#define GETAIRLINEBRANDSRQ_VERSION "1.0.0"

namespace tse
{
FALLBACK_DECL(fallbackBrandedServiceInterface);
FALLBACK_DECL(fallbackDebugOverrideBrandingServiceResponses);
FALLBACK_DECL(reworkTrxAborter);

static Logger
logger("atseintl.BrandedFares.BrandingRequestResponseHandler");

bool
BrandingRequestResponseHandler::getBrandedFares()
{
  _xmlData = EMPTY_STRING();

  createDiag();

  if (!_brandingCriteria)
  {
    printBrandedDataError();
    return false;
  }

  if (_trx.getRequest()->isMultiAccCorpId())
  {
    _brandingCriteria->accountCodes().reserve(_trx.getRequest()->corpIdVec().size() +
                                              _trx.getRequest()->accCodeVec().size());
    _brandingCriteria->accountCodes().insert(_brandingCriteria->accountCodes().end(),
                                             _trx.getRequest()->corpIdVec().begin(),
                                             _trx.getRequest()->corpIdVec().end());
    _brandingCriteria->accountCodes().insert(_brandingCriteria->accountCodes().end(),
                                             _trx.getRequest()->accCodeVec().begin(),
                                             _trx.getRequest()->accCodeVec().end());
  }
  else
  {
    if (!_trx.getRequest()->corporateID().empty())
    {
      _brandingCriteria->accountCodes().push_back(_trx.getRequest()->corporateID());
    }

    if (!_trx.getRequest()->accountCode().empty())
    {
      _brandingCriteria->accountCodes().push_back(_trx.getRequest()->accountCode());
    }
  }

  _brandingCriteria->salesDate() = _trx.ticketingDate();
  XMLConstruct construct;

  createRequest(construct);
  _xmlData = construct.getXMLData();

  LOG4CXX_DEBUG(logger, "BrandingRequestResponseHandler::getBrandedFares _xmlData " << _xmlData);

  if (_getOnlyXmlData)
  {
    printBrandedDataError();
    return true;
  }

  std::string response = callBrandingService(_xmlData);

  LOG4CXX_DEBUG(logger, "BrandingRequestResponseHandler::getBrandedFares response " << response);

  if (response.empty())
  {
    printBrandedDataError();
    return false;
  }

  S8BrandingResponseParser s8BrandingResponseParser(_trx);
  try
  {
    s8BrandingResponseParser.parse(response);
  }
  catch(...)
  {
    _status = StatusBrandingService::BS_INVALID_RESPONSE;
    printBrandedDataError();
    return false;
  }
  if (isError(s8BrandingResponseParser))
  {
    printBrandedDataError();
    return false;
  }
  s8BrandingResponseParser.process(_marketIDFareMarketMap);
  s8BrandingResponseParser.print();

  if (_diag898)
  {
    _diag898->printDiagnostic(_marketIDFareMarketMap);
    _diag898->flushMsg();
  }

  printBrandedMarketMap();

  _marketIDFareMarketMap.clear();
  return true;
}

void
BrandingRequestResponseHandler::printBrandedMarketMap()
{
  if (_trx.getRequest()->diagnosticNumber() == Diagnostic893)
  {
    if (!_trx.diagnostic().isActive())
      return;

    if (_trx.diagnostic().diagnosticType() == Diagnostic893)
    {
      Diag893Collector* diag893 = dynamic_cast<Diag893Collector*>(DCFactory::instance()->create(_trx));
      if (diag893 == nullptr)
        return;

      diag893->enable(Diagnostic893);
      diag893->activate();

      *diag893 << _trx.brandedMarketMap();

      diag893->flushMsg();
    }
  }
}

void
BrandingRequestResponseHandler::printBrandedDataError()
{
  if (_diag898)
  {
    _diag898->printBrandedDataError();
    _diag898->flushMsg();
  }

  if (_diag891 && _status == StatusBrandingService::BS_INVALID_RESPONSE)
  {
    std::string msg;
    getBrandingServiceErrorMessage(_status, msg);
    *_diag891 << msg << "\n";
    _diag891->flushMsg();
  }
}

int
BrandingRequestResponseHandler::buildMarketRequest(const LocCode& depAirportCode,
                                                   const LocCode& arrivalAirportCode,
                                                   const DateTime& travelDate,
                                                   const std::vector<PaxTypeCode>& paxTypes,
                                                   const MarketRequest::CarrierList& carriers,
                                                   GlobalDirection gd,
                                                   AlphaCode direction)
{
  if (_brandingCriteria == nullptr)
  {
    _trx.dataHandle().get(_brandingCriteria);
  }
  MarketRequest* marketRequest = nullptr;
  _trx.dataHandle().get(marketRequest);
  marketRequest->setMarketID() = ++_marketID;

  _trx.dataHandle().get(marketRequest->marketCriteria());
  marketRequest->marketCriteria()->direction() = direction;
  marketRequest->marketCriteria()->globalDirection() = gd;
  marketRequest->marketCriteria()->departureDate() = travelDate;
  marketRequest->marketCriteria()->departureAirportCode() = depAirportCode;
  marketRequest->marketCriteria()->arrivalAirportCode() = arrivalAirportCode;
  marketRequest->marketCriteria()->paxType() = paxTypes;
  marketRequest->carrriers() = carriers;
  _brandingCriteria->marketRequest().push_back(marketRequest);
  return _marketID;
}

int
BrandingRequestResponseHandler::buildMarketRequest(const LocCode& depAirportCode,
                                                   const LocCode& arrivalAirportCode,
                                                   const DateTime& travelDate,
                                                   const std::vector<PaxTypeCode>& paxTypes,
                                                   const MarketRequest::CarrierList& carriers,
                                                   const std::vector<FareMarket*>& fareMarketVec,
                                                   GlobalDirection gd,
                                                   AlphaCode direction)
{

  int marketID = buildMarketRequest(
      depAirportCode, arrivalAirportCode, travelDate, paxTypes, carriers, gd, direction);

  _marketIDFareMarketMap.insert(std::make_pair(marketID, fareMarketVec));

  populateFareMarketMarketID(fareMarketVec, marketID);

  return marketID;
}

void
BrandingRequestResponseHandler::createRequest(XMLConstruct& construct)
{

  construct.addSpecialElement("xml version=\"1.0\" encoding=\"UTF-8\"");
  construct.openElement("GetAirlineBrandsRQ");
  construct.addAlsoEmptyAttribute("xmlns", XML_NAMESPACE);
  construct.addAlsoEmptyAttribute("version", GETAIRLINEBRANDSRQ_VERSION);
  construct.addAlsoEmptyAttribute("xmlns:ANCS", ANCS_XML_NAMESPACE_SCHEMA_LOCATION);
  construct.addAlsoEmptyAttribute("xmlns:xsi", XML_NAMESPACE_XS);
  construct.addAlsoEmptyAttribute("xsi:schemaLocation", XML_NAMESPACE_SCHEMA_LOCATION);
  construct.openElement("BrandingRequest");

  if (fallback::fallbackBrandedServiceInterface(&_trx))
    construct.addAlsoEmptyAttribute("version", BRANDINGREQUEST_VERSION);

  buildRequestSource(construct);

  addXray(construct, _trx.getXrayJsonMessage());

  construct.openElement("BrandingCriteria");
  std::vector<MarketRequest*>::const_iterator marketReqBeg =
      _brandingCriteria->marketRequest().begin();
  std::vector<MarketRequest*>::const_iterator marketReqEnd =
      _brandingCriteria->marketRequest().end();
  for (; marketReqBeg != marketReqEnd; marketReqBeg++)
    addMarketRequestInfo(construct, (*marketReqBeg));

  construct.openElement("AccountCodeList");
  addAccountCodeInfo(construct, _brandingCriteria->accountCodes());
  construct.closeElement(); // AccountCodeList

  construct.openElement("SalesDate");
  std::string salesDateTime = _brandingCriteria->salesDate().dateToString(YYYYMMDD, "-");
  salesDateTime += "T";
  salesDateTime += _brandingCriteria->salesDate().timeToString(HHMMSS, ":");
  construct.addElementData(salesDateTime.c_str());
  construct.closeElement(); // SalesDate

  construct.closeElement(); // BrandingCriteria
  construct.closeElement(); // BrandingRequest
  construct.closeElement(); // GetAirlineBrandsRQ
}

void
BrandingRequestResponseHandler::buildRequestSource(XMLConstruct& construct)
{
  std::string requestType;
  if (_trx.getRequest()->diagnosticNumber() == Diagnostic898)
  {
    requestType = "D";
  }

  construct.openElement("RequestSource");
  construct.addAlsoEmptyAttribute("dutyCode", _trx.getRequest()->ticketingAgent()->agentDuty());

  if (!_trx.getRequest()->ticketingAgent()->agentFunctions().empty())
  {
    std::string functionCode(1, _trx.getRequest()->ticketingAgent()->agentFunctions().at(0));
    construct.addAlsoEmptyAttribute("functionCode", functionCode);
  }

  construct.addAlsoEmptyAttribute("geoLocation", _trx.getRequest()->ticketingAgent()->agentCity());
  construct.addAlsoEmptyAttribute("clientID", _clientID);

  if (!fallback::fallbackBrandedServiceInterface(&_trx))
    construct.addAlsoEmptyAttribute("action", _action);

  construct.addAttribute("iataNumber", _trx.getRequest()->ticketingAgent()->tvlAgencyIATA());
  construct.addAttribute("departmentCode", _trx.getRequest()->ticketingAgent()->airlineDept());
  construct.addAttribute("officeDesignator",
                         _trx.getRequest()->ticketingAgent()->officeDesignator());
  construct.addAttribute("pseudoCityCode", _trx.getRequest()->ticketingAgent()->tvlAgencyPCC());

  construct.addAttribute("requestType", requestType);
  if (_trx.getRequest()->ticketingAgent()->agentTJR() == nullptr)
  {
    construct.addAlsoEmptyAttribute("requestingCarrierGDS", _trx.billing()->partitionID());
  }
  else
  {
    construct.addAlsoEmptyAttribute("requestingCarrierGDS",
                                    _trx.getRequest()->ticketingAgent()->cxrCode());
  }
  construct.closeElement(); // RequestSource
}

void
BrandingRequestResponseHandler::addMarketRequestInfo(XMLConstruct& construct,
                                                     const MarketRequest* marketReq)
{

  construct.openElement("MarketRequest");
  construct.addAttributeInteger("marketID", marketReq->getMarketID());
  construct.openElement("MarketCriteria");
  construct.addAttribute("direction", marketReq->marketCriteria()->direction());
  std::string globalDirectionStr;
  bool rc =
      globalDirectionToStr(globalDirectionStr, marketReq->marketCriteria()->globalDirection());

  if (rc)
  {
    if (globalDirectionStr == "XX")
      globalDirectionStr = "";
    construct.addAttribute("globalIndicator", globalDirectionStr);
  }

  construct.openElement("DepartureDate");
  construct.addElementData(
      marketReq->marketCriteria()->departureDate().dateToString(YYYYMMDD, "-").c_str());
  construct.closeElement(); // DepartureDate

  construct.openElement("DepartureAirportCode");
  construct.addElementData(marketReq->marketCriteria()->departureAirportCode().c_str());
  construct.closeElement(); // DepartureAirportCode

  construct.openElement("ArrivalAirportCode");
  construct.addElementData(marketReq->marketCriteria()->arrivalAirportCode().c_str());
  construct.closeElement(); // ArrivalAirportCode"

  construct.openElement("PassengerTypes");
  addPassengerTypeInfo(construct, marketReq->marketCriteria()->paxType());
  construct.closeElement(); // PassengerTypes
  construct.closeElement(); // MarketCriteria

  construct.openElement("ANCS:CarrierList");
  addCarrierInfo(construct, marketReq->carrriers());
  construct.closeElement(); // CarrierList

  construct.closeElement(); // MarketRequest
}

void
BrandingRequestResponseHandler::addPassengerTypeInfo(XMLConstruct& construct,
                                                     const std::vector<PaxTypeCode>& paxTypes)
{
  std::vector<PaxTypeCode>::const_iterator paxTypeCodeBeg = paxTypes.begin();
  std::vector<PaxTypeCode>::const_iterator paxTypeCodeEnd = paxTypes.end();
  for (; paxTypeCodeBeg != paxTypeCodeEnd; paxTypeCodeBeg++)
  {
    construct.openElement("Type");
    construct.addElementData((*paxTypeCodeBeg).c_str());
    construct.closeElement(); // Type
  }
}

void
BrandingRequestResponseHandler::addCarrierInfo(XMLConstruct& construct,
                                               const MarketRequest::CarrierList& carrierList)
{
  MarketRequest::CarrierList::const_iterator carrriersBeg = carrierList.begin();
  MarketRequest::CarrierList::const_iterator carrriersEnd = carrierList.end();

  for (; carrriersBeg != carrriersEnd; carrriersBeg++)
  {
    construct.openElement("ANCS:Carrier");
    construct.addElementData((*carrriersBeg).c_str());
    construct.closeElement(); // Carrier
  }
}

void
BrandingRequestResponseHandler::addAccountCodeInfo(XMLConstruct& construct,
                                                   const std::vector<std::string>& accountCodes)
{
  std::vector<std::string>::const_iterator accountCodesBeg = accountCodes.begin();
  std::vector<std::string>::const_iterator accountCodesEnd = accountCodes.end();

  for (; accountCodesBeg != accountCodesEnd; accountCodesBeg++)
  {
    construct.openElement("AccountCode");
    construct.addElementData((*accountCodesBeg).c_str());
    construct.closeElement(); // AccountCode
  }
}

void
BrandingRequestResponseHandler::addXray(XMLConstruct& construct, xray::JsonMessage* jsonMessage)
{
  if (jsonMessage == nullptr)
    return;

  construct.openElement("XRA");
  construct.addAttribute("MID", jsonMessage->generateMessageId("S8BRAND").c_str());
  construct.addAttribute("CID", jsonMessage->generateConversationId().c_str());
  construct.closeElement();
}

void
BrandingRequestResponseHandler::brandedFaresDiagnostic888()
{
  if (_trx.diagnostic().diagnosticType() == Diagnostic888)
  {
    if (!(_trx.getTrxType() == PricingTrx::PRICING_TRX ||
          _trx.getTrxType() == PricingTrx::MIP_TRX ||
          _trx.getTrxType() == PricingTrx::FAREDISPLAY_TRX))
      return;

    DCFactory* factory = DCFactory::instance();
    _diag888 = dynamic_cast<Diag888Collector*>(factory->create(_trx));

    _diag888->enable(Diagnostic888);
    _diag888->activate();

    const std::vector<FareMarket*>& markets = _trx.fareMarket();
    if (fallback::reworkTrxAborter(&_trx))
      checkTrxAborted(_trx);
    else
      _trx.checkTrxAborted();
    for (FareMarket* fm : markets)
      printS8BrandedFaresData(*fm);

    _diag888->flushMsg();
  }
}

void
BrandingRequestResponseHandler::printS8BrandedFaresData(FareMarket& fareMarket)
{
  _diag888->printS8Banner();
  const std::string& diagDD = _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL);

  _diag888->printS8FareMarket(fareMarket);

  if (!_diag888->parseFareMarket(_trx, fareMarket))
  {
    // Do not display this fare market
    _diag888->printS8NotProcessed();
    //      _diag888->flushMsg();
    //      factory->reclaim(_diag888);
    return;
  }
  VendorCode vendor;
  CarrierCode carrier;

  const std::string& diagVendor = _trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_VENDOR);
  const std::string& diagCarrier = _trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CARRIER);
  if (diagVendor.empty())
    vendor = "ATP";
  else
    vendor = diagVendor;
  if (diagCarrier.empty())
    carrier = fareMarket.governingCarrier();
  else
    carrier = diagCarrier;

  const std::vector<BrandedFare*>& brandedFareData = getBrandedFaresData(vendor, carrier);

  if (brandedFareData.empty())
  {
    _diag888->printS8CommonHeader();
    _diag888->printS8NotFound(vendor, carrier);
    //     _diag888->flushMsg();
    //     factory->reclaim( _diag888);
    return;
  }

  if (diagDD.empty())
    _diag888->printS8CommonHeader();
  std::vector<BrandedFare*>::const_iterator it = brandedFareData.begin();
  std::vector<BrandedFare*>::const_iterator itE = brandedFareData.end();
  for (; it != itE; ++it)
  {
    BrandedFare* bf = *it;
    if (!(_trx.diagnostic().diagParamMapItem(Diagnostic::SEQ_NUMBER).empty()) &&
        (bf->seqNo() !=
         (int32_t)atoi(_trx.diagnostic().diagParamMapItem(Diagnostic::SEQ_NUMBER).c_str())))
      continue;

    if (!(_trx.diagnostic().diagParamMapItem(Diagnostic::PROGRAM_NAME).empty()) &&
        (bf->programCode() != _trx.diagnostic().diagParamMapItem(Diagnostic::PROGRAM_NAME).c_str()))
      continue;

    if (!(_trx.diagnostic().diagParamMapItem(Diagnostic::BRAND_ID).empty()))
    {
      bool match = false;
      std::vector<BrandedFareSeg*>::const_iterator bs = bf->segments().begin();
      std::vector<BrandedFareSeg*>::const_iterator bsE = bf->segments().end();
      for (; bs != bsE; ++bs)
      {
        if ((*bs)->brandName() == _trx.diagnostic().diagParamMapItem(Diagnostic::BRAND_ID).c_str())
        {
          match = true;
          break;
        }
      }
      if (!match)
        continue;
    }

    if (_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO")
    {
      _diag888->printS8DetailContent(_trx, bf);
    }
    else
      _diag888->printS8BrandedFaresContent(bf);
  }
}

const std::vector<BrandedFare*>&
BrandingRequestResponseHandler::getBrandedFaresData(const VendorCode& vn, const CarrierCode& cxr)
{
  return _trx.dataHandle().getBrandedFare(vn, cxr);
}

bool
BrandingRequestResponseHandler::createDiag890()
{
  if (_diag890)
    return true;

  _diag890 = dynamic_cast<Diag890Collector*>(DCFactory::instance()->create(_trx));
  if (_diag890 == nullptr)
    return false;

  _diag890->enable(Diagnostic890);
  return true;
}

bool BrandingRequestResponseHandler::createDiag891()
{
  if (_diag891)
    return true;

  _diag891 = dynamic_cast<Diag891Collector*>(DCFactory::instance()->create(_trx));
  if (_diag891 == nullptr)
    return false;
  _diag891->enable(Diagnostic891);
  return true;
}

bool BrandingRequestResponseHandler::createDiag898()
{
  if (_diag898)
    return true;

  _diag898 = dynamic_cast<Diag898Collector*>(DCFactory::instance()->create(_trx));
  if (_diag898 == nullptr)
    return false;

  _diag898->enable(Diagnostic898);
  _diag898->activate();
  _diag898->initTrx(_trx);
  return true;
}

bool
BrandingRequestResponseHandler::createDiag()
{
  if (!_trx.diagnostic().isActive())
    return false;

  DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();
  switch(diagType)
  {
    case Diagnostic890:
      return createDiag890();
    case Diagnostic891:
      return createDiag891();
    case Diagnostic898:
      return createDiag898();
    default:
      break;
  }
  return false;
}

bool
BrandingRequestResponseHandler::isError(const S8BrandingResponseParser& s8BrandingResponseParser)
{
  if (s8BrandingResponseParser.isError())
  {
    LOG4CXX_ERROR(logger,
                  "BrandingRequestResponseHandler::printParsingAndClear BrandingResponse "
                  "Got Error Message from Branding Service.");
    s8BrandingResponseParser.print();
    _marketIDFareMarketMap.clear();
    return true;
  }
  return false;
}

void
BrandingRequestResponseHandler::populateFareMarketMarketID(
    const std::vector<FareMarket*>& fareMarketVec, int marketID)
{
  for (FareMarket* fareMarket : fareMarketVec)
    fareMarket->marketIDVec().push_back(marketID);
}

std::string
BrandingRequestResponseHandler::callBrandingService(std::string& request)
{
  if (fallback::fallbackDebugOverrideBrandingServiceResponses(&_trx))
  {
    std::string response;
    BrandingServiceCaller<S8> s8Bsc(_trx);
    s8Bsc.callBranding(request, response);
    _status = s8Bsc.getStatusBrandingService();
    return response;
  }
  else
  {
    ConfigurableBrandingServiceCaller<BrandingServiceCaller<S8>> cs8Bsc(_trx);
    std::string response = cs8Bsc.callBranding(request);
    _status = cs8Bsc.getStatusBrandingService();
    return response;
  }
}

} //namespace tse
