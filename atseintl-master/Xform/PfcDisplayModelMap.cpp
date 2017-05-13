//----------------------------------------------------------------------------
//
//  File:  PfcDisplayModelMap.cpp
//  Description: See PfcDisplayModelMap.h file
//  Created:  May, 2008
//  Authors:  Piotr Lach
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

#include "Xform/PfcDisplayModelMap.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/XMLChString.h"
#include "DataModel/Agent.h"
#include "DataModel/PfcDisplayRequest.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/TaxRequest.h"

#include <boost/lexical_cast.hpp>

#include <set>
#include <string>
#include <vector>

namespace tse
{
FALLBACK_DECL(neutralToActualCarrierMapping);

static Logger
logger("atseintl.Xform.PfcDisplayModelMap");

PfcDisplayModelMap::~PfcDisplayModelMap()
{
  for (auto& elem : _classMap)
  {
    delete (Mapping*)elem.second;
  }
}

bool
PfcDisplayModelMap::createMap()
{
  using NV = tse::ConfigMan::NameValue;
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
      Mapping* mapping = new Mapping();

      // Recognized class tags
      if (tagName == "PFCDISPLAYREQUEST")
      {
        mapping->func = &tse::PfcDisplayModelMap::storePfcDisplayInformation;
        mapping->trxFunc = &tse::PfcDisplayModelMap::savePfcDisplayInformation;
      }
      else if (tagName == "AGI") // Agent Information
      {
        mapping->func = &tse::PfcDisplayModelMap::storeAgentInformation;
        mapping->trxFunc = &tse::PfcDisplayModelMap::saveAgentInformation;
      }
      else if (tagName == "BIL") // Billing Information
      {
        mapping->func = &tse::PfcDisplayModelMap::storeBillingInformation;
        mapping->trxFunc = &tse::PfcDisplayModelMap::saveBillingInformation;
      }
      else if (tagName == "PRO") // Processing Information
      {
        mapping->func = &tse::PfcDisplayModelMap::storeProcOptsInformation;
        mapping->trxFunc = &tse::PfcDisplayModelMap::saveProcOptsInformation;
      }
      else if (tagName == "SGI") // Segment Information
      {
        mapping->func = &tse::PfcDisplayModelMap::storeSegmentInformation;
        mapping->trxFunc = &tse::PfcDisplayModelMap::saveSegmentInformation;
      }
      else if (tagName == "DYNAMICCONFIG")
      {
        mapping->func = &tse::PfcDisplayModelMap::storeDynamicConfigOverride;
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
PfcDisplayModelMap::classMapEntry(std::string& tagName, const xercesc::Attributes& atts)
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
PfcDisplayModelMap::saveMapEntry(std::string& tagName, std::string& text)
{
  unsigned int hashedTagName = SDBMHash(toUpper(tagName));

  // Find the mapping
  Mapping* mapping = (Mapping*)_classMap[hashedTagName];

  // Some mappings don't interact directly with a Trx object
  LOG4CXX_DEBUG(logger, "tag=" << tagName << ", mapping=" << mapping);

  if (mapping && mapping->trxFunc)
  {
    (this->*mapping->trxFunc)(tagName, text);
    _currentMapEntryStack.pop();
    if (_currentMapEntryStack.size() > 0)
    {
      _currentMapEntry = _currentMapEntryStack.top();
    }
  }
}

void
PfcDisplayModelMap::storePfcDisplayInformation(const std::string& tagName,
                                               const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);

  _dataHandle.get(_taxTrx);
  _trx = _taxTrx;
  _taxTrx->requestType() = PFC_DISPLAY_REQUEST;
  LOG4CXX_DEBUG(logger, "Got _taxTrx of type " << _taxTrx->requestType());

  _taxTrx->taxDisplayRequestRootElementType() = tagName;
  LOG4CXX_DEBUG(logger,
                "Got OtaRootElement of type " << _taxTrx->taxDisplayRequestRootElementType());

  // PFC Display Reqest
  PfcDisplayRequest* pfcDisplayRequest;
  _taxTrx->dataHandle().get(pfcDisplayRequest);
  _taxTrx->pfcDisplayRequest() = pfcDisplayRequest;

  TaxRequest* request;
  _taxTrx->dataHandle().get(request);
  request->ticketingDT() = DateTime::localTime();
  _taxTrx->setRequest(request);
  LOG4CXX_DEBUG(logger, "Got _request");

  PricingOptions* options;
  _taxTrx->dataHandle().get(options);
  _taxTrx->setOptions(options);

  LOG4CXX_DEBUG(logger, "Start " << tagName);
  Billing* billing;
  _taxTrx->dataHandle().get(billing);
  _taxTrx->billing() = billing;

  Agent* agent = nullptr;
  _taxTrx->dataHandle().get(agent);
  _taxTrx->pfcDisplayRequest()->ticketingAgent() = agent;
  _taxTrx->getRequest()->ticketingAgent() = agent;
}

void
PfcDisplayModelMap::savePfcDisplayInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

//--------------------------------------------------------------------------
// @function PfcDisplayModelMap::storeBillingInformation
//
// Description: Convenience method for mapping BIL sent XML summary
//              information into the Data Model.  Must have a valid
//              mapping scheme.
//
// @param tagName - No meaning here
// @param attrs - attribute list
// @return void
//--------------------------------------------------------------------------
void
PfcDisplayModelMap::storeBillingInformation(const std::string& tagName,
                                            const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeBillingInformation");

  Billing* billing = _taxTrx->billing();

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
    case 9: // A70 - action code
      billing->actionCode() = xmlValue.c_str();
      break;
    case 10: // C01
      billing->clientTransactionID() = Billing::string2transactionId(xmlValue.c_str());
      break;
    case 11: // C00
      billing->parentTransactionID() = Billing::string2transactionId(xmlValue.c_str());
      break;
    case 12: // C21
      billing->clientServiceName() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger,
                   "storeBillingInformation - Attribute: " << xmlValue.c_str() << ", not mapped");
      break;
    }
  }

  billing->updateTransactionIds(_taxTrx->transactionId());
  billing->updateServiceNames(Billing::SVC_TAX);

  LOG4CXX_DEBUG(logger, "Finished in storeBillingInformation");
}

