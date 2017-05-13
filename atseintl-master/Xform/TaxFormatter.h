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


#pragma once


#include "Common/XMLConstruct.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OCFeesUsage.h"

namespace tse
{

class TaxFormatter
{
  friend class TaxFormatterTest;

  const size_t TAX_ON_OC_BUFF_SIZE = 15;

  XMLConstruct& _construct;
  bool _isExemptAllTaxes;
  bool _isExemptSpecificTaxes;
  const std::vector<std::string>& _taxIdExempted;

public:
  TaxFormatter(XMLConstruct& construct,
          bool isExemptAllTaxes,
          bool isExemptSpecificTaxes,
          const std::vector<std::string>& taxIdExempted)
      : _construct(construct),
        _isExemptAllTaxes(isExemptAllTaxes),
        _isExemptSpecificTaxes(isExemptSpecificTaxes),
        _taxIdExempted(taxIdExempted)
  {}

  template<typename T>
  void
  format(const T& ocFees)
  {
    if (ocFees.getTaxes().empty())
      return;

    auto iTaxItem = ocFees.getTaxes().begin();
    auto iEndTaxItem = getEndTaxOnOcIterator(ocFees.getTaxes());

    for (; iTaxItem != iEndTaxItem; iTaxItem++)
    {
      if (isTaxExempted(iTaxItem->getTaxCode()))
        formatTaxExempt(*iTaxItem);
      else
        formatTaxInformation(*iTaxItem);
    }
  }

private:

  TaxFormatter(const TaxFormatter&) = delete;
  TaxFormatter& operator=(const TaxFormatter&) = delete;

  void
  formatTaxInformation(const OCFees::TaxItem& taxItem);

  void
  formatTaxExempt(const OCFees::TaxItem& taxItem);

  bool
  isTaxExempted(const std::string& taxCode);

  std::vector<OCFees::TaxItem>::const_iterator
  getEndTaxOnOcIterator(const std::vector<OCFees::TaxItem>& taxItems);
};

}
