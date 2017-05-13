#include "Xform/CommonRequestHandler.h"

#include "Common/CurrencyUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/RBDByCabinUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/PaxType.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"
#include "Xform/AncillarySchemaNames.h"
#include "Xform/CustomXMLParser/IParser.h"
#include "Xform/CustomXMLParser/IXMLSchema.h"
#include "Xform/CustomXMLParser/IXMLUtils.h"
#include "Xform/DataModelMap.h"
#include "Xform/DynamicConfig.h"
#include "Xform/STLTicketingCxrSchemaNames.h"

#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/regex.hpp>

#include <algorithm>

using namespace boost::assign;

namespace tse
{
FALLBACK_DECL(fallbackNewRBDforWPAE);
FALLBACK_DECL(neutralToActualCarrierMapping);
FALLBACK_DECL(fallbackNewRBDforAB240);
FALLBACK_DECL(ab240FixSsdmps171);

using namespace ancillary;
using namespace stlticketingcxr;

namespace
{
Logger
_logger("atseintl.Xform.CommonRequestHandler");

ILookupMap _elemLookupMap, _attrLookupMap;

bool
init(IXMLUtils::initLookupMaps(_AncillaryElementNames,
                               _NumberOfElementNames_,
                               _elemLookupMap,
                               _AncillaryAttributeNames,
                               _NumberOfAttributeNames_,
                               _attrLookupMap));
}

CommonRequestHandler::CommonRequestHandler(Trx*& trx)
  : _trx(trx),
    _pricingTrx(nullptr),
    _request(nullptr),
    _paxType(nullptr),
    _paxInputOrder(0),
    _itin(nullptr),
    _currentTvlSeg(nullptr),
    _prevTvlSeg(nullptr),
    _segmentOrder(0),
    _datesInRequest(FIRST_MATCH),
    _timesInRequest(FIRST_MATCH),
    _itinOrderNum(1),
    _itinGroupNum(0),
    _reqType(M70),
    _acsBaggageRequest(false),
    _MISC6BaggageRequest(false),
    _maxITNNo(5),
    _ignoreBE0(false),
    _unFlownSegMatch(false),
    _tktRefNumber(2)
{
}

void
CommonRequestHandler::parse(DataHandle& dataHandle,
                            const std::string& content,
                            IBaseHandler& handler)
{
  createTransaction(dataHandle, content);
  IValueString attrValueArray[_NumberOfAttributeNames_];
  int attrRefArray[_NumberOfAttributeNames_];
  IXMLSchema schema(
      _elemLookupMap, _attrLookupMap, _NumberOfAttributeNames_, attrValueArray, attrRefArray, true);
  const char* pChar(content.c_str());
  size_t length(content.length());
  size_t pos(content.find_first_of('<'));
  if (pos != std::string::npos)
  {
    pChar += pos;
    length -= pos;
  }
  IParser parser(pChar, length, handler, schema);
  parser.parse();
}

bool
CommonRequestHandler::startElement(const IKeyString&, const IAttributes&)
{
  return false;
}

bool
CommonRequestHandler::endElement(const IKeyString&)
{
  return false;
}

void
CommonRequestHandler::characters(const char* value, size_t length)
{
  _value.assign(value, length);
}
void
CommonRequestHandler::readAgentAttrs(const IAttributes& attrs, Agent* agent)
{
  // Agent Information

  // A10 - Agent City
  getAttr(attrs, _A10, agent->agentCity());

  // A20 - Travel agent PCC
  getAttr(attrs, _A20, agent->tvlAgencyPCC());

  // A21 - Main travel agency PCC
  std::string tmpBuf = "";
  getAttr(attrs, _A21, tmpBuf);
  if (tmpBuf.size() > 3 && tmpBuf[3] == ' ')
    tmpBuf[3] = '\0';

  agent->mainTvlAgencyPCC() = tmpBuf;

  // A80 - Airline department
  getAttr(attrs, _A80, agent->airlineDept());

  // A90 - Agent function code
  getAttr(attrs, _A90, agent->agentFunctions());

  // AB0 - Agency IATA number
  getAttr(attrs, _AB0, agent->tvlAgencyIATA());

  // AB1 - Home agency IATA number
  getAttr(attrs, _AB1, agent->homeAgencyIATA());

  // AE0 - Vendor CRS Code
  tmpBuf = "";
  getAttr(attrs, _AE0, tmpBuf);
  if (MCPCarrierUtil::isPseudoCarrier(tmpBuf))
  {
    _pricingTrx->mcpCarrierSwap() = true;
    if (_pricingTrx->billing())
      _pricingTrx->billing()->partitionID() = MCPCarrierUtil::swapToActual(_pricingTrx, tmpBuf);
  }
  agent->vendorCrsCode() = MCPCarrierUtil::swapToActual(_pricingTrx, tmpBuf);
  // LATAM MCP-S
  if(!fallback::neutralToActualCarrierMapping(_pricingTrx) &&
      MCPCarrierUtil::isNeutralCarrier(tmpBuf))
  {
    if (_pricingTrx->billing())
      _pricingTrx->billing()->partitionID() = MCPCarrierUtil::swapFromNeutralToActual(tmpBuf);
    agent->vendorCrsCode() = MCPCarrierUtil::swapFromNeutralToActual(tmpBuf);
  }

  if (!fallback::ab240FixSsdmps171(static_cast<Trx*>(_pricingTrx)))
  {
    // AE3 - Default Ticketing Carrier
    getAttr(attrs, _AE3, agent->defaultTicketingCarrier());
  }

  // B00 - Originating carrier
  tmpBuf = "";
  getAttr(attrs, _B00, tmpBuf);
  if (tmpBuf.empty())
    tmpBuf = "1S";
  agent->cxrCode() = MCPCarrierUtil::swapToActual(_pricingTrx, tmpBuf);

  // C40 - Agent currency code
  getAttr(attrs, _C40, agent->currencyCodeAgent());
  checkCurrency(CurrencyCode(agent->currencyCodeAgent()));

  // C6C - Agent commission amount
  int intBuf = attrs.get<int>(_C6C, 0);
  agent->agentCommissionAmount() = intBuf;

  // N0G - Agent duty code
  getAttr(attrs, _N0G, agent->agentDuty());

  // N0L - Agent commission type
  getAttr(attrs, _N0L, agent->agentCommissionType());

  // Q01 - coHost ID
  attrs.get(_Q01, intBuf, 0);
  agent->coHostID() = intBuf;

  // AE1 - Office Designator
  getAttr(attrs, _AE1, agent->officeDesignator());

  // AE2 - Office/Station Code
  getAttr(attrs, _AE2, agent->officeStationCode());
}

void
CommonRequestHandler::onStartAGI(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter AGI");
  readAgentAttrs(attrs, _pricingTrx->getRequest()->ticketingAgent());
}

void
CommonRequestHandler::onStartBIL(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter BIL");

  std::string tmpBuf = "";

  // A20 - User pseudoCity code
  getAttr(attrs, _A20, _pricingTrx->billing()->userPseudoCityCode());

  // A22 - aaaCity
  getAttr(attrs, _A22, _pricingTrx->billing()->aaaCity());

  // A70 - Action code
  getAttr(attrs, _A70, _actionCode);
  _pricingTrx->billing()->actionCode() = _actionCode;

  // AA0 - agentSine
  getAttr(attrs, _AA0, _pricingTrx->billing()->aaaSine());

  // AD0 - User set address
  getAttr(attrs, _AD0, _pricingTrx->billing()->userSetAddress());

  // AE0 - Partition ID
  getAttr(attrs, _AE0, tmpBuf);
  if (MCPCarrierUtil::isPseudoCarrier(tmpBuf))
    _pricingTrx->mcpCarrierSwap() = true;

  _pricingTrx->billing()->partitionID() = tmpBuf;
  std::string realCxr;
  realCxr = MCPCarrierUtil::swapToActual(_pricingTrx, _pricingTrx->billing()->partitionID());
  // LATAM MCP-S
  if(!fallback::neutralToActualCarrierMapping(_pricingTrx) &&
      MCPCarrierUtil::isNeutralCarrier(tmpBuf))
  {
    realCxr = MCPCarrierUtil::swapFromNeutralToActual(tmpBuf);
    _pricingTrx->billing()->partitionID() = realCxr;
  }
  if (MCPCarrierUtil::isIAPCarrierRestricted(realCxr))
    throw ErrorResponseException(
        ErrorResponseException::MCP_IAP_RESTRICTED,
        ("UNABLE TO PROCESS-ENTRY RESTRICTED IN " + realCxr + " PARTITION").c_str());

  // C00 - Parent Transaction Id
  std::string transactionID;
  getAttr(attrs, _C00, transactionID);
  _pricingTrx->billing()->parentTransactionID() =
      Billing::string2transactionId(transactionID.c_str());

  // C01 - Client Transaction Id
  transactionID = "";
  getAttr(attrs, _C01, transactionID);
  _pricingTrx->billing()->clientTransactionID() =
      Billing::string2transactionId(transactionID.c_str());

  // C20 - Parent service name
  getAttr(attrs, _C20, _pricingTrx->billing()->parentServiceName());

  // C21 - Not parse it

  // S0R - Source of Request - Not parse it
  getAttr(attrs, _S0R, _pricingTrx->billing()->requestPath());

  // Q02 - User branch
  getAttr(attrs, _Q02, _pricingTrx->billing()->userBranch());

  // Q03 - User station
  getAttr(attrs, _Q03, _pricingTrx->billing()->userStation());

  // L00 - Client Transaction ID
  std::string showBaggageTravelIndex;
  getAttr(attrs, _L00, showBaggageTravelIndex);
  if (!showBaggageTravelIndex.compare("V2DIAG"))
    _pricingTrx->showBaggageTravelIndex() = true;
}

// TicketingCxrService and TicketingCxrDisplay BillingInformation
void
CommonRequestHandler::onStartValCxrBIL(const IAttributes& attrs, Billing& billing)
{
  LOG4CXX_DEBUG(_logger, "Enter ValCxr BIL");
  getAttr(attrs, ACTIONCODE, _actionCode);
  billing.actionCode() = _actionCode;

  getAttr(attrs, BUSINESS_FUNCTION, billing.clientServiceName());
  getAttr(attrs, PARENT_SERVICE_NAME, billing.parentServiceName());

  std::string transactionID;
  getAttr(attrs, PARENT_TRANSACTION_ID, transactionID);
  billing.parentTransactionID() = Billing::string2transactionId(transactionID.c_str());

  getAttr(attrs, SOURCE_OF_REQUEST, billing.requestPath());
  getAttr(attrs, USER_BRANCH, billing.userBranch());
  getAttr(attrs, USER_SET_ADDRESS, billing.userSetAddress());
  getAttr(attrs, USER_STATION, billing.userStation());
}

void
CommonRequestHandler::onEndValCxrBIL(Billing& billing,
                                     Billing::Service serviceId,
                                     uint64_t transactionId)
{
  billing.updateTransactionIds(transactionId);
  billing.updateServiceNames(serviceId);
  LOG4CXX_DEBUG(_logger, "Leave BillingInformation");
}

void
CommonRequestHandler::addFrequentFlyerStatus(PaxType::FreqFlyerTierWithCarrier* ffd,
                                             const PaxTypeCode& ptc)
{
  // Only do this when we are parsing within an ITN element
  if (_pricingTrx->activationFlags().isAB240() && _itin != 0)
  {
    if (ffd->freqFlyerTierLevel() > 0)
      _ffDataForItin.push_back(ffd);
    return;
  }

  // skip check for ACS requests
  if (!(isWPBGRequest() || _acsBaggageRequest || _MISC6BaggageRequest))
  {
    std::multimap<PaxTypeCode, PaxType::FreqFlyerTierWithCarrier*>::iterator it;
    std::pair<std::multimap<PaxTypeCode, PaxType::FreqFlyerTierWithCarrier*>::iterator,
              std::multimap<PaxTypeCode, PaxType::FreqFlyerTierWithCarrier*>::iterator> ret =
        _ffData.equal_range(ptc);

    for (it = ret.first; it != ret.second; it++)
    {
      if ((*it).second->cxr() == ffd->cxr())
      {
        std::string err = "CHECK FREQUENT FLIER STATUS FOR ";
        err += (ptc.empty() ? "BLANK PTC" : ptc);
        err += " ";
        err += (ffd->cxr().empty() ? "BLANK CXR" : ffd->cxr());
        throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, err.c_str());
      }
    }
  }
  if (ffd->freqFlyerTierLevel() > 0)
    _ffData.insert(std::pair<PaxTypeCode, PaxType::FreqFlyerTierWithCarrier*>(ptc, ffd));
}

