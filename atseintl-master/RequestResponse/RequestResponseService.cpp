/**
 *  Copyright Sabre 2005
 *
 *          The copyright to the computer program(s) herein
 *          is the property of Sabre.
 *          The program(s) may be used and/or copied only with
 *          the written permission of Sabre or in accordance
 *          with the terms and conditions stipulated in the
 *          agreement/contract under which the program(s)
 *          have been supplied.
 *
 */

#include "RequestResponse/RequestResponseService.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/DataCollectorUtil.h"
#include "Common/Logger.h"
#include "Common/StopWatch.h"
#include "Common/TseEnums.h"
#include "Common/TseSrvStats.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/Billing.h"
#include "DataModel/BrandingTrx.h"
#include "DataModel/CurrencyTrx.h"
#include "DataModel/DecodeTrx.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FlightFinderTrx.h"
#include "DataModel/FrequentFlyerTrx.h"
#include "DataModel/MileageTrx.h"
#include "DataModel/MultiExchangeTrx.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PricingDetailTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RexShoppingTrx.h"
#include "DataModel/SettlementTypesTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/StructuredRuleTrx.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TicketingCxrDisplayRequest.h"
#include "DataModel/TicketingCxrDisplayTrx.h"
#include "DataModel/TicketingCxrTrx.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/TrxAborter.h"
#include "RequestResponse/XformHelper.h"
#include "Util/Base64.h"
#include "Xform/PricingResponseXMLTags.h"

#include <algorithm>
#include <string>

