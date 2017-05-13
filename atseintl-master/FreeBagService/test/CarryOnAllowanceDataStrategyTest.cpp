// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/PricingTrx.h"
#include "DBAccess/SubCodeInfo.h"
#include "FreeBagService/CarryOnAllowanceDataStrategy.h"
#include "FreeBagService/test/S5Builder.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"

namespace tse
{

class CarryOnAllowanceDataStrategyTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CarryOnAllowanceDataStrategyTest);
  CPPUNIT_TEST(testRetrieveS5Record_MGMR);
  CPPUNIT_TEST(testRetrieveS5Record_ATP);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _strategy = _memHandle.insert(new CarryOnAllowanceDataStrategy(*_trx));
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _dataHandleMock = _memHandle.create<CarryOnAllowanceDataStrategyDataHandleMock>();
  }

  void tearDown() { _memHandle.clear(); }

  void testRetrieveS5Record_MGMR()
  {
    const SubCodeInfo* s5 = _strategy->retrieveS5Record("AA");
    CPPUNIT_ASSERT(s5->vendor() == MERCH_MANAGER_VENDOR_CODE);
  }

  void testRetrieveS5Record_ATP()
  {
    const SubCodeInfo* s5 = _strategy->retrieveS5Record("LH");
    CPPUNIT_ASSERT(s5->vendor() == ATPCO_VENDOR_CODE);
  }

private:
  class CarryOnAllowanceDataStrategyDataHandleMock : public DataHandleMock
  {
    TestMemHandle _memHandle;

    SubCodeInfo* createS5(const VendorCode& vendor, const CarrierCode& carrier)
    {
      return S5Builder(&_memHandle)
          .withVendCarr(vendor, carrier)
          .withFltTktMerchInd(CARRY_ON_ALLOWANCE)
          .withSubCode("0LN")
          .build();
    }

  public:
    const std::vector<SubCodeInfo*>& getSubCode(const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const ServiceTypeCode& serviceTypeCode,
                                                const ServiceGroup& serviceGroup,
                                                const DateTime& date)
    {
      std::vector<SubCodeInfo*>* s5vector = _memHandle.create<std::vector<SubCodeInfo*> >();

      if (vendor == ATPCO_VENDOR_CODE)
      {
        if (carrier == "LH")
          s5vector->push_back(createS5(ATPCO_VENDOR_CODE, "LH"));
        else if (carrier == "LT")
          s5vector->push_back(createS5(ATPCO_VENDOR_CODE, "LT"));
      }
      else
      {
        if (carrier == "LH")
          s5vector->push_back(createS5(MERCH_MANAGER_VENDOR_CODE, "LH"));
        else if (carrier == "LT")
          s5vector->push_back(createS5(MERCH_MANAGER_VENDOR_CODE, "LT"));
        else if (carrier == "AA")
          s5vector->push_back(createS5(MERCH_MANAGER_VENDOR_CODE, "AA"));
      }

      return *s5vector;
    }
  };

  TestMemHandle _memHandle;
  PricingTrx* _trx;
  CarryOnAllowanceDataStrategy* _strategy;
  CarryOnAllowanceDataStrategyDataHandleMock* _dataHandleMock;
};

CPPUNIT_TEST_SUITE_REGISTRATION(CarryOnAllowanceDataStrategyTest);

} // tse
