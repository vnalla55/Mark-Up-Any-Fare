#include "test/include/CppUnitHelperMacros.h"
#include <sstream>

#include "DBAccess/MinFareRuleLevelExcl.h"
#include "DBAccess/Queries/QueryGetMinFareRule.h"
#include "DBAccess/Queries/QueryGetMinFareRuleSQLStatement.h"
#include "DBAccess/test/QueryTester.h"
#include "DBAccess/test/GlobalTseTestUtils.h"
#include "DBAccess/test/TestRow.h"

#include "test/include/TimeBomb.h"

using namespace std;
using tseTestUtils::tic;

namespace tse
{
class QueryGetMinFareRuleTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(QueryGetMinFareRuleTest);
  CPPUNIT_TEST(testGetFareSetNoRowsReturned);
  CPPUNIT_TEST(testGetFareSetOneRowReturned);
  CPPUNIT_TEST(testGetFareSetMultipleRowsReturned);
  CPPUNIT_TEST(testGetFareTypesNoRowsReturned);
  CPPUNIT_TEST_SUITE_END();

public:
  DBAdapter dbAdapter;
  QueryTester* tester;

  void setUp()
  {
    tester = new QueryTester(dbAdapter);
    SQLQueryInitializer::initAllQueryClasses(false);
  }

  void tearDown() { delete tester; }

  void testGetFareSetNoRowsReturned()
  {
    QueryGetMinFareRuleSameFareGroupChild query(&dbAdapter);
    query.initialize();
    MinFareRuleLevelExcl ruleLevelExcl;
    populateWithQueryData(ruleLevelExcl);

    query.getFareSet(&ruleLevelExcl);

    stringstream expectedSql;
    expectedSql << "select SETNO,FAREGROUPRULETARIFF,RULE,FARETYPE,FARECLASS "
                << "from EXCLBYRULESAMEGRPFAREQUAL " << where(ruleLevelExcl) << " order by SETNO";

    CPPUNIT_ASSERT_EQUAL(expectedSql.str(), tester->lastQuery());

    CPPUNIT_ASSERT(ruleLevelExcl.fareSet().empty());
    CPPUNIT_ASSERT(ruleLevelExcl.sameFareGroupFareTypes().empty());
    CPPUNIT_ASSERT(ruleLevelExcl.sameFareGroupTariff().empty());
    CPPUNIT_ASSERT(ruleLevelExcl.sameFareGroupFareClasses().empty());
    CPPUNIT_ASSERT(ruleLevelExcl.sameFareGroupRules().empty());
  }

  void testGetFareSetOneRowReturned()
  {
    short setNo(99);
    int fareGroupRuleTariff(18);
    string fareType("X");
    string fareClass("Y26");
    RuleNumber rule("12");

    TestRow row;
    addResultRow(row, setNo, fareGroupRuleTariff, fareClass, rule, fareType);

    QueryGetMinFareRuleSameFareGroupChild query(&dbAdapter);
    query.initialize();

    MinFareRuleLevelExcl ruleLevelExcl;
    populateWithQueryData(ruleLevelExcl);

    query.getFareSet(&ruleLevelExcl);

    assertRow(ruleLevelExcl, 0, setNo, fareType, fareGroupRuleTariff, fareClass, rule);
  }

  void testGetFareSetMultipleRowsReturned()
  {
    short setNo(99), setNo2(100);
    int fareGroupRuleTariff(18), fareGroupRuleTariff2(19);
    string fareType("X"), fareType2("Y");
    string fareClass("Y26"), fareClass2("Y27");
    RuleNumber rule("12"), rule2("13");

    TestRow row, row2;
    addResultRow(row, setNo, fareGroupRuleTariff, fareClass, rule, fareType);
    addResultRow(row2, setNo2, fareGroupRuleTariff2, fareClass2, rule2, fareType2);

    QueryGetMinFareRuleSameFareGroupChild query(&dbAdapter);
    query.initialize();

    MinFareRuleLevelExcl ruleLevelExcl;
    populateWithQueryData(ruleLevelExcl);

    query.getFareSet(&ruleLevelExcl);

    assertRow(ruleLevelExcl, 0, setNo, fareType, fareGroupRuleTariff, fareClass, rule);
    assertRow(ruleLevelExcl, 1, setNo2, fareType2, fareGroupRuleTariff2, fareClass2, rule2);
  }

