//----------------------------------------------------------------------------
//
//  File:  PricingDisplayModelMap.cpp
//  Description: See PricingDisplayModelMap.h file
//  Created:  January 29, 2005
//  Authors:  Mike Carroll
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
//----------------------------------------------------------------------------

#include "Xform/PricingDisplayModelMap.h"

#include "Common/ErrorResponseException.h"
#include "Common/Logger.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TseUtil.h"
#include "Common/XMLChString.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Customer.h"
#include "DBAccess/MultiTransport.h"

#include <set>
#include <string>
#include <vector>

#include <time.h>

namespace tse
{
static Logger
logger("atseintl.Xform.PricingDisplayModelMap");

PricingDisplayModelMap::~PricingDisplayModelMap()
{
  for (auto& elem : _classMap)
  {
    delete (Mapping*)elem.second;
  }
}

bool
PricingDisplayModelMap::createMap()
{
  using NV = ConfigMan::NameValue;
  using NVVector = std::vector<NV>;
  using NVVectorIter = std::vector<NV>::const_iterator;

  NVVector classVector;
  NVVector memberVector;

  if (_config.getValues(classVector, "Classes") == false)
  {
    LOG4CXX_ERROR(logger, "Cannot fing config section classes");
    return false;
  }

  NVVectorIter iter = classVector.begin();
  if (iter == classVector.end())
  {
    LOG4CXX_ERROR(logger, "No Classes information in configuration file");
    return false;
  }
  else
  {
    for (; iter != classVector.end(); ++iter)
    {
      NV entry = *iter;
      std::string tagName = entry.name;
      // std::string tagValue = entry.value;
      Mapping* mapping = new Mapping();

      // Recognized class tags
      LOG4CXX_INFO(logger, "Tag name: " << tagName);
      if (tagName == "PricingDisplayRequest" || tagName == "PRICINGDISPLAYREQUEST")
      {
        mapping->func = &tse::PricingDisplayModelMap::storePricingDisplayInformation;
        mapping->trxFunc = &tse::PricingDisplayModelMap::savePricingDisplayInformation;
      }
      else if (tagName == "AGI")
      {
        mapping->func = &tse::PricingDisplayModelMap::storeAgentInformation;
        mapping->trxFunc = &tse::PricingDisplayModelMap::saveAgentInformation;
      }
      else if (tagName == "BIL")
      {
        mapping->func = &tse::PricingDisplayModelMap::storeBillingInformation;
        mapping->trxFunc = &tse::PricingDisplayModelMap::saveBillingInformation;
      }
      else if (tagName == "PRO")
      {
        mapping->func = &tse::PricingDisplayModelMap::storeProcOptsInformation;
        mapping->trxFunc = &tse::PricingDisplayModelMap::saveProcOptsInformation;
      }
      else if (tagName == "OPT")
      {
        mapping->func = &tse::PricingDisplayModelMap::storeOptionInformation;
        mapping->trxFunc = &tse::PricingDisplayModelMap::saveOptionInformation;
      }
      else if (tagName == "DYNAMICCONFIG")
      {
        mapping->func = &tse::PricingDisplayModelMap::storeDynamicConfigOverride;
        mapping->trxFunc = nullptr;
      }
      else
      {
        LOG4CXX_ERROR(logger, "Function for class name tag " << tagName << ", not mapped");
        delete mapping;
        return false;
      }

      // Get the members
      std::string memberTag = tagName + "_MEMBERS";
      if (_config.getValues(memberVector, memberTag) == false)
      {
        LOG4CXX_ERROR(logger,
                      "Cannot find config section " << memberTag << " associated with class tag "
                                                    << tagName);
        delete mapping;
        return false;
      }

      // Set the members
      for (NVVectorIter memIter = memberVector.begin(); memIter != memberVector.end(); ++memIter)
      {
        NV memEntry = *memIter;
        std::string memberName = memEntry.name;
        // std::string memberPosition = memEntry.value;
        mapping->members[SDBMHash(memberName)] = atoi(memEntry.value.c_str());
      }

      // Set the map
      _classMap[SDBMHash(tagName)] = mapping;
    }
  }
  return true;
}

bool
PricingDisplayModelMap::classMapEntry(std::string& tagName, const xercesc::Attributes& atts)
{
  unsigned int hashedTagName = SDBMHash(tagName);

  // Find the mapping
  Mapping* mapping = (Mapping*)_classMap[hashedTagName];
  if (mapping && mapping->func)
  {
    _currentMapEntry = (void*)mapping;
    _currentMapEntryStack.push(_currentMapEntry);

    (this->*mapping->func)(atts);
  }
  else
  {
    LOG4CXX_WARN(logger, "Tag name: " << tagName << ", not mapped!");
    return false;
  }
  return true;
}

void
PricingDisplayModelMap::saveMapEntry(std::string& tagName)
{
  unsigned int hashedTagName = SDBMHash(tagName);

  // Find the mapping
  Mapping* mapping = (Mapping*)_classMap[hashedTagName];

  // Some mappings don't interact directly with a Trx object
  LOG4CXX_DEBUG(logger, "tag=" << tagName << ", mapping=" << mapping);

  if (mapping && mapping->trxFunc)
  {
    (this->*mapping->trxFunc)();
    _currentMapEntryStack.pop();
    if (_currentMapEntryStack.size() > 0)
    {
      _currentMapEntry = _currentMapEntryStack.top();
    }
  }
}

void
PricingDisplayModelMap::storePricingDisplayInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storePricingDisplayInformation");

