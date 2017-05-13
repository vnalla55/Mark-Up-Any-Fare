// ---------------------------------------------------------------------------
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------

#include "Taxes/Pfc/PfcDisplayData.h"
#include "Taxes/Pfc/test/PfcDisplayDbMock.h"
#include "DataModel/Agent.h"

#include <iostream>
#include <memory>
#include <string>
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
namespace
{
class PfcDisplayDbAxessMock : public PfcDisplayDbMock
{
  Customer _customer;

public:
  PfcDisplayDbAxessMock(TaxTrx* trx) : PfcDisplayDbMock(trx)
  {
    _customer.crsCarrier() = AXESS_MULTIHOST_ID;
  }
  virtual ~PfcDisplayDbAxessMock() {}

  const Customer* getCustomer(const PseudoCityCode& key) const { return &_customer; }
};
}

class PfcDisplayDataTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PfcDisplayDataTest);
  CPPUNIT_TEST(newInstanceTest);
  CPPUNIT_TEST(getAxessPrefixTest);
  CPPUNIT_TEST_SUITE_END();

public:
  void tearDown() { _memHandle.clear(); }

  void newInstanceTest()
  {
    TaxTrx trx;
    PfcDisplayRequest pfcDisplayRequest;
    trx.pfcDisplayRequest() = &pfcDisplayRequest;
    trx.pfcDisplayRequest()->overrideDate() = DateTime::localTime();

    PfcDisplayDb* db = _memHandle.insert(new PfcDisplayDbMock(&trx));

    std::unique_ptr<PfcDisplayData> ptr(new PfcDisplayData(&trx, db));

    CPPUNIT_ASSERT(ptr);
  }

  void getAxessPrefixTest()
  {
    TaxTrx trx;
    PfcDisplayRequest pfcDisplayRequest;
    trx.pfcDisplayRequest() = &pfcDisplayRequest;

    Agent agent;
    agent.mainTvlAgencyPCC() = "FC86";
    trx.pfcDisplayRequest()->ticketingAgent() = &agent;

    PfcDisplayDb* db = _memHandle.insert(new PfcDisplayDbAxessMock(&trx));
    PfcDisplayData da(&trx, db);

    CPPUNIT_ASSERT(da.getAxessPrefix() == PfcDisplayData::AXESS_PREFIX + "\n");
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(PfcDisplayDataTest);
}
