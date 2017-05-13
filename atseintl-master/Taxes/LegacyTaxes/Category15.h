//----------------------------------------------------------------------------
//  File:           Category15.h
//  Description:    Category15 header file for ATSE International Project
//  Created:        8/22/2007
//  Authors:        Sommapan Lathitham
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

#ifndef CATEGORY15_H
#define CATEGORY15_H

#include "Common/TseStringTypes.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include <string>

namespace tse
{
class TaxTrx;
class TaxDisplayItem;

class Category15
{

public:
  Category15();
  virtual ~Category15();
  static const std::string name() { return std::string("MISCELLANEOUS"); }
  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------

  Category15(const Category15& item);
  Category15& operator=(const Category15& item);

  void build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  std::string& subCat1() { return _subCat1; }
  const std::string& subCat1() const { return _subCat1; }

  std::string& subCat2() { return _subCat2; }
  const std::string& subCat2() const { return _subCat2; }

  std::string& subCat3() { return _subCat3; }
  const std::string& subCat3() const { return _subCat3; }

  std::string& subCat4() { return _subCat4; }
  const std::string& subCat4() const { return _subCat4; }

protected:
private:
  std::string _subCat1;
  std::string _subCat2;
  std::string _subCat3;
  std::string _subCat4;
};

} // namespace tse
#endif // CATEGORY15_H
