#include "DBAccess/BoundParameterTypes.h"
#include "DBAccess/ORACLEBoundParameterTypes.h"
#include "DBAccess/ParameterSubstitutor.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

class SQLQueryStringTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SQLQueryStringTest);
  CPPUNIT_TEST(testFillParameterString);
  CPPUNIT_TEST(testSQLString);
  CPPUNIT_TEST_SUITE_END();

public:
  SQLQueryStringTest()
    : _vendor("ATP")
    , _dateTime(time(0))
    , _cat(33)
    , _seqNo(100000001)
  {
  }

  void addBoundParameters(DBAccess::ParameterSubstitutor& parameterSubstitutor)
  {
    parameterSubstitutor.addBoundParameter<DBAccess::ORACLEBoundString>(1, 10, _vendor);
    parameterSubstitutor.addBoundParameter<DBAccess::ORACLEBoundDate>(2, 20, _dateTime.date());
    parameterSubstitutor.addBoundParameter<DBAccess::ORACLEBoundInteger>(3, 30, _cat);
    parameterSubstitutor.addBoundParameter<DBAccess::ORACLEBoundLong>(4, 40, _seqNo);
    parameterSubstitutor.addBoundParameter<DBAccess::ORACLEBoundDateTime>(5, 50, _dateTime);
  }

  void checkResult(const std::string& orig,
                   const std::string& result)
  {
    //std::cerr << "orig - " << orig << "\nresult: - " << result << std::endl;
    size_t pos(result.find(_vendor));
    CPPUNIT_ASSERT(pos != std::string::npos);
    std::ostringstream oss;
    oss << _cat;
    pos = result.find(oss.str());
    CPPUNIT_ASSERT(pos != std::string::npos);
    oss.str("");
    oss << _seqNo;
    pos = result.find(oss.str());
    CPPUNIT_ASSERT(pos != std::string::npos);
  }

  void testFillParameterString()
  {
    std::string parameterString;
    DBAccess::ParameterSubstitutor parameterSubstitutor;
    addBoundParameters(parameterSubstitutor);
    parameterSubstitutor.fillParameterString(parameterString);
    //std::cerr << "parameterString:" << parameterString << std::endl;
    size_t pos(parameterString.find(_vendor));
    CPPUNIT_ASSERT(pos != std::string::npos);
    std::ostringstream oss;
    oss << _cat;
    pos = parameterString.find(oss.str());
    CPPUNIT_ASSERT(pos != std::string::npos);
    oss.str("");
    oss << _seqNo;
    pos = parameterString.find(oss.str());
    CPPUNIT_ASSERT(pos != std::string::npos);
  }

  void testSQLString()
  {
    std::string origQueryString;
    DBAccess::ParameterSubstitutor parameterSubstitutor;
    addBoundParameters(parameterSubstitutor);
    const std::string orig1("dummy1 :1 dummy2 :2 dummy3 :3 dummy4 :4 dummy5 :5");
    std::string result1;
    parameterSubstitutor.getSQLString(orig1, result1);
    checkResult(orig1, result1);
    const std::string orig2("dummy1 :1 dummy2 :2 dummy3 :3 dummy4 :4 dummy5 :5 dummy6");
    std::string result2;
    parameterSubstitutor.getSQLString(orig2, result2);
    checkResult(orig2, result2);
  }

private:
  const std::string _vendor;
  const DateTime _dateTime;
  const int _cat;
  const long _seqNo;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SQLQueryStringTest);

}
