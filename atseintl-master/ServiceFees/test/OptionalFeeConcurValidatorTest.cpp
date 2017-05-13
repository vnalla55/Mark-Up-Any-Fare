#include <boost/assign/std/set.hpp>
#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/OptionalServicesConcur.h"
#include "DBAccess/SubCodeInfo.h"
#include "Diagnostic/Diag876Collector.h"
#include "ServiceFees/MerchCarrierStrategy.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OptionalFeeConcurValidator.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

using namespace boost::assign;

namespace tse
{

class OptionalFeeConcurValidatorTest : public CppUnit::TestFixture
{
  struct OptionalFeeConcurValidatorMock : public OptionalFeeConcurValidator
  {
  public:
    OptionalFeeConcurValidatorMock(PricingTrx& trx, FarePath* farePath)
      : OptionalFeeConcurValidator(trx, farePath)
    {
    }
  };
  struct MyDataHandle : public DataHandleMock
  {
    const std::vector<SubCodeInfo*>& getSubCode(const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const ServiceTypeCode& serviceTypeCode,
                                                const ServiceGroup& serviceGroup,
                                                const DateTime& date)
    {
      std::vector<SubCodeInfo*>* ret = _memHandle.create<std::vector<SubCodeInfo*> >();
      if (vendor == "MMGR")
        return *ret;
      else if (serviceGroup == "BL")
        return *ret;
      else if (serviceGroup == "S1")
      {
        // four different sub codes
        *ret += getSCI(vendor, carrier, serviceTypeCode, "SS1", serviceGroup, "", '1'),
            getSCI(vendor, carrier, serviceTypeCode, "SS1", serviceGroup, "", '2'),
            getSCI(vendor, carrier, serviceTypeCode, "SS2", serviceGroup, "", '1'),
            getSCI(vendor, carrier, serviceTypeCode, "SS1", serviceGroup, "D", '1');
        return *ret;
      }
      else if (serviceGroup == "S2")
      {
        // same subcodes
        *ret += getSCI(vendor, carrier, serviceTypeCode, "SS2", serviceGroup, "", '1'),
            getSCI(vendor, carrier, serviceTypeCode, "SS2", serviceGroup, "", '1');
        return *ret;
      }
      else if (serviceGroup == "S3")
      {
        *ret += getSCI(vendor, carrier, serviceTypeCode, "SS3", serviceGroup, "", '1');
        return *ret;
      }
      else if (serviceGroup == "T1" || serviceGroup == "T2" || serviceGroup == "T3")
      {
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT2", serviceGroup, "T2", '1');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT2", serviceGroup, "T2", '2');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT2", serviceGroup, "T2", 'X');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT2", serviceGroup, "T1", '1');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT2", serviceGroup, "T1", '2');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT2", serviceGroup, "T1", 'X');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT2", serviceGroup, "T3", '1');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT2", serviceGroup, "T3", '2');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT2", serviceGroup, "T3", 'X');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT1", serviceGroup, "T2", '1');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT1", serviceGroup, "T2", '2');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT1", serviceGroup, "T2", 'X');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT1", serviceGroup, "T1", '1');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT1", serviceGroup, "T1", '2');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT1", serviceGroup, "T1", 'X');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT1", serviceGroup, "T3", '1');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT1", serviceGroup, "T3", '2');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT1", serviceGroup, "T3", 'X');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT3", serviceGroup, "T2", '1');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT3", serviceGroup, "T2", '2');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT3", serviceGroup, "T2", 'X');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT3", serviceGroup, "T1", '1');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT3", serviceGroup, "T1", '2');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT3", serviceGroup, "T1", 'X');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT3", serviceGroup, "T3", '1');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT3", serviceGroup, "T3", '2');
        *ret += getSCI(vendor, carrier, serviceTypeCode, "TT3", serviceGroup, "T3", 'X');
        return *ret;
      }
      else if (serviceGroup == "C1")
      {
        // four different sub codes
        *ret += getSCI(vendor, carrier, serviceTypeCode, "SS1", serviceGroup, "", '1', 'C'),
            getSCI(vendor, carrier, serviceTypeCode, "SS1", serviceGroup, "", '2', 'C'),
            getSCI(vendor, carrier, serviceTypeCode, "SS2", serviceGroup, "", '1', 'I'),
            getSCI(vendor, carrier, serviceTypeCode, "SS1", serviceGroup, "D", '1', 'C');
        return *ret;
      }
      return DataHandleMock::getSubCode(vendor, carrier, serviceTypeCode, serviceGroup, date);
    }
    const std::vector<OptionalServicesConcur*>&
    getOptionalServicesConcur(const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const ServiceTypeCode& serviceTypeCode,
                              const DateTime& date)
    {
      std::vector<OptionalServicesConcur*>* ret =
          _memHandle.create<std::vector<OptionalServicesConcur*> >();
      if (carrier == "AA")
      {
        *ret += getS6(serviceTypeCode, "SS3", "XX", "", "UA", 'M', 'Y'),
            getS6(serviceTypeCode, "SS3", "S3", "XX", "UA", 'M', 'Y'),
            getS6(serviceTypeCode, "XXX", "S3", "", "UA", 'M', 'Y'),
            getS6(serviceTypeCode, "SS3", "S3", "", "UA", 'M', 'Y'),
            getS6(serviceTypeCode, "SS3", "", "", "$$", 'E', 'Y'),
            getS6(serviceTypeCode, "", "S3", "", "CO", 'O', 'N');
        return *ret;
      }
      else if (carrier == "BB")
        return *ret;
      else if (carrier == "A1")
      {
        *ret += getS6(serviceTypeCode, "SS3", "", "", "$$", 'E', 'Y');
        return *ret;
      }
      else if (carrier == "A2")
      {
        *ret += getS6(serviceTypeCode, "SS1", "", "", "AA", 'E', 'N');
        *ret += getS6(serviceTypeCode, "SS1", "", "", "$$", 'E', 'Y');
        *ret += getS6(serviceTypeCode, "SS2", "", "", "AA", 'M', 'N');
        *ret += getS6(serviceTypeCode, "SS2", "", "", "$$", 'E', 'Y');
        return *ret;
      }
      else if (carrier == "A3")
      {
        *ret += getS6(serviceTypeCode, "SS1", "", "", "AA", 'E', 'Y');
        *ret += getS6(serviceTypeCode, "SS2", "", "", "$$", 'E', 'Y');
        return *ret;
      }

      return DataHandleMock::getOptionalServicesConcur(vendor, carrier, serviceTypeCode, date);
    }

    SubCodeInfo* getSCI(VendorCode vendor,
                        CarrierCode cxr,
                        ServiceTypeCode stc,
                        ServiceSubTypeCode sstc,
                        ServiceGroup sg,
                        ServiceGroup ssg,
                        Indicator concur,
                        Indicator ind = 'I')
    {
      SubCodeInfo* ret = _memHandle.create<SubCodeInfo>();
      ret->vendor() = vendor;
      ret->carrier() = cxr;
      ret->serviceTypeCode() = stc;
      ret->serviceSubTypeCode() = sstc;
      ret->serviceGroup() = sg;
      ret->serviceSubGroup() = ssg;
      ret->concur() = concur;
      ret->industryCarrierInd() = ind;
      return ret;
    }

    OptionalServicesConcur* getS6(ServiceTypeCode stc,
                                  ServiceSubTypeCode sstc,
                                  ServiceGroup sg,
                                  ServiceGroup ssg,
                                  CarrierCode acxr,
                                  Indicator mkop,
                                  Indicator concur)
    {
      OptionalServicesConcur* ret = _memHandle.create<OptionalServicesConcur>();
      ret->serviceTypeCode() = stc;
      ret->serviceSubTypeCode() = sstc;
      ret->serviceGroup() = sg;
      ret->serviceSubGroup() = ssg;
      ret->accessedCarrier() = acxr;
      ret->mkgOperFareOwner() = mkop;
      ret->concur() = concur;
      return ret;
    }
    TestMemHandle _memHandle;
  };

  CPPUNIT_TEST_SUITE(OptionalFeeConcurValidatorTest);
  CPPUNIT_TEST(testCheckMarkOperCxrs_SameMarketAndOper);
  CPPUNIT_TEST(testCheckMarkOperCxrs_DiffMarketAndOper);
  CPPUNIT_TEST(testCheckMarkOperCxrs_OneMarkFewOper);
  CPPUNIT_TEST(testCheckMarkOperCxrs_FewMarkFewOper);

  CPPUNIT_TEST(testInitializeOCMap_NoOCFeees);
  CPPUNIT_TEST(testInitializeOCMap_DiffOCFeees);
  CPPUNIT_TEST(testInitializeOCMap_SameOCFeees);
  CPPUNIT_TEST(testInitializeOCMap_CheckIndCxrInd);
  CPPUNIT_TEST(testGetConcurs_NoRecords);
  CPPUNIT_TEST(testGetConcurs_NoMatch);
  CPPUNIT_TEST(testGetConcurs_Match);
  CPPUNIT_TEST(testCollectS6_Find);
  CPPUNIT_TEST(testCollectS6_NoFind);
  CPPUNIT_TEST(testShouldProcessS5_Concur1);
  CPPUNIT_TEST(testShouldProcessS5_Concur2);
  CPPUNIT_TEST(testShouldProcessS5_ConcurX);
  CPPUNIT_TEST(testCollectS6_PassAllCarriers);
  CPPUNIT_TEST(testCollectS6_NoPassAllCarriers);

  CPPUNIT_TEST(testCanCarrierBeAssesed_UA_F_Y_Fail);
  CPPUNIT_TEST(testCanCarrierBeAssesed_UA_M_Y_Pass);
  CPPUNIT_TEST(testCanCarrierBeAssesed_UA_M_Y_FailCxr);
  CPPUNIT_TEST(testCanCarrierBeAssesed_UA_M_Y_FailOper);
  CPPUNIT_TEST(testCanCarrierBeAssesed_UA_M_N_FailConcur);
  CPPUNIT_TEST(testCanCarrierBeAssesed_UA_E_Y_PassMkt);
  CPPUNIT_TEST(testCanCarrierBeAssesed_UA_E_Y_PassOper);
  CPPUNIT_TEST(testCanCarrierBeAssesed_UA_O_Y_FailMkt);
  CPPUNIT_TEST(testCanCarrierBeAssesed_SS_E_Y_PassCxr);
  CPPUNIT_TEST(testCanCarrierBeAssesed_PassSet);
  CPPUNIT_TEST(testCanCarrierBeAssesed_FailAA);
  CPPUNIT_TEST(testCanCarrierBeAssesed_FailBB);

  CPPUNIT_TEST(testGetOptCarrierVec);
  CPPUNIT_TEST(testGetOptCarrierVec_sameMarketing);

  CPPUNIT_TEST(testCreateDiag_Create876);
  CPPUNIT_TEST(testCreateDiag_NoCreate875);
  CPPUNIT_TEST(testCreateDiag_NoRequestes);

  CPPUNIT_TEST(testValidate_MarketingWithDiag);
  CPPUNIT_TEST(testValidate_OperatingWithDiag);
  CPPUNIT_TEST(testStatusString_NotProcessed);
  CPPUNIT_TEST(testStatusString_Fail);
  CPPUNIT_TEST(testStatusString_NoS6ForCxr);
  CPPUNIT_TEST(testStatusString_NoMatchOnS6);
  CPPUNIT_TEST(testStatusString_NotAllowed);
  CPPUNIT_TEST(testStatusString_FareBuster);
  CPPUNIT_TEST(testStatusString_Pass);

  CPPUNIT_TEST(testDiagSortS5_CX);
  CPPUNIT_TEST(testDiagSortS5_SG);
  CPPUNIT_TEST(testDiagSortS5_SC);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _farePath = _memHandle.create<FarePath>();
    _trx = _memHandle.create<PricingTrx>();
    _validator = _memHandle.insert(new OptionalFeeConcurValidatorMock(*_trx, _farePath));
    _sGrp = _memHandle.create<ServiceFeesGroup>();
    _first = _memHandle.create<AirSeg>();
    _last = _memHandle.create<AirSeg>();
    _myDataHandle = _memHandle.create<MyDataHandle>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _multipleStrategy = _memHandle.create<MultipleSegmentStrategy>();
    _validator->_needValidation = true;
  }
  void tearDown() { _memHandle.clear(); }

  void testCheckMarkOperCxrs_SameMarketAndOper()
  {
    CPPUNIT_ASSERT(
        !_validator->checkMarkOperCxrs(*createCxrSet("UA"), *createCxrSet("UA"), _first, _last));
  }
  void testCheckMarkOperCxrs_DiffMarketAndOper()
  {
    CPPUNIT_ASSERT(
        _validator->checkMarkOperCxrs(*createCxrSet("LO"), *createCxrSet("UA"), _first, _last));
  }
  void testCheckMarkOperCxrs_OneMarkFewOper()
  {
    CPPUNIT_ASSERT(_validator->checkMarkOperCxrs(
        *createCxrSet("LO"), *createCxrSet("UA", "LO"), _first, _last));
  }
  void testCheckMarkOperCxrs_FewMarkFewOper()
  {
    CPPUNIT_ASSERT(_validator->checkMarkOperCxrs(
        *createCxrSet("LO", "BA"), *createCxrSet("UA", "LO"), _first, _last));
  }

  void testInitializeOCMap_NoOCFeees() // no records for group BL
  {
    initializeServiceGroup("AA", "BL");
    CPPUNIT_ASSERT(!_validator->initializeOCMap("AA", _sGrp, false, false));
  }
  void testInitializeOCMap_DiffOCFeees() // 4 different OC
  {
    initializeServiceGroup("AA", "S1");
    CPPUNIT_ASSERT(_validator->initializeOCMap("AA", _sGrp, false, false));
    CPPUNIT_ASSERT_EQUAL(size_t(4), _validator->_s5Map.size());
  }
  void testInitializeOCMap_SameOCFeees() // 2 OC with same S6Key
  {
    initializeServiceGroup("AA", "S2");
    CPPUNIT_ASSERT(_validator->initializeOCMap("AA", _sGrp, false, false));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _validator->_s5Map.size());
    CPPUNIT_ASSERT_EQUAL(size_t(2), _validator->_s5Map[0]->ocFees().size());
  }
  void testInitializeOCMap_CheckIndCxrInd()
  {
    initializeServiceGroup("AA", "C1");
    CPPUNIT_ASSERT(_validator->initializeOCMap("AA", _sGrp, true, false));
    CPPUNIT_ASSERT_EQUAL(size_t(1), _validator->_s5Map.size());
  }
  void testGetConcurs_NoRecords() // no rec for BB
  {
    SubCodeInfo* sci = _myDataHandle->getSCI("ATP", "AA", "OC", "SS3", "S3", "", '1');
    std::vector<OptionalServicesConcur*> ret;
    CPPUNIT_ASSERT(!_validator->getConcurs("BB", sci, ret, false));
  }
  void testGetConcurs_NoMatch() // no match on group BG
  {
    SubCodeInfo* sci = _myDataHandle->getSCI("ATP", "AA", "OC", "SS4", "BG", "", '1');
    std::vector<OptionalServicesConcur*> ret;
    CPPUNIT_ASSERT(!_validator->getConcurs("AA", sci, ret, false));
  }
  void testGetConcurs_Match() // should match 3 records
  {
    SubCodeInfo* sci = _myDataHandle->getSCI("ATP", "AA", "OC", "SS3", "S3", "", '1');
    std::vector<OptionalServicesConcur*> ret;
    CPPUNIT_ASSERT(_validator->getConcurs("AA", sci, ret, false));
    CPPUNIT_ASSERT_EQUAL(size_t(3), ret.size());
  }
  void testCollectS6_Find() // match for carrier AA
  {
    SubCodeInfo* sci = _myDataHandle->getSCI("ATP", "AA", "OC", "SS3", "S3", "", '1');
    CPPUNIT_ASSERT(_validator->collectS6(*createCxrSet("AA"), createS5Wraper(sci), false));
  }
  void testCollectS6_NoFind() // match for AA, fail for BB
  {
    SubCodeInfo* sci = _myDataHandle->getSCI("ATP", "AA", "OC", "SS3", "S3", "", '1');
    CPPUNIT_ASSERT(!_validator->collectS6(*createCxrSet("AA", "BB"), createS5Wraper(sci), false));
  }
  void testShouldProcessS5_Concur1()
  {
    SubCodeInfo* sci = _myDataHandle->getSCI("ATP", "AA", "OC", "SS3", "S3", "", '1');
    CPPUNIT_ASSERT_EQUAL(OptionalFeeConcurValidator::S5Required,
                         _validator->shouldProcessS5(sci, false));
  }
  void testShouldProcessS5_Concur2()
  {
    SubCodeInfo* sci = _myDataHandle->getSCI("ATP", "AA", "OC", "SS3", "S3", "", '2');
    CPPUNIT_ASSERT_EQUAL(OptionalFeeConcurValidator::S5NotAllowed,
                         _validator->shouldProcessS5(sci, false));
  }
  void testShouldProcessS5_ConcurX()
  {
    SubCodeInfo* sci = _myDataHandle->getSCI("ATP", "AA", "OC", "SS3", "S3", "", 'X');
    CPPUNIT_ASSERT_EQUAL(OptionalFeeConcurValidator::S5NoConcurence,
                         _validator->shouldProcessS5(sci, false));
  }
  void testCollectS6_PassAllCarriers() // match for A1 and AA
  {
    SubCodeInfo* sci = _myDataHandle->getSCI("ATP", "AA", "OC", "SS3", "S3", "", '1');
    CPPUNIT_ASSERT(_validator->collectS6(*createCxrSet("A1", "AA"), createS5Wraper(sci), false));
  }
  void testCollectS6_NoPassAllCarriers() // match A1, fail BB
  {
    SubCodeInfo* sci = _myDataHandle->getSCI("ATP", "AA", "OC", "SS3", "S3", "", '1');
    CPPUNIT_ASSERT(!_validator->collectS6(*createCxrSet("A1", "BB"), createS5Wraper(sci), false));
  }
  void testCanCarrierBeAssesed_UA_F_Y_Fail()
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* ret = createDataForAssess();
    ret->matchedS6()["AA"].push_back(_myDataHandle->getS6("", "", "", "", "UA", 'F', 'Y'));
    CPPUNIT_ASSERT_EQUAL(OptionalFeeConcurValidator::S6OCFeesWrapper::FAIL_FARE_BUSTER,
                         ret->canCarrierBeAssesed("UA", *createCxrSet("AA"), true));
  }
  void testCanCarrierBeAssesed_UA_M_Y_Pass()
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* ret = createDataForAssess();
    ret->matchedS6()["AA"].push_back(_myDataHandle->getS6("", "", "", "", "UA", 'M', 'Y'));
    CPPUNIT_ASSERT_EQUAL(OptionalFeeConcurValidator::S6OCFeesWrapper::CONCUR_PASS,
                         ret->canCarrierBeAssesed("UA", *createCxrSet("AA"), true));
  }
  void testCanCarrierBeAssesed_UA_M_Y_FailCxr() // fail - no match to BA
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* ret = createDataForAssess();
    ret->matchedS6()["AA"].push_back(_myDataHandle->getS6("", "", "", "", "UA", 'M', 'Y'));
    CPPUNIT_ASSERT_EQUAL(OptionalFeeConcurValidator::S6OCFeesWrapper::FAIL_NO_S6_MATCH,
                         ret->canCarrierBeAssesed("BA", *createCxrSet("AA"), true));
  }
  void testCanCarrierBeAssesed_UA_M_Y_FailOper() // for 'M' operating fail
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* ret = createDataForAssess();
    ret->matchedS6()["AA"].push_back(_myDataHandle->getS6("", "", "", "", "UA", 'M', 'Y'));
    CPPUNIT_ASSERT_EQUAL(OptionalFeeConcurValidator::S6OCFeesWrapper::FAIL_NO_S6_MATCH,
                         ret->canCarrierBeAssesed("UA", *createCxrSet("AA"), false));
  }
  void testCanCarrierBeAssesed_UA_M_N_FailConcur() // S6 concur = 'N' - fail
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* ret = createDataForAssess();
    ret->matchedS6()["AA"].push_back(_myDataHandle->getS6("", "", "", "", "UA", 'M', 'N'));
    CPPUNIT_ASSERT_EQUAL(OptionalFeeConcurValidator::S6OCFeesWrapper::FAIL_CONCUR_NOT_ALLOWED,
                         ret->canCarrierBeAssesed("UA", *createCxrSet("AA"), true));
  }
  void testCanCarrierBeAssesed_UA_E_Y_PassMkt() // on 'E' marketing pass
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* ret = createDataForAssess();
    ret->matchedS6()["AA"].push_back(_myDataHandle->getS6("", "", "", "", "UA", 'E', 'Y'));
    CPPUNIT_ASSERT_EQUAL(OptionalFeeConcurValidator::S6OCFeesWrapper::CONCUR_PASS,
                         ret->canCarrierBeAssesed("UA", *createCxrSet("AA"), true));
  }
  void testCanCarrierBeAssesed_UA_E_Y_PassOper() // on 'E' operating pass
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* ret = createDataForAssess();
    ret->matchedS6()["AA"].push_back(_myDataHandle->getS6("", "", "", "", "UA", 'E', 'Y'));
    CPPUNIT_ASSERT_EQUAL(OptionalFeeConcurValidator::S6OCFeesWrapper::CONCUR_PASS,
                         ret->canCarrierBeAssesed("UA", *createCxrSet("AA"), false));
  }
  void testCanCarrierBeAssesed_UA_O_Y_FailMkt() // on 'O' marketing fail
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* ret = createDataForAssess();
    ret->matchedS6()["AA"].push_back(_myDataHandle->getS6("", "", "", "", "UA", 'O', 'Y'));
    CPPUNIT_ASSERT_EQUAL(OptionalFeeConcurValidator::S6OCFeesWrapper::FAIL_NO_S6_MATCH,
                         ret->canCarrierBeAssesed("UA", *createCxrSet("AA"), true));
  }
  void testCanCarrierBeAssesed_SS_E_Y_PassCxr()
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* ret = createDataForAssess();
    ret->matchedS6()["AA"].push_back(_myDataHandle->getS6("", "", "", "", "$$", 'E', 'Y'));
    CPPUNIT_ASSERT_EQUAL(OptionalFeeConcurValidator::S6OCFeesWrapper::CONCUR_PASS,
                         ret->canCarrierBeAssesed("UA", *createCxrSet("AA"), false));
  }
  void testCanCarrierBeAssesed_PassSet()
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* ret = createDataForAssess();
    ret->matchedS6()["AA"] += _myDataHandle->getS6("", "", "", "", "DL", 'E', 'Y'),
        _myDataHandle->getS6("", "", "", "", "$$", 'M', 'Y');
    ret->matchedS6()["BB"] += _myDataHandle->getS6("", "", "", "", "$$", 'O', 'Y'),
        _myDataHandle->getS6("", "", "", "", "UA", 'E', 'Y');
    CPPUNIT_ASSERT_EQUAL(OptionalFeeConcurValidator::S6OCFeesWrapper::CONCUR_PASS,
                         ret->canCarrierBeAssesed("UA", *createCxrSet("AA", "BB"), true));
  }
  void testCanCarrierBeAssesed_FailAA()
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* ret = createDataForAssess();
    ret->matchedS6()["AA"] += _myDataHandle->getS6("", "", "", "", "DL", 'E', 'Y'),
        _myDataHandle->getS6("", "", "", "", "BA", 'M', 'Y');
    ret->matchedS6()["BB"] += _myDataHandle->getS6("", "", "", "", "$$", 'O', 'Y'),
        _myDataHandle->getS6("", "", "", "", "UA", 'E', 'Y');
    CPPUNIT_ASSERT_EQUAL(OptionalFeeConcurValidator::S6OCFeesWrapper::FAIL_NO_S6_MATCH,
                         ret->canCarrierBeAssesed("UA", *createCxrSet("AA", "BB"), true));
  }
  void testCanCarrierBeAssesed_FailBB()
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* ret = createDataForAssess();
    ret->matchedS6()["AA"] += _myDataHandle->getS6("", "", "", "", "DL", 'E', 'Y'),
        _myDataHandle->getS6("", "", "", "", "$$", 'M', 'Y');
    ret->matchedS6()["BB"] += _myDataHandle->getS6("", "", "", "", "$$", 'O', 'Y'),
        _myDataHandle->getS6("", "", "", "", "BA", 'E', 'Y');
    CPPUNIT_ASSERT_EQUAL(OptionalFeeConcurValidator::S6OCFeesWrapper::FAIL_NO_S6_MATCH,
                         ret->canCarrierBeAssesed("UA", *createCxrSet("AA", "BB"), true));
  }
  void testGetOptCarrierVec()
  {
    std::vector<TravelSeg*> tvSegs;
    tvSegs += getSeg("KRK", "WAW", "LO"), getSeg("WAW", "GLA", "BA"), getSeg("GLA", "LON", "BA"),
        getSeg("LON", "NYC", "AA"), getArunk("NYC", "WAS"), getSeg("WAS", "DFW", "UA");
    std::vector<CarrierCode> outCxrs;
    _validator->getOptCarrierVec(*createCxrSet("XX"), tvSegs.begin(), tvSegs.end(), outCxrs);
    CPPUNIT_ASSERT_EQUAL(size_t(4), outCxrs.size());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BA"), outCxrs[0]);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("AA"), outCxrs[1]);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LO"), outCxrs[2]);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("UA"), outCxrs[3]);
  }
  void testGetOptCarrierVec_sameMarketing()
  {
    std::vector<TravelSeg*> tvSegs;
    tvSegs += getSeg("KRK", "WAW", "DL"), getSeg("WAW", "KRK", "KL"), getSeg("KRK", "FRA", "KL"),
        getSeg("FRA", "LON", "BA"), getSeg("LON", "KRK", "BA"), getArunk("KRK", "WAW"),
        getSeg("WAW", "LON", "DL");
    std::vector<CarrierCode> outCxrs;
    _validator->getOptCarrierVec(*createCxrSet("KL"), tvSegs.begin(), tvSegs.end(), outCxrs);
    CPPUNIT_ASSERT_EQUAL(size_t(2), outCxrs.size());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("BA"), outCxrs[0]);
    CPPUNIT_ASSERT_EQUAL(CarrierCode("DL"), outCxrs[1]);
  }
  void testCreateDiag_Create876()
  {
    _trx->diagnostic().activate();
    _trx->diagnostic().diagnosticType() = Diagnostic876;
    _validator->createDiag();
    CPPUNIT_ASSERT(_validator->_diag876 != 0);
  }
  void testCreateDiag_NoCreate875()
  {
    _trx->diagnostic().activate();
    _trx->diagnostic().diagnosticType() = Diagnostic875;
    _validator->createDiag();
    CPPUNIT_ASSERT(_validator->_diag876 == 0);
  }
  void testCreateDiag_NoRequestes()
  {
    _trx->diagnostic().activate();
    _validator->createDiag();
    CPPUNIT_ASSERT(_validator->_diag876 == 0);
  }
  void
  testValidate_MarketingWithDiag() // expect to fail all records SS2 no for marketing, SS1 no for AA
  {
    _trx->diagnostic().activate();
    _trx->diagnostic().diagnosticType() = Diagnostic876;
    _trx->diagnostic().diagParamMap()["SC"] = "SS2";
    _validator->createDiag();
    _validator->_shouldDiagDisplay = true;
    initializeServiceGroup("AA", "S1");
    ServiceFeesGroup* retsg = _memHandle.create<ServiceFeesGroup>();
    CPPUNIT_ASSERT(!_validator->validateS6("AA", *createCxrSet("A2", "A3"), _sGrp, true, retsg));
    CPPUNIT_ASSERT_EQUAL(
        std::string("S5 RECORD:\n"
                    " CARRIER:    AA       VENDOR:      ATP     CONCUR:      1\n"
                    " SRVTYPE:    OC       SRVGROUP:    S1\n"
                    " SRVSUBTYPE: SS2      SRVSUBGROUP:   \n"
                    " SERVICE:          \n"
                    "  S6 RECORD:\n"
                    "   CARRIER:             SEQ:          0\n"
                    "   SRVTYPE:    OC       SRVGROUP:      \n"
                    "   SRVSUBTYPE: SS2      SRVSUBGROUP:   \n"
                    "    ASSESED CXR: AA  MKGOPER: M   CONCUR: N\n"
                    "  S6 RECORD:\n"
                    "   CARRIER:             SEQ:          0\n"
                    "   SRVTYPE:    OC       SRVGROUP:      \n"
                    "   SRVSUBTYPE: SS2      SRVSUBGROUP:   \n"
                    "    ASSESED CXR: $$  MKGOPER: E   CONCUR: Y\n"
                    "                                         ** S6 FOUND FOR S5**\n"
                    "  S6 RECORD:\n"
                    "   CARRIER:             SEQ:          0\n"
                    "   SRVTYPE:    OC       SRVGROUP:      \n"
                    "   SRVSUBTYPE: SS2      SRVSUBGROUP:   \n"
                    "    ASSESED CXR: $$  MKGOPER: E   CONCUR: Y\n"
                    "                                         ** S6 FOUND FOR S5**\n"
                    "MARKETING CARRIER THAT CAN BE ASSESED:\n"
                    " CX VENDR TYPE GROP SUBTYPE SUBGROP CONCUR               STATUS\n"
                    "***************************************************************\n"
                    "                            *** NO S5 PASS S6 VALIDATION ***\n"
                    "***************************************************************\n"),
        _validator->_diag876->str());
  }
  void testValidate_OperatingWithDiag() // expect to pass on subcode SS2 for operating , fail all
                                        // SS1
  {
    _trx->diagnostic().activate();
    _trx->diagnostic().diagnosticType() = Diagnostic876;
    _trx->diagnostic().diagParamMap()["SC"] = "SS2";
    _validator->createDiag();
    _validator->_shouldDiagDisplay = true;
    initializeServiceGroup("AA", "S1");
    ServiceFeesGroup* retsg = _memHandle.create<ServiceFeesGroup>();
    CPPUNIT_ASSERT(_validator->validateS6("AA", *createCxrSet("A2", "A3"), _sGrp, false, retsg));
    CPPUNIT_ASSERT_EQUAL(
        std::string("S5 RECORD:\n"
                    " CARRIER:    AA       VENDOR:      ATP     CONCUR:      1\n"
                    " SRVTYPE:    OC       SRVGROUP:    S1\n"
                    " SRVSUBTYPE: SS2      SRVSUBGROUP:   \n"
                    " SERVICE:          \n"
                    "  S6 RECORD:\n"
                    "   CARRIER:             SEQ:          0\n"
                    "   SRVTYPE:    OC       SRVGROUP:      \n"
                    "   SRVSUBTYPE: SS2      SRVSUBGROUP:   \n"
                    "    ASSESED CXR: AA  MKGOPER: M   CONCUR: N\n"
                    "  S6 RECORD:\n"
                    "   CARRIER:             SEQ:          0\n"
                    "   SRVTYPE:    OC       SRVGROUP:      \n"
                    "   SRVSUBTYPE: SS2      SRVSUBGROUP:   \n"
                    "    ASSESED CXR: $$  MKGOPER: E   CONCUR: Y\n"
                    "                                         ** S6 FOUND FOR S5**\n"
                    "  S6 RECORD:\n"
                    "   CARRIER:             SEQ:          0\n"
                    "   SRVTYPE:    OC       SRVGROUP:      \n"
                    "   SRVSUBTYPE: SS2      SRVSUBGROUP:   \n"
                    "    ASSESED CXR: $$  MKGOPER: E   CONCUR: Y\n"
                    "                                         ** S6 FOUND FOR S5**\n"
                    "OPERATING CARRIER THAT CAN BE ASSESED:\n"
                    " CX VENDR TYPE GROP SUBTYPE SUBGROP CONCUR               STATUS\n"
                    "***************************************************************\n"
                    " AA   ATP   OC   S1     SS2              1                 PASS\n"
                    "                                             ON SEQ          0\n"
                    "                                             ON SEQ          0\n"
                    "***************************************************************\n"),
        _validator->_diag876->str());
  }
  void testStatusString_NotProcessed()
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* sci = createDataForAssess();
    sci->status() = OptionalFeeConcurValidator::S6OCFeesWrapper::NOT_ROCESSED;
    CPPUNIT_ASSERT_EQUAL(std::string("NOT PROCESSED"), std::string(sci->statusString()));
  }
  void testStatusString_Fail()
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* sci = createDataForAssess();
    sci->status() = OptionalFeeConcurValidator::S6OCFeesWrapper::FAIL_CONCUR;
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL CONCUR"), std::string(sci->statusString()));
  }
  void testStatusString_NoS6ForCxr()
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* sci = createDataForAssess();
    sci->status() = OptionalFeeConcurValidator::S6OCFeesWrapper::FAIL_NO_S6;
    CPPUNIT_ASSERT_EQUAL(std::string("NO S6 FOR CXR"), std::string(sci->statusString()));
  }
  void testStatusString_NoMatchOnS6()
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* sci = createDataForAssess();
    sci->status() = OptionalFeeConcurValidator::S6OCFeesWrapper::FAIL_NO_S6_MATCH;
    CPPUNIT_ASSERT_EQUAL(std::string("NO MATCH ON S6"), std::string(sci->statusString()));
  }
  void testStatusString_NotAllowed()
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* sci = createDataForAssess();
    sci->status() = OptionalFeeConcurValidator::S6OCFeesWrapper::FAIL_CONCUR_NOT_ALLOWED;
    CPPUNIT_ASSERT_EQUAL(std::string("NOT ALLOWED"), std::string(sci->statusString()));
  }
  void testStatusString_FareBuster()
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* sci = createDataForAssess();
    sci->status() = OptionalFeeConcurValidator::S6OCFeesWrapper::FAIL_FARE_BUSTER;
    CPPUNIT_ASSERT_EQUAL(std::string("FAIL FARE BUSTER"), std::string(sci->statusString()));
  }
  void testStatusString_Pass()
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* sci = createDataForAssess();
    sci->status() = OptionalFeeConcurValidator::S6OCFeesWrapper::CONCUR_PASS;
    CPPUNIT_ASSERT_EQUAL(std::string("PASS"), std::string(sci->statusString()));
  }
  void testStatusString_PassNoConcur()
  {
    OptionalFeeConcurValidator::S6OCFeesWrapper* sci = createDataForAssess();
    sci->status() = OptionalFeeConcurValidator::S6OCFeesWrapper::CONCUR_PASS_NO_CONCUR;
    CPPUNIT_ASSERT_EQUAL(std::string("NO CONCUR REQUIRED"), std::string(sci->statusString()));
  }

  void testDiagSortS5_CX()
  {
    createTestS5Set();
    _trx->diagnostic().diagParamMap()["SO"] = "CX";
    CPPUNIT_ASSERT(_validator->initializeOCMap("T1", _sGrp, false, true));
    CPPUNIT_ASSERT_EQUAL(size_t(3 * 3 * 27), _validator->_s5Map.size());
    for (int i = 0; i < 3 * 27; i++)
      CPPUNIT_ASSERT_EQUAL(CarrierCode("T1"), _validator->_s5Map[i]->carrier());
  }
  void testDiagSortS5_SG()
  {
    createTestS5Set();
    _trx->diagnostic().diagParamMap()["SO"] = "SG";
    CPPUNIT_ASSERT(_validator->initializeOCMap("T1", _sGrp, false, true));
    CPPUNIT_ASSERT_EQUAL(size_t(3 * 3 * 27), _validator->_s5Map.size());
    for (int i = 0; i < 3 * 27; i++)
      CPPUNIT_ASSERT_EQUAL(ServiceGroup("T1"), _validator->_s5Map[i]->serviceGroup());
  }
  void testDiagSortS5_SC()
  {
    createTestS5Set();
    _trx->diagnostic().diagParamMap()["SO"] = "SC";
    CPPUNIT_ASSERT(_validator->initializeOCMap("T1", _sGrp, false, true));
    CPPUNIT_ASSERT_EQUAL(size_t(3 * 3 * 27), _validator->_s5Map.size());
    for (int i = 0; i < 3 * 27; i++)
      CPPUNIT_ASSERT_EQUAL(ServiceSubTypeCode("TT1"), _validator->_s5Map[i]->serviceSubTypeCode());
  }

