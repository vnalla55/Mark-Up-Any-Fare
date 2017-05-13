//----------------------------------------------------------------------------
//
//  File:  TaxDisplayModelMap.cpp
//  Description: See TaxDisplayModelMap.h file
//  Created:  August, 2007
//  Authors:  Dean Van Decker
//
//  Copyright Sabre 2007
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

#include "Xform/TaxDisplayModelMap.h"

#include "Common/FallbackUtil.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/XMLChString.h"
#include "DataModel/Agent.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/TariffCrossRefInfo.h"

#include <boost/lexical_cast.hpp>

#include <set>
#include <string>

namespace tse
{
FIXEDFALLBACK_DECL(saveAtpcoTaxDisplayData);
FALLBACK_DECL(neutralToActualCarrierMapping);

static Logger
logger("atseintl.Xform.TaxDisplayModelMap");

TaxDisplayModelMap::~TaxDisplayModelMap()
{
  for (auto& elem : _classMap)
  {
    delete (Mapping*)elem.second;
  }
}

bool
TaxDisplayModelMap::createMap()
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
      if (tagName == "TAXDISPLAYRQ" || tagName == "AIRTAXDISPLAYRQ")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeTaxInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveTaxInformation;
      }
      else if (tagName == "NATION")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeNationInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveNationInformation;
      }
      else if (tagName == "AIRPORT")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeAirportInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveAirportInformation;
      }
      else if (tagName == "TAXCODE")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeTaxCodeInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveTaxCodeInformation;
      }
      else if (tagName == "TAXTYPE")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeTaxTypeInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveTaxTypeInformation;
      }
      else if (tagName == "CARRIERCODE")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeCarrierCodeInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveCarrierCodeInformation;
      }
      else if (tagName == "SEQUENCE")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeSequenceInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveSequenceInformation;
      }
      else if (tagName == "CATEGORY")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeCategoryInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveCategoryInformation;
      }
      else if (tagName == "RETRIEVALDATE")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeRetrievalDateInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveRetrievalDateInformation;
      }
      else if (tagName == "HISTORYDATE")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeHistoryDateInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveHistoryDateInformation;
      }
      else if (tagName == "MENU")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeMenuInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveMenuInformation;
      }
      else if (tagName == "REISSUE")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeReissueInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveReissueInformation;
      }
      else if (tagName == "TAXHELP")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeTaxHelpInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveTaxHelpInformation;
      }
      else if (tagName == "USTAXHELP")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeUSTaxHelpInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveUSTaxHelpInformation;
      }
      else if (tagName == "CALCULATETAX")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeCalculateTaxInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveCalculateTaxInformation;
      }
      else if (tagName == "SOURCE")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeSourceInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveSourceInformation;
      }
      else if (tagName == "TPA_EXTENSIONS")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeTPAExtensionsInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveTPAExtensionsInformation;
      }
      else if (tagName == "USERINFO")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeUserInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveUserInformation;
      }
      else if (tagName == "STATION")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeStationInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveStationInformation;
      }
      else if (tagName == "BRANCH")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeBranchInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveBranchInformation;
      }
      else if (tagName == "PARTITION")
      {
        mapping->func = &tse::TaxDisplayModelMap::storePartitionInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::savePartitionInformation;
      }
      else if (tagName == "SETADDRESS")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeSetAddressInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveSetAddressInformation;
      }
      else if (tagName == "SERVICE")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeServiceInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveServiceInformation;
      }
      else if (tagName == "CLIENTSERVICE")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeClientServiceInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveClientServiceInformation;
      }
      else if (tagName == "PARENTSERVICE")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeParentServiceInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveParentServiceInformation;
      }
      else if (tagName == "AAACITY")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeAAACityInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveAAACityInformation;
      }
      else if (tagName == "AGENTSINE")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeAgentSineInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveAgentSineInformation;
      }
      else if (tagName == "ACTION")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeActionInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveActionInformation;
      }
      else if (tagName == "TRANSACTION")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeTransactionInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveTransactionInformation;
      }
      else if (tagName == "CLIENTTRANSACTION")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeClientTransactionInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveClientTransactionInformation;
      }
      else if (tagName == "PARENTTRANSACTION")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeParentTransactionInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveParentTransactionInformation;
      }
      else if (tagName == "OFFICEDESIGNATOR")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeOfficeDesignatorInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveOfficeDesignatorInformation;
      }
      else if (tagName == "DYNAMICCONFIG")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeDynamicConfigOverride;
        mapping->trxFunc = nullptr;
      }
      else if (tagName == "TXENTRY")
      {
        mapping->func = &tse::TaxDisplayModelMap::storeTxEntryInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveTxEntryInformation;
      }
      else if (tagName == "CARRIERCODES" && !fallback::fixed::saveAtpcoTaxDisplayData())
      {
        mapping->func = &tse::TaxDisplayModelMap::storeCarrierCodesInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveCarrierCodesInformation;
      }
      else if (tagName == "DETAILLEVELS" && !fallback::fixed::saveAtpcoTaxDisplayData())
      {
        mapping->func = &tse::TaxDisplayModelMap::storeDetailLevelsInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveDetailLevelsInformation;
      }
      else if (tagName == "REQUESTDATE" && !fallback::fixed::saveAtpcoTaxDisplayData())
      {
        mapping->func = &tse::TaxDisplayModelMap::storeRequestDateInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveRequestDateInformation;
      }
      else if (tagName == "TRAVELDATE" && !fallback::fixed::saveAtpcoTaxDisplayData())
      {
        mapping->func = &tse::TaxDisplayModelMap::storeTravelDateInformation;
        mapping->trxFunc = &tse::TaxDisplayModelMap::saveTravelDateInformation;
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
TaxDisplayModelMap::classMapEntry(std::string& tagName, const xercesc::Attributes& atts)
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
TaxDisplayModelMap::saveMapEntry(std::string& tagName, std::string& text)
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
TaxDisplayModelMap::storeTaxInformation(const std::string& tagName,
                                        const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);

  _dataHandle.get(_taxTrx);
  _trx = _taxTrx;

  _taxTrx->taxDisplayRequestRootElementType() = tagName;
  LOG4CXX_DEBUG(logger,
                "Got OtaRootElement of type " << _taxTrx->taxDisplayRequestRootElementType());

  TaxRequest* request;
  _taxTrx->dataHandle().get(request);
  request->ticketingDT() = DateTime::localTime();
  _taxTrx->setRequest(request);
  LOG4CXX_DEBUG(logger, "Got _request");

  Agent* agent = nullptr;
  _taxTrx->dataHandle().get(agent);
  _taxTrx->getRequest()->ticketingAgent() = agent;

  PricingOptions* options;
  _taxTrx->dataHandle().get(options);
  _taxTrx->setOptions(options);
  LOG4CXX_DEBUG(logger, "Got options");

  LOG4CXX_DEBUG(logger, "Start " << tagName);
  Billing* billing;
  _taxTrx->dataHandle().get(billing);
  _taxTrx->billing() = billing;

  _taxTrx->billing()->updateTransactionIds(_taxTrx->transactionId());
}

