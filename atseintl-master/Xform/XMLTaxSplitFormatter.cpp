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

#include "Common/FallbackUtil.h"
#include "Xform/AbstractTaxSummaryInfo.h"
#include "Xform/PricingResponseXMLTags.h"
#include "Xform/TaxSplitData.h"
#include "Xform/XformUtil.h"
#include "Xform/XMLTaxSplitFormatter.h"

#include <sstream>

namespace tse
{

void
XMLTaxSplitFormatter::formatTAX(const AbstractTaxSummaryInfo& taxSummaryInfo)
{
  _construct.openElement(xml2::TaxInformation);
  _construct.addAttribute(xml2::ATaxCode, taxSummaryInfo.getTaxCode());  // BC0
  _construct.addAttribute(xml2::TaxAmount,
      SplitFormatterUtil::formatAmount(taxSummaryInfo.getTaxValue(), taxSummaryInfo.getCurrencyNoDec()));  // C6B
  _construct.addAttribute(xml2::TaxCurrencyCode, taxSummaryInfo.getTaxCurrencyCode());  // C40
  /*
   * According to Waitman Gale suggestions attributes: S05, S04, C6A, C41 should be
   * removed form TAX element.
   *
  _construct.addAttribute(xml2::StationCode, taxSplitData.getStationCode());  // S05
  _construct.addAttribute(xml2::ATaxDescription, taxSplitData.getTaxDescription());  // S04
  _construct.addAttribute(xml2::AmountPublished, taxSplitData.getAmountPublished());  // C6A
  _construct.addAttribute(xml2::PublishedCurrency, taxSplitData.getPublishedCurrency());  // C41
  */
  _construct.addAttribute(xml2::TaxCountryCode, taxSummaryInfo.getTaxCountryCode());  // A40
  _construct.addAttributeBoolean(xml2::GSTTax, taxSummaryInfo.getGoodAndServicesTax());  // P2Q
  _construct.closeElement();
}

void
XMLTaxSplitFormatter::formatTBD(const AbstractTaxSplitData& taxSplitData)
{
  _construct.openElement(xml2::TaxBreakdown);
  _construct.addAttribute(xml2::ATaxCode, taxSplitData.getTaxCode());  // BC0
  _construct.addAttribute(xml2::TaxAmount,
      SplitFormatterUtil::formatAmount(taxSplitData.getTaxValue(), taxSplitData.getCurrencyNoDec()));  // C6B
  _construct.addAttribute(xml2::TaxCurrencyCode, taxSplitData.getTaxCurrencyCode());  // C40
  _construct.addAttribute(xml2::StationCode, taxSplitData.getStationCode());  // S05
  _construct.addAttribute(xml2::ATaxDescription, taxSplitData.getTaxDescription());  // S04
  _construct.addAttribute(xml2::AmountPublished,
      SplitFormatterUtil::formatAmount(taxSplitData.getAmountPublished()));  // C6A
  _construct.addAttribute(xml2::PublishedCurrency, taxSplitData.getPublishedCurrency());  // C41
  _construct.addAttribute(xml2::TaxCountryCode, taxSplitData.getTaxCountryCode());  // A40
  _construct.addAttribute(xml2::TaxAirlineCode, taxSplitData.getTaxAirlineCode());  // A04

  _construct.addAttributeChar(xml2::TbdTaxType, taxSplitData.getTaxType());  // A06

  _construct.addAttributeBoolean(xml2::GSTTax, taxSplitData.getGoodAndServicesTax());  // P2Q
  _construct.closeElement();
}

}
