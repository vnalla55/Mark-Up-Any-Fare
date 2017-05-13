//----------------------------------------------------------------------------
//  File:           Category16.h
//  Description:    Category16 header file for ATSE International Project
//  Created:        9/06/2007
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

#ifndef CATEGORY16_H
#define CATEGORY16_H

#include "Common/TseStringTypes.h"

namespace tse
{
class TaxTrx;
class TaxDisplayItem;

class Category16
{

public:
  Category16();
  virtual ~Category16();
  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------
  static const std::string name() { return std::string("REISSUE"); }
  Category16(const Category16& item);
  Category16& operator=(const Category16& item);

  void build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  std::string& subCat1() { return _subCat1; }
  const std::string& subCat1() const { return _subCat1; }

protected:
private:
  std::string _subCat1;
};

} // namespace tse
#endif // CATEGORY16_H
