// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "ServiceInterfaces/DefaultServices.h"
#include "TaxDisplay/Common/TaxDisplayRequest.h"
#include "TaxDisplay/EntryHelp.h"
#include "TaxDisplay/EntryNationReportingRecord.h"
#include "TaxDisplay/EntryTaxReportingRecord.h"
#include "TaxDisplay/TaxDisplay.h"
#include "test/include/CppUnitHelperMacros.h"

#include <sstream>

namespace tax
{
namespace display
{

class TaxDisplayTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxDisplayTest);

  CPPUNIT_TEST(testSetStrategy);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}
  void tearDown() {}

  void testSetStrategy()
  {
    DefaultServices services;

    TaxDisplayRequest request1;
    request1.entryType = TaxDisplayRequest::EntryType::ENTRY_HELP;
    TaxDisplay taxDisplay1(request1, services);
    taxDisplay1.setEntry();
    CPPUNIT_ASSERT(taxDisplay1._entry);
    CPPUNIT_ASSERT(typeid(*taxDisplay1._entry) == typeid(EntryHelp));

    TaxDisplayRequest request2;
    request2.entryType = TaxDisplayRequest::EntryType::REPORTINGRECORD_ENTRY_BY_NATION;
    TaxDisplay taxDisplay2(request2, services);
    request2.nationCode = "ZZ";
    taxDisplay2.setEntry();
    CPPUNIT_ASSERT(taxDisplay2._entry);
    CPPUNIT_ASSERT(typeid(*taxDisplay2._entry) == typeid(EntryNationReportingRecord));

    TaxDisplayRequest request3;
    request3.entryType = TaxDisplayRequest::EntryType::REPORTINGRECORD_ENTRY_BY_NATION;
    TaxDisplay taxDisplay3(request3, services);
    request3.nationName = "NATIONNAME";
    taxDisplay3.setEntry();
    CPPUNIT_ASSERT(taxDisplay3._entry);
    CPPUNIT_ASSERT(typeid(*taxDisplay3._entry) == typeid(EntryNationReportingRecord));

    TaxDisplayRequest request4;
    request4.entryType = TaxDisplayRequest::EntryType::REPORTINGRECORD_ENTRY_BY_NATION;
    TaxDisplay taxDisplay4(request4, services);
    request4.airportCode = "ASD";
    taxDisplay4.setEntry();
    CPPUNIT_ASSERT(taxDisplay4._entry);
    CPPUNIT_ASSERT(typeid(*taxDisplay4._entry) == typeid(EntryNationReportingRecord));

    TaxDisplayRequest request5;
    request5.entryType = TaxDisplayRequest::EntryType::REPORTINGRECORD_ENTRY_BY_TAX;
    TaxDisplay taxDisplay5(request5, services);
    request5.taxCode = "AA";
    taxDisplay5.setEntry();
    CPPUNIT_ASSERT(taxDisplay5._entry);
    CPPUNIT_ASSERT(typeid(*taxDisplay5._entry) == typeid(EntryTaxReportingRecord));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxDisplayTest);
} /* namespace display */
} /* namespace tax */
