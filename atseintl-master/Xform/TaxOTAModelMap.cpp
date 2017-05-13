//----------------------------------------------------------------------------
//
//  File:  TaxOTAModelMap.cpp
//  Description: See TaxOTAModelMap.h file
//  Created:  September, 2006
//  Authors:  Hitha Alex
//
//  Copyright Sabre 2006
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

#include "Xform/TaxOTAModelMap.h"

#include "Common/FallbackUtil.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TseUtil.h"
#include "Common/XMLChString.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/MultiTransport.h"
#include "DBAccess/Nation.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "Xray/XrayUtil.h"

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include <set>
#include <string>
#include <vector>

#include <time.h>

namespace tse
{
FALLBACK_DECL(neutralToActualCarrierMapping);
FIXEDFALLBACK_DECL(xrayTaxOTAModelMapTagsParsing);


const int MAJOR_VERSION_POS = 0;
const int MINOR_VERSION_POS = 1;

static Logger
logger("atseintl.Xform.TaxOTAModelMap");

TaxOTAModelMap::~TaxOTAModelMap()
{
  for (auto& elem : _classMap)
  {
    delete (Mapping*)elem.second;
  }
}

bool
TaxOTAModelMap::createMap()
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
      // std::string tagValue = entry.value;
      Mapping* mapping = new Mapping();

      // Recognized class tags
      if (tagName == "TAXRQ" || tagName == "AIRTAXRQ")
      {
        mapping->func = &tse::TaxOTAModelMap::storeTaxInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveTaxInformation;
      }
      else if (tagName == "PROCESSINGOPTIONS")
      {
        mapping->func = &tse::TaxOTAModelMap::storeProcessingOptionsInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveProcessingOptionsInformation;
      }
      else if (tagName == "POS")
      {
        mapping->func = &tse::TaxOTAModelMap::storePointOfSaleInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::savePointOfSaleInformation;
      }
      else if (tagName == "SOURCE")
      {
        mapping->func = &tse::TaxOTAModelMap::storeSourceInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveSourceInformation;
      }
      else if (tagName == "TPA_EXTENSIONS")
      {
        mapping->func = &tse::TaxOTAModelMap::storeTPAExtensionsInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveTPAExtensionsInformation;
      }
      else if (tagName == "USERINFO")
      {
        mapping->func = &tse::TaxOTAModelMap::storeUserInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveUserInformation;
      }
      else if (tagName == "STATION")
      {
        mapping->func = &tse::TaxOTAModelMap::storeStationInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveStationInformation;
      }
      else if (tagName == "BRANCH")
      {
        mapping->func = &tse::TaxOTAModelMap::storeBranchInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveBranchInformation;
      }
      else if (tagName == "PARTITION")
      {
        mapping->func = &tse::TaxOTAModelMap::storePartitionInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::savePartitionInformation;
      }
      else if (tagName == "SETADDRESS")
      {
        mapping->func = &tse::TaxOTAModelMap::storeSetAddressInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveSetAddressInformation;
      }

      else if (tagName == "SERVICE")
      {
        mapping->func = &tse::TaxOTAModelMap::storeServiceInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveServiceInformation;
      }
      else if (tagName == "AAACITY")
      {
        mapping->func = &tse::TaxOTAModelMap::storeAAACityInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveAAACityInformation;
      }
      else if (tagName == "AGENTSINE")
      {
        mapping->func = &tse::TaxOTAModelMap::storeAgentSineInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveAgentSineInformation;
      }
      else if (tagName == "ACTION")
      {
        mapping->func = &tse::TaxOTAModelMap::storeActionInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveActionInformation;
      }
      else if (tagName == "TRANSACTION")
      {
        mapping->func = &tse::TaxOTAModelMap::storeTransactionInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveTransactionInformation;
      }
      else if (tagName == "ITINERARYINFOS")
      {
        mapping->func = &tse::TaxOTAModelMap::storeItinerariesInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveItinerariesInformation;
      }
      else if (tagName == "ITINERARYINFO")
      {
        mapping->func = &tse::TaxOTAModelMap::storeItineraryInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveItineraryInformation;
      }

