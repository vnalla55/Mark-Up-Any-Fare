//----------------------------------------------------------------------------
//
//  File:  MileageModelMap.cpp
//  Description: See MileageModelMap.h file
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

#include "Xform/MileageModelMap.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TseUtil.h"
#include "Common/XMLChString.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"

#include <set>
#include <string>
#include <vector>

#include <time.h>

namespace tse
{
FALLBACK_DECL(neutralToActualCarrierMapping);

static Logger
logger("atseintl.Xform.MileageModelMap");

MileageModelMap::~MileageModelMap()
{
  for (auto& elem : _classMap)
  {
    delete (Mapping*)elem.second;
  }
}

bool
MileageModelMap::createMap()
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
      if (tagName == "MILEAGEREQUEST")
      {
        mapping->func = &tse::MileageModelMap::storeMileageInformation;
        mapping->trxFunc = &tse::MileageModelMap::saveMileageInformation;
      }
      else if (tagName == "AGI")
      {
        mapping->func = &tse::MileageModelMap::storeAgentInformation;
        mapping->trxFunc = &tse::MileageModelMap::saveAgentInformation;
      }
      else if (tagName == "BIL")
      {
        mapping->func = &tse::MileageModelMap::storeBillingInformation;
        mapping->trxFunc = &tse::MileageModelMap::saveBillingInformation;
      }
      else if (tagName == "PRO")
      {
        mapping->func = &tse::MileageModelMap::storeProcOptsInformation;
        mapping->trxFunc = &tse::MileageModelMap::saveProcOptsInformation;
      }
      else if (tagName == "OPT")
      {
        mapping->func = &tse::MileageModelMap::storeOptionInformation;
        mapping->trxFunc = &tse::MileageModelMap::saveOptionInformation;
      }
      else if (tagName == "WNI")
      {
        mapping->func = &tse::MileageModelMap::storeItemInformation;
        mapping->trxFunc = &tse::MileageModelMap::saveItemInformation;
      }
      else if (tagName == "DIG")
      {
        mapping->func = &tse::MileageModelMap::storeDiagInformation;
        mapping->trxFunc = &tse::MileageModelMap::saveDiagInformation;
      }
      else if (tagName == "DYNAMICCONFIG")
      {
        mapping->func = &tse::MileageModelMap::storeDynamicConfigOverride;
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
MileageModelMap::classMapEntry(std::string& tagName, const xercesc::Attributes& atts)
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
MileageModelMap::saveMapEntry(std::string& tagName)
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
MileageModelMap::storeMileageInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeMileageInformation");

  _dataHandle.get(_mileageTrx);
  PricingRequest* request;
  _mileageTrx->dataHandle().get(request); // lint !e530
  _mileageTrx->setRequest(request);
  _trx = _mileageTrx;
}

void
MileageModelMap::saveMileageInformation()
{
  LOG4CXX_DEBUG(logger, "In saveMileageInformation");
}

void
MileageModelMap::storeAgentInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeAgentInformation");

