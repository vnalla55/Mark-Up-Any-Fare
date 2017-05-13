//
// Copyright Sabre 2015
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
//
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "Common/EmdValidator.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/EmdInterlineAgreementInfo.h"
#include "Diagnostic/Diag875Collector.h"
#include "ServiceFees/OCEmdDataProvider.h"

namespace tse
{

class EmdValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(EmdValidatorTest);
  CPPUNIT_TEST(testCheckEmdInterlineAgreementMscNoMatch_noSuchCarrier);
  CPPUNIT_TEST(testCheckEmdInterlineAgreementMscNoMatch_CarrierIsValidatingInsteadOfParticipating);
  CPPUNIT_TEST(testCheckEmdInterlineAgreementMscMatchFirst);
  CPPUNIT_TEST(testCheckEmdInterlineAgreementMscMatchLast);

  CPPUNIT_TEST(testCheckEmdAgreementAllPassed);
  CPPUNIT_TEST(testCheckEmdAgreementMarketingFailed);
  CPPUNIT_TEST(testCheckEmdAgreementOperatingFailed);
  CPPUNIT_TEST(testCheckRegularEmdAgreementAllCarriersTheSame);
  CPPUNIT_TEST(testCheckRegularEmdAgreementMixedCarriersWithValidating);
  CPPUNIT_TEST(testCheckRegularEmdAgreementMixedCarriers);
  CPPUNIT_TEST(testCheckRegularEmdAgreementFailedMarketing);
  CPPUNIT_TEST(testCheckRegularEmdAgreementFailedOperating);
  CPPUNIT_TEST(testIsAnyCarrierEmpty);
  CPPUNIT_TEST(testIsValidationNeeded);


  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  PricingTrx* _trx;
  OCEmdDataProvider* _dataProvider;
  Diag875Collector* _diag875;
  EmdValidator* _emdValidator;
  std::vector<EmdInterlineAgreementInfo*> _eiaList;

public:
  void setUp()
  {
    _trx = _memHandle(new PricingTrx);
    _dataProvider = _memHandle(new OCEmdDataProvider);
    _diag875 = _memHandle(new Diag875Collector);
    _emdValidator = _memHandle(new EmdValidator(*_trx, *_dataProvider, _diag875));

    EmdInterlineAgreementInfo* eiaInfo = _memHandle(new EmdInterlineAgreementInfo);
    eiaInfo->setParticipatingCarrier("AP");
    eiaInfo->setValidatingCarrier("AV");
    eiaInfo->setCountryCode("US");
    _eiaList.push_back(eiaInfo);

    eiaInfo = _memHandle(new EmdInterlineAgreementInfo);
    eiaInfo->setParticipatingCarrier("BP");
    eiaInfo->setValidatingCarrier("BV");
    eiaInfo->setCountryCode("UA");
    _eiaList.push_back(eiaInfo);

    eiaInfo = _memHandle(new EmdInterlineAgreementInfo);
    eiaInfo->setParticipatingCarrier("CP");
    eiaInfo->setValidatingCarrier("CV");
    eiaInfo->setCountryCode("RU");
    _eiaList.push_back(eiaInfo);

    eiaInfo = _memHandle(new EmdInterlineAgreementInfo);
    eiaInfo->setParticipatingCarrier("DP");
    eiaInfo->setValidatingCarrier("DV");
    eiaInfo->setCountryCode("PL");
    _eiaList.push_back(eiaInfo);
  }

  void testCheckEmdInterlineAgreementMscNoMatch_noSuchCarrier()
  {
    _dataProvider->setEmdMostSignificantCarrier("EE");
    CPPUNIT_ASSERT_EQUAL(false, _emdValidator->checkEmdInterlineAgrrementMsc(_eiaList));
  }

  void testCheckEmdInterlineAgreementMscNoMatch_CarrierIsValidatingInsteadOfParticipating()
  {
    _dataProvider->setEmdMostSignificantCarrier("AV");
    CPPUNIT_ASSERT_EQUAL(false, _emdValidator->checkEmdInterlineAgrrementMsc(_eiaList));
  }

  void testCheckEmdInterlineAgreementMscMatchFirst()
  {
    _dataProvider->setEmdMostSignificantCarrier("AP");
    CPPUNIT_ASSERT_EQUAL(true, _emdValidator->checkEmdInterlineAgrrementMsc(_eiaList));
  }

  void testCheckEmdInterlineAgreementMscMatchSecond()
  {
    _dataProvider->setEmdMostSignificantCarrier("BP");
    CPPUNIT_ASSERT_EQUAL(true, _emdValidator->checkEmdInterlineAgrrementMsc(_eiaList));
  }

  void testCheckEmdInterlineAgreementMscMatchLast()
  {
    _dataProvider->setEmdMostSignificantCarrier("DP");
    CPPUNIT_ASSERT_EQUAL(true, _emdValidator->checkEmdInterlineAgrrementMsc(_eiaList));
  }

  void testCheckEmdAgreementAllPassed()
  {
    std::set<CarrierCode> marketingCarriers, operatingCarriers, emdInfoParticipatingCarriers;
    marketingCarriers = {"AA", "BB"};
    operatingCarriers = {"CC", "DD"};
    emdInfoParticipatingCarriers = {"AA", "BB", "CC", "DD"};
    CPPUNIT_ASSERT_EQUAL(true, _emdValidator->checkEmdAgreement(marketingCarriers, operatingCarriers, emdInfoParticipatingCarriers) );
  }

