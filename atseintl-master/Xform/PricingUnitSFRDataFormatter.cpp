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

#include "Xform/PricingUnitSFRDataFormatter.h"

#include "Common/XMLConstruct.h"
#include "Xform/PricingResponseXMLTags.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FarePath.h"
#include "DataModel/StructuredRuleData.h"

namespace tse
{
void
PricingUnitSFRDataFormatter::printPricingUnitData(const MostRestrictivePricingUnitSFRData& ruleData)
{
  for (const auto& element : ruleData._minStayMap)
  {
    const std::pair<DateTime, LocCode> minStayData = element.second;
    _commonFormatter.printCat6Data(minStayData.first, minStayData.second);
  }
  _commonFormatter.printCat7Data(ruleData._maxStayMap);
}

void
PricingUnitSFRDataFormatter::openPUDTag(const uint16_t pricingUnitNumber)
{
  _xml.openElement(xml2::PricingUnitLevelData);
  _xml.addAttributeShort(xml2::PricingUnitNumber, pricingUnitNumber);
}

void
PricingUnitSFRDataFormatter::addEmptyPUDTag(const uint16_t pricingUnitNumber)
{
  openPUDTag(pricingUnitNumber);
  _xml.closeElement();
}

void
PricingUnitSFRDataFormatter::format(const uint16_t pricingUnitNumber,
                                    const MostRestrictivePricingUnitSFRData& ruleData)
{
  openPUDTag(pricingUnitNumber);
  printPricingUnitData(ruleData);
  _xml.closeElement();
}
}
