//----------------------------------------------------------------------------
//  File:           Category11.h
//  Description:    Category11 header file for ATSE International Project
//  Created:        6/25/2007
//  Authors:        Dean Van Decker
//
//  Description: This Object will be used for Tax Display functionality.
//
//  Updates:
//
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

#ifndef CATEGORY11_H
#define CATEGORY11_H

#include "Common/TseStringTypes.h"
#include <log4cxx/helpers/objectptr.h>

namespace tse
{
class TaxTrx;
class TaxDisplayItem;

class Category11
{

public:
  Category11();
  virtual ~Category11();
  static const std::string name() { return std::string("TRANSIT"); }
  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------
  Category11(const Category11& item);
  Category11& operator=(const Category11& item);

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
#endif // CATEGORY11_H
