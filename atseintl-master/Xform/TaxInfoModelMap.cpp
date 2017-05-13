//----------------------------------------------------------------------------
//
//      File: TaxInfoModelMap.cpp
//      Description: Create and interpret Data Model mappings
//      Created: Dec, 2008
//      Authors: Jakub Kubica
//
//  Copyright Sabre 2008
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

#include "Xform/TaxInfoModelMap.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/TseUtil.h"
#include "Common/XMLChString.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"


namespace tse
{
FALLBACK_DECL(neutralToActualCarrierMapping);

static Logger
logger("atseintl.Xform.TaxInfoModelMap");

TaxInfoModelMap::~TaxInfoModelMap()
{
  for (auto& elem : _classMap)
  {
    delete (Mapping*)elem.second;
  }
}

void
TaxInfoModelMap::isTransactionOK()
{
  if (nullptr == _taxTrx)
    throw ErrorResponseException(ErrorResponseException::MEMORY_EXCEPTION, "out of memory");

  if (nullptr == _taxTrx->getRequest())
    throw ErrorResponseException(ErrorResponseException::MEMORY_EXCEPTION, "out of memory");

  if (_taxTrx->requestType() != TAX_INFO_REQUEST)
    throw ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION, "wrong requet type");
}

bool
TaxInfoModelMap::createMap()
{
  typedef tse::ConfigMan::NameValue NV;
  typedef std::vector<NV> NVVector;
  typedef std::vector<NV>::const_iterator NVVectorIter;

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
    for (; iter != classVector.end(); ++iter)
    {
      NV entry = *iter;
      std::string tagName = entry.name;
      Mapping* mapping = new Mapping();

      if (tagName == "TAXINFOREQUEST")
      {
        mapping->func = &tse::TaxInfoModelMap::storeTaxCommonInformation;
        mapping->trxFunc = &tse::TaxInfoModelMap::saveTaxCommonInformation;
      }
      else if (tagName == "AGI")
      {
        mapping->func = &tse::TaxInfoModelMap::storeAgentInformation;
        mapping->trxFunc = &tse::TaxInfoModelMap::saveAgentInformation;
      }
      else if (tagName == "BIL")
      {
        mapping->func = &tse::TaxInfoModelMap::storeBillingInformation;
        mapping->trxFunc = &tse::TaxInfoModelMap::saveBillingInformation;
      }
      else if (tagName == "PRO")
      {
        mapping->func = &tse::TaxInfoModelMap::storeProcOptsInformation;
        mapping->trxFunc = &tse::TaxInfoModelMap::saveProcOptsInformation;
      }
      else if (tagName == "TAX")
      {
        mapping->func = &tse::TaxInfoModelMap::storeTaxInformation;
        mapping->trxFunc = &tse::TaxInfoModelMap::saveTaxInformation;
      }
      else if (tagName == "CRY")
      {
        mapping->func = &tse::TaxInfoModelMap::storeCountryInformation;
        mapping->trxFunc = &tse::TaxInfoModelMap::saveCountryInformation;
      }
      else if (tagName == "APT")
      {
        mapping->func = &tse::TaxInfoModelMap::storeAirportInformation;
        mapping->trxFunc = &tse::TaxInfoModelMap::saveAirportInformation;
      }
      else if (tagName == "DYNAMICCONFIG")
      {
        mapping->func = &tse::TaxInfoModelMap::storeDynamicConfigOverride;
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
        mapping->members[SDBMHash(memberName)] = atoi(memEntry.value.c_str());
      }

      // Set the map
      _classMap[SDBMHash(tagName)] = mapping;
    }
  return true;
}

bool
TaxInfoModelMap::classMapEntry(std::string& tagName, const xercesc::Attributes& atts)
{
  unsigned int hashedTagName = SDBMHash(toUpper(tagName));

  // Find the mapping
  Mapping* mapping = (Mapping*)_classMap[hashedTagName];
  if (mapping && mapping->func)
  {
    _currentMapEntry = (void*)mapping;
    _currentMapEntryStack.push(_currentMapEntry);

    (this->*mapping->func)(tagName, atts);
  }
  else
  {
    LOG4CXX_WARN(logger, "Tag name: " << tagName << ", not mapped!");
    return false;
  }
  return true;
}

