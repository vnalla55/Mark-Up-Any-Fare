
#include "Common/Config/ConfigMan.h"
#include "Common/MetricsMan.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/Diag852Collector.h"
#include "Diagnostic/test/Diag852ParsedParamsTester.h"
#include "FreeBagService/BaggageTravelInfo.h"
#include "FreeBagService/ChargesDataStrategy.h"
#include "FreeBagService/test/BaggageDataBuilders.h"
#include "FreeBagService/test/BaggageTravelBuilder.h"
#include "FreeBagService/test/S5Builder.h"
#include "FreeBagService/test/S7Builder.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

#include <boost/assign/std/vector.hpp>
#include <boost/lexical_cast.hpp>

namespace tse
{

using boost::assign::operator+=;

class ChargesDataStrategyTest : public CppUnit::TestFixture
{
  // MOCKS
  class MyDataHandle : public DataHandleMock
  {
  public:
    MyDataHandle() : _subCodes(0) {}

    const std::vector<SubCodeInfo*>& getSubCode(const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const ServiceTypeCode& /*serviceTypeCode*/,
                                                const ServiceGroup& /*serviceGroup*/,
                                                const DateTime& /*date*/)
    {
      std::vector<SubCodeInfo*>* ret = _memHandle.create<std::vector<SubCodeInfo*> >();
      if (_subCodes)
      {
        for (SubCodeInfo* subCodeInfo : *_subCodes)
        {
          if ((subCodeInfo->vendor() == vendor) && (subCodeInfo->carrier() == carrier))
            ret->push_back(subCodeInfo);
        }
      }

      return *ret;
    }

    void setSubCode(const std::vector<SubCodeInfo*>* subCodes) { _subCodes = subCodes; }

  private:
    TestMemHandle _memHandle;
    const std::vector<SubCodeInfo*>* _subCodes;
  };

  struct CheckSubCodeInfo : std::unary_function<const SubCodeInfo*, bool>
  {
    bool _isInterLine;

    CheckSubCodeInfo(bool isInterLine) : _isInterLine(isInterLine) {}

    bool operator()(const SubCodeInfo* s5) const
    {
      bool result = (s5->fltTktMerchInd() == BAGGAGE_CHARGE) && s5->serviceSubGroup().empty() &&
                    isNumeric(s5->description1());

      if (_isInterLine)
        result = result && s5->industryCarrierInd() == 'I';

      return result;
    }

    bool isNumeric(const ServiceGroupDescription& description) const
    {
      bool ret(true);
      try { boost::lexical_cast<short>(description); }
      catch (boost::bad_lexical_cast&) { ret = false; }
      return ret;
    }
  };

  class TestDiagCollector : public Diag852Collector
  {
  public:
    TestDiagCollector() : _diagParamsHandler(Diag852Collector::_params)
    {
      rootDiag() = _memHandle.create<Diagnostic>();
    }

    TestDiagCollector(Diag852Collector::DiagType diagType)
      : _diagParamsHandler(Diag852Collector::_params)
    {
      rootDiag() = _memHandle.create<Diagnostic>();
      _diagParamsHandler.updateDiagType(diagType);
    }

    void setDiagType(Diag852Collector::DiagType diagType)
    {
      _diagParamsHandler.updateDiagType(diagType);
    }

    void addParam(const std::string& key, const std::string& value)
    {
      rootDiag()->diagParamMap().insert(std::make_pair(key, value));
      _diagParamsHandler.forceInitialization();
    }

  private:
    TestMemHandle _memHandle;
    Diag852ParsedParamsTester _diagParamsHandler;
  };

  CPPUNIT_TEST_SUITE(ChargesDataStrategyTest);

  CPPUNIT_TEST(testRetrieveS5Records1st_AllowanceS7Present);
  CPPUNIT_TEST(testRetrieveS5Records1st_NoS7Marketing);
  CPPUNIT_TEST(testRetrieveS5Records1st_NoS7Operating);
  CPPUNIT_TEST(testRetrieveS5Records1st_NoS7NonUsDot);

