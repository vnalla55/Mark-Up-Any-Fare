//----------------------------------------------------------------------------
//  File:           Category2.h
//  Description:    Category2 header file for ATSE International Project
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

#ifndef CATEGORY2_H
#define CATEGORY2_H

#include "Common/TseStringTypes.h"
#include <log4cxx/helpers/objectptr.h>

namespace tse
{
class TaxTrx;
class TaxDisplayItem;

class Category2
{

public:
  Category2();
  virtual ~Category2();
  static const std::string name() { return std::string("TAX DETAIL"); }
  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------
  Category2(const Category2& item);
  Category2& operator=(const Category2& item);

  void build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  std::string& subCat1() { return _subCat1; }
  const std::string& subCat1() const { return _subCat1; }

  std::string& subCat2() { return _subCat2; }
  const std::string& subCat2() const { return _subCat2; }

  std::string& subCat3() { return _subCat3; }
  const std::string& subCat3() const { return _subCat3; }

  std::string& subCat4() { return _subCat4; }
  const std::string& subCat4() const { return _subCat4; }

  std::string& subCat5() { return _subCat5; }
  const std::string& subCat5() const { return _subCat5; }

  std::string& subCat6() { return _subCat6; }
  const std::string& subCat6() const { return _subCat6; }

  std::string& subCat7() { return _subCat7; }
  const std::string& subCat7() const { return _subCat7; }

  std::string& subCat8() { return _subCat8; }
  const std::string& subCat8() const { return _subCat8; }

  std::string& subCat9() { return _subCat9; }
  const std::string& subCat9() const { return _subCat9; }

protected:
private:
  std::string _subCat1; // Minimum
  std::string _subCat2; // Maximum
  std::string _subCat3; // Plus Up
  std::string _subCat4; // Tax On Tax
  std::string _subCat5; // Excess Baggage
  std::string _subCat6; // Full Fare Amount
  std::string _subCat7; // Occurance
  std::string _subCat8; // Range
  std::string _subCat9; // Common Tax Text

  static const uint32_t EARTH_CIRCUMFERENCE;
};
} // namespace tse
#endif // CATEGORY2_H
