#include "Common/TseUtil.h"
#include "DataModel/MileageTrx.h"
#include "Xform/MileageResponseFormatter.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

#include <string>

namespace tse
{

using namespace std;

namespace
{
class MockMileageResponseFormatter : public MileageResponseFormatter
{
public:
  void dbDiagString(std::vector<std::string>& dbMsgVec) const override
  {
    std::string dbMsg = "DB SERVER/PORT: DB SERVER";
    TseUtil::splitTextLine(dbMsg, dbMsgVec);
  }

  bool hostDiagString(std::vector<std::string>& hostMsgVec) const
  {
    std::string hostMsg = "HOSTNAME/PORT:  SERVER/11111";
    TseUtil::splitTextLine(hostMsg, hostMsgVec);
    return true;
  }

  void buildDiagString(std::vector<std::string>& buildMsgVec) const override
  {
    std::string buildMsg = "BASELINE: MANUALBUILD";
    TseUtil::splitTextLine(buildMsg, buildMsgVec);
  }
};
}

class MileageResponseFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MileageResponseFormatterTest);
  CPPUNIT_TEST(testUpdateDiagResponseText_DiagnosticNone);
  CPPUNIT_TEST(testUpdateDiagResponseText_DiagnosticStringEmpty);
  CPPUNIT_TEST(testUpdateDiagResponseText_DiagnosticStringNotEmpty);
  CPPUNIT_TEST(testUpdateDiagResponseText_Diagnostic854);
  CPPUNIT_TEST(testUpdateDiagResponseText_Diagnostic999);

  CPPUNIT_TEST(testFormatResponseText_Regular);
  CPPUNIT_TEST(testFormatResponseText_EmptyDiagnostic);
  CPPUNIT_TEST(testFormatResponseText_Diag854);
  CPPUNIT_TEST_SUITE_END();

  // data
private:
  MileageResponseFormatter* _formatter;
  MileageTrx* _trx;
  TestMemHandle _memHandle;

  // helper methods
public:
  void setUp()
  {
    _formatter = _memHandle.insert(new MockMileageResponseFormatter);
    _trx = _memHandle.insert(new MileageTrx);
    _trx->setRequest(_memHandle.insert(new PricingRequest));
    _trx->response() << "RESPONSE";
  }

  void tearDown() { _memHandle.clear(); }

  void setDiagnostic452(const string& msg = string(""))
  {
    _trx->diagnostic().diagnosticType() = Diagnostic452;
    _trx->getRequest()->diagnosticNumber() = 452;
    _trx->diagnostic().activate();
    if (!msg.empty())
      _trx->diagnostic().insertDiagMsg(msg);
  }

  // tests
public:
  void testUpdateDiagResponseText_DiagnosticNone()
  {
    string response;
    _formatter->updateDiagResponseText(*_trx, response);
    CPPUNIT_ASSERT_EQUAL(string(""), response);
  }

  void testUpdateDiagResponseText_DiagnosticStringEmpty()
  {
    setDiagnostic452();
    string response;
    _formatter->updateDiagResponseText(*_trx, response);
    CPPUNIT_ASSERT_EQUAL(string("DIAGNOSTIC 452 RETURNED NO DATA"), response);
  }

  void testUpdateDiagResponseText_DiagnosticStringNotEmpty()
  {
    setDiagnostic452("some message");
    string response;
    _formatter->updateDiagResponseText(*_trx, response);
    CPPUNIT_ASSERT_EQUAL(string("some message"), response);
  }

  void testUpdateDiagResponseText_Diagnostic854()
  {
    string response;
    _trx->diagnostic().diagnosticType() = Diagnostic854;
    _formatter->updateDiagResponseText(*_trx, response);
    CPPUNIT_ASSERT_EQUAL(string(""), response);
  }

  void testUpdateDiagResponseText_Diagnostic999()
  {
    string response;
    string result = "***************************************************************\n"
                    "MILEAGE DIAGNOSTICS MASTER LIST\n"
                    "***************************************************************\n"
                    "ADD CROSS OF LORRAINE Q/* BEFORE EACH MILEAGE DIAGNOSTIC.\n"
                    "ENTER A SLASH AFTER THE NUMBER TO ADD APPLICABLE QUALIFIERS\n"
                    "  /FM                SPECIFIC FARE MARKET\n"
                    "  /SQ                SEQUENCE NUMBER\n"
                    " \n"
                    " \n"
                    "196                  MILEAGE XML REQUEST TO ATSE\n"
                    "197                  MILEAGE XML RESPONSE FROM ATSE\n"
                    "452                  TPM EXCLUSION\n"
                    "854                  DATABASE SERVER AND PORT NUMBER\n"
                    "***************************************************************\n";

    _trx->diagnostic().diagnosticType() = UpperBound;
    _formatter->updateDiagResponseText(*_trx, response);
    CPPUNIT_ASSERT_EQUAL(result, response);
  }

  void testFormatResponseText_Regular()
  {
    string result = "<MileageResponse>"
                    "<MSG N06=\"X\" Q0K=\"000003\" S18=\"RESPONSE\"/>"
                    "</MileageResponse>";
    CPPUNIT_ASSERT_EQUAL(result, _formatter->formatResponse(*_trx));
  }

  void testFormatResponseText_EmptyDiagnostic()
  {
    setDiagnostic452();
    string result = "<MileageResponse>"
                    "<MSG N06=\"X\" Q0K=\"000003\" S18=\"DIAGNOSTIC 452 RETURNED NO DATA\"/>"
                    "</MileageResponse>";
    CPPUNIT_ASSERT_EQUAL(result, _formatter->formatResponse(*_trx));
  }

  void testFormatResponseText_Diag854()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic854;
    string result = "<MileageResponse>"
                    "<MSG N06=\"X\" Q0K=\"000002\" S18=\"HOSTNAME/PORT:  SERVER/11111\"/>"
                    "<MSG N06=\"X\" Q0K=\"000003\" S18=\"BASELINE: MANUALBUILD\"/>"
                    "<MSG N06=\"X\" Q0K=\"000004\" S18=\"DB SERVER/PORT: DB SERVER\"/>"
                    "<MSG N06=\"X\" Q0K=\"000006\" S18=\"RESPONSE\"/>"
                    "</MileageResponse>";
    CPPUNIT_ASSERT_EQUAL(result, _formatter->formatResponse(*_trx));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MileageResponseFormatterTest);
} // namespace tse