  CPPUNIT_TEST(testRetrieveS5Records2nd_EmptyVector);
  CPPUNIT_TEST(testRetrieveS5Records2nd_Atp);
  CPPUNIT_TEST(testRetrieveS5Records2nd_Atp_Interline);
  CPPUNIT_TEST(testRetrieveS5Records2nd_Mm);
  CPPUNIT_TEST(testRetrieveS5Records2nd_Mm_Interline);

  CPPUNIT_TEST(testRetrieveS5Records3rd_EmptyVector);
  CPPUNIT_TEST(testRetrieveS5Records3rd_Match1st);
  CPPUNIT_TEST(testRetrieveS5Records3rd_Match2nd);
  CPPUNIT_TEST(testRetrieveS5Records3rd_MatchBoth);
  CPPUNIT_TEST(testRetrieveS5Records3rd_FailSubGroup);
  CPPUNIT_TEST(testRetrieveS5Records3rd_FailSvcType);
  CPPUNIT_TEST(testRetrieveS5Records3rd_FailDesc);

  CPPUNIT_TEST(testShouldDisplayS7Diagnostic_True);
  CPPUNIT_TEST(testShouldDisplayS7Diagnostic_False);

  CPPUNIT_TEST(testShouldDisplayChargesDiagnostic_True);
  CPPUNIT_TEST(testShouldDisplayChargesDiagnostic_FalseDc);
  CPPUNIT_TEST(testShouldDisplayChargesDiagnostic_FalseFl);

  CPPUNIT_TEST(testShouldDisplayDiagnostic_True_NoS7);
  CPPUNIT_TEST(testShouldDisplayDiagnostic_True_St);
  CPPUNIT_TEST(testShouldDisplayDiagnostic_False_Fl);
  CPPUNIT_TEST(testShouldDisplayDiagnostic_False_St);

  CPPUNIT_TEST(testFindLowestCharges_Empty);
  CPPUNIT_TEST(testFindLowestCharges_BothBags_FindLowest1st);
  CPPUNIT_TEST(testFindLowestCharges_BothBags_FindLowest1st_MixedOccurrence);
  CPPUNIT_TEST(testFindLowestCharges_BothBags_Fail1stOccurrence);
  CPPUNIT_TEST(testFindLowestCharges_BothBags_FindLowest2nd_SameAs1st);
  CPPUNIT_TEST(testFindLowestCharges_BothBags_FindLowest2nd_SameStcAs1st);
  CPPUNIT_TEST(testFindLowestCharges_BothBags_FindLowest2nd_DifferentStcThan1st);
  CPPUNIT_TEST(testFindLowestCharges_BothBags_FindLowest2nd_No1st);
  CPPUNIT_TEST(testFindLowestCharges_2ndOnly);
  CPPUNIT_TEST(testFindLowestCharges_NoAvailNoCharges_X);

  CPPUNIT_TEST(testMatchOccurrence_Both_Blank_Both_Pass_First_Bag);
  CPPUNIT_TEST(testMatchOccurrence_Both_Blank_Both_Pass_Second_Bag);

  CPPUNIT_TEST(testGetAllowanceS7_Null);
  CPPUNIT_TEST(testGetAllowanceS7_NotNull);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _mdh = _memHandle.create<MyDataHandle>();
    _itin = _memHandle.create<Itin>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->itin() += _itin;
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _strategy = _memHandle.insert(new ChargesDataStrategy(*_trx));
  }

  void tearDown()
  {
    _memHandle.clear();
    MockGlobal::clear();
  }

protected:
  SubCodeInfo* createRecordS5(const ServiceGroupDescription& desc = "99",
                              const Indicator& srvType = 'C',
                              const ServiceGroup& subGroup = "",
                              const VendorCode& vendor = ATPCO_VENDOR_CODE,
                              const CarrierCode& carrier = "AA",
                              const Indicator& industryCarrierInd = ' ')
  {
    return S5Builder(&_memHandle)
        .withVendCarr(vendor, carrier)
        .withFltTktMerchInd(srvType)
        .withSubGroup(subGroup)
        .withDesc(desc)
        .withIndustryCarrier(industryCarrierInd)
        .build();
  }