//--------------------------------------------------------------------------
// @function PfcDisplayModelMap::saveBillingInformation
//
// Description: Convenience method for mapping BIL sent XML summary
//              information into the Data Model.  Must have a valid
//              mapping scheme.
//
// @param tagName - value to be stored away
// @param attrs - attribute list
// @return void
//--------------------------------------------------------------------------
void
PfcDisplayModelMap::saveBillingInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "In saveBillingInformation");
}

//--------------------------------------------------------------------------
// @function PfcDisplayModelMap::storeAgentInformation
//
// Description: Convenience method for mapping AGI sent XML agent
//              information into the response stream.  Must have a valid
//              mapping scheme.
//
// @param tagName - value to be stored away
// @param attrs - attribute list
// @return void
//--------------------------------------------------------------------------
void
PfcDisplayModelMap::storeAgentInformation(const std::string& tagName,
                                          const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeAgentInformation");

  Agent* agent = _taxTrx->pfcDisplayRequest()->ticketingAgent();

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
    {
      char tmpBuf[10];
      strcpy(tmpBuf, xmlValue.c_str());
      if (tmpBuf[3] == ' ')
        tmpBuf[3] = '\0';
      agent->airlineDept() = tmpBuf;
    }
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
    case 14: // AE1 - Office Designator
      agent->officeDesignator() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger,
                   "storeAgentInformation - Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  LOG4CXX_DEBUG(logger, "Finished in storeAgentInformation");
}

//--------------------------------------------------------------------------
// @function PfcDisplayModelMap::saveAgentInformation
//
// Description: Convenience method for saving a populated AGI information
//
// @return void
//--------------------------------------------------------------------------
void
PfcDisplayModelMap::saveAgentInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "In saveAgentInformation");
}

