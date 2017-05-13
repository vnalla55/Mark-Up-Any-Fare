#include "Pricing/PaxFarePathFactory.h"
#include "DBAccess/Customer.h"
#include "Pricing/test/FactoriesConfigStub.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class PaxFarePathFactoryTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PaxFarePathFactoryTest);
  CPPUNIT_TEST(testIsValidForIntegratedEmpty);
  CPPUNIT_TEST(testIsValidForIntegratedAllNoMatches);
  CPPUNIT_TEST(testIsValidForIntegratedValid);
  CPPUNIT_TEST(testIsValidForIntegratedNotValid);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _factory = _memHandle.create<PaxFarePathFactory>(_factoriesConfig); }

  void tearDown() { _memHandle.clear(); }

  void testIsValidForIntegratedEmpty()
  {
    FarePath fp;
    NoPNRPricingTrx trx;
    PricingRequest pr;
    Agent agent;

    pr.ticketingAgent() = &agent;
    trx.setRequest(&pr);

    Itin itin;
    trx.itin().push_back(&itin);
    _factory->trx() = &trx;

    CPPUNIT_ASSERT(_factory->isValidForIntegrated(&fp));
  }

  void testIsValidForIntegratedAllNoMatches()
  {
    FarePath fp;
    NoPNRPricingTrx trx;
    PricingRequest pr;
    Agent agent;
    Customer customer;

    customer.crsCarrier() = "1B";
    customer.hostName() = "ABAC";

    agent.agentTJR() = &customer;
    pr.ticketingAgent() = &agent;
    trx.setRequest(&pr);

    Itin itin;
    trx.itin().push_back(&itin);
    _factory->trx() = &trx;

    FarePath fp1, fp2, fp3;
    fp1.noMatchOption() = true;
    fp2.noMatchOption() = true;
    fp3.noMatchOption() = true;

    itin.farePath().push_back(&fp1);
    itin.farePath().push_back(&fp2);
    itin.farePath().push_back(&fp3);

    CPPUNIT_ASSERT(!_factory->isValidForIntegrated(&fp));
  }

  void testIsValidForIntegratedValid()
  {
    FarePath noMatchFarePath, matchFarePath;

    PaxType paxType;
    paxType.paxType() = "ADT";

    noMatchFarePath.paxType() = &paxType;
    matchFarePath.paxType() = &paxType;

    // no-match
    FareClassAppSegInfo info;
    info._bookingCode[0] = "Y";

    PaxTypeFare fare;
    (FareClassAppSegInfo*&)(fare.fareClassAppSegInfo()) = &info;

    FareInfo fareInfo;
    fareInfo.fareClass() = "YOW";
    fare.fare()->setFareInfo(&fareInfo);

    FareUsage fu;
    fu.paxTypeFare() = &fare;
    PricingUnit pu;
    pu.fareUsage().push_back(&fu);

    noMatchFarePath.pricingUnit().push_back(&pu);
    noMatchFarePath.noMatchOption() = true;

    // match
    PaxTypeFare fare1;
    FareInfo fareInfo1;
    fareInfo1.fareClass() = "YOW";
    fare1.fare()->setFareInfo(&fareInfo1);

    PricingUnit pu1;
    FareUsage fu1;
    fu1.paxTypeFare() = &fare1;
    pu1.fareUsage().push_back(&fu1);

    AirSeg seg;
    seg.setBookingCode("Y");
    pu1.travelSeg().push_back(&seg);

    matchFarePath.pricingUnit().push_back(&pu1);
    matchFarePath.noMatchOption() = false;

    NoPNRPricingTrx trx;
    PricingRequest pr;
    Agent agent;

    pr.ticketingAgent() = &agent;
    trx.setRequest(&pr);

    // itin
    Itin itin;
    trx.itin().push_back(&itin);
    _factory->trx() = &trx;

    itin.farePath().push_back(&matchFarePath);

    CPPUNIT_ASSERT(!_factory->isValidForIntegrated(&noMatchFarePath));
  }

  void testIsValidForIntegratedNotValid()
  {
    FarePath noMatchFarePath, matchFarePath;

    PaxType paxType;
    paxType.paxType() = "ADT";

    noMatchFarePath.paxType() = &paxType;
    matchFarePath.paxType() = &paxType;

    // no-match
    FareClassAppSegInfo info;
    info._bookingCode[0] = "Y";

    PaxTypeFare fare;
    FareInfo fareInfo;
    fareInfo.fareClass() = "YOW";
    fare.fare()->setFareInfo(&fareInfo);
    (FareClassAppSegInfo*&)(fare.fareClassAppSegInfo()) = &info;

    FareUsage fu;
    fu.paxTypeFare() = &fare;
    PricingUnit pu;
    pu.fareUsage().push_back(&fu);

    noMatchFarePath.pricingUnit().push_back(&pu);
    noMatchFarePath.noMatchOption() = true;

    // match
    PaxTypeFare fare1;
    FareInfo fareInfo1;
    fareInfo1.fareClass() = "YOW";
    fare1.fare()->setFareInfo(&fareInfo1);
    PricingUnit pu1;
    FareUsage fu1;
    fu1.paxTypeFare() = &fare1;
    pu1.fareUsage().push_back(&fu1);

    AirSeg seg;
    seg.setBookingCode("C");
    pu1.travelSeg().push_back(&seg);

    matchFarePath.pricingUnit().push_back(&pu1);
    matchFarePath.noMatchOption() = false;

    NoPNRPricingTrx trx;
    PricingRequest pr;
    Agent agent;

    pr.ticketingAgent() = &agent;
    trx.setRequest(&pr);

    // itin
    Itin itin;
    trx.itin().push_back(&itin);
    _factory->trx() = &trx;

    itin.farePath().push_back(&matchFarePath);

    CPPUNIT_ASSERT(!_factory->isValidForIntegrated(&noMatchFarePath));
  }

private:
  test::FactoriesConfigStub _factoriesConfig;
  PaxFarePathFactory* _factory;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PaxFarePathFactoryTest);
}
