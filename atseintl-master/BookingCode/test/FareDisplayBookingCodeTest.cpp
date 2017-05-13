#include "BookingCode/test/FareDisplayBookingCodeExceptionTest.h"

#include "BookingCode/FareDisplayBookingCodeRB.h"
#include "BookingCode/RBData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/DiscountInfo.h"
#include "DataModel/PaxTypeFare.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestPaxTypeFareFactory.h"
#include "test/testdata/TestAirSegFactory.h"
#include "test/testdata/TestDiscountInfoFactory.h"

#include <vector>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>

namespace tse
{
class FareDisplayBookingCodeMock : public FareDisplayBookingCodeRB
{
public:
  FareDisplayBookingCodeMock(RBData* rbData, FareDisplayTrx* trx)
    : FareDisplayBookingCodeRB(rbData),
      _requestedItemNo(-1),
      _requestedConventionNo(' '),
      _isT999validation(false)
  {
    _trx = trx;
  };
  bool validateT999(PaxTypeFare& paxTfare, std::vector<BookingCode>& bkgCodes)
  {
    FareDisplayBookingCodeExceptionMock fdbce(_trx, &paxTfare, _rbData);
    bool ret = fdbce.getBookingCodeException(bkgCodes);
    _requestedItemNo = fdbce.requestedItemNo();
    _requestedVendorCode = fdbce.requestedVendorCode();
    _requestedCarrierCode = fdbce.requestedCarrierCode();
    _requestedTariffNumber = fdbce.requestedTariffNumber();
    _requestedRuleNumber = fdbce.requestedRuleNumber();
    _requestedConventionNo = fdbce.requestedConventionNo();
    _isT999validation = fdbce.isT999validation();
    return ret;
  }
  bool getBookingCodeException(PaxTypeFare& paxTypeFare,
                               VendorCode vendorA,
                               AirSeg* airSeg,
                               std::vector<BookingCode>& bkgCodes,
                               bool convention)
  {
    FareDisplayBookingCodeExceptionMock fdbce(_trx, &paxTypeFare, _rbData);
    bool ret = fdbce.getBookingCodeException(vendorA, bkgCodes, airSeg, convention);
    _requestedItemNo = fdbce.requestedItemNo();
    _requestedVendorCode = fdbce.requestedVendorCode();
    _requestedCarrierCode = fdbce.requestedCarrierCode();
    _requestedTariffNumber = fdbce.requestedTariffNumber();
    _requestedRuleNumber = fdbce.requestedRuleNumber();
    _requestedConventionNo = fdbce.requestedConventionNo();
    _isT999validation = fdbce.isT999validation();
    return ret;
  };

  int& requestedItemNo() { return _requestedItemNo; }
  VendorCode& requestedVendorCode() { return _requestedVendorCode; }
  CarrierCode& requestedCarrierCode() { return _requestedCarrierCode; }
  TariffNumber& requestedTariffNumber() { return _requestedTariffNumber; }
  RuleNumber& requestedRuleNumber() { return _requestedRuleNumber; }
  Indicator& requestedConventionNo() { return _requestedConventionNo; }
  bool& isT999validation() { return _isT999validation; }