  Agent* agent = _mileageTrx->getRequest()->ticketingAgent();
  if (agent == nullptr)
  {
    _mileageTrx->dataHandle().get(agent); // lint !e530
    _mileageTrx->getRequest()->ticketingAgent() = agent;
  }

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
      agent->coHostID() = static_cast<int16_t>(atoi(xmlValue.c_str()));
      break;
    case 12: // N0L - Agent commission type
      agent->agentCommissionType() = xmlValue.c_str();
      break;
    case 13: // C6C - Agent commission amount
      agent->agentCommissionAmount() = static_cast<int32_t>(atol(xmlValue.c_str()));
      break;
    case 15: // AE0 - Vendor CRS Code
      if (MCPCarrierUtil::isPseudoCarrier(xmlValue.c_str()))
      {
        _mileageTrx->mcpCarrierSwap() = true;
      }
      // LATAM MCP-S
      if(!fallback::neutralToActualCarrierMapping(_mileageTrx) &&
          MCPCarrierUtil::isNeutralCarrier(xmlValue.c_str()))
      {
        agent->vendorCrsCode() = MCPCarrierUtil::swapFromNeutralToActual(xmlValue.c_str());
      }
      else
        agent->vendorCrsCode() = xmlValue.c_str();
      break;
    case 16: // AE1 - Office Designator
      agent->officeDesignator() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
MileageModelMap::saveAgentInformation()
{
  LOG4CXX_DEBUG(logger, "In saveAgentInformation");

  Agent*& agent = _mileageTrx->getRequest()->ticketingAgent();
  if (agent == nullptr)
  {
    Agent* myAgent;
    _trx->dataHandle().get(myAgent); // lint !e530
    _mileageTrx->getRequest()->ticketingAgent() = myAgent;
  }

  //
  // Per marketing requirement the Customer Record can ONLY use agent city and tvlAgencyPCC
  // Home Psuedo City(Z360) is not Valid for branch(PCC-2N72)
  //
  // Change to use multiTranport table after the conversation with Vladi, Darrin and Gary
  // on 9-27 due to PL 10328
  //
  // For PCC-TP4A located near EAP airport that serves two nations
  // the POS needs to be correct nation

  //

  LocCode agentCity;

  const Loc* agentLocation = _trx->dataHandle().getLoc(agent->agentCity(), time(nullptr));

  std::vector<Customer*> custList = _trx->dataHandle().getCustomer(agent->tvlAgencyPCC());

  if (custList.empty())
  {
    if (agent->tvlAgencyPCC().size() == 4)
    {
      std::stringstream msg;
      msg << "Customer record: '" << agent->tvlAgencyPCC() << "' AGENT_PCC_NON_EXISTENT";
      LOG4CXX_ERROR(logger, msg.str());
    }
    agent->agentLocation() = agentLocation;

    if (!agent->agentLocation())
    {
      throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
    }
    LOG4CXX_DEBUG(logger, "End of saveAgentInformation");
    return;
  }

  agent->agentTJR() = custList.front();

  agentCity = _trx->dataHandle().getMultiTransportCity(agent->agentCity());
  agent->agentLocation() = _trx->dataHandle().getLoc(agentCity, time(nullptr));

  if (agent->agentLocation() && agentLocation)
  {
    if (agent->agentLocation()->nation() != agentLocation->nation())
      agent->agentLocation() = agentLocation;
  }

  if (!agent->agentLocation())
    agent->agentLocation() = agentLocation;

  if (!agent->agentLocation())
    throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
  LOG4CXX_DEBUG(logger, "End of saveAgentInformation");
}

void
MileageModelMap::storeBillingInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeBillingInformation");

  Billing* billing;
  _mileageTrx->dataHandle().get(billing); // lint !e530
  _mileageTrx->billing() = billing;

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
    {
      if (MCPCarrierUtil::isPseudoCarrier(xmlValue.c_str()))
      {
        _mileageTrx->mcpCarrierSwap() = true;
      }
      std::string realCxr;
      // LATAM MCP-S
      if(!fallback::neutralToActualCarrierMapping(_mileageTrx) &&
          MCPCarrierUtil::isNeutralCarrier(xmlValue.c_str()))
      {
        realCxr = MCPCarrierUtil::swapFromNeutralToActual(xmlValue.c_str());
        billing->partitionID() = realCxr;
      }
      else
      {
        billing->partitionID() = xmlValue.c_str();
        realCxr = MCPCarrierUtil::swapToActual(_mileageTrx, billing->partitionID());
      }
      if (MCPCarrierUtil::isIAPCarrierRestricted(realCxr))
        throw ErrorResponseException(
            ErrorResponseException::MCP_IAP_RESTRICTED,
            ("UNABLE TO PROCESS-ENTRY RESTRICTED IN " + realCxr + " PARTITION").c_str());
      break;
    }
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
    case 13: // S0R - Pricing Orchestrator request path
      billing->requestPath() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  billing->updateTransactionIds(_mileageTrx->transactionId());
  billing->updateServiceNames(Billing::SVC_MILEAGE);
}

void
MileageModelMap::saveBillingInformation()
{
  LOG4CXX_DEBUG(logger, "In saveBillingInformation");
}

void
MileageModelMap::storeOptionInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeOptionInformation");

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // D01 - Input date
      _mileageTrx->inputDT() = convertDateDDMMMYY(xmlValue.c_str());

      ///@TODO this is shortcut for now. normally travelDate
      // should be received from PSS for both - "short" and
      // "long" WN entries.                     VN - 3/14/06
      _mileageTrx->travelDT() = _mileageTrx->inputDT();
      _mileageTrx->dataHandle().setTicketDate(_mileageTrx->travelDT());
      break;
    case 2: // PBL - is WN from itinerary
      if (xmlValue.c_str()[0] == 'T')
        _mileageTrx->isFromItin() = true;
      else
        _mileageTrx->isFromItin() = false;
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
MileageModelMap::saveOptionInformation()
{
  LOG4CXX_DEBUG(logger, "In saveOptionInformation");
}

