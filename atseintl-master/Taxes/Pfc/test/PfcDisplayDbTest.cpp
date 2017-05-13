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

#include "Taxes/Pfc/PfcDisplayDb.h"
#include "DataModel/PfcDisplayRequest.h"
#include "DataModel/TaxTrx.h"
#include "test/include/CppUnitHelperMacros.h"

#include <iostream>
#include <memory>
#include <string>

#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "DBAccess/Customer.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/PfcPFC.h"
#include "DBAccess/PfcEssAirSvc.h"

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  PfcPFC* getPfc(LocCode loc)
  {
    PfcPFC* ret = _memHandle.create<PfcPFC>();
    ret->effDate() = DateTime(1980, 1, 1);
    ret->expireDate() = DateTime(3000, 1, 1);
    ret->pfcAirport() = loc;
    return ret;
  }

public:
  const std::vector<Customer*>& getCustomer(const PseudoCityCode& key)
  {
    if (key == "17R7")
    {
      Customer* c = _memHandle.create<Customer>();
      std::vector<Customer*>& ret = *_memHandle.create<std::vector<Customer*> >();
      c->pseudoCity() = key;
      ret.push_back(c);
      return ret;
    }
    return DataHandleMock::getCustomer(key);
  }
  const Nation* getNation(const NationCode& nationCode, const DateTime& date)
  {
    if (nationCode == "PL")
      return _memHandle.create<Nation>();
    return DataHandleMock::getNation(nationCode, date);
  }
  const std::vector<TaxCodeReg*>& getTaxCode(const TaxCode& taxCode, const DateTime& date)
  {
    if (taxCode == "US1")
    {
      std::vector<TaxCodeReg*>& ret = *_memHandle.create<std::vector<TaxCodeReg*> >();
      ret.push_back(_memHandle.create<TaxCodeReg>());
      return ret;
    }
    return DataHandleMock::getTaxCode(taxCode, date);
  }
  const std::vector<PfcPFC*>& getAllPfcPFC()
  {
    std::vector<PfcPFC*>& ret = *_memHandle.create<std::vector<PfcPFC*> >();
    ret.push_back(getPfc("ATW"));
    ret.push_back(getPfc("JFK"));
    return ret;
  }
  const std::vector<PfcEssAirSvc*>& getAllPfcEssAirSvc()
  {
    std::vector<PfcEssAirSvc*>& ret = *_memHandle.create<std::vector<PfcEssAirSvc*> >();
    ret.push_back(_memHandle.create<PfcEssAirSvc>());
    ret.front()->effDate() = DateTime(1980, 1, 1);
    ret.front()->expireDate() = DateTime(3000, 1, 1);
    ret.front()->easHubArpt() = "ABQ";
    return ret;
  }
  const PfcMultiAirport* getPfcMultiAirport(const LocCode& key, const DateTime& date)
  {
    if (key == "NYC")
      return _memHandle.create<PfcMultiAirport>();
    return DataHandleMock::getPfcMultiAirport(key, date);
  }
  const std::vector<PfcMultiAirport*>& getAllPfcMultiAirport()
  {
    std::vector<PfcMultiAirport*>& ret = *_memHandle.create<std::vector<PfcMultiAirport*> >();
    ret.push_back(_memHandle.create<PfcMultiAirport>());
    ret.front()->effDate() = DateTime(1980, 1, 1);
    ret.front()->expireDate() = DateTime(3000, 1, 1);
    return ret;
  }
  const std::vector<PfcAbsorb*>& getAllPfcAbsorb()
  {
    std::vector<PfcAbsorb*>& ret = *_memHandle.create<std::vector<PfcAbsorb*> >();
    ret.push_back(_memHandle.create<PfcAbsorb>());
    ret.front()->effDate() = DateTime(1980, 1, 1);
    ret.front()->expireDate() = DateTime(3000, 1, 1);
    ret.front()->pfcAirport() = "ABE";
    ret.front()->localCarrier() = "US";
    return ret;
  }
};
}
class PfcDisplayDbTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PfcDisplayDbTest);

  CPPUNIT_TEST(newInstanceTest);
  CPPUNIT_TEST(getCustomerTest);
  CPPUNIT_TEST(getLocTest);
  CPPUNIT_TEST(getNationTest);
  CPPUNIT_TEST(getTaxCodeTest);
  CPPUNIT_TEST(getPfcPFCTest);
  CPPUNIT_TEST(getAllPfcPFCTest);
  CPPUNIT_TEST(getDateIndependentPfcPFCTest);
  CPPUNIT_TEST(getPfcEssAirSvcTest);
  CPPUNIT_TEST(getAllPfcEssAirSvcTest);
  CPPUNIT_TEST(getPfcMultiAirportTest);
  CPPUNIT_TEST(getAllPfcMultiAirportTest);
  CPPUNIT_TEST(getPfcAbsorbTest);
  CPPUNIT_TEST(getAllPfcAbsorbTest);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _trx = _memHandle.create<TaxTrx>();
    _pfcDisplayRequest = _memHandle.create<PfcDisplayRequest>();
    _trx->pfcDisplayRequest() = _pfcDisplayRequest;
    _trx->pfcDisplayRequest()->overrideDate() = DateTime::localTime();
    _memHandle.create<MyDataHandle>();
  }
  void tearDown() { _memHandle.clear(); }

  void newInstanceTest()
  {
    std::unique_ptr<PfcDisplayDb> ptr(new PfcDisplayDb(_trx));

    CPPUNIT_ASSERT(ptr);
  }

  void getCustomerTest()
  {
    PfcDisplayDb db(_trx);

    const Customer* customer = db.getCustomer("17R7");

    if (!customer)
    {
      CPPUNIT_ASSERT(false);
    }
    else
    {
      CPPUNIT_ASSERT(!customer->pseudoCity().empty());
    }
  }

  void getLocTest()
  {
    PfcDisplayDb db(_trx);

    const Loc* loc = db.getLoc("DFW");

    CPPUNIT_ASSERT(loc);
  }

  void getNationTest()
  {
    PfcDisplayDb db(_trx);

    const Nation* nation = db.getNation("PL");

    CPPUNIT_ASSERT(nation);
  }

  void getTaxCodeTest()
  {
    PfcDisplayDb db(_trx);

    const std::vector<TaxCodeReg*>& us1V = db.getTaxCode("US1");

    CPPUNIT_ASSERT(!us1V.empty());
  }

  void getPfcPFCTest()
  {
    PfcDisplayDb db(_trx);

    const std::vector<PfcPFC*>& pfcV = db.getPfcPFC("JFK");

    CPPUNIT_ASSERT(!pfcV.empty());
  }

  void getAllPfcPFCTest()
  {
    PfcDisplayDb db(_trx);

    const std::vector<PfcPFC*>& pfcV = db.getAllPfcPFC();

    CPPUNIT_ASSERT(!pfcV.empty());
  }

  void getDateIndependentPfcPFCTest()
  {
    PfcDisplayDb db(_trx);

    const std::vector<PfcPFC*>& pfcV = db.getDateIndependentPfcPFC("ATW");

    CPPUNIT_ASSERT(!pfcV.empty());
  }

  void getPfcMultiAirportTest()
  {
    PfcDisplayDb db(_trx);

    const PfcMultiAirport* ma = db.getPfcMultiAirport("NYC");

    CPPUNIT_ASSERT(ma);
  }

  void getAllPfcMultiAirportTest()
  {
    PfcDisplayDb db(_trx);

    const std::vector<PfcMultiAirport*>& pmaV = db.getAllPfcMultiAirport();

    CPPUNIT_ASSERT(!pmaV.empty());
  }

  void getPfcEssAirSvcTest()
  {
    PfcDisplayDb db(_trx);

    const std::vector<PfcEssAirSvc*>& essAirSvcV = db.getPfcEssAirSvc("ABQ");

    CPPUNIT_ASSERT(!essAirSvcV.empty());
  }

  void getAllPfcEssAirSvcTest()
  {
    PfcDisplayDb db(_trx);

    const std::vector<PfcEssAirSvc*>& essAirSvcV = db.getAllPfcEssAirSvc();

    CPPUNIT_ASSERT(!essAirSvcV.empty());
  }

  void getPfcAbsorbTest()
  {
    PfcDisplayDb db(_trx);

    const std::vector<PfcAbsorb*>& absorpV = db.getPfcAbsorb("ABE", "US");

    CPPUNIT_ASSERT(!absorpV.empty());
  }

  void getAllPfcAbsorbTest()
  {
    PfcDisplayDb db(_trx);

    const std::vector<PfcAbsorb*>& absorbV = db.getAllPfcAbsorb();

    CPPUNIT_ASSERT(!absorbV.empty());
  }

private:
  TaxTrx* _trx;
  PfcDisplayRequest* _pfcDisplayRequest;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(PfcDisplayDbTest);
}
