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

#include "Common/TaxName.h"
#include "DataModel/Services/ReportingRecord.h"
#include "ServiceInterfaces/DefaultServices.h"
#include "TaxDisplay/Common/TaxDisplayRequest.h"
#include "TaxDisplay/EntryTaxReportingRecord.h"
#include "TaxDisplay/Response/ResponseFormatter.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/ReportingRecordServiceMock.h"

namespace tax
{
namespace display
{

class EntryTaxReportingRecordTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(EntryTaxReportingRecordTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}
  void tearDown() {}
private:
  DefaultServices _services;
};

CPPUNIT_TEST_SUITE_REGISTRATION(EntryTaxReportingRecordTest);
} /* namespace display */
} /* namespace tax */
