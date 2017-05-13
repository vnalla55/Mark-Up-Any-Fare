//----------------------------------------------------------------------------
//
//  File   :  AtaeResponseHandler.h
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
//---------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"

#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>

#include <vector>

namespace tse
{
class ClassOfService;
class FareMarket;
class PricingTrx;
class TravelSeg;

class AtaeResponseHandler : public xercesc::DefaultHandler
{
public:
  AtaeResponseHandler(PricingTrx& trx) : _trx(trx) {}

  bool initialize();

  bool parse(const char* content);

  std::vector<FareMarket*>& fareMarketsSentToAtae() { return _fareMarketsSentToAtae; }
  const std::vector<FareMarket*>& fareMarketsSentToAtae() const { return _fareMarketsSentToAtae; }

protected:
  xercesc::SAX2XMLReader* _reader = nullptr;
  PricingTrx& _trx;
  std::vector<FareMarket*> _fareMarketsSentToAtae;

  void startElement(const XMLCh* const uri,
                    const XMLCh* const localname,
                    const XMLCh* const qname,
                    const xercesc::Attributes& attrs) override;

  void processATSAtts(const xercesc::Attributes& attrs);
  void processASLAtts(const xercesc::Attributes& attrs) {}
  void processDIAAtts(const xercesc::Attributes& attrs) {}
  void processASOAtts(const xercesc::Attributes& attrs) {}
  void processSGSAtts(const xercesc::Attributes& attrs);

  void
  endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) const
  {
  }

  void characters(const XMLCh* const chars, const unsigned int length) const {}

private:
  ClassOfService* getCos(const BookingCode& bookingCode, const TravelSeg* travelSeg);
  void getCos(TravelSeg* tvlSeg,
              std::vector<BookingCode> bookingCodes,
              std::vector<ClassOfService*>* cosVec,
              std::vector<int> numSeats,
              bool loaclFlag);

  FareMarket* getFareMarket();

  void arunkCos(FareMarket* fm);
};
} // end namespace tse
