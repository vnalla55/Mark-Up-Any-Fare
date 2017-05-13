#include <boost/assign/list_of.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/PricingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

DiagnosticTypes diagType = AllFareDiagnostic; // Diag 200

namespace
{

typedef TseCallableTrxTask::SequenceID TaskID;

class DiagnosticMock : public Diagnostic
{
public:
  DiagnosticMock(DiagnosticTypes diagType) : Diagnostic(diagType)
  {
  }
  DiagnosticMock& stubCurrentTaskID(TaskID id)
  {
    _taskId = id;
    return *this;
  }

protected:
  /**
   * @override
   */
  TaskID getCurrentTaskSequenceID() { return _taskId; }

private:
  TaskID _taskId;
};

} // anon ns

class DiagnosticTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DiagnosticTest);
  CPPUNIT_TEST(testDiagnostic);
  CPPUNIT_TEST(testSortDiagMsgs);
  CPPUNIT_TEST(testNormDiagParam);
  CPPUNIT_TEST(testAppendAdditionalDataSection_RunTwice);
  CPPUNIT_TEST(testAppendAdditionalDataSection_WithNullInside);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _trx = _memHandle(new PricingTrx);
    DCFactory::instance();
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown() { _memHandle.clear(); }

  void testDiagnostic()
  {
    RootLoggerGetOff();

    Diagnostic& rootDiag = _trx->diagnostic();
    rootDiag.diagnosticType() = diagType;
    rootDiag.activate();

    // get a pointer to the diag factory
    DCFactory* factory = DCFactory::instance();
    DiagCollector& diag = *(factory->create(*_trx));

    // enable and collect diag messages
    // following info will be collected for Diag 200, 201
    diag.enable(AllFareDiagnostic, Diagnostic201);
    diag << "Message 01 " << 200.01f << '\n';
    diag << "Message 02 " << 200.02f << '\n';
    diag.flushMsg();
    diag.disable(AllFareDiagnostic, Diagnostic201);

    const std::string expect = "Message 01 200.01\nMessage 02 200.02\n";
    CPPUNIT_ASSERT_EQUAL(expect, rootDiag.toString());
  }

  void testSortDiagMsgs()
  {
    using boost::assign::list_of;
    RootLoggerGetOff();

    DiagnosticMock rootDiag(diagType);
    rootDiag.diagParamMap()["NORM_DIAG"] = "Y";
    rootDiag.activate();

    // diag output from trx thread
    rootDiag.stubCurrentTaskID(TaskID::EMPTY).insertDiagMsg("*** Message 01 ***\n");

    // concurrent task 1 & task 2 mixed output to diag
    rootDiag.stubCurrentTaskID(list_of(2)).insertDiagMsg("*** Task 2 message ***\n");
    rootDiag.stubCurrentTaskID(list_of(1)).insertDiagMsg("*** Task 1 message ***\n");
    rootDiag.stubCurrentTaskID(list_of(1)(2)).insertDiagMsg("*** Task 1.2 message ***\n");
    rootDiag.stubCurrentTaskID(list_of(2)(1)).insertDiagMsg("*** Task 2.1 message ***\n");
    rootDiag.stubCurrentTaskID(list_of(1)(1)).insertDiagMsg("*** Task 1.1 message ***\n");

    // diag output from trx thread, it is supposed be used as a barrier to don't
    //  glue trx thread output together
    rootDiag.stubCurrentTaskID(TaskID::EMPTY).insertDiagMsg("*** Message 02 ***\n");
    rootDiag.stubCurrentTaskID(TaskID::EMPTY).insertDiagMsg("*** Message 03 ***\n");

    // another task concurrent threads mixed output
    rootDiag.stubCurrentTaskID(list_of(3)(2)).insertDiagMsg("*** Task 3.2 message ***\n");
    rootDiag.stubCurrentTaskID(list_of(3)(1)).insertDiagMsg("*** Task 3.1 message ***\n");

    const std::string expect = "*** Message 01 ***\n"
                               "*** Task 1 message ***\n"
                               "*** Task 1.1 message ***\n"
                               "*** Task 1.2 message ***\n"
                               "*** Task 2 message ***\n"
                               "*** Task 2.1 message ***\n"
                               "*** Message 02 ***\n"
                               "*** Message 03 ***\n"
                               "*** Task 3.1 message ***\n"
                               "*** Task 3.2 message ***\n";
    CPPUNIT_ASSERT_EQUAL(expect, rootDiag.toString());
  }

  void testNormDiagParam()
  {
    using boost::assign::list_of;
    RootLoggerGetOff();

    DiagnosticMock rootDiag(diagType);
    // NORM_DIAG is off:
    // rootDiag.diagParamMap()["NORM_DIAG"] = "Y";
    rootDiag.activate();

    // diag output from trx thread
    rootDiag.stubCurrentTaskID(TaskID::EMPTY).insertDiagMsg("*** Message 01 ***\n");

    // concurrent task 1 & task 2 mixed output to diag
    rootDiag.stubCurrentTaskID(list_of(2)).insertDiagMsg("*** Task 2 message ***\n");
    rootDiag.stubCurrentTaskID(list_of(1)).insertDiagMsg("*** Task 1 message ***\n");

    const std::string expect = "*** Message 01 ***\n"
                               "*** Task 2 message ***\n"
                               "*** Task 1 message ***\n";
    CPPUNIT_ASSERT_EQUAL(expect, rootDiag.toString());
  }

  void testAppendAdditionalDataSection_RunTwice()
  {
    Diagnostic testedClass(diagType);

    std::string input_for_first_call = "<NothingImportant>";
    std::string input_for_second_call = "<ReallyNothing>";
    std::string expectedOutput = "<![CDATA[<NothingImportant>]]>\n<![CDATA[<ReallyNothing>]]>\n";

    testedClass.appendAdditionalDataSection(input_for_first_call);
    testedClass.appendAdditionalDataSection(input_for_second_call);
    CPPUNIT_ASSERT_EQUAL(expectedOutput, testedClass.getAdditionalData());
  }

  void testAppendAdditionalDataSection_WithNullInside()
  {
    Diagnostic testedClass(diagType);

    std::string input = "<Nothing";
    input += '\0';
    input += "Important>";
    input += '\0';
    std::string expectedOutput = "<![CDATA[<Nothing\\0Important>\\0]]>\n";

    testedClass.appendAdditionalDataSection(input);
    CPPUNIT_ASSERT_EQUAL(expectedOutput, testedClass.getAdditionalData());
  }

private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
};

CPPUNIT_TEST_SUITE_REGISTRATION(DiagnosticTest);

} // ns tse