//--------------------------------------------------------------------------
// @function PfcDisplayModelMap::storeProcOptsInformation
//
// Description: Convenience method for mapping PRO sent XML agent
//              information into the response stream.  Must have a valid
//              mapping scheme.
//
// @param tagName - value to be stored away
// @param attrs - attribute list
// @return void
//--------------------------------------------------------------------------
void
PfcDisplayModelMap::storeProcOptsInformation(const std::string& tagName,
                                             const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeProcOptsInformation");

  int numAtts = attrs.getLength();

  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // PXT - Command type
    {
      std::string cmdType = xmlValue.c_str();
      _taxTrx->getOptions()->lineEntry() = "PX" + cmdType;

      if (cmdType == "CH")
        _taxTrx->pfcDisplayRequest()->cmdType() = PfcDisplayRequest::HELP;
      else if (cmdType == "C*")
        _taxTrx->pfcDisplayRequest()->cmdType() = PfcDisplayRequest::PXC;
      else if (cmdType == "CI")
      {
        _taxTrx->pfcDisplayRequest()->cmdType() = PfcDisplayRequest::PXC;
        _taxTrx->pfcDisplayRequest()->isPNR() = true;
      }
      else if (cmdType == "E*")
        _taxTrx->pfcDisplayRequest()->cmdType() = PfcDisplayRequest::PXE;
      else if (cmdType == "M*")
        _taxTrx->pfcDisplayRequest()->cmdType() = PfcDisplayRequest::PXM;
      else if (cmdType == "T*")
        _taxTrx->pfcDisplayRequest()->cmdType() = PfcDisplayRequest::PXT;
      else if (cmdType == "Q*")
        _taxTrx->pfcDisplayRequest()->cmdType() = PfcDisplayRequest::PXQ;
      else if (cmdType == "A*")
        _taxTrx->pfcDisplayRequest()->cmdType() = PfcDisplayRequest::PXA;
      else if (cmdType == "AI")
      {
        _taxTrx->pfcDisplayRequest()->cmdType() = PfcDisplayRequest::PXA;
        _taxTrx->pfcDisplayRequest()->isPNR() = true;
      }
      else if (cmdType == "H*")
        _taxTrx->pfcDisplayRequest()->cmdType() = PfcDisplayRequest::PXH;
      else
        _taxTrx->pfcDisplayRequest()->cmdType() = PfcDisplayRequest::ERROR;
    }
    break;
    case 2: // A50 - Airport/City Code
      _taxTrx->pfcDisplayRequest()->markets().push_back(xmlValue.c_str());
      break;
    case 3: // B05 - Carrier Code 1
      if (MCPCarrierUtil::isPseudoCarrier(xmlValue.c_str()))
      {
        _taxTrx->mcpCarrierSwap() = true;
      }
      // LATAM MCP-S
      if(!fallback::neutralToActualCarrierMapping(_taxTrx) &&
          MCPCarrierUtil::isNeutralCarrier(xmlValue.c_str()))
      {
        _taxTrx->pfcDisplayRequest()->carrier1() =
            MCPCarrierUtil::swapFromNeutralToActual(xmlValue.c_str()); // MCP-S project
      }
      else
      {
        _taxTrx->pfcDisplayRequest()->carrier1() =
            MCPCarrierUtil::swapToActual(_taxTrx, xmlValue.c_str()); // MCP project
        //              _taxTrx->pfcDisplayRequest()->carrier1() = xmlValue.c_str();
      }
      break;
    case 4: // B06 - Carrier Code 2
      if (MCPCarrierUtil::isPseudoCarrier(xmlValue.c_str()))
      {
        _taxTrx->mcpCarrierSwap() = true;
      }
      // LATAM MCP-S
      if(!fallback::neutralToActualCarrierMapping(_taxTrx) &&
          MCPCarrierUtil::isNeutralCarrier(xmlValue.c_str()))
      {
        _taxTrx->pfcDisplayRequest()->carrier2() =
            MCPCarrierUtil::swapFromNeutralToActual(xmlValue.c_str()); // MCP-S project
      }
      else
      {
        _taxTrx->pfcDisplayRequest()->carrier2() =
            MCPCarrierUtil::swapToActual(_taxTrx, xmlValue.c_str()); // MCP project
        //              _taxTrx->pfcDisplayRequest()->carrier2() = xmlValue.c_str();
      }
      break;
    case 5: // D07 - Override Date
      _taxTrx->pfcDisplayRequest()->overrideDate() = convertDate(xmlValue.c_str());
      break;
    case 6: // Q17 - Percentage
    {
      try
      {
        _taxTrx->pfcDisplayRequest()->percentageRate() =
            boost::lexical_cast<PfcDisplayRequest::Percentage>(xmlValue.c_str());
      }
      catch (boost::bad_lexical_cast& e) { _taxTrx->pfcDisplayRequest()->percentageRate() = 0.0; }
    }
    break;
    case 7: // Line number after PXA*
    {
      try
      {
        _taxTrx->pfcDisplayRequest()->absorptionRecordNumber() =
            boost::lexical_cast<uint32_t>(xmlValue.c_str());
      }
      catch (boost::bad_lexical_cast& e)
      {
        _taxTrx->pfcDisplayRequest()->absorptionRecordNumber() = 0;
      }
    }
    break;
    default:
      LOG4CXX_WARN(logger,
                   "storeAgentInformation - Attribute: " << xmlValue.c_str() << ", not mapped");
      break;
    }
  }

  LOG4CXX_DEBUG(logger, "Finished in storeProcOptsInformation");
}