void
TaxDisplayModelMap::saveTaxInformation(const std::string& tagName, const std::string& text)
{
  if (TrxUtil::isAtpcoTaxesDisplayEnabled(*_taxTrx) &&
      !_taxTrx->getRequest()->txEntryType().empty())
  {
    _taxTrx->requestType() = ATPCO_DISPLAY_REQUEST;
  }
  else
  {
    _taxTrx->requestType() = DISPLAY_REQUEST;
  }

  LOG4CXX_DEBUG(logger, "Got _taxTrx of type " << _taxTrx->requestType());

  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::storeNationInformation(const std::string& tagName,
                                           const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Nation Code
      _taxTrx->getRequest()->nation() = xmlValue.c_str();
      _taxTrx->getRequest()->nationDescription() = xmlValue.c_str();
      break;
    case 2: // Nation Name
      _taxTrx->getRequest()->nationName() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeAirportInformation(const std::string& tagName,
                                            const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    int value = static_cast<Mapping*>(_currentMapEntry)->members[SDBMHash(toUpper(xmlString))];
    switch (value)
    {
    case 1: // Code
      _taxTrx->getRequest()->airportCode() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }

}

void
TaxDisplayModelMap::storeTaxCodeInformation(const std::string& tagName,
                                            const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // TaxCode
      _taxTrx->getRequest()->taxCode() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeTaxTypeInformation(const std::string& tagName,
                                            const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    int value = static_cast<Mapping*>(_currentMapEntry)->members[SDBMHash(toUpper(xmlString))];
    switch (value)
    {
    case 1: // Code
      _taxTrx->getRequest()->taxType() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeCarrierCodeInformation(const std::string& tagName,
                                                const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Carrier Code
    {
      std::string carrierCode = xmlValue.c_str();
      if(TrxUtil::isAtpcoTaxesDisplayEnabled(*_taxTrx) &&
         carrierCode.substr(0, 3) == "CXR")
      {
        carrierCode.erase(0, 3);
      }

      _taxTrx->getRequest()->carrierCode() = carrierCode;
    }
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeSequenceInformation(const std::string& tagName,
                                             const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // specific sequence request
      _taxTrx->getRequest()->sequenceNumber() = atoi(xmlValue.c_str());
      break;
    case 2: // compare sequence request
      _taxTrx->getRequest()->sequenceNumber2() = atoi(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeCategoryInformation(const std::string& tagName,
                                             const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Category Number
      _taxTrx->getRequest()->categoryVec().push_back(atoi(xmlValue.c_str()));
      break;
    case 2: // CatList
      _taxTrx->getRequest()->categories() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeHistoryDateInformation(const std::string& tagName,
                                                const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // History Date
      try { _taxTrx->getRequest()->effectiveDate() = convertDate(xmlValue.c_str()); }
      catch (...)
      {
        _taxTrx->response().str(EMPTY_STRING()); //
        _taxTrx->response() << "INVALID DATE - MODIFY AND REENTER";
        throw ErrorResponseException(ErrorResponseException::INVALID_INPUT);
      }
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeRetrievalDateInformation(const std::string& tagName,
                                                  const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Retrieval Date
      _taxTrx->getRequest()->travelDate() = convertDate(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeMenuInformation(const std::string& tagName,
                                         const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Display Menu
      _taxTrx->getRequest()->menu() = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeReissueInformation(const std::string& tagName,
                                            const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Display Reissue Info
      _taxTrx->getRequest()->setReissue(TypeConvert::pssCharToBool(xmlValue.c_str()[0]));
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeTaxHelpInformation(const std::string& tagName,
                                            const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Display Main Help Info
      _taxTrx->getRequest()->setHelp(TypeConvert::pssCharToBool(xmlValue.c_str()[0]));
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeUSTaxHelpInformation(const std::string& tagName,
                                              const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Display US Help Information
      _taxTrx->getRequest()->setHelp(TypeConvert::pssCharToBool(xmlValue.c_str()[0]));
      _taxTrx->getRequest()->helpTaxUS() = TypeConvert::pssCharToBool(xmlValue.c_str()[0]);
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeCalculateTaxInformation(const std::string& tagName,
                                                 const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // fareAmount Information
      _taxTrx->getRequest()->fareAmt() = atof(xmlValue.c_str());
      break;
    case 2: // fareAmountType Information
      _taxTrx->getRequest()->amtType() = xmlValue.c_str()[0];
      break;
    case 3: // Location Information
      _taxTrx->getRequest()->loc1() = xmlValue.c_str();
      break;
    case 4: // Location Information
      _taxTrx->getRequest()->loc2() = xmlValue.c_str();
      break;
    case 5: // Trip Type Information
      _taxTrx->getRequest()->tripType() = xmlValue.c_str()[0];
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeSourceInformation(const std::string& tagName,
                                           const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // User pseudoCity Code
      _taxTrx->billing()->userPseudoCityCode() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeTPAExtensionsInformation(const std::string& tagName,
                                                  const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxDisplayModelMap::storeUserInformation(const std::string& tagName,
                                         const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxDisplayModelMap::storeStationInformation(const std::string& tagName,
                                            const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // User Station
      _taxTrx->billing()->userStation() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeBranchInformation(const std::string& tagName,
                                           const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // User branch
      _taxTrx->billing()->userBranch() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storePartitionInformation(const std::string& tagName,
                                              const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Partition ID
    {
      if (MCPCarrierUtil::isPseudoCarrier(xmlValue.c_str()))
        _taxTrx->mcpCarrierSwap() = true;
      std::string realCxr;
      // LATAM MCP-S
      if(!fallback::neutralToActualCarrierMapping(_taxTrx) &&
          MCPCarrierUtil::isNeutralCarrier(xmlValue.c_str()))
      {
        realCxr = MCPCarrierUtil::swapFromNeutralToActual(xmlValue.c_str());
        _taxTrx->billing()->partitionID() = realCxr;
      }
      else
      {
        _taxTrx->billing()->partitionID() = xmlValue.c_str();
        realCxr = MCPCarrierUtil::swapToActual(_taxTrx, _taxTrx->billing()->partitionID());
      }
      if (MCPCarrierUtil::isIAPCarrierRestricted(realCxr))
        throw ErrorResponseException(
            ErrorResponseException::MCP_IAP_RESTRICTED,
            ("UNABLE TO PROCESS-ENTRY RESTRICTED IN " + realCxr + " PARTITION").c_str());
      break;
    }
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeSetAddressInformation(const std::string& tagName,
                                               const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxDisplayModelMap::storeServiceInformation(const std::string& tagName,
                                            const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Service Name
      //_taxTrx->billing()->serviceName() = xmlValue.c_str();
      _taxTrx->billing()->serviceName() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeClientServiceInformation(const std::string& tagName,
                                                  const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Client Service Name
      _taxTrx->billing()->clientServiceName() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeParentServiceInformation(const std::string& tagName,
                                                  const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Parent Service Name
      _taxTrx->billing()->parentServiceName() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeAAACityInformation(const std::string& tagName,
                                            const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // aaaCity1
      _taxTrx->billing()->aaaCity() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeAgentSineInformation(const std::string& tagName,
                                              const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // agentSine
      _taxTrx->billing()->aaaSine() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeActionInformation(const std::string& tagName,
                                           const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Action code
      _taxTrx->billing()->actionCode() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeTransactionInformation(const std::string& tagName,
                                                const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Transaction ID
      _taxTrx->billing()->transactionID() = Billing::string2transactionId(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeClientTransactionInformation(const std::string& tagName,
                                                      const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Client Transaction ID
      _taxTrx->billing()->clientTransactionID() = Billing::string2transactionId(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeParentTransactionInformation(const std::string& tagName,
                                                      const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Parent Transaction ID
      _taxTrx->billing()->parentTransactionID() = Billing::string2transactionId(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeOfficeDesignatorInformation(const std::string& tagName,
                                                     const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Office Designator Code
      _taxTrx->getRequest()->ticketingAgent()->officeDesignator() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeDynamicConfigOverride(const std::string& /*tagName*/,
                                               const xercesc::Attributes& attrs)
{
  handleDynamicConfig(((Mapping*)_currentMapEntry)->members, attrs);
}

void
TaxDisplayModelMap::storeTxEntryInformation(const std::string& tagName,
                                            const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    int value = static_cast<Mapping*>(_currentMapEntry)->members[SDBMHash(toUpper(xmlString))];
    switch (value)
    {
    case 1: // Command
      _taxTrx->getRequest()->txEntryCmd() = xmlValue.c_str();
      break;
    case 2: // Type
      _taxTrx->getRequest()->txEntryType() = xmlValue.c_str();
      break;
    case 3: // DetailLevels
      _taxTrx->getRequest()->txEntryDetailLevels() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeCarrierCodesInformation(const std::string& tagName, const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    int value = static_cast<Mapping*>(_currentMapEntry)->members[SDBMHash(toUpper(xmlString))];
    switch (value)
    {
    case 1: // Code
      _taxTrx->getRequest()->carrierCodes() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeDetailLevelsInformation(const std::string& tagName, const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    int value = static_cast<Mapping*>(_currentMapEntry)->members[SDBMHash(toUpper(xmlString))];
    switch (value)
    {
    case 1: // Levels
      _taxTrx->getRequest()->txEntryDetailLevels() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeRequestDateInformation(const std::string& tagName, const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    int value = static_cast<Mapping*>(_currentMapEntry)->members[SDBMHash(toUpper(xmlString))];
    switch (value)
    {
    case 1: // Code
      _taxTrx->getRequest()->requestDate() = convertDate(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxDisplayModelMap::storeTravelDateInformation(const std::string& tagName, const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    int value = static_cast<Mapping*>(_currentMapEntry)->members[SDBMHash(toUpper(xmlString))];
    switch (value)
    {
    case 1: // Code
      _taxTrx->getRequest()->travelDate() = convertDate(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}


void
TaxDisplayModelMap::saveNationInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveAirportInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveTaxCodeInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveTaxTypeInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveCarrierCodeInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveCategoryInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxDisplayModelMap::saveHistoryDateInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxDisplayModelMap::saveRetrievalDateInformation(const std::string& tagName,
                                                 const std::string& text)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxDisplayModelMap::saveSequenceInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxDisplayModelMap::saveMenuInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxDisplayModelMap::saveCalculateTaxInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxDisplayModelMap::saveReissueInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxDisplayModelMap::saveTaxHelpInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxDisplayModelMap::saveUSTaxHelpInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxDisplayModelMap::saveSourceInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveTPAExtensionsInformation(const std::string& tagName,
                                                 const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveUserInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveStationInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveBranchInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::savePartitionInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveSetAddressInformation(const std::string& tagName, const std::string& text)
{
  _taxTrx->billing()->userSetAddress() = text;
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveServiceInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveClientServiceInformation(const std::string& tagName,
                                                 const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveParentServiceInformation(const std::string& tagName,
                                                 const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveAAACityInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveAgentSineInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveActionInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveTransactionInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveClientTransactionInformation(const std::string& tagName,
                                                     const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveParentTransactionInformation(const std::string& tagName,
                                                     const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveOfficeDesignatorInformation(const std::string& tagName,
                                                    const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveTxEntryInformation(const std::string& tagName, const std::string& /*text*/)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxDisplayModelMap::saveCarrierCodesInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveDetailLevelsInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveRequestDateInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxDisplayModelMap::saveTravelDateInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

} // tse namespace
