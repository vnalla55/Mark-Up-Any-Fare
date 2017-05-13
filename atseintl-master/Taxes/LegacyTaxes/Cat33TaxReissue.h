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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/TaxReissue.h"
#include <vector>

namespace tse
{

class TaxReissueSelector
{
  const std::vector<TaxReissue*>& _taxReissues;

  TaxReissueSelector(const TaxReissueSelector&) = delete;
  TaxReissueSelector& operator=(const TaxReissueSelector&) = delete;
public:
  static const std::string LEGACY_TAXES_TAX_TYPE;

  explicit TaxReissueSelector(const std::vector<TaxReissue*>& taxReissues)
      : _taxReissues(taxReissues)
  {
  }

  TaxReissue* getTaxReissue(const TaxType& taxType, const CarrierCode& carrier,
      bool skipCat33Only = false) const;
};

class Cat33TaxReissue
{
  const TaxReissue* _taxReissue;

  Cat33TaxReissue(const Cat33TaxReissue&) = delete;
  Cat33TaxReissue& operator=(const Cat33TaxReissue&) = delete;
public:
  static const std::string DEFAULT_CARRIER;

  explicit Cat33TaxReissue(const TaxReissue* taxReissue)
      : _taxReissue(taxReissue)
  {
  }

  const TaxReissue* getTaxReissue() const
  {
    return _taxReissue;
  }

  Indicator getRefundableTaxTag() const;
};

} // end of tse namespace
