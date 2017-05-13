// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include <string>

class XMLConstruct;

namespace tse
{

class TaxTrx;

class AtpcoTaxDisplayResponseFormatter
{
public:
  AtpcoTaxDisplayResponseFormatter();
  void formatResponse(TaxTrx& trx);

private:
  // TODO: all above should be inherited from shared with TaxDisplayResponse parent
  void addResponseHeader(TaxTrx& taxTrx, XMLConstruct& construct);
  void addResponseFooter(XMLConstruct& construct);
  const std::string readConfigXMLNamespace(const std::string& configName);

  static const std::string XML_DECLARATION_TAG_TEXT;
  static const std::string TAX_DISPLAY_XML_VERSION_TEXT;
  const std::string XML_NAMESPACE_TEXT;
  const std::string XML_NAMESPACE_XS_TEXT;
  const std::string XML_NAMESPACE_XSI_TEXT;
};

} /* namespace tse */