namespace tse
{
namespace
{
ConfigurableValue<std::string>
groupName("APPLICATION_CONSOLE", "GROUPNAME", "unknown");
ConfigurableValue<bool>
writeSegments("REQUEST_RESPONSE_SVC", "WRITE_SEGMENTS", true);
ConfigurableValue<bool>
writeRequest("REQUEST_RESPONSE_SVC", "WRITE_REQUEST", true);

ClientSocketConfigurableValues cscv("REQUEST_RESPONSE_SVC");
}

static const char* REQ_CMD = "REQ";
static const char* SCHEMA_VERSION = "0001";
static const char* SCHEMA_REVISION = "0000";
static const char* XMD_START_TAG = "<XMD><FSE>";
static const char* XMD_END_TAG = "</FSE></XMD>";

static const char* SUCCESS_RC = "OK";
static const char* SINGLE_CARRIER = "SCA";
static const char* MULTI_CARRIER = "MCA";

static const std::string
FareSearchResponse("FSR");
static const std::string
FSERequestParent("F1R");
static const std::string
FSERequestChild("F2R");
static const std::string
FSEResponseParent("F1P");
static const std::string
TransactionID("C00");
static const std::string
AgentFunctionCode("N0Q");
static const std::string
IgnoreAvailability("P1V");
static const std::string
BoardPoint("A10");
static const std::string
OffPoint("A14");
static const std::string
DepartureDate1("D01");
static const std::string
DepartureDate2("D09");
static const std::string
SequenceNo("Q1K");
static const std::string
TransactionStartTime("D10");
static const std::string
TransactionEndTime("D11");
static const std::string
NodeID("N0S");
static const std::string
AppConsoleGroupID("CAG");
static const std::string
InputCmdAttribute("S93");
static const std::string
PaxType2("B71");
static const std::string
PaxType3("B72");
static const std::string
FIRSTPARAM("E60"); // this will store itin->geotype data
static const std::string
SECONDPARAM("E61"); // this will store fare display - single carrier, multi carrier request
static const std::string
ReturnCode("Q3M");
static const std::string
TimerExpireCode("Q2N");
static const std::string
WEB("P1Y");
static const std::string
totalNumberFares("Q2I");
static const std::string
numberCat25Fares("Q2K");
static const std::string
numberAddOnFares("Q2J");

static const std::string
numberValidatedFares("Q1W");
static const std::string
numberFareMarkets("Q0R");
static const std::string
numberPaxTypes("Q1X");

static const std::string
numberNonStopItins("Q1Z"); // NUMODRETRIEVEDFARES
static const std::string
numberOneConnectItins("Q2F"); // NUMPQITEMSPROCESSED
static const std::string
numberTwoConnectItins("Q2G"); // NUMPQITEMSFAILRULES
static const std::string
numberThreeConnectItins("Q2H"); // NUMPQITEMSFAILAVAIL
static const std::string
numberOnlineItins("Q2L"); // NUMCACHEHITS
static const std::string
numberInterlineItins("Q2M"); // NUMDBREADS

static const std::string
HurryUpLogicApplies("PHL");

namespace
{
const size_t SOCKET_HEADER_SIZE =
    16; // Client socket didnt expose the sizeof(TRX_HEADER) so had to add this

inline Logger&
getLogger()
{
  static Logger logger("atseintl.Service.RequestResponseService");
  return logger;
}
}

static LoadableModuleRegister<Service, RequestResponseService>
_("libRequestResponse.so");

RequestResponseService::RequestResponseService(const std::string& name, tse::TseServer& server)
  : Service(name, server)
{
  char nodeId[256];
  if (gethostname(nodeId, 256))
  {
    LOG4CXX_WARN(getLogger(), "Failed to get host name.");
  }
  else
    _nodeId.assign(nodeId);

  // Populate the Application Console GroupName (INF-1567)

  _groupName.assign(groupName.getValue());
}

bool
RequestResponseService::process(PricingTrx& trx)
{
  const Billing* billing = trx.billing();
  const PricingRequest* request = trx.getRequest();

  if (billing == nullptr || request == nullptr)
    return true;

  return processRequestResponse(trx, *billing, *request);
}

bool
RequestResponseService::process(RexPricingTrx& trx)
{
  return process(static_cast<PricingTrx&>(trx));
}

bool
RequestResponseService::process(RefundPricingTrx& trx)
{
  return process(static_cast<PricingTrx&>(trx));
}

bool
RequestResponseService::process(ExchangePricingTrx& trx)
{
  return process(static_cast<PricingTrx&>(trx));
}

bool
RequestResponseService::process(FareDisplayTrx& trx)
{
  const Billing* billing = trx.billing();
  const PricingRequest* request = trx.getRequest();

  if (billing == nullptr || request == nullptr)
    return true;

  if (request->ticketingAgent() == nullptr)
    return false;

  return processRequestResponse(trx, *billing, *request);
}

bool
RequestResponseService::process(TaxTrx& trx)
{
  const Billing* billing = trx.billing();
  const PricingRequest* request = trx.getRequest();

  if (billing == nullptr || request == nullptr)
    return true;

  return processRequestResponse(trx, *billing, *request);
}

bool
RequestResponseService::process(ShoppingTrx& trx)
{
  const Billing* billing = trx.billing();
  const PricingRequest* request = trx.getRequest();

  if (billing == nullptr || request == nullptr)
    return true;

  return processRequestResponse(trx, *billing, *request);
}

bool
RequestResponseService::process(RexShoppingTrx& trx)
{
  const Billing* billing = trx.billing();
  const PricingRequest* request = trx.getRequest();

  if (billing == nullptr || request == nullptr)
    return true;

  return processRequestResponse(trx, *billing, *request);
}

bool
RequestResponseService::process(RexExchangeTrx& trx)
{
  return process(static_cast<RexPricingTrx&>(trx));
}

bool
RequestResponseService::process(FlightFinderTrx& trx)
{
  return process(static_cast<ShoppingTrx&>(trx));
}

bool
RequestResponseService::process(AltPricingTrx& trx)
{
  const Billing* billing = trx.billing();
  const PricingRequest* request = trx.getRequest();

  if (billing == nullptr || request == nullptr)
    return true;

  return processRequestResponse(trx, *billing, *request);
}

bool
RequestResponseService::process(NoPNRPricingTrx& trx)
{
  return process((AltPricingTrx&)trx);
}

bool
RequestResponseService::process(CurrencyTrx& trx)
{
  const Billing* billing = trx.billing();
  const PricingRequest* request = trx.getRequest();

  if (billing == nullptr || request == nullptr)
    return true;

  return processRequestResponse(trx, *billing, *request);
}

bool
RequestResponseService::process(MileageTrx& trx)
{
  const Billing* billing = trx.billing();
  const PricingRequest* request = trx.getRequest();

  if (billing == nullptr || request == nullptr)
    return true;

  return processRequestResponse(trx, *billing, *request);
}

bool
RequestResponseService::process(TicketingCxrTrx& trx)
{
  const Billing* billing = trx.billing();
  const PricingRequest* request = trx.getRequest();

  if (billing == nullptr || request == nullptr)
    return true;

  return processRequestResponse(trx, *billing, *request);
}

bool
RequestResponseService::process(TicketingCxrDisplayTrx& trx)
{
  const Billing* billing = trx.billing();
  const PricingRequest* request = trx.getRequest();

  if (billing == nullptr || request == nullptr)
    return true;

  return processRequestResponse(trx, *billing, *request);
}

bool
RequestResponseService::process(MultiExchangeTrx& trx)
{
  if (!trx.newPricingTrx())
    return true;

  const Billing* billing = trx.newPricingTrx()->billing();
  const PricingRequest* request = trx.newPricingTrx()->getRequest();

  if (billing == nullptr || request == nullptr)
    return true;

  const Agent* agent = request->ticketingAgent();

  const std::string fullXML = buildReqResp(trx, *billing, agent); // requestXML+responseXML;

  if (!send(fullXML, trx))
  {
    LOG4CXX_FATAL(getLogger(), "RequestResponseService::Failed To Send Response:");
  }

  return true;
}

bool
RequestResponseService::process(PricingDetailTrx& trx)
{
  const Billing* billing = trx.billing();
  const PricingRequest* request = trx.getRequest();

  if (billing == nullptr || request == nullptr)
    return true;

  return processRequestResponse(trx, *billing, *request);
}

bool
RequestResponseService::process(AncillaryPricingTrx& trx)
{
  const Billing* billing = trx.billing();
  const PricingRequest* request = trx.getRequest();

  if (billing == nullptr || request == nullptr)
    return true;

  return processRequestResponse(trx, *billing, *request);
}

bool
RequestResponseService::process(BaggageTrx& trx)
{
  const Billing* billing = trx.billing();
  const PricingRequest* request = trx.getRequest();

  if (billing == nullptr || request == nullptr)
    return true;

  return processRequestResponse(trx, *billing, *request);
}

bool
RequestResponseService::process(TktFeesPricingTrx& trx)
{
  const Billing* billing = trx.billing();
  const PricingRequest* request = trx.getRequest();

  if (billing == nullptr || request == nullptr)
    return true;

  return processRequestResponse(trx, *billing, *request);
}

bool
RequestResponseService::process(BrandingTrx& trx)
{
  // Treat as a regular Pricing trx
  return process(static_cast<PricingTrx&>(trx));
}

bool
RequestResponseService::process(SettlementTypesTrx& trx)
{
  // Treat as a regular Pricing trx
  return process(static_cast<PricingTrx&>(trx));
}

bool
RequestResponseService::process(StructuredRuleTrx& trx)
{
  return process(static_cast<PricingTrx&>(trx));
}

bool
RequestResponseService::process(DecodeTrx& trx)
{
  const Billing* billing = trx.getBilling();

  if (!billing)
    return true;

  return processRequestResponse(
      trx, *billing, PricingRequest()); // dummy pricing request is intended
}

bool
RequestResponseService::send(const std::string& message, Trx& trx) const
{
  std::string command(REQ_CMD);
  std::string version(SCHEMA_VERSION);
  std::string revision(SCHEMA_REVISION);
  std::string wrappedMessage = XMD_START_TAG + message + XMD_END_TAG;

  tse::StopWatch sw;
  sw.start();

  uint64_t reqSize = wrappedMessage.length() + SOCKET_HEADER_SIZE;
  uint64_t rspSize = 0;

  ClientSocket socket(cscv);
  const bool init = socket.initialize();
  if (!init)
  {
    LOG4CXX_ERROR(getLogger(), "Couldn't initialize request-response socket");
    return false;
  }

  if (!socket.send(command, version, revision, wrappedMessage))
  {
    LOG4CXX_FATAL(getLogger(), "Can't send request-response");
    sw.stop();
    TseSrvStats::recordRequestResponseCall(false, reqSize, rspSize, sw.elapsedTime(), trx);
    return false;
  }

  LOG4CXX_INFO(getLogger(), "Sent request-response: " << wrappedMessage);

  std::string response;
  if (!socket.receive(command, version, revision, response))
  {
    LOG4CXX_FATAL(getLogger(), "Can't receive response from DataCollector.");
    sw.stop();
    TseSrvStats::recordRequestResponseCall(false, reqSize, rspSize, sw.elapsedTime(), trx);
    return false;
  }

  sw.stop();

  LOG4CXX_INFO(getLogger(),
               "Received from DataCollector:"
                   << "\nCommand:  " << command << "\nVersion:  " << version
                   << "\nRevision: " << revision << "\nMessage:  " << response);

  bool success = (response == SUCCESS_RC);

  rspSize = response.length() + SOCKET_HEADER_SIZE;
  TseSrvStats::recordRequestResponseCall(success, reqSize, rspSize, sw.elapsedTime(), trx);

  return true;
}

std::string
RequestResponseService::getResponse(PricingTrx& trx, tse::ConfigMan& config) const
{
  return createResponse(trx, config);
}

class SerializeSegment
{
public:
  SerializeSegment(XMLConstruct& construct,
                   uint64_t trxId,
                   const std::string& nodeId,
                   const std::string& trxStartTime,
                   const std::string& trxEndTime)
    : _construct(construct),
      _trxId(trxId),
      _nodeId(nodeId),
      _trxStartTime(trxStartTime),
      _trxEndTime(trxEndTime)
  {
  }
  void operator()(TravelSeg* seg);
  void operator()(PricingTrx& trx);

private:
  XMLConstruct& _construct;
  uint64_t _trxId;
  const std::string& _nodeId;
  const std::string& _trxStartTime;
  const std::string& _trxEndTime;
};

void
SerializeSegment::
operator()(TravelSeg* seg)
{
  _construct.openElement(FSERequestChild);
  _construct.addAttribute(BoardPoint, seg->origAirport());
  _construct.addAttribute(OffPoint, seg->destAirport());
  std::string cxr(SURFACE_CARRIER);
  AirSeg* air(dynamic_cast<AirSeg*>(seg));
  if (air != nullptr)
    cxr.assign(air->carrier());
  _construct.addAttributeULong(TransactionID, _trxId);
  _construct.addAttribute(NodeID, _nodeId);
  _construct.addAttribute(DepartureDate1, DataCollectorUtil::format(seg->departureDT(), false));
  _construct.addAttribute(DepartureDate2, DataCollectorUtil::format(seg->departureDT(), false));
  std::ostringstream seq;
  seq << seg->segmentOrder();
  _construct.addAttribute(SequenceNo, seq.str());

  _construct.addAttribute(TransactionStartTime, _trxStartTime);
  _construct.addAttribute(TransactionEndTime, _trxEndTime);

  _construct.closeElement();
}

void
SerializeSegment::
operator()(PricingTrx& pricingTrx)
{
  std::for_each(pricingTrx.travelSeg().begin(), pricingTrx.travelSeg().end(), *this);
}

std::string
RequestResponseService::buildRequest(const Trx& trx,
                                     const Billing& billing,
                                     const PricingRequest& request,
                                     const Agent& agent) const
{
  const PricingTrx* pricingTrx = dynamic_cast<const PricingTrx*>(&trx);

  XMLConstruct construct;
  construct.openElement(FSERequestParent);
  construct.addAttribute(xml2::AgentCity, agent.agentCity());
  construct.addAttribute(xml2::TvlAgencyPCC, agent.tvlAgencyPCC());
  construct.addAttribute(xml2::AirlineDept, agent.airlineDept());
  construct.addAttribute(xml2::TvlAgencyIATA, agent.tvlAgencyIATA());
  construct.addAttribute(xml2::HomeAgencyIATA, agent.homeAgencyIATA());
  construct.addAttribute(xml2::CorpId, request.corporateID());
  construct.addAttribute(xml2::CxrCode, agent.cxrCode());

  if (pricingTrx != nullptr)
  {
    if (!pricingTrx->paxType().empty())
      construct.addAttribute(xml2::PassengerType, pricingTrx->paxType().front()->paxType());
  }
  construct.addAttributeULong(TransactionID, billing.transactionID());
  construct.addAttribute(NodeID, _nodeId);
  // construct.addAttribute(AgentFunctionCode, agent.agentFunctions());
  // construct.addAttribute(IgnoreAvailability, string(1, req.priceNoAvailability()));

  std::string startTime = DataCollectorUtil::format(trx.transactionStartTime());
  std::string endTime = DataCollectorUtil::format(trx.transactionEndTime());

  construct.addAttribute(TransactionStartTime, startTime);
  construct.addAttribute(TransactionEndTime, endTime);
  construct.closeElement();

  if (pricingTrx != nullptr && writeSegments.getValue())
  {
    if (!pricingTrx->travelSeg().empty())
      std::for_each(
          pricingTrx->travelSeg().begin(),
          pricingTrx->travelSeg().end(),
          SerializeSegment(
              construct, pricingTrx->billing()->transactionID(), _nodeId, startTime, endTime));
  }

  return construct.getXMLData();
}

std::string
RequestResponseService::buildResponse(const Trx& trx, const Billing& billing) const
{
  XMLConstruct construct;
  construct.openElement(FSEResponseParent);
  construct.addAttribute(xml2::ActionCode, billing.actionCode());
  construct.addAttribute(xml2::UserSetAddress, billing.userSetAddress());
  construct.addAttribute(TransactionStartTime,
                         DataCollectorUtil::format(trx.transactionStartTime()));
  construct.addAttribute(TransactionEndTime, DataCollectorUtil::format(trx.transactionEndTime()));
  construct.addAttribute(xml2::PartitionID, billing.partitionID());
  construct.addAttributeULong(TransactionID, billing.transactionID());
  construct.addAttribute(NodeID, _nodeId);
  construct.addAttribute(xml2::ServiceName, billing.serviceName());
  construct.addAttribute(xml2::UserBranch, billing.userBranch());
  construct.addAttribute(xml2::UserStation, billing.userStation());
  construct.closeElement();

  return construct.getXMLData();
}

bool
RequestResponseService::processRequestResponse(Trx& trx,
                                               const Billing& billing,
                                               const PricingRequest& request) const
{
  LOG4CXX_INFO(getLogger(), "Processing Request :");
  const Agent* agent = request.ticketingAgent();

  const std::string fullXML = buildReqResp(trx, billing, request, agent); // requestXML+responseXML;

  if (!send(fullXML, trx))
  {
    LOG4CXX_FATAL(getLogger(), "RequestResponseService::Failed To Send Response:");
  }

  return true;
}

std::string
RequestResponseService::buildReqResp(const Trx& trx,
                                     const Billing& billing,
                                     const PricingRequest& request,
                                     const Agent* agent) const
{
  const PricingTrx* pricingTrx = dynamic_cast<const PricingTrx*>(&trx);

  XMLConstruct construct;
  construct.openElement(FareSearchResponse);
  //////////////////////////////////key index

  std::string startTime, endTime;

  buildReqRespTrxPart(construct, trx, billing, agent, startTime, endTime);

  construct.addAttribute(xml2::CorpId, request.corporateID()); // AC0

  if (pricingTrx != nullptr)
  {
    if (!pricingTrx->paxType().empty())
      construct.addAttribute(xml2::PassengerType, pricingTrx->paxType().front()->paxType());
    const PricingOptions& pOptions = *pricingTrx->getOptions();
    if (&pOptions != nullptr && 'T' == pOptions.web())
    {
      construct.addAttributeChar(WEB, 'T');
    }
    construct.addAttributeLong(totalNumberFares, TseSrvStats::getTotalNumberFares(trx));
    construct.addAttributeLong(numberCat25Fares, TseSrvStats::getNumberCat25Fares(trx));
    construct.addAttributeLong(numberAddOnFares, TseSrvStats::getNumberAddOnFares(trx));

    construct.addAttributeLong(numberValidatedFares, TseSrvStats::getNumberValidatedFares(trx));
    construct.addAttributeLong(numberFareMarkets, pricingTrx->fareMarket().size());
    construct.addAttributeLong(numberPaxTypes, pricingTrx->paxType().size());

    PricingTrx::ItinMetrics itinMetrics = pricingTrx->getItinMetrics();
    construct.addAttributeLong(numberNonStopItins, itinMetrics.numNonStopItins);
    construct.addAttributeLong(numberOneConnectItins, itinMetrics.numOneConnectItins);
    construct.addAttributeLong(numberTwoConnectItins, itinMetrics.numTwoConnectItins);
    construct.addAttributeLong(numberThreeConnectItins, itinMetrics.numThreeConnectItins);
    construct.addAttributeLong(numberOnlineItins, itinMetrics.numOnlineItins);
    construct.addAttributeLong(numberInterlineItins, itinMetrics.numInterlineItins);

    if (pricingTrx->excTrxType() == PricingTrx::AR_EXC_TRX ||
        pricingTrx->excTrxType() == PricingTrx::AF_EXC_TRX)
      construct.addAttributeUInteger(
          TimerExpireCode,
          static_cast<const RexBaseTrx*>(pricingTrx)->redirectReasonError().code()); // Q2N
    /*
    std::cerr << "totalNumberFares=" << TseSrvStats::getTotalNumberFares(trx)
              << ",numberCat25Fares=" << TseSrvStats::getNumberCat25Fares(trx)
              << ",numberAddOnFares=" << TseSrvStats::getNumberAddOnFares(trx)
              << ",numberValidatedFares=" << TseSrvStats::getNumberValidatedFares(trx)
              << ",numberFareMarkets=" << pricingTrx->fareMarket().size()
              << ",numberPaxTypes=" << pricingTrx->paxType().size()
              << std::endl;
    */
  }

  if (pricingTrx != nullptr && pricingTrx->getTrxType() == PricingTrx::FAREDISPLAY_TRX)
  {
    const FareDisplayTrx* fareDisplayTrx = static_cast<const FareDisplayTrx*>(&trx);
    if (fareDisplayTrx && fareDisplayTrx->isShopperRequest())
      construct.addAttribute(SECONDPARAM, MULTI_CARRIER); // E61
    else
      construct.addAttribute(SECONDPARAM, SINGLE_CARRIER); // E61
  }

  construct.closeElement();

  if (pricingTrx != nullptr && writeSegments.getValue())
  {
    if (!pricingTrx->travelSeg().empty())
    {
      std::for_each(
          pricingTrx->travelSeg().begin(),
          pricingTrx->travelSeg().end(),
          SerializeSegment(
              construct, pricingTrx->billing()->transactionID(), _nodeId, startTime, endTime));
    }
  }
  return construct.getXMLData();
}

void
RequestResponseService::buildReqRespTrxPart(XMLConstruct& construct,
                                            const Trx& trx,
                                            const Billing& billing,
                                            const Agent* agent,
                                            std::string& startTime,
                                            std::string& endTime) const
{
  construct.addAttributeULong(TransactionID, billing.transactionID()); // C00
  construct.addAttributeULong(xml2::ClientTrxID, billing.clientTransactionID()); // C01
  construct.addAttribute(NodeID, _nodeId);
  if (billing.serviceName().empty() && boost::istarts_with(trx.rawRequest(), "<SelectionRequest"))
  {
    construct.addAttribute(xml2::ServiceName, "SELREQ1"); // C20
  }
  else
  {
    construct.addAttribute(xml2::ServiceName, billing.serviceName()); // C20
  }
  //////////////////////////////////idx_01

  construct.addAttributeULong(xml2::ParentTrxID, billing.parentTransactionID()); // C02
  construct.addAttribute(AppConsoleGroupID, _groupName);
  startTime = DataCollectorUtil::format(trx.transactionStartTime());
  endTime = DataCollectorUtil::format(trx.transactionEndTime());
  construct.addAttribute(TransactionStartTime, startTime); // D10
  construct.addAttribute(TransactionEndTime, endTime); // D11
  construct.addAttributeUInteger(ReturnCode, trx.transactionRC()); // Q3M

  construct.addAttribute(xml2::ParentSvcName, billing.parentServiceName()); // C22
  construct.addAttribute(xml2::ClientSvcName, billing.clientServiceName()); // C21

  construct.addAttribute(xml2::ActionCode, billing.actionCode()); // A70
  construct.addAttribute(xml2::PartitionID, billing.partitionID()); // AE0

  const TrxAborter* aborter = trx.aborter();

  if (aborter && (trx.transactionRC() == ErrorResponseException::NO_ERROR))
  {
    construct.addAttribute(HurryUpLogicApplies, // PHL
                           (aborter->getHurryLogicActivatedFlag()) ? "T" : "F");
  }

  // First Parameter -- this is unused column so using it to store itin geo type
  std::string geotraveltype = addGeoTravelType(trx);
  construct.addAttribute(FIRSTPARAM, geotraveltype); // E60

  if (agent)
  {
    construct.addAttribute(xml2::AgentCity, agent->agentCity()); // A10
    construct.addAttribute(xml2::AirlineDept, agent->airlineDept()); // A80
    construct.addAttribute(xml2::TvlAgencyIATA, agent->tvlAgencyIATA()); // AB0
    construct.addAttribute(xml2::HomeAgencyIATA, agent->homeAgencyIATA()); // AB1
    construct.addAttribute(xml2::CxrCode, agent->cxrCode()); // B00
  }

  construct.addAttribute(xml2::UserSetAddress, billing.userSetAddress()); // AD0
  construct.addAttribute(xml2::UserBranch, billing.userBranch()); // Q02
  construct.addAttribute(xml2::UserStation, billing.userStation()); // Q03

  if (agent && (!agent->officeDesignator().empty()))
  {
    // populate from Agent Office Designator (first 5 characters)
    construct.addAttribute(xml2::UserPseudoCityCode,
                           agent->officeDesignator().substr(0, 5)); // AE20
  }
  else
  {
    construct.addAttribute(xml2::UserPseudoCityCode, billing.userPseudoCityCode()); // A20
  }

  construct.addAttribute(xml2::AaaCity, billing.aaaCity()); // A22
  construct.addAttribute(xml2::RequestPath, billing.requestPath()); // S0R
  construct.addAttributeULong(xml2::SolutionsRequested,
                              TseSrvStats::getSolutionsRequested(trx)); // Q0S
  construct.addAttributeULong(xml2::SolutionsProduced, TseSrvStats::getSolutionsFound(trx)); // Q1L
  if (writeRequest.getValue())
  {
    construct.addAttribute(InputCmdAttribute, Base64::encode(trx.rawRequest())); // S93
  }
  {
    // Convert Seconds to Microseconds
    uint64_t cpuMicros =
        static_cast<uint64_t>(TseSrvStats::getTrxCPUTime(const_cast<Trx&>(trx)) * 1000000);
    std::ostringstream oss;
    oss << cpuMicros;
    construct.addAttribute(xml2::TrxCPU, oss.str());
  }
}

std::string
RequestResponseService::buildReqResp(const MultiExchangeTrx& trx,
                                     const Billing& billing,
                                     const Agent* agent) const
{
  XMLConstruct construct;
  construct.openElement(FareSearchResponse);
  //////////////////////////////////key index
  std::string startTime, endTime;

  buildReqRespTrxPart(construct, trx, billing, agent, startTime, endTime);

  if (!trx.skipNewPricingTrx() && !trx.newPricingTrx()->getRequest()->corporateID().empty())
    construct.addAttribute(xml2::CorpId, trx.newPricingTrx()->getRequest()->corporateID()); // AC0
  else if (!trx.skipExcPricingTrx1() && !trx.excPricingTrx1()->getRequest()->corporateID().empty())
    construct.addAttribute(xml2::CorpId, trx.excPricingTrx1()->getRequest()->corporateID()); // AC0
  else if (!trx.skipExcPricingTrx2() && !trx.excPricingTrx2()->getRequest()->corporateID().empty())
    construct.addAttribute(xml2::CorpId, trx.excPricingTrx2()->getRequest()->corporateID()); // AC0

  // Only allow one B70, was told that B71, B72 is used to indicate paxType2,
  // paxType3, unknown why not use <PXI B70=" " /><PXI B70=" " />...
  if (!trx.skipNewPricingTrx() && !trx.newPricingTrx()->paxType().empty())
    construct.addAttribute(xml2::PassengerType, trx.newPricingTrx()->paxType().front()->paxType());
  if (!trx.skipExcPricingTrx1() && !trx.excPricingTrx1()->paxType().empty())
    construct.addAttribute(PaxType2, trx.excPricingTrx1()->paxType().front()->paxType());
  if (!trx.skipExcPricingTrx2() && !trx.excPricingTrx2()->paxType().empty())
    construct.addAttribute(PaxType3, trx.excPricingTrx2()->paxType().front()->paxType());

  construct.closeElement();

  SerializeSegment serialSeg(construct, billing.transactionID(), _nodeId, startTime, endTime);

  if (trx.newPricingTrx() != nullptr)
    serialSeg(*trx.newPricingTrx());
  if (trx.excPricingTrx1() != nullptr)
    serialSeg(*trx.excPricingTrx1());
  if (trx.excPricingTrx2() != nullptr)
    serialSeg(*trx.excPricingTrx2());

  return construct.getXMLData();
}

std::string
RequestResponseService::addGeoTravelType(const Trx& trx) const
{
  std::string itinGeoType("");
  const PricingTrx* pricingTrx = dynamic_cast<const PricingTrx*>(&trx);

  if (pricingTrx != nullptr && !(pricingTrx->itin().empty()))
  {
    const Itin* itin = pricingTrx->itin().front();
    if (itin != nullptr)
      itinGeoType = TseUtil::getGeoType(itin->geoTravelType());
  }
  return itinGeoType;
}
}
