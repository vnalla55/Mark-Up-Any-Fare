//-------------------------------------------------------------------
//
//  File:        DecodeContentHandler.h
//  Created:     September 5, 2014
//  Authors:     Roland Kwolek
//
//  Description: Decode request XML parser
//
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Billing.h"

#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>

namespace tse
{
class Trx;
class DataHandle;
class DecodeTrx;

class DecodeContentHandler : public xercesc::DefaultHandler
{
public:
  DecodeContentHandler() : DefaultHandler(), _location(), _reader(nullptr) {}

  virtual ~DecodeContentHandler() {}

  bool parse(const char* content);

  bool setTrx(DecodeTrx& dch);

  bool setTrxType(DataHandle& dataHandle, Trx*& trx) const;

  const LocCode& getLocation() const { return _location; }

  const Billing& getBilling() const { return _billing; }

private:
  void startElement(const XMLCh* const uri,
                    const XMLCh* const localname,
                    const XMLCh* const qname,
                    const xercesc::Attributes& attrs) override;

  void transcodeToStrUpper(const XMLCh* const xch, std::string& str);

  bool getAttrValue(const xercesc::Attributes& attrs, const char* const attr, std::string& value);

  void handleOpt(const xercesc::Attributes& attrs);

  void handleBil(const xercesc::Attributes& attrs);

  bool isDataValid() const { return !_location.empty(); }
  LocCode _location; // RTG

  xercesc::SAX2XMLReader* _reader;

  Billing _billing;

  static Logger _logger;
};
}