protected:
  std::set<CarrierCode>* createCxrSet(const char* cxr, const char* cxr2 = 0)
  {
    std::set<CarrierCode>* ret = _memHandle.create<std::set<CarrierCode> >();
    ret->insert(cxr);
    if (cxr2)
      ret->insert(cxr2);
    return ret;
  }
  void initializeServiceGroup(CarrierCode cxr, ServiceGroup srvGroup)
  {
    boost::bind<void>(
        ServiceFeesGroup::SubCodeInitializer(*_trx, _farePath, _first, _last, *_multipleStrategy),
        _1,
        _2,
        _3,
        _4,
        _5)(_sGrp, cxr, "OC", srvGroup, DateTime::localTime());
  }
  OptionalFeeConcurValidator::S6OCFeesWrapper* createS5Wraper(SubCodeInfo* sci)
  {
    OCFees* ocfee = _memHandle.create<OCFees>();
    ocfee->subCodeInfo() = sci;
    OptionalFeeConcurValidator::S6OCFeesWrapper* ret =
        _memHandle.create<OptionalFeeConcurValidator::S6OCFeesWrapper>();
    ret->ocFees().push_back(ocfee);
    return ret;
  }
  OptionalFeeConcurValidator::S6OCFeesWrapper* createDataForAssess()
  {
    SubCodeInfo* sci = _myDataHandle->getSCI("ATP", "AA", "OC", "SS3", "S3", "", '1');
    return createS5Wraper(sci);
  }
  AirSeg* getSeg(LocCode orig, LocCode dest, CarrierCode cxr)
  {
    AirSeg* ret = _memHandle.create<AirSeg>();
    ret->origin() = _myDataHandle->getLoc(orig, DateTime::localTime());
    ret->destination() = _myDataHandle->getLoc(dest, DateTime::localTime());
    ret->carrier() = cxr;
    ret->setOperatingCarrierCode(cxr);
    return ret;
  }
  ArunkSeg* getArunk(LocCode orig, LocCode dest)
  {
    ArunkSeg* ret = _memHandle.create<ArunkSeg>();
    ret->origin() = _myDataHandle->getLoc(orig, DateTime::localTime());
    ret->destination() = _myDataHandle->getLoc(dest, DateTime::localTime());
    return ret;
  }
  void createTestS5Set()
  {
    initializeServiceGroup("T1", "T1");
    initializeServiceGroup("T1", "T2");
    initializeServiceGroup("T1", "T3");
    initializeServiceGroup("T2", "T1");
    initializeServiceGroup("T2", "T2");
    initializeServiceGroup("T2", "T3");
    initializeServiceGroup("T3", "T1");
    initializeServiceGroup("T3", "T2");
    initializeServiceGroup("T3", "T3");
  }

private:
  TestMemHandle _memHandle;
  OptionalFeeConcurValidatorMock* _validator;
  FarePath* _farePath;
  PricingTrx* _trx;
  ServiceFeesGroup* _sGrp;
  TravelSeg* _first;
  TravelSeg* _last;
  MyDataHandle* _myDataHandle;
  MultipleSegmentStrategy* _multipleStrategy;
};
CPPUNIT_TEST_SUITE_REGISTRATION(OptionalFeeConcurValidatorTest);
}
