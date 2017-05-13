/*----------------------------------------------------------------------------
 *  Copyright Sabre 2015
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

#include "Common/DateTime.h"
#include "DataModel/StructuredRuleData.h"
#include "Xform/StructuredFareRulesCommon.h"

class XMLConstruct;

namespace tse
{
class PricingUnitSFRDataFormatter
{
public:
  explicit PricingUnitSFRDataFormatter(XMLConstruct& xml) : _xml(xml), _commonFormatter(xml) {}

  void format(const uint16_t pricingUnitNumber, const MostRestrictivePricingUnitSFRData& ruleData);

  void addEmptyPUDTag(const uint16_t pricingUnitNumber);

private:
  void openPUDTag(const uint16_t pricingUnitNumber);
  void printPricingUnitData(const MostRestrictivePricingUnitSFRData& ruleData);

  XMLConstruct& _xml;
  StructuredFareRulesCommon _commonFormatter;
};
}

