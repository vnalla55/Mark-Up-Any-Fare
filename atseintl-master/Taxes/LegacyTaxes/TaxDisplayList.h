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
#ifndef TAX_DISPLAY_LIST_H
#define TAX_DISPLAY_LIST_H

#include "Taxes/LegacyTaxes/Tax.h"
#include <log4cxx/helpers/objectptr.h>
#include "DBAccess/Nation.h"

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class TaxTrx;
class TaxNation;

//---------------------------------------------------------------------------
// Tax special process 38 for Great Britian
//---------------------------------------------------------------------------

class TaxDisplayList
{
public:
  TaxDisplayList();
  virtual ~TaxDisplayList();

  void buildList(TaxTrx& taxTrx);

private:
  bool storeTaxNationError(TaxTrx& taxTrx, const std::vector<Nation*>& allNationList);
  void storeTaxDetailInfo(TaxTrx& taxTrx, const TaxNation* taxNation);

  static log4cxx::LoggerPtr _logger;

  TaxDisplayList(const TaxDisplayList& map);
  TaxDisplayList& operator=(const TaxDisplayList& map);
};

} /* end tse namespace */

#endif /* TAX_DISPLAY_LIST_H */
