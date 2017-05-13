//----------------------------------------------------------------------------
//  File:           Category10.h
//  Description:    Category10 header file for ATSE International Project
//  Created:        8/21/2007
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

#ifndef CATEGORY10_H
#define CATEGORY10_H

#include "Common/TseStringTypes.h"

namespace tse
{
class TaxTrx;
class TaxDisplayItem;

class Category10
{

public:
  Category10();
  virtual ~Category10();
  static const std::string name() { return std::string("FARE TYPE/CLASS"); }
  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------

  Category10(const Category10& item);
  Category10& operator=(const Category10& item);

  void build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  std::string& subCat1() { return _subCat1; }
  const std::string& subCat1() const { return _subCat1; }

  std::string& subCat2() { return _subCat2; }
  const std::string& subCat2() const { return _subCat2; }

protected:
private:
  std::string _subCat1;
  std::string _subCat2;
};

} // namespace tse
#endif // CATEGORY10_H