  void testCheckEmdAgreementMarketingFailed()
  {
    std::set<CarrierCode> marketingCarriers, operatingCarriers, emdInfoParticipatingCarriers;
    marketingCarriers = {"AA", "EE"};
    operatingCarriers = {"CC", "DD"};
    emdInfoParticipatingCarriers = {"AA", "BB", "CC", "DD"};
    CPPUNIT_ASSERT_EQUAL(false, _emdValidator->checkEmdAgreement(marketingCarriers, operatingCarriers, emdInfoParticipatingCarriers) );
  }

  void testCheckEmdAgreementOperatingFailed()
  {
    std::set<CarrierCode> marketingCarriers, operatingCarriers, emdInfoParticipatingCarriers;
    marketingCarriers = {"AA", "BB"};
    operatingCarriers = {"CC", "EE"};
    emdInfoParticipatingCarriers = {"AA", "BB", "CC", "DD"};
    CPPUNIT_ASSERT_EQUAL(false, _emdValidator->checkEmdAgreement(marketingCarriers, operatingCarriers, emdInfoParticipatingCarriers) );
  }

  void testCheckRegularEmdAgreementAllCarriersTheSame()
  {
    _dataProvider->emdValidatingCarrier() = "VA";
    _dataProvider->marketingCarriers() = {"VA"};
    _dataProvider->operatingCarriers() = {"VA"};

    CPPUNIT_ASSERT_EQUAL(true, _emdValidator->checkRegularEmdAgreement(_eiaList));
  }

  void testCheckRegularEmdAgreementMixedCarriersWithValidating()
  {
    _dataProvider->emdValidatingCarrier() = "VA";
    _dataProvider->marketingCarriers() = {"AP", "BP", "VA"};
    _dataProvider->operatingCarriers() = {"CP", "DP", "VA"};

    CPPUNIT_ASSERT_EQUAL(true, _emdValidator->checkRegularEmdAgreement(_eiaList));
  }

  void testCheckRegularEmdAgreementMixedCarriers()
  {
    _dataProvider->emdValidatingCarrier() = "VA";
    _dataProvider->marketingCarriers() = {"AP", "BP"};
    _dataProvider->operatingCarriers() = {"CP", "DP"};

    CPPUNIT_ASSERT_EQUAL(true, _emdValidator->checkRegularEmdAgreement(_eiaList));
  }

  void testCheckRegularEmdAgreementFailedMarketing()
  {
    _dataProvider->emdValidatingCarrier() = "VA";
    _dataProvider->marketingCarriers() = {"AP", "BP", "AA"};
    _dataProvider->operatingCarriers() = {"CP", "DP"};

    CPPUNIT_ASSERT_EQUAL(false, _emdValidator->checkRegularEmdAgreement(_eiaList));
  }

  void testCheckRegularEmdAgreementFailedOperating()
  {
    _dataProvider->emdValidatingCarrier() = "VA";
    _dataProvider->marketingCarriers() = {"AP", "BP"};
    _dataProvider->operatingCarriers() = {"CP", "DP", "AA"};

    CPPUNIT_ASSERT_EQUAL(false, _emdValidator->checkRegularEmdAgreement(_eiaList));
  }

  void testIsAnyCarrierEmpty()
  {
    CPPUNIT_ASSERT_EQUAL(true, _emdValidator->isAnyCarrierEmpty());
    _dataProvider->emdValidatingCarrier() = "VA";
    CPPUNIT_ASSERT_EQUAL(true, _emdValidator->isAnyCarrierEmpty());
    _dataProvider->marketingCarriers() = {"AP", "BP"};
    CPPUNIT_ASSERT_EQUAL(true, _emdValidator->isAnyCarrierEmpty());
    _dataProvider->operatingCarriers() = {"CP", "DP", "AA"};
    CPPUNIT_ASSERT_EQUAL(false, _emdValidator->isAnyCarrierEmpty());
  }

  void testIsValidationNeeded()
  {
    _dataProvider->emdValidatingCarrier() = "VA";
    _dataProvider->marketingCarriers() = {"VA"};
    _dataProvider->operatingCarriers() = {"VA"};
    CPPUNIT_ASSERT_EQUAL(false, _emdValidator->isValidationNeeded());
    _dataProvider->marketingCarriers() = {"VA", "SQ"};
    CPPUNIT_ASSERT_EQUAL(true, _emdValidator->isValidationNeeded());
    _dataProvider->marketingCarriers() = {"VA"};
    _dataProvider->operatingCarriers() = {"VA", "SQ"};
    CPPUNIT_ASSERT_EQUAL(true, _emdValidator->isValidationNeeded());
  }

  void testValidate()
  {
    std::map<CarrierCode, std::vector<EmdInterlineAgreementInfo*>> carrierEmdInfoMap;
    _dataProvider->emdValidatingCarrier() = "VA";
    _dataProvider->marketingCarriers() = {"VA"};
    _dataProvider->operatingCarriers() = {"VA"};

    CPPUNIT_ASSERT_EQUAL(true, _emdValidator->validate(carrierEmdInfoMap));

    _dataProvider->marketingCarriers() = {"AP", "BP"};
    _dataProvider->operatingCarriers() = {"CP", "DP"};

    carrierEmdInfoMap["AP"] = _eiaList;
    CPPUNIT_ASSERT_EQUAL(false, _emdValidator->validate(carrierEmdInfoMap));
    carrierEmdInfoMap["VA"];
    CPPUNIT_ASSERT_EQUAL(false, _emdValidator->validate(carrierEmdInfoMap));
    carrierEmdInfoMap["VA"] = _eiaList;
    CPPUNIT_ASSERT_EQUAL(true, _emdValidator->validate(carrierEmdInfoMap));
    _dataProvider->operatingCarriers() = {};
    CPPUNIT_ASSERT_EQUAL(false, _emdValidator->validate(carrierEmdInfoMap));
  }


};

CPPUNIT_TEST_SUITE_REGISTRATION(EmdValidatorTest);

} // namsepace tse
