//----------------------------------------------------------------------------
//
//  File:  CurrencyModelMap.cpp
//  Description: See CurrencyModelMap.h file
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

#include "Xform/CurrencyModelMap.h"

#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TseUtil.h"
#include "Common/XMLChString.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"

#include <set>
#include <string>
#include <vector>

#include <time.h>

namespace tse
{
FALLBACK_DECL(neutralToActualCarrierMapping);

static Logger
logger("atseintl.Xform.CurrencyModelMap");

CurrencyModelMap::~CurrencyModelMap()
{
  for (auto& elem : _classMap)
  {
    delete (Mapping*)elem.second;
  }
}

bool
CurrencyModelMap::createMap()
{
  using NV = ConfigMan::NameValue;
  using NVVector = std::vector<NV>;
  using NVVectorIter = std::vector<NV>::const_iterator;

  NVVector classVector;
  NVVector memberVector;

  if (_config.getValues(classVector, "Classes") == false)
  {
    LOG4CXX_ERROR(logger, "Cannot find config section classes");
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
      if (tagName == "CURRENCYCONVERSIONREQUEST")
      {
        mapping->func = &tse::CurrencyModelMap::storeCurrencyInformation;
        mapping->trxFunc = &tse::CurrencyModelMap::saveCurrencyInformation;
      }
      else if (tagName == "AGI")
      {
        mapping->func = &tse::CurrencyModelMap::storeAgentInformation;
        mapping->trxFunc = &tse::CurrencyModelMap::saveAgentInformation;
      }
      else if (tagName == "BIL")
      {
        mapping->func = &tse::CurrencyModelMap::storeBillingInformation;
        mapping->trxFunc = &tse::CurrencyModelMap::saveBillingInformation;
      }
      else if (tagName == "PRO")
      {
        mapping->func = &tse::CurrencyModelMap::storeProcOptsInformation;
        mapping->trxFunc = &tse::CurrencyModelMap::saveProcOptsInformation;
      }
      else if (tagName == "OPT")
      {
        mapping->func = &tse::CurrencyModelMap::storeOptionInformation;
        mapping->trxFunc = &tse::CurrencyModelMap::saveOptionInformation;
      }
      else if (tagName == "DYNAMICCONFIG")
      {
        mapping->func = &tse::CurrencyModelMap::storeDynamicConfigOverride;
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
CurrencyModelMap::classMapEntry(std::string& tagName, const xercesc::Attributes& atts)
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
CurrencyModelMap::saveMapEntry(std::string& tagName)
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
CurrencyModelMap::storeCurrencyInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeCurrencyInformation");

  _dataHandle.get(_currencyTrx);
  PricingRequest* request;
  // lint -e{530}
  _currencyTrx->dataHandle().get(request);
  _currencyTrx->setRequest(request);
  _trx = _currencyTrx;
}

void
CurrencyModelMap::saveCurrencyInformation()
{
  LOG4CXX_DEBUG(logger, "In saveCurrencyInformation");
}

void
CurrencyModelMap::storeAgentInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeAgentInformation");

  Agent* agent;
  // lint -e{530}
  _currencyTrx->dataHandle().get(agent);
  _currencyTrx->getRequest()->ticketingAgent() = agent;

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
    case 15: // AE0 - Vendor CRS Code
      // LATAM MCP-S
      if(!fallback::neutralToActualCarrierMapping(_currencyTrx) &&
          MCPCarrierUtil::isNeutralCarrier(xmlValue.c_str()))
        agent->vendorCrsCode() = MCPCarrierUtil::swapFromNeutralToActual(xmlValue.c_str());
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
CurrencyModelMap::saveAgentInformation()
{
  LOG4CXX_DEBUG(logger, "In saveAgentInformation");

  LocCode agentCity;

  Agent*& agent = _currencyTrx->getRequest()->ticketingAgent();
  if (agent == nullptr)
  {
    Agent* myAgent;
    // lint -e{530}
    _trx->dataHandle().get(myAgent);
    _currencyTrx->getRequest()->ticketingAgent() = myAgent;
  }

  // lint --e{661}
  // PSS Bug
  if (agent->tvlAgencyPCC().length() == 0)
  {
    std::string tmpString = agent->agentCity();
    agent->agentCity() = agent->mainTvlAgencyPCC();
    agent->mainTvlAgencyPCC() = tmpString;
  }

  // Check customer table first
  std::vector<Customer*> custList = _trx->dataHandle().getCustomer(agent->tvlAgencyPCC());

  if (custList.size() == 0)
  {
    agent->agentLocation() = _trx->dataHandle().getLoc(agent->agentCity(), time(nullptr));

    if (!agent->agentLocation())
    {
      if (agent->tvlAgencyPCC().size() == 4)
      {
        std::stringstream msg;
        msg << "Customer record: '" << agent->tvlAgencyPCC() << "' AGENT_PCC_NON_EXISTENT";
        LOG4CXX_ERROR(logger, msg.str());
      }
      throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
    }
  }
  else
  {
    agent->agentTJR() = custList.front();

    agentCity = _trx->dataHandle().getMultiTransportCity(agent->agentCity());

    agent->agentLocation() = _trx->dataHandle().getLoc(agentCity, time(nullptr));

    if (!agent->agentLocation())
    {
      agent->agentLocation() = _trx->dataHandle().getLoc(agent->agentCity(), time(nullptr));

      if (!agent->agentLocation())
        throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
    }
  }
  LOG4CXX_DEBUG(logger, "End of saveAgentInformation");
}

void
CurrencyModelMap::updateAgentInformation()
{
  LOG4CXX_DEBUG(logger, "In updateAgentInformation");

  if (_currencyTrx->baseDT() < _currencyTrx->transactionStartTime())
    return;

  Agent* agent = _currencyTrx->getRequest()->ticketingAgent();

  if (!agent || !agent->agentLocation())
    return;

  if (!LocUtil::isFormerNetherlandsAntilles(agent->agentLocation()->nation()))
    return;

  DataHandle& dataHandle = _trx->dataHandle();

  DateTime ticketDT = dataHandle.ticketDate();
  dataHandle.setTicketDate(_currencyTrx->baseDT());

  const Loc* agentLocation = dataHandle.getLoc(agent->agentCity(), _currencyTrx->baseDT());

  dataHandle.setTicketDate(ticketDT);

  if (agentLocation)
    agent->agentLocation() = agentLocation;
}

void
CurrencyModelMap::storeBillingInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeBillingInformation");

  Billing* billing;
  // lint -e{530}
  _currencyTrx->dataHandle().get(billing);
  _currencyTrx->billing() = billing;

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
        _currencyTrx->mcpCarrierSwap() = true;
      std::string realCxr;
      // LATAM MCP-S
      if(!fallback::neutralToActualCarrierMapping(_currencyTrx) &&
          MCPCarrierUtil::isNeutralCarrier(xmlValue.c_str()))
      {
        realCxr = MCPCarrierUtil::swapFromNeutralToActual(xmlValue.c_str());
        billing->partitionID() = realCxr;
      }
      else
      {
        billing->partitionID() = xmlValue.c_str();
        realCxr = MCPCarrierUtil::swapToActual(_currencyTrx, billing->partitionID());
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
    case 6: // C20 - Service name
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
    case 13: // S0R - Pricing Orchestrator request Path
      billing->requestPath() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  billing->updateTransactionIds(_currencyTrx->transactionId());
  billing->updateServiceNames(Billing::SVC_CURRENCY);
}

void
CurrencyModelMap::saveBillingInformation()
{
  LOG4CXX_DEBUG(logger, "In saveBillingInformation");
}

void
CurrencyModelMap::storeOptionInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeOptionInformation");

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // C52 - Amount
      _currencyTrx->amount() = (double)atof(xmlValue.c_str());
      _currencyTrx->amountStr() = xmlValue.c_str();
      break;
    case 2: // A40 - Country code local/base
      _currencyTrx->baseCountry() = xmlValue.c_str();
      break;
    case 3: // C46 - Source currency
      _currencyTrx->sourceCurrency() = xmlValue.c_str();
      break;
    case 4: // C42 - Target currency
      _currencyTrx->targetCurrency() = xmlValue.c_str();
      break;
    case 5: // D00 - Base date(historical back date) (ddmmmyy)

      _currencyTrx->baseDT() = getHistoricalDate(xmlValue.c_str());

      break;
    case 6: // PAH - Reciprocal
      _currencyTrx->reciprocal() = xmlValue.c_str()[0];
      break;
    case 7: // N1F - Command type
      _currencyTrx->commandType() = xmlValue.c_str()[0];
      break;
    case 8: // PAG - BSR keyword
      _currencyTrx->eprBDK() = xmlValue.c_str()[0];
      break;
    case 9: // D01 - Local PCC date(YYYY-MM-DD)
      _currencyTrx->pssLocalDate() = convertDate(xmlValue.c_str());
      break;

    case 10: // N1Q - DC Alpha Char
      _currencyTrx->dcAlphaChar() = xmlValue.c_str()[0];
      break;
    case 11: // N1V - Request Type (DC or FC)
      _currencyTrx->requestType() = xmlValue.c_str()[0];
      break;
    case 12: // N1W - Future Date indicator
      _currencyTrx->futureDateInd() = xmlValue.c_str()[0];
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
CurrencyModelMap::saveOptionInformation()
{
  updateAgentInformation();

  LOG4CXX_DEBUG(logger, "In saveOptionInformation");
}

void
CurrencyModelMap::storeProcOptsInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeProcOptionInformation");

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // C10 - Diagnostic number
      _currencyTrx->getRequest()->diagnosticNumber() = atoi(xmlValue.c_str());
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
CurrencyModelMap::saveProcOptsInformation()
{
  LOG4CXX_DEBUG(logger, "In saveProcOptionInformation");
}

void
CurrencyModelMap::storeDynamicConfigOverride(const xercesc::Attributes& attrs)
{
  handleDynamicConfig(((Mapping*)_currentMapEntry)->members, attrs);
}

DateTime
CurrencyModelMap::getHistoricalDate(const char* dateStr)
{

  DateTime tmpDate = convertDateDDMMMYY(dateStr);

  DateTime historicalDate(tmpDate.year(), tmpDate.month(), tmpDate.day(), 23, 59, 59);

  return historicalDate;
}
} // tse namespace
