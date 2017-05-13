//----------------------------------------------------------------------------
//  File:           Category1.h
//  Description:    Category1 header file for ATSE International Project
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

#ifndef CATEGORY1_H
#define CATEGORY1_H

#include "Common/TseStringTypes.h"
#include <log4cxx/helpers/objectptr.h>

namespace tse
{
class TaxTrx;
class TaxDisplayItem;

class Category1
{

public:
  Category1();
  virtual ~Category1();
  static const std::string name() { return std::string("TAX"); }
  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------
  Category1(const Category1& item);
  Category1& operator=(const Category1& item);

  void build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem, bool singleSequence = false);

  std::string formatPoiPosLine(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  std::string& displayOnly() { return _displayOnly; }
  const std::string& displayOnly() const { return _displayOnly; }

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

  std::string& subCat10() { return _subCat10; }
  const std::string& subCat10() const { return _subCat10; }

  std::string& subCat11() { return _subCat11; }
  const std::string& subCat11() const { return _subCat11; }

protected:
private:
  void buildCat3(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat4(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat5(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat6(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat7(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  void buildCat9(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  std::string _displayOnly; // DISPLAY ONLY MESSAGE
  std::string _subCat1; // CURRENCY-AMOUNT-PERCENT-SPECIAL PROCESS-SEQUENCE
  std::string _subCat2; // POS - POI
  std::string _subCat3; // TRAVEL
  std::string _subCat4; // LOCATION
  std::string _subCat5; // TRAVEL TYPE
  std::string _subCat6; // ITINERARY TYPE
  std::string _subCat7; // VALIDATING CARRIER
  std::string _subCat8; // CURRENCY
  std::string _subCat9; // PASSENGER TYPE
  std::string _subCat10; // CARRIERS RESTRICTED
  std::string _subCat11; // CARRIER FLIGHT RESTRICTIONS
};
} // namespace tse
#endif // CATEGORY1_H
