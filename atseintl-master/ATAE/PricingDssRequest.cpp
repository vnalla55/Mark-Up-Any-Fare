//----------------------------------------------------------------------------
//
//  File   :  PricingDssRequest.cpp
//
//  Author :  Kul Shekhar
//
//  Copyright Sabre 2005
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s) have
//          been supplied.
//
//----------------------------------------------------------------------------

#include "ATAE/PricingDssRequest.h"
#include "ATAE/PricingDssFlightMapBuilder.h"

#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Customer.h"

namespace tse
{
Logger
PricingDssRequest::_logger("atseintl.ATAE.PricingDssRequest");

const std::string PricingDssRequest::DSS_REQUEST = "DSS";
const std::string PricingDssRequest::INDICATOR_TRUE = "true";
const std::string PricingDssRequest::INDICATOR_FALSE = "false";
const std::string PricingDssRequest::DEFAULT_TRX_ID = "12345";
const std::string PricingDssRequest::DEFAULT_USER = "20";
const std::string PricingDssRequest::DEFAULT_ACTION_CODE = "**";

void
PricingDssRequest::addXray(XMLConstruct& construct) const
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
PricingDssRequest::addBilling(XMLConstruct& construct) const
{
  const Billing& billing = *_trx.billing();

  construct.openElement("BIL");

  construct.addAttributeULong("TXN", billing.clientTransactionID()); // obsolete
  construct.addAttributeULong("C00", billing.transactionID());
  construct.addAttributeULong("C01", billing.clientTransactionID());

  construct.addAttribute("UCD", billing.userPseudoCityCode().c_str());
  construct.addAttribute("AAA", billing.aaaCity().c_str());
  construct.addAttribute("UST", billing.userStation().c_str());
  construct.addAttribute("UBR", billing.userBranch().c_str());
  construct.addAttribute("ASI", billing.aaaSine().c_str());

  if (billing.actionCode().empty())
    construct.addAttribute("AKD", DEFAULT_ACTION_CODE.c_str());
  else
    construct.addAttribute("AKD", billing.actionCode().c_str());

  construct.addAttribute("USA", billing.userSetAddress().c_str());
  construct.addAttribute("PID", billing.partitionID().c_str());
  construct.addAttribute("CSV", "SGSCHEDS");
  construct.addAttribute("C20", "SGSCHEDS");
  construct.addAttribute("C21", billing.clientServiceName());
  construct.closeElement();
}

void
PricingDssRequest::addUserId(XMLConstruct& construct) const
{
  if (_trx.getRequest()->ticketingAgent() == nullptr)
  {
    return;
  }

  const Agent& agent = *(_trx.getRequest()->ticketingAgent());
  bool isTvl(isTravelocity());
  construct.openElement("UID");

  if (agent.coHostID() != 0)
  {
    construct.addAttributeShort("UDD", agent.coHostID());
  }
  else
  {
    construct.addAttribute("UDD", DEFAULT_USER);
  }

  construct.addAttribute("OWN", agent.cxrCode().c_str());
  construct.addAttribute("CTY", agent.agentCity().c_str());
  construct.addAttribute("WEB", setWEB(isTvl));
  construct.closeElement();
}

bool
PricingDssRequest::isTravelocity() const
{
  if (_trx.getRequest()->ticketingAgent()->agentTJR() == nullptr)
    return false;

  // Please compare with DSS/DSSRequest.cpp implementation of following functionality
  return _trx.getRequest()->ticketingAgent()->agentTJR()->tvlyLocation() == YES;
}

std::string
PricingDssRequest::setWEB(bool isTvl) const
{
  if (isTvl)
    return INDICATOR_TRUE;
  else
    return INDICATOR_FALSE;
}

void
PricingDssRequest::addAgent(XMLConstruct& construct) const
{
  if (_trx.getRequest()->ticketingAgent() == nullptr)
  {
    return;
  }

  const Agent& agent = *(_trx.getRequest()->ticketingAgent());

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
}
