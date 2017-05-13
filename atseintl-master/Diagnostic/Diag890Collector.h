//----------------------------------------------------------------------------
//  File:        Diag890Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 890 Branded Fares - request to Branded service
//  Updates:
//
//  Copyright Sabre 2013
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

#pragma once

#include "Diagnostic/DiagCollector.h"

namespace tse
{

enum BrandingMessageFormat: unsigned int
{
  BS_NO_FORMAT = 0,
  BS_GREEN_SCREEN_FORMAT = 1,
  BS_CDATA_SECTION = 2
};

class Diag890Collector : public DiagCollector
{
  static const std::string XSLT_TRANSFORM_TO_INTERMEDIATE_XML_FORMAT;
  static const std::string XSLT_TRANSFORM_TO_TEXT_FORMAT;

public:

  explicit Diag890Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag890Collector() {}

  void displayXML(const std::string& xmlData,
                  const std::string& diagHeader,
                  BrandingMessageFormat xmlFormat,
                  StatusBrandingService status = StatusBrandingService::NO_BS_ERROR);

  void displayXML_old(const std::string& xmlData,
                  const std::string& diagHeader,
                  bool formatXml,
                  StatusBrandingService status = StatusBrandingService::NO_BS_ERROR);

private:
  void printHeader(DiagCollector& dc, const std::string& diagHeader);
  void printFooter(DiagCollector& dc, const std::string& diagHeader);
  void printFormattedXml(DiagCollector& dc, const std::string& xmlData);
  void convertXmlToText(DiagCollector& dc, const std::string& xmlData);

};

} // namespace tse