void
MileageModelMap::storeItemInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeItemInformation");

  _trx->dataHandle().get(_mileageItem);

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // Q41 - Item number
      _mileageItem->itemNumber = (uint16_t)atoi(xmlValue.c_str());
      break;
    case 2: // A11 - City code
      _mileageItem->cityLoc = _trx->dataHandle().getLoc(xmlValue.c_str(), time(nullptr));
      if (!_mileageItem->cityLoc)
      {
        Loc* loc;
        _trx->dataHandle().get(loc);
        _mileageItem->cityLoc = loc;
        loc->loc() = xmlValue.c_str();
      }
      break;
    case 3: // B00 - Carrier code
      _mileageItem->carrierCode = MCPCarrierUtil::swapToActual(_mileageTrx, xmlValue.c_str());
      break;
    case 4: // Q0G - Stopover indicator
      if (xmlValue.c_str()[0] == 'T')
        _mileageItem->stopType = StopType::StopOver;
      else
        _mileageItem->stopType = StopType::Connection;
      break;
    case 5: // P2M - Non-ticketed point
      if (xmlValue.c_str()[0] == 'T')
        _mileageItem->isHidden = true;
      else
        _mileageItem->isHidden = false;
      break;
    case 6: // A60 - TPM global direction
      _mileageItem->tpmGlobalDirection = xmlValue.c_str();
      break;
    case 7: // A18 - Location
      LOG4CXX_INFO(logger, "Not storing A18");
      break;
    case 8: // PAI - Arunk indicator
      if (xmlValue.c_str()[0] == 'T')
        _mileageItem->isSurface = true;
      else
        _mileageItem->isSurface = false;
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
MileageModelMap::saveItemInformation()
{
  LOG4CXX_DEBUG(logger, "In saveItemInformation");
  if (_mileageItem != nullptr)
  {
    _mileageTrx->items().push_back(_mileageItem);
    _mileageItem = nullptr;
  }
}

void
MileageModelMap::storeProcOptsInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeProcOptsInformation");
  PricingRequest* request = nullptr;
  int16_t diagNum;

  if (_mileageTrx != nullptr)
    request = _mileageTrx->getRequest();
  if (request == nullptr)
  {
    LOG4CXX_WARN(logger, "Request is NULL in Trx object");
    return;
  }

  Agent*& agent = request->ticketingAgent();
  if (agent == nullptr)
  {
    Agent* tmpAgent;
    _mileageTrx->dataHandle().get(tmpAgent); // lint !e530
    _mileageTrx->getRequest()->ticketingAgent() = tmpAgent;
    agent = tmpAgent;
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // S13 -

      break;

    case 2: // C10 - #diagnostic
      diagNum = static_cast<int16_t>(atoi(xmlValue.c_str()));
      if (diagNum != 197)
        request->diagnosticNumber() = diagNum;
      break;

    case 3: // AE0 - PCC Host Carrier
      // LATAM MCP-S
      if(!fallback::neutralToActualCarrierMapping(_mileageTrx) &&
          MCPCarrierUtil::isNeutralCarrier(xmlValue.c_str()))
        agent->hostCarrier() = MCPCarrierUtil::swapFromNeutralToActual(xmlValue.c_str());
      else
        agent->hostCarrier() = xmlValue.c_str();
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
MileageModelMap::saveProcOptsInformation()
{
  LOG4CXX_DEBUG(logger, "In saveProcOptsInformation");
}

void
MileageModelMap::storeDiagInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeDiagInformation");

  PricingRequest* request = nullptr;

  if (_mileageTrx != nullptr)
    request = _mileageTrx->getRequest();
  if (request == nullptr)
  {
    LOG4CXX_WARN(logger, "Request is NULL in Trx object");
    return;
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // QOA - Diag Argument Type
      request->diagArgType().push_back(xmlValue.c_str());
      break;
    case 2: // S01 - Diag Argument Data
      request->diagArgData().push_back(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
MileageModelMap::saveDiagInformation()
{
  LOG4CXX_DEBUG(logger, "In saveDiagInformation");
}

void
MileageModelMap::storeDynamicConfigOverride(const xercesc::Attributes& attrs)
{
  handleDynamicConfig(((Mapping*)_currentMapEntry)->members, attrs);
}
} // tse namespace
