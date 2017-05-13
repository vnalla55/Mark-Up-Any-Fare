//----------------------------------------------------------------------------
//  File:           Category13.h
//  Description:    Category13 header file for ATSE International Project
//  Created:        8/30/2007
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

#ifndef CATEGORY13_H
#define CATEGORY13_H

#include "DataModel/TaxTrx.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "DBAccess/TaxCodeCabin.h"
#include <string>
#include <set>
#include <map>
#include <vector>

namespace tse
{
class TaxTrx;
class TaxDisplayItem;

class Category13
{

public:
  typedef std::set<std::string> Cabins;
  typedef std::set<std::string> Carriers;
  typedef std::map<std::string, Cabins> CarrierCabins;

  Category13();
  virtual ~Category13();

  static const std::string name() { return std::string("CABIN"); }
  static const uint32_t NUMBER;

  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------
  Category13(const Category13& item);
  Category13& operator=(const Category13& item);

  void build(TaxTrx& trx, TaxDisplayItem& taxDisplayItem);

  std::string& subCat1() { return _subCat1; }
  const std::string& subCat1() const { return _subCat1; }

  std::string& subCat2() { return _subCat2; }
  const std::string& subCat2() const { return _subCat2; }

  std::string& subCat3() { return _subCat3; }
  const std::string& subCat3() const { return _subCat3; }

private:
  std::string formatCabinDetailInfo(TaxTrx& taxTrx,
                                    TaxDisplayItem& taxDisplayItem,
                                    CarrierCabins& carrierCabins,
                                    CarrierCabins& carrierExclCabins);
  bool locInfo(TaxTrx& trx, std::vector<TaxCodeCabin*>::iterator cabinPtrs);

  bool flightInfo(std::vector<TaxCodeCabin*>::iterator cabinPtrs);

  void insertCabin(CarrierCabins& carrierCabins, std::string carrier, std::string cabin);

  void getCarrierCabinInfo(TaxTrx& trx,
                           TaxDisplayItem& taxDisplayItem,
                           Cabins& cabinsNoCarrier,
                           Cabins& cabinsExclNoCarrier,
                           Carriers& carriers,
                           CarrierCabins& carrierCabins,
                           CarrierCabins& carrierExclCabins);

  std::string _subCat1;
  std::string _subCat2;
  std::string _subCat3;
};
} // namespace tse
#endif // CATEGORY13_H
