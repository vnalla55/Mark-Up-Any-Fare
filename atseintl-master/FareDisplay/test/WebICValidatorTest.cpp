#include "test/include/CppUnitHelperMacros.h"
#include "Rules/RuleUtil.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "FareDisplay/WebICValidator.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxType.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class MyWebICValidator : public WebICValidator
{
public:
  MyWebICValidator()
  {
    fdWeb.displayInd() = 'I';
    fdWeb.paxType() = "ADT";
    fdWeb.tktDesignator() = "TKT123";
    fdWeb.vendor() = "ATP";
    fdWeb.carrier() = "LL";
    fdWeb.ruleTariff() = 999;
    fdWeb.fareClass() = "FQ123";
    fdWeb.rule() = "ABC1";
    _webRecords.push_back(&fdWeb);
  }

  virtual const std::vector<FareDisplayWeb*>& getFareDisplayWeb(const FareDisplayTrx& trx,
                                                                const Indicator& dispInd,
                                                                const VendorCode& vendor,
                                                                const CarrierCode& carrier,
                                                                const TariffNumber& ruleTariff,
                                                                const RuleNumber& rule,
                                                                const PaxTypeCode& paxTypeCode)
  {

    if (dispInd == fdWeb.displayInd() && vendor == fdWeb.vendor() && carrier == fdWeb.carrier() &&
        ruleTariff == fdWeb.ruleTariff() && rule == fdWeb.rule() && paxTypeCode == fdWeb.paxType())
      return _webRecords;
    return _webRecordsEmpty;
  }

private:
  std::vector<FareDisplayWeb*> _webRecords;
  std::vector<FareDisplayWeb*> _webRecordsEmpty;
  FareDisplayWeb fdWeb;
};

class WebICValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(WebICValidatorTest);
  CPPUNIT_TEST(testPasses);
  CPPUNIT_TEST(testFailsWithWrongCarrier);
  CPPUNIT_TEST(testFailsWithWrongDisplayIndicator);
  CPPUNIT_TEST(testFailsWithWrongFareClass);
  CPPUNIT_TEST(testFailsWithWrongTktDesignator);
  CPPUNIT_TEST(testFailsWithWrongPaxType);
  CPPUNIT_TEST(testFailsWithWrongVendor);
  CPPUNIT_TEST(testFailsWithNoRequest);
  CPPUNIT_TEST(testFailsWithChildRequest);
  CPPUNIT_TEST(testFailsWithInfantRequest);
  CPPUNIT_TEST(testFailsWithAdultRequest);
  CPPUNIT_TEST(testPassWithNoRequest);
  CPPUNIT_TEST(testPassWithChildRequest);
  CPPUNIT_TEST(testPassWithInfantRequest);
  CPPUNIT_TEST(testPassWithAdultRequest);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {

    _trx = _memHandle.create<FareDisplayTrx>();

    _options = _memHandle.create<FareDisplayOptions>();
    _trx->setOptions(_options);

    _fareClassAppSegInfo = _memHandle.create<FareClassAppSegInfo>();
    _fareClassAppSegInfo->_paxType = "ADT";
    _fareClassAppSegInfo->_tktDesignator = "TKT123";

    _fareClassAppInfo = _memHandle.create<FareClassAppInfo>();
    _fareClassAppInfo->_displayCatType = 'I';

    _fareInfo = _memHandle.create<FareInfo>();
    _fareInfo->vendor() = "ATP";
    _fareInfo->carrier() = "LL";
    _fareInfo->fareTariff() = 999;
    _fareInfo->fareClass() = "FQ123";
    _fareInfo->ruleNumber() = "ABC1";

    _fare = _memHandle.create<Fare>();
    _fare->setFareInfo(_fareInfo);

    _actualPaxType = _memHandle.create<PaxType>();
    _paxTypeInfo = _memHandle.create<PaxTypeInfo>();
    _paxTypeInfo->paxType() = "ADT";
    _paxTypeInfo->childInd() = 'N';
    _paxTypeInfo->infantInd() = 'N';
    _paxTypeInfo->adultInd() = 'Y';
    _paxTypeInfo->initPsgType();
    _actualPaxType->paxTypeInfo() = _paxTypeInfo;

    _ptFare = _memHandle.create<PaxTypeFare>();
    _ptFare->setFare(_fare);
    _ptFare->fareClassAppSegInfo() = _fareClassAppSegInfo;
    _ptFare->fareClassAppInfo() = _fareClassAppInfo;
    _ptFare->actualPaxType() = _actualPaxType;

    _validator = _memHandle.create<MyWebICValidator>();
    _validator->initialize(*_trx);
  }

  void tearDown() { _memHandle.clear(); }

  void testPasses() { CPPUNIT_ASSERT(_validator->validate(*_ptFare)); }

  void testFailsWithWrongCarrier()
  {
    _fareInfo->carrier() = "DO";
    _ptFare->setFare(_fare);
    CPPUNIT_ASSERT(!_validator->validate(*_ptFare));
  }

  void testFailsWithWrongDisplayIndicator()
  {
    _fareClassAppInfo->_displayCatType = 'B';

    CPPUNIT_ASSERT(!_validator->validate(*_ptFare));
  }

  void testFailsWithWrongFareClass()
  {
    _fareInfo->fareClass() = "FDUPA";

    CPPUNIT_ASSERT(!_validator->validate(*_ptFare));
  }

  void testFailsWithWrongTktDesignator()
  {
    _fareClassAppSegInfo->_tktDesignator = "BABCIA";

    CPPUNIT_ASSERT(!_validator->validate(*_ptFare));
  }

  void testFailsWithWrongPaxType()
  {
    _fareClassAppSegInfo->_paxType = "MIL";

    CPPUNIT_ASSERT(!_validator->validate(*_ptFare));
  }

  void testFailsWithWrongVendor()
  {
    _fareInfo->vendor() = "SITA";

    CPPUNIT_ASSERT(!_validator->validate(*_ptFare));
  }

  void testFailsWithNoRequest()
  {
    // no request - pass only adult fares
    _paxTypeInfo->paxType() = "CHD";
    _paxTypeInfo->childInd() = 'Y';
    _paxTypeInfo->adultInd() = 'N';
    _paxTypeInfo->initPsgType();

    CPPUNIT_ASSERT(!_validator->validate(*_ptFare));
  }
  void testFailsWithChildRequest()
  {
    _trx->getOptions()->childFares() = 'Y';

    CPPUNIT_ASSERT(!_validator->validate(*_ptFare));
  }
  void testFailsWithInfantRequest()
  {
    _trx->getOptions()->infantFares() = 'Y';

    CPPUNIT_ASSERT(!_validator->validate(*_ptFare));
  }
  void testFailsWithAdultRequest()
  {
    _trx->getOptions()->adultFares() = 'Y';

    _paxTypeInfo->paxType() = "CHD";
    _paxTypeInfo->childInd() = 'Y';
    _paxTypeInfo->adultInd() = 'N';
    _paxTypeInfo->initPsgType();

    CPPUNIT_ASSERT(!_validator->validate(*_ptFare));
  }
  void testPassWithNoRequest() { CPPUNIT_ASSERT(_validator->validate(*_ptFare)); }
  void testPassWithChildRequest()
  {
    _trx->getOptions()->childFares() = 'Y';

    _paxTypeInfo->paxType() = "CHD";
    _paxTypeInfo->childInd() = 'Y';
    _paxTypeInfo->adultInd() = 'N';
    _paxTypeInfo->initPsgType();

    CPPUNIT_ASSERT(_validator->validate(*_ptFare));
  }
  void testPassWithInfantRequest()
  {
    _trx->getOptions()->infantFares() = 'Y';

    _paxTypeInfo->paxType() = "INF";
    _paxTypeInfo->infantInd() = 'Y';
    _paxTypeInfo->adultInd() = 'N';
    _paxTypeInfo->initPsgType();

    CPPUNIT_ASSERT(_validator->validate(*_ptFare));
  }
  void testPassWithAdultRequest()
  {
    _trx->getOptions()->adultFares() = 'Y';

    CPPUNIT_ASSERT(_validator->validate(*_ptFare));
  }

private:
  FareDisplayTrx* _trx;
  PaxTypeFare* _ptFare;
  PaxType* _actualPaxType;
  PaxTypeInfo* _paxTypeInfo;
  Fare* _fare;
  FareInfo* _fareInfo;
  FareClassAppSegInfo* _fareClassAppSegInfo;
  FareClassAppInfo* _fareClassAppInfo;
  WebICValidator* _validator;
  FareDisplayOptions* _options;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(WebICValidatorTest);
}
