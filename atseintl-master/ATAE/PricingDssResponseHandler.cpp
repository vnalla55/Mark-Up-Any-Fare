//----------------------------------------------------------------------------
//
//  File   :  PricingDssResponseHandler.cpp
//
//  Author :  Kul Shekhar
//
//  Copyright Sabre 2005
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s) have
//          been supplied.
//
//-----------------------------------------------------------------------

#include "ATAE/PricingDssResponseHandler.h"
#include "ATAE/PricingDssFlightKey.h"

#include "Common/ClassOfService.h"
#include "Common/HiddenStopDetails.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/RBDByCabinUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "Common/XMLChString.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/Loc.h"
#include "Util/IteratorRange.h"

#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>

#include <sstream>
#include <boost/lexical_cast.hpp>

namespace tse
{

static Logger
logger("atseintl.Xform.PricingDssResponseHandler");

bool
PricingDssResponseHandler::initialize()
{
  try
  {
    xercesc::XMLPlatformUtils::Initialize();
  }
  catch (const xercesc::XMLException& e) { return false; }
  return true;
}

void
PricingDssResponseHandler::startElement(const XMLCh* const,
                                        const XMLCh* const,
                                        const XMLCh* const qname,
                                        const xercesc::Attributes& attrs)
{
  char* chTagName = xercesc::XMLString::transcode(qname);
  std::string tagName(chTagName);
  delete[] chTagName;
  std::transform(tagName.begin(), tagName.end(), tagName.begin(), (int (*)(int))toupper);

  if (tagName == "DSS")
  {
    processDSSAtts(attrs);
  }
  else if (tagName == "FLL")
  {
    processFLLAtts(attrs);
  }
  else if (tagName == "ASG")
  {
    processASGAtts(attrs);
  }
  else if (tagName == "HSG")
  {
    processHSGAtts(attrs);
  }
  else if (tagName == "PTM")
  {
    processPTMAtts(attrs);
  }
  else if (tagName == "MER")
  {
    processMERAtts(attrs);
  }
  else
  {
    LOG4CXX_INFO(logger, "Unknown tag: " + tagName);
  }
}

void
PricingDssResponseHandler::processASGAtts(const xercesc::Attributes& attrs)
{
  LOG4CXX_INFO(logger, "In processASGAtts");
  _dssFlights.emplace_back();

  PricingDssFlight& currentFlight = _dssFlights.back();
  int numAtts = attrs.getLength();

  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    if (xmlStr == "BBR") // BBR carrier
    {
      currentFlight._bbrCarrier = TseUtil::boolValue(xmlValue.c_str());
    }
    else if (xmlStr == "ORG") // origin
    {
      currentFlight._origAirport = xmlValue.c_str();
    }
    else if (xmlStr == "DST") // destination
    {
      currentFlight._destAirport = xmlValue.c_str();
    }
    else if (xmlStr == "MXC") // marketing carrier code
    {
      currentFlight._marketingCarrierCode = MCPCarrierUtil::swapToActual(&_trx, xmlValue.c_str());
    }
    else if (xmlStr == "FLT") // marketing flight number
    {
      currentFlight._marketingFlightNumber = atoi(xmlValue.c_str());
    }
    else if (xmlStr == "CXC") // offered class of services
    {
      parseBookingCodes(currentFlight._offeredBookingCodes, xmlValue.c_str());
    }
    else if (xmlStr == "OCX") // operating carrier
    {
      currentFlight._operatingCarrierCode = MCPCarrierUtil::swapToActual(&_trx, xmlValue.c_str());
    }
    else if (xmlStr == "OFN") // operating flight number
    {
      currentFlight._operatingFlightNumber = atoi(xmlValue.c_str());
    }
    else if (xmlStr == "EQP") // equipemnt code
    {
      currentFlight._equipmentCode = xmlValue.c_str();
    }
    else if (xmlStr == "LOF") // Hidden stops or also called Line of Flight
    {
      parseHiddenStops(currentFlight._hiddenStops, xmlValue.c_str());
    }
    else if (xmlStr == "DD2") // Arrival date adjust
    {
      currentFlight._arrivalDayAdjust = boost::lexical_cast<int32_t>(xmlValue.c_str());
    }
    else if (xmlStr == "DSA") // Arrival local time
    {
      currentFlight._localArrivalTime = xmlValue.c_str();
    }
    else if (xmlStr == "EQ1")
    {
      currentFlight._equipTypeFirstLeg = xmlValue.c_str();
    }
    else if (xmlStr == "DQ1")
    {
      currentFlight._equipTypeLastLeg = xmlValue.c_str();
    }
  } // end for(int i = 0; i < numAtts; i++)
}