//--------------------------------------------------------------------------
// @function PfcDisplayModelMap::saveProcOptsInformation
//
// Description: Convenience method for saving a populated PRO information
//
// @return void
//--------------------------------------------------------------------------
void
PfcDisplayModelMap::saveProcOptsInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "In saveProcOptsInformation");
}

//--------------------------------------------------------------------------
// @function PfcDisplayModelMap::storeSegmentInformation
//
// Description: Convenience method for mapping SGI sent XML agent
//              information into the response stream.  Must have a valid
//              mapping scheme.
//
// @param tagName - value to be stored away
// @param attrs - attribute list
// @return void
//--------------------------------------------------------------------------
void
PfcDisplayModelMap::storeSegmentInformation(const std::string& tagName,
                                            const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeSegmentInformation");

  constexpr uint32_t WRONG_SEGMENT_NUMBER = 0xFFFFFFFF;
  PfcDisplayRequest::Segment segment;
  std::get<0>(segment) = WRONG_SEGMENT_NUMBER;

  int numAtts = attrs.getLength();

  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // Q0C - Segment Number
    {
      try
      {
        std::get<PfcDisplayRequest::SEGMENT_NUMBER>(segment) =
            boost::lexical_cast<uint32_t>(xmlValue.c_str());
      }
      catch (boost::bad_lexical_cast& e)
      {
        std::get<PfcDisplayRequest::SEGMENT_NUMBER>(segment) = WRONG_SEGMENT_NUMBER;
      }
    }
    break;
    case 2: // A01 - Departure Airport
      std::get<PfcDisplayRequest::DEPARTURE_AIRPORT>(segment) = xmlValue.c_str();
      break;
    case 3: // B00 - Carrier Code
      if (MCPCarrierUtil::isPseudoCarrier(xmlValue.c_str()))
      {
        _taxTrx->mcpCarrierSwap() = true;
      }
      std::get<PfcDisplayRequest::CARRIER_CODE>(segment) =
          MCPCarrierUtil::swapToActual(_taxTrx, xmlValue.c_str()); // MCP project
      //      	std::get<PfcDisplayRequest::CARRIER_CODE>(segment) = xmlValue.c_str();
      break;
    case 4: // D01 - Departure Date
      /*std::get<PfcDisplayRequest::DEPARTURE_DATE>(segment) = convertDate(xmlValue.c_str());*/
      break;
    case 5: // Q0B - Flight Number
      try
      {
        std::get<PfcDisplayRequest::FLIGHT_NUMBER>(segment) =
            boost::lexical_cast<FlightNumber>(xmlValue.c_str());
      }
      catch (boost::bad_lexical_cast& e) { std::get<PfcDisplayRequest::FLIGHT_NUMBER>(segment) = 0; }
      break;
    default:
      LOG4CXX_WARN(logger,
                   "storeSegmentInformation - Attribute: " << xmlValue.c_str() << ", not mapped");
      break;
    }
  }

  _taxTrx->pfcDisplayRequest()->segments().push_back(segment);

  LOG4CXX_DEBUG(logger, "Finished in storeSegmentInformation");
}

//--------------------------------------------------------------------------
// @function PfcDisplayModelMap::saveSegmentInformation
//
// Description: Convenience method for saving a populated SGI information
//
// @return void
//--------------------------------------------------------------------------
void
PfcDisplayModelMap::saveSegmentInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "In saveSegmentInformation");
}

void
PfcDisplayModelMap::storeDynamicConfigOverride(const std::string& /*tagName*/,
                                               const xercesc::Attributes& attrs)
{
  handleDynamicConfig(((Mapping*)_currentMapEntry)->members, attrs);
}
} // tse namespace
