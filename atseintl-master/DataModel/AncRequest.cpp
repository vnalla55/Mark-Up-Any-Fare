//-------------------------------------------------------------------
//
//  File:        AncRequest.cpp
//  Created:
//  Authors:
//
//  Description:
//
//  Updates:
//          10/28/10 - Jayanthi Shyam Mohan - file created.
//
//  Copyright Sabre 2004
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

#include "DataModel/AncRequest.h"

#include "Common/FallbackUtil.h"
#include "DataModel/Agent.h"

#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>

namespace tse
{
AncRequest::AncRequest()
{
  setActiveAgent(CheckInAgent);
}

std::vector<PaxType*>
AncRequest::paxType(const Itin* itin) const
{
  std::vector<PaxType*> ret;
  std::map<const Itin*, std::vector<PaxType*> >::const_iterator it = _paxTypesPerItin.find(itin);

  if (!_multiTicket && it != _paxTypesPerItin.end())
  {
    std::copy(it->second.begin(), it->second.end(), std::back_inserter(ret));
  }

  std::map<const Itin*, std::vector<Itin*> >::const_iterator dit = _pricingItins.find(itin);
  if (dit != _pricingItins.end())
  {
    for (const auto elem : dit->second)
    {
      it = _paxTypesPerItin.find(elem);
      if (it != _paxTypesPerItin.end())
        std::copy(it->second.begin(), it->second.end(), std::back_inserter(ret));
    }
  }
  return ret;
}

void
AncRequest::AncAttrMapTree::addAttribute(const std::string& name,
                                         const std::string& value,
                                         int hashIndex)
{
  _attrs[name] = std::make_pair(value, hashIndex);
}
size_t
AncRequest::AncAttrMapTree::getHash(int index) const
{
  size_t ret = 0;
  std::map<std::string, std::pair<std::string, int> >::const_iterator at = _attrs.begin();
  for (; at != _attrs.end(); at++)
  {
    if ((*at).second.second == index)
      boost::hash_combine(ret, (*at).second.first);
  }
  std::map<std::string, AncAttrMapTree>::const_iterator it = _childs.begin();
  std::map<std::string, AncAttrMapTree>::const_iterator ie = _childs.end();
  for (; it != ie; it++)
  {
    size_t hashChild = it->second.getHash(index);
    if (hashChild != 0)
      boost::hash_combine(ret, hashChild);
  }
  return ret;
}
AncRequest::AncAttrMapTree::AncAttrMapTree(const AncAttrMapTree& r)
{
  _attrs = r._attrs;
  _childs = r._childs;
}
const std::string&
AncRequest::AncAttrMapTree::getAttributeValue(const std::string& name)
{
  return _attrs[name].first;
}
AncRequest::AncAttrMapTree&
AncRequest::AncAttrMapTree::
operator[](const std::string& name)
{
  return _childs[name];
}
AncRequest::AncAttrMapTree&
AncRequest::AncAttrMapTree::
operator[](int name)
{
  return _childs[boost::lexical_cast<std::string>(name)];
}

Agent*&
AncRequest::ticketingAgent()
{
  // WARNING: This function changes behavior:
  // it can return one of the ticketing agents or the checkin agent according
  // to how it is configured using AncRequest::setActiveAgent
  // FIXME Refactor this ASAP
  return (*_activeAgent) ? *_activeAgent : _checkInAgent;
}

const Agent*
AncRequest::ticketingAgent() const
{
  // WARNING: This function is changes behavior:
  // it can return one of the ticketing agents or the checkin agent according
  // to how it is configured using AncRequest::setActiveAgent
  // FIXME Refactor this ASAP
  return (*_activeAgent) ? *_activeAgent : _checkInAgent;
}

Itin*&
AncRequest::subItin(uint16_t groupNum)
{
  std::map<uint16_t, Itin*>::iterator itr =
    _subItins.find(groupNum);

  if ( itr != _subItins.end())
    return (*itr).second;

  return _masterItin;
}

const Itin*
AncRequest::subItin(uint16_t groupNum) const
{
  const std::map<uint16_t, Itin*>::const_iterator itr =
    _subItins.find(groupNum);

  if ( itr != _subItins.end())
    return (*itr).second;

  return _masterItin;
}

bool
AncRequest::isPostTktRequest() const
{
  return (_ancRequestType == AncRequest::PostTktRequest);
}

bool
AncRequest::isWPBGRequest() const
{
  return (_ancRequestType == AncRequest::WPBGRequest);
}

bool
AncRequest::isWPAERequest() const
{
  return (_ancRequestType == AncRequest::WPAERequest);
}

std::string
AncRequest::getDefaultTicketingCarrierFromTicketingAgent(uint8_t ticketNumber) const
{
  std::string gdsCodeFromRequest;
  setActiveAgent(BagReqAgent::TicketingAgent, ticketNumber);
  gdsCodeFromRequest = ticketingAgent()->defaultTicketingCarrier(); // TAG/@AE3
  setActiveAgent(BagReqAgent::CheckInAgent);
  return gdsCodeFromRequest;
}

std::string
AncRequest::getAirlineCarrierCodeFromTicketingAgent(uint8_t ticketNumber) const
{
  std::string gdsCodeFromRequest;
  setActiveAgent(BagReqAgent::TicketingAgent, ticketNumber);
  gdsCodeFromRequest = ticketingAgent()->cxrCode(); // TAG/@B00
  setActiveAgent(BagReqAgent::CheckInAgent);
  return gdsCodeFromRequest;
}

}