  void addAirSegToItin(const CarrierCode& marketingCxr, const CarrierCode& operatingCxr)
  {
    _itin->travelSeg() +=
        AirSegBuilder(_memHandle).withCxr(marketingCxr, operatingCxr).build();
  }

  BaggageTravel* createBaggageTravel(OCFees* ocfees = 0)
  {
    FarePath* farePath = _memHandle.create<FarePath>();
    _itin->farePath() += farePath;
    farePath->itin() = _itin;
    return BaggageTravelBuilder(&_memHandle).withFarePath(farePath).withAllowance(ocfees).build();
  }

  OptionalServicesInfo* createS7(int occ1st, int occLast, const ServiceSubTypeCode& subCode = "")
  {
    return S7Builder(&_memHandle)
        .withBaggageOccurrence(occ1st, occLast)
        .withSubTypeCode(subCode)
        .build();
  }

  OptionalServicesInfo* createS7(int occ1st,
                                 int occLast,
                                 const ServiceSubTypeCode& subCode,
                                 const Indicator& noAvailNoCharges)
  {
    return S7Builder(&_memHandle)
        .withBaggageOccurrence(occ1st, occLast)
        .withSubTypeCode(subCode)
        .withNotAvailNoCharge(noAvailNoCharges)
        .build();
  }

  std::vector<BaggageTravel*>* create1ItemBaggageTravelVector()
  {
    std::vector<BaggageTravel*>* bagTvls = _memHandle.create<std::vector<BaggageTravel*> >();
    *bagTvls += createBaggageTravel();
    return bagTvls;
  }

  BaggageCharge* createBaggageCharge(OptionalServicesInfo* s7 = 0,
                                     const CurrencyCode& currency = "",
                                     const MoneyAmount amount = 0)
  {
    BaggageCharge* charge = _memHandle.create<BaggageCharge>();
    charge->optFee() = s7;
    charge->feeCurrency() = currency;
    charge->feeAmount() = amount;
    charge->setMatched1stBag(true);
    charge->setMatched2ndBag(true);
    return charge;
  }

  OCFees* createOCFees(OptionalServicesInfo* s7 = 0) { return createBaggageCharge(s7); }

  MyDataHandle* _mdh;
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  Itin* _itin;
  ChargesDataStrategy* _strategy;

public:
  // TESTS
  void testRetrieveS5Records1st_AllowanceS7Present()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5();
    _mdh->setSubCode(&subCodesIn);

    BaggageTravel* baggageTravel =
        createBaggageTravel(createOCFees(S7Builder(&_memHandle).withCarrier("AA").build()));

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records(baggageTravel, true, subCodesOut);