void
CommonRequestHandler::onStartDIG(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter DIG");

  // QOA - Diag Argument Type
  std::string diagArgType;
  getAttr(attrs, _Q0A, diagArgType);
  _pricingTrx->getRequest()->diagArgType().push_back(diagArgType);

  // S01 - Diag Argument Data
  std::string diagArgData;
  getAttr(attrs, _S01, diagArgData);
  _pricingTrx->getRequest()->diagArgData().push_back(diagArgData);
  if (diagArgData.length() >= 2)
  {
    _pricingTrx->diagnostic().diagParamMap().insert(std::map<std::string, std::string>::value_type(
        diagArgData.substr(0, 2), diagArgData.substr(2, diagArgData.length() - 2)));
  }
}

void
CommonRequestHandler::createITN(const IAttributes& attrs)
{
  _pricingTrx->dataHandle().get(_itin);
  if (_itin == nullptr)
    throw std::runtime_error("Null pointer to int data");
  _segmentOrder = 1;
  attrs.get(_Q00, _itinOrderNum, _itinOrderNum);
  _itin->setItinOrderNum(_itinOrderNum++);

  if (attrs.has(_TKN))
  {
    uint8_t ticketNumber = 0;
    attrs.get(_TKN, ticketNumber, ticketNumber);
    _itin->setTicketNumber(ticketNumber);
  }

  _unFlownSegMatch = false;
}

void
CommonRequestHandler::onStartITN(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter ITN");
  createITN(attrs);

  if (_pricingTrx->itin().size() > _maxITNNo)
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                 "INVALID - MAX NUMBER OF ITINERARIES EXCEEDED");
}

void
CommonRequestHandler::onStartIRO(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter IRO");

  // SHC - Tour Code
  std::string tourCode;
  getAttr(attrs, _SHC, tourCode);

  // ANG - Ancillaries non guarantee (by default false)
  bool nonGuaranteeIndicator;
  attrs.get(_ANG, nonGuaranteeIndicator, false);
  if (_reqType != M70)
  {
    Itin* itin = _request->masterItin();
    if (_request->isMultiTicketRequest())
      itin = _request->subItin(_itinGroupNum);
    if (itin != nullptr && !tourCode.empty())
    {
      if (_request->tourCodePerItin().empty())
        _request->tourCodePerItin()[itin] = tourCode;
    }

    if (itin != nullptr && nonGuaranteeIndicator)
    {
      if (_request->ancillNonGuaranteePerItin().empty())
        _request->ancillNonGuaranteePerItin()[itin] = nonGuaranteeIndicator;
    }
  }

  _request->tourCodePerItin()[_itin] = tourCode;
  _request->ancillNonGuaranteePerItin()[_itin] = nonGuaranteeIndicator;

  if (isPostTktReqType())
  {
    // S14 - original Pricing command
    getAttr(attrs, _S14, _originalPricingCommand);
    _originalPricingCommand = cleanOriginalPricingCommandPostTkt();
    _ignoreBE0 = !_originalPricingCommand.empty();

    AncRequest::AncAttrMapTree& map = _request->itinAttrMap()[_itin]["IRO"];
    addAttributeToHashMap(map, _ANG, attrs, 5);
    map.addAttribute(_AncillaryAttributeNames[_S14], _originalPricingCommand, 5);
  }
}

