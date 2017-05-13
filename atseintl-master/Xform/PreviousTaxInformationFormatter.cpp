// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Xform/AbstractTaxBreakdownModel.h"
#include "Xform/AbstractTaxInformationModel.h"
#include "Xform/PreviousTaxInformationFormatter.h"
#include "Xform/PreviousTaxInformationModel.h"
#include "Xform/PricingResponseXMLTags.h"

namespace tse
{
PreviousTaxInformationFormatter::PreviousTaxInformationFormatter(XMLConstruct& construct)
  : _construct(construct)
{
}

void
PreviousTaxInformationFormatter::formatTBD(const AbstractTaxBreakdownModel& model)
{
  _construct.openElement(xml2::TaxBreakdown);
  _construct.addAttribute(xml2::ATaxCode, model.getTaxCode());
  _construct.addAttributeDouble(
      xml2::TaxAmount, model.getTaxAmount(), model.getPaymentCurrencyPrecision());
  _construct.addAttribute(xml2::TaxCurrencyCode, model.getTaxCurrencyCode());
  _construct.addAttributeChar(xml2::RefundableTaxTag, model.getRefundableTaxTag());
  _construct.addAttribute(xml2::TaxAirlineCode, model.getTaxAirlineCode());
  _construct.closeElement();
}

void
PreviousTaxInformationFormatter::formatTaxInfoTBD(const AbstractTaxBreakdownModel& model)
{
  _construct.openElement(xml2::TaxBreakdown);
  _construct.addAttribute(xml2::ATaxCode, model.getTaxCode());
  _construct.addAttributeChar(xml2::RefundableTaxTag, model.getRefundableTaxTag());
  _construct.closeElement();
}

void
PreviousTaxInformationFormatter::formatTAX(const AbstractTaxInformationModel& model)
{
  _construct.openElement(xml2::TaxInformation);
  _construct.addAttribute(xml2::ATaxCode, model.getTaxCode());
  _construct.addAttributeDouble(
      xml2::TaxAmount, model.getTaxAmount(), model.getPaymentCurrencyPrecision());
  _construct.addAttribute(xml2::TaxCurrencyCode, model.getTaxCurrencyCode());
  _construct.addAttributeDouble(
      xml2::AmountPublished, model.getAmountPublished(), model.getPublishedCurrencyPrecision());
  _construct.addAttribute(xml2::PublishedCurrency, model.getPublishedCurrency());
  _construct.addAttribute(xml2::TaxCountryCode, model.getTaxCountryCode());
  if (model.getGoodAndServicesTax())
    _construct.addAttributeBoolean(xml2::GSTTax, true);
  _construct.addAttribute(xml2::ATaxDescription, model.getTaxDescription());
  _construct.closeElement();
}

void
PreviousTaxInformationFormatter::formatPTI(const AbstractPreviousTaxInformationModel& model)
{
  if (!model.isEmpty())
  {
    _construct.openElement(xml2::PreviousTaxInformation);

    for (const auto& each : model.getTaxInformation())
      formatTAX(*each);

    for (const auto& each : model.getTaxBreakdown())
      formatTBD(*each);

    for (const auto& each : model.getTaxInfoBreakdown())
      formatTaxInfoTBD(*each);

    if (!model.isNetEmpty())
    {
      _construct.openElement(xml2::NetRemitInfo);

      for (const auto& taxInfo : model.getNetTaxInformation())
        formatTAX(*taxInfo);

      for (const auto& taxBreakdown : model.getNetTaxBreakdown())
        formatTBD(*taxBreakdown);

      _construct.closeElement();
    }

    _construct.closeElement();
  }
}

} // end of tse namespace
