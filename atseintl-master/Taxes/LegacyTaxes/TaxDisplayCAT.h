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
#ifndef TAX_DISPLAY_CAT_H
#define TAX_DISPLAY_CAT_H

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
class TaxDisplayItem;

//---------------------------------------------------------------------------
// Tax special process 38 for Great Britian
//---------------------------------------------------------------------------

class TaxDisplayCAT
{
public:
  TaxDisplayCAT();
  virtual ~TaxDisplayCAT();

  bool buildCats(TaxTrx& taxTrx);

private:
  void buildSequenceInfo(TaxTrx& taxTrx, uint16_t cat, TaxDisplayItem* taxDisplayItem);

  void buildCatDiff(TaxDisplayItem* taxDisplayItem1, TaxDisplayItem* taxDisplayItem2);

  void buildCat1(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem, bool singleSequence = true);

  void buildCat2(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat3(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat4(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat5(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat6(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat7(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat8(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat9(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat10(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat11(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat12(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat13(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat14(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat15(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat16(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat17(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat18(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildReissue(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildDateVerbiage(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  static log4cxx::LoggerPtr _logger;

  TaxDisplayCAT(const TaxDisplayCAT& map);
  TaxDisplayCAT& operator=(const TaxDisplayCAT& map);
};

} /* end tse namespace */

#endif /* TAX_DISPLAY_CAT_H */