void
CommonRequestHandler::onStartSGI(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter SGI");
  _pricingTrx->dataHandle().get(_currentTvlSeg);
  if (_currentTvlSeg == nullptr)
    throw std::runtime_error("Null pointer to int data");

  // Q00 - Segment ID

  _currentTvlSeg->pnrSegment() = attrs.get<int16_t>(_Q00, 0);
  _currentTvlSeg->segmentOrder() = _segmentOrder;

  if (_ignoreBE0)
    return;

  // BE0 - Ticket Desginator
  TktDesignator ticketDesignator;
  getAttr(attrs, _BE0, ticketDesignator);
  if (!ticketDesignator.empty())
  {
    _request->tktDesignatorPerItin()[_itin].insert(
        std::pair<int16_t, TktDesignator>(_currentTvlSeg->segmentOrder(), ticketDesignator));
    if (isPostTktReqType())
    {
      AncRequest::AncAttrMapTree& map =
          _request->itinAttrMap()[_itin]["SGI"][_currentTvlSeg->pnrSegment()];
      addAttributeToHashMap(map, _BE0, attrs, 5);
    }
  }
}
void
CommonRequestHandler::onStartFBI(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter FBI");
  if (_reqType != M70 && !isWPBGRequest())
    checkFBIDataForWPAE(attrs);

  AncRequest::AncFareBreakInfo* ancFbi(nullptr);
  _pricingTrx->dataHandle().get(ancFbi);
  if (ancFbi == nullptr)
    throw std::runtime_error("Null pointer to int data");

  // Q00 - Fare Component ID
  ancFbi->fareComponentID() = attrs.get<SequenceNumber>(_Q00, 0);

  // B02 - Governing Carrier
  getAttr(attrs, _B02, ancFbi->governingCarrier());

  // C50 - Fare Amount
  std::string fareAmount;
  getAttr(attrs, _C50, fareAmount);
  ancFbi->fareAmount() = atof(fareAmount.c_str());

  // B50 - Fare basis code with slashes and resulting ticket designator
  getAttr(attrs, _B50, ancFbi->fareBasis());

  // S53 - Record1 Fare type
  getAttr(attrs, _S53, ancFbi->fareType());

  if (_reqType != M70)
    // S37 - Vendor code
    getAttr(attrs, _S37, ancFbi->vendorCode());

  // Q3W - Fare tariff
  ancFbi->fareTariff() = attrs.get<TariffNumber>(_Q3W, 0);

  // S90 - Fare Rule
  getAttr(attrs, _S90, ancFbi->fareRule());

  // FTY - Fare types
  std::string ocFareStat;
  getAttr(attrs, _FTY, ocFareStat);
  if (!ocFareStat.empty())
  {
    ancFbi->fareStat() = ocFareStat;
  }
  else
  {
    // Use Q3V, P1Z
    // Q3V - Fare Indicator
    ancFbi->fareIndicator() = attrs.get<int>(_Q3V, 0);

    // P1Z - PrivateTariff Indicator (Public by default)
    bool privateIndicator;
    attrs.get(_P1Z, privateIndicator, false);
    ancFbi->privateIndicator() = privateIndicator;
  }

  bool isRemoved(false);
  if (isPostTktReqType())
  {
    std::vector<int> fbas;
    for (AncRequest::AncFareBreakAssociation* fba : _request->fareBreakAssociationPerItin()[_itin])
      fbas.push_back(fba->fareComponentID());

    // if removed and not referenced by FBA then remove FBI
    isRemoved = std::find(_removedFBA[_itin].begin(),
                          _removedFBA[_itin].end(),
                          ancFbi->fareComponentID()) != _removedFBA[_itin].end() &&
                std::find(fbas.begin(), fbas.end(), ancFbi->fareComponentID()) == fbas.end();
  }
  if (!isRemoved)
    _request->fareBreakPerItin()[_itin].push_back(ancFbi);

  if (_reqType != M70)
  {
    AncRequest::AncAttrMapTree& map =
        _request->itinAttrMap()[_itin]["FBI"][_currentTvlSeg->pnrSegment()];
    addAttributeToHashMap(map, _Q00, attrs, 3);
    addAttributeToHashMap(map, _B02, attrs, 3);
    addAttributeToHashMap(map, _C50, attrs, 3);
    addAttributeToHashMap(map, _B50, attrs, 3);
    addAttributeToHashMap(map, _S53, attrs, 3);
    addAttributeToHashMap(map, _S37, attrs, 3);
    addAttributeToHashMap(map, _Q3W, attrs, 3);
    addAttributeToHashMap(map, _S90, attrs, 3);
    addAttributeToHashMap(map, _FTY, attrs, 3);
    addAttributeToHashMap(map, _Q3V, attrs, 3);
    addAttributeToHashMap(map, _P1Z, attrs, 3);
  }
}

void
CommonRequestHandler::onStartPXI(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter PXI");

  _pricingTrx->dataHandle().get(_paxType);
  if (_paxType == nullptr)
    throw std::runtime_error("Null pointer to int data");

  // B70 - Passenger type
  getAttr(attrs, _B70, _paxType->paxType());
  if (_paxType->paxType().empty())
    _paxType->paxType() = ADULT;
  _paxType->requestedPaxType() = _paxType->paxType();

  PaxTypeUtil::parsePassengerWithAge(*_paxType);

  // Q0U - Passenger Count for the Type
  _paxType->number() = attrs.get<int>(_Q0U, 1);

  if (_reqType != M70)
  {
    AncRequest::AncAttrMapTree& map = _request->itinAttrMap()[_itin]["PXI"];
    addAttributeToHashMap(map, _B70, attrs, 4);
    addAttributeToHashMap(map, _Q0U, attrs, -1);
  }
}

void
CommonRequestHandler::onStartPNM(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter PNM");

  PaxType::TktInfo* tktInfo = nullptr;
  _pricingTrx->dataHandle().get(tktInfo);
  if (tktInfo == nullptr)
    throw std::runtime_error("Null pointer to int data");

  getAttr(attrs, _S0L, tktInfo->psgNameNumber());
  getAttr(attrs, _Q87, tktInfo->tktNumber());
  getAttr(attrs, _Q86, tktInfo->tktRefNumber());
  if (tktInfo->tktRefNumber().empty())
  {
    if (isWPBGRequest() && _request->wpbgPostTicket())
      throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                   "TICKET NUMBER IS MISSING");

    if (isPostTktReqType())
      _request->noTktRefNumberInR7() = true;
    char buf[10];
    sprintf(buf, "%d", _tktRefNumber++);
    tktInfo->tktRefNumber() = buf;
  }

  if (isPostTktReqType())
  {
    if (tktInfo->psgNameNumber().empty())
      throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                   "INVALID PASSENGER NAME NUMBER");
    if (tktInfo->tktNumber().empty())
      throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                   "TICKET NUMBER IS MISSING");
  }

  _paxType->psgTktInfo().push_back(tktInfo);
  if (_reqType != M70)
  {
    AncRequest::AncAttrMapTree& map =
        _request->itinAttrMap()[_itin]["PNM"][_paxType->psgTktInfo().size()];
    addAttributeToHashMap(map, _S0L, attrs, 4);
    addAttributeToHashMap(map, _Q87, attrs, 4);
  }
}

void
CommonRequestHandler::onStartACI(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter ACI");
  if (isPostTktReqType() && !_originalPricingCommand.empty())
    return;

  // ACC - Account Code
  std::string accountCode;
  getAttr(attrs, _ACC, accountCode);
  addAccountCode(accountCode);
  if (isPostTktReqType())
  {
    AncRequest::AncAttrMapTree& map = _request->itinAttrMap()[_itin]["ACI"][accountCode];
    addAttributeToHashMap(map, _ACC, attrs, 5);
  }
}

void
CommonRequestHandler::onStartCII(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter CII");
  if (isPostTktReqType() && !_originalPricingCommand.empty())
    return;

  // CID - CorporateID
  std::string corpId;
  getAttr(attrs, _CID, corpId);
  addCorpId(corpId);
  if (isPostTktReqType())
  {
    AncRequest::AncAttrMapTree& map = _request->itinAttrMap()[_itin]["CII"][corpId];
    addAttributeToHashMap(map, _CID, attrs, 5);
  }
}

void
CommonRequestHandler::onStartFBA(const IAttributes& attrs)
{
  LOG4CXX_DEBUG(_logger, "Enter FBA");

  if (_reqType != M70)
    checkFBADataForWPAE(attrs);

  AncRequest::AncFareBreakAssociation* fba(nullptr);
  _pricingTrx->dataHandle().get(fba);
  if (fba == nullptr)
    throw std::runtime_error("Null pointer to int data");

  // set segment ID
  fba->segmentID() = _currentTvlSeg->segmentOrder();

  // Q6D - FareComponent ID
  SequenceNumber id = attrs.get<SequenceNumber>(_Q6D, 0);
  fba->fareComponentID() = id;

  // Q6E - Side Trip ID
  attrs.get(_Q6E, id, 0);
  fba->sideTripID() = id;

  // S07 - Side Trip Start Indicator
  bool indicator;
  attrs.get(_S07, indicator, false);
  fba->sideTripStart() = indicator;

  // S08 - Side Trip End Indicator
  attrs.get(_S08, indicator, false);
  fba->sideTripEnd() = indicator;

  _request->fareBreakAssociationPerItin()[_itin].push_back(fba);

  if (_reqType != M70)
  {
    AncRequest::AncAttrMapTree& map =
        _request->itinAttrMap()[_itin]["FBA"][_currentTvlSeg->pnrSegment()];
    addAttributeToHashMap(map, _Q6D, attrs, 2);
    addAttributeToHashMap(map, _Q6E, attrs, 2);
    addAttributeToHashMap(map, _S07, attrs, 2);
    addAttributeToHashMap(map, _S08, attrs, 2);
  }
}

void
CommonRequestHandler::onStartDynamicConfig(const IAttributes& attrs)
{
  DynamicConfigHandler handler(*_trx);
  if (!handler.check())
    return;

  DynamicConfigInput input;

  if (attrs.has(_Name))
    input.setName(attrs.get<std::string>(_Name));
  if (attrs.has(_Value))
    input.setValue(attrs.get<std::string>(_Value));
  if (attrs.has(_Substitute))
    input.setSubstitute(attrs.get<std::string>(_Substitute));
  if (attrs.has(_Optional))
    input.setOptional(attrs.get<std::string>(_Optional));

  handler.process(input);
}

