//---------------------------------------------------------------------------
//  Copyright Sabre 2004
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
#ifndef TAX_DISPLAY_DRIVER_H
#define TAX_DISPLAY_DRIVER_H

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
class TaxTrx;

//---------------------------------------------------------------------------
// Tax special process 38 for Great Britian
//---------------------------------------------------------------------------

class TaxDisplayDriver
{
public:
  TaxDisplayDriver();
  virtual ~TaxDisplayDriver();

  bool buildTaxDisplay(TaxTrx& taxTrx);

private:
  bool setUp(TaxTrx& taxTrx);
  bool buildTaxCodeInfo(TaxTrx& taxTrx);

  static log4cxx::LoggerPtr _logger;

  TaxDisplayDriver(const TaxDisplayDriver& map);
  TaxDisplayDriver& operator=(const TaxDisplayDriver& map);
};

} /* end tse namespace */

#endif /* TAX_DISPLAY_DRIVER_H */
