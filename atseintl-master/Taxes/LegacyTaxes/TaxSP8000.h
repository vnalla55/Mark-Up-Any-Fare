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
#ifndef TAX_SP8000_H
#define TAX_SP8000_H

#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/Tax.h"

#include <log4cxx/helpers/objectptr.h>
namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class TaxResponse;
class TaxCodeReg;
class PricingTrx;

class TaxSP8000 : public Tax
{
  friend class TaxSP8000Test;

public:
  TaxSP8000() {}
  virtual ~TaxSP8000() {}

  bool validateLocRestrictions(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               TaxCodeReg& taxCodeReg,
                               uint16_t& startIndex,
                               uint16_t& endIndex) override;

private:
  static log4cxx::LoggerPtr _logger;

  TaxSP8000(const TaxSP8000& tax);
  TaxSP8000& operator=(const TaxSP8000& tax);
};

} /* end tse namespace */

#endif /* TAX_SP8000_H */
