//----------------------------------------------------------------------------
//
//  File:  TaxModelMap.cpp
//  Description: See TaxModelMap.h file
//  Created:  December 21, 2004
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

#include "Xform/TaxModelMap.h"

#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TseUtil.h"
#include "Common/XMLChString.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"

#include <set>
#include <string>
#include <vector>

#include <time.h>

namespace tse
{
FALLBACK_DECL(neutralToActualCarrierMapping);
}

namespace tse
{
static Logger
logger("atseintl.Xform.TaxModelMap");

TaxModelMap::~TaxModelMap()
{
  for (auto& elem : _classMap)
  {
    delete (Mapping*)elem.second;
  }
}

bool
TaxModelMap::createMap()
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
      if (tagName == "TAXREQUEST")
      {
        mapping->func = &tse::TaxModelMap::storeTaxInformation;
        mapping->trxFunc = &tse::TaxModelMap::saveTaxInformation;
      }
      else if (tagName == "ITN")
      {
        mapping->func = &tse::TaxModelMap::storeItineraryInformation;
        mapping->trxFunc = &tse::TaxModelMap::saveItineraryInformation;
      }
      else if (tagName == "PXI")
      {
        mapping->func = &tse::TaxModelMap::storePassengerInformation;
        mapping->trxFunc = &tse::TaxModelMap::savePassengerInformation;
      }
      else if (tagName == "FLI")
      {
        mapping->func = &tse::TaxModelMap::storeFlightInformation;
        mapping->trxFunc = &tse::TaxModelMap::saveFlightInformation;
      }
      else if (tagName == "BCC")
      {
        mapping->func = &tse::TaxModelMap::storeBookingClass;
        mapping->trxFunc = &tse::TaxModelMap::saveBookingClass;
      }
      else if (tagName == "BIL")
      {
        mapping->func = &tse::TaxModelMap::storeBillingInformation;
        mapping->trxFunc = &tse::TaxModelMap::saveBillingInformation;
      }
      else if (tagName == "DYNAMICCONFIG")
      {
        mapping->func = &tse::TaxModelMap::storeDynamicConfigOverride;
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
TaxModelMap::classMapEntry(std::string& tagName, const xercesc::Attributes& atts)
{
  unsigned int hashedTagName = SDBMHash(toUpper(tagName));

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
TaxModelMap::saveMapEntry(std::string& tagName, std::string& text)
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
TaxModelMap::storeTaxInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeTaxInformation");

  _dataHandle.get(_taxTrx);
  _trx = _taxTrx;
  LOG4CXX_DEBUG(logger, "Got _taxTrx");

  TaxRequest* request;
  _taxTrx->dataHandle().get(request); // lint !e530
  request->ticketingDT() = DateTime::localTime();
  _taxTrx->setRequest(request);
  LOG4CXX_DEBUG(logger, "Got _request");

  Agent* agent;
  _taxTrx->dataHandle().get(agent); // lint !e530
  request->ticketingAgent() = agent;
  LOG4CXX_DEBUG(logger, "Got agent");

  PricingOptions* options;
  _taxTrx->dataHandle().get(options); // lint !e530
  _taxTrx->setOptions(options);
  LOG4CXX_DEBUG(logger, "Got options");

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // Q0A - Diagnostic number
      _taxTrx->getRequest()->diagnosticNumber() = atoi(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
  LOG4CXX_DEBUG(logger, "Done");
}

void
TaxModelMap::saveTaxInformation()
{
  LOG4CXX_DEBUG(logger, "In saveTaxInformation");
}

void
TaxModelMap::storeItineraryInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeItineraryInformation");
  _taxTrx->dataHandle().get(_itin);

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    LOG4CXX_DEBUG(logger, "xmlStr: " << xmlStr.c_str());
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // A20 - Sale PCC
      _taxTrx->getRequest()->ticketingAgent()->agentCity() = xmlValue.c_str();
      break;
    case 2: // S12 - Ticketing carrier
      _itin->ticketingCarrier() = xmlValue.c_str();
      break;
    case 3: // Q1K - Sequence number
      _itin->sequenceNumber() = atoi(xmlValue.c_str());
      break;
    case 4: // B05 - Validating Carrier
      _taxTrx->getRequest()->validatingCarrier() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
TaxModelMap::saveItineraryInformation()
{
  LOG4CXX_DEBUG(logger, "In saveItineraryInformation");

  std::stringstream msg;

  Agent* agent = _taxTrx->getRequest()->ticketingAgent();
  std::vector<Customer*> custList = _taxTrx->dataHandle().getCustomer(agent->agentCity());

  if (custList.size() > 0)
  {
    agent->agentTJR() = custList.front();
    agent->currencyCodeAgent() = agent->agentTJR()->defaultCur();
    agent->agentLocation() = _taxTrx->dataHandle().getLoc(agent->agentTJR()->aaCity(), time(nullptr));
    if (!agent->agentLocation())
    {
      throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
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
    throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
  }

  _taxTrx->itin().push_back(_itin);
  _itin = nullptr;
}

void
TaxModelMap::storePassengerInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storePassengerInformation");
  _trx->dataHandle().get(_paxType);

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // B70 - Passenger type
      _paxType->paxType() = xmlValue.c_str();
      break;
    case 2: // Q0U - Passenger type count
      _paxType->number() = atoi(xmlValue.c_str());
      break;
    case 3: // Q0T - Passenger child age
      _paxType->age() = atoi(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  // FarePath...
  _dataHandle.get(_farePath);
}

void
TaxModelMap::savePassengerInformation()
{
  LOG4CXX_DEBUG(logger, "In savePassengerInformation");
  if (_paxType != nullptr)
  {
    // @todo need to update PaxTypeUtil
    // PaxTypeUtil::initialize(*_taxTrx,
    //	*_paxType,
    //	_paxType->paxType(),
    //	_paxType->number(),
    //	_paxType->age(),
    //	_paxInputOrder++);
    _taxTrx->paxType().push_back(_paxType);
    _itin->paxGroup().push_back(_paxType);

    // PaxTypeInfo...
    PaxTypeInfo* paxTypeInfo;
    _taxTrx->dataHandle().get(paxTypeInfo); // lint !e530
    _paxType->paxTypeInfo() = paxTypeInfo;

    if (_farePath != nullptr)
    {
      _farePath->itin() = _itin;
      _farePath->paxType() = _paxType;
      _itin->farePath().push_back(_farePath);

      if (!_itin->farePath().empty())
      {
        PricingUnit* pricingUnit;
        _taxTrx->dataHandle().get(pricingUnit);

        FareUsage* fareUsage;
        _taxTrx->dataHandle().get(fareUsage);

        PaxTypeFare* paxTypeFare;
        _taxTrx->dataHandle().get(paxTypeFare);

        Fare* fare;
        _taxTrx->dataHandle().get(fare);

        FareMarket* fareMarket;
        _taxTrx->dataHandle().get(fareMarket);

        FareInfo* fareInfo;
        _taxTrx->dataHandle().get(fareInfo);

        TariffCrossRefInfo* tariffCrossRefInfo;
        _taxTrx->dataHandle().get(tariffCrossRefInfo);

        if (pricingUnit == nullptr || fareUsage == nullptr || paxTypeFare == nullptr || fare == nullptr || fareInfo == nullptr ||
            tariffCrossRefInfo == nullptr || fareMarket == nullptr)
        {
          _paxType = nullptr;
          _farePath = nullptr;
          return;
        }

        paxTypeFare->paxType() = _paxType;
        paxTypeFare->nucFareAmount() = _farePath->getTotalNUCAmount();
        fareUsage->paxTypeFare() = paxTypeFare;

        pricingUnit->fareUsage().push_back(fareUsage);
        _farePath->pricingUnit().push_back(pricingUnit);

        std::vector<TravelSeg*>::iterator tsI = _itin->travelSeg().begin();
        std::vector<TravelSeg*>::iterator tsEndI = _itin->travelSeg().end();

        for (; tsI != tsEndI; tsI++)
        {
          fareUsage->travelSeg().push_back(*tsI);
          fareMarket->travelSeg().push_back(*tsI);
        }

        fare->initialize(Fare::FS_International, fareInfo, *fareMarket, tariffCrossRefInfo);

        fareUsage->paxTypeFare()->setFare(fare);
      }
    }
    _paxType = nullptr;
    _farePath = nullptr;
  }
}

void
TaxModelMap::storeFlightInformation(const xercesc::Attributes& attrs)
{
  if (!_itin->paxGroup().empty())
    return;

  int timeInMins = 0;

  LOG4CXX_DEBUG(logger, "In storeFlightInformation");
  _dataHandle.get(_airSeg);
  //	_dataHandle.get(_farePath);

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // Q0B - Marketing flight number
      _airSeg->marketingFlightNumber() = atoi(xmlValue.c_str());
      break;
    case 2: // D01 - Departure date
      _airSeg->departureDT() = convertDate(xmlValue.c_str());
      break;
    case 3: // D31 - Departure time
      timeInMins = atoi(xmlValue.c_str());
      _airSeg->departureDT() = _airSeg->departureDT() + tse::Hours(timeInMins / 60) +
                               tse::Minutes(timeInMins % 60) + tse::Seconds(0);
      break;
    case 4: // A01 - Departure airport
      _airSeg->origAirport() = xmlValue.c_str();
      break;
    case 5: // D02 - Arrival date
      _airSeg->arrivalDT() = convertDate(xmlValue.c_str());
      break;
    case 6: // D32 - Arrival time
      timeInMins = atoi(xmlValue.c_str());
      _airSeg->arrivalDT() = _airSeg->arrivalDT() + tse::Hours(timeInMins / 60) +
                             tse::Minutes(timeInMins % 60) + tse::Seconds(0);
      break;
    case 7: // A02 - Arrival airport
      _airSeg->destAirport() = xmlValue.c_str();
      break;
    case 8: // B00 - Marketing carrier code
      _airSeg->setMarketingCarrierCode(xmlValue.c_str());
      break;
    case 9: // B40 - Equipment code
      _airSeg->equipmentType() = xmlValue.c_str();
      break;
    case 10: // C5A - Base fare amount
      _farePath->setTotalNUCAmount(atof(xmlValue.c_str()));
      break;
    case 11: // C46 - Base currency code
      //_farePath->currency() = xmlValue.c_str();
      break;
    case 12: // C5F - Equivalent fare amount
      LOG4CXX_WARN(logger, "Not storing C5F");
      //_farePath->totalNUCAmount() = atof(xmlValue.c_str());
      break;
    case 13: // C45 - Equivalent currency code
      LOG4CXX_WARN(logger, "Not storing C45");
      break;
    case 14: // B50 - Fare basis code
      //_fare->fareClass() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
TaxModelMap::saveFlightInformation()
{
  if (!_itin->paxGroup().empty())
    return;

  LOG4CXX_DEBUG(logger, "In saveFlightInformation");

  _airSeg->origin() = _taxTrx->dataHandle().getLoc(_airSeg->origAirport(), _airSeg->departureDT());
  _airSeg->destination() =
      _taxTrx->dataHandle().getLoc(_airSeg->destAirport(), _airSeg->arrivalDT());

  LOG4CXX_DEBUG(logger,
                "Flight From: " << _airSeg->origAirport() << " TO " << _airSeg->destAirport());
  LOG4CXX_DEBUG(logger, "PaxType Group Size: " << _itin->paxGroup().size());

  _itin->travelSeg().push_back(_airSeg);
  _itin->setTravelDate(_airSeg->departureDT());

  // PaxTypeFare...
  PaxTypeFare* paxTypeFare;
  _taxTrx->dataHandle().get(paxTypeFare);

  paxTypeFare->paxType() = _paxType;
  paxTypeFare->nucFareAmount() = _farePath->getTotalNUCAmount();

  // FareUsage...
  FareUsage* fareUsage;
  _taxTrx->dataHandle().get(fareUsage); // lint !e530
  fareUsage->paxTypeFare() = paxTypeFare;
}

void
TaxModelMap::storeBookingClass(const xercesc::Attributes& attrs)
{
  if (!_itin->paxGroup().empty())
    return;

  LOG4CXX_DEBUG(logger, "In storeBookingClass");

  ClassOfService* classOfService;
  _taxTrx->dataHandle().get(classOfService); // lint !e530

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // B30 - Booking code
      classOfService->bookingCode() = xmlValue.c_str();
      _airSeg->classOfService().push_back(classOfService);
      _airSeg->setBookingCode(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
TaxModelMap::saveBookingClass()
{
  LOG4CXX_DEBUG(logger, "In saveBookingClass");
}

void
TaxModelMap::storeBillingInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeBillingInformation");

  Billing* billing;
  _taxTrx->dataHandle().get(billing); // lint !e530
  _taxTrx->billing() = billing;

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
    case 12: // C21
      billing->clientServiceName() = xmlValue.c_str();
      break;
    case 13: // L00 is obsolete
      if (billing->clientTransactionID() == 0)
      {
        billing->clientTransactionID() = Billing::string2transactionId(xmlValue.c_str());
      }
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  billing->updateTransactionIds(_taxTrx->transactionId());
  billing->updateServiceNames(Billing::SVC_TAX);
}

void
TaxModelMap::saveBillingInformation()
{
  LOG4CXX_DEBUG(logger, "In saveBillingInformation");
}

void
TaxModelMap::storeDynamicConfigOverride(const xercesc::Attributes& attrs)
{
  handleDynamicConfig(((Mapping*)_currentMapEntry)->members, attrs);
}
} // tse namespace
