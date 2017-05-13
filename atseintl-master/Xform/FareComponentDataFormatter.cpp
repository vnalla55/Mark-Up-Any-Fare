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

#include "Xform/FareComponentDataFormatter.h"

#include "DataModel/StructuredRuleData.h"
#include "Common/XMLConstruct.h"
#include "Xform/PricingResponseXMLTags.h"

namespace tse
{
FareComponentDataFormatter::FareComponentDataFormatter(XMLConstruct& xml)
  : _xml(xml), _commonFormatter(xml)
{
}

void
FareComponentDataFormatter::format(const StructuredRuleData& structuredRuleData,
                                   const uint16_t fareComponentNumber,
                                   const uint16_t pricingUnitNumber)
{
  openFDCTag(fareComponentNumber, pricingUnitNumber);
  printStructuredFareRulesData(structuredRuleData);
  _xml.closeElement();
}

void
FareComponentDataFormatter::addEmptyFCDTag(const uint16_t fareComponentNumber,
                                           const uint16_t pricingUnitNumber)
{
  openFDCTag(fareComponentNumber, pricingUnitNumber);
  _xml.closeElement();
}

void
FareComponentDataFormatter::openFDCTag(const uint16_t fareComponentNumber,
                                       const uint16_t pricingUnitNumber)
{
  _xml.openElement(xml2::FareComponentLevelData);
  _xml.addAttributeShort(xml2::FareComponentNumber, fareComponentNumber);
  _xml.addAttributeShort(xml2::PricingUnitNumber, pricingUnitNumber);
}

void
FareComponentDataFormatter::printStructuredFareRulesData(
    const StructuredRuleData& structuredRuleData)
{
  _commonFormatter.printCat5Data(structuredRuleData._advanceReservation,
                                 structuredRuleData._advanceTicketing);
  _commonFormatter.printCat6Data(structuredRuleData._minStayDate,
                                 structuredRuleData._minStayLocation);
  _commonFormatter.printCat7Data(structuredRuleData._maxStayMostRestrictiveFCData);
}
}
