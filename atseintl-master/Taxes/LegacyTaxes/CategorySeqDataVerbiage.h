//----------------------------------------------------------------------------
//  File:
//  Description:    Category8 header file for ATSE International Project
//  Created:        17/12/2007
//  Authors:        Jakub Kubica
//
//  Description: This Object will be used for Tax Display functionality.
//
//  Updates:
//
//  Copyright Sabre 2007
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

#ifndef CATEGORYSEQDATAVER_H
#define CATEGORYSEQDATAVER_H

#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include <string>

namespace tse
{
class TaxTrx;
class TaxDisplayItem;

class CategorySeqDataVerb
{

public:
  CategorySeqDataVerb();
  virtual ~CategorySeqDataVerb();
  static const std::string name() { return std::string("DATA VERBIAGE"); }
  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------
  CategorySeqDataVerb(const CategorySeqDataVerb& item);
  CategorySeqDataVerb& operator=(const CategorySeqDataVerb& item);
  void build(TaxTrx& trx, TaxDisplayItem& taxDisplayItem);
  void expiryDataInfo(TaxTrx& trx, TaxCodeReg* tcreg);

  std::string& subCat1() { return _subCat1; }
  const std::string& subCat1() const { return _subCat1; }

private:
  std::string _subCat1;
};

} // namespace tse
#endif // CATEGORYSEQDATAVER_H