void
TaxInfoModelMap::saveMapEntry(std::string& tagName, std::string& text)
{
  unsigned int hashedTagName = SDBMHash(toUpper(tagName));

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

//----------------------------------------------------------------------------
// TaxInfoModelMap::saveMapEntry
// Top of XML message, creating objects
//----------------------------------------------------------------------------
void
TaxInfoModelMap::storeTaxCommonInformation(const std::string& tagName,
                                           const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);

  _dataHandle.get(_taxTrx);
  _trx = _taxTrx;
  _taxTrx->requestType() = TAX_INFO_REQUEST;
  LOG4CXX_DEBUG(logger, "Got _taxTrx of type " << _taxTrx->requestType());

  TaxRequest* request;
  _taxTrx->dataHandle().get(request);
  _taxTrx->setRequest(request);

  TaxInfoRequest* taxInfoRequest;
  _taxTrx->dataHandle().get(taxInfoRequest);
  _taxTrx->taxInfoRequest() = taxInfoRequest;

  // not enough memory
  if (!request || !taxInfoRequest)
    throw ErrorResponseException(ErrorResponseException::MEMORY_EXCEPTION, "out of memory");
}

void
TaxInfoModelMap::saveTaxCommonInformation()
{
  LOG4CXX_DEBUG(logger, "saveTaxCommonInformation");

  if (!_taxTrx->billing())
  {
    LOG4CXX_DEBUG(logger, "no billing information in XML request");
    Billing* billing;
    _taxTrx->dataHandle().get(billing);
    _taxTrx->billing() = billing;
  }

  if (!_taxTrx->getRequest()->ticketingAgent())
  {
    LOG4CXX_DEBUG(logger, "no ticketing agent information in XML request");
  }

  LOG4CXX_DEBUG(logger, "End ");
}

void
TaxInfoModelMap::storeAgentInformation(const std::string& tagName, const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "storeAgentInformation");

  isTransactionOK();

  int numAtts = attrs.getLength();

  Agent* agent = nullptr;

  if (!_taxTrx->getRequest()->ticketingAgent())
  {
    _taxTrx->dataHandle().get(agent);
    // out of memory
    if (!agent)
      throw ErrorResponseException(ErrorResponseException::MEMORY_EXCEPTION, "out of memory");

    _taxTrx->getRequest()->ticketingAgent() = agent;
  }
  else
  {
    agent = _taxTrx->getRequest()->ticketingAgent();
    LOG4CXX_WARN(logger, "double agent information !");
  }

  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // a10
      agent->agentCity() = xmlValue.c_str();
      break;
    case 2: // a20
      agent->tvlAgencyPCC() = xmlValue.c_str();
      break;
    case 3: // a21
    {
      char tmpBuf[10];
      strcpy(tmpBuf, xmlValue.c_str());
      if (tmpBuf[3] == ' ')
        tmpBuf[3] = '\0';
      agent->mainTvlAgencyPCC() = tmpBuf;
    }
    break;
    case 4: // A80 Airline Dept Code
      agent->airlineDept() = xmlValue.c_str();
      break;
    case 5: // A90
      agent->agentFunctions() = xmlValue.c_str();
      break;
    case 6: // AB0
      agent->tvlAgencyIATA() = xmlValue.c_str();
      break;
    case 7: // AB1
      agent->homeAgencyIATA() = xmlValue.c_str();
      break;
    case 8: // B00
      agent->cxrCode() = xmlValue.c_str();
      break;
    case 9: // C40
      agent->currencyCodeAgent() = xmlValue.c_str();
      break;
    case 10: // N0G
      agent->agentDuty() = xmlValue.c_str();
      break;
    case 11: // q01
      agent->coHostID() = atoi(xmlValue.c_str());
      break;
    case 12: // NOG
      agent->agentDuty() = xmlValue.c_str();
      break;
    case 13: // N0L
      agent->agentCommissionType() = xmlValue.c_str();
      break;
    case 14: // C6C
      agent->agentCommissionAmount() = atol(xmlValue.c_str());
      break;
    case 15: // AE1 - Office Designator
      agent->officeDesignator() = xmlValue.c_str();
      break;

    default:
      LOG4CXX_WARN(logger, "AGI - attr: " << xmlValue.c_str() << ", not mapped");
    }
  }
}

void
TaxInfoModelMap::saveAgentInformation()
{
  Agent*& agent = _taxTrx->getRequest()->ticketingAgent();

  if (agent->tvlAgencyPCC().length() == 0)
  {
    std::string tmpString = agent->agentCity();
    agent->agentCity() = agent->mainTvlAgencyPCC();
    agent->mainTvlAgencyPCC() = tmpString;
  }

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
  LOG4CXX_DEBUG(logger, "saveAgentInformation : End ");
}

