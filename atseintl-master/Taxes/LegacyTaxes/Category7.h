//----------------------------------------------------------------------------
//  File:           Category7.h
//  Description:    Category7 header file for ATSE International Project
//  Created:        6/27/2007
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

#ifndef CATEGORY7_H
#define CATEGORY7_H

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include <log4cxx/helpers/objectptr.h>

namespace tse
{
class TaxTrx;
class TaxDisplayItem;

class Category7
{

public:
  Category7();
  virtual ~Category7();
  static const std::string name() { return std::string("AIRLINE"); }
  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------
  Category7(const Category7& item);
  Category7& operator=(const Category7& item);

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
  std::string
  cityPairFlightInfo(TaxDisplayItem& taxDisplayItem, CarrierCode carrierCode, char direction);

  std::string flightInfo(uint16_t flight1, uint16_t flight2);

  std::string _subCat1; //
  std::string _subCat2; //
  std::string _subCat3; //
  std::string _subCat4; //
};
} // namespace tse
#endif // CATEGORY7_H