  _dataHandle.get(_pricingTrx);
  PricingRequest* request;
  _pricingTrx->dataHandle().get(request); // lint !e530
  _pricingTrx->setRequest(request);
  _dataHandle.get(_wddAirSeg);
  PricingOptions* options;
  _pricingTrx->dataHandle().get(options); // lint !e530
  _pricingTrx->setOptions(options);
  _pricingTrx->displayOnly() = true;
  _trx = _pricingTrx;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // C10 - Diagnostic number
      request->diagnosticNumber() = (uint16_t)atoi(xmlValue.c_str());
      break;
    case 2: // A01 - Origin city
      _wddAirSeg->boardMultiCity() = xmlValue.c_str();
      _wddAirSeg->origAirport() = xmlValue.c_str();
      break;
    case 3: // A02 - Destination city
      _wddAirSeg->offMultiCity() = xmlValue.c_str();
      _wddAirSeg->destAirport() = xmlValue.c_str();
      break;
    case 4: // B00 - Carrier code
      _wddAirSeg->setOperatingCarrierCode(xmlValue.c_str());
      _wddAirSeg->setMarketingCarrierCode(xmlValue.c_str());
      break;
    case 5: // D15 - Departure date
      _wddAirSeg->departureDT() = convertDateDDMMMYY(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
PricingDisplayModelMap::savePricingDisplayInformation()
{
  LOG4CXX_DEBUG(logger, "In savePricingDisplayInformation");

  if (_pricingTrx->getOptions() == nullptr)
  {
    PricingOptions* options = nullptr;
    _pricingTrx->dataHandle().get(options);
    _pricingTrx->setOptions(options);
  }
  if (_pricingTrx->getRequest()->ticketingAgent() == nullptr)
    _pricingTrx->dataHandle().get(_pricingTrx->getRequest()->ticketingAgent());
  // Create default PaxType
  _pricingTrx->dataHandle().get(_paxType);
  _paxType->paxType() = ADULT;
  _paxType->number() = 1;

  PaxTypeUtil::initialize(*_pricingTrx,
                          *_paxType,
                          _paxType->paxType(),
                          _paxType->number(),
                          _paxType->age(),
                          _paxType->stateCode(),
                          0);
  _pricingTrx->paxType().push_back(_paxType);

  // Fill in locs in the travel segment
  _wddAirSeg->segmentOrder() = 1;
  _wddAirSeg->pnrSegment() = 1;
  _wddAirSeg->origin() =
      _pricingTrx->dataHandle().getLoc(_wddAirSeg->boardMultiCity(), _wddAirSeg->departureDT());
  _wddAirSeg->destination() =
      _pricingTrx->dataHandle().getLoc(_wddAirSeg->offMultiCity(), _wddAirSeg->departureDT());
  _pricingTrx->travelSeg().push_back(_wddAirSeg);

  // Assign offMultiCity and boardMultiCity
  const std::vector<tse::MultiTransport*>& boardMACList =
      _trx->dataHandle().getMultiTransportCity(_wddAirSeg->boardMultiCity(),
                                               _wddAirSeg->carrier(),
                                               _wddAirSeg->geoTravelType(),
                                               _wddAirSeg->departureDT());
  std::vector<tse::MultiTransport*>::const_iterator listIter = boardMACList.begin();

  if (listIter == boardMACList.end())
  {
    LOG4CXX_WARN(logger, "Board multi city vector is empty");
  }
  else
  {
    _wddAirSeg->boardMultiCity() = (**listIter).multitranscity();
  }
  LOG4CXX_DEBUG(logger, "boardMultiCity=" << _wddAirSeg->boardMultiCity().c_str());

  const std::vector<tse::MultiTransport*>& offMACList =
      _trx->dataHandle().getMultiTransportCity(_wddAirSeg->offMultiCity(),
                                               _wddAirSeg->carrier(),
                                               _wddAirSeg->geoTravelType(),
                                               _wddAirSeg->departureDT());
  listIter = offMACList.begin();
  if (listIter == offMACList.end())
  {
    LOG4CXX_WARN(logger, "Off multi city vector is empty");
  }
  else
  {
    // Take the first, they should all be the same
    _wddAirSeg->offMultiCity() = (**listIter).multitranscity();
  }
  LOG4CXX_DEBUG(logger, "offMultiCity=" << _wddAirSeg->offMultiCity());

  // Create an itinerary
  _pricingTrx->dataHandle().get(_itin);
  _pricingTrx->getRequest()->requestedDepartureDT() = _wddAirSeg->departureDT();
  _pricingTrx->getRequest()->ticketingDT() = _wddAirSeg->departureDT();
  _itin->travelSeg().push_back(_wddAirSeg);
  _itin->ticketingCarrier() = _wddAirSeg->operatingCarrierCode();

  // Add the itinerary to the pricing transaction
  _pricingTrx->itin().push_back(_itin);

  // Remaining diagnostic args
  if (!_ruleCategories.empty())
    _pricingTrx->getRequest()->diagArgData().push_back(Diagnostic::RULE_NUMBER + _ruleCategories);
}

void
PricingDisplayModelMap::storeAgentInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeAgentInformation");

  Agent* agent;
  _pricingTrx->dataHandle().get(agent); // lint !e530
  _pricingTrx->getRequest()->ticketingAgent() = agent;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // A10 - Agent city
      agent->agentCity() = xmlValue.c_str();
      break;
    case 2: // A20 - Travel agent PCC
      agent->tvlAgencyPCC() = xmlValue.c_str();
      break;
    case 3: // A21 - Main travel agency PCC
    {
      char tmpBuf[10];
      strcpy(tmpBuf, xmlValue.c_str());
      if (tmpBuf[3] == ' ')
        tmpBuf[3] = '\0';
      agent->mainTvlAgencyPCC() = tmpBuf;
    }
    break;
    case 4: // AB0 - Agency IATA number
      agent->tvlAgencyIATA() = xmlValue.c_str();
      break;
    case 5: // AB1 - Home agency IATA number
      agent->homeAgencyIATA() = xmlValue.c_str();
      break;
    case 6: // A90 - Agent function code
      agent->agentFunctions() = xmlValue.c_str();
      break;
    case 7: // N0G - Agent duty code
      agent->agentDuty() = xmlValue.c_str();
      break;
    case 8: // A80 - Airline department
      agent->airlineDept() = xmlValue.c_str();
      break;
    case 9: // B00 - Originating carrier
      agent->cxrCode() = xmlValue.c_str();
      break;
    case 10: // C40 - Agent currency code
      agent->currencyCodeAgent() = xmlValue.c_str();
      break;
    case 11: // Q01 - coHost ID
      agent->coHostID() = atoi(xmlValue.c_str());
      break;
    case 12: // N0L - Agent commission type
      agent->agentCommissionType() = xmlValue.c_str();
      break;
    case 13: // C6C - Agent commission amount
      agent->agentCommissionAmount() = atol(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
PricingDisplayModelMap::saveAgentInformation()
{
  LOG4CXX_DEBUG(logger, "In saveAgentInformation");

  Agent*& agent = _pricingTrx->getRequest()->ticketingAgent();
  if (agent == nullptr)
  {
    Agent* myAgent;
    _trx->dataHandle().get(myAgent); // lint !e530
    _pricingTrx->getRequest()->ticketingAgent() = myAgent;
  }

  // lint -e661
  // PSS Bug
  if (agent->tvlAgencyPCC().length() == 0)
  {
    std::string tmpString = agent->agentCity();
    agent->agentCity() = agent->mainTvlAgencyPCC();
    agent->mainTvlAgencyPCC() = tmpString;
  }

  // Check customer table first
  std::vector<Customer*> custList = _trx->dataHandle().getCustomer(agent->agentCity());
  if (custList.size() == 0)
  {
    custList = _trx->dataHandle().getCustomer(agent->mainTvlAgencyPCC());
    if (custList.size() == 0)
    {
      std::stringstream msg; // msd
      msg << "Customer record: '" << agent->tvlAgencyPCC() << "' PCC missing!"; // msd
      LOG4CXX_ERROR(logger, msg.str()); // msd

      agent->agentLocation() = _trx->dataHandle().getLoc(agent->agentCity(), time(nullptr));

      if (!agent->agentLocation())
      {
        LOG4CXX_ERROR(logger, msg.str());
        throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
      }
    }
    else
    {
      agent->agentTJR() = custList.front();
      agent->agentLocation() = _trx->dataHandle().getLoc(agent->agentTJR()->aaCity(), time(nullptr));
      if (!agent->agentLocation())
      {
        throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
      }
    }
  }
  else
  {
    agent->agentTJR() = custList.front();
    agent->agentLocation() = _trx->dataHandle().getLoc(agent->agentTJR()->aaCity(), time(nullptr));
    if (!agent->agentLocation())
    {
      throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
    }
  }
  LOG4CXX_DEBUG(logger, "End of saveAgentInformation");
}

void
PricingDisplayModelMap::storeBillingInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeBillingInformation");

  Billing* billing;
  _pricingTrx->dataHandle().get(billing); // lint !e530
  _pricingTrx->billing() = billing;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // A20 - User pseudoCity code
      billing->userPseudoCityCode() = xmlValue.c_str();
      break;
    case 2: // Q03 - User station
      billing->userStation() = xmlValue.c_str();
      break;
    case 3: // Q02 - User branch
      billing->userBranch() = xmlValue.c_str();
      break;
    case 4: // AE0 - Partition ID
      billing->partitionID() = xmlValue.c_str();
      break;
    case 5: // AD0 - User set address
      billing->userSetAddress() = xmlValue.c_str();
      break;
    case 6: // C20 - Parent service name
      billing->parentServiceName() = xmlValue.c_str();
      break;
    case 7: // A22 - aaaCity
      billing->aaaCity() = xmlValue.c_str();
      break;
    case 8: // AA0 - agentSine
      billing->aaaSine() = xmlValue.c_str();
      break;
    case 9: // A70 - Action code
      billing->actionCode() = xmlValue.c_str();
      break;
    case 10: // C01
      billing->clientTransactionID() = Billing::string2transactionId(xmlValue.c_str());
      break;
    case 11: // C00
      billing->parentTransactionID() = Billing::string2transactionId(xmlValue.c_str());
      break;
    case 12: // C21 - Not parse it
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  billing->updateTransactionIds(_pricingTrx->transactionId());
  billing->updateServiceNames(Billing::SVC_PRICING_DISPLAY);
}

void
PricingDisplayModelMap::saveBillingInformation()
{
  LOG4CXX_DEBUG(logger, "In saveBillingInformation");
}

void
PricingDisplayModelMap::storeOptionInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeOptionInformation");

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // D15 - Sales date
      _wddAirSeg->bookingDT() = convertDateDDMMMYY(xmlValue.c_str());
      break;
    case 2: // S30 - Fare type code
      _pricingTrx->getRequest()->diagArgData().push_back(Diagnostic::FARE_TYPE + xmlValue.c_str());
      break;
    case 3: // S54 - Fare designator
      _pricingTrx->getRequest()->diagArgData().push_back(Diagnostic::FARE_TYPE_DESIGNATOR +
                                                         xmlValue.c_str());
      break;
    case 4: // A60 - Global direction
      _pricingTrx->getRequest()->diagArgData().push_back(Diagnostic::GLOBAL_DIRECTION +
                                                         xmlValue.c_str());
      break;
    case 5: // S55 - Footnote
      _pricingTrx->getRequest()->diagArgData().push_back(Diagnostic::FOOTNOTE + xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
PricingDisplayModelMap::saveOptionInformation()
{
  LOG4CXX_DEBUG(logger, "In saveOptionInformation");
}

void
PricingDisplayModelMap::storeDynamicConfigOverride(const xercesc::Attributes& attrs)
{
  handleDynamicConfig(((Mapping*)_currentMapEntry)->members, attrs);
}
} // tse namespace
