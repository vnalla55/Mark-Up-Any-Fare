//----------------------------------------------------------------------------
//
//
//  Copyright Sabre 2003
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

#include "DSS/DSSRequest.h"

#include "Common/DateTime.h"
#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "Common/XMLConstruct.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Customer.h"
#include "DSS/PreferredDateCalculator.h"
#include "Xray/JsonMessage.h"

#include <algorithm>
#include <sstream>

namespace tse
{
static Logger
logger("atseintl.DSS.DSSRequest");

const std::string DSSRequest::DSS_REQUEST = "DSS";
const std::string DSSRequest::CITY_CODE = "C";
const std::string DSSRequest::AREA_CODE = "A";
const std::string DSSRequest::INDICATOR_TRUE = "true";
const std::string DSSRequest::INDICATOR_FALSE = "false";
const std::string DSSRequest::NOT_APPLICABLE = EMPTY_STRING();
const std::string DSSRequest::DEFAULT_MAX_STOP = "15";
const std::string DSSRequest::TWO_SEG = "2";
const std::string DSSRequest::THREE_SEG = "3";
const std::string DSSRequest::MAX_SOLUTION = "100000";
const std::string DSSRequest::ANY_NUMBER = "12345";
const std::string DSSRequest::DEFAULT_ZERO = "0";
const std::string DSSRequest::DEFAULT_USER = "34";

DSSRequest::DSSRequest(FareDisplayTrx& trx)
  : _trx(trx), _geoTravelType(trx.itin().front()->geoTravelType())
{
}

void
DSSRequest::build(FareDisplayTrx& trx, std::set<CarrierCode>& carrierList, std::string& request)
    const
{
  LOG4CXX_INFO(logger, " Entered DSSRequest::buildRequest()");
  XMLConstruct construct;
  TravelSeg* tvlSeg = trx.travelSeg().front();
  FareDisplayRequest* rq = trx.getRequest();
  FareDisplayOptions* op = trx.getOptions();
  bool isWebUser(isTravelocity());
  construct.openElement(DSS_REQUEST);
  addXrayRecord(construct);
  addBillingRecord(*trx.billing(), construct);
  addAgentRecord(*(rq->ticketingAgent()), isWebUser, construct);
  addTravelSegRecord(*tvlSeg, construct);
  addDateRecord(tvlSeg->departureDT(), *rq, *op, isWebUser, construct);
  addTimeRecord(construct);
  addFlightRecord(construct);
  addEntryTypeRecord(construct);
  addConnectTimeRecord(construct);
  addSegmentRecord(construct, isWebUser);
  addStopRecord(construct);
  addCarrierListRecord(construct, carrierList);
  addFlightCountRecord(isWebUser, construct);
  construct.closeElement();
  request = construct.getXMLData();
  LOG4CXX_INFO(logger, " Returning DSSRequest::buildRequest()");
}

void
DSSRequest::addXrayRecord(XMLConstruct& construct) const
{
  xray::JsonMessage* jsonMessage = _trx.getXrayJsonMessage();
  if (jsonMessage == nullptr)
    return;
  construct.openElement("XRA");
  construct.addAttribute("MID", jsonMessage->generateMessageId("DSS").c_str());
  construct.addAttribute("CID", jsonMessage->generateConversationId().c_str());
  construct.closeElement();
}

void
DSSRequest::addBillingRecord(const Billing& billing, XMLConstruct& construct) const
{
  construct.openElement("BIL");
  construct.addAttribute("TXN", ANY_NUMBER);
  construct.addAttribute("UCD", billing.userPseudoCityCode().c_str());
  construct.addAttribute("AAA", billing.aaaCity().c_str());
  construct.addAttribute("UST", billing.userStation().c_str());
  construct.addAttribute("UBR", billing.userBranch().c_str());
  construct.addAttribute("ASI", billing.aaaSine().c_str());
  construct.addAttribute("AKD", billing.actionCode().c_str());
  construct.addAttribute("USA", billing.userSetAddress().c_str());
  construct.addAttribute("PID", billing.partitionID().c_str());
  construct.addAttribute("CSV", "SGSCHEDS");
  construct.closeElement();
}

void
DSSRequest::addAgentRecord(const Agent& agent, bool isTvl, XMLConstruct& construct) const
{
  construct.openElement("UID");
  if (agent.agentTJR())
  {
    std::ostringstream tmp;
    tmp << agent.agentTJR()->ssgGroupNo();
    construct.addAttribute("UDD", tmp.str());
  }
  else
  {
    construct.addAttribute("UDD", DEFAULT_USER);
  }
  construct.addAttribute("OWN", agent.cxrCode().c_str());
  construct.addAttribute("CTY", agent.agentCity().c_str());
  construct.addAttribute("WEB", setWEB(isTvl));
  construct.closeElement();

  construct.openElement("AGI");

  if (agent.officeDesignator().size() >= 5)
  {
    construct.addAttribute("A11", agent.officeDesignator().substr(0, 3));
    construct.addAttribute("B11", agent.officeDesignator().substr(3, 2));
  }

  if ((!agent.officeStationCode().empty()))
  {
    construct.addAttribute("C11", agent.officeStationCode());
  }

  if ((!agent.defaultTicketingCarrier().empty()))
  {
    construct.addAttribute("D11", agent.defaultTicketingCarrier());
  }

  construct.closeElement();
}

void
DSSRequest::addTravelSegRecord(const TravelSeg& tvlSeg, XMLConstruct& construct) const
{
  construct.openElement("ODI");
  construct.addAttribute("BRD", tvlSeg.origin()->loc().c_str());
  std::string btpType = (tvlSeg.origin()->cityInd() == true ? CITY_CODE : AREA_CODE);
  construct.addAttribute("BTP", btpType.c_str());
  construct.addAttribute("OFF", tvlSeg.destination()->loc().c_str());
  std::string ctpType = (tvlSeg.destination()->cityInd() == true ? CITY_CODE : AREA_CODE);
  construct.addAttribute("OTP", ctpType.c_str());
  construct.closeElement();
}

void
DSSRequest::addDateRecord(const DateTime& date,
                          FareDisplayRequest& request,
                          FareDisplayOptions& options,
                          bool isWebUser,
                          XMLConstruct& construct) const
{
  uint16_t upperRange(0);
  uint16_t lowerRange(0);
  DateTime travelDate(date);

  if (isWebUser)
  {
    PreferredDateCalculator::calculate(request, upperRange, lowerRange);
    travelDate = request.preferredTravelDate() == DateTime::emptyDate()
                     ? travelDate
                     : request.preferredTravelDate();
    LOG4CXX_DEBUG(logger, " Processing DSSRequest For WebUser");
  }

  construct.openElement("DAT");
  if (upperRange != 0)
  {
    std::ostringstream tmp;
    tmp << upperRange;
    construct.addAttribute("BK1", tmp.str());
  }
  else
  {
    construct.addAttribute("BK1", DEFAULT_ZERO);
  }

  if (lowerRange != 0)
  {
    std::ostringstream tmp;
    tmp << lowerRange;
    construct.addAttribute("OUT", tmp.str());
  }
  else
  {
    construct.addAttribute("OUT", DEFAULT_ZERO);
  }

  std::string dateToString;
  if (travelDate == DateTime::emptyDate())
  {
    LOG4CXX_DEBUG(logger, " Processing DSSRequest For Non-WebUser With Date Range");
    travelDate = request.dateRangeLower();
    dateToString = travelDate.dateToString(YYYYMMDD, "-");
  }
  else
  {
    dateToString = travelDate.dateToString(YYYYMMDD, "-");
  }
  construct.addAttribute("TG1", dateToString);
  construct.closeElement();
}

void
DSSRequest::addTimeRecord(XMLConstruct& construct) const
{
  construct.openElement("TIM");
  construct.addAttribute("BK2", "360");
  construct.addAttribute("OUT", "1080");
  construct.addAttribute("TGT", "360");
  construct.closeElement();
}

void
DSSRequest::addFlightRecord(XMLConstruct& construct) const
{
  construct.openElement("FLT");
  construct.addAttribute("SOL", MAX_SOLUTION);
  construct.closeElement();
}

void
DSSRequest::addEntryTypeRecord(XMLConstruct& construct) const
{
  construct.openElement("ENT");
  construct.addAttribute("TYP", "FD");
  construct.addAttribute("DAT", "");
  construct.closeElement();
}

void
DSSRequest::addConnectTimeRecord(XMLConstruct& construct) const
{
  construct.openElement("CNX");
  construct.addAttribute("CMN", DEFAULT_ZERO);
  construct.addAttribute("CMX", DEFAULT_ZERO);
  construct.closeElement();
}

void
DSSRequest::addSegmentRecord(XMLConstruct& construct, bool isWebUser) const
{
  construct.openElement("SEG");
  construct.addAttribute("SMN", DEFAULT_ZERO);
  construct.addAttribute("SMX", isWebUser ? THREE_SEG : TWO_SEG);
  construct.closeElement();
}

void
DSSRequest::addStopRecord(XMLConstruct& construct) const
{
  construct.openElement("STP");
  construct.addAttribute("TMN", DEFAULT_ZERO);
  construct.addAttribute("TMX", DEFAULT_MAX_STOP);
  construct.closeElement();
}

void
DSSRequest::addFlightCountRecord(bool isTravelocity, XMLConstruct& construct) const
{
  construct.openElement("FCT");
  construct.addAttribute("IIC", setIIC(isTravelocity));
  construct.addAttribute("IOD", setIOD(isTravelocity));
  construct.closeElement();
}

bool
DSSRequest::isTravelocity() const
{
  if (_trx.getRequest()->ticketingAgent()->agentTJR() == nullptr)
    return false;

  return _trx.getRequest()->ticketingAgent()->agentTJR()->tvlyLocation() == YES;
}

//----------------------------------------------------------------------------
// DSSRequest::setIOD() : IOD -- Indicator Online Double Connection
//----------------------------------------------------------------------------
std::string
DSSRequest::setIOD(bool isTvl) const
{
  return isTvl ? INDICATOR_TRUE : INDICATOR_FALSE;
}

//----------------------------------------------------------------------------
// DSSRequest::setIIC() : IIC -- Indicator Interline Connection
//----------------------------------------------------------------------------
std::string
DSSRequest::setIIC(bool isTvl) const
{
  if (isTvl && ((_geoTravelType == GeoTravelType::International) ||
                (_geoTravelType == GeoTravelType::ForeignDomestic)))
    return INDICATOR_TRUE;
  else
    return INDICATOR_FALSE;
}

std::string
DSSRequest::setWEB(bool isTvl) const
{
  return isTvl ? INDICATOR_TRUE : INDICATOR_FALSE;
}

void
DSSRequest::addCarrierListRecord(XMLConstruct& construct, std::set<CarrierCode>& carrierList) const
{
  construct.openElement("CRR");
  construct.addAttribute("CRL", getCarrierString(carrierList));
  construct.closeElement();
}

std::string
DSSRequest::getCarrierString(std::set<CarrierCode>& cxrs) const
{
  std::string emptyString = " ";
  std::string cxrList;
  cxrList.clear();
  std::set<CarrierCode>::iterator i(cxrs.begin()), end(cxrs.end());

  for (; i != end; ++i)
  {
    if ((*i) == INDUSTRY_CARRIER)
      continue;

    if (i != cxrs.begin())
      cxrList += emptyString;
    cxrList += *i;
  }
  return cxrList;
}
}
