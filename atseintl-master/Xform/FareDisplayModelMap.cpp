//----------------------------------------------------------------------------
//
//  File:  FareDisplayModelMap.cpp
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

#include "Xform/FareDisplayModelMap.h"

#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "Common/XMLChString.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/ERDFareComp.h"
#include "DataModel/ERDFltSeg.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/PaxTypeInfo.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Rules/RuleConst.h"

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include <set>
#include <string>
#include <vector>

#include <ctype.h>
#include <time.h>

namespace tse
{
FALLBACK_DECL(neutralToActualCarrierMapping);
FALLBACK_DECL(fallbackFareDisplayByCabinActivation);
FALLBACK_DECL(fallbackFRRProcessingRetailerCode);
FALLBACK_DECL(fallbackSanitizerError1);
FALLBACK_DECL(fallbackFixProcessingRetailerCodeXRS);
FALLBACK_DECL(fallbackWPRDASLFix);
FALLBACK_DECL(fallbackAgencyRetailerHandlingFees);

const std::string FareDisplayModelMap::BAD_GLOBAL = "XX";
static std::string EPR_FFOCUS = "FFOCUS";
static std::string EPR_ORDFQD = "ORGFQD";
static std::string EPR_AGYRET = "AGYRET";


static Logger
logger("atseintl.Xform.FareDisplayModelMap");

FareDisplayModelMap::~FareDisplayModelMap()
{
  for (auto& elem : _classMap)
  {
    delete (Mapping*)elem.second;
  }
}

const char*
FareDisplayModelMap::verifyLocation(const char* loc)
{
  if (!_trx->dataHandle().getLoc(loc, _fareDisplayTrx->getRequest()->ticketingDT()))
  {
    std::string errMsg = std::string("UNKNOWN LOC ") + loc;
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, errMsg.c_str());
  }

  return loc;
}

bool
FareDisplayModelMap::createMap()
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
      if (tagName == "FAREDISPLAYREQUEST")
      {
        mapping->func = &tse::FareDisplayModelMap::storeFareDisplayInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveFareDisplayInformation;
      }
      else if (tagName == "AGI")
      {
        mapping->func = &tse::FareDisplayModelMap::storeAgentInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveAgentInformation;
      }
      else if (tagName == "BIL")
      {
        mapping->func = &tse::FareDisplayModelMap::storeBillingInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveBillingInformation;
      }
      else if (tagName == "PCL")
      {
        mapping->func = &tse::FareDisplayModelMap::storePreferredCarrier;
        mapping->trxFunc = &tse::FareDisplayModelMap::savePreferredCarrier;
      }
      else if (tagName == "PRO")
      {
        mapping->func = &tse::FareDisplayModelMap::storeProcessingOptions;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveProcessingOptions;
      }
      else if (tagName == "PXI")
      {
        mapping->func = &tse::FareDisplayModelMap::storePassengerInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::savePassengerInformation;
      }
      else if (tagName == "DIG")
      {
        mapping->func = &tse::FareDisplayModelMap::storeDiagInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveDiagInformation;
      }
      else if (tagName == "RCG")
      {
        mapping->func = &tse::FareDisplayModelMap::storeRuleCategoryInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveRuleCategoryInformation;
      }
      else if (tagName == "ACD")
      {
        mapping->func = &tse::FareDisplayModelMap::storeAlphaCodeInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveAlphaCodeInformation;
      }
      else if (tagName == "CCD")
      {
        mapping->func = &tse::FareDisplayModelMap::storeCombinabilityCodeInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveCombinabilityCodeInformation;
      }
      else if (tagName == "SMC")
      {
        mapping->func = &tse::FareDisplayModelMap::storeSecondaryMarketsAndCarrierInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveSecondaryMarketsAndCarrierInformation;
      }
      else if (tagName == "C25")
      {
        mapping->func = &tse::FareDisplayModelMap::storeCAT25Information;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveCAT25Information;
      }
      else if (tagName == "C35")
      {
        mapping->func = &tse::FareDisplayModelMap::storeCAT35Information;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveCAT35Information;
      }
      else if (tagName == "DFI")
      {
        mapping->func = &tse::FareDisplayModelMap::storeDFIInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveDFIInformation;
      }
      else if (tagName == "ECN")
      {
        mapping->func = &tse::FareDisplayModelMap::storeECNInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::nullSaveFunction;
      }
      else if (tagName == "OAO")
      {
        mapping->func = &tse::FareDisplayModelMap::storeOAOInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveOAOInformation;
      }
      else if (tagName == "DAO")
      {
        mapping->func = &tse::FareDisplayModelMap::storeDAOInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveDAOInformation;
      }
      else if (tagName == "CAL")
      {
        mapping->func = &tse::FareDisplayModelMap::storeCALInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveCALInformation;
      }
      else if (tagName == "SEG")
      {
        mapping->func = &tse::FareDisplayModelMap::storeSEGInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveSEGInformation;
      }
      else if (tagName == "ERD")
      {
        mapping->func = &tse::FareDisplayModelMap::storeERDInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveERDInformation;
      }
      else if (tagName == "HPU")
      {
        mapping->func = &tse::FareDisplayModelMap::storeHPUInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveHPUInformation;
      }
      else if (tagName == "PRICINGRESPONSE" || tagName == "OCM" || tagName == "OCH" ||
               tagName == "OCG" || tagName == "OCP" || tagName == "OCT" || tagName == "OBF")
      {
        mapping->func = &tse::FareDisplayModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::FareDisplayModelMap::nullSaveFunction;
      }
      else if (tagName == "MSG")
      {
        mapping->func = &tse::FareDisplayModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::FareDisplayModelMap::nullSaveFunction;
      }
      else if (tagName == "SUR")
      {
        mapping->func = &tse::FareDisplayModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::FareDisplayModelMap::nullSaveFunction;
      }
      else if (tagName == "TBR")
      {
        mapping->func = &tse::FareDisplayModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::FareDisplayModelMap::nullSaveFunction;
      }
      else if (tagName == "FIR")
      {
        mapping->func = &tse::FareDisplayModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::FareDisplayModelMap::nullSaveFunction;
      }
      else if (tagName == "P45")
      {
        mapping->func = &tse::FareDisplayModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::FareDisplayModelMap::nullSaveFunction;
      }
      else if (tagName == "CAT")
      {
        mapping->func = &tse::FareDisplayModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::FareDisplayModelMap::nullSaveFunction;
      }
      else if (tagName == "TAX")
      {
        mapping->func = &tse::FareDisplayModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::FareDisplayModelMap::nullSaveFunction;
      }
      else if (tagName == "TBD")
      {
        mapping->func = &tse::FareDisplayModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::FareDisplayModelMap::nullSaveFunction;
      }
      else if (tagName == "SUM")
      {
        mapping->func = &tse::FareDisplayModelMap::storeSUMInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::nullSaveFunction;
      }
      else if (tagName == "ABL")
      {
        mapping->func = &tse::FareDisplayModelMap::nullStoreFunction;
        mapping->trxFunc = &tse::FareDisplayModelMap::nullSaveFunction;
      }
      else if (tagName == "DTS")
      {
        mapping->func = &tse::FareDisplayModelMap::storeDTSInformation;
        mapping->trxFunc = &tse::FareDisplayModelMap::saveDTSInformation;
      }
      else if (tagName == "DYNAMICCONFIG")
      {
        mapping->func = &tse::FareDisplayModelMap::storeDynamicConfigOverride;
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
  }
  return true;
}

bool
FareDisplayModelMap::classMapEntry(std::string& tagName, const xercesc::Attributes& atts)
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
FareDisplayModelMap::saveMapEntry(std::string& tagName)
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
FareDisplayModelMap::nullStoreFunction(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In nullStoreFunction");

  LOG4CXX_DEBUG(logger, "Finished in nullStoreFunction");
}

void
FareDisplayModelMap::nullSaveFunction()
{
  LOG4CXX_DEBUG(logger, "In nullSaveFunction");
}

