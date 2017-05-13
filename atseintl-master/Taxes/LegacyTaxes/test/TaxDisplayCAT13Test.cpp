// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2007
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include <iostream>
#include <memory>
#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Taxes/LegacyTaxes/test/TaxDisplayTestBuilder.h"
#include "Taxes/LegacyTaxes/Category13.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "Taxes/LegacyTaxes/TaxDisplayList.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TaxRequest.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "test/include/TestMemHandle.h"

using namespace tx_test;
using namespace boost::assign;
namespace tse
{
class TaxDisplayCAT13Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayCAT13Test);
  CPPUNIT_TEST(testCreation);
  CPPUNIT_TEST(testDisplay);
  CPPUNIT_TEST_SUITE_END();

public:
  void testCreation()
  {
    std::unique_ptr<Category13> ptr(new Category13);
    CPPUNIT_ASSERT(ptr);
  }

  void testDisplay()
  {
    IOContainer io;
    TaxCodeReg* taxCodeReg1 = _memHandle.create<TaxCodeReg>();
    TaxCodeReg* taxCodeReg2 = _memHandle.create<TaxCodeReg>();
    TaxCodeReg* taxCodeReg3 = _memHandle.create<TaxCodeReg>();
    TaxCodeReg* taxCodeReg4 = _memHandle.create<TaxCodeReg>();
    TaxCodeReg* taxCodeReg5 = _memHandle.create<TaxCodeReg>();
    TaxCodeReg* taxCodeReg6 = _memHandle.create<TaxCodeReg>();

    taxCodeReg1->cabins() += getTCabin();
    taxCodeReg2->cabins() += getTCabin(), getTCabin('N', "Y"), getTCabin('N', "Y"),
        getTCabin('N', "T", "9W", WITHIN, 'N', "GB", 'N', "GB", 7, 78);
    taxCodeReg3->cabins() += getTCabin('Y', "F", "AC", FROM, 'N', "GB", 'N', "PL", 1, 8770);
    taxCodeReg4->cabins() += getTCabin('N', "Y"),
        getTCabin('Y', "F", "AC", FROM, 'N', "GB", 'N', "PL", 1, 8770), getTCabin('N', "A"),
        getTCabin('N', "R", "AC", WITHIN, 'N', "GB", 'N', "GB", 333, 4442);
    taxCodeReg5->cabins() += getTCabin('N', "Y"),
        getTCabin('Y', "F", "AC", FROM, 'N', "GB", 'N', "PL", 1, 8770), getTCabin('N', "A"),
        getTCabin('N', "R", "AC", WITHIN, 'N', "GB", 'N', "GB", 333, 4442),
        getTCabin('N', "R", "AF", FROM, 'N', "GB", 'N', "PL", 1, 4444),
        getTCabin('N', "R", "AF", FROM, 'N', "MX", 'N', "PL", 1, 4444),
        getTCabin('N', "A", "ZU", FROM, 'N', "US", 'N', "PL", 10, 144),
        getTCabin('N', "P", "EI", FROM, 'N', "GB", 'N', "PL", 12, 4445);
    taxCodeReg6->cabins() += getTCabin('N', "A"),
        getTCabin('N', "P", "EI", FROM, 'N', "GB", 'N', "PL", 0, 4445),
        getTCabin('N', "R", "ZU", FROM, 'N', "", 'N', "PL", 2, 4445), getTCabin('N', "K", "ZU"),
        getTCabin('N', "L", "ZU"), getTCabin('N', "C", "", WITHIN, 'N', "", 'N', "", 2, 4445),
        getTCabin('N', "D", "", FROM, 'N', "PL", 'N', "", 2, 4445),
        getTCabin('N', "X", "", FROM, 'N', "", 'N', "PL", 0, 4445),
        getTCabin('N', "M", "ZU", FROM, 'N', "", 'N', "PL", 2, 0),
        getTCabin('N', "T", "BA", FROM, 'Z', "03961", 'Z', "", 1, 9000);

    io.push_back(buildIORecord(taxCodeReg1, "     NO CABIN RESTRICTIONS APPLY.\n"));
    io.push_back(
        buildIORecord(taxCodeReg2,
                      "* APPLICABLE TO THE FOLLOWING DEFAULT CABIN INVENTORY CLASS/ES:\n"
                      "  Y.\n"
                      "  CARRIER/S WITH EXCEPTION DATA 9W -"
                      " REFER TO SABRE ENTRY TXNHELP FOR SPECIFIC TAX CODE/CARRIER CABIN DATA.\n"));
    io.push_back(buildIORecord(taxCodeReg3,
                               "  CARRIER/S WITH EXCEPTION DATA AC - REFER TO SABRE ENTRY "
                               "TXNHELP FOR SPECIFIC TAX CODE/CARRIER CABIN DATA.\n"));
    io.push_back(buildIORecord(taxCodeReg4,
                               "* APPLICABLE TO THE FOLLOWING DEFAULT CABIN INVENTORY CLASS/ES:\n"
                               "  A, Y.\n"
                               "  CARRIER/S WITH EXCEPTION DATA AC - REFER TO SABRE ENTRY TXNHELP FOR "
                               "SPECIFIC TAX CODE/CARRIER CABIN DATA.\n"));
    io.push_back(buildIORecord(taxCodeReg5,
                               "* APPLICABLE TO THE FOLLOWING DEFAULT CABIN INVENTORY CLASS/ES:\n"
                               "  A, Y.\n"
                               "  CARRIER/S WITH EXCEPTION DATA AC, AF, EI, ZU - REFER TO SABRE ENTRY "
                               "TXNHELP FOR SPECIFIC TAX CODE/CARRIER CABIN DATA.\n"));

    TaxDisplayTestBuilder<Category13> test;
    test.execute(io);
  }

  TaxCodeCabin* getTCabin(Indicator exceptInd = ' ',
                          std::string cos = "",
                          CarrierCode carrier = "",
                          Directionality dir = FROM,
                          LocTypeCode lt1 = ' ',
                          LocCode lc1 = "",
                          LocTypeCode lt2 = ' ',
                          LocCode lc2 = "",
                          FlightNumber fl1 = 0,
                          FlightNumber fl2 = 0)
  {
    TaxCodeCabin* ret = new TaxCodeCabin;
    ret->exceptInd() = exceptInd;
    ret->carrier() = carrier;
    ret->directionalInd() = dir;
    ret->loc1().locType() = lt1;
    ret->loc1().loc() = lc1;
    ret->loc2().locType() = lt2;
    ret->loc2().loc() = lc2;
    ret->classOfService() = cos;
    ret->flight1() = fl1;
    ret->flight2() = fl2;
    return ret;
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayCAT13Test);
}
