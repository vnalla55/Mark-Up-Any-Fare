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
#include <cstdint>
#include <memory>
#include <utility>

class XMLConstruct;

namespace tse
{
struct StructuredRuleData;
class FareComponentDataFormatter
{
public:
  FareComponentDataFormatter(XMLConstruct& xml);

  void format(const StructuredRuleData& structuredRuleData,
              const uint16_t fareComponentNumber,
              const uint16_t pricingUnitNumber);
  void addEmptyFCDTag(const uint16_t fareComponentNumber, const uint16_t pricingUnitNumber);

private:
  void openFDCTag(const uint16_t fareComponentNumber, const SegmentOrder pricingUnitNumber);
  void printStructuredFareRulesData(const StructuredRuleData& structuredRuleData);

  XMLConstruct& _xml;
  StructuredFareRulesCommon _commonFormatter;
};
}
