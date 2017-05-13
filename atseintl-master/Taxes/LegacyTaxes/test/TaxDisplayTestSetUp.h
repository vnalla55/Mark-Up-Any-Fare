//----------------------------------------------------------------------------
//  File:           TaxDisplayTestSetUp.h
//  Created:        8/31/2007
//  Authors:        Piotr Lach
//
//  Description: This Object will be used for Test Tax Display.
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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Itin.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxRequest.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayList.h"

#include "test/testdata/TestTaxCodeRegFactory.h"

#include <boost/any.hpp>

#include <string>

namespace tx_test
{

template <typename CategoryX>
class TaxDisplayTestSetUp
{

public:
  TaxDisplayTestSetUp();
  void init();
  void build(boost::any inputData);
  void setSubCat(std::string& (CategoryX::*subCatN)(void) = &CategoryX::subCat1);
  std::string& result();

private:
  tse::TaxTrx _trx;
  tse::TaxRequest _taxRequest;
  tse::TaxResponse _taxResponse;
  tse::TaxDisplayItem _taxDisplayItem;

  CategoryX _category;
  std::string& (CategoryX::*_subCat)(void);
};

template <typename CategoryX>
TaxDisplayTestSetUp<CategoryX>::TaxDisplayTestSetUp()
{
  init();
}

template <typename CategoryX>
void
TaxDisplayTestSetUp<CategoryX>::init()
{
  tse::DateTime dt;
  tse::Itin itin;

  _trx.setRequest(&_taxRequest);
  _trx.getRequest()->ticketingDT() = dt.localTime();
  _trx.itin().push_back(&itin);
  _trx.itin().front()->mutableTaxResponses().push_back(&_taxResponse);
}

template <typename CategoryX>
void
TaxDisplayTestSetUp<CategoryX>::setSubCat(std::string& (CategoryX::*subCatN)(void))
{
  _subCat = subCatN;
}

template <typename CategoryX>
void
TaxDisplayTestSetUp<CategoryX>::build(boost::any inputData)
{
  try
  {
    std::string inputFile = boost::any_cast<std::string>(inputData);
    _taxDisplayItem.taxCodeReg() = TestTaxCodeRegFactory::create(inputFile);
  }
  catch (boost::bad_any_cast&)
  {
    tse::TaxCodeReg* taxCodeReg = boost::any_cast<tse::TaxCodeReg*>(inputData);
    _taxDisplayItem.taxCodeReg() = taxCodeReg;
  }

  _category.build(_trx, _taxDisplayItem);
}

template <typename CategoryX>
std::string&
TaxDisplayTestSetUp<CategoryX>::result()
{
  return (_category.*_subCat)();
}

} // namespace tx_test
