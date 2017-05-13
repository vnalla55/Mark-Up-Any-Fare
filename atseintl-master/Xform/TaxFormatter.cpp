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


#include "Xform/PricingResponseXMLTags.h"
#include "Xform/TaxFormatter.h"

#include <vector>

namespace tse
{

bool
TaxFormatter::isTaxExempted(const std::string& taxCode)
{
  if (_isExemptAllTaxes)
    return true;

  if (_isExemptSpecificTaxes)
  {
    if (_taxIdExempted.empty())
      return true;

    for(const std::string& taxIdExempted : _taxIdExempted)
    {
      if (taxCode.compare(0, taxIdExempted.size(), taxIdExempted) == 0)
        return true;
    }
  }

  return false;
}

void
TaxFormatter::formatTaxInformation(const OCFees::TaxItem& taxItem)
{
  _construct.openElement(xml2::TaxInformation);
  _construct.addAttribute(xml2::ATaxCode, taxItem.getTaxCode());
  _construct.addAttributeDouble(xml2::TaxAmount, taxItem.getTaxAmount(), taxItem.getNumberOfDec());
  _construct.addAttributeChar(xml2::TaxPercentage, taxItem.getTaxType());
  _construct.closeElement();
}

void
TaxFormatter::formatTaxExempt(const OCFees::TaxItem& taxItem)
{
  _construct.openElement(xml2::TaxExempt);
  _construct.addAttribute(xml2::ATaxCode, taxItem.getTaxCode());
  _construct.addAttributeDouble(xml2::TaxAmount, taxItem.getTaxAmount(), taxItem.getNumberOfDec());
  _construct.addAttributeChar(xml2::TaxPercentage, taxItem.getTaxType());
  _construct.closeElement();
}


std::vector<OCFees::TaxItem>::const_iterator
TaxFormatter::getEndTaxOnOcIterator(const std::vector<OCFees::TaxItem>& taxItems)
{
  std::vector<OCFees::TaxItem>::const_iterator iEndTaxItem;

  if (taxItems.size() > TAX_ON_OC_BUFF_SIZE)
  {
    iEndTaxItem = taxItems.begin();
    advance(iEndTaxItem, TAX_ON_OC_BUFF_SIZE);
  }
  else
  {
    iEndTaxItem = taxItems.end();
  }

  return iEndTaxItem;
}

} // end of tse namespace
