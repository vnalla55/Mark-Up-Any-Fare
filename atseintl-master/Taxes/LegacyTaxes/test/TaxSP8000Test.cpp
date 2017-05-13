#include <string>
#include <time.h>
#include <iostream>
#include <vector>

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxTrx.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxSP8000.h"
#include "DataModel/Itin.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/FarePath.h"
#include "DBAccess/TaxSpecConfigReg.h"

namespace tse
{

class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  ~MyDataHandle() { _memHandle.clear(); }

  std::vector<TaxSpecConfigReg*>& getTaxSpecConfig(const TaxSpecConfigName& name)
  {
    std::vector<TaxSpecConfigReg*>& ret = *_memHandle.create<std::vector<TaxSpecConfigReg*> >();
    ret.push_back(_memHandle.create<TaxSpecConfigReg>());
    TaxSpecConfigReg::TaxSpecConfigRegSeq* seq = new TaxSpecConfigReg::TaxSpecConfigRegSeq();
    ret.front()->taxSpecConfigName() = name;
    seq->paramName() = "ANCILLARYSERVICE";
    if (name == "POSITIVEMATCH")
      seq->paramValue() = "TRUE";
    else if (name == "ANCILLARYSERVICE")
      seq->paramValue() = "X...";
    else
      seq->paramValue() = "ERROR";
    ret.front()->seqs().push_back(seq);
    return ret;
  }
};

class TaxSP8000Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxSP8000Test);
  CPPUNIT_TEST(validateAnciliaryTag_pricingTrx);
  /* CPPUNIT_TEST(validateAnciliaryTag_empty);
   CPPUNIT_TEST(validateAnciliaryTag_ok);
 Jakub Kubica - temporary disable
 */
  CPPUNIT_TEST_SUITE_END();

  TaxSP8000* tax;
  Itin* itin;
  TaxResponse* taxResponse;
  TaxCodeReg* taxCodeReg;
  FarePath* farePath;

public:
  void setUp()
  {
    // std::cout << "setup" << std::endl;
    tax = new TaxSP8000();
    itin = new Itin();
    taxResponse = new TaxResponse();
    taxCodeReg = new TaxCodeReg();
    farePath = new FarePath();
    farePath->itin() = itin;
    taxResponse->farePath() = farePath;
  }

  void validateAnciliaryTag_pricingTrx()
  {
    PricingTrx trx;
    uint16_t a = 0;
    bool res = tax->validateLocRestrictions(trx, *taxResponse, *taxCodeReg, a, a);
    CPPUNIT_ASSERT(res == false);
  }

  void validateAnciliaryTag_empty()
  {

    TaxTrx trx;
    uint16_t a = 0;
    itin->anciliaryServiceCode() = "";
    bool res = tax->validateLocRestrictions(trx, *taxResponse, *taxCodeReg, a, a);
    CPPUNIT_ASSERT(res == false);
  }

  void validateAnciliaryTag_ok()
  {
    TaxTrx trx;
    uint16_t a = 0;
    itin->anciliaryServiceCode() = "ZYZ";
    bool res = tax->validateLocRestrictions(trx, *taxResponse, *taxCodeReg, a, a);
    CPPUNIT_ASSERT(res == true);
  }

  void validateAnciliaryTag_nok()
  {
    TaxTrx trx;
    uint16_t a = 0;
    itin->anciliaryServiceCode() = "UUU";
    bool res = tax->validateLocRestrictions(trx, *taxResponse, *taxCodeReg, a, a);
    CPPUNIT_ASSERT(res == false);
  }

  void tearDown()
  {
    delete itin;
    delete farePath;
    delete taxCodeReg;
    delete taxResponse;
    delete tax;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TaxSP8000Test);
}
