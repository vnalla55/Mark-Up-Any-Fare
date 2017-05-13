//----------------------------------------------------------------------------
//  File:           Category3.h
//  Description:    Category3 header file for ATSE International Project
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

#ifndef CATEGORY3_H
#define CATEGORY3_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{
class TaxTrx;
class TaxDisplayItem;

class Category3
{

public:
  Category3();
  virtual ~Category3();
  static const std::string name() { return std::string("SALE"); }
  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------
  Category3(const Category3& item);
  Category3& operator=(const Category3& item);

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

protected:
private:
  class PosPoi
  {
  public:
    PosPoi(TaxTrx& taxTrx, LocType& locType, LocCode& loc, Indicator& exlcInd);
    ~PosPoi() {};
    bool operator!=(const PosPoi& rhs);
    std::string locDecorator(TaxTrx& taxTrx);
    std::string addInOrSpace();

  private:
    Indicator& _exlcInd;
    LocType& _locType;
    std::string _loc;

    const std::string ANYWHERE;

    friend bool operator!=(const PosPoi& lhs, const PosPoi& rhs);
  };

  std::string formatPoiPosLine(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  std::string _subCat1; // POS - POI Detail
  std::string _subCat2; //
  std::string _subCat3; //
  std::string _subCat4; //
  std::string _subCat5; //
  std::string _subCat6; //
};

} // namespace tse
#endif // CATEGORY3_H