void
FareDisplayModelMap::storeFareDisplayInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeFareDisplayInformation");

  _dataHandle.get(_fareDisplayTrx);
  _trx = _fareDisplayTrx;

  FareDisplayRequest* request;
  _fareDisplayTrx->dataHandle().get(request); // lint !e530
  _fareDisplayTrx->setRequest(request);

  FareDisplayOptions* options;
  _fareDisplayTrx->dataHandle().get(options); // lint !e530
  _fareDisplayTrx->setOptions(options);

  _fareDisplayTrx->dataHandle().get(_airSeg);

  bool ticketingDateReceived = false;
  _ticketDatePresent = false;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // A01 - Departure airport code
      _airSeg->origAirport() = xmlValue.c_str();
      break;
    case 2: // A02 - Arrival airport code
      _airSeg->destAirport() = xmlValue.c_str();
      break;
    case 3: // D06 - Ticketing date
    {
      request->ticketingDT() = convertDate(xmlValue.c_str());
      ticketingDateReceived = true;
      _ticketDatePresent = true;
    }
    break;
    case 4: // D01 - Departure date
      _airSeg->departureDT() = convertDate(xmlValue.c_str());
      _airSeg->earliestDepartureDT() = convertDate(xmlValue.c_str());
      _airSeg->latestDepartureDT() = convertDate(xmlValue.c_str());
      // Change time from 00:00:00 to 23:59:59
      _airSeg->latestDepartureDT() =
          _airSeg->latestDepartureDT().addSeconds(23 * 60 * 60 + 59 * 60 + 59);
      _airSeg->earliestArrivalDT() = convertDate(xmlValue.c_str());
      _airSeg->latestArrivalDT() = convertDate(xmlValue.c_str());
      // Change time from 00:00:00 to 23:59:59
      _airSeg->latestArrivalDT() =
          _airSeg->latestArrivalDT().addSeconds(23 * 60 * 60 + 59 * 60 + 59); // 86399
      _departureDatePresent = true;
      break;
    case 5: // BI0 - Inclusion code
    {
      if(fallback::fallbackFareDisplayByCabinActivation(_trx))
      {
        char tmpBuf[10];
        strcpy(tmpBuf, xmlValue.c_str());
        if (tmpBuf[2] == ' ')
          tmpBuf[2] = '\0';
        else if (tmpBuf[3] == ' ')
          tmpBuf[3] = '\0';
        request->inclusionCode() = tmpBuf;
        request->requestedInclusionCode() =
            tmpBuf; // This inclusion code is never altered and used for Group/Sort
      }
      else
      {
        std::string tmpBuf(xmlValue.c_str());
        if (fallback::fallbackSanitizerError1(_trx))
        {
          if (tmpBuf[2] == ' ')
            tmpBuf[2] = '\0';
          else if (tmpBuf[3] == ' ')
            tmpBuf[3] = '\0';
        }
        else
        {
          size_t sz(tmpBuf.size());
          if (sz > 2 && tmpBuf[2] == ' ')
          {
            tmpBuf[2] = '\0';
          }
          else if (sz > 3 && tmpBuf[3] == ' ')
          {
            tmpBuf[3] = '\0';
          }
        }
        request->inclusionCode() = tmpBuf;
        request->requestedInclusionCode() = tmpBuf;
        // This inclusion code is never altered and used for Group/Sort
        // for multiple consecutive inclusion codes
        if (request->inclusionCode() == ALL_CLASS_FARES)
        {
          request->inclusionCode() = "PBFBJBBBSBYB";
          request->requestedInclusionCode() = "PBFBJBBBSBYB";
          request->multiInclusionCodes() = true;
        }
        else if((request->inclusionCode().size() == 4 &&
                 request->inclusionCode() != "CTRW" && request->inclusionCode() != "RWCT") ||
                request->inclusionCode().size() > 4)
        {
          if(request->requestedInclusionCode().find("AB") != std::string::npos)
            throw NonFatalErrorResponseException(ErrorResponseException::DUPLICATE_CABIN_INCLUSION_CODE);

          if(request->requestedInclusionCode().size()%2 != 0)
             throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INCLUSION_CODE);

          std::string sub_string;
          InclusionCode checkedInclCodes;
          uint8_t sizeIncl = request->requestedInclusionCode().size()/2;
          for (uint8_t number = 0; number < sizeIncl;  ++number)
          {
            sub_string = request->requestedInclusionCode().substr(number*2, 2);
            if(checkedInclCodes.find(sub_string) != std::string::npos)
              throw NonFatalErrorResponseException(ErrorResponseException::DUPLICATE_CABIN_INCLUSION_CODE);

            if(request->inclusionNumber(sub_string) == 0)
              throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INCLUSION_CODE);

            checkedInclCodes.append(sub_string);
          }
          request->multiInclusionCodes() = true;
        }
      }
    }
    break;
    case 6: // S58 - Request type
      request->requestType() = xmlValue.c_str();
      break;
    case 7: // Q46 - line number (short RD only)
      options->lineNumber() = atoi(xmlValue.c_str());
      break;
    case 8: // PBB - No carrier entered
      request->carrierNotEntered() = xmlValue.c_str()[0];
      break;
    case 9: // PBB - No carrier entered
      options->templateType() = xmlValue.c_str()[0];
      break;
    case 10: // S82 - Output type
      request->outputType() = xmlValue.c_str();
      break;
    case 11: // D54 - Ticket time override
    {
      if (ticketingDateReceived)
      {
        options->ticketTimeOverride() = atoi(xmlValue.c_str());
        request->ticketingDT() = request->ticketingDT() +
                                 tse::Hours(options->ticketTimeOverride() / 60) +
                                 tse::Minutes(options->ticketTimeOverride() % 60) + tse::Seconds(0);
      }
    }
    break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
  _trx->dataHandle().setTicketDate(request->ticketingDT());

  if (!_airSeg->origAirport().empty() && _airSeg->origAirport() == _airSeg->destAirport())
    _fareDisplayTrx->getOptions()->setRtw(true);
}

void
FareDisplayModelMap::saveFareDisplayInformation()
{
  LOG4CXX_DEBUG(logger, "In saveFareDisplayInformation");

  FareDisplayRequest* request = _fareDisplayTrx->getRequest();

  if (_fareDisplayTrx->isShortRequest())
  {
    request->displayPassengerTypes().clear();
  }

  // Validate requested pax type codes
  if (_fareDisplayTrx->isFQ() && !request->displayPassengerTypes().empty() &&
      !isValidPaxType(request->displayPassengerTypes(), _fareDisplayTrx->dataHandle()))
  {
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_PAX_TYPE_CODE);
  }

  _reqXmlValidator.validateETicketOptionStatus(_fareDisplayTrx);
  _reqXmlValidator.validateFareQuoteCurrencyIndicator(_fareDisplayTrx);

  if (!_ticketDatePresent)
  {
    _reqXmlValidator.setTicketingDate(_fareDisplayTrx, _ticketDatePresent);
  }

  _reqXmlValidator.validateFQDateRange(_fareDisplayTrx);

  if (!_departureDatePresent)
  {
    _airSeg->departureDT() = _fareDisplayTrx->getRequest()->ticketingDT();
    _airSeg->earliestDepartureDT() = _fareDisplayTrx->getRequest()->ticketingDT();
    _airSeg->latestDepartureDT() = _fareDisplayTrx->getRequest()->ticketingDT();
    // Change time from 00:00:00 to 23:59:59
    _airSeg->latestDepartureDT() =
        _airSeg->latestDepartureDT().addSeconds(23 * 60 * 60 + 59 * 60 + 59);
    _airSeg->earliestArrivalDT() = _fareDisplayTrx->getRequest()->ticketingDT();
    _airSeg->latestArrivalDT() = _fareDisplayTrx->getRequest()->ticketingDT();
    // Change time from 00:00:00 to 23:59:59
    _airSeg->latestArrivalDT() =
        _airSeg->latestArrivalDT().addSeconds(23 * 60 * 60 + 59 * 60 + 59); // 86399
    _departureDatePresent = true;
  }
  _reqXmlValidator.validateFQReturnDate(_fareDisplayTrx, _airSeg);

  FareDisplayOptions* options = _fareDisplayTrx->getOptions();

  // Set validate Rules indicator to false for long RD requests
  if (_fareDisplayTrx->isLongRD() || _fareDisplayTrx->isERD())
  {
    options->validateRules() = 'F';
  }

  // Create an itinerary
  Itin* itin;
  _fareDisplayTrx->travelSeg().push_back(_airSeg);
  _fareDisplayTrx->dataHandle().get(itin); // lint !e530

  // Save the Air Segment
  itin->travelSeg().push_back(_airSeg);

  if (!_fareDisplayTrx->isERD())
  {
    // set origin and destination in the AirSeg
    const Loc* origin =
        _fareDisplayTrx->dataHandle().getLoc(_airSeg->origAirport(), DateTime::localTime());
    const Loc* destination =
        _fareDisplayTrx->dataHandle().getLoc(_airSeg->destAirport(), DateTime::localTime());

    if ((origin == nullptr || destination == nullptr) && (request->requestType() != FARE_MILEAGE_REQUEST))
    {
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_CITY_AIRPORT_CODE);
    }
    else
    {
      _airSeg->origin() = origin;
      _airSeg->destination() = destination;
    }

    // Assign offMultiCity and boardMultiCity
    setMultiTransportCity();
    LOG4CXX_DEBUG(logger, "boardMultiCity=" << _airSeg->boardMultiCity().c_str());
    LOG4CXX_DEBUG(logger, "offMultiCity=" << _airSeg->offMultiCity());

    // Is there a return date?
    LOG4CXX_DEBUG(logger,
                  "Return date valid: " << _fareDisplayTrx->getRequest()->returnDate().isValid());
    if (_fareDisplayTrx->getRequest()->returnDate().isValid())
    {
      AirSeg* returnSeg;
      _fareDisplayTrx->dataHandle().get(returnSeg); // lint !e530
      returnSeg->segmentOrder() = 1;
      returnSeg->departureDT() = _fareDisplayTrx->getRequest()->returnDate();
      returnSeg->earliestDepartureDT() = _fareDisplayTrx->getRequest()->returnDate();
      returnSeg->latestDepartureDT() = _fareDisplayTrx->getRequest()->returnDate();
      returnSeg->earliestArrivalDT() = _fareDisplayTrx->getRequest()->returnDate();
      returnSeg->latestArrivalDT() = _fareDisplayTrx->getRequest()->returnDate();
      returnSeg->origAirport() = _airSeg->destAirport();
      returnSeg->origin() = destination;
      returnSeg->boardMultiCity() = _airSeg->offMultiCity();
      returnSeg->destAirport() = _airSeg->origAirport();
      returnSeg->destination() = origin;
      returnSeg->offMultiCity() = _airSeg->boardMultiCity();
      itin->travelSeg().push_back(returnSeg);
    }
  }
  _fareDisplayTrx->itin().push_back(itin);

  // if MCP carrier partition with no carrier - get all MCP carriers
  if (_isMcpPartition && _fareDisplayTrx->preferredCarriers().empty() && !options->isAllCarriers())
  {
    MCPCarrierUtil::getPreferedCarriers(_fareDisplayTrx->billing()->partitionID(),
                                        _fareDisplayTrx->preferredCarriers());
  }

  if (_fareDisplayTrx->preferredCarriers().size() > 1)
    _fareDisplayTrx->multipleCarriersEntered() = true;

  if (_ticketDatePresent)
  {
    _reqXmlValidator.validateTicketingDate(_fareDisplayTrx, itin);
  }
}

