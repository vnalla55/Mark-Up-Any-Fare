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

#include "Common/XMLConstruct.h"
#include "DataModel/StructuredRuleData.h"

namespace tse
{
class StructuredFareRulesCommon
{
public:
  explicit StructuredFareRulesCommon(XMLConstruct& xml) : _xml(xml) {}

  void printCat5Data(const DateTime& advResDate, const DateTime& advTktDate);
  void printCat7Data(const MaxStayMap& maxStayData);
  void printCat6Data(const DateTime& date, const LocCode& location);

private:
  XMLConstruct& _xml;
};
}