void
CommonRequestHandler::finalizeAgent(Agent* agent)
{
  // set agent location
  const Loc* agentLocation = nullptr;

  if (!agent->agentCity().empty())
    agentLocation =
        _pricingTrx->dataHandle().getLoc(agent->agentCity(), _pricingTrx->ticketingDate());

  std::vector<Customer*> custList;
  if (!agent->tvlAgencyPCC().empty())
    custList = _pricingTrx->dataHandle().getCustomer(agent->tvlAgencyPCC());

  LocCode agentCity;
  // set TJR
  if (!custList.empty())
  {
    agent->agentTJR() = custList.front();
    const Loc* tjrLoc =
        _pricingTrx->dataHandle().getLoc(agent->agentTJR()->aaCity(), _pricingTrx->ticketingDate());
    agentCity = tjrLoc ? tjrLoc->loc() : "";
    if (agent->agentCity().empty())
      agent->agentCity() = agentCity;
  }
  else
  {
    if (agent->tvlAgencyPCC().size() == 4)
    {
      std::stringstream msg;
      msg << "Customer record: '" << agent->tvlAgencyPCC() << "' PCC missing!";
      LOG4CXX_ERROR(_logger, msg.str());
    }
    agentCity = _pricingTrx->dataHandle().getMultiTransportCity(agent->agentCity());
  }
  if (!agentCity.empty())
    agent->agentLocation() =
        _pricingTrx->dataHandle().getLoc(agentCity, _pricingTrx->ticketingDate());

  // if no multicity/TJR city, or nations different, use def agengLocation
  if (!agent->agentLocation() ||
      (agentLocation && (agent->agentLocation()->nation() != agentLocation->nation())))
  {
    agent->agentLocation() = agentLocation;
  }

  // should we throw exception if no match on agent location?
  if (!agent->agentLocation())
  {
    throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
  }
  // if no A21 in request
  if (agent->mainTvlAgencyPCC().empty())
  {
    // if A20 present, and there is customer, use HOMEPSEUDOCITY to populate A21
    if (agent->agentTJR())
      agent->mainTvlAgencyPCC() = agent->agentTJR()->homePseudoCity();
    else // else copy A10 value to A21
      agent->mainTvlAgencyPCC() = agent->agentCity();
  }

  // if currency not defined, use from agent location
  if (agent->currencyCodeAgent().empty())
  {
    if (agent->agentTJR() != nullptr && !agent->agentTJR()->defaultCur().empty())
    {
      agent->currencyCodeAgent() = agent->agentTJR()->defaultCur();
    }
    else
    {
      // last chance to get currency base on agent location
      CurrencyCode cur;
      if (CurrencyUtil::getSaleLocOverrideCurrency(
              agent->agentLocation()->loc(), cur, _pricingTrx->ticketingDate()))
        agent->currencyCodeAgent() = cur;
    }
  }
  if (agent->agentTJR() != nullptr)
  {
    if (agent->tvlAgencyIATA().empty())
      agent->tvlAgencyIATA() = agent->agentTJR()->arcNo();
    if (agent->homeAgencyIATA().empty())
      agent->homeAgencyIATA() = agent->agentTJR()->homeArcNo();
  }
}

void
CommonRequestHandler::onEndAGI()
{
  finalizeAgent(_pricingTrx->getRequest()->ticketingAgent());
  LOG4CXX_DEBUG(_logger, "Leave AGI");
}

void
CommonRequestHandler::onEndFFY()
{
  LOG4CXX_DEBUG(_logger, "Leave FFY");
}

void
CommonRequestHandler::onEndDIG()
{
  LOG4CXX_DEBUG(_logger, "Leave DIG");
}

void
CommonRequestHandler::onEndIRO()
{
  LOG4CXX_DEBUG(_logger, "Leave IRO");

  setFrequentFlyerStatus();

  // check for maximum number of account id & corp id
  if (_request->accountCodeIdPerItin()[_itin].size() + _request->corpIdPerItin()[_itin].size() +
          _request->invalidCorpIdPerItin()[_itin].size() >
      4)
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                 "INVALID - MAXIMUM 4 ACCOUNT CODE AND/OR CORPORATE ID PERMITTED");
}

void
CommonRequestHandler::onEndSGI()
{
  // skip flown segments for PostTkt request
  if (isPostTktReqType() && !_unFlownSegMatch)
  {
    if (_currentTvlSeg->resStatus() == "OK" &&
        _currentTvlSeg->departureDT() != DateTime::openDate())
    {
      DateTime compareDT = _currentTvlSeg->departureDT().subtractSeconds(
          60 * 60 * _request->noHoursBeforeDeparture());

      if (compareDT < _curDate)
      {
        // remova all segments, reset segment order
        _segmentOrder = 1;
        _itin->travelSeg().clear();
        for (AncRequest::AncFareBreakAssociation* fba :
             _request->fareBreakAssociationPerItin()[_itin])
          _removedFBA[_itin].push_back(fba->fareComponentID());

        _request->fareBreakAssociationPerItin()[_itin].clear();
        _request->tktDesignatorPerItin()[_itin].clear();
        return;
      }
      else
        _unFlownSegMatch = true;
    }
  }

  // set multicity
  bool isInternational =
      LocUtil::isInternational(*(_currentTvlSeg->origin()), *(_currentTvlSeg->destination()));

  _currentTvlSeg->boardMultiCity() =
      FareMarketUtil::getMultiCity(_currentTvlSeg->carrier(),
                                   _currentTvlSeg->origAirport(),
                                   isInternational ? GeoTravelType::International : GeoTravelType::Domestic,
                                   _currentTvlSeg->departureDT());
  _currentTvlSeg->offMultiCity() =
      FareMarketUtil::getMultiCity(_currentTvlSeg->carrier(),
                                   _currentTvlSeg->destAirport(),
                                   isInternational ? GeoTravelType::International : GeoTravelType::Domestic,
                                   _currentTvlSeg->departureDT());

  // set status to confirmed
  if (_reqType == M70)
    _currentTvlSeg->resStatus() = "OK";
  _currentTvlSeg->realResStatus() = "HK";

  // skip flown segments for M70 request
  if (_reqType == M70 && !_unFlownSegMatch &&
      _request->ancRequestType() != AncRequest::BaggageRequest && !_acsBaggageRequest &&
      !_MISC6BaggageRequest)
  {
    if (!_currentTvlSeg->isAir())
    {
      // remova all segments, reset segment order
      _segmentOrder = 1;
      _itin->travelSeg().clear();
      for (AncRequest::AncFareBreakAssociation* fba :
           _request->fareBreakAssociationPerItin()[_itin])
        _removedFBA[_itin].push_back(fba->fareComponentID());

      _request->fareBreakAssociationPerItin()[_itin].clear();
      _request->tktDesignatorPerItin()[_itin].clear();
      return;
    }
    else
      _unFlownSegMatch = true;
  }

  // if previos segment exist, and arrival difeerent than departure fo current, add arunk
  // only if not same city
  bool addArunk =
      ((_prevTvlSeg != nullptr) && (_prevTvlSeg->destAirport() != _currentTvlSeg->origAirport()));

  if (_reqType == M70)
    addArunk = addArunk && (_prevTvlSeg->offMultiCity() != _currentTvlSeg->boardMultiCity());

  if (addArunk)
  {
    ArunkSeg* arunkSeg = nullptr;
    _pricingTrx->dataHandle().get(arunkSeg);
    if (arunkSeg == nullptr)
      throw std::runtime_error("Null pointer to int data");
    arunkSeg->segmentType() = Arunk;
    arunkSeg->pnrSegment() = ARUNK_PNR_SEGMENT_ORDER;
    arunkSeg->segmentOrder() = _segmentOrder;
    arunkSeg->origAirport() = _prevTvlSeg->destAirport();
    arunkSeg->destAirport() = _currentTvlSeg->origAirport();
    arunkSeg->origin() = _prevTvlSeg->destination();
    arunkSeg->destination() = _currentTvlSeg->origin();
    arunkSeg->departureDT() = _prevTvlSeg->arrivalDT();
    arunkSeg->arrivalDT() = _currentTvlSeg->departureDT();
    arunkSeg->boardMultiCity() = _prevTvlSeg->offMultiCity();
    arunkSeg->offMultiCity() = _currentTvlSeg->boardMultiCity();
    if (_reqType == M70)
    {
      arunkSeg->pssDepartureTime() = _prevTvlSeg->pssArrivalTime();
      arunkSeg->pssArrivalTime() = _currentTvlSeg->pssDepartureTime();
    }

    _itin->travelSeg().push_back(arunkSeg);
    _pricingTrx->travelSeg().push_back(arunkSeg);

    if (_request && !_request->fareBreakAssociationPerItin()[_itin].empty() &&
        _request->fareBreakAssociationPerItin()[_itin].back()->segmentID() == _segmentOrder)
    {
      ++_request->fareBreakAssociationPerItin()[_itin].back()->segmentID();
    }
    _currentTvlSeg->segmentOrder() = ++_segmentOrder;
  }
  // update prev segment
  _prevTvlSeg = _currentTvlSeg;
  _itin->travelSeg().push_back(_currentTvlSeg);

  // TODO: without this ItinAnalyzer will seg fault
  _pricingTrx->travelSeg().push_back(_currentTvlSeg);
  _segmentOrder++;

  LOG4CXX_DEBUG(_logger, "Leave SGI");
}

void
CommonRequestHandler::onEndFBI()
{
  LOG4CXX_DEBUG(_logger, "Leave FBI");
}