  void testGetFareTypesNoRowsReturned()
  {
    MinFareRuleLevelExcl ruleLevelExcl;
    populateWithQueryData(ruleLevelExcl);

    QueryGetMinFareRuleFareTypesHistorical query(&dbAdapter);
    query.initialize();

    query.getFareTypes(&ruleLevelExcl);

    stringstream expectedSql;
    expectedSql << "(select FARETYPE from EXCLBYRULEFARETYPEH " << where(ruleLevelExcl) << ") "
                << "union all "
                << "(select FARETYPE from EXCLBYRULEFARETYPE " << where(ruleLevelExcl) << ")";

    CPPUNIT_ASSERT_EQUAL(expectedSql.str(), tester->lastQuery());
  }

  string where(MinFareRuleLevelExcl& ruleLevelExcl)
  {
    stringstream where;
    where << "where VENDOR = " << tic(ruleLevelExcl.vendor())
          << " and TEXTTBLITEMNO = " << ruleLevelExcl.textTblItemNo()
          << " and GOVERNINGCARRIER = " << tic(ruleLevelExcl.governingCarrier())
          << " and RULETARIFF = " << ruleLevelExcl.ruleTariff()
          << " and VERSIONDATE = " << ruleLevelExcl.versionDate().get64BitRep()
          << " and SEQNO = " << ruleLevelExcl.seqNo()
          << " and CREATEDATE = " << ruleLevelExcl.createDate().get64BitRep();
    return where.str();
  }

  void addResultRow(TestRow& row,
                    short setNo,
                    int fareGroupRuleTariff,
                    string& fareClass,
                    RuleNumber& rule,
                    string& fareType)
  {
    typedef QueryGetMinFareRuleSameFareGroupChildSQLStatement<QueryGetMinFareRuleSameFareGroupChild>
    Cols;
    row.set(Cols::SETNO, setNo);
    row.set(Cols::FAREGROUPRULETARIFF, fareGroupRuleTariff);
    row.set(Cols::FARECLASS, fareClass);
    row.set(Cols::RULE, rule);
    row.set(Cols::FARETYPE, fareType);
    tester->addRow(&row);
  }

  void assertRow(MinFareRuleLevelExcl& ruleLevelExcl,
                 int i,
                 short setNo,
                 string& fareType,
                 int fareGroupRuleTariff,
                 string& fareClass,
                 RuleNumber& rule)
  {
    CPPUNIT_ASSERT_EQUAL(setNo, ruleLevelExcl.fareSet()[i]);
    CPPUNIT_ASSERT_EQUAL(string(fareType), ruleLevelExcl.sameFareGroupFareTypes()[i]);
    CPPUNIT_ASSERT_EQUAL(fareGroupRuleTariff, ruleLevelExcl.sameFareGroupTariff()[i]);
    CPPUNIT_ASSERT_EQUAL(fareClass, ruleLevelExcl.sameFareGroupFareClasses()[i]);
    CPPUNIT_ASSERT_EQUAL(rule, ruleLevelExcl.sameFareGroupRules()[i]);
  }

  void populateWithQueryData(MinFareRuleLevelExcl& ruleLevelExcl)
  {
    ruleLevelExcl.textTblItemNo() = 10;
    ruleLevelExcl.ruleTariff() = 1;
    ruleLevelExcl.seqNo() = 100;
    ruleLevelExcl.vendor() = "A";
    ruleLevelExcl.governingCarrier() = "AA";
    ruleLevelExcl.versionDate() = DateTime(2008, 9, 9);
    ruleLevelExcl.createDate() = DateTime(2008, 9, 9);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(QueryGetMinFareRuleTest);
};