void
FareDisplayModelMap::storeAgentInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeAgentInformation");

  Agent* agent = _fareDisplayTrx->getRequest()->ticketingAgent();
  if (agent == nullptr)
  {
    _fareDisplayTrx->dataHandle().get(agent);
    _fareDisplayTrx->getRequest()->ticketingAgent() = agent;
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
    {
      char tmpBuf[10];
      strcpy(tmpBuf, xmlValue.c_str());
      if (tmpBuf[3] == ' ')
        tmpBuf[3] = '\0';
      agent->airlineDept() = tmpBuf;
    }
    break;
    case 9: // B00 - Originating carrier
      agent->cxrCode() = checkAndSwapCarrier(xmlValue.c_str());
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
    case 15: // AE0 - PCC Host Carrier
      // LATAM MCP-S
      if(!fallback::neutralToActualCarrierMapping(_trx) &&
        MCPCarrierUtil::isNeutralCarrier(xmlValue.c_str()))
      {
        agent->hostCarrier() = MCPCarrierUtil::swapFromNeutralToActual(xmlValue.c_str());
      }
      else
        agent->hostCarrier() = MCPCarrierUtil::swapToActual(_trx, xmlValue.c_str());
      break;
    case 16: // AE2 - Office/Station code
      agent->officeStationCode() = xmlValue.c_str();
      break;
    case 17: // AE3 - Default ticketing carrier
      agent->defaultTicketingCarrier() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::saveAgentInformation()
{
  FareDisplayRequest* request = _fareDisplayTrx->getRequest(); // convenience reference

  Agent* agent = request->ticketingAgent();

  if (agent == nullptr)
  {
    _trx->dataHandle().get(agent); // lint !e530
    request->ticketingAgent() = agent;
  }

  // lint --e{661}
  // PSS Bug
  if (agent->mainTvlAgencyPCC().length() > 0 && agent->tvlAgencyPCC().length() == 0)
  {
    std::string tmpString = agent->agentCity();
    agent->agentCity() = agent->mainTvlAgencyPCC();
    agent->mainTvlAgencyPCC() = tmpString;
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

  if (TrxUtil::swsPoAtsePath(*_fareDisplayTrx))
    _reqXmlValidator.getAgentLocationAndCurrency(_fareDisplayTrx);

  if (!agent->agentLocation() && _fareDisplayTrx->billing())
    throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
  LOG4CXX_DEBUG(logger, "End of saveAgentInformation");
}

void
FareDisplayModelMap::storeBillingInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeBillingInformation");

  Billing* billing;
  _fareDisplayTrx->dataHandle().get(billing); // lint !e530
   if (_fareDisplayTrx->billing())
      billing->requestPath() = _fareDisplayTrx->billing()->requestPath();
  _fareDisplayTrx->billing() = billing;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    if (_inDts)
    {
      switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
      {
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 7:
      case 8:
      case 9:
        if (_fareDisplayTrx->isERD())
        {
          storeCommonBillingInformation(i, attrs, *billing);
        }
        break;
      case 6: // C20 - Parent service name
        _fareDisplayTrx->getOptions()->isWqrd() = (xmlValue == "INTLWQPR");
        break;

      default:
        LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
        break;
      }
    }
    else
    {
      switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
      {
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 7:
      case 8:
      case 9:
        storeCommonBillingInformation(i, attrs, *billing);
        break;
      case 6: // C20 - Parent service name
        billing->parentServiceName() = xmlValue.c_str();
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
  }

  billing->updateTransactionIds(_fareDisplayTrx->transactionId());
  if (_fareDisplayTrx->isERD())
  {
    billing->parentServiceName() = billing->getServiceName(Billing::SVC_ENHANCED_RULE_DISPLAY);
    billing->clientServiceName() = billing->getBusinessFunction(Billing::SVC_FARE_DISPLAY);
    billing->updateServiceNames(Billing::SVC_ENHANCED_RULE_DISPLAY);
  }
  else
    billing->updateServiceNames(Billing::SVC_FARE_DISPLAY);
}

void
FareDisplayModelMap::storeCommonBillingInformation(const int i,
                                                   const xercesc::Attributes& attrs,
                                                   Billing& billing)
{
  LOG4CXX_DEBUG(logger, "In storeCommonBillingInformation");

  XMLChString xmlStr(attrs.getLocalName(i));
  XMLChString xmlValue(attrs.getValue(i));

  switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
  {
  case 1: // A20 - User pseudoCity code
    billing.userPseudoCityCode() = xmlValue.c_str();
    break;
  case 2: // Q03 - User station
    billing.userStation() = xmlValue.c_str();
    break;
  case 3: // Q02 - User branch
    billing.userBranch() = xmlValue.c_str();
    break;
  case 4: // AE0 - Partition ID
    // LATAM MCP-S
    if(!fallback::neutralToActualCarrierMapping(_trx) &&
        MCPCarrierUtil::isNeutralCarrier(xmlValue.c_str()))
    {
      billing.partitionID() = MCPCarrierUtil::swapFromNeutralToActual(xmlValue.c_str());
    }
    else
      billing.partitionID() = checkAndSwapCarrier(xmlValue.c_str());

    _isMcpPartition = MCPCarrierUtil::isMcpHost(billing.partitionID());
    if (MCPCarrierUtil::isIAPCarrierRestricted(billing.partitionID()))
      throw ErrorResponseException(ErrorResponseException::MCP_IAP_RESTRICTED,
                                   ("UNABLE TO PROCESS-ENTRY RESTRICTED IN " +
                                    billing.partitionID() + " PARTITION").c_str());
    break;
  case 5: // AD0 - User set address
    billing.userSetAddress() = xmlValue.c_str();
    break;
  case 7: // A22 - aaaCity
    billing.aaaCity() = xmlValue.c_str();
    break;
  case 8: // AA0 - agentSine
    billing.aaaSine() = xmlValue.c_str();
    break;
  case 9: // A70 - Action code
    billing.actionCode() = xmlValue.c_str();
    break;
  default:
    LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
    break;
  }
}

void
FareDisplayModelMap::saveBillingInformation()
{
  LOG4CXX_DEBUG(logger, "In saveBillingInformation");

  if (TrxUtil::swsPoAtsePath(*_fareDisplayTrx) && _fareDisplayTrx->getRequest()->ticketingAgent())
    _reqXmlValidator.getAgentLocationAndCurrency(_fareDisplayTrx);

  if (_fareDisplayTrx->getRequest()->ticketingAgent() &&
      !_fareDisplayTrx->getRequest()->ticketingAgent()->agentLocation())
    throw ErrorResponseException(ErrorResponseException::AGENT_PCC_NON_EXISTENT);
}

void
FareDisplayModelMap::storePreferredCarrier(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storePreferredCarrier");

  DateTime travelDate;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // B01 - Carrier code
     {
      CarrierCode cxr = xmlValue.c_str();
      if(!fallback::neutralToActualCarrierMapping(_trx) &&
          MCPCarrierUtil::isNeutralCarrier(cxr))
      {
        cxr = "**";
      }
      addPreferredCarrier(checkAndSwapCarrier(cxr));
     }
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::savePreferredCarrier()
{
  LOG4CXX_DEBUG(logger, "In savePreferredCarrier");
}

void
FareDisplayModelMap::storeProcessingOptions(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeProcessingOptions");

  FareDisplayRequest* request = _fareDisplayTrx->getRequest();
  FareDisplayOptions* options = _fareDisplayTrx->getOptions();
  int32_t diagNum;

  bool isPRM = false;
  uint8_t rcqCount = 0;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // A60 - Global direction
      if (xmlValue.c_str() == FareDisplayModelMap::BAD_GLOBAL ||
          !strToGlobalDirection(request->globalDirection(), xmlValue.c_str()))
      {
        throw NonFatalErrorResponseException(
            ErrorResponseException::INVALID_GLOBAL_DIRECTION_REQUESTED);
      }
      break;
    case 2: // C40 - Display currency code
      options->currencyOverride() = xmlValue.c_str();
      break;
    case 3: // C41 - Alternate display currency code
      options->alternateCurrency() = xmlValue.c_str();
      break;
    case 4: // A22 - AAA override
      request->salePointOverride() = request->ticketPointOverride() =
          verifyLocation(xmlValue.c_str());
      break;
    case 5: // D02 - Return date
      request->returnDate() = convertDate(xmlValue.c_str());
      break;
    case 6: // D05 - Date range lower
      request->dateRangeLower() = convertDate(xmlValue.c_str());
      _airSeg->departureDT() = convertDate(xmlValue.c_str());
      _airSeg->earliestDepartureDT() = convertDate(xmlValue.c_str());
      _airSeg->latestDepartureDT() = convertDate(xmlValue.c_str());
      _airSeg->earliestArrivalDT() = convertDate(xmlValue.c_str());
      _airSeg->latestArrivalDT() = convertDate(xmlValue.c_str());
      _departureDatePresent = true;
      break;
    case 7: // D06 - Date range upper
      request->dateRangeUpper() = convertDate(xmlValue.c_str());
      _airSeg->latestDepartureDT() = convertDate(xmlValue.c_str());
      _airSeg->latestArrivalDT() = convertDate(xmlValue.c_str());
      break;
    case 8: // D00 - Preferred travel date
      request->preferredTravelDate() = convertDate(xmlValue.c_str());
      break;
    case 9: // P04 - One way fare
      options->oneWayFare() = xmlValue.c_str()[0];
      break;
    case 10: // P05 - Round trip fare
      options->roundTripFare() = xmlValue.c_str()[0];
      break;
    case 11: // P03 - Half round trip fare
      options->halfRoundTripFare() = xmlValue.c_str()[0];
      break;
    case 12: // P88 - Sort ascending
      options->sortAscending() = xmlValue.c_str()[0];
      break;
    case 13: // P89 - Sort descending
      options->sortDescending() = xmlValue.c_str()[0];
      break;
    case 14: // B30 - Booking code
      request->bookingCode() = xmlValue.c_str();
      break;
    case 15: // BE0 - Ticket designator
      request->ticketDesignator() = xmlValue.c_str();
      break;
    case 16: // B50 - Fare basis code
      request->fareBasisCode() = xmlValue.c_str();
      break;
    case 17: // P1Z - Private fares
      options->privateFares() = xmlValue.c_str()[0];
      break;
    case 18: // P1Y - Public fares
      options->publicFares() = xmlValue.c_str()[0];
      break;
    //       case 19:                // P0K - Opaque entry    delete when removing FB flag
    //            options->opaqueEntry() = xmlValue.c_str()[0];
    //            break;
    case 20: // AC0 - Corporate ID
    {
      request->corporateID() = xmlValue.c_str();

      DateTime travelDate;

      if (_airSeg && _airSeg->departureDT().isValid())
      {
        travelDate = _airSeg->departureDT();
      }
      else
      {
        travelDate = DateTime::localTime();
      }

      if (!_trx->dataHandle().corpIdExists(request->corporateID(), travelDate))
      {
        std::string errMsg = "INVALID CORPORATE ID ";
        errMsg += request->corporateID();
        throw NonFatalErrorResponseException(ErrorResponseException::INVALID_CORP_ID,
                                             errMsg.c_str());
      }
    }
    break;
    case 21: // S07 - Account code
      request->accountCode() = xmlValue.c_str();
      break;
    case 22: // P84 - Exclude penalty fares
      options->excludePenaltyFares() = xmlValue.c_str()[0];
      break;
    case 23: // P85 - Exclude adv purchase fares
      options->excludeAdvPurchaseFares() = xmlValue.c_str()[0];
      break;
    case 24: // P86 - Exclude restricted fares
      options->excludeAdvPurchaseFares() = xmlValue.c_str()[0];
      break;
    case 25: // P87 - All carriers
      options->allCarriers() = xmlValue.c_str()[0];
      break;
    case 26: // Q16 - Exclude fares with penalty percentage
      options->excludePercentagePenaltyFares() = atoi(xmlValue.c_str());
      break;
    case 27: // Q17 - Number of fare levels returned
      request->numberOfFareLevels() = atoi(xmlValue.c_str());
      break;
    case 28: // P80 - Validate rules
      options->validateRules() = xmlValue.c_str()[0];
      break;
    case 29: // P81 - Reverse sort
      options->reverseSort() = xmlValue.c_str()[0];
      break;
    case 30: // P82 - Display base, tax and total amounts
      options->displayBaseTaxTotalAmounts() = xmlValue.c_str()[0];
      break;
    case 31: // P83 - Exclude min max stay fares
      options->excludeMinMaxStayFares() = xmlValue.c_str()[0];
      break;
    case 32: // P90 - Adult fares
      options->adultFares() = xmlValue.c_str()[0];
      break;
    case 33: // P91 - Child fares
      options->childFares() = xmlValue.c_str()[0];
      break;
    case 34: // P92 - Infant fares
      options->infantFares() = xmlValue.c_str()[0];
      break;
    case 35: // P93 - Unique account code
      options->uniqueAccountCode() = xmlValue.c_str()[0];
      break;
    case 36: // P94 - Unique corporate id
      options->uniqueCorporateID() = xmlValue.c_str()[0];
      break;
    case 37: // Q40 - template override
      diagNum = atoi(xmlValue.c_str());
      if (diagNum < MIN_DIAG_NBR)
      {
        options->templateOverride() = diagNum;
      }
      else // MIN_DIAG_NBR and above, are diagnostic requests
      {
        if (diagNum != 197)
          request->diagnosticNumber() = diagNum;
      }
      break;
    case 38: // PAM - Rule menu display request
      options->ruleMenuDisplay() = xmlValue.c_str()[0];
      break;
    case 39: // PAN - Routing display request
      options->routingDisplay() = xmlValue.c_str()[0];
      break;
    case 40: // PAO - Header display request
      options->headerDisplay() = xmlValue.c_str()[0];
      break;
    case 41: // PAP - Intl construction request
      options->IntlConstructionDisplay() = xmlValue.c_str()[0];
      break;
    case 42: // PAQ - FB display
      options->FBDisplay() = xmlValue.c_str()[0];
      break;
    case 43: // PBA - Combinability scoreboard display request
      options->combScoreboardDisplay() = xmlValue.c_str()[0];
      break;
    case 44: // C46 - Base fare currency code
      options->baseFareCurrency() = xmlValue.c_str();
      break;
    case 45: // B00 - Fare carrier code
      options->carrierCode() = checkAndSwapCarrier(xmlValue.c_str());
      break;
    case 46: // BJ0 - Fare class
      options->fareClass() = xmlValue.c_str();
      break;
    case 47: // Q3W - Fare tariff
      options->fareTariff() = atoi(xmlValue.c_str());
      break;
    case 48: // S49 - routing
      options->routing() = xmlValue.c_str();
      break;
    case 49: // S37 - Vendor code
      options->vendorCode() = xmlValue.c_str();
      break;
    case 50: // Q46 - Link number
      options->linkNumber() = atoi(xmlValue.c_str());
      break;
    case 51: // Q1K - Sequence number
      options->sequenceNumber() = atoi(xmlValue.c_str());
      break;
    case 52: // D12 - Create date
      options->createDate() = convertDate(xmlValue.c_str());
      break;
    case 53: // D55 - Create time HH-MM-SS.mmmmmm
    {
      std::string crTime = xmlValue.c_str();
      if (!FareDisplayUtil::updateCreateTimeFormat(crTime, false))
      {
        LOG4CXX_ERROR(logger, "Unable to convert time: " << xmlValue.c_str());
      }
      else
        options->createTime() = crTime;
    }
    break;
    case 54: // Q4N - Add/Sub line number
      request->addSubLineNumber() = atoi(xmlValue.c_str());
      break;
    case 55: // Q37 - Add/Sub percentage
      request->addSubPercentage() = atoi(xmlValue.c_str());
      break;
    case 56: // AM0 - gateway 1
      options->constructedAttributes().gateway1() = xmlValue.c_str();
      break;
    case 57: // AN0 - gateway 2
      options->constructedAttributes().gateway2() = xmlValue.c_str();
      break;
    case 58: // N1J - Construction Type
      options->constructedAttributes().constructionType() =
          (ConstructedFareInfo::ConstructionType)(atoi(xmlValue.c_str()));
      break;
    case 59: // C66 - Specified fare Amount
      options->constructedAttributes().specifiedFareAmount() = atof(xmlValue.c_str());
      break;
    case 60: // C6K - Constructed NUC Amount
      options->constructedAttributes().constructedNucAmount() = atof(xmlValue.c_str());
      break;
    case 61: // PBN - View private fares
      options->viewPrivateFares() = xmlValue.c_str()[0];
      break;
    case 62: // C5A - Calculated Fare Amount
      request->calcFareAmount() = atof(xmlValue.c_str());
      break;
    case 63: // PBO - Selling currency
      options->sellingCurrency() = xmlValue.c_str()[0];
      break;
    case 64: // PBP - One way
      options->oneWay() = xmlValue.c_str()[0];
      break;
    case 65: // PBQ - Half round trip
      options->halfRoundTrip() = xmlValue.c_str()[0];
      break;
    case 66: //  N1P - FD's cat35 type (valid for all PTFs)
      options->FDcat35Type() = xmlValue.c_str()[0];
      break;
    case 67: // S86 - routing number
      options->routingNumber() = xmlValue.c_str();
      break;
    case 68: // PBR - bsr reciprocal indicator
      if (xmlValue.c_str()[0] == 'T')
        options->bsrReciprocal() = true;
      break;
    case 69: // S53 - fare type
      options->fareType() = xmlValue.c_str();
      break;
    case 70: // S14 - green screen entry
      options->lineEntry() = xmlValue.c_str();
      break;
    case 71: // AK0 - fare origin
      request->fareOrigin() = xmlValue.c_str();
      break;
    case 72: // AL0 - fare destination
      request->fareDestination() = xmlValue.c_str();
      break;
    case 73: // S90 - rule number
      options->ruleNumber() = xmlValue.c_str();
      break;
    case 74: // PCC - Exclude Day/Time validation
      options->excludeDayTimeValidation() = xmlValue.c_str()[0];
      break;
    case 75: // PCF - Brand Grouping Opt Out
      options->brandGroupingOptOut() = xmlValue.c_str()[0];
      break;
    case 76: // PCG - rule number
      options->privateIndicator() = xmlValue.c_str();
      break;
    case 77: // AC1-AC4 - Corporate ID (multi AccCode/CorpId)
    case 78:
    case 79:
    case 80:
    {
      DateTime travelDate;

      if (_airSeg && _airSeg->departureDT().isValid())
      {
        travelDate = _airSeg->departureDT();
      }
      else
      {
        travelDate = DateTime::localTime();
      }

      if (_trx->dataHandle().corpIdExists(xmlValue.c_str(), travelDate))
      {
        std::vector<std::string>& corpIdVec = request->corpIdVec();
        if (std::find(corpIdVec.begin(), corpIdVec.end(), xmlValue.c_str()) == corpIdVec.end())
          corpIdVec.push_back(xmlValue.c_str());
      }
      else
      {
        std::vector<std::string>& incorrectCorpIdVec = request->incorrectCorpIdVec();
        if (std::find(incorrectCorpIdVec.begin(), incorrectCorpIdVec.end(), xmlValue.c_str()) ==
            incorrectCorpIdVec.end())
          incorrectCorpIdVec.push_back(xmlValue.c_str());
      }
    }
    break;
    case 81: // SM1-SM4 - Account code (multi AccCode/CorpId)
    case 82:
    case 83:
    case 84:
    {
      std::vector<std::string>& accCodeVec = request->accCodeVec();
      if (std::find(accCodeVec.begin(), accCodeVec.end(), xmlValue.c_str()) == accCodeVec.end())
        accCodeVec.push_back(xmlValue.c_str());
    }
    break;
    case 85:
      if (xmlValue.c_str()[0] == 'T')
        request->displayAccCode() = true;
      break;

    case 86: // Q0C - Requested segment numbers
    {
      std::string str(xmlValue.c_str());
      parseSegmentNumbers(str, options->requestedSegments());
    }
    break;

    case 87: // N0F - Surface segment numbers
    {
      std::string str(xmlValue.c_str());
      parseSegmentNumbers(str, options->surfaceSegments());
    }
    break;

    case 88: // B70 - Requested passenger type code
      options->requestedPaxTypeCode() = xmlValue.c_str();
      break;

    case 89: // B71 - Requested passenger type number
      options->requestedPaxTypeNumber() = atoi(xmlValue.c_str());
      break;

    case 90: // Q47 - Requested line number
      options->requestedLineNumber() = atoi(xmlValue.c_str());
      break;

    case 91: // ERR - Error returned from PSS
      options->errorFromPSS() = xmlValue.c_str()[0];
      break;

    case 92: // S15 - Employee profile entries
    {
      std::set<std::string>& theSet = options->eprKeywords();
      char* pHolder = nullptr;
      for (char* token = strtok_r((char*)xmlValue.c_str(), ",", &pHolder); token != nullptr;
           token = strtok_r(nullptr, ",", &pHolder))
      {
        theSet.insert(token);
      }
    }
    break;

    case 93: // XFF - exclude Fare Focus rules
    {
      if (xmlValue.c_str()[0] == 'T')
        options->setExcludeFareFocusRule(true);
    }
    break;

    case 94: // PDO - Fare Retailer rule
    {
      if (xmlValue.c_str()[0] == 'T')
        options->setPDOForFRRule(true);
    }
    break;

    case 95: // PDR - Fare Retailer rule
    {
      if (xmlValue.c_str()[0] == 'T')
        options->setPDRForFRRule(true);
    }
    break;
    case 96: // XRS - Fare Retailer rule
    {
      if (xmlValue.c_str()[0] == 'T')
        options->setXRSForFRRule(true);
    }
    break;
    case 97: // S38 - SourcePCC code for Cat35 FareRetailerRule
      options->setSourcePCCForCat35(xmlValue.c_str());
      break;
    case 98: // S39 - Source PCC code for Adjusted Selling fare FareRetailerRule
      options->setSourcePCCForASL(xmlValue.c_str());
      break;
    case 99:  // RT1 - fares with specified tariff number
      options->frrTariffNumber() = atoi(xmlValue.c_str());
      break;
    case 100: // RT2 - fares with specified rule number
      options->frrRuleNumber() = xmlValue.c_str();
      break;
    case 101: // RT3 - fares with specified fare type code
      options->frrFareTypeCode()= xmlValue.c_str();
      break;
    case 102: // RT4 - fares with specified display type
      options->frrDisplayType()= xmlValue.c_str()[0];
      break;
    case 103: // RT5 - fares with specified private indicator
      options->frrPrivateIndicator()= xmlValue.c_str()[0];
      break;
    case 104: // RT6 - select negotiated fares
      if (xmlValue.c_str()[0] == 'T')
        options->frrSelectCat35Fares() = true;
      break;
    case 105: // RT7 - select fare by rule fares
      if (xmlValue.c_str()[0] == 'T')
        options->frrSelectCat25Fares() = true;
      break;
    case 106: // RT8 - select cat15 fares
      if (xmlValue.c_str()[0] == 'T')
        options->frrSelectCat15Fares() = true;
      break;
    case 107: // PRM - related to retailer code setting
      if (!fallback::fallbackFRRProcessingRetailerCode(_trx))
        if (xmlValue.c_str()[0] == 'T' && request != nullptr)
        {
          request->setPRM(true);
          isPRM = true;
        }
      break;
    case 108: // Retailer code
      parseRetailerCode(xmlValue, request, rcqCount);
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  checkParsingRetailerCodeError(isPRM, rcqCount);
}

void
FareDisplayModelMap::saveProcessingOptions()
{
  LOG4CXX_DEBUG(logger, "In saveProcessingOptions");

  FareDisplayRequest* request = _fareDisplayTrx->getRequest();

  if (request != nullptr)
  {
    if (!request->accCodeVec().empty() || !request->corpIdVec().empty() ||
        !request->incorrectCorpIdVec().empty())
    {
      request->isMultiAccCorpId() = true;
    }

    const std::vector<std::string>& incorrectCorpIdVec = request->incorrectCorpIdVec();

    if (!incorrectCorpIdVec.empty() && request->corpIdVec().empty() &&
        request->accCodeVec().empty())
    {
      std::string errMsg = "INVALID CORPORATE ID";
      std::vector<std::string>::const_iterator corpIdIter = incorrectCorpIdVec.begin();
      std::vector<std::string>::const_iterator corpIdEnd = incorrectCorpIdVec.end();

      while (corpIdIter != corpIdEnd)
      {
        errMsg = errMsg + " " + (*corpIdIter);
        ++corpIdIter;
      }

      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_CORP_ID, errMsg.c_str());
    }

    int flag = 0;
    if (_fareDisplayTrx->getOptions()->isOneWayFare())
      flag++;
    if (_fareDisplayTrx->getOptions()->isRoundTripFare())
      flag++;
    if (_fareDisplayTrx->getOptions()->isHalfRoundTripFare())
      flag++;
    if (flag > 1)
    {
      throw NonFatalErrorResponseException(ErrorResponseException::ENTER_ONLY_1_JOURNEY_TYPE);
    }

    if (_fareDisplayTrx->getOptions()->sortAscending() &&
        _fareDisplayTrx->getOptions()->sortDescending()) // Validate P88 and P99
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "CONFLICT SORTING ORDER");
    }

    if (_fareDisplayTrx->getOptions()->privateFares() &&
        _fareDisplayTrx->getOptions()->publicFares()) // validate  P1Y and P1Z
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "SPECIFY PUBLIC OR PRIVATE");
    }

    if (_fareDisplayTrx->getOptions()->uniqueAccountCode() &&
        request->accountCode().empty()) // Validate P93
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "USE CORP ID-OR-ACCOUNT CODE");
    }

    if (_fareDisplayTrx->getOptions()->uniqueCorporateID() &&
        request->corporateID().empty()) // Validate P94
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "FORMAT - CORP ID MUST BE 3 ALPHA, 2 NUMERIC");
    }

    if (_fareDisplayTrx->getOptions()->isExcludeFareFocusRule() &&
        !_fareDisplayTrx->getOptions()->isKeywordPresent(EPR_FFOCUS))
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "NEED KEYWORD FFOCUS");

    if (_fareDisplayTrx->getOptions()->isPDOForFRRule() &&
        !_fareDisplayTrx->getOptions()->isKeywordPresent(EPR_ORDFQD))
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "NEED KEYWORD ORGFQD");

    if (_fareDisplayTrx->getOptions()->isPDRForFRRule() &&
        !_fareDisplayTrx->getOptions()->isKeywordPresent(EPR_AGYRET))
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "NEED KEYWORD AGYRET");

    if (_fareDisplayTrx->getOptions()->isXRSForFRRule() &&
        !_fareDisplayTrx->getOptions()->isKeywordPresent(EPR_ORDFQD))
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "NEED KEYWORD ORGFQD");

    if (!fallback::fallbackAgencyRetailerHandlingFees(_trx))
    { 
      if (_fareDisplayTrx->getOptions()->isPDOForFRRule() &&
          _fareDisplayTrx->getOptions()->isXRSForFRRule())
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                           "UNABLE TO COMBINE ORG AND XRS");

      if (_fareDisplayTrx->getOptions()->isPDRForFRRule() &&
          _fareDisplayTrx->getOptions()->isXRSForFRRule())
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                           "UNABLE TO COMBINE RET AND XRS");

      if (_fareDisplayTrx->getOptions()->isPDOForFRRule() &&
          _fareDisplayTrx->getOptions()->isPDRForFRRule())
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                           "UNABLE TO COMBINE RET AND ORG");
    }
  }
}