void
TaxInfoModelMap::storeBillingInformation(const std::string& tagName,
                                         const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "storeBillingInformation");

  isTransactionOK();

  Billing* billing = nullptr;

  if (!_taxTrx->billing())
  {
    _taxTrx->dataHandle().get(billing);
    // if this happens we have serious error in server
    if (!billing)
      throw ErrorResponseException(ErrorResponseException::MEMORY_EXCEPTION, "out of memory");
    _taxTrx->billing() = billing;
  }
  else
  {
    // it this happens we have double billing information, produce warning
    billing = _taxTrx->billing();
    LOG4CXX_WARN(logger, "double billing information !");
  }

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
        _taxTrx->mcpCarrierSwap() = true;
      std::string realCxr;
      // LATAM MCP-S
      if(!fallback::neutralToActualCarrierMapping(_taxTrx) &&
          MCPCarrierUtil::isNeutralCarrier(xmlValue.c_str()))
      {
        realCxr = MCPCarrierUtil::swapFromNeutralToActual(xmlValue.c_str());
        billing->partitionID() = realCxr;
      }
      else
      {
        billing->partitionID() = xmlValue.c_str();
        realCxr = MCPCarrierUtil::swapToActual(_taxTrx, billing->partitionID());
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
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
    } // switch
  } // for

  billing->updateTransactionIds(_taxTrx->transactionId());
  billing->updateServiceNames(Billing::SVC_TAX);
}

void
TaxInfoModelMap::saveBillingInformation()
{
  LOG4CXX_DEBUG(logger, "In saveBillingInformation");
}

void
TaxInfoModelMap::storeTaxInformation(const std::string& tagName, const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "storeTaxInformation");

  isTransactionOK();

  if (!_actualTaxInfoItem)
  {
    // We can have many TAX items in one request, if "save" was called that means that we have to
    // assign new TaxInfoItem, because its whole new "TAX" request inside "TaxInfoResuest"
    TaxInfoItem* newTaxInfoItem;
    _taxTrx->dataHandle().get(newTaxInfoItem);
    _actualTaxInfoItem = newTaxInfoItem;
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // BC0   TAX CODE
      _actualTaxInfoItem->taxCode() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
    }
  }
}

void
TaxInfoModelMap::saveTaxInformation()
{
  // End Tax Section, save collected info and
  // clear pointer
  if (_actualTaxInfoItem)
  {
    _taxTrx->taxInfoRequest()->taxItems().push_back(*_actualTaxInfoItem);
    _actualTaxInfoItem = nullptr;
  }
  LOG4CXX_DEBUG(logger, "In saveTaxInformation");
}

void
TaxInfoModelMap::storeProcOptsInformation(const std::string& tagName,
                                          const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeProcOptsInformation");

  isTransactionOK();
  int minutes;
  int hours;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // D07 - ticketing date override
      _taxTrx->taxInfoRequest()->overrideDate() = convertDate(xmlValue.c_str());
      break;

    case 2: // D54 - ticketing time
      convertTimeHHMM(xmlValue.c_str(), hours, minutes);
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
    }
  }
  _taxTrx->taxInfoRequest()->overrideDate() += Hours(hours);
  _taxTrx->taxInfoRequest()->overrideDate() += Minutes(minutes);
  _taxTrx->dataHandle().setTicketDate(_taxTrx->taxInfoRequest()->overrideDate());
}

void
TaxInfoModelMap::saveProcOptsInformation()
{
  LOG4CXX_DEBUG(logger, "In saveProcOptsInformation");
}

void
TaxInfoModelMap::storeCountryInformation(const std::string& tagName,
                                         const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeCountryInformation");

  isTransactionOK();

  TaxInfoRequest* request = _taxTrx->taxInfoRequest();

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1:
      request->countryCodes().push_back((NationCode)xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
    }
  }
}

void
TaxInfoModelMap::saveCountryInformation()
{
  LOG4CXX_DEBUG(logger, "In saveCountryInformation");
}

void
TaxInfoModelMap::storeAirportInformation(const std::string& tagName,
                                         const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeAirportInformation");

  isTransactionOK();

  if (!_actualTaxInfoItem) // APT is valid only inside TAX section
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "Wrong section in XML");
  }

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // airport, put inside taxitem vector
      _actualTaxInfoItem->airports().push_back(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
    }
  }
}

void
TaxInfoModelMap::saveAirportInformation()
{
  LOG4CXX_DEBUG(logger, "In saveAirportInformation");
}

void
TaxInfoModelMap::storeDynamicConfigOverride(const std::string& /*tagName*/,
                                            const xercesc::Attributes& attrs)
{
  handleDynamicConfig(((Mapping*)_currentMapEntry)->members, attrs);
}

// Utility

const bool
TaxInfoModelMap::convertTimeHHMM(const char* inDate, int32_t& hour, int32_t& min)
{
  if ((inDate == nullptr) || (strlen(inDate) == 0))
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID DATETIME");

  // Expect HH-MM
  const char* cSource = inDate;
  char tmpBuf[5];
  char* target = tmpBuf;

  // Get hour
  *target++ = *cSource++;
  *target++ = *cSource++;
  *target++ = '\0';
  hour = atoi(tmpBuf);

  // Get minutes
  target = tmpBuf;
  *target++ = *cSource++;
  *target++ = *cSource++;
  *target++ = '\0';
  min = atoi(tmpBuf);

  return true;
}
} // tse namespace
