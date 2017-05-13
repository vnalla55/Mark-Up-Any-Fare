//----------------------------------------------------------------------------
//  File:           Category14.h
//  Description:    Category14 header file for ATSE International Project
//  Created:        6/25/2007
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

#ifndef CATEGORY14_H
#define CATEGORY14_H

#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include <string>

namespace tse
{
class TaxTrx;
class TaxDisplayItem;

class Category14
{

public:
  Category14();
  virtual ~Category14();
  static const std::string name() { return std::string("DISCOUNT"); }
  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------
  Category14(const Category14& item);
  Category14& operator=(const Category14& item);

  void build(TaxTrx& trx, TaxDisplayItem& taxDisplayItem);
  std::string& subCat1();
  std::string& subCat2();

private:
  std::string _subCat1;
  std::string _subCat2;
};
} // namespace tse
#endif // CATEGORY14_H
