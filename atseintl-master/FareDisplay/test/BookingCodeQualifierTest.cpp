#include "test/include/CppUnitHelperMacros.h"
#include "FareDisplay/BookingCodeQualifier.h"
#include "Common/TseCodeTypes.h"
#include "Common/Config/ConfigMan.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayOptions.h"
#include "Rules/RuleConst.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockGlobal.h"

namespace tse
{

class BookingCodeQualifierTest : public CppUnit::TestFixture
{
  class BookingCodeQualifierFD : public BookingCodeQualifier
  {
  public:
    BookingCodeQualifierFD() {}

  protected:
    virtual void getBookingCode(FareDisplayTrx& trx, const PaxTypeFare& paxTypeFare) const
    {
      (const_cast<PaxTypeFare&>(paxTypeFare)).bookingCode() = "YN";
    }
  };

  class PaxTypeFareMock : public PaxTypeFare
  {
  public:
    PaxTypeFareMock() : _havePrimaryBookingCodes(false) {}
    bool getPrimeBookingCode(std::vector<BookingCode>& bookingCodeVec) const
    {
      bookingCodeVec.clear();
      if (_havePrimaryBookingCodes)
      {
        bookingCodeVec.push_back("AB");
        bookingCodeVec.push_back("RK");
        bookingCodeVec.push_back("YN");
        bookingCodeVec.push_back("CJ");
      }
      return (!bookingCodeVec.empty());
    }

    FBRPaxTypeFareRuleData* getFbrRuleData() const
    {
      FBRPaxTypeFareRuleData* ret = new FBRPaxTypeFareRuleData();
      std::vector<BookingCode> bCodeVec;
      bCodeVec.push_back("AB");
      bCodeVec.push_back("RK");
      bCodeVec.push_back("YN");
      bCodeVec.push_back("CJ");
      ret->setBaseFarePrimeBookingCode(bCodeVec);
      return ret;
    }

    bool _havePrimaryBookingCodes;
  };

  CPPUNIT_TEST_SUITE(BookingCodeQualifierTest);
  CPPUNIT_TEST(testQualifyBookingCode);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
  }

  void tearDown() { _memHandle.clear(); }

  void testQualifyBookingCode()
  {
    FareDisplayTrx fdTrx;
    FareDisplayRequest request;
    PaxTypeFareMock ptFare;
    FareInfo fareInfo;
    Fare fare;
    Fare fareFD;
    FareMarket fm;
    TariffCrossRefInfo tcrInfo;
    FareDisplayOptions options;
    FareClassAppSegInfo fcasi;
    BookingCodeQualifierFD fq;
    AirSeg seg1;
    FBRPaxTypeFareRuleData fbrData;

    fdTrx.setRequest(&request);
    fdTrx.setOptions(&options);

    fare.initialize(Fare::FS_Domestic, &fareInfo, fm, &tcrInfo);
    ptFare.setFare(&fare);
    ptFare.fareClassAppSegInfo() = &fcasi;
    ptFare.fareMarket() = &fm;

    {
      request.bookingCode() = "";
      request.fareBasisCode() = "";
      const tse::PaxTypeFare::FareDisplayState ret = fq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }
    {
      request.bookingCode() = "Y";
      request.fareBasisCode() = "M";
      ptFare.setFare(&fareFD);
      const tse::PaxTypeFare::FareDisplayState ret = fq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_Valid);
    }
    {
      request.bookingCode() = "Z";
      request.fareBasisCode() = "M";
      ptFare.setFare(&fareFD);
      const tse::PaxTypeFare::FareDisplayState ret = fq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_BookingCode);
    }
    {
      request.bookingCode() = "CJ";
      request.fareBasisCode() = "M";
      ptFare.setFare(&fareFD);
      ptFare._havePrimaryBookingCodes = true;
      const tse::PaxTypeFare::FareDisplayState ret = fq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_BookingCode);
    }
    {
      request.bookingCode() = "CJ";
      request.fareBasisCode() = "M";
      ptFare.setFare(&fareFD);
      ptFare._havePrimaryBookingCodes = false;
      ptFare.status().set(PaxTypeFare::PTF_FareByRule);
      const tse::PaxTypeFare::FareDisplayState ret = fq.qualify(fdTrx, ptFare);
      CPPUNIT_ASSERT(ret == PaxTypeFare::FD_BookingCode);
    }
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(BookingCodeQualifierTest);
}
