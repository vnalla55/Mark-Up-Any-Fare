//----------------------------------------------------------------------------
//
//  File   :  PricingDssResponseHandler.h
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

#include "ATAE/PricingDssFlightKey.h"
#include "Common/TseCodeTypes.h"

#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/util/XercesDefs.hpp>

namespace tse
{
class PricingTrx;

class PricingDssResponseHandler : public xercesc::DefaultHandler
{
protected:
  friend class PricingDssResponseHandlerTest;

public:
  PricingDssResponseHandler(const PricingTrx& trx, bool isFirstSegmentUnflown)
    : _trx(trx), _isFirstSegmentUnflown(isFirstSegmentUnflown)
  {
  }

  bool initialize();

  bool parse(const char* content);

  const std::size_t numFltRtnFromDSS() const { return _dssFlights.size(); }

  std::vector<PricingDssFlight> _dssFlights;

protected:
  xercesc::SAX2XMLReader* _reader = nullptr;
  const PricingTrx& _trx;

  void startElement(const XMLCh* const uri,
                    const XMLCh* const localname,
                    const XMLCh* const qname,
                    const xercesc::Attributes& attrs) override;

  void processDSSAtts(const xercesc::Attributes& attrs) {}

  void processFLLAtts(const xercesc::Attributes& attrs) {}

  void processASGAtts(const xercesc::Attributes& attrs);
  void processHSGAtts(const xercesc::Attributes& attrs);

  void processPTMAtts(const xercesc::Attributes& attrs) {}

  void processMERAtts(const xercesc::Attributes& attrs);

  void endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname)
      override
  {
  }

  void characters(const XMLCh* const chars, const unsigned int length) override {}

  void parseBookingCodes(std::vector<BookingCode>& dssFlightOfferedBookingCodes,
                         const std::string& offeredBookingCodes) const;
  void parseHiddenStops(std::vector<LocCode>& dssFlightHiddenStops,
                        const std::string& hiddenStops) const;

private:
  const bool _isFirstSegmentUnflown;
};
} // end namespace tse
