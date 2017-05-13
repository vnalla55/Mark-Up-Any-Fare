/*----------------------------------------------------------------------------
 *  Copyright Sabre 2016
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/
#pragma once

#include "DataModel/StructuredRuleData.h"
#include "Xform/StructuredFareRulesCommon.h"

class XMLConstruct;

namespace tse
{

class FarePathSFRDataFormatter
{
public:
  explicit FarePathSFRDataFormatter(XMLConstruct& xml) : _xml(xml), _commonFormatter(xml) {}
  void format(const MostRestrictiveJourneySFRData& data);

private:
  void
  printCat5Data(const DateTime& mostRestrictiveAdvResData, const DateTime& mostRestrictiveTktData);
  void printJourneyData(const MostRestrictiveJourneySFRData& data);

  XMLConstruct& _xml;
  StructuredFareRulesCommon _commonFormatter;
};
}