void
PricingDssResponseHandler::processHSGAtts(const xercesc::Attributes& attrs)
{
  LOG4CXX_INFO(logger, "In processHSGAtts");
  HiddenStopDetails hiddenStop;

  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    if (xmlStr == "A00")
    {
      hiddenStop.airport() = xmlValue.c_str();
    }
    else if (xmlStr == "B40")
    {
      hiddenStop.equipment() = xmlValue.c_str();
    }
    else if (xmlStr == "D03")   //departureDate
    {
      DateTime newDate = DateTime::convertDate(xmlValue.c_str());

      hiddenStop.departureDate() = DateTime(
        newDate.date(), hiddenStop.departureDate().time_of_day());
    }
    else if (xmlStr == "D04")   //arrivalDate
    {
      DateTime newDate = DateTime::convertDate(xmlValue.c_str());

      hiddenStop.arrivalDate() = DateTime(
        newDate.date(), hiddenStop.arrivalDate().time_of_day());
    }
    else if (xmlStr == "D31")   //departureTime
    {
      std::stringstream stream(xmlValue.c_str());
      unsigned int hours, minutes;
      stream >> hours;
      stream.ignore();
      stream >> minutes;

      hiddenStop.departureDate() = DateTime(
        hiddenStop.departureDate().date(), hours, minutes, 0);
    }
    else if (xmlStr == "D32")   //arrivalTime
    {
      std::stringstream stream(xmlValue.c_str());
      unsigned int hours, minutes;
      stream >> hours;
      stream.ignore();
      stream >> minutes;

      hiddenStop.arrivalDate() = DateTime(hiddenStop.arrivalDate().date(), hours, minutes, 0);
    }
  }
  PricingDssFlight& currentFlight = _dssFlights.back();
  currentFlight._hiddenStopsDetails.push_back(hiddenStop);
}

void
PricingDssResponseHandler::parseBookingCodes(std::vector<BookingCode>& dssFlightOfferedBookingCodes,
                                             const std::string& offeredBookingCodes) const
{
  size_t cosIndex = 0;
  size_t cosSize = offeredBookingCodes.size();
  BookingCode bc;
  for (; cosIndex < cosSize;)
  {
    bc.clear();
    bc.push_back(offeredBookingCodes[cosIndex]);

    if (cosIndex + 1 < cosSize && offeredBookingCodes[cosIndex + 1] == 'N')
    {
      // night booking code
      bc.push_back('N');
    }
    dssFlightOfferedBookingCodes.push_back(bc);
    cosIndex += 2;
  }
}

void
PricingDssResponseHandler::parseHiddenStops(std::vector<LocCode>& dssFlightHiddenStops,
                                            const std::string& hiddenStops) const
{
  boost::tokenizer<boost::char_separator<char>>
    tokens(hiddenStops, boost::char_separator<char>(" "));
  for(const std::string& token : tokens)
  {
    dssFlightHiddenStops.emplace_back(token);
  }
}

void
PricingDssResponseHandler::processMERAtts(const xercesc::Attributes& attrs)
{
  LOG4CXX_INFO(logger, "In processMERAtts");
  std::string msg ="";
  int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    if (xmlStr == "CXR")
    {
      msg = msg + xmlValue.c_str();
    }
    else if (xmlStr == "FLN")
    {
      msg = msg + xmlValue.c_str();
    }
  }
  if ( _trx.isValidatingCxrGsaApplicable() && !TrxUtil::isHistorical(_trx) && _isFirstSegmentUnflown)
    throw msg;   // catch is in ContentServices
}

bool
PricingDssResponseHandler::parse(const char* content)
{
  bool retCode = true;
  xercesc::MemBufInputSource* mbis = new xercesc::MemBufInputSource(
      (const unsigned char*)content, static_cast<unsigned int>(strlen(content)), ""); // lint !e668
  _reader = xercesc::XMLReaderFactory::createXMLReader();

  // Flag settings
  _reader->setFeature(xercesc::XMLUni::fgSAX2CoreValidation, false);
  _reader->setFeature(xercesc::XMLUni::fgSAX2CoreNameSpaces, true);

  // Set document handlers
  _reader->setContentHandler(this);
  _reader->setErrorHandler(this);

  try { _reader->parse(*mbis); }
  catch (const xercesc::XMLException& xmle)
  {
    char* msgTxt = xercesc::XMLString::transcode(xmle.getMessage());
    LOG4CXX_ERROR(logger, "XMLException: " << msgTxt);
    delete[] msgTxt;
    retCode = false;
  }
  catch (xercesc::SAXParseException& spe)
  {
    char* msgTxt = xercesc::XMLString::transcode(spe.getMessage());
    LOG4CXX_ERROR(logger, "SAXParseException: " << msgTxt);
    delete[] msgTxt;
    retCode = false;
  }
  if(mbis != nullptr)
    delete mbis;
  if(_reader != nullptr)
    delete _reader;
  _reader = nullptr;
  return retCode;
}
}