void
FareDisplayModelMap::storePassengerInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storePassengerInformation");

  FareDisplayRequest* request = _fareDisplayTrx->getRequest();

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // B70 - Passenger type
      _paxType = xmlValue.c_str();

      if (!_fareDisplayTrx->isERD())
      {
        request->displayPassengerTypes().push_back(_paxType);
        request->inputPassengerTypes().push_back(_paxType);
      }
      break;

    case 2: // Q4P - PaxType Number
      _paxTypeNumber = atoi(xmlValue.c_str());
      break;

    case 3: // B71 - Requested passenger type
      _requestedPaxType = xmlValue.c_str();
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::savePassengerInformation()
{
  LOG4CXX_DEBUG(logger, "In savePassengerInformation");
}

void
FareDisplayModelMap::storeDiagInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeDiagInformation");

  FareDisplayRequest* request = _fareDisplayTrx->getRequest();
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
FareDisplayModelMap::saveDiagInformation()
{
  LOG4CXX_DEBUG(logger, "In saveDiagInformation");
}

void
FareDisplayModelMap::storeRuleCategoryInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeRuleCategoryInformation");

  FareDisplayOptions* options = _fareDisplayTrx->getOptions();

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // Q3W - Rule category number
      options->ruleCategories().push_back(atoi(xmlValue.c_str()));
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::saveRuleCategoryInformation()
{
  LOG4CXX_DEBUG(logger, "In saveRuleCategoryInformation");
}