      else if (tagName == "RESERVATIONITEMS")
      {
        mapping->func = &tse::TaxOTAModelMap::storeReservationItemsInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveReservationItemsInformation;
      }
      else if (tagName == "ITEM")
      {
        mapping->func = &tse::TaxOTAModelMap::storeItemInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveItemInformation;
      }
      else if (tagName == "FLIGHTSEGMENT")
      {
        mapping->func = &tse::TaxOTAModelMap::storeFlightSegmentInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveFlightSegmentInformation;
      }
      else if (tagName == "DEPARTUREAIRPORT")
      {
        mapping->func = &tse::TaxOTAModelMap::storeDepartureAirportInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveDepartureAirportInformation;
      }
      else if (tagName == "ARRIVALAIRPORT")
      {
        mapping->func = &tse::TaxOTAModelMap::storeArrivalAirportInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveArrivalAirportInformation;
      }
      else if (tagName == "EQUIPMENT")
      {
        mapping->func = &tse::TaxOTAModelMap::storeEquipmentInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveEquipmentInformation;
      }
      else if (tagName == "MARKETINGAIRLINE")
      {
        mapping->func = &tse::TaxOTAModelMap::storeMarketingAirlineInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveMarketingAirlineInformation;
      }
      else if (tagName == "OPERATINGAIRLINE")
      {
        mapping->func = &tse::TaxOTAModelMap::storeOperatingAirlineInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveOperatingAirlineInformation;
      }
      else if (tagName == "AIRFAREINFO")
      {
        mapping->func = &tse::TaxOTAModelMap::storeAirFareInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveAirFareInformation;
      }
      else if (tagName == "PTC_FAREBREAKDOWN")
      {
        mapping->func = &tse::TaxOTAModelMap::storePTCFareBreakdownInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::savePTCFareBreakdownInformation;
      }
      else if (tagName == "PASSENGERTYPE")
      {
        mapping->func = &tse::TaxOTAModelMap::storePassengerTypeInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::savePassengerTypeInformation;
      }
      else if (tagName == "FAREBASISCODE")
      {
        mapping->func = &tse::TaxOTAModelMap::storeFareBasisCodeInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveFareBasisCodeInformation;
      }
      else if (tagName == "PASSENGERFARE")
      {
        mapping->func = &tse::TaxOTAModelMap::storePassengerFareInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::savePassengerFareInformation;
      }
      else if (tagName == "BASEFARE")
      {
        mapping->func = &tse::TaxOTAModelMap::storeBaseFareInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveBaseFareInformation;
      }
      else if (tagName == "EQUIVFARE")
      {
        mapping->func = &tse::TaxOTAModelMap::storeEquivalentFareInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveEquivalentFareInformation;
      }
      else if (tagName == "DIAGNOSTIC")
      {
        mapping->func = &tse::TaxOTAModelMap::storeDiagnostic;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveDiagnostic;
      }
      else if (tagName == "REQUESTEDDIAGNOSTIC")
      {
        mapping->func = &tse::TaxOTAModelMap::storeDiagInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveDiagInformation;
      }
      else if (tagName == "PARAMETERS")
      {
        mapping->func = &tse::TaxOTAModelMap::storeDiagParameters;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveDiagParameters;
      }
      else if (tagName == "PARAMETER")
      {
        mapping->func = &tse::TaxOTAModelMap::storeDiagParameter;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveDiagParameter;
      }
      // new - - - - - - - -- - - - - - - - -- - -
      else if (tagName == "HIDDENSTOPS")
      {
        mapping->func = &tse::TaxOTAModelMap::storeHiddenStops;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveHiddenStops;
      }
      else if (tagName == "HIDDENSTOP")
      {
        mapping->func = &tse::TaxOTAModelMap::storeHiddenStop;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveHiddenStop;
      }
      else if (tagName == "FAREBREAKINFO")
      {
        mapping->func = &tse::TaxOTAModelMap::storeFareBreakInfo;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveFareBreakInfo;
      }
      else if (tagName == "DYNAMICCONFIG")
      {
        mapping->func = &tse::TaxOTAModelMap::storeDynamicConfigOverride;
        mapping->trxFunc = nullptr;
      }
      else if (!fallback::fixed::xrayTaxOTAModelMapTagsParsing() && tagName == "XRA")
      {
        mapping->func = &tse::TaxOTAModelMap::storeXrayInformation;
        mapping->trxFunc = &tse::TaxOTAModelMap::saveXrayInformation;
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
TaxOTAModelMap::classMapEntry(std::string& tagName, const xercesc::Attributes& atts)
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
TaxOTAModelMap::saveMapEntry(std::string& tagName, std::string& text)
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

DateTime
TaxOTAModelMap::convertOTADate(const char* inDate)
{
  std::string dateTimeString(inDate);
  DateTime dateTime;
  std::string time;

  // validate the date pattern as per the OTA_TaxRQ schema
  boost::regex datePattern("[2][0-9]{3}-[0-1][0-9]-[0-3][0-9][T][0-9]{2}:[0-9]{2}:[0-9]{2}");
  if (!boost::regex_match(inDate, datePattern))
  {
    LOG4CXX_WARN(logger, "Date: " << inDate << " is not in the corrrect OTA format");
    std::string errorMessage = "INVALID DATE ";
    errorMessage.append(inDate);
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         errorMessage.c_str());
  }

  // Date
  std::string::size_type pos = dateTimeString.find('T');
  if (pos != std::string::npos)
  {
    dateTime = convertDate((dateTimeString.substr(0, pos)).c_str());
    time = dateTimeString.substr(pos + 1);
  }
  // Hours
  pos = time.find(':');
  if (pos != std::string::npos)
  {
    dateTime += tse::Hours(atoi(time.substr(0, pos).c_str()));
    time = time.substr(pos + 1);
  }
  // Minutes and Seconds
  pos = time.find(':');
  if (pos != std::string::npos)
  {
    dateTime += tse::Minutes(atoi(time.substr(0, pos).c_str()));
    dateTime += tse::Seconds(atoi(time.substr(pos + 1).c_str()));
  }
  return dateTime;
}

// return version from specified location in string
// eg: 1.2.3 to get first pas 1 in pos
// -1 return value means error
int
TaxOTAModelMap::getVersionPos(int pos)
{
  int retver = -1;
  std::string received = _taxTrx->otaXmlVersion();
  static std::string tsxml("TsabreXML");
  int len = tsxml.length();
  int firstelem = received.find(tsxml);
  // version string check
  if (firstelem == -1)
  {
    // version over 1.x.x
    firstelem = 0;
    len = 0; // do not erase any string part
  }
  // remove unused string parts
  firstelem += len;
  received.erase(0, firstelem);
  //
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(".");
  tokenizer tokens(received, sep);
  int loopcnt = 0;

  for (tokenizer::iterator tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
  {
    retver = boost::lexical_cast<int>(*tok_iter);
    if (loopcnt >= pos)
      break;
    loopcnt++;
  }
  return retver;
}

void
TaxOTAModelMap::storeTaxInformation(const std::string& tagName, const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);

  _dataHandle.get(_taxTrx);
  _trx = _taxTrx;
  _taxTrx->requestType() = OTA_REQUEST;
  LOG4CXX_DEBUG(logger, "Got _taxTrx of type " << _taxTrx->requestType());

  _taxTrx->otaRequestRootElementType() = tagName;
  LOG4CXX_DEBUG(logger, "Got OtaRootElement of type " << _taxTrx->otaRequestRootElementType());

  TaxRequest* request;
  _taxTrx->dataHandle().get(request);
  _taxTrx->setRequest(request);
  LOG4CXX_DEBUG(logger, "Got _request");

  PricingOptions* options;
  _taxTrx->dataHandle().get(options);
  _taxTrx->setOptions(options);
  LOG4CXX_DEBUG(logger, "Got options");

  Billing* billing;
  _taxTrx->dataHandle().get(billing);
  _taxTrx->billing() = billing;

  // This was added to over write serviceName, parentServiceName to the proper value
  // since the web service client may not supply the write value for these fields.
  _taxTrx->billing()->parentServiceName() = _taxTrx->billing()->getServiceName(Billing::SVC_TAX);

  _taxTrx->billing()->updateTransactionIds(_taxTrx->transactionId());
  _taxTrx->billing()->updateServiceNames(Billing::SVC_TAX);

  int numAtts = attrs.getLength();

  for (int i = 0; i < numAtts; ++i)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();
    std::string xmlValueString = xmlValue.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // XML version
      _taxTrx->otaXmlVersion() = xmlValue.c_str();
      break;

    case 2: // Shopping path
      if (xmlValueString == "T")
      {
        _taxTrx->setShoppingPath(true);
      }
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::saveTaxInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::storeProcessingOptionsInformation(const std::string& tagName,
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
    case 1: // SettlementMethod
      _taxTrx->getRequest()->setSettlementMethod(xmlValue.c_str());
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storePointOfSaleInformation(const std::string& tagName,
                                            const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxOTAModelMap::storeSourceInformation(const std::string& tagName, const xercesc::Attributes& attrs)
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
      _taxTrx->billing()->userPseudoCityCode() = strUCase(xmlValue).c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storeTPAExtensionsInformation(const std::string& tagName,
                                              const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxOTAModelMap::storeUserInformation(const std::string& tagName, const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxOTAModelMap::storeStationInformation(const std::string& tagName,
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
      _taxTrx->billing()->userStation() = strUCase(xmlValue).c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storeBranchInformation(const std::string& tagName, const xercesc::Attributes& attrs)
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
      _taxTrx->billing()->userBranch() = strUCase(xmlValue).c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storePartitionInformation(const std::string& tagName,
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
          MCPCarrierUtil::isNeutralCarrier(strUCase(xmlValue).c_str()))
      {
        realCxr = MCPCarrierUtil::swapFromNeutralToActual(strUCase(xmlValue).c_str());
        _taxTrx->billing()->partitionID() = realCxr;
      }
      else
      {
        _taxTrx->billing()->partitionID() = strUCase(xmlValue).c_str();
        realCxr = MCPCarrierUtil::swapToActual(_taxTrx, _taxTrx->billing()->partitionID().c_str());
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
TaxOTAModelMap::storeSetAddressInformation(const std::string& tagName,
                                           const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxOTAModelMap::storeServiceInformation(const std::string& tagName,
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
      _taxTrx->billing()->parentServiceName() = strUCase(xmlValue).c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}
void
TaxOTAModelMap::storeAAACityInformation(const std::string& tagName,
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
      _taxTrx->billing()->aaaCity() = strUCase(xmlValue).c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storeAgentSineInformation(const std::string& tagName,
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
      _taxTrx->billing()->aaaSine() = strUCase(xmlValue).c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storeActionInformation(const std::string& tagName, const xercesc::Attributes& attrs)
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
      _taxTrx->billing()->actionCode() = strUCase(xmlValue).c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storeTransactionInformation(const std::string& tagName,
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
      _taxTrx->billing()->clientTransactionID() = Billing::string2transactionId(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storeItinerariesInformation(const std::string& tagName,
                                            const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);

  _taxTrx->getRequest()->ticketingDT() = DateTime::localTime();

  int numAttrs = attrs.getLength();

  for (int i = 0; i < numAttrs; ++i)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Ticketing date time
      _taxTrx->getRequest()->ticketingDT() = convertOTADate(xmlValue.c_str());
      break;
    }
  }
}

void
TaxOTAModelMap::storeItineraryInformation(const std::string& tagName,
                                          const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  _taxTrx->dataHandle().get(_itin);
}

void
TaxOTAModelMap::storeReservationItemsInformation(const std::string& tagName,
                                                 const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxOTAModelMap::storeItemInformation(const std::string& tagName, const xercesc::Attributes& attrs)
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
    case 1: // RPH
      _itin->sequenceNumber() = atoi(xmlValue.c_str());
      break;
    case 2: // Ticketing Carrier
      _itin->ticketingCarrier() = strUCase(xmlValue).c_str();
      break;
    case 3: // Validating Carrier
      _taxTrx->getRequest()->validatingCarrier() = strUCase(xmlValue).c_str();
      // Allow different validating carrier overrides for different itineraries in one request..
      _itin->validatingCarrier() = strUCase(xmlValue).c_str();
      break;
    case 4: // Sale PseudoCity Code
    {
      if (_itin->agentPCCOverride() != nullptr)
        return;
      LocCode code = strUCase(xmlValue).c_str();
      _itin->agentPCCOverride() = getAgent(code);
      _taxTrx->getRequest()->ticketingAgent() = _itin->agentPCCOverride();
      //---------------------------------------------------------------------
      break;
    }
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storeFlightSegmentInformation(const std::string& tagName,
                                              const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  _taxTrx->dataHandle().get(_airSeg);

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Departure Date Time
      _airSeg->departureDT() = convertOTADate(xmlValue.c_str());
      break;
    case 2: // Arrival Date Time
      _airSeg->arrivalDT() = convertOTADate(xmlValue.c_str());
      break;
    case 3: // Flight Number
      _airSeg->marketingFlightNumber() = atoi(xmlValue.c_str());
      break;
    case 4: // Res Book Desig Code
      ClassOfService* classOfService;
      _taxTrx->dataHandle().get(classOfService);
      classOfService->bookingCode() = strUCase(xmlValue).c_str();
      _airSeg->setBookingCode(strUCase(xmlValue).c_str());
      _airSeg->classOfService().push_back(classOfService);
      break;
    case 5: // forced connection ind
      if (readXmlBool(xmlValue)) // T = true, legacy code depends on that
        _airSeg->forcedConx() = 'T';
      break;
    case 6:
      if (readXmlBool(xmlValue)) // T = true, legacy code depends on that
        _airSeg->forcedStopOver() = 'T';
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storeDepartureAirportInformation(const std::string& tagName,
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
    case 1: // Location Code
      _airSeg->origAirport() = strUCase(xmlValue).c_str();
      break;
    case 2: // Code Context (example = "IATA")
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storeArrivalAirportInformation(const std::string& tagName,
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
    case 1: // Location Code
      _airSeg->destAirport() = strUCase(xmlValue).c_str();
      break;
    case 2: // Code Context (example = "IATA")
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storeEquipmentInformation(const std::string& tagName,
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
    case 1: // Equipment Type
      _airSeg->equipmentType() = strUCase(xmlValue).c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storeOperatingAirlineInformation(const std::string& tagName,
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
    case 1: // Operating Airline Code
      _airSeg->setOperatingCarrierCode(strUCase(xmlValue).c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storeMarketingAirlineInformation(const std::string& tagName,
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
    case 1: // Marketing Airline Code
      _airSeg->setMarketingCarrierCode(strUCase(xmlValue).c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storeAirFareInformation(const std::string& tagName,
                                        const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxOTAModelMap::storePTCFareBreakdownInformation(const std::string& tagName,
                                                 const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
  _taxTrx->dataHandle().get(_paxType); /****CHECK if this needs to be trx or taxtrx*********/
}

void
TaxOTAModelMap::storePassengerTypeInformation(const std::string& tagName,
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
    case 1: // Quantity
      _paxType->number() = atoi(xmlValue.c_str());
      break;
    case 2: // Code
      _paxType->paxType() = strUCase(xmlValue).c_str();
      break;
    case 3: // Age
      _paxType->age() = atoi(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storeFareBasisCodeInformation(const std::string& tagName,
                                              const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxOTAModelMap::storePassengerFareInformation(const std::string& tagName,
                                              const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);

  if (!_itin)
    return;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1:
      _itin->anciliaryServiceCode() = (xmlValue).c_str();
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
    }
  }

  if (!_farePath)
    _taxTrx->dataHandle().get(_farePath);
}

void
TaxOTAModelMap::storeBaseFareInformation(const std::string& tagName,
                                         const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);

  if (!_farePath)
  {
    _taxTrx->dataHandle().get(_farePath);
  }

  FareUsageMap::iterator fareUsageMapIter = _fareUsageMap.find(_fareComponentNumber);
  if (fareUsageMapIter == _fareUsageMap.end())
    return;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Base Fare Amount
      // when we are in PTC_FareBreakDown _airSeg is NULL
      if (_airSeg || !_fareCompDefinedAtLeastOnce)
      {
        Fare* fare;
        _taxTrx->dataHandle().get(fare);
        std::get<1>(fareUsageMapIter->second) = fare;
        (std::get<1>(fareUsageMapIter->second))->nucFareAmount() = atof(xmlValue.c_str());
      }

      if (!_airSeg)
      {
        _totalNUCAmount = atof(xmlValue.c_str());
      }
      // _farePath->baseFareCurrency() = "NUC";
      break;
    case 2: // Currency Code
      //_taxTrx->getOptions()->currencyOverride() = strUCase(xmlValue).c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::saveBaseFareInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::storeFareBreakInfo(const std::string& tagName, const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);

  int numAtts = attrs.getLength();
  bool fareComponentDefined = false;

  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // Fare Component number
      _fareComponentNumber = atoi(xmlValue.c_str());
      fareComponentDefined = true;
      _fareCompDefinedAtLeastOnce = true;
      // std::cout << " Fare component number " << strUCase(xmlValue).c_str();
      break;
    case 2: // SideTripNumber
      // std::cout << " SideTripNumber " << atoi(xmlValue.c_str());
      break;
    case 3: // SideTripStartInd
      // std::cout << " SideTripStartInd " << atoi(xmlValue.c_str());
      break;
    case 4: // SideTripEndInd
      // std::cout << " SideTripEndInd " << readXmlBool(xmlValue);
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }

  if (!fareComponentDefined)
    _fareCompNotDefinedAtLeastOnce = true;

  if (_fareCompNotDefinedAtLeastOnce && _fareCompDefinedAtLeastOnce)
  {
    LOG4CXX_WARN(logger, "Missing fare component definition");
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                 "MISSING FARE COMPONENT DEFINITION");
  }

  FareUsageMap::iterator fareUsageMapIter = _fareUsageMap.find(_fareComponentNumber);
  if (fareUsageMapIter == _fareUsageMap.end())
  {
    std::vector<TravelSeg*> travelSegVector;
    Fare* fare = nullptr;
    _fareUsageMap[_fareComponentNumber] = std::make_tuple(travelSegVector, fare);
  }
}

void
TaxOTAModelMap::saveFareBreakInfo(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::storeEquivalentFareInformation(const std::string& tagName,
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
    case 1: // Amount
      // new functionality for SPR fix
      //_taxTrx->getRequest()->equivAmountOverride() = (double)atof(xmlValue.c_str());
      if (_airSeg)
      {
        _equivAmount += (double)atof(xmlValue.c_str());
      }
      else
      {
        _equivAmount = (double)atof(xmlValue.c_str());
      }
      break;
    case 2: // Currency Code
      //  _taxTrx->getOptions()->currencyOverride() = strUCase(xmlValue).c_str();
      //  _farePath->calculationCurrency() = strUCase(xmlValue).c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storeDiagInformation(const std::string& tagName, const xercesc::Attributes& attrs)
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
    case 1:
    {
      const int diag = atoi(xmlValue.c_str());
      _taxTrx->getRequest()->diagnosticNumber() = diag;
      _taxTrx->diagnostic().diagnosticType() = static_cast<DiagnosticTypes>(diag);
      _taxTrx->diagnostic().activate();
    }
    break;
    case 2:
      // diag parameter

      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::storeDiagParameter(const std::string& tagName, const xercesc::Attributes& attrs)
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
    case 1:
      {
        const std::string strValue = xmlValue.c_str();
        if (strValue.length()>=2)
          _taxTrx->diagnostic().diagParamMap()[strValue.substr(0, 2)] =
             strValue.substr(2, strValue.length() - 2);
      }
      break;
    }
  }
}

void
TaxOTAModelMap::storeDynamicConfigOverride(const std::string& /*tagName*/,
                                           const xercesc::Attributes& attrs)
{
  handleDynamicConfig(((Mapping*)_currentMapEntry)->members, attrs);
}

void
TaxOTAModelMap::saveDiagParameter(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::storeDiagParameters(const std::string& tagName, const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxOTAModelMap::saveDiagParameters(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveProcessingOptionsInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::savePointOfSaleInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveSourceInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveTPAExtensionsInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveUserInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveStationInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveBranchInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::savePartitionInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveSetAddressInformation(const std::string& tagName, const std::string& text)
{
  _taxTrx->billing()->userSetAddress() = text;
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveServiceInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveAAACityInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveAgentSineInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveActionInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveTransactionInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveItinerariesInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveItineraryInformation(const std::string& tagName, const std::string& text)
{
  if (_itin->travelSeg().empty())
    throw ErrorResponseException(ErrorResponseException::EMPTY_TRAVEL_SEG);

  _taxTrx->itin().push_back(_itin);
  _itin = nullptr;
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

//----------------------------------------------------------------------------
// TaxOTAModelMap::getAgent
// with caching
//----------------------------------------------------------------------------
Agent*
TaxOTAModelMap::getAgent(LocCode& loc)
{
  AgentCache::iterator agentCacheIter = _agentCache.find(loc);
  if (agentCacheIter == _agentCache.end())
  { // no agent in cache found
    Agent* agent;
    _taxTrx->dataHandle().get(agent);
    agent->agentCity() = loc;
    agentSetUp(agent);
    _agentCache.insert(std::map<LocCode, Agent*>::value_type(loc, agent));
    return agent;
  }
  return agentCacheIter->second; // from cache
}

void
TaxOTAModelMap::agentSetUp(Agent* agent)
{
  const Loc* agentLocation = _taxTrx->dataHandle().getLoc(agent->agentCity(), time(nullptr));

  std::vector<Customer*> cusList = _taxTrx->dataHandle().getCustomer(agent->agentCity());

  if (cusList.size() > 0)
  {
    agent->agentTJR() = cusList.front();
    agent->currencyCodeAgent() = agent->agentTJR()->defaultCur();
    agent->agentLocation() = _taxTrx->dataHandle().getLoc(agent->agentTJR()->aaCity(), time(nullptr));
    if (!agent->agentLocation())
    {
      LOG4CXX_WARN(logger, "Agent: " << agent->agentCity() << " not found!");
      throw NonFatalErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
    }
  }
  else
  {
    if (agent->agentCity().size() == 4)
    {
      std::stringstream msg;
      msg << "Customer record: '" << agent->agentCity() << "' AGENT_PCC_NON_EXISTENT";
      LOG4CXX_ERROR(logger, msg.str());
    }

    LocCode agentCity;
    agentCity = _taxTrx->dataHandle().getMultiTransportCity(agent->agentCity());
    agent->agentLocation() = _taxTrx->dataHandle().getLoc(agentCity, time(nullptr));

    if (agent->agentLocation() && agentLocation)
    {
      if (agent->agentLocation()->nation() != agentLocation->nation())
        agent->agentLocation() = agentLocation;
    }

    if (!agent->agentLocation())
      agent->agentLocation() = agentLocation;
    // workaround for agents without currency set
    if (agent->agentLocation())
    {
      if (agent->currencyCodeAgent().empty())
      {
        DateTime today = DateTime::localTime();
        NationCode nationCode = agent->agentLocation()->nation();
        const Nation* nation = _taxTrx->dataHandle().getNation(nationCode, today);
        if ((nation == nullptr) || (nation->primeCur().empty()))
        {
          throw ErrorResponseException(ErrorResponseException::AGENT_CURRENCY_CODE_MISSING);
        }
        else
        {
          agent->currencyCodeAgent() = nation->primeCur();
        }
      }
    }

    // Checking for the pricing currency
    if (agent != nullptr && agent->agentLocation() != nullptr)
    {
      DateTime today = DateTime::localTime();
      NationCode nationCode = agent->agentLocation()->nation();
      const Nation* nation = _taxTrx->dataHandle().getNation(nationCode, today);
      if ((nation != nullptr) && !nation->alternateCur().empty())
      {
        agent->currencyCodeAgent() = nation->alternateCur();
      }
    }

    if (!agent->agentLocation())
    {
      LOG4CXX_ERROR(logger, "Customer record: " << agent->agentCity() << " not found!");
      throw NonFatalErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
    }
  }
}

void
TaxOTAModelMap::saveReservationItemsInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveItemInformation(const std::string& tagName, const std::string& text)
{
  saveFareUsage();
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveFlightSegmentInformation(const std::string& tagName, const std::string& text)
{
  if (_airSeg->origAirport().empty() || _airSeg->destAirport().empty())
  {
    LOG4CXX_WARN(logger, "Missing XML Location Information");
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "MISSING LOCATION INFORMATION IN REQUEST");
  }

  _airSeg->origin() = _taxTrx->dataHandle().getLoc(_airSeg->origAirport(), _airSeg->departureDT());
  _airSeg->destination() =
      _taxTrx->dataHandle().getLoc(_airSeg->destAirport(), _airSeg->arrivalDT());

  if (!_airSeg->origin() || !_airSeg->destination())
  {
    LOG4CXX_WARN(logger, "Wrong XML Location Information after decoding");
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "INVALID LOCATION CODE");
  }

  //                check major version
  if (getVersionPos(MAJOR_VERSION_POS) > 1)
  {
    // Assign offMultiCity and boardMultiCity
    setBoardCity(*_airSeg);
    setOffCity(*_airSeg);

    _intlItin = false;
    if (LocUtil::isInternational(*_airSeg->origin(), *_airSeg->destination()))
    {
      _intlItin = true;
    }
    // check is this firs flight segment
    //   if( _itin->travelSeg() )
    bool seg_added = false;
    processMissingArunkSegForOTA(_taxTrx, _itin, _airSeg, seg_added);
  }

  // this is needed to handle old type of requests
  FareUsageMap::iterator fareUsageMapIter = _fareUsageMap.find(_fareComponentNumber);
  if (fareUsageMapIter == _fareUsageMap.end())
  {
    std::vector<TravelSeg*> travelSegVector;
    travelSegVector.push_back(_airSeg);
    Fare* fare = nullptr;
    _fareUsageMap[_fareComponentNumber] = std::make_tuple(travelSegVector, fare);
  }
  else
  {
    (std::get<0>(fareUsageMapIter->second)).push_back(_airSeg);
    if (_airSeg->fareBasisCode().empty() &&
        !((std::get<0>(fareUsageMapIter->second)).back()->fareBasisCode().empty()))
    {
      LOG4CXX_WARN(logger, "Fare basis defined not in last segment for fare component");
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                           "FARE BASIS DEFINED NOT IN LAST SEGMENT");
    }
  }

  LOG4CXX_DEBUG(logger,
                "Flight From: " << _airSeg->origAirport() << " TO " << _airSeg->destAirport());
  _itin->travelSeg().push_back(_airSeg);
  _itin->setTravelDate(_airSeg->departureDT());

  _airSeg = nullptr;
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveDepartureAirportInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveArrivalAirportInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveEquipmentInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveOperatingAirlineInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveMarketingAirlineInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveAirFareInformation(const std::string& tagName, const std::string& text)
{
  // LOG4CXX_DEBUG(logger, "End " <<tagName);
}

void
TaxOTAModelMap::savePTCFareBreakdownInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::savePassengerTypeInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::savePassengerFareInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveFareBasisCodeInformation(const std::string& tagName, const std::string& text)
{
  if (getVersionPos(MAJOR_VERSION_POS) < 2)
    return;

  FareUsageMap::iterator fareUsageMapIter = _fareUsageMap.find(_fareComponentNumber);
  if (fareUsageMapIter != _fareUsageMap.end())
  {
    std::vector<TravelSeg*>& travelSegVector = std::get<0>(fareUsageMapIter->second);
    std::vector<TravelSeg*>::iterator travelSegIt;
    for (travelSegIt = travelSegVector.begin(); travelSegIt != travelSegVector.end(); ++travelSegIt)
    {
      (*travelSegIt)->fareBasisCode() = text;
    }
  }
  if (_airSeg)
  {
    _airSeg->fareBasisCode() = text;
  }
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveEquivalentFareInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveDiagInformation(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::saveFareUsage()
{
  if (_paxType != nullptr)
  {
    _taxTrx->paxType().push_back(_paxType);
    _itin->paxGroup().push_back(_paxType);

    // PaxTypeInfo...
    PaxTypeInfo* paxTypeInfo;
    _taxTrx->dataHandle().get(paxTypeInfo);
    _paxType->paxTypeInfo() = paxTypeInfo;

    if (_farePath != nullptr)
    {
      _farePath->itin() = _itin;
      _farePath->paxType() = _paxType;
      _itin->farePath().push_back(_farePath);
      MoneyAmount farePathAmount = 0;

      if (!_itin->farePath().empty())
      {
        FareUsageMap::iterator fareUsageMapIter;
        for (fareUsageMapIter = _fareUsageMap.begin(); fareUsageMapIter != _fareUsageMap.end();
             ++fareUsageMapIter)
        {
          PricingUnit* pricingUnit;
          _taxTrx->dataHandle().get(pricingUnit);

          FareUsage* fareUsage;
          _taxTrx->dataHandle().get(fareUsage);

          Fare* fare = std::get<1>(fareUsageMapIter->second);
          if (!fare)
          {
            LOG4CXX_WARN(logger, "Missing base fare for fare component");
            throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                                 "MISSING BASE FARE");
          }
          farePathAmount += fare->nucFareAmount();

          PaxTypeFare* paxTypeFare;
          _taxTrx->dataHandle().get(paxTypeFare);

          FareMarket* fareMarket;
          _taxTrx->dataHandle().get(fareMarket);

          FareInfo* fareInfo;
          _taxTrx->dataHandle().get(fareInfo);

          TariffCrossRefInfo* tariffCrossRefInfo;
          _taxTrx->dataHandle().get(tariffCrossRefInfo);

          if (pricingUnit == nullptr || fareUsage == nullptr || paxTypeFare == nullptr || fare == nullptr ||
              fareInfo == nullptr || tariffCrossRefInfo == nullptr || fareMarket == nullptr)
          {
            _paxType = nullptr;
            _farePath = nullptr;
            continue;
          }

          paxTypeFare->paxType() = _paxType;
          fareUsage->paxTypeFare() = paxTypeFare;

          pricingUnit->fareUsage().push_back(fareUsage);
          _farePath->pricingUnit().push_back(pricingUnit);

          fareUsage->travelSeg() = std::get<0>(fareUsageMapIter->second);
          fareMarket->travelSeg() = std::get<0>(fareUsageMapIter->second);

          fare->initialize(Fare::FS_International, fareInfo, *fareMarket, tariffCrossRefInfo);
          fareUsage->paxTypeFare()->setFare(fare);

          for (const tse::TravelSeg* travelSeg : fareUsage->travelSeg())
          {
            PaxTypeFare::SegmentStatus segStatus;
            segStatus._bkgCodeReBook = travelSeg->getBookingCode();
            fareUsage->paxTypeFare()->segmentStatus().push_back(segStatus);
          }
        }

        if ((farePathAmount != _totalNUCAmount) && _itin->anciliaryServiceCode().empty() &&
            !_taxTrx->isShoppingPath())
        {
          LOG4CXX_WARN(logger, "Total amount doesn't match");
          throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                       "TOTAL AMOUNT DOESN'T MATCH");
        }

        _farePath->setTotalNUCAmount(_totalNUCAmount);
        _taxTrx->getRequest()->equivAmountOverride() = _equivAmount;
      }
    }
    _paxType = nullptr;
    _farePath = nullptr;
    _fareUsageMap.clear();
  }
}

void
TaxOTAModelMap::storeDiagnostic(const std::string& tagName, const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxOTAModelMap::saveDiagnostic(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::storeHiddenStop(const std::string& tagName, const xercesc::Attributes& attrs)
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
    case 1: // Location Code
    {
      const Loc* location = _taxTrx->dataHandle().getLoc(xmlValue.c_str(), DateTime::localTime());

      if (location)
        _airSeg->hiddenStops().push_back((Loc*)location);
    }
    break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::saveHiddenStop(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::storeHiddenStops(const std::string& tagName, const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);
}

void
TaxOTAModelMap::saveHiddenStops(const std::string& tagName, const std::string& text)
{
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

void
TaxOTAModelMap::storeXrayInformation(const std::string& tagName, const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "Start " << tagName);

  const int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string xmlString = xmlStr.c_str();

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(toUpper(xmlString))])
    {
    case 1: // MID
    {
      _xrayMessageId.assign(xmlValue.c_str());
      LOG4CXX_DEBUG(logger, "MID = " << xmlValue.c_str());
      break;
    }
    case 2: // CID
    {
      _xrayConversationId.assign(xmlValue.c_str());
      LOG4CXX_DEBUG(logger, "CID = " << xmlValue.c_str());
      break;
    }
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlString << ", not mapped");
      break;
    }
  }
}

void
TaxOTAModelMap::saveXrayInformation(const std::string& tagName, const std::string& text)
{
  if (!fallback::fixed::xrayTaxOTAModelMapTagsParsing())
  {
    if (xray::xrayEnabledCfg.isValid(_taxTrx))
    {
      _taxTrx->assignXrayJsonMessage(
            std::make_unique<xray::JsonMessage>(_xrayMessageId, _xrayConversationId));
    }
    else
    {
      LOG4CXX_WARN(logger, "Xray tracking is disabled");
    }
  }
  LOG4CXX_DEBUG(logger, "End " << tagName);
}

std::string
TaxOTAModelMap::strUCase(tse::XMLChString& str)
{
  const char* src = str.c_str();
  std::string stringSrc(src);
  std::transform(stringSrc.begin(), stringSrc.end(), stringSrc.begin(), (int (*)(int))std::toupper);
  return stringSrc;
}

// no schema validation on ATSEV-2 side, no possibility of reading boolean values directly
bool
TaxOTAModelMap::readXmlBool(tse::XMLChString& str)
{
  const char* src = str.c_str();
  std::string stringSrc(src);
  if (stringSrc.compare("true") == 0 || stringSrc.compare("1") == 0)
    return true;
  else
    return false;
}

void
TaxOTAModelMap::addArunkSegmentForOTA(TaxTrx* trx,
                                      Itin& itin,
                                      const TravelSeg& travelSeg,
                                      bool& isMissingArunkForOTA)
{
  std::vector<TravelSeg*>& ts = itin.travelSeg();
  TravelSeg* prevTvlSeg = ts.back();

  ArunkSeg* arunkSeg = nullptr;
  trx->dataHandle().get(arunkSeg);
  if (arunkSeg == nullptr)
    return;

  arunkSeg->segmentType() = Arunk;
  arunkSeg->forcedStopOver() = 'T';

  arunkSeg->forcedStopOver() = 'F';
  arunkSeg->departureDT() =  prevTvlSeg->arrivalDT();
  arunkSeg->arrivalDT() =  travelSeg.departureDT();

  arunkSeg->origAirport() = prevTvlSeg->destAirport();
  arunkSeg->origin() = prevTvlSeg->destination();
  arunkSeg->boardMultiCity() = prevTvlSeg->offMultiCity();

  arunkSeg->destAirport() = travelSeg.origAirport();
  arunkSeg->destination() = travelSeg.origin();
  arunkSeg->offMultiCity() = travelSeg.boardMultiCity();
  arunkSeg->segmentOrder() = prevTvlSeg->segmentOrder() + 1;

  ts.push_back(arunkSeg);
  isMissingArunkForOTA = true;
  trx->travelSeg().push_back(arunkSeg);
}

void
TaxOTAModelMap::processMissingArunkSegForOTA(TaxTrx* trx,
                                             Itin* itin,
                                             const TravelSeg* travelSeg,
                                             bool& isMissingArunkForOTA)
{
  if (needToAddArunkSegment(trx, *itin, *travelSeg))
  {
    addArunkSegmentForOTA(trx, *itin, *travelSeg, isMissingArunkForOTA);
  }
}

bool
TaxOTAModelMap::needToAddArunkSegment(TaxTrx* trx, Itin& itin, const TravelSeg& travelSeg)
{
  std::vector<TravelSeg*>& ts = itin.travelSeg();
  if (ts.size() == 0)
    return false;
  if (travelSeg.segmentType() == Arunk)
    return false;
  TravelSeg* prevTvlSeg = ts.back();
  if (prevTvlSeg->segmentType() == Arunk)
    return false;
  if (prevTvlSeg->offMultiCity() != travelSeg.boardMultiCity())
    return true;
  return false;
}

void
TaxOTAModelMap::setBoardCity(AirSeg& airSeg)
{

  const std::vector<tse::MultiTransport*>& boardMACList =
      _trx->dataHandle().getMultiTransportCity(airSeg.origAirport(),
                                               airSeg.carrier(),
                                               (_intlItin ? GeoTravelType::International : airSeg.geoTravelType()),
                                               airSeg.departureDT());

  std::vector<tse::MultiTransport*>::const_iterator listIter = boardMACList.begin();

  if (listIter == boardMACList.end())
  {
    airSeg.boardMultiCity() = airSeg.origAirport();
    LOG4CXX_WARN(logger, "Board multi city vector is empty");
  }
  else
  {
    // If the flight is Domestic use the and the domAppl is set
    // use this multitranscity.  If the flight is International
    // and the intlAppl is set use this multitranscity
    airSeg.boardMultiCity() = (**listIter).multitranscity();
  }
  LOG4CXX_DEBUG(logger, "boardMultiCity=" << airSeg.boardMultiCity().c_str());
}

void
TaxOTAModelMap::setOffCity(AirSeg& airSeg)
{
  const std::vector<tse::MultiTransport*>& offMACList =
      _trx->dataHandle().getMultiTransportCity(airSeg.destAirport(),
                                               airSeg.carrier(),
                                               (_intlItin ? GeoTravelType::International : airSeg.geoTravelType()),
                                               airSeg.departureDT());
  std::vector<tse::MultiTransport*>::const_iterator listIter = offMACList.begin();
  if (listIter == offMACList.end())
  {
    airSeg.offMultiCity() = airSeg.destAirport();
  }
  else
  {
    // Take the first, they should all be the same
    airSeg.offMultiCity() = (**listIter).multitranscity();
  }
  LOG4CXX_DEBUG(logger, "offMultiCity=" << airSeg.offMultiCity());
}
} // tse namespace
