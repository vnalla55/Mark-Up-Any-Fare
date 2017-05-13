//----------------------------------------------------------------------------
//  File:           Category6.h
//  Description:    Category6 header file for ATSE International Project
//  Created:        6/25/2007
//  Authors:        Dean Van Decker
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

#ifndef CATEGORY6_H
#define CATEGORY6_H

#include "DBAccess/TaxCodeReg.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"

namespace tse
{
class TaxTrx;
class TaxDisplayItem;

class Category6
{

public:
  Category6();
  virtual ~Category6();

  static const std::string name() { return std::string("CURRENCY"); }
  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------
  Category6(const Category6& item);
  Category6& operator=(const Category6& item);

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

protected:
private:
  std::string _subCat1;
  std::string _subCat2;
  std::string _subCat3;
  std::string _subCat4;
  std::string _subCat5;
  std::string _subCat6;
  std::string _subCat7;

  std::string getGroup4str(const RoundingRule rrule,
                           const CurrencyNoDec currnodec,
                           const std::string& roundfac);

  std::string getGroup3str(Indicator formOfPayment); // group 3 string construction
  std::string getGroup1str(Indicator taxequivAmtInd); // group 1 string construction
  std::string getGroup5str(Indicator val); // group 5 string construction
  std::string getGroup6str(Indicator val); // group 6 string construction
};
} // namespace tse
#endif // CATEGORY6_H
