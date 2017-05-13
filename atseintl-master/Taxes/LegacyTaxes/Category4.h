//----------------------------------------------------------------------------
//  File:           Category4.h
//  Description:    Category4 header file for ATSE International Project
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

#ifndef CATEGORY4_H
#define CATEGORY4_H

#include "Common/TseStringTypes.h"
#include <log4cxx/helpers/objectptr.h>

namespace tse
{
class TaxTrx;
class TaxDisplayItem;

class Category4
{

public:
  Category4();
  virtual ~Category4();
  static const std::string name() { return std::string("TRAVEL"); }
  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------
  Category4(const Category4& item);
  Category4& operator=(const Category4& item);

  void build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem, bool singleSequence = false);

  std::string formatTravelLine(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  std::string formatLocationLine(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

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

protected:
private:
  std::string formatTripType(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  std::string formatLocationRestriction1(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  std::string formatLocationRestriction2(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  std::string formatOriginRestriction(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  std::string _location1; // Loc1 Descriptions
  std::string _location2; // Loc2 Descriptions
  std::string _originLoc;
  std::string _subCat1; //
  std::string _subCat2; //
  std::string _subCat3; //
  std::string _subCat4; //
  std::string _subCat5; //
  std::string _subCat6; //
};
} // namespace tse
#endif // CATAGORY4_H
