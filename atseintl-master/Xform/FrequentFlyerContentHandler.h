//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"

#include <string>
#include <vector>

#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>

namespace tse
{
class Trx;
class DataHandle;
class FrequentFlyerTrx;

class FrequentFlyerContentHandler : public xercesc::DefaultHandler
{
public:

  bool parse(const char* content);

  void setTrxType(DataHandle& dataHandle, Trx*& trx) const;

  void setTrx(FrequentFlyerTrx* trx) const;

private:
  void startElement(const XMLCh* const uri,
                    const XMLCh* const localname,
                    const XMLCh* const qname,
                    const xercesc::Attributes& attrs) override;

  std::string transcodeToStrUpper(const XMLCh* const xch);

  bool getAttrValue(const xercesc::Attributes& attrs, const char* const attr, std::string& value);

  std::set<CarrierCode> _cxrs;
};
}