void
FareDisplayModelMap::storeAlphaCodeInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeAlphaCodeInformation");

  FareDisplayOptions* options = _fareDisplayTrx->getOptions();

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // S59 - Alpha code
    {
      options->alphaCodes().push_back(xmlValue.c_str());
      std::string ac = xmlValue.c_str();
      if (ac.find(RETAILER_CODE) != std::string::npos)
        options->retailerDisplay() = IND_YES;
    }
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::saveAlphaCodeInformation()
{
  LOG4CXX_DEBUG(logger, "In saveAlphaCodeInformation");
}

void
FareDisplayModelMap::storeCombinabilityCodeInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeCombinabilityCodeInformation");

  FareDisplayOptions* options = _fareDisplayTrx->getOptions();

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // S61 - Combinability code
      options->combinabilityCodes().push_back(xmlValue.c_str());
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::saveCombinabilityCodeInformation()
{
  LOG4CXX_DEBUG(logger, "In saveCombinabilityCodeInformation");
}

void
FareDisplayModelMap::storeSecondaryMarketsAndCarrierInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeSecondaryMarketsAndCarrierInformation");

  FareDisplayRequest* request = _fareDisplayTrx->getRequest();

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // A10 - Secondary city 1
      request->secondaryCity1().push_back(verifyLocation(xmlValue.c_str()));
      break;
    case 2: // A11 - Secondary city 2
      request->secondaryCity2().push_back(verifyLocation(xmlValue.c_str()));
      break;
    case 3: // B03 - Secondary governing carrier
      request->secondaryCarrier().push_back(checkAndSwapCarrier(xmlValue.c_str()));
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::saveSecondaryMarketsAndCarrierInformation()
{
  LOG4CXX_DEBUG(logger, "In saveSecondaryMarketsAndInformation");
}