    CPPUNIT_ASSERT_EQUAL((size_t)1, subCodesOut.size());
  }

  void testRetrieveS5Records1st_NoS7Marketing()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5();
    _mdh->setSubCode(&subCodesIn);

    addAirSegToItin("AA", "UA");

    FarePath* farePath = _memHandle.create<FarePath>();
    _itin->farePath() += farePath;
    farePath->itin() = _itin;

    BaggageTravel* baggageTravel = BaggageTravelBuilder(&_memHandle)
                                       .withFarePath(farePath)
                                       .withCarrierTravelSeg(*_itin->travelSeg().begin())
                                       .build();

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records(baggageTravel, true, subCodesOut);

    CPPUNIT_ASSERT_EQUAL((size_t)0, subCodesOut.size());
  }

  void testRetrieveS5Records1st_NoS7Operating()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5();
    _mdh->setSubCode(&subCodesIn);

    addAirSegToItin("UA", "AA");

    FarePath* farePath = _memHandle.create<FarePath>();
    _itin->farePath() += farePath;
    farePath->itin() = _itin;

    BaggageTravel* baggageTravel = BaggageTravelBuilder(&_memHandle)
                                       .withFarePath(farePath)
                                       .withCarrierTravelSeg(*_itin->travelSeg().begin())
                                       .build();

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records(baggageTravel, true, subCodesOut);

    CPPUNIT_ASSERT_EQUAL((size_t)0, subCodesOut.size());
  }

  void testRetrieveS5Records1st_NoS7NonUsDot()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5();
    _mdh->setSubCode(&subCodesIn);

    addAirSegToItin("AA", "UA");

    FarePath* farePath = _memHandle.create<FarePath>();
    _itin->farePath() += farePath;
    farePath->itin() = _itin;

    BaggageTravel* baggageTravel = BaggageTravelBuilder(&_memHandle)
                                       .withFarePath(farePath)
                                       .withMSSJourney(_itin->travelSeg().begin())
                                       .build();

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records(baggageTravel, false, subCodesOut);

    CPPUNIT_ASSERT_EQUAL((size_t)0, subCodesOut.size());
  }

  void testRetrieveS5Records2nd_EmptyVector()
  {
    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records("XX", subCodesOut, CheckSubCodeInfo(true));

    CPPUNIT_ASSERT_EQUAL((size_t)0, subCodesOut.size());
  }

  void testRetrieveS5Records2nd_Atp()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5("99", 'C', "", ATPCO_VENDOR_CODE, "AA");
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records("AA", subCodesOut, CheckSubCodeInfo(false));

    CPPUNIT_ASSERT_EQUAL((size_t)1, subCodesOut.size());
    CPPUNIT_ASSERT_EQUAL((VendorCode)ATPCO_VENDOR_CODE, subCodesOut.front()->vendor());
  }

  void testRetrieveS5Records2nd_Atp_Interline()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5("99", 'C', "", ATPCO_VENDOR_CODE, "AA", 'I');
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records("AA", subCodesOut, CheckSubCodeInfo(true));

    CPPUNIT_ASSERT_EQUAL((size_t)1, subCodesOut.size());
    CPPUNIT_ASSERT_EQUAL((VendorCode)ATPCO_VENDOR_CODE, subCodesOut.front()->vendor());
  }

  void testRetrieveS5Records2nd_Mm()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5("99", 'C', "", MERCH_MANAGER_VENDOR_CODE, "AA");
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records("AA", subCodesOut, CheckSubCodeInfo(false));

    CPPUNIT_ASSERT_EQUAL((size_t)1, subCodesOut.size());
    CPPUNIT_ASSERT_EQUAL((VendorCode)MERCH_MANAGER_VENDOR_CODE, subCodesOut.front()->vendor());
  }

  void testRetrieveS5Records2nd_Mm_Interline()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5("99", 'C', "", MERCH_MANAGER_VENDOR_CODE, "AA", 'I');
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records("AA", subCodesOut, CheckSubCodeInfo(true));

    CPPUNIT_ASSERT_EQUAL((size_t)1, subCodesOut.size());
    CPPUNIT_ASSERT_EQUAL((VendorCode)MERCH_MANAGER_VENDOR_CODE, subCodesOut.front()->vendor());
  }

  void testRetrieveS5Records3rd_EmptyVector()
  {
    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records("XX", subCodesOut, CheckSubCodeInfo(false));

    CPPUNIT_ASSERT_EQUAL((size_t)0, subCodesOut.size());
  }

  void testRetrieveS5Records3rd_Match1st()
  {
    ServiceGroupDescription desc = "01";
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5(desc);
    subCodesIn += createRecordS5("XX");
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records("AA", subCodesOut, CheckSubCodeInfo(false));

    CPPUNIT_ASSERT_EQUAL((size_t)1, subCodesOut.size());
    CPPUNIT_ASSERT_EQUAL(desc, subCodesOut.front()->description1());
  }

  void testRetrieveS5Records3rd_Match2nd()
  {
    ServiceGroupDescription desc = "02";
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5("XX");
    subCodesIn += createRecordS5(desc);
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records("AA", subCodesOut, CheckSubCodeInfo(false));

    CPPUNIT_ASSERT_EQUAL((size_t)1, subCodesOut.size());
    CPPUNIT_ASSERT_EQUAL(desc, subCodesOut.front()->description1());
  }

  void testRetrieveS5Records3rd_MatchBoth()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5("01");
    subCodesIn += createRecordS5("02");
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records("AA", subCodesOut, CheckSubCodeInfo(false));

    CPPUNIT_ASSERT_EQUAL((size_t)1, subCodesOut.size());
    CPPUNIT_ASSERT_EQUAL((ServiceGroupDescription) "01", subCodesOut[0]->description1());
  }

  void testRetrieveS5Records3rd_FailSubGroup()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5("99", 'C', "XX");
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records("AA", subCodesOut, CheckSubCodeInfo(true));

    CPPUNIT_ASSERT_EQUAL((size_t)0, subCodesOut.size());
  }

  void testRetrieveS5Records3rd_FailSvcType()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5("99", 'I');
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records("AA", subCodesOut, CheckSubCodeInfo(true));

    CPPUNIT_ASSERT_EQUAL((size_t)0, subCodesOut.size());
  }

  void testRetrieveS5Records3rd_FailDesc()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn += createRecordS5("XX");
    _mdh->setSubCode(&subCodesIn);

    std::vector<const SubCodeInfo*> subCodesOut;

    _strategy->retrieveS5Records("AA", subCodesOut, CheckSubCodeInfo(true));

    CPPUNIT_ASSERT_EQUAL((size_t)0, subCodesOut.size());
  }

  void testShouldDisplayS7Diagnostic_True()
  {
    TestDiagCollector diag(Diag852Collector::STACTIVE);
    CPPUNIT_ASSERT(_strategy->shouldDisplayS7Diagnostic(&diag));
  }

  void testShouldDisplayS7Diagnostic_False()
  {
    TestDiagCollector diag(Diag852Collector::CPACTIVE);
    CPPUNIT_ASSERT(!_strategy->shouldDisplayS7Diagnostic(&diag));
  }

  void testShouldDisplayChargesDiagnostic_True()
  {
    TestDiagCollector diag;
    diag.addParam("FL", "01");
    diag.addParam("CP", "01");
    diag.addParam("DC", "");
    std::vector<BaggageTravel*>* baggageTravels = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(_strategy->shouldDisplayChargesDiagnostic(
        (*baggageTravels)[0], BaggageTravelInfo(0, 0), &diag));
  }

  void testShouldDisplayChargesDiagnostic_FalseDc()
  {
    TestDiagCollector diag;
    diag.addParam("FL", "01");
    diag.addParam("CP", "01");
    std::vector<BaggageTravel*>* baggageTravels = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(!_strategy->shouldDisplayChargesDiagnostic(
        (*baggageTravels)[0], BaggageTravelInfo(0, 0), &diag));
  }

  void testShouldDisplayChargesDiagnostic_FalseFl()
  {
    TestDiagCollector diag;
    diag.addParam("FL", "02");
    diag.addParam("CP", "01");
    diag.addParam("DC", "");
    std::vector<BaggageTravel*>* baggageTravels = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(!_strategy->shouldDisplayChargesDiagnostic(
        (*baggageTravels)[0], BaggageTravelInfo(0, 0), &diag));
  }

  void testShouldDisplayDiagnostic_True_NoS7()
  {
    TestDiagCollector diag;
    diag.addParam("FL", "01");
    diag.addParam("CP", "01");
    diag.addParam("DC", "");
    std::vector<BaggageTravel*>* baggageTravels = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(_strategy->shouldDisplayDiagnostic(
        (*baggageTravels)[0], BaggageTravelInfo(0, 0), "", &diag));
  }

  void testShouldDisplayDiagnostic_True_St()
  {
    TestDiagCollector diag;
    diag.addParam("FL", "01");
    diag.addParam("CP", "01");
    diag.addParam("DC", "");
    std::string subTypeCode("ABC");
    diag.addParam("ST", subTypeCode);
    std::vector<BaggageTravel*>* baggageTravels = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(_strategy->shouldDisplayDiagnostic(
        (*baggageTravels)[0], BaggageTravelInfo(0, 0), subTypeCode, &diag));
  }

  void testShouldDisplayDiagnostic_False_Fl()
  {
    TestDiagCollector diag;
    diag.addParam("FL", "02");
    diag.addParam("CP", "01");
    diag.addParam("DC", "");
    std::string subTypeCode("ABC");
    diag.addParam("ST", subTypeCode);
    std::vector<BaggageTravel*>* baggageTravels = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(!_strategy->shouldDisplayDiagnostic(
        (*baggageTravels)[0], BaggageTravelInfo(0, 0), subTypeCode, &diag));
  }

  void testShouldDisplayDiagnostic_False_St()
  {
    TestDiagCollector diag;
    diag.addParam("FL", "01");
    diag.addParam("CP", "01");
    diag.addParam("DC", "");
    diag.addParam("ST", "ABC");
    std::vector<BaggageTravel*>* baggageTravels = create1ItemBaggageTravelVector();

    CPPUNIT_ASSERT(!_strategy->shouldDisplayDiagnostic(
        (*baggageTravels)[0], BaggageTravelInfo(0, 0), "XXX", &diag));
  }

  void testFindLowestCharges_Empty()
  {
    BaggageTravel* bt = createBaggageTravel();
    ChargeVector charges;

    _strategy->findLowestCharges(bt, BaggageTravelInfo(0, 0), charges, 0);

    const auto notNull = std::find_if(bt->_charges.begin(),
                                      bt->_charges.end(),
                                      [](const BaggageCharge* bc)
                                      { return bc != nullptr; });
    CPPUNIT_ASSERT_EQUAL(ptrdiff_t(MAX_BAG_PIECES), notNull - bt->_charges.begin());
  }

  void testFindLowestCharges_BothBags_FindLowest1st()
  {
    BaggageTravel* bt = createBaggageTravel();
    ChargeBuilder bld(_memHandle);
    ChargeVector charges;
    charges += bld.withS7(createS7(1, 1)).forBags(0).withAmt(100, "USD").build();
    charges += bld.withS7(createS7(1, 1)).forBags(0).withAmt(50, "USD").build();
    charges += bld.withS7(createS7(1, 1)).forBags(0).withAmt(150, "USD").build();

    _strategy->findLowestCharges(bt, BaggageTravelInfo(0, 0), charges, 0);

    CPPUNIT_ASSERT(bt->_charges[0]);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"), bt->_charges[0]->feeCurrency());
    CPPUNIT_ASSERT_EQUAL(50.0, bt->_charges[0]->feeAmount());
  }

  void testFindLowestCharges_BothBags_FindLowest1st_MixedOccurrence()
  {
    BaggageTravel* bt = createBaggageTravel();
    ChargeBuilder bld(_memHandle);
    ChargeVector charges;
    charges += bld.withS7(createS7(1, 1)).forBags(0).withAmt(100, "USD").build();
    charges += bld.withS7(createS7(2, 2)).forBags(1).withAmt(50, "USD").build();
    charges += bld.withS7(createS7(1, 1)).forBags(0).withAmt(150, "USD").build();

    _strategy->findLowestCharges(bt, BaggageTravelInfo(0, 0), charges, 0);

    CPPUNIT_ASSERT(bt->_charges[0]);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"), bt->_charges[0]->feeCurrency());
    CPPUNIT_ASSERT_EQUAL(100.0, bt->_charges[0]->feeAmount());
  }

  void testFindLowestCharges_BothBags_Fail1stOccurrence()
  {
    BaggageTravel* bt = createBaggageTravel();
    ChargeBuilder bld(_memHandle);
    ChargeVector charges;
    charges += bld.withS7(createS7(2, 2)).forBags(1).withAmt(100, "USD").build();
    charges += bld.withS7(createS7(2, 2)).forBags(1).withAmt(50, "USD").build();
    charges += bld.withS7(createS7(2, 2)).forBags(1).withAmt(150, "USD").build();

    _strategy->findLowestCharges(bt, BaggageTravelInfo(0, 0), charges, 0);

    CPPUNIT_ASSERT(!bt->_charges[0]);
  }

  void testFindLowestCharges_BothBags_FindLowest2nd_SameAs1st()
  {
    BaggageTravel* bt = createBaggageTravel();
    ChargeBuilder bld(_memHandle);
    ChargeVector charges;
    charges += bld.withS7(createS7(1, 2)).forBags(0, 1).withAmt(100, "USD").build();
    charges += bld.withS7(createS7(2, 2)).forBags(1).withAmt(50, "USD").build();
    charges += bld.withS7(createS7(1, 2)).forBags(0, 1).withAmt(150, "USD").build();

    _strategy->findLowestCharges(bt, BaggageTravelInfo(0, 0), charges, 0);

    CPPUNIT_ASSERT(bt->_charges[0]);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"), bt->_charges[0]->feeCurrency());
    CPPUNIT_ASSERT_EQUAL(100.00, bt->_charges[0]->feeAmount());

    CPPUNIT_ASSERT(bt->_charges[1]);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"), bt->_charges[1]->feeCurrency());
    CPPUNIT_ASSERT_EQUAL(50.0, bt->_charges[1]->feeAmount());
  }

  void testFindLowestCharges_BothBags_FindLowest2nd_SameStcAs1st()
  {
    BaggageTravel* bt = createBaggageTravel();
    SubCodeInfo* qweS5 = createRecordS5("99", 'C', "QWE");
    SubCodeInfo* asdS5 = createRecordS5("99", 'C', "ASD");
    ChargeBuilder bld(_memHandle);
    ChargeVector charges;
    charges += bld.withS7(createS7(1, 1)).withS5(qweS5).forBags(0).withAmt(100, "USD").build();
    charges += bld.withS7(createS7(2, 2)).withS5(qweS5).forBags(1).withAmt(150, "USD").build();
    charges += bld.withS7(createS7(2, 2)).withS5(asdS5).forBags(1).withAmt(50, "USD").build();

    _strategy->findLowestCharges(bt, BaggageTravelInfo(0, 0), charges, 0);

    CPPUNIT_ASSERT(bt->_charges[0]);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"), bt->_charges[0]->feeCurrency());
    CPPUNIT_ASSERT_EQUAL(100.0, bt->_charges[0]->feeAmount());

    CPPUNIT_ASSERT(bt->_charges[1]);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"), bt->_charges[1]->feeCurrency());
    CPPUNIT_ASSERT_EQUAL(50.0, bt->_charges[1]->feeAmount());
  }

  void testFindLowestCharges_BothBags_FindLowest2nd_DifferentStcThan1st()
  {
    BaggageTravel* bt = createBaggageTravel();
    SubCodeInfo* qweS5 = createRecordS5("99", 'C', "QWE");
    SubCodeInfo* asdS5 = createRecordS5("99", 'C', "ASD");
    ChargeBuilder bld(_memHandle);
    ChargeVector charges;
    charges += bld.withS7(createS7(1, 1)).withS5(qweS5).forBags(0).withAmt(100, "USD").build();
    charges += bld.withS7(createS7(2, 2)).withS5(asdS5).forBags(1).withAmt(150, "USD").build();
    charges += bld.withS7(createS7(2, 2)).withS5(asdS5).forBags(1).withAmt(50, "USD").build();

    _strategy->findLowestCharges(bt, BaggageTravelInfo(0, 0), charges, 0);

    CPPUNIT_ASSERT(bt->_charges[0]);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"), bt->_charges[0]->feeCurrency());
    CPPUNIT_ASSERT_EQUAL(100.0, bt->_charges[0]->feeAmount());

    CPPUNIT_ASSERT(bt->_charges[1]);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"), bt->_charges[1]->feeCurrency());
    CPPUNIT_ASSERT_EQUAL(50.0, bt->_charges[1]->feeAmount());
  }

  void testFindLowestCharges_BothBags_FindLowest2nd_No1st()
  {
    BaggageTravel* bt = createBaggageTravel();
    ChargeBuilder bld(_memHandle);
    ChargeVector charges;
    charges += bld.withS7(createS7(2, 2)).forBags(1).withAmt(100, "USD").build();
    charges += bld.withS7(createS7(2, 2)).withAmt(50, "USD").build();
    charges += bld.withS7(createS7(2, 2)).forBags(1).withAmt(150, "USD").build();

    _strategy->findLowestCharges(bt, BaggageTravelInfo(0, 0), charges, 0);

    CPPUNIT_ASSERT(!bt->_charges[0]);

    CPPUNIT_ASSERT(bt->_charges[1]);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"), bt->_charges[1]->feeCurrency());
    CPPUNIT_ASSERT_EQUAL(100.0, bt->_charges[1]->feeAmount());
  }

  void testFindLowestCharges_2ndOnly()
  {
    BaggageTravel* bt = createBaggageTravel();
    bt->_allowance = createOCFees(S7Builder(&_memHandle).withBaggagePcs(1).build());
    ChargeBuilder bld(_memHandle);
    ChargeVector charges;
    charges += bld.withS7(createS7(2, 2)).forBags(1).withAmt(150, "USD").build();
    charges += bld.withS7(createS7(2, 2)).forBags(1).withAmt(60, "USD").build();
    charges += bld.withS7(createS7(2, 2)).forBags(1).withAmt(100, "USD").build();
    charges += bld.withS7(createS7(1, 1)).forBags(0).withAmt(50, "USD").build();

    _strategy->findLowestCharges(bt, BaggageTravelInfo(0, 0), charges, 0);

    CPPUNIT_ASSERT(!bt->_charges[0]);

    CPPUNIT_ASSERT(bt->_charges[1]);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"), bt->_charges[1]->feeCurrency());
    CPPUNIT_ASSERT_EQUAL(60.0, bt->_charges[1]->feeAmount());
  }

  void testFindLowestCharges_NoAvailNoCharges_X()
  {
    BaggageTravel* bt = createBaggageTravel();
    ChargeBuilder bld(_memHandle);
    ChargeVector charges;
    charges += bld.withS7(createS7(1, 1)).forBags(0).withAmt(100, "USD").build();
    charges += bld.withS7(createS7(1, 1)).forBags(0).withAmt(50, "USD").build();
    charges += bld.withS7(createS7(1, 1, "0GO", 'X')).forBags(0).withAmt(150, "USD").build();

    _strategy->findLowestCharges(bt, BaggageTravelInfo(0, 0), charges, 0);

    CPPUNIT_ASSERT(bt->_charges[0]);
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("USD"), bt->_charges[0]->feeCurrency());
    CPPUNIT_ASSERT_EQUAL(150.0, bt->_charges[0]->feeAmount());
  }

  void testMatchOccurrence_Both_Blank_Both_Pass_First_Bag()
  {
    CPPUNIT_ASSERT(_strategy->matchOccurrence(
        S7Builder(&_memHandle).withBaggageOccurrence(-1, -1).build(), 1));
  }

  void testMatchOccurrence_Both_Blank_Both_Pass_Second_Bag()
  {
    CPPUNIT_ASSERT(_strategy->matchOccurrence(
        S7Builder(&_memHandle).withBaggageOccurrence(-1, -1).build(), 2));
  }

  void testGetAllowanceS7_Null()
  {
    CPPUNIT_ASSERT(
        !_strategy->getAllowanceS7(BaggageTravelBuilder(&_memHandle).withNoAllowance().build()));
  }

  void testGetAllowanceS7_NotNull()
  {
    OptionalServicesInfo s7;

    CPPUNIT_ASSERT(&s7 == _strategy->getAllowanceS7(
                              BaggageTravelBuilder(&_memHandle).withAllowanceS7(&s7).build()));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(ChargesDataStrategyTest);
} // tse
