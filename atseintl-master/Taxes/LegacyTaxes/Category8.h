//----------------------------------------------------------------------------
//  File:           Category8.h
//  Description:    Category8 header file for ATSE International Project
//  Created:        8/29/2007
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

#ifndef CATEGORY8_H
#define CATEGORY8_H

#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include <string>

namespace tse
{
class TaxTrx;
class TaxDisplayItem;

class Category8
{

public:
  Category8();
  virtual ~Category8();
  static const std::string name() { return std::string("EQUIPMENT"); }
  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------
  Category8(const Category8& item);
  Category8& operator=(const Category8& item);

  void build(TaxTrx& trx, TaxDisplayItem& taxDisplayItem);

  std::string& subCat1() { return _subCat1; }
  const std::string& subCat1() const { return _subCat1; }

  std::string& subCat2() { return _subCat2; }
  const std::string& subCat2() const { return _subCat2; }

private:
  std::string _subCat1;
  std::string _subCat2;
};

} // namespace tse
#endif // CATEGORY8_H