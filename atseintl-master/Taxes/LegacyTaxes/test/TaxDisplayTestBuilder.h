//----------------------------------------------------------------------------
//  File:           TaxDisplayTestSetUp.h
//  Created:        8/18/2007
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

#ifndef TAX_DISPLAY_TEST_BUILDER_H
#define TAX_DISPLAY_TEST_BUILDER_H

#include "Taxes/LegacyTaxes/test/TaxDisplayTestSetUp.h"

#include <string>
#include <tuple>
#include <vector>

#include <boost/any.hpp>
#include <cppunit/extensions/HelperMacros.h>

namespace tx_test
{

const static std::string xmlDir = "./testdata/";

typedef std::tuple<boost::any, // input data (XML file name or TaxCodeReg ptr)
                   std::string, // expected param#1
                   std::string, // expected param#2
                   std::string, // expected param#3
                   std::string, // expected param#4
                   std::string // expected param#5
                   > IORecord;
typedef std::vector<IORecord> IOContainer;

template <class AnyType>
inline static IORecord
buildIORecord(AnyType&& any,
              std::string arg1 = std::string(),
              std::string arg2 = std::string(),
              std::string arg3 = std::string(),
              std::string arg4 = std::string(),
              std::string arg5 = std::string())
{
  return IORecord(std::forward<AnyType>(any),
                  std::move(arg1),
                  std::move(arg2),
                  std::move(arg3),
                  std::move(arg4),
                  std::move(arg5));
}

template <typename CategoryX>
class TaxDisplayTestBuilder
{

public:
  TaxDisplayTestBuilder();
  void execute(IOContainer& io, std::string& (CategoryX::*subCatN)(void) = &CategoryX::subCat1);

private:
  std::string expectedDisplay(IORecord& ior) const;

  int _counter;
};

template <typename CategoryX>
TaxDisplayTestBuilder<CategoryX>::TaxDisplayTestBuilder()
  : _counter(0)
{
}

template <typename CategoryX>
void
TaxDisplayTestBuilder<CategoryX>::execute(IOContainer& io, std::string& (CategoryX::*subCatN)(void))
{
  std::ostringstream oss;
  for (IOContainer::iterator ioRecordIt = io.begin(); ioRecordIt != io.end(); ioRecordIt++)
  {
    boost::any inputData = std::get<0>(*ioRecordIt);
    std::string file;
    try { file = boost::any_cast<std::string>(inputData); }
    catch (boost::bad_any_cast&) { file.clear(); }

    TaxDisplayTestSetUp<CategoryX> setUp;

    setUp.setSubCat(subCatN);
    setUp.build(inputData);

    oss << "SubTestCase #" << _counter++;
    CPPUNIT_ASSERT_EQUAL_MESSAGE(oss.str(), expectedDisplay(*ioRecordIt), setUp.result());
    oss.str("");
  }
}

template <typename CategoryX>
std::string
TaxDisplayTestBuilder<CategoryX>::expectedDisplay(IORecord& ior) const
{
  return (std::get<1>(ior));
}

} // namespace

#endif // TAX_DISPLAY_TEST_BUILDER_H
