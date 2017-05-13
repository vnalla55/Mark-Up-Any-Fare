//---------------------------------------------------------------------------
//  Copyright Sabre 2010
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
#ifndef TAX_YN_H
#define TAX_YN_H

#include "Taxes/LegacyTaxes/Tax.h"
#include "DataModel/TaxResponse.h"
#include <boost/utility.hpp>

namespace tse
{
class TaxYN : public Tax, boost::noncopyable
{
  friend class TaxYNMock;

public:
  TaxYN() {}
  ~TaxYN() {}

private:
  MoneyAmount fareAmountInNUC(const PricingTrx& trx, const TaxResponse& taxResponse) override;
  MoneyAmount discFactor(const PricingTrx& trx, int16_t segmentOrder);
};
}

#endif
