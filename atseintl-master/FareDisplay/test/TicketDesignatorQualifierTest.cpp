#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "FareDisplay/TicketDesignatorQualifier.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayOptions.h"
#include "Rules/RuleConst.h"

namespace tse
{
class TicketDesignatorQualifierTest : public CppUnit::TestFixture
{
  class PaxTypeFareMock : public PaxTypeFare
  {
  public:
    PaxTypeFareMock() {}
    std::string createFareBasisCodeFD(FareDisplayTrx& trx) const { return _fareBasisCode; }

    std::string _fareBasisCode;
  };

  CPPUNIT_TEST_SUITE(TicketDesignatorQualifierTest);
  CPPUNIT_TEST(testQualifyTicketDesignator);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testQualifyTicketDesignator()
  {
    FareDisplayTrx fdTrx;
    PaxTypeFareMock ptFare;
    FareDisplayRequest request;
    TicketDesignatorQualifier desq;

    TktDesignator tk;
    TktDesignator tkt21 = "TKT21";
    TktDesignator tkt = "ATS06";

    fdTrx.setRequest(&request);

    {
      request.ticketDesignator() = tk;
      const tse::PaxTypeFare::FareDisplayState ret = desq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }
    {
      request.ticketDesignator() = tkt;
      ptFare._fareBasisCode = "C25ATS6/ATS06";
      const tse::PaxTypeFare::FareDisplayState ret = desq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }
    {
      request.ticketDesignator() = tkt21;
      ptFare._fareBasisCode = "C25ATS6/ATS06";
      const tse::PaxTypeFare::FareDisplayState ret = desq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_TicketDesignator);
    }
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(TicketDesignatorQualifierTest);
}
