//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include <sstream>
#include "Common/TseCodeTypes.h"
#include "DBAccess/ServiceFeesCxrActivation.h"
#include "DBAccess/Queries/QueryGetServiceFeesCxrActivation.h"
#include "DBAccess/Queries/QueryGetServiceFeesCxrActivationSQLStatement.h"
#include "DBAccess/test/QueryTester.h"
#include "DBAccess/test/GlobalTseTestUtils.h"
#include "DBAccess/test/TestRow.h"
#include "test/include/TestMemHandle.h"

using namespace std;
using tseTestUtils::tic;

namespace tse
{
class QueryGetServiceFeesCxrActivationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(QueryGetServiceFeesCxrActivationTest);
  CPPUNIT_TEST(testFindServiceFeesCxrActivationNoRowsReturned);
  CPPUNIT_TEST(testFindServiceFeesCxrActivationOneRowReturned);
  CPPUNIT_TEST(testFindServiceFeesCxrActivationTwoRowsReturned);
  CPPUNIT_TEST(testFindServiceFeesCxrActivationNoRowsReturnedHistorical);
  CPPUNIT_TEST_SUITE_END();

public:
  DBAdapter dbAdapter;
  QueryTester* tester;
  TestMemHandle _memHandle;

  void setUp()
  {
    tester = _memHandle.insert<QueryTester>(new QueryTester(dbAdapter));
    SQLQueryInitializer::initAllQueryClasses(false);
  }

  void tearDown()
  {
    // delete tester;
  }

  void testFindServiceFeesCxrActivationNoRowsReturned()
  {
    QueryGetServiceFeesCxrActivation query(&dbAdapter);
    query.initialize();
    query.resetSQL();
    vector<tse::ServiceFeesCxrActivation*> svcCxrVec;
    CarrierCode carrier = "AA";
    DateTime todaysDate = DateTime::localTime();
    query.findServiceFeesCxrActivation(svcCxrVec, carrier);

    stringstream expectedSql;
    expectedSql << "select CARRIER,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE"
                << " from SERVICEFEESCXRACTIVATION"
                << " where CARRIER= '" << carrier << "'"
                << " and " << todaysDate.get64BitRepDateOnly() << " <= EXPIREDATE";

    CPPUNIT_ASSERT_EQUAL(expectedSql.str(), tester->lastQuery());
    CPPUNIT_ASSERT(svcCxrVec.empty());
  }

  void testFindServiceFeesCxrActivationOneRowReturned()
  {
    CarrierCode carrier("AA");
    TestRow row;
    addResultRow(row);
    QueryGetServiceFeesCxrActivation query(&dbAdapter);
    query.initialize();
    query.resetSQL();
    vector<tse::ServiceFeesCxrActivation*> svcCxrVec;
    query.findServiceFeesCxrActivation(svcCxrVec, carrier);

    CPPUNIT_ASSERT(!svcCxrVec.empty());
    CPPUNIT_ASSERT_EQUAL(carrier, svcCxrVec[0]->carrier());
  }

  void testFindServiceFeesCxrActivationTwoRowsReturned()
  {
    CarrierCode carrier("AA");
    TestRow row1, row2;
    addResultRow(row1);
    addResultRow(row2);
    QueryGetServiceFeesCxrActivation query(&dbAdapter);
    query.initialize();
    query.resetSQL();
    vector<tse::ServiceFeesCxrActivation*> svcCxrVec;
    query.findServiceFeesCxrActivation(svcCxrVec, carrier);

    CPPUNIT_ASSERT_EQUAL((size_t)2, svcCxrVec.size());
  }

  void testFindServiceFeesCxrActivationNoRowsReturnedHistorical()
  {
    QueryGetServiceFeesCxrActivationHistorical query(&dbAdapter);
    query.initialize();
    query.resetSQL();
    vector<tse::ServiceFeesCxrActivation*> svcCxrVec;
    CarrierCode carrier = "AA";
    DateTime todaysDate = DateTime::localTime();
    query.findServiceFeesCxrActivation(svcCxrVec, carrier, todaysDate, todaysDate);

    stringstream expectedSql;
    expectedSql << "select CARRIER,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE"
                << " from SERVICEFEESCXRACTIVATION"
                << " where CARRIER= '" << carrier << "' and " << todaysDate.get64BitRep()
                << " <= EXPIREDATE and " << todaysDate.get64BitRep() << " >= CREATEDATE";

    CPPUNIT_ASSERT_EQUAL(expectedSql.str(), tester->lastQuery());
    CPPUNIT_ASSERT(svcCxrVec.empty());
  }

  void addResultRow(TestRow& row)
  {
    CarrierCode carrier("AA");
    DateTime todaysDate = DateTime::localTime();
    typedef QueryGetServiceFeesCxrActivationSQLStatement<QueryGetServiceFeesCxrActivation> Cols;
    row.set(Cols::CARRIER, carrier);
    row.set(Cols::CREATEDATE, todaysDate);
    row.set(Cols::EXPIREDATE, todaysDate);
    row.set(Cols::EFFDATE, todaysDate);
    row.set(Cols::DISCDATE, todaysDate);
    tester->addRow(&row);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(QueryGetServiceFeesCxrActivationTest);
};