  virtual const std::vector<CarrierApplicationInfo*>&
  getCarrierApplication(const VendorCode& vendor, int carrierApplTblItemNo) const
  {
    return _carrierApplInfo;
  }
  std::vector<CarrierApplicationInfo*> _carrierApplInfo;

private:
  int _requestedItemNo;
  VendorCode _requestedVendorCode;
  CarrierCode _requestedCarrierCode;
  TariffNumber _requestedTariffNumber;
  RuleNumber _requestedRuleNumber;
  Indicator _requestedConventionNo;
  bool _isT999validation;
};

class FareDisplayBookingCodeTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareDisplayBookingCodeTest);
  CPPUNIT_TEST(testFBRFare_T999);
  CPPUNIT_TEST(testFBRFare_Conv2);
  CPPUNIT_TEST(testFBRFare_R1B);
  CPPUNIT_TEST(testFBRFare2_Primary);
  CPPUNIT_TEST(testFBRFare2_Secondary);
  CPPUNIT_TEST(testValidateIndustryFareBkgCodeNoT990);
  CPPUNIT_TEST(testValidateIndustryFareBkgCodeNoMatchT990);
  CPPUNIT_TEST(testValidateIndustryFareBkgCodeMatchT990);
  CPPUNIT_TEST(testFindCXRNoCarrierApplication);
  CPPUNIT_TEST(testFindCXRFailOnCarrier);
  CPPUNIT_TEST(testFindCXRFailOnApplInd);
  CPPUNIT_TEST(testFindCXRMatchOnCarrier);
  CPPUNIT_TEST(testFindCXRMacthOnAnyCarrier);
  CPPUNIT_TEST(testIsYYCarrierRequestEmpty);
  CPPUNIT_TEST(testIsYYCarrierRequestNoMatch);
  CPPUNIT_TEST(testIsYYCarrierRequestMacth);
  CPPUNIT_TEST(testGetPrimeBookingCodeNoR1B);
  CPPUNIT_TEST(testGetPrimeBookingCodeInFareNoIndCarrier);
  CPPUNIT_TEST(testGetPrimeBookingCodeInFareIndCarrier);
  CPPUNIT_TEST(testGetPrimeBookingCodeMultiR1BNoT990);
  CPPUNIT_TEST(testGetPrimeBookingCodeMultiR1BT990);
  CPPUNIT_TEST(testGetPrimeBookingCodeMultiR1BPaxType);
  CPPUNIT_TEST(testGetPrimeBookingCodeMultiR1BMainR1B);
  CPPUNIT_TEST(testValidateFBR_RBDInternational_PrimaryDiscNoT999NoR1B);
  CPPUNIT_TEST(testValidateFBR_RBDInternational_PrimaryDiscNoT999WithR1B);
  CPPUNIT_TEST(testValidateFBR_RBDInternational_PrimaryDiscNoT999WithR3);
  CPPUNIT_TEST(testValidateFBR_RBDInternational_PrimaryDiscWithT999NoR1B);
  CPPUNIT_TEST(testValidateFBR_RBDInternational_PrimaryDiscWithT999WithR1B);
  CPPUNIT_TEST(testValidateFBR_RBDInternational_PrimaryDiscWithT999WithR3);
  CPPUNIT_TEST(testValidateFBR_RBDInternational_SecondaryDiscNoT999);
  CPPUNIT_TEST(testValidateFBR_RBDInternational_SecondaryDiscWithT999);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _ptftemp = 0; // will be initialized when used
    _fcai = 0; // will be initialized when used
    _rbData = _memHandle.create<RBData>();
    _trx = _memHandle.create<FareDisplayTrx>();
    _options = _memHandle.create<FareDisplayOptions>();
    _request = _memHandle.create<FareDisplayRequest>();
    _fdbc = _memHandle.insert(new FareDisplayBookingCodeMock(_rbData, _trx));
    _trx->setOptions(_options);
    _options->allCarriers() = 'N';
    _trx->setRequest(_request);
    _request->requestType() = "RB";
    _rbData->setSecondary(false);
    _rbData->setAirSeg(0);
    _rbData->setCarrierMatchedTable990(false);
    _memHandle.create<TestConfigInitializer>();
  }
  ////////////////////////////////////////////////////////////////////////////////
  void tearDown() { _memHandle.clear(); }
  ////////////////////////////////////////////////////////////////////////////////
  void testFBRFare_T999()
  {
    // FBR with T999, apply from T999
    _ptf = TestPaxTypeFareFactory::create("/vobs/atseintl/BookingCode/test/testdata/FBR1.xml");
    _rbData->setAirSeg(dynamic_cast<AirSeg*>(_ptf->fareMarket()->travelSeg().front()));
    BookingCode bce;
    _fdbc->getBookingCode(*_trx, *_ptf, bce);
    CPPUNIT_ASSERT_EQUAL(BookingCode("P$"), bce);
    CPPUNIT_ASSERT_EQUAL(626236, _fdbc->requestedItemNo());
    CPPUNIT_ASSERT_EQUAL(RuleNumber(""), _fdbc->requestedRuleNumber());
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testFBRFare_Conv2()
  {
    // FBR without T999, no match on cat25 rec2, get BCE base fare conv2("P")
    _ptf = TestPaxTypeFareFactory::create("/vobs/atseintl/BookingCode/test/testdata/FBR1.xml");
    _rbData->setAirSeg(dynamic_cast<AirSeg*>(_ptf->fareMarket()->travelSeg().front()));
    FareClassAppSegInfo* fcseg = const_cast<FareClassAppSegInfo*>(_ptf->fareClassAppSegInfo());
    fcseg->_bookingCodeTblItemNo = 0;
    BookingCode bce;
    _fdbc->getBookingCode(*_trx, *_ptf, bce);
    CPPUNIT_ASSERT_EQUAL(BookingCode("A$"), bce);
    CPPUNIT_ASSERT_EQUAL(RuleNumber("FRAR"), _fdbc->requestedRuleNumber());
    CPPUNIT_ASSERT_EQUAL(27, _fdbc->requestedTariffNumber());
    CPPUNIT_ASSERT_EQUAL('2', _fdbc->requestedConventionNo());
    CPPUNIT_ASSERT_EQUAL(-1, _fdbc->requestedItemNo());
    fcseg->_bookingCodeTblItemNo = 626236;
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testFBRFare_R1B()
  {
    // FBR without T999, match on cat25 rec 2, get BC form rec 1B
    _ptf = TestPaxTypeFareFactory::create("/vobs/atseintl/BookingCode/test/testdata/FBR1.xml");
    _rbData->setAirSeg(dynamic_cast<AirSeg*>(_ptf->fareMarket()->travelSeg().front()));
    FareClassAppSegInfo* fcseg = const_cast<FareClassAppSegInfo*>(_ptf->fareClassAppSegInfo());
    fcseg->_bookingCodeTblItemNo = 0;
    PaxTypeFareRuleData* fbrPTFare = _ptf->paxTypeFareRuleData(25);
    CategoryRuleInfo* fbrRuleInfo = const_cast<CategoryRuleInfo*>(fbrPTFare->categoryRuleInfo());
    fbrRuleInfo->carrierCode() = "JJ";
    BookingCode bce;
    // FBR without T999, no match on cat25 rec2, get BCE base fare conv2("P")
    _fdbc->getBookingCode(*_trx, *_ptf, bce);
    CPPUNIT_ASSERT_EQUAL(BookingCode("P"), bce);
    CPPUNIT_ASSERT_EQUAL(RuleNumber(""), _fdbc->requestedRuleNumber());
    CPPUNIT_ASSERT_EQUAL(-1, _fdbc->requestedItemNo());
    fcseg->_bookingCodeTblItemNo = 626236;
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testFBRFare2_Primary()
  {
    // check primary booking code (A is from Rec1B)
    _ptf = TestPaxTypeFareFactory::create("testdata/BCE1/PTF_Constr.xml");
    _rbData->setAirSeg(dynamic_cast<AirSeg*>(_ptf->fareMarket()->travelSeg().front()));
    BookingCode bce;
    _fdbc->getBookingCode(*_trx, *_ptf, bce);
    CPPUNIT_ASSERT_EQUAL(BookingCode("A"), bce);
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testFBRFare2_Secondary()
  {
    _ptf = TestPaxTypeFareFactory::create("testdata/BCE1/PTF_Constr.xml");
    BookingCode bce;
    // secondary segment validation. Secondary segment filled with T999, but there is no match
    // should return BCE='L' from LG conv 1
    _rbData->setSecondary(true);
    _rbData->setAirSeg(TestAirSegFactory::create("testdata/BCE1/SecondarySeg.xml"));
    _fdbc->getBookingCode(*_trx, *_ptf, bce);
    CPPUNIT_ASSERT_EQUAL(BookingCode("L"), bce);
    CPPUNIT_ASSERT_EQUAL('1', _fdbc->requestedConventionNo());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LG"), _fdbc->requestedCarrierCode());
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testValidateIndustryFareBkgCodeNoT990()
  {
    _ptf = TestPaxTypeFareFactory::create("testdata/BCE1/PTF_Ind.xml");
    std::vector<BookingCode> bkgCodes;
    // no T990, fail
    CPPUNIT_ASSERT_EQUAL(false, _fdbc->validateIndustryFareBkgCode(*_ptf, bkgCodes));
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testValidateIndustryFareBkgCodeNoMatchT990()
  {
    _ptf = TestPaxTypeFareFactory::create("testdata/BCE1/PTF_Ind.xml");
    std::vector<BookingCode> bkgCodes;
    // T990, but no match on CarrierApplicationInfo
    (const_cast<FareClassAppSegInfo*>(_ptf->fareClassAppSegInfo()))->_carrierApplTblItemNo = 1;
    CPPUNIT_ASSERT_EQUAL(false, _fdbc->validateIndustryFareBkgCode(*_ptf, bkgCodes));
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testValidateIndustryFareBkgCodeMatchT990()
  {
    _ptf = TestPaxTypeFareFactory::create("testdata/BCE1/PTF_Ind.xml");
    std::vector<BookingCode> bkgCodes;
    CarrierApplicationInfo cai;
    (const_cast<FareClassAppSegInfo*>(_ptf->fareClassAppSegInfo()))->_carrierApplTblItemNo = 1;
    // T990, match CarrierApplicationInfo
    cai.applInd() = '$';
    _fdbc->_carrierApplInfo.push_back(&cai);
    CPPUNIT_ASSERT_EQUAL(true, _fdbc->validateIndustryFareBkgCode(*_ptf, bkgCodes));
    CPPUNIT_ASSERT_EQUAL(size_t(1), bkgCodes.size());
    CPPUNIT_ASSERT_EQUAL(BookingCode("C"), bkgCodes[0]);
    (const_cast<FareClassAppSegInfo*>(_ptf->fareClassAppSegInfo()))->_carrierApplTblItemNo = 0;
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testFindCXRNoCarrierApplication()
  {
    // no carrier application, fail validation
    CPPUNIT_ASSERT_EQUAL(false, _fdbc->findCXR("AA", _fdbc->_carrierApplInfo));
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testFindCXRFailOnCarrier()
  {
    CarrierApplicationInfo cai;
    cai.applInd() = 'A';
    cai.carrier() = "AB";
    _fdbc->_carrierApplInfo.push_back(&cai);
    CPPUNIT_ASSERT_EQUAL(false, _fdbc->findCXR("AA", _fdbc->_carrierApplInfo));
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testFindCXRFailOnApplInd()
  {
    CarrierApplicationInfo cai;
    cai.applInd() = 'X';
    cai.carrier() = "AA";
    _fdbc->_carrierApplInfo.push_back(&cai);
    CPPUNIT_ASSERT_EQUAL(false, _fdbc->findCXR("AA", _fdbc->_carrierApplInfo));
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testFindCXRMatchOnCarrier()
  {
    CarrierApplicationInfo cai;
    cai.applInd() = 'A';
    cai.carrier() = "AA";
    _fdbc->_carrierApplInfo.push_back(&cai);
    _rbData->setCarrierMatchedTable990(false);
    CPPUNIT_ASSERT_EQUAL(true, _fdbc->findCXR("AA", _fdbc->_carrierApplInfo));
    CPPUNIT_ASSERT_EQUAL(true, _rbData->isCarrierMatchedTable990());
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testFindCXRMacthOnAnyCarrier()
  {
    CarrierApplicationInfo cai;
    cai.applInd() = '$';
    cai.carrier() = "YY";
    _fdbc->_carrierApplInfo.push_back(&cai);
    _rbData->setCarrierMatchedTable990(false);
    CPPUNIT_ASSERT_EQUAL(true, _fdbc->findCXR("AA", _fdbc->_carrierApplInfo));
    CPPUNIT_ASSERT_EQUAL(false, _rbData->isCarrierMatchedTable990());
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testIsYYCarrierRequestEmpty()
  {
    std::set<CarrierCode> rqstedCxrs;
    // empty set
    CPPUNIT_ASSERT_EQUAL(false, _fdbc->isYYCarrierRequest(rqstedCxrs));
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testIsYYCarrierRequestNoMatch()
  {
    std::set<CarrierCode> rqstedCxrs;
    rqstedCxrs.insert("AA");
    rqstedCxrs.insert("ZZ");
    CPPUNIT_ASSERT_EQUAL(false, _fdbc->isYYCarrierRequest(rqstedCxrs));
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testIsYYCarrierRequestMacth()
  {
    std::set<CarrierCode> rqstedCxrs;
    rqstedCxrs.insert("AA");
    rqstedCxrs.insert("ZZ");
    rqstedCxrs.insert("YY");
    CPPUNIT_ASSERT_EQUAL(true, _fdbc->isYYCarrierRequest(rqstedCxrs));
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testGetPrimeBookingCodeNoR1B()
  {
    std::vector<BookingCode> bookingCodeVec;
    PaxTypeFare tempPtf;
    CPPUNIT_ASSERT_EQUAL(false, _fdbc->getPrimeBookingCode(tempPtf, bookingCodeVec));
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testGetPrimeBookingCodeInFareNoIndCarrier()
  {
    std::vector<BookingCode> bookingCodeVec;

    // industry fare without T990, with not industry carrier in fare market
    _ptf = TestPaxTypeFareFactory::create("testdata/BCE1/PTF_Ind.xml");
    (const_cast<FareMarket*>(_ptf->fareMarket()))->governingCarrier() = "AA";
    CPPUNIT_ASSERT_EQUAL(false, _fdbc->getPrimeBookingCode(*_ptf, bookingCodeVec));
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testGetPrimeBookingCodeInFareIndCarrier()
  {
    std::vector<BookingCode> bookingCodeVec;
    _ptf = TestPaxTypeFareFactory::create("testdata/BCE1/PTF_Ind.xml");
    // match on industry carrier, one R1B, return BookingCode1 = "C"
    (const_cast<FareMarket*>(_ptf->fareMarket()))->governingCarrier() = "YY";
    CPPUNIT_ASSERT_EQUAL(true, _fdbc->getPrimeBookingCode(*_ptf, bookingCodeVec));
    CPPUNIT_ASSERT_EQUAL(size_t(1), bookingCodeVec.size());
    CPPUNIT_ASSERT_EQUAL(BookingCode("C"), bookingCodeVec[0]);
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testGetPrimeBookingCodeMultiR1BNoT990()
  {
    std::vector<BookingCode> bookingCodeVec;
    // Loop throuhg all PTF R1Bs, no T990, return all BCE's
    CPPUNIT_ASSERT_EQUAL(true, _fdbc->getPrimeBookingCode(*getHelperTestFare(), bookingCodeVec));
    CPPUNIT_ASSERT_EQUAL(size_t(3), bookingCodeVec.size());
    CPPUNIT_ASSERT_EQUAL(BookingCode("A"), bookingCodeVec[0]);
    CPPUNIT_ASSERT_EQUAL(BookingCode("B"), bookingCodeVec[1]);
    CPPUNIT_ASSERT_EQUAL(BookingCode("C"), bookingCodeVec[2]);
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testGetPrimeBookingCodeMultiR1BT990()
  {
    std::vector<BookingCode> bookingCodeVec;
    getHelperTestFareClasAppInfo()->_segs[0]->_carrierApplTblItemNo = 1;
    // Loop throuhg all PTF R1Bs, first not matched on T990
    CarrierApplicationInfo cai;
    cai.applInd() = 'X';
    cai.carrier() = "YY";
    _fdbc->_carrierApplInfo.push_back(&cai);
    CPPUNIT_ASSERT_EQUAL(true, _fdbc->getPrimeBookingCode(*getHelperTestFare(), bookingCodeVec));
    CPPUNIT_ASSERT_EQUAL(size_t(1), bookingCodeVec.size());
    CPPUNIT_ASSERT_EQUAL(BookingCode("C"), bookingCodeVec[0]);
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testGetPrimeBookingCodeMultiR1BPaxType()
  {
    std::vector<BookingCode> bookingCodeVec;
    getHelperTestFareClasAppInfo()->_segs[0]->_paxType = "ADT";
    CPPUNIT_ASSERT_EQUAL(true, _fdbc->getPrimeBookingCode(*getHelperTestFare(), bookingCodeVec));
    CPPUNIT_ASSERT_EQUAL(size_t(1), bookingCodeVec.size());
    CPPUNIT_ASSERT_EQUAL(BookingCode("C"), bookingCodeVec[0]);
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testGetPrimeBookingCodeMultiR1BMainR1B()
  {
    std::vector<BookingCode> bookingCodeVec;
    getHelperTestFareClasAppInfo()->_segs[0]->_carrierApplTblItemNo = 1;
    getHelperTestFareClasAppInfo()->_segs[1]->_carrierApplTblItemNo = 1;
    CarrierApplicationInfo cai;
    cai.applInd() = 'X';
    cai.carrier() = "YY";
    _fdbc->_carrierApplInfo.push_back(&cai);
    // no match on on T990, last try on mailn R1B
    CPPUNIT_ASSERT_EQUAL(true, _fdbc->getPrimeBookingCode(*getHelperTestFare(), bookingCodeVec));
    CPPUNIT_ASSERT_EQUAL(size_t(1), bookingCodeVec.size());
    CPPUNIT_ASSERT_EQUAL(BookingCode("C"), bookingCodeVec[0]);
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testValidateFBR_RBDInternational_PrimaryDiscNoT999NoR1B()
  {
    // no T999, no R1B, no R3, return false to use Conv2
    createDiscountFare();
    const_cast<FareClassAppSegInfo*>(_ptf->fareClassAppSegInfo())->_bookingCode[0] = "";
    std::vector<BookingCode> bce;
    CPPUNIT_ASSERT(!_fdbc->validateFBR_RBDInternational(*_ptf, bce, false));
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testValidateFBR_RBDInternational_PrimaryDiscNoT999WithR1B()
  {
    // no T999, get from R1B
    createDiscountFare();
    std::vector<BookingCode> bce;
    CPPUNIT_ASSERT(_fdbc->validateFBR_RBDInternational(*_ptf, bce, false));
    CPPUNIT_ASSERT_EQUAL(BookingCode("D"), bce[0]);
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testValidateFBR_RBDInternational_PrimaryDiscNoT999WithR3()
  {
    // no T999, R3 take precedence before R1B
    createDiscountFare("E");
    std::vector<BookingCode> bce;
    CPPUNIT_ASSERT(_fdbc->validateFBR_RBDInternational(*_ptf, bce, false));
    CPPUNIT_ASSERT_EQUAL(BookingCode("E"), bce[0]);
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testValidateFBR_RBDInternational_PrimaryDiscWithT999NoR1B()
  {
    // T999 exists, No R1B, No R3, take BC from base fare
    createDiscountFare();
    const_cast<FareClassAppSegInfo*>(_ptf->fareClassAppSegInfo())->_bookingCode[0] = "";
    std::vector<BookingCode> bce;
    CPPUNIT_ASSERT(_fdbc->validateFBR_RBDInternational(*_ptf, bce, true));
    CPPUNIT_ASSERT_EQUAL(BookingCode("C"), bce[0]);
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testValidateFBR_RBDInternational_PrimaryDiscWithT999WithR1B()
  {
    // T999 exists, R1B, No R3, take BC from base fare
    createDiscountFare();
    std::vector<BookingCode> bce;
    CPPUNIT_ASSERT(_fdbc->validateFBR_RBDInternational(*_ptf, bce, true));
    CPPUNIT_ASSERT_EQUAL(BookingCode("D"), bce[0]);
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testValidateFBR_RBDInternational_PrimaryDiscWithT999WithR3()
  {
    // T999 exists, R1B, R3 take precedence
    createDiscountFare("E");
    std::vector<BookingCode> bce;
    CPPUNIT_ASSERT(_fdbc->validateFBR_RBDInternational(*_ptf, bce, true));
    CPPUNIT_ASSERT_EQUAL(BookingCode("E"), bce[0]);
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testValidateFBR_RBDInternational_SecondaryDiscNoT999()
  {
    // No T999 exists, validate using conv 2
    _rbData->setSecondary(true);
    _rbData->setAirSeg(TestAirSegFactory::create("testdata/BCE1/SecondarySeg.xml"));
    createDiscountFare();
    std::vector<BookingCode> bce;
    CPPUNIT_ASSERT(!_fdbc->validateFBR_RBDInternational(*_ptf, bce, false));
  }
  ////////////////////////////////////////////////////////////////////////////////
  void testValidateFBR_RBDInternational_SecondaryDiscWithT999()
  {
    // T999 exists, validate using conv 1
    _rbData->setSecondary(true);
    _rbData->setAirSeg(TestAirSegFactory::create("testdata/BCE1/SecondarySeg.xml"));
    createDiscountFare();
    std::vector<BookingCode> bce;
    CPPUNIT_ASSERT(_fdbc->validateFBR_RBDInternational(*_ptf, bce, true));
    CPPUNIT_ASSERT_EQUAL(BookingCode("C"), bce[0]);
    CPPUNIT_ASSERT_EQUAL('1', _fdbc->requestedConventionNo());
    CPPUNIT_ASSERT_EQUAL(CarrierCode("LG"), _fdbc->requestedCarrierCode());
  }

protected:
  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////
  ////                      Helping functions                                 ////
  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////
  void createDiscountFare(const char* discInfoBC = "")
  {
    _ptf = TestPaxTypeFareFactory::create(
        "/vobs/atseintl/BookingCode/test/testdata/BCE2/PTF_FBR_Disc.xml");
    DiscountInfo* discInfo = TestDiscountInfoFactory::create("testdata/BCE2/PTF_FBR_DiscInfo.xml");
    discInfo->bookingCode() = discInfoBC;
    PaxTypeFareRuleData* diskPTfare = _ptf->paxTypeFareRuleData(19);
    diskPTfare->ruleItemInfo() = discInfo;
    _ptf->status().set(PaxTypeFare::PTF_Discounted, true);
    const_cast<FareClassAppSegInfo*>(_ptf->fareClassAppSegInfo())->_bookingCode[0] = "D";
  }

  PaxTypeFare* getHelperTestFare()
  {
    if (_ptftemp)
      return _ptftemp;
    _ptftemp = _memHandle.create<PaxTypeFare>();
    _ptftemp->fareClassAppInfo() = getHelperTestFareClasAppInfo();
    _ptftemp->fareClassAppSegInfo() = _ptftemp->fareClassAppInfo()->_segs[1];
    PaxTypeFare* p = TestPaxTypeFareFactory::create("testdata/BCE1/PTF_Ind.xml");
    _ptftemp->setFare(p->fare());
    _ptftemp->fareMarket() = p->fareMarket();
    return _ptftemp;
  }
  FareClassAppInfo* getHelperTestFareClasAppInfo()
  {
    if (_fcai)
      return _fcai;
    _fcai = _memHandle.create<FareClassAppInfo>();
    _fcai->_segs.push_back(new FareClassAppSegInfo); // will be deleted in fca destructor
    _fcai->_segs.push_back(new FareClassAppSegInfo);
    _fcai->_segs[0]->_bookingCode[0] = "A";
    _fcai->_segs[0]->_bookingCode[1] = "B";
    _fcai->_segs[1]->_bookingCode[0] = "C";
    _fcai->_segs[0]->_carrierApplTblItemNo = 0;
    _fcai->_segs[1]->_carrierApplTblItemNo = 0;
    return _fcai;
  }

  PaxTypeFare* _ptf;
  RBData* _rbData;
  FareDisplayBookingCodeMock* _fdbc;
  FareDisplayTrx* _trx;
  FareDisplayOptions* _options;
  FareDisplayRequest* _request;
  PaxTypeFare* _ptftemp;
  FareClassAppInfo* _fcai;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareDisplayBookingCodeTest);
}; // namespace
