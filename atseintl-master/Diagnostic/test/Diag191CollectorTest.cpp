//-----------------------------------------------------------------------------
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "DataModel/TicketingCxrTrx.h"
#include "Diagnostic/Diag191Collector.h"

using namespace std;

namespace tse
{

class Diag191CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag191CollectorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testDisplayInterlineAgreements_Success);
  CPPUNIT_TEST(testDisplayInterlineAgreements_Failure);

  CPPUNIT_TEST_SUITE_END();

private:
  Diag191Collector* _diag191Collector;
  Diagnostic* _diagroot;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    try
    {
      _diag191Collector = _memHandle(new Diag191Collector);
      _diag191Collector->activate();
    }
    catch (...) { CPPUNIT_ASSERT(false); }
  }

  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try
    {
      std::string expected = "*** START OF VALIDATING CXR DIAG ***\n";
      expected += "*** END OF VALIDATING CXR DIAG ***\n";

      TicketingCxrTrx trx;
      *_diag191Collector << trx;

      CPPUNIT_ASSERT_EQUAL(expected, _diag191Collector->str());
    }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  void testDisplayInterlineAgreements_Success()
  {
    std::stringstream expectedDiag;
    expectedDiag << "\n  INTERLINE AGREEMENT INFO FOR VC:" << std::endl << "  PARTICIPATES IN ITIN: Y"
                 << std::endl << "  TICKET TYPE: ETKTPREF" << std::endl
                 << "  REQUESTED TICKET TYPE: PTKTPREF" << std::endl
                 << "  PARTICIPATING CARRIER: P1  AGREEMENT: PPR" << std::endl
                 << "  PARTICIPATING CARRIER: P2  AGREEMENT: STD" << std::endl
                 << "  PARTICIPATING CARRIER: P3  AGREEMENT: 3PT" << std::endl << std::endl;

    const CarrierCode validatingCarrier = "VC";
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    vcx::ParticipatingCxr pcx1("P1", vcx::PAPER_ONLY);
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("P2", vcx::STANDARD);
    vcxrData.participatingCxrs.push_back(pcx2);
    vcx::ParticipatingCxr pcx3("P3", vcx::THIRD_PARTY);
    vcxrData.participatingCxrs.push_back(pcx3);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::PAPER_TKT_PREF;
    const std::string failureText;

    _diag191Collector->displayInterlineAgreements(
        validatingCarrier, vcxrData, isVcxrPartOfItin, requestedTicketType, failureText);

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _diag191Collector->str());
  }

  void testDisplayInterlineAgreements_Failure()
  {
    std::stringstream expectedDiag;
    expectedDiag << "\n  INTERLINE AGREEMENT INFO FOR VC:" << std::endl << "  PARTICIPATES IN ITIN: Y"
                 << std::endl << "  TICKET TYPE: ETKTPREF" << std::endl
                 << "  REQUESTED TICKET TYPE: PTKTPREF" << std::endl
                 << "  PARTICIPATING CARRIER: P1  AGREEMENT: PPR" << std::endl
                 << "  PARTICIPATING CARRIER: P2  AGREEMENT: STD" << std::endl
                 << "  PARTICIPATING CARRIER: P3  AGREEMENT: 3PT" << std::endl
                 << "  SOME FAILURE TEXT" << std::endl << std::endl;

    const CarrierCode validatingCarrier = "VC";
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::ETKT_PREF;
    vcx::ParticipatingCxr pcx1("P1", vcx::PAPER_ONLY);
    vcxrData.participatingCxrs.push_back(pcx1);
    vcx::ParticipatingCxr pcx2("P2", vcx::STANDARD);
    vcxrData.participatingCxrs.push_back(pcx2);
    vcx::ParticipatingCxr pcx3("P3", vcx::THIRD_PARTY);
    vcxrData.participatingCxrs.push_back(pcx3);
    const bool isVcxrPartOfItin = true;
    const vcx::TicketType requestedTicketType = vcx::PAPER_TKT_PREF;
    const std::string failureText = "SOME FAILURE TEXT";

    _diag191Collector->displayInterlineAgreements(
        validatingCarrier, vcxrData, isVcxrPartOfItin, requestedTicketType, failureText);

    CPPUNIT_ASSERT_EQUAL(expectedDiag.str(), _diag191Collector->str());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Diag191CollectorTest);
}