void
CommonRequestHandler::onEndPNM()
{
  LOG4CXX_DEBUG(_logger, "Leave PNM");
}

void
CommonRequestHandler::onEndPXI()
{
  addPaxType();
  LOG4CXX_DEBUG(_logger, "Leave PXI");
}

void
CommonRequestHandler::onEndACI()
{
  LOG4CXX_DEBUG(_logger, "Leave ACI");
}

void
CommonRequestHandler::onEndCII()
{
  LOG4CXX_DEBUG(_logger, "Leave CII");
}

void
CommonRequestHandler::onEndFLI()
{
  LOG4CXX_DEBUG(_logger, "Leave FLI");
}

void
CommonRequestHandler::onEndFBA()
{
  LOG4CXX_DEBUG(_logger, "Leave FBA");
}

void
CommonRequestHandler::setFrequentFlyerStatus()
{
  // update frequent flyer statuses
  if (_pricingTrx->activationFlags().isAB240() && !_ffDataForItin.empty())
  {
    for (PaxType* paxType : _request->paxTypesPerItin()[_itin])
    {
      // Do this only once. This function is called multiple times.
      if (!paxType->freqFlyerTierWithCarrier().empty())
        continue;

      std::copy(_ffDataForItin.begin(),
                _ffDataForItin.end(),
                std::back_inserter(paxType->freqFlyerTierWithCarrier()));
    }

    return;
  }

  std::vector<PaxType*>::iterator ipt = _request->paxTypesPerItin()[_itin].begin();
  std::vector<PaxType*>::iterator ipe = _request->paxTypesPerItin()[_itin].end();
  std::multimap<PaxTypeCode, PaxType::FreqFlyerTierWithCarrier*>::iterator it;
  std::pair<std::multimap<PaxTypeCode, PaxType::FreqFlyerTierWithCarrier*>::iterator,
            std::multimap<PaxTypeCode, PaxType::FreqFlyerTierWithCarrier*>::iterator> ret,
      retBlank;

  retBlank = _ffData.equal_range(PaxTypeCode(""));
  for (; ipt != ipe; ipt++)
  {
    PaxTypeCode paxTypeCode = (*ipt)->paxType();
    if (((*ipt)->age() > 0) && _acsBaggageRequest)
    {
      std::ostringstream paxTypeBuf;
      paxTypeBuf.fill('0');
      paxTypeBuf << (*ipt)->paxType()[0] << std::setw(2) << (*ipt)->age();
      paxTypeCode = paxTypeBuf.str();
    }

    ret = _ffData.equal_range(paxTypeCode);

    if (!(*ipt)->freqFlyerTierWithCarrier().empty())
      continue;

    for (it = ret.first; it != ret.second; it++)
      (*ipt)->freqFlyerTierWithCarrier().push_back((*it).second);
    for (it = retBlank.first; it != retBlank.second; it++)
      (*ipt)->freqFlyerTierWithCarrier().push_back((*it).second);
  }
}

DateTime
CommonRequestHandler::convertDate(const std::string& date) const
{
  bool valid = date.length() == 10 && date[4] == '-' && date[7] == '-' &&
               std::count_if(date.begin(), date.end(), isdigit) == 8;
  if (!valid)
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                 "INVALID - DATETIME FORMAT");

  int year = 0, month = 0, day = 0;
  year = atol(date.substr(0, 4).c_str());
  month = atol(date.substr(5, 2).c_str());
  day = atol(date.substr(8, 2).c_str());

  return DateTime(year, month, day);
}
void
CommonRequestHandler::checkForDateConsistency(const std::string& depatureDate)
{
  if (depatureDate.empty())
  {
    // if no date, but was provided in previous segmnt (first segment is ignored), then generate
    // error
    if (_datesInRequest == CommonRequestHandler::DATE_PRESENT)
      throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID - CHECK DATES");
    _datesInRequest = CommonRequestHandler::DATE_NOT_PRESENT;
  }
  else
  {
    // if date provided, but no date in previous segment - generate error
    if (_datesInRequest == CommonRequestHandler::DATE_NOT_PRESENT)
      throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID - CHECK DATES");

    // date can ba only in all segments, first segment, or no segment
    if (_datesInRequest == CommonRequestHandler::FIRST_MATCH)
      _datesInRequest = CommonRequestHandler::DATE_IN_FRIST_SEGMENT;
    else
      _datesInRequest = CommonRequestHandler::DATE_PRESENT;
  }
}

void
CommonRequestHandler::checkForTimeConsistency(const std::string& departureTime,
                                              const std::string& arriveTime)
{
  if (departureTime.empty() ^ arriveTime.empty())
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                 "INVALID - ENTER TIMES ALL SEGMENTS");

  if (departureTime.empty())
  {
    if (_timesInRequest == CommonRequestHandler::DATE_PRESENT)
      throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                   "INVALID - ENTER TIMES ALL SEGMENTS");
    _timesInRequest = CommonRequestHandler::DATE_NOT_PRESENT;
  }
  else
  {
    if (_timesInRequest == CommonRequestHandler::DATE_NOT_PRESENT)
      throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                   "INVALID - ENTER TIMES ALL SEGMENTS");
    _timesInRequest = CommonRequestHandler::DATE_PRESENT;
  }
}

void
CommonRequestHandler::setCabin(Indicator cabin)
{
 if(TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(*_pricingTrx) &&
   (isBaggageTransaction() || isNewRBDforAncillaryActive()))
 {
  if (!_currentTvlSeg->getBookingCode().empty())
  {
    const Cabin* cabin = getCabin();
    if (cabin)
     _currentTvlSeg->bookedCabin() = cabin->cabin();
    else
     _currentTvlSeg->bookedCabin().setInvalidClass(); // UNKNOWN CABIN valid values are 1,2,4,5,
  }
  else
  {
    switch (cabin)
    {
    case PREMIUM_ECONOMY_CLASS:
      _currentTvlSeg->bookedCabin().setClassFromAlphaNumAnswer('W');
      break;
    case UNDEFINED_CLASS:
      break;
    default:
      _currentTvlSeg->bookedCabin().setClassFromAlphaNumAnswer(cabin);
    }
  }
 }
 else
 {
  if (!_currentTvlSeg->getBookingCode().empty())
  {
    const Cabin* cabin = _pricingTrx->dataHandle().getCabin(_currentTvlSeg->carrier(),
                                                            _currentTvlSeg->getBookingCode(),
                                                            _currentTvlSeg->departureDT());
    if (cabin)
      _currentTvlSeg->bookedCabin() = cabin->cabin();
    else
      _currentTvlSeg->bookedCabin().setInvalidClass(); // UNKNOWN CABIN valid values are 1,2,4,5,
  }
  else
  {
    switch (cabin)
    {
    case PREMIUM_FIRST_CLASS:
      _currentTvlSeg->bookedCabin().setPremiumFirstClass();
      break;
    case PREMIUM_ECONOMY_CLASS:
      _currentTvlSeg->bookedCabin().setPremiumEconomyClass();
      break;
    case UNDEFINED_CLASS:
      break;
    default:
      _currentTvlSeg->bookedCabin().setClassFromAlphaNum(cabin);
    }
  }
 }
  if (_currentTvlSeg->bookedCabin().isInvalidClass())
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID CABIN");
}

const Cabin*
CommonRequestHandler::getCabin()
{
  RBDByCabinUtil rbdUtil(*_pricingTrx, ANC_BAG_RQ);
  return rbdUtil.getCabinByRBD( _currentTvlSeg->carrier(),
                                _currentTvlSeg->getBookingCode(),
                                *_currentTvlSeg, true, _itinTicketIssueDate);
}

void
CommonRequestHandler::addPaxType()
{
  if (_paxType == nullptr)
    return;

  std::vector<PaxType*>::iterator it = _request->paxTypesPerItin()[_itin].begin();
  std::vector<PaxType*>::iterator ie = _request->paxTypesPerItin()[_itin].end();
  for (; it != ie; it++)
  {
    if ((*it)->requestedPaxType() == _paxType->requestedPaxType())
    {
      if (isPostTktReqType())
        (*it)->number() += _paxType->number();
      else if ((*it)->number() < _paxType->number())
        (*it)->number() = _paxType->number();
      (*it)->psgTktInfo().insert(
          (*it)->psgTktInfo().end(), _paxType->psgTktInfo().begin(), _paxType->psgTktInfo().end());
      _paxType = nullptr;
      return;
    }
  }

  if (PaxTypeUtil::initialize(*_pricingTrx,
                              *_paxType,
                              _paxType->paxType(),
                              _paxType->number(),
                              _paxType->age(),
                              _paxType->stateCode(),
                              _paxInputOrder++))
  {
    _request->paxTypesPerItin()[_itin].push_back(_paxType);
    _request->paxToOriginalItinMap()[_paxType] = _itin;
    _pricingTrx->paxType().push_back(_paxType);
  }

  _paxType = nullptr;
}

