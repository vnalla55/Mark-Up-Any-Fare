#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "Common/CurrencyRoundingUtil.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "DBAccess/ContractPreference.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"

namespace
{
using namespace tse;
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  char getVendorType(const VendorCode& vendor)
  {
    if (vendor == "5KAD")
      return 'T';
    else
      return 'P';
  }

  const std::vector<ContractPreference*>& getContractPreferences(const PseudoCityCode& pseudoCity,
                                                                 const CarrierCode& carrier,
                                                                 const RuleNumber& rule,
                                                                 const DateTime& versionDate)
  {
    std::vector<ContractPreference*>& cpv = *_memHandle.create<std::vector<ContractPreference*> >();

    if (carrier == "OZ")
    {
      ContractPreference* cp = _memHandle.create<ContractPreference>();
      cp->applyRoundingException() = 'X';
      cpv.push_back(cp);
    }
    else if (carrier == "BA")
    {
      ContractPreference* cp = _memHandle.create<ContractPreference>();
      cp->applyRoundingException() = 'N';
      cpv.push_back(cp);
    }

    return cpv;
  }
};
} // namespace

namespace tse
{
class CurrencyRoundingUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CurrencyRoundingUtilTest);
  CPPUNIT_TEST(testApplyNonIATARoundingWhenFailOnVendor);
  CPPUNIT_TEST(testApplyNonIATARoundingWhenEmptyContractPreference);
  CPPUNIT_TEST(testApplyNonIATARoundingContractPreferenceSetToN);
  CPPUNIT_TEST(testApplyNonIATARoundingContractPreferenceSetToX);
  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingRequest* _request;
  VendorCode* _vendor;
  CarrierCode* _carrier;
  RuleNumber* _ruleNumber;

public:
  void setUp()
  {
    _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _vendor = _memHandle.create<VendorCode>();
    _carrier = _memHandle.create<CarrierCode>();
    _ruleNumber = _memHandle.create<RuleNumber>();
    _trx->setRequest(_request);
  }

  void tearDown() { _memHandle.clear(); }

  void testApplyNonIATARoundingWhenFailOnVendor()
  {
    *_vendor = "8CI1";
    CPPUNIT_ASSERT(
        !CurrencyRoundingUtil::applyNonIATARounding(*_trx, *_vendor, *_carrier, *_ruleNumber));
  }

  void testApplyNonIATARoundingWhenEmptyContractPreference()
  {
    *_vendor = "5KAD";
    *_carrier = "AA";
    CPPUNIT_ASSERT(
        !CurrencyRoundingUtil::applyNonIATARounding(*_trx, *_vendor, *_carrier, *_ruleNumber));
  }

  void testApplyNonIATARoundingContractPreferenceSetToN()
  {
    *_vendor = "5KAD";
    *_carrier = "BA";
    CPPUNIT_ASSERT(
        !CurrencyRoundingUtil::applyNonIATARounding(*_trx, *_vendor, *_carrier, *_ruleNumber));
  }

  void testApplyNonIATARoundingContractPreferenceSetToX()
  {
    *_vendor = "5KAD";
    *_carrier = "OZ";
    CPPUNIT_ASSERT(
        CurrencyRoundingUtil::applyNonIATARounding(*_trx, *_vendor, *_carrier, *_ruleNumber));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(CurrencyRoundingUtilTest);
}
