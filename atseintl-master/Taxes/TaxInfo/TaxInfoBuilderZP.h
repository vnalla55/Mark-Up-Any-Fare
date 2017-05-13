//----------------------------------------------------------------------------
//  File:           PfcInfoBuilderZP.h
//  Authors:        Piotr Lach
//  Created:        12/12/2008
//  Description:    TaxInfoBuilderZP header file for ATSE V2 Tax Info Project.
//                  ZP Tax builder.
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------
#ifndef TAX_INFO_BUILDER_ZP_H
#define TAX_INFO_BUILDER_ZP_H

#include "Taxes/TaxInfo/TaxInfoBuilder.h"

namespace tse
{

class TaxInfoBuilderZP : public TaxInfoBuilder
{
  friend class TaxInfoBuilderZPTest;

public:
  typedef TaxInfoBuilder::Response Response;

  static const TaxCode TAX_CODE;

  TaxInfoBuilderZP();
  virtual ~TaxInfoBuilderZP();

  void buildDetails(TaxTrx& trx) override;

private:
  void buildItem(TaxTrx& trx, LocCode& airport);
  bool isDomestic(const Loc& loc);

  LocCode _prevArpt;
};

} // namespace tse
#endif