void
FareDisplayModelMap::storeCAT25Information(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeCAT25Information");

  NonPublishedValues* values;

  if (_fareDisplayTrx->isERD() && _erdFareComp)
    values = &(_erdFareComp->cat25Values());
  else
    values = &(_fareDisplayTrx->getOptions()->cat25Values());

  values->isNonPublishedFare() = true;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // S37 - CAT25 vendor code
      values->vendorCode() = xmlValue.c_str();
      break;
    case 2: // Q41 - CAT25 item number
      values->itemNo() = atoi(xmlValue.c_str());
      break;
    case 3: // D12 - CAT25 create date
      values->createDate() = convertDate(xmlValue.c_str());
      break;
    case 4: // S70 - CAT25 directionality
      values->directionality() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::saveCAT25Information()
{
  LOG4CXX_DEBUG(logger, "In saveCAT25Information");
}

void
FareDisplayModelMap::storeCAT35Information(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeCAT35Information");

  NonPublishedValues* values;

  if (_fareDisplayTrx->isERD() && _erdFareComp)
    values = &(_erdFareComp->cat35Values());
  else
    values = &(_fareDisplayTrx->getOptions()->cat35Values());

  values->isNonPublishedFare() = true;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // S37 - CAT35 vendor code
      values->vendorCode() = xmlValue.c_str();
      break;
    case 2: // Q41 - CAT35 item number
      values->itemNo() = atoi(xmlValue.c_str());
      break;
    case 3: // D12 - CAT35 create date
      values->createDate() = convertDate(xmlValue.c_str());
      break;
    case 4: // S70 - CAT35 directionality
      values->directionality() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::saveCAT35Information()
{
  LOG4CXX_DEBUG(logger, "In saveCAT35Information");
}

void
FareDisplayModelMap::storeDFIInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeDFIInformation");

  NonPublishedValues* values;

  if (_fareDisplayTrx->isERD() && _erdFareComp)
    values = &(_erdFareComp->discountedValues());
  else
    values = &(_fareDisplayTrx->getOptions()->discountedValues());

  values->isNonPublishedFare() = true;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // S37 - Discounted vendor code
      values->vendorCode() = xmlValue.c_str();
      break;
    case 2: // Q41 - Discounted item number
      values->itemNo() = atoi(xmlValue.c_str());
      break;
    case 3: // D12 - Discounted create date
      values->createDate() = convertDate(xmlValue.c_str());
      break;
    case 4: // S70 - Discounted directionality
      values->directionality() = xmlValue.c_str();
      break;
    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::saveDFIInformation()
{
  LOG4CXX_DEBUG(logger, "In saveDFIInformation");
}

void
FareDisplayModelMap::storeECNInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeECNInformation");

  // Check if we're processing ERD transaction
  if (!_fareDisplayTrx->isERD() || !_erdFareComp)
    return;

  _erdFareComp->constructedAttributes().isConstructedFare() = true;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // AM0 - gateway 1
      _erdFareComp->constructedAttributes().gateway1() = xmlValue.c_str();
      break;
    case 2: // AN0 - gateway 2
      _erdFareComp->constructedAttributes().gateway2() = xmlValue.c_str();
      break;
    case 3: // N1J - Construction Type
      _erdFareComp->constructedAttributes().constructionType() =
          (ConstructedFareInfo::ConstructionType)(atoi(xmlValue.c_str()));
      break;
    case 4: // C66 - Specified fare Amount
      _erdFareComp->constructedAttributes().specifiedFareAmount() = atof(xmlValue.c_str());
      break;
    case 5: // C6K - Constructed NUC Amount
      _erdFareComp->constructedAttributes().constructedNucAmount() = atof(xmlValue.c_str());
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::storeOAOInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeOAOInformation");

  FareDisplayOptions* options = _fareDisplayTrx->getOptions();
  AddOnAttributes* addonAttrs;

  if (_fareDisplayTrx->isERD() && _erdFareComp)
  {
    addonAttrs = &(_erdFareComp->origAttributes());
  }
  else
  {
    addonAttrs = &(options->origAttributes());
    options->constructedAttributes().isConstructedFare() = true;
  }
  addonAttrs->isAddOn() = true;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // S55 - Origin Footnote 1
      addonAttrs->addonFootNote1() = xmlValue.c_str();
      break;
    case 2: // S64 - Origin Footnote 2
      addonAttrs->addonFootNote2() = xmlValue.c_str();
      break;
    case 3: // BJ0 - Origin fare Class
      addonAttrs->addonFareClass() = xmlValue.c_str();
      break;
    case 4: // Q3W - Origin tariff
      addonAttrs->addonTariff() = atoi(xmlValue.c_str());
      break;
    case 5: // S65 - Origin routing
      addonAttrs->addonRouting() = xmlValue.c_str();
      break;
    case 6: // C50 - Origin Amount
      addonAttrs->addonAmount() = atof(xmlValue.c_str());
      break;
    case 7: // C40 - Origin Currency
      addonAttrs->addonCurrency() = xmlValue.c_str();
      break;
    case 8: // P04 - Origin OW/RT Indicator
      addonAttrs->oWRT() = xmlValue.c_str()[0];
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::saveOAOInformation()
{
  LOG4CXX_DEBUG(logger, "In saveOAOInformation");
}

void
FareDisplayModelMap::storeDAOInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeDAOInformation");

  FareDisplayOptions* options = _fareDisplayTrx->getOptions();
  AddOnAttributes* addonAttrs;

  if (_fareDisplayTrx->isERD() && _erdFareComp)
  {
    addonAttrs = &(_erdFareComp->destAttributes());
  }
  else
  {
    addonAttrs = &(options->destAttributes());
    options->constructedAttributes().isConstructedFare() = true;
  }
  addonAttrs->isAddOn() = true;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // S55 - Origin Footnote 1
      addonAttrs->addonFootNote1() = xmlValue.c_str();
      break;
    case 2: // S64 - Origin Footnote 2
      addonAttrs->addonFootNote2() = xmlValue.c_str();
      break;
    case 3: // BJ0 - Origin fare Class
      addonAttrs->addonFareClass() = xmlValue.c_str();
      break;
    case 4: // Q3W - Origin tariff
      addonAttrs->addonTariff() = atoi(xmlValue.c_str());
      break;
    case 5: // S65 - Origin routing
      addonAttrs->addonRouting() = xmlValue.c_str();
      break;
    case 6: // C50 - Origin Amount
      addonAttrs->addonAmount() = atof(xmlValue.c_str());
      break;
    case 7: // C40 - Origin Currency
      addonAttrs->addonCurrency() = xmlValue.c_str();
      break;
    case 8: // P04 - Origin OW/RT Indicator
      addonAttrs->oWRT() = xmlValue.c_str()[0];
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::saveDAOInformation()
{
  LOG4CXX_DEBUG(logger, "In saveDAOInformation");
}

void
FareDisplayModelMap::storeCALInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeCALInformation");

  // Check if we're processing ERD transaction
  if (!_fareDisplayTrx->isERD())
    return;

  if (_erdFareComp == nullptr)
  {
    _fareDisplayTrx->dataHandle().get(_erdFareComp);

    if (!_erdFareComp)
      return;
  }

  int numAtts = attrs.getLength();

  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // B08 - True Governing Carrier
      _erdFareComp->trueGoverningCarrier() = checkAndSwapCarrier(xmlValue.c_str());
      break;

    case 2: // B02 - Governing Carrier
      _erdFareComp->governingCarrier() = checkAndSwapCarrier(xmlValue.c_str());
      break;

    case 3: // A11 - Departure City
      _erdFareComp->departureCity() = xmlValue.c_str();
      break;

    case 4: // A12 - Arrival City
      _erdFareComp->arrivalCity() = xmlValue.c_str();
      break;

    case 5: // A01 - Departure Airport
      _erdFareComp->departureAirport() = xmlValue.c_str();
      break;

    case 6: // A02 - Arrival Airport
      _erdFareComp->arrivalAirport() = xmlValue.c_str();
      break;

    case 7: // B50 - Fare Basis Code
      _erdFareComp->fareBasis() = xmlValue.c_str();
      break;

    case 8: // C40 - Currency
      _erdFareComp->fareCurrency() = xmlValue.c_str();
      break;

    case 9: // Q4J - Pricing Unit Number
      _erdFareComp->pricingUnitNumber() = atoi(xmlValue.c_str());
      break;

    case 10: // P04 - One Way Fare
      _erdFareComp->oneWayFare() = xmlValue.c_str()[0];
      break;

    case 11: // P05 - Round Trip Fare
      _erdFareComp->roundTripFare() = xmlValue.c_str()[0];
      break;

    case 12: // S70 - Fare Component Directionality
      _erdFareComp->directionality() = xmlValue.c_str();
      break;

    case 13: // AC0 - Account Code
      _erdFareComp->accountCode() = xmlValue.c_str();
      break;

    case 14: // C50 - NUC Fare Amount
      _erdFareComp->nucFareAmount() = atof(xmlValue.c_str());
      break;

    case 15: // A60 - GlobalIndicator
      if (xmlValue.c_str() == FareDisplayModelMap::BAD_GLOBAL ||
          !strToGlobalDirection(_erdFareComp->globalIndicator(), xmlValue.c_str()))
      {
        throw NonFatalErrorResponseException(
            ErrorResponseException::INVALID_GLOBAL_DIRECTION_REQUESTED);
      }
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::saveCALInformation()
{
  LOG4CXX_DEBUG(logger, "In saveCALInformation");

  if (_erdFareComp)
  {
    _erdFareComp->paxTypeCode() = _paxType;
    _erdFareComp->paxTypeNumber() = _paxTypeNumber;
    _erdFareComp->requestedPaxTypeCode() = _requestedPaxType;
    _erdFareComp->actualPaxTypeCode() = _actualPaxType;
    setCAL_FareCompDepartureDate();

    _fareDisplayTrx->getOptions()->erdFareComps().push_back(_erdFareComp);

    _erdFareComp = nullptr;
    _erdFltSeg = nullptr;
  }
}

void
FareDisplayModelMap::storeSEGInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeSEGInformation");

  // Check if we're processing ERD transaction
  if (!_fareDisplayTrx->isERD() || !_erdFareComp)
    return;

  // Create a ERD flight segment record if necessary
  if (_erdFltSeg == nullptr)
  {
    _fareDisplayTrx->dataHandle().get(_erdFltSeg);

    if (!_erdFltSeg)
      return;
  }

  int numAtts = attrs.getLength();

  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // Q0Z - Itin Segment Number
      _erdFltSeg->itinSegNumber() = atoi(xmlValue.c_str());
      break;

    case 2: // S10 - Surface Segment
      _erdFltSeg->surface() = true;
      break;

    case 3: // C6I - Departure Airport
      _erdFltSeg->departureAirport() = xmlValue.c_str();
      break;

    case 4: // A02 - Arrival Airport
      _erdFltSeg->arrivalAirport() = xmlValue.c_str();
      break;

    case 5: // A11 - Departure City
      _erdFltSeg->departureCity() = xmlValue.c_str();
      break;

    case 6: // A12 - Arrival City
      _erdFltSeg->arrivalCity() = xmlValue.c_str();
      break;

    case 7: // S37 - Vendor
      _erdFareComp->vendor() = xmlValue.c_str();
      break;

    case 8: // S89 - Fare Tariff
      _erdFareComp->fareTariff() = atoi(xmlValue.c_str());
      break;

    case 9: // S90 - Rule Number
      _erdFareComp->ruleNumber() = xmlValue.c_str();
      break;

    case 10: // P2F - Fare Break
      _erdFareComp->fareBreak() = xmlValue.c_str()[0];
      break;

    case 11: // C13 - Geo Travel Type
      _erdFltSeg->geoTravelType() = TseUtil::getGeoType(xmlValue.c_str()[0]);
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::saveSEGInformation()
{
  LOG4CXX_DEBUG(logger, "In saveSEGInformation");

  if (_erdFltSeg && _erdFareComp)
  {
    // Add only air segments
    if (!_erdFltSeg->surface())
    {
      _erdFltSeg->fareComponent() = _erdFareComp;
      _erdFareComp->segments().push_back(_erdFltSeg);
    }
    _erdFltSeg = nullptr;
  }
}

void
FareDisplayModelMap::storeSUMInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeSUMInformation");

  // Check if we're processing ERD transaction
  if (!_fareDisplayTrx->isERD())
    return;

  FareDisplayRequest* request = _fareDisplayTrx->getRequest();

  unsigned int numAtts = attrs.getLength();

  for (unsigned int i = 0; i < numAtts; ++i)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // AO0 - AAA override
      request->salePointOverride() = request->ticketPointOverride() =
          verifyLocation(xmlValue.c_str());
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::storeHPUInformation(const xercesc::Attributes& attrs)
{
  if (fallback::fallbackWPRDASLFix(_fareDisplayTrx))
    return;

  if (!_fareDisplayTrx->isERD() || !_erdFareComp)
    return;

  int numAtts = attrs.getLength();

  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // A52 - SourcePCC
      char tmpBuf[10];
      strcpy(tmpBuf, xmlValue.c_str());
      if (tmpBuf[3] == ' ')
        tmpBuf[3] = '\0';
      _hpuSection.sourcePCC = tmpBuf;
      break;

    case 2: // C51 - Adjusted fare amount
      _hpuSection.totalFareAmount = atof(xmlValue.c_str());
      break;

    case 3: // APP - Adjustment type
      _hpuSection.markupType = xmlValue.c_str();
      break;
    }
  }
}