short
CommonRequestHandler::getTimeDiff()
{
  short utcOffset = 0;
  const Loc* hdqLoc =
      _pricingTrx->dataHandle().getLoc(/*RuleConst::HDQ_CITY*/ "HDQ", _pricingTrx->ticketingDate());
  const Loc* salesLoc =
      _pricingTrx->getRequest()->salePointOverride().empty()
          ? _pricingTrx->getRequest()->ticketingAgent()->agentLocation()
          : _pricingTrx->dataHandle().getLoc(_pricingTrx->getRequest()->salePointOverride(),
                                             _pricingTrx->ticketingDate());
  if (hdqLoc && salesLoc &&
      LocUtil::getUtcOffsetDifference(
          *salesLoc, *hdqLoc, utcOffset, _pricingTrx->dataHandle(), _curDate, _curDate))
    return utcOffset;
  return 0;
}

void
CommonRequestHandler::setDate(const DateTime& date)
{
  _curDate = date;
  _pricingTrx->getRequest()->ticketingDT() = date;
  _pricingTrx->dataHandle().setTicketDate(date);

  _pricingTrx->ticketingDate() = date;
}

void
CommonRequestHandler::setTime(DateTime& date, std::string stime)
{
  if (!stime.empty())
  {
    int time = atoi(stime.c_str());
    date = date + tse::Hours(time / 100) + tse::Minutes(time % 100) + tse::Seconds(0);
  }
  else
    date = date + tse::Hours(23) + tse::Minutes(59) + tse::Seconds(59);
}

std::string
CommonRequestHandler::pssTime(std::string stime)
{
  if (stime.empty())
    return "";
  int time = atoi(stime.c_str());
  int hrs = time / 100;
  int mins = time % 100;
  char buf[10]; // 0000 to 1439
  sprintf(buf, "%d", hrs * 60 + mins);
  return buf;
}
namespace
{
struct TravelSegSegmentOrderComparator
{
  bool operator()(TravelSeg* s1, TravelSeg* s2) { return s1->segmentOrder() < s2->segmentOrder(); }
};
}

void
CommonRequestHandler::checkSideTrip(Itin* itin)
{
  if (itin->travelSeg().empty())
    return;

  std::vector<AncRequest::AncFareBreakAssociation*>& ancFareBreakVec =
      _request->fareBreakAssociationPerItin()[itin];
  if (ancFareBreakVec.empty())
    return;

  int segMin = (*std::min_element(itin->travelSeg().begin(),
                                  itin->travelSeg().end(),
                                  TravelSegSegmentOrderComparator()))->segmentOrder();
  int segMax = (*std::max_element(itin->travelSeg().begin(),
                                  itin->travelSeg().end(),
                                  TravelSegSegmentOrderComparator()))->segmentOrder();
  std::vector<AncRequest::AncFareBreakAssociation*>::iterator it = ancFareBreakVec.begin();
  std::vector<AncRequest::AncFareBreakAssociation*>::iterator ie = ancFareBreakVec.end();
  for (; it != ie; it++)
  {
    if ((*it)->sideTripID())
    {
      if ((*it)->segmentID() == segMin || (*it)->segmentID() == segMax)
        throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                     "INVALID - PLACE OF SIDE TRIP");
    }
  }
}
void
CommonRequestHandler::checkCurrency(const CurrencyCode& cur)
{
  CurrencyUtil::validateCurrencyCode(*_pricingTrx, cur);
}

void
CommonRequestHandler::checkData(const IAttributes& attrs,
                                const std::vector<int>& requredAttrNames,
                                const char* attributesNames,
                                const std::string& msg) const
{
  for (int attr : requredAttrNames)
  {
    VALIDATE_OR_THROW(attrs.has(attr),
                      ErrorResponseException::INVALID_FORMAT,
                      "INVALID - MISSING " << attributesNames[attr] << " " << msg;);
  }
}

void
CommonRequestHandler::checkFBADataForWPAE(const IAttributes& attrs)
{
  std::vector<int> requredAttrNames;
  requredAttrNames += _Q6D; // _Q7E, _S07, _S08
  checkData(attrs, requredAttrNames, *_AncillaryAttributeNames, "ATTRIBUTE IN FBA SECTION");
}

void
CommonRequestHandler::checkFBIDataForWPAE(const IAttributes& attrs)
{
  std::vector<int> requredAttrNames;
  requredAttrNames += _Q00, _B02, _C50, _B50, _S53, _S37, _Q3W, _S90,
      _FTY; //, Q3V, P1Z are optional
  checkData(attrs, requredAttrNames, *_AncillaryAttributeNames, "ATTRIBUTE IN FBI SECTION");
}

void
CommonRequestHandler::parseOriginalPricingCommandPostTkt()
{
  if (_originalPricingCommand.size() <= 2)
    return;

  // create tokens to parse
  boost::char_separator<char> sep("$");
  boost::char_separator<char> slashSep("/");
  boost::tokenizer<boost::char_separator<char> > tokens(_originalPricingCommand, sep);

  // define what we are looking for and handling function
  boost::regex accCode("^AC\\*([0-9A-Z]{1,20})$");
  boost::regex corpId("^I([A-Z]{3}[0-9]{2})$");
  boost::regex tktDesignator("^Q([A-Z0-9]*(?:/[A-Z0-9]*)?)$");
  boost::regex segSel("^S((?:[0-9]+/?)*)(?:\\*Q([A-Z0-9]*(?:/[A-Z0-9]*)?))?$");
  boost::regex rangeSegSel("^S([0-9]+)-([0-9]+)(?:\\*Q([A-Z0-9]*(?:/[A-Z0-9]*)?))?$");
  boost::regex taxExempt("^TE-([A-Z0-9]{2,3}(/[A-Z0-9]{2,3})*)$");
  boost::regex allTaxExempt("^TE$");
  boost::regex allFeeExempt("^TN$");

  std::vector<int> secSel;
  bool parseTktDesig = _request->tktDesignatorPerItin()[_itin].empty();

  for (const std::string& wpPart : tokens)
  {
    boost::match_results<std::string::const_iterator> m;
    if (boost::regex_search(wpPart, m, accCode))
      addAccountCode(std::string(m[1].first, m[1].second));
    else if (boost::regex_search(wpPart, m, corpId))
      addCorpId(std::string(m[1].first, m[1].second));
    else if (parseTktDesig && boost::regex_search(wpPart, m, tktDesignator))
    {
      addTktDesignator(std::string(m[1].first, m[1].second), std::vector<int>());
    }
    else if (boost::regex_search(wpPart, m, segSel))
    {
      secSel.clear();
      std::string segIds(m[1].first, m[1].second);
      boost::tokenizer<boost::char_separator<char> > secTokens(segIds, slashSep);
      for (std::string seg : secTokens)
        secSel.push_back(std::atoi(seg.c_str()));
      if (parseTktDesig && m[2].matched)
        addTktDesignator(std::string(m[2].first, m[2].second), secSel);
    }
    else if (boost::regex_search(wpPart, m, rangeSegSel))
    {
      int n1 = std::atoi(std::string(m[1].first, m[1].second).c_str());
      int n2 = std::atoi(std::string(m[2].first, m[2].second).c_str());
      for (int n = n1; n <= n2; n++)
        secSel.push_back(n);
      if (parseTktDesig && m[3].matched)
        addTktDesignator(std::string(m[3].first, m[3].second), secSel);
    }
    else
    {
      if (boost::regex_search(wpPart, m, taxExempt))
      {
        _pricingTrx->getRequest()->exemptSpecificTaxes() = 'Y';
        std::string taxExemptIds(m[1].first, m[1].second);
        boost::tokenizer<boost::char_separator<char> > taxExemptTokens(taxExemptIds, slashSep);
        for (std::string te : taxExemptTokens)
          _pricingTrx->getRequest()->taxIdExempted().push_back(te);
      }
      else if (boost::regex_search(wpPart, m, allTaxExempt))
      {
        _pricingTrx->getRequest()->exemptAllTaxes() = 'Y';
      }
      else if (boost::regex_search(wpPart, m, allFeeExempt))
      {
        _pricingTrx->getRequest()->exemptAllTaxes() = 'Y';
      }
    }
  }
  // need to parse this only once
  _originalPricingCommand = "";
}

