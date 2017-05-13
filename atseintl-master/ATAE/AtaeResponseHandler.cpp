//----------------------------------------------------------------------------
//
//  File   :  AtaeResponseHandler.cpp
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

#include "ATAE/AtaeResponseHandler.h"

#include "Common/ClassOfService.h"
#include "Common/Logger.h"
#include "Common/XMLChString.h"
#include "Common/TrxUtil.h"
#include "Common/RBDByCabinUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/Cabin.h"

#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>

#include <string>

namespace tse
{
static Logger
logger("atseintl.Xform.AtaeResponseHandler");

bool
AtaeResponseHandler::initialize()
{
  try
  {
    xercesc::XMLPlatformUtils::Initialize();
  }
  catch (const xercesc::XMLException& e)
  {
    return false;
  }
  return true;
}

void
AtaeResponseHandler::startElement(const XMLCh* const,
                                  const XMLCh* const,
                                  const XMLCh* const qname,
                                  const xercesc::Attributes& attrs)
{
  char* chTagName = xercesc::XMLString::transcode(qname);
  std::string tagName(chTagName);
  delete[] chTagName;
  std::transform(tagName.begin(), tagName.end(), tagName.begin(), (int (*)(int))toupper);

  if (tagName == "ATS")
    processATSAtts(attrs);
  else if (tagName == "ASL")
    processASLAtts(attrs);
  else if (tagName == "DIA")
    processDIAAtts(attrs);
  else if (tagName == "ASO")
    processASOAtts(attrs);
  else if (tagName == "SGS")
    processSGSAtts(attrs);
  else
    LOG4CXX_INFO(logger, "Unknown tag: " + tagName);
}

void
AtaeResponseHandler::processATSAtts(const xercesc::Attributes& attrs)
{
  LOG4CXX_INFO(logger, "In processATSAtts");
  const int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));
    if (xmlStr == "S1I") // return code
    {
      if (xmlValue == "ERR")
      {
        LOG4CXX_FATAL(logger, "RESPONSE RETURNED FROM ATAE HAS ERROR  S1I ERR");
      }
    }
  }
}

void
AtaeResponseHandler::processSGSAtts(const xercesc::Attributes& attrs)
{
  LOG4CXX_INFO(logger, "In processSGSAtts");
  FareMarket* fm = getFareMarket();
  if (fm == nullptr)
    return;
  arunkCos(fm);

  // get the current Travel Seg
  const size_t indexOfFlightToBeProcessed = fm->classOfServiceVec().size();
  if (indexOfFlightToBeProcessed >= fm->travelSeg().size())
  {
    // safety check
    LOG4CXX_FATAL(logger, "TRAVEL SEG SIZE IS BAD");
    return;
  }
  TravelSeg* tvlSeg = fm->travelSeg()[indexOfFlightToBeProcessed];

  std::vector<ClassOfService*>* cosVec = nullptr;
  DataHandle& dataHandle = _trx.dataHandle();
  dataHandle.get(cosVec);
  fm->classOfServiceVec().push_back(cosVec);
  if (cosVec == nullptr)
  {
    LOG4CXX_FATAL(logger, "ERROR IN DATAHANDLE get(cosVec)");
    return;
  }

  ClassOfService* cs = nullptr;
  int iLocalCos = 0;
  int nLocalCos = 0;

  std::vector<int> numSeats;
  std::string cosString;
  std::string numberOfSeats;
  int numSeatsInteger = 0;
  uint32_t nCos = 0;
  BookingCode bc;
  bool bcFound = false;
  std::vector<BookingCode> bookingCodes;

  const int numAtts = attrs.getLength();
  for (int i = 0; i < numAtts; i++)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    if (xmlStr == "S1F") // number Of seats
    {
      cosString = xmlValue.c_str();
      nCos = cosString.size();
      for (uint32_t iCos = 0; iCos < nCos; ++iCos)
      {
        if (isdigit(cosString[iCos]))
        {
          numberOfSeats.push_back(cosString[iCos]);
        }

        if (isspace(cosString[iCos]) || iCos == nCos - 1)
        {
          numSeatsInteger = atoi(numberOfSeats.c_str());
          numSeats.push_back(numSeatsInteger);
          numSeatsInteger = 0;
          numberOfSeats.clear();
        }
      }
    }
    else if (xmlStr == "S1G") // booking codes
    {
      cosString = xmlValue.c_str();
      nCos = cosString.size();
      for (uint32_t iCos = 0; iCos < nCos; ++iCos)
      {
        bc.clear();
        if (isalpha(cosString[iCos]))
        {
          bc.push_back(cosString[iCos]);
          bookingCodes.push_back(bc);
        }
      }

      if(TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(_trx))
      {
        // fill cosVec of the faremarket
        bool localFlag = false;
        if (fm->travelSeg().size() == 1)
        {
          localFlag = true;
          tvlSeg->classOfService().clear();
        }
        getCos(tvlSeg, bookingCodes, cosVec, numSeats, localFlag);
      }
      else
      {

        // fill cosVec of the faremarket

        // ideally the size of numSeats and bookingCodes MUST be
        // same but just in case....
        nCos = numSeats.size();
        if (nCos > bookingCodes.size())
          nCos = bookingCodes.size();

        for (uint32_t iCos = 0; iCos < nCos; ++iCos)
        {
          cs = getCos(bookingCodes[iCos], tvlSeg);
          if (cs != nullptr)
          {
            cs->numSeats() = numSeats[iCos];
            cosVec->push_back(cs);
          }
        }

        // if its a local market (only 1 flight)
        // then make sure we put the booking codes in airSeg also
        if (fm->travelSeg().size() == 1)
        {
          // nLocalCos = tvlSeg->classOfService().size();
          bcFound = false;
          if (tvlSeg->classOfService().size() > 1)
          {
            // do not clear if there is only 1 or 0 booking codes present
            // Reason is that in case AS V2 returns emtpy booking code then
            // we know that we put the booked bookin code in this vector
            // before calling AS V2.
            tvlSeg->classOfService().clear();
          }
          nLocalCos = tvlSeg->classOfService().size();
          for (uint32_t iCos = 0; iCos < nCos; ++iCos)
          {
            bcFound = false;
            for (iLocalCos = 0; iLocalCos < nLocalCos; iLocalCos++)
            {
              ClassOfService& localCos = *(tvlSeg->classOfService()[iLocalCos]);
              if (localCos.bookingCode() == bookingCodes[iCos])
              {
                localCos.numSeats() = numSeats[iCos];
                bcFound = true;
                break;
              }
            }
            if (!bcFound)
            {
              cs = getCos(bookingCodes[iCos], tvlSeg);
              if (cs != nullptr)
              {
                cs->numSeats() = numSeats[iCos];
                tvlSeg->classOfService().push_back(cs);
              }
            }
          }
        }
      }
    }
  }
}

