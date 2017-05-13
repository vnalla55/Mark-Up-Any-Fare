//----------------------------------------------------------------------------
//  File:           Category18.h
//  Description:    Category18 header file for ATSE International Project
//  Created:        8/31/2007
//  Authors:        Piotr Lach
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

#ifndef CATEGORY18_H
#define CATEGORY18_H

#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"

namespace tse
{
class TaxTrx;
class TaxDisplayItem;

class Category18
{

public:
  Category18();
  virtual ~Category18();
  static const std::string name() { return std::string("REFUND"); }
  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------
  Category18(const Category18& item);
  Category18& operator=(const Category18& item);

  void build(TaxTrx& trx, TaxDisplayItem& taxDisplayItem);

  std::string& subCat1() { return _subCat1; }
  const std::string& subCat1() const { return _subCat1; }

private:
  std::string _subCat1;
};
} // namespace tse
#endif // CATEGORY18_H