std::string
CommonRequestHandler::cleanOriginalPricingCommandPostTkt()
{
  if (_originalPricingCommand.size() <= 2)
    return "";

  std::string ret("");

  // create tokens to parse
  boost::char_separator<char> sep("$");
  boost::tokenizer<boost::char_separator<char> > tokens(_originalPricingCommand, sep);

  // define what we are looking for and handling function
  boost::regex accCode("^AC\\*([0-9A-Z]{1,20})$");
  boost::regex corpId("^I([A-Z]{3}[0-9]{2})$");
  boost::regex tktDesignator("^Q([A-Z0-9]*(?:/[A-Z0-9]*)?)$");
  boost::regex segSel("^S((?:[0-9]+/?)*)(?:\\*Q([A-Z0-9]*(?:/[A-Z0-9]*)?))?$");
  boost::regex rangeSegSel("^S([0-9]+)-([0-9]+)(?:\\*Q([A-Z0-9]*(?:/[A-Z0-9]*)?))?$");
  boost::regex taxExempt("^TE-([A-Z0-9]{2,3}(/[A-Z0-9]{2,3})*)$");
  boost::regex allTaxExempt("^TE$");
  boost::regex allFeeExempt("^TN$");

  for (const std::string& wpPart : tokens)
  {
    boost::match_results<std::string::const_iterator> m;
    if (boost::regex_search(wpPart, m, accCode) || boost::regex_search(wpPart, m, corpId) ||
        boost::regex_search(wpPart, m, tktDesignator) || boost::regex_search(wpPart, m, segSel) ||
        boost::regex_search(wpPart, m, rangeSegSel) || boost::regex_search(wpPart, m, taxExempt) ||
        boost::regex_search(wpPart, m, allTaxExempt) ||
        boost::regex_search(wpPart, m, allFeeExempt))
    {
      ret += wpPart;
      ret += "$";
    }
  }
  return ret;
}

void
CommonRequestHandler::addAccountCode(const std::string& accountCode)
{
  std::vector<std::string>& accCodeVec = _request->accountCodeIdPerItin()[_itin];
  if (std::find(accCodeVec.begin(), accCodeVec.end(), accountCode) == accCodeVec.end())
    accCodeVec.push_back(accountCode);
}

void
CommonRequestHandler::addCorpId(const std::string& corpId)
{
  bool isACCValid = _pricingTrx->dataHandle().corpIdExists(corpId, _pricingTrx->ticketingDate());
  std::vector<std::string>& corpIdVec =
      isACCValid ? _request->corpIdPerItin()[_itin] : _request->invalidCorpIdPerItin()[_itin];

  if (std::find(corpIdVec.begin(), corpIdVec.end(), corpId) == corpIdVec.end())
    corpIdVec.push_back(corpId);
}

void
CommonRequestHandler::addTktDesignator(const std::string tktDesig, const std::vector<int>& segIds)
{
  if (segIds.empty())
  {
    for (TravelSeg* ts : _itin->travelSeg())
      _request->tktDesignatorPerItin()[_itin].insert(
          std::pair<int16_t, TktDesignator>(ts->segmentOrder(), tktDesig.c_str()));
  }
  else
  {
    for (TravelSeg* ts : _itin->travelSeg())
      if (std::find(segIds.begin(), segIds.end(), ts->pnrSegment()) != segIds.end())
        _request->tktDesignatorPerItin()[_itin].insert(
            std::pair<int16_t, TktDesignator>(ts->segmentOrder(), tktDesig.c_str()));
  }
}
void
CommonRequestHandler::addAttributeToHashMap(AncRequest::AncAttrMapTree& map,
                                            int name,
                                            const IAttributes& attr,
                                            int hashIndex)
{
  if (!attr.has(name))
    return;

  std::string val = "";
  getAttr(attr, name, val);
  map.addAttribute(_AncillaryAttributeNames[name], val, hashIndex);
}
void
CommonRequestHandler::mergeSameFBAAndFBI(Itin* it)
{
  if (_mergedPaxItins[it].empty())
    _mergedPaxItins[it].push_back(it);
  // ifa paxtypeas are the same - merge numbers
  for (const Itin* i : _mergedPaxItins[it])
  {
    if (_request->itinAttrMap()[_itin].getHash(4) == _request->itinAttrMap()[i].getHash(4))
    {
      for (PaxType* px : _request->paxTypesPerItin()[it])
      {
        if (px->paxType() == _request->paxTypesPerItin()[_itin].front()->paxType())
        {
          px->number() += _request->paxTypesPerItin()[_itin].front()->number();
          return;
        }
      }
    }
  }
  // if pax typeas are diferent then accumulate pax types
  _request->paxTypesPerItin()[it].insert(_request->paxTypesPerItin()[it].end(),
                                         _request->paxTypesPerItin()[_itin].begin(),
                                         _request->paxTypesPerItin()[_itin].end());
  _mergedPaxItins[it].push_back(_itin);
}

void
CommonRequestHandler::parseOriginalPricingCommandWPAE()
{
  if (_originalPricingCommand.size() <= 2)
    return;

  // eat up WP
  if (_originalPricingCommand.substr(0, 5) == "WPNCS")
  {
    _pricingTrx->getRequest()->lowFareNoRebook() = 'T';
    _originalPricingCommand = _originalPricingCommand.substr(5);
  }
  else if (_originalPricingCommand.substr(0, 5) == "WPNCB")
    _originalPricingCommand = _originalPricingCommand.substr(5);
  else if (_originalPricingCommand.substr(0, 4) == "WPNC")
  {
    _pricingTrx->getRequest()->lowFareNoRebook() = 'T';
    _originalPricingCommand = _originalPricingCommand.substr(4);
  }
  else if (_originalPricingCommand.substr(0, 2) == "WP")
    _originalPricingCommand = _originalPricingCommand.substr(2);

  // create tokens to parse
  boost::char_separator<char> sep("$");
  boost::char_separator<char> slashSep("/");
  boost::tokenizer<boost::char_separator<char> > tokens(_originalPricingCommand, sep);

  // define what we are looking for and han dling function
  boost::regex accCode("^AC\\*([0-9A-Z]{1,20})$");
  boost::regex corpId("^I([A-Z]{3}[0-9]{2})$");
  boost::regex buyingDate(
      "^B([0-9]{1,2})(JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC)([0-9]{2})$");
  boost::regex tktDesignator("^Q([A-Z0-9]*(?:/[A-Z0-9]*)?)$");
  boost::regex segSel("^S((?:[0-9]+/?)*)(?:\\*Q([A-Z0-9]*(?:/[A-Z0-9]*)?))?$");
  boost::regex rangeSegSel("^S([0-9]+)-([0-9]+)(?:\\*Q([A-Z0-9]*(?:/[A-Z0-9]*)?))?$");
  boost::regex overPointOfSale("^S([A-Z]{3})$");
  boost::regex overPointOfTkt("^T([A-Z]{3})$");
  boost::regex overCurrency("^M([A-Z]{3})$");
  boost::regex frequentFlyer("^FFS-([A-Z]{2}[0-9](/[A-Z]{2}[0-9])*)$");
  boost::regex taxExempt("^TE-([A-Z0-9]{2,3}(/[A-Z0-9]{2,3})*)$");
  boost::regex allTaxExempt("^TE$");
  boost::regex allFeeExempt("^TN$");

  std::vector<int> secSel;
  bool parseTktDesig = _request->tktDesignatorPerItin()[_itin].empty();
  bool revalidateCurOrSaleLoc = false;
  _pricingTrx->getOptions()->mOverride() = 0;

  for (const std::string& wpPart : tokens)
  {
    boost::match_results<std::string::const_iterator> m;
    if (boost::regex_search(wpPart, m, overPointOfSale))
    {
      revalidateCurOrSaleLoc = true;
      _pricingTrx->getRequest()->salePointOverride() = std::string(m[1].first, m[1].second);
    }
    else if (boost::regex_search(wpPart, m, overPointOfTkt))
    {
      _pricingTrx->getRequest()->ticketPointOverride() = std::string(m[1].first, m[1].second);
    }
    else if (boost::regex_search(wpPart, m, overCurrency))
    {
      revalidateCurOrSaleLoc = true;
      _pricingTrx->getOptions()->currencyOverride() = std::string(m[1].first, m[1].second);
      checkCurrency(_pricingTrx->getOptions()->currencyOverride());
      _pricingTrx->getOptions()->mOverride() = 'T';
    }
    else if (boost::regex_search(wpPart, m, accCode))
      addAccountCode(std::string(m[1].first, m[1].second));
    else if (boost::regex_search(wpPart, m, corpId))
      addCorpId(std::string(m[1].first, m[1].second));
    else if (boost::regex_search(wpPart, m, buyingDate))
    {
      std::vector<std::string> mths(MONTHS_UPPER_CASE, MONTHS_UPPER_CASE + 13);
      DateTime buyingDate(
          std::atoi(std::string(m[3].first, m[3].second).c_str()) + 2000,
          std::find(mths.begin(), mths.end(), std::string(m[2].first, m[2].second)) - mths.begin(),
          std::atoi(std::string(m[1].first, m[1].second).c_str()));
      setDate(buyingDate);
    }
    else if (parseTktDesig && boost::regex_search(wpPart, m, tktDesignator))
    {
      addTktDesignator(std::string(m[1].first, m[1].second), std::vector<int>());
    }
    else if (boost::regex_search(wpPart, m, segSel))
    {
      secSel.clear();
      std::string segIds(m[1].first, m[1].second);
      boost::tokenizer<boost::char_separator<char> > secTokens(segIds, slashSep);
      for (std::string seg : secTokens)
        secSel.push_back(std::atoi(seg.c_str()));
      if (parseTktDesig && m[2].matched)
        addTktDesignator(std::string(m[2].first, m[2].second), secSel);
    }
    else if (boost::regex_search(wpPart, m, rangeSegSel))
    {
      int n1 = std::atoi(std::string(m[1].first, m[1].second).c_str());
      int n2 = std::atoi(std::string(m[2].first, m[2].second).c_str());
      for (int n = n1; n <= n2; n++)
        secSel.push_back(n);
      if (parseTktDesig && m[3].matched)
        addTktDesignator(std::string(m[3].first, m[3].second), secSel);
    }
    else if (boost::regex_search(wpPart, m, frequentFlyer) && _ffData.empty())
    {
      std::string freqFlyerStatuses(m[1].first, m[1].second);
      boost::tokenizer<boost::char_separator<char> > ffsTokens(freqFlyerStatuses, slashSep);
      for (std::string ffs : ffsTokens)
      {
        PaxType::FreqFlyerTierWithCarrier* ffd(nullptr);
        _pricingTrx->dataHandle().get(ffd);
        if (ffd == nullptr)
          throw std::runtime_error("Null pointer to int data");

        ffd->setCxr(ffs.substr(0, 2));
        ffd->setFreqFlyerTierLevel(std::atoi(ffs.substr(2, 1).c_str()));
        addFrequentFlyerStatus(ffd, "");
      }
      // force ffy for current ITN
      setFrequentFlyerStatus();
    }
    else if (boost::regex_search(wpPart, m, taxExempt))
    {
      _pricingTrx->getRequest()->exemptSpecificTaxes() = 'Y';
      std::string taxExemptIds(m[1].first, m[1].second);
      boost::tokenizer<boost::char_separator<char> > taxExemptTokens(taxExemptIds, slashSep);
      for (std::string te : taxExemptTokens)
        _pricingTrx->getRequest()->taxIdExempted().push_back(te);
    }
    else if (boost::regex_search(wpPart, m, allTaxExempt))
    {
      _pricingTrx->getRequest()->exemptAllTaxes() = 'Y';
    }
    else if (boost::regex_search(wpPart, m, allFeeExempt))
    {
      _pricingTrx->getRequest()->exemptAllTaxes() = 'Y';
    }
  }

  if (revalidateCurOrSaleLoc)
    (static_cast<AncillaryPricingTrx*>(_pricingTrx))->checkCurrencyAndSaleLoc();

  // need to parse this only once
  _originalPricingCommand = "";
}