bool
AtaeResponseHandler::parse(const char* content)
{
  bool retCode = true;

  if (content == nullptr)
    return false;

  xercesc::MemBufInputSource* mbis =
      new xercesc::MemBufInputSource((const unsigned char*)content, strlen(content), "");
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
  if (mbis)
    delete mbis;
  if (_reader)
  {
    delete _reader;
    _reader = nullptr;
  }

  return retCode;
}

ClassOfService*
AtaeResponseHandler::getCos(const BookingCode& bookingCode, const TravelSeg* travelSeg)
{
  DataHandle& dataHandle = _trx.dataHandle();

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);
  ClassOfService* cs = nullptr;
  dataHandle.get(cs);
  if (cs == nullptr)
    return nullptr;
  const Cabin* aCabin = nullptr;
  aCabin = dataHandle.getCabin(airSeg->carrier(), bookingCode, airSeg->departureDT());
  cs->bookingCode() = bookingCode;

  if (aCabin == nullptr)
  {
    LOG4CXX_ERROR(logger,
                  "AtaeResponseHandler::getCos() CABIN TABLE ERROR FLIGHT:"
                      << airSeg->carrier() << airSeg->flightNumber() << " " << bookingCode
                      << airSeg->departureDT().dateToString(DDMMMYYYY, ""));
    return nullptr;
  }
  cs->cabin() = aCabin->cabin();
  return cs;
}

//----------------------------------------------------------------------------
// AtaeResponseHandler::getCos()   New for ATPCO RBD
//----------------------------------------------------------------------------
void
AtaeResponseHandler::getCos(TravelSeg* tvlSeg,
                            std::vector<BookingCode> bookingCodes,
                            std::vector<ClassOfService*>* cosVec,
                            std::vector<int> numSeats,
                            bool localFlag)
{
  AirSeg* airSeg = dynamic_cast<AirSeg*>(tvlSeg);
  RBDByCabinUtil rbdCabin(_trx, AVAIL_RSP);
  rbdCabin.getCabinsByRbd(*airSeg, bookingCodes, cosVec);
  for (uint16_t j = 0; j < cosVec->size(); ++j)
  {
    ((*cosVec)[j])->numSeats() = numSeats[j];
  }
  if (localFlag)
  {
    tvlSeg->classOfService() = *cosVec;
  }
}

FareMarket*
AtaeResponseHandler::getFareMarket()
{
  int iFm = 0;
  const int nFm = _fareMarketsSentToAtae.size();
  uint32_t numTvlsegs = 0;
  FareMarket* fm = nullptr;

  for (; iFm < nFm; iFm++)
  {
    fm = _fareMarketsSentToAtae[iFm];
    numTvlsegs = fm->travelSeg().size();
    if (fm->classOfServiceVec().size() != numTvlsegs)
      return fm;
  }
  return nullptr;
}

void
AtaeResponseHandler::arunkCos(FareMarket* fm)
{
  uint32_t indexOfFlightToBeProcessed = 0;
  indexOfFlightToBeProcessed = fm->classOfServiceVec().size();

  if (indexOfFlightToBeProcessed >= fm->travelSeg().size())
  {
    // safety check
    return;
  }
  const TravelSeg* tvlSeg = fm->travelSeg()[indexOfFlightToBeProcessed];
  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(tvlSeg);

  if (airSeg == nullptr || tvlSeg->segmentType() == Arunk)
  {
    std::vector<ClassOfService*>* cosVec = nullptr;
    DataHandle& dataHandle = _trx.dataHandle();
    dataHandle.get(cosVec);
    fm->classOfServiceVec().push_back(cosVec);
  }
}
} // tse
