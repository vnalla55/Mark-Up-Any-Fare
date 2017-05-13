//----------------------------------------------------------------------------
//  File:        TaxFactory.h
//  Created:     2012-10-31
//
//  Description: Factory class for Tax objects
//
//  Copyright Sabre 2012
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

#ifndef TAXFACTORY_H
#define TAXFACTORY_H

#include "DBAccess/DataHandle.h"
#include "Taxes/LegacyTaxes/Tax.h"

namespace tse
{

class TaxFactoryBase
{
public:
  virtual Tax* getTax(DataHandle& dataHandle) = 0;

  virtual ~TaxFactoryBase() {}
};

template <typename TTax>
class TaxFactory : public TaxFactoryBase
{
public:
  virtual ~TaxFactory() {}

  Tax* getTax(DataHandle& dataHandle) override
  {
    TTax* tax = nullptr;
    dataHandle.get(tax);

    return tax;
  }
};

} // namespace tse

#endif // TAXFACTORY_H