void
CommonRequestHandler::getAttr(const IAttributes& attrs, int idx, std::string& str) const
{
  if (attrs.has(idx))
  {
    const IValueString& attr = attrs.get(idx);
    str.assign(attr.c_str(), attr.length());
  }
}

void
CommonRequestHandler::getAttr(const IAttributes& attrs, int idx, BoostString& str) const
{
  if (attrs.has(idx))
  {
    const IValueString& attr = attrs.get(idx);
    str.assign(attr.c_str(), attr.length());
  }
}

void
CommonRequestHandler::getValue(std::string& str) const
{
  if (!_value.empty())
    str.assign(_value.c_str(), _value.length());
}

void
CommonRequestHandler::setSegmentDates(const std::string& departureDT,
                                      const std::string& depTime,
                                      const std::string& arrivalDT,
                                      const std::string& arrTime)
{
  // D01 - Departure Date
  bool openDepartureDate = false;

  if (_currentTvlSeg->segmentType() == Open)
  {
    if (!departureDT.empty())
      _currentTvlSeg->departureDT() = convertDate(departureDT);
    else
    {
      if (_prevTvlSeg)
        openDepartureDate = true;
      else
        _currentTvlSeg->departureDT() = DateTime(_curDate.year(), _curDate.month(), _curDate.day());
    }
  }
  else
    _currentTvlSeg->departureDT() = convertDate(departureDT);

  // D02 - Arrival Date
  if (openDepartureDate)
  {
    const DateTime dtd = _prevTvlSeg->departureDT().addDays(1);
    _currentTvlSeg->departureDT() = DateTime(dtd.year(), dtd.month(), dtd.day());

    const DateTime dta = _prevTvlSeg->arrivalDT().addDays(1);
    _currentTvlSeg->arrivalDT() = DateTime(dta.year(), dta.month(), dta.day());
  }
  else
  {
    if (arrivalDT.empty())
    {
      DateTime newDate(_curDate.year(), _curDate.month(), _curDate.day());
      _currentTvlSeg->arrivalDT() = newDate.addSeconds(60);
    }
    else
      _currentTvlSeg->arrivalDT() = convertDate(arrivalDT);
  }
  if (!depTime.empty())
    setTime(_currentTvlSeg->departureDT(), depTime);

  if (!arrTime.empty())
    setTime(_currentTvlSeg->arrivalDT(), arrTime);

  adjustOpenSegmentDates();
}

void
CommonRequestHandler::adjustOpenSegmentDates()
{
  if (_prevTvlSeg && !_prevTvlSeg->arrivalDT().isEmptyDate())
  {
    // Adjust departure date/time
    if (_currentTvlSeg->departureDT().hours() == 0 && _currentTvlSeg->departureDT().minutes() == 0)
    {
      const DateTime& prevArrDT = _prevTvlSeg->arrivalDT();
      DateTime newDate(_currentTvlSeg->departureDT().date(),
                       prevArrDT.hours(),
                       prevArrDT.minutes(),
                       prevArrDT.seconds());
      _currentTvlSeg->departureDT() = newDate.addSeconds(60);
    }
    // Adjust arrival date/time
    if (_prevTvlSeg->arrivalDT().hours() == 0 && _prevTvlSeg->arrivalDT().minutes() == 0)
    {
      const DateTime& currDepDT = _currentTvlSeg->departureDT();
      DateTime newDate(_prevTvlSeg->arrivalDT().date(),
                       currDepDT.hours(),
                       currDepDT.minutes(),
                       currDepDT.seconds());
      _prevTvlSeg->arrivalDT() = newDate.subtractSeconds(60);
    }
  }
}

void
CommonRequestHandler::checkFlight(const Itin& itin, bool checkSequence, bool checkHistorical) const
{
  if (itin.travelSeg().size() >= 1)
  {
    if (checkSequence)
    {
      std::vector<TravelSeg*>::const_iterator it = itin.travelSeg().begin();

      for (std::vector<TravelSeg*>::const_iterator ie = it + 1; ie != itin.travelSeg().end(); ie++)
      {
        if (!(*ie)->isAir())
          continue;

        if ((*it)->arrivalDT() > (*ie)->departureDT())
        {
          throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                       "TRAVEL DATES OUT OF SEQUENCE");
        }
        it = ie;
      }
    }

    if (checkHistorical)
    {
      if (!itin.travelSeg().empty() && !itin.travelSeg().front()->isAir())
      {
        throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
                                     "INVALID - HISTORICAL DATES NOT ALLOWED");
      }
    }
  }
}

void
CommonRequestHandler::detectRtw()
{
  if (_pricingTrx && _pricingTrx->itin().size() &&
      _pricingTrx->itin().front()->travelSeg().size())
  {
    const std::vector<TravelSeg*>& traveSegs = _pricingTrx->itin().front()->travelSeg();

    if (!traveSegs.front()->origAirport().empty() &&
        traveSegs.front()->origAirport() == traveSegs.back()->destAirport() &&
        serviceSpecyficRtwProperty())
    {
      _pricingTrx->getOptions()->setRtw(true);
    }
  }
}

bool
CommonRequestHandler::isWPAEReqType() const
{
  return (_reqType == WPAE);
}

bool
CommonRequestHandler::isPostTktReqType() const
{
  return (_reqType == PostTkt);
}

bool
CommonRequestHandler::isWPBGRequest() const
{
  return (_reqType == WPBG);
}

bool
CommonRequestHandler::isNewRBDforAncillaryActive() const
{
  return isNewRBDforWPAEActive() ||
         isNewRBDforAB240Active() ||
         _pricingTrx->activationFlags().isNewRBDbyCabinForM70();
}

bool
CommonRequestHandler::isNewRBDforWPAEActive() const
{
  return !fallback::fallbackNewRBDforWPAE(_pricingTrx) &&
      (isWPAEReqType() || isPostTktReqType());
}

bool
CommonRequestHandler::isBaggageTransaction() const
{
          //AncillaryPricingRequest v2 - ACS baggage
  return (_acsBaggageRequest && !_pricingTrx->activationFlags().isAB240()) ||
        //AncillaryPricingRequest v2 - MISC6 baggage
        _MISC6BaggageRequest ||
        //AncillaryPricingRequest v2 - WP*BG or WPBG* request
        isWPBGRequest() ||
        //xml2 - BaggageRequest
        isBaggageRequest();
}

bool CommonRequestHandler::isNewRBDforAB240Active() const
{
  return !fallback::fallbackNewRBDforAB240(_pricingTrx) &&
      _pricingTrx->activationFlags().isAB240();
}


bool
CommonRequestHandler::isBaggageRequest() const
{
  return (_reqType == M70 && _request->ancRequestType() == AncRequest::BaggageRequest);
}

} // tse