void
FareDisplayModelMap::storeERDInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeERDInformation");

  // Check if we're processing ERD transaction
  if (!_fareDisplayTrx->isERD() || !_erdFareComp)
    return;

  int numAtts = attrs.getLength();

  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1: // Q46 - Link Number
      _erdFareComp->linkNumber() = atoi(xmlValue.c_str());
      break;

    case 2: // Q1K - Sequence Number
      _erdFareComp->sequenceNumber() = atoi(xmlValue.c_str());
      break;

    case 3: // D12 - Create Date
      _erdFareComp->createDate() = convertDate(xmlValue.c_str());
      break;

    case 4: // D55 - Create Time
    {
      std::string crTime = xmlValue.c_str();

      if (!FareDisplayUtil::updateCreateTimeFormat(crTime, false))
      {
        LOG4CXX_ERROR(logger, "Unable to convert time: " << xmlValue.c_str());
      }
      else
        _erdFareComp->createTime() = crTime;
    }
    break;

    case 5: // S53 - Fare Type
      _erdFareComp->fareType() = xmlValue.c_str();
      break;

    case 6: // BJ0 - Fare Class
      _erdFareComp->fareClass() = xmlValue.c_str();
      break;

    case 7: // N1P - CAT35 type
      _erdFareComp->cat35Type() = xmlValue.c_str();
      break;

    case 8: // C5A - Fare Amount
      _erdFareComp->fareAmount() = atof(xmlValue.c_str());
      break;

    case 9: // P72 - Booking Code
      _erdFareComp->bookingCode() = xmlValue.c_str();
      break;

    case 10: // B50 - Fare Basis Code
      _erdFareComp->uniqueFareBasis() = _erdFareComp->fareBasis();
      _erdFareComp->fareBasis() = xmlValue.c_str();
      break;

    case 11: // BE0 - Ticket Designator
      _erdFareComp->ticketDesignator() = xmlValue.c_str();
      break;

    case 12: // AC0 - Account Code
      if (_erdFareComp->accountCode().empty())
      {
        _erdFareComp->accountCode() = xmlValue.c_str();
      }
      break;

    case 13: // D08 - Departure Date
      _erdFareComp->departureDT() = convertDate(xmlValue.c_str());
      break;

    case 14: // B00 - Validating Carrier
      _erdFareComp->validatingCarrier() = checkAndSwapCarrier(xmlValue.c_str());
      break;

    case 15: // B70 - Actual Pax Type
      _actualPaxType = xmlValue.c_str();
      break;

    case 16: // Q04 - Fare Class Length
      _erdFareComp->fareClassLength() = atoi(xmlValue.c_str());
      break;

    case 17: // CP0 - Command Pricing
      _erdFareComp->wasCommandPricing() = (xmlValue.c_str()[0] == TRUE_INDICATOR);
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::saveERDInformation()
{
  LOG4CXX_DEBUG(logger, "In saveERDInformation");

  // Check if we're processing ERD transaction
  if (!_fareDisplayTrx->isERD() || !_erdFareComp)
    return;

  if (_erdFareComp->cat35Type() == NET_QUALIFIER && _erdFareComp->uniqueFareBasis().length())
  {
    _erdFareComp->cat35FareBasis() = _erdFareComp->uniqueFareBasis();
    _erdFareComp->uniqueFareBasis() = "";
  }
}

