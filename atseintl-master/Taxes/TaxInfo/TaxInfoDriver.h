//----------------------------------------------------------------------------
//  File:           TaxInfoDriver.h
//  Authors:        Piotr Lach
//  Created:        12/18/2008
//  Description:    TaxInfoBuilderFactory creates TaxInfoBuilder instance.
//                  TaxInfoDriver builds TaxInfoResponseItems.
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
#ifndef TAX_INFO_DRIVER_H
#define TAX_INFO_DRIVER_H

#include "DataModel/TaxTrx.h"
#include "Taxes/TaxInfo/TaxInfoBuilder.h"

namespace tse
{
class TaxInfoBuilderFactory
{
public:
  static TaxInfoBuilder* getInstance(TaxTrx& trx, const TaxCode& taxCode);
};

class TaxInfoDriver
{
public:
  TaxInfoDriver(TaxTrx* trx);
  virtual ~TaxInfoDriver();

  void buildTaxInfoResponse();

private:
  void build(const TaxInfoItem& item);

  const TaxTrx* trx() const { return _trx; }
  TaxTrx* trx() { return _trx; }

  TaxTrx* _trx;
};

} // namespace tse
#endif
