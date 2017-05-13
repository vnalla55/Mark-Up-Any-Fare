//---------------------------------------------------------------------------
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
#ifndef TAX_YS1_00_H
#define TAX_YS1_00_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "DBAccess/Loc.h"

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

class TaxYS1_00 : public Tax
{

public:
  virtual void taxCreate(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t travelSegStartIndex,
                         uint16_t travelSegEndIndex) override;

private:
  static log4cxx::LoggerPtr _logger;
};

} /* end tse namespace */

#endif /* TAX_YS1_00_H */