void
FareDisplayModelMap::saveHPUInformation()
{
  if (fallback::fallbackWPRDASLFix(_fareDisplayTrx))
    return;

  if (!_fareDisplayTrx->isERD() || !_erdFareComp)
    return;

  _erdFareComp->hpuInfo().push_back(_hpuSection);
}


void
FareDisplayModelMap::storeDTSInformation(const xercesc::Attributes& attrs)
{
  LOG4CXX_DEBUG(logger, "In storeDTSInformation");

  // Check if we're processing ERD transaction
  if (!_fareDisplayTrx->isERD())
    return;

  _inDts = true;

  int numAtts = attrs.getLength();

  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    switch (((Mapping*)_currentMapEntry)->members[SDBMHash(xmlStr.c_str())])
    {
    case 1:
      _fareDisplayTrx->getOptions()->lineNumber() = atoi(xmlValue.c_str()); // Q0S - WPALastOption
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }
}

void
FareDisplayModelMap::saveDTSInformation()
{
  LOG4CXX_DEBUG(logger, "In saveDTSInformation");
  _inDts = false;
}

void
FareDisplayModelMap::addPreferredCarrier(const CarrierCode& cxr)
{
  if (_isMcpPartition && cxr.equalToConst("**"))
    return;
  std::set<CarrierCode>::iterator i(_fareDisplayTrx->preferredCarriers().end()),
      start(_fareDisplayTrx->preferredCarriers().begin()),
      end(_fareDisplayTrx->preferredCarriers().end());
  i = find(start, end, cxr);
  if (i == end)
  {
    _fareDisplayTrx->preferredCarriers().insert(cxr);
  }
  else
  {
    LOG4CXX_WARN(logger, "DUPLICATE CARRRIER FOR THE SAME REQUEST");
  }
}

void
FareDisplayModelMap::setMultiTransportCity()
{
  // Assign offMultiCity and boardMultiCity
  if (_airSeg->origin() != nullptr && _airSeg->destination() != nullptr)
  {
    if (_airSeg->carrier().empty() && _fareDisplayTrx->preferredCarriers().size() == 1)
    {
      _airSeg->carrier() = *_fareDisplayTrx->preferredCarriers().begin();
    }

    GeoTravelType geoType =
        LocUtil::isInternational(*(_airSeg->origin()), *(_airSeg->destination())) == true
            ? GeoTravelType::International
            : GeoTravelType::Domestic;
    _airSeg->boardMultiCity() = FareMarketUtil::getMultiCity(
        _airSeg->carrier(), _airSeg->origAirport(), geoType, _airSeg->departureDT());

    _airSeg->offMultiCity() = FareMarketUtil::getMultiCity(
        _airSeg->carrier(), _airSeg->destAirport(), geoType, _airSeg->departureDT());
  }
}

bool
FareDisplayModelMap::isValidPaxType(const std::vector<PaxTypeCode>& paxTypeCodes,
                                    DataHandle& dataHandle)
{
  // Get all valid pax types from PsgType table
  const std::vector<PaxTypeInfo*>& validPaxTypes = dataHandle.getAllPaxType();

  // Validate pax type codes input
  std::vector<PaxTypeCode>::const_iterator ptcIter = paxTypeCodes.begin();
  std::vector<PaxTypeCode>::const_iterator ptcIterEnd = paxTypeCodes.end();

  for (; ptcIter != ptcIterEnd; ptcIter++)
  {
    bool validPaxType = false;

    // Loop through all valid pax types to find a match
    std::vector<PaxTypeInfo*>::const_iterator vptIter = validPaxTypes.begin();
    std::vector<PaxTypeInfo*>::const_iterator vptIterEnd = validPaxTypes.end();

    for (; vptIter != vptIterEnd; vptIter++)
    {
      if (*ptcIter == (*vptIter)->paxType())
      {
        validPaxType = true;
        break;
      }
    }

    if (!validPaxType)
    {
      return false;
    }
  }

  return true;
}

//---------------------------------------------------------
//      setCAL_FareCompDepartureDate
//
//   set the erdFareComp departureDt to the date of the
//   first leg of the Pricing Unit.
//---------------------------------------------------------
void
FareDisplayModelMap::setCAL_FareCompDepartureDate()
{
  std::vector<ERDFareComp*>::const_iterator fcIter =
      _fareDisplayTrx->getOptions()->erdFareComps().begin();
  std::vector<ERDFareComp*>::const_iterator fcIterEnd =
      _fareDisplayTrx->getOptions()->erdFareComps().end();

  for (; fcIter != fcIterEnd; ++fcIter)
  {
    ERDFareComp* existingFC = *fcIter;

    if (!existingFC)
      continue;

    if (_erdFareComp->pricingUnitNumber() == existingFC->pricingUnitNumber())
    {
      _erdFareComp->departureDT() = existingFC->departureDT();
      return;
    }
  }

  // If it falls thru the loop -- the erdFareComp departureDt is the date retrieved
  // from XML which is the date of first travel Seg of that fare.
}

void
FareDisplayModelMap::parseSegmentNumbers(const std::string& string, std::vector<uint16_t>& result)
{
  boost::char_separator<char> separator("/");
  tokenizer tokens(string, separator);

  for (tokenizer::iterator tkIter = tokens.begin(); tkIter != tokens.end(); ++tkIter)
  {
    std::string token = *tkIter;

    try
    {
      uint16_t number = boost::lexical_cast<uint16_t>(token);

      result.push_back(number);
    }
    catch (boost::bad_lexical_cast&)
    {
      // Do nothing here
    }
  }
}

CarrierCode
FareDisplayModelMap::checkAndSwapCarrier(const CarrierCode& carrier)
{
  _trx->mcpCarrierSwap() |= MCPCarrierUtil::isPseudoCarrier(carrier);
  // LATAM MCP-S
  if(!fallback::neutralToActualCarrierMapping(_trx) &&
      MCPCarrierUtil::isNeutralCarrier(carrier))
  {
    return MCPCarrierUtil::swapFromNeutralToActual(carrier);
  }
  else
  {
    return MCPCarrierUtil::swapToActual(_trx, carrier);
  }
}

void
FareDisplayModelMap::storeDynamicConfigOverride(const xercesc::Attributes& attrs)
{
  handleDynamicConfig(((Mapping*)_currentMapEntry)->members, attrs);
}

void
FareDisplayModelMap::parseRetailerCode(const XMLChString& xmlValue,
                                          FareDisplayRequest* request, uint8_t& rcqCount)
{
  if (!fallback::fallbackFRRProcessingRetailerCode(_trx) && request != nullptr)
    rcqCount = request->setRCQValues(xmlValue.c_str());
}

void
FareDisplayModelMap::checkParsingRetailerCodeError(bool isPRM, uint8_t rcqCount)
{
  if (!fallback::fallbackFRRProcessingRetailerCode(_trx))
  {
    if (isPRM && (rcqCount > 1))
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "MAXIMUM 1 RETAILER RULE QUALIFIER PERMITTED");

    if (!fallback::fallbackFixProcessingRetailerCodeXRS(_trx)
        && (rcqCount > 0)
        && _fareDisplayTrx->getOptions()->isXRSForFRRule())
    {
      std::string sMsg = "UNABLE TO COMBINE ";
      if (isPRM)
        sMsg+= "RRM*";
      else
        sMsg+= "RR*";

      sMsg += " WITH XRS";
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, sMsg.c_str());
    }
  }
}

} // tse namespace
