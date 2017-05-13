#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "Fares/FareByRuleFareCreator.h"

#include "DataModel/PricingTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "Fares/FareByRuleProcessingInfo.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Record2Types.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/Loc.h"
#include "DBAccess/BaseFareRule.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
namespace
{
class FareByRuleFareCreatorMock : public FareByRuleFareCreator
{
public:
  FareByRuleFareCreatorMock(const PricingTrx& trx, const Itin& itin, const FareMarket& fareMarket)
    : FareByRuleFareCreator(trx, itin, fareMarket)
  {
    _baseFareRules.push_back(&_baseFareRule);
    _baseFareRule.baseglobalDir() = GlobalDirection::ZZ;
    _baseFareRule.baseFareClass() = "FARECLAS";
    _baseFareRule.basepsgType() = ADULT;
  }

  bool matchNationCurrency(const NationCode& nation, const CurrencyCode& currency) const
  {
    return (currency == "PLN" && nation == "PL") || (currency == "EUR" && nation == "DE") ||
           (currency == "USD" && nation == "US");
  }

  const std::vector<const BaseFareRule*>&
  getBaseFareRule(const VendorCode& vendor, int itemNo) const
  {
    return _baseFareRules;
  }

  BaseFareRule _baseFareRule;
  std::vector<const BaseFareRule*> _baseFareRules;
};
}

class FareByRuleFareCreatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareByRuleFareCreatorTest);
  CPPUNIT_TEST(testCreateFareInfo);
  CPPUNIT_TEST(testCreateFareInfo_Specified);
  CPPUNIT_TEST(testCreateFareInfo_RT);
  CPPUNIT_TEST(testCreateFareInfo_OW);

  CPPUNIT_TEST(testCreateFareInfo_ConstructedSita);
  CPPUNIT_TEST(testCreateFareInfo_ConstructedNotSita);

  CPPUNIT_TEST(testCloneFareClassAppInfo_OWRT);
  CPPUNIT_TEST(testCloneFareClassAppInfo_Routing);
  CPPUNIT_TEST(testCloneFareClassAppInfo_FareType);
  CPPUNIT_TEST(testCloneFareClassAppInfo_SeasonType);
  CPPUNIT_TEST(testCloneFareClassAppInfo_DowType);
  CPPUNIT_TEST(testCloneFareClassAppInfo_PricingCatType);
  CPPUNIT_TEST(testCloneFareClassAppInfo_DisplayCatType);

  CPPUNIT_TEST(testCloneFareClassAppSegInfo_TktCode);
  CPPUNIT_TEST(testCloneFareClassAppSegInfo_TktCodeModifier);
  CPPUNIT_TEST(testCloneFareClassAppSegInfo_TktDes);
  CPPUNIT_TEST(testCloneFareClassAppSegInfo_TktDesModifier);
  CPPUNIT_TEST(testCloneFareClassAppSegInfo_BookingCode);

  CPPUNIT_TEST(testGetDirectionalityReturnFROMWhenMatchCountryOfOrigin);
  CPPUNIT_TEST(testGetDirectionalityReturnTOWhenMatchCountryOfDestination);
  CPPUNIT_TEST(testGetDirectionalityReturnTOWhenMatchCountryOfOriginInReverseDirection);
  CPPUNIT_TEST(testGetDirectionalityReturnFROMWhenMatchCountryOfDestinationInReverseDirection);
  CPPUNIT_TEST(testGetDirectionalityReturnBOTHWhenMatchCountryOfOriginAndDestination);
  CPPUNIT_TEST(testGetDirectionalityReturnBOTHWhenFareMarketIsTransborder);

  CPPUNIT_TEST(testUpdateFbrFareClass_DashEnd);
  CPPUNIT_TEST(testUpdateFbrFareClass_DashEndLong);
  CPPUNIT_TEST(testUpdateFbrFareClass_DashBegin);
  CPPUNIT_TEST(testUpdateFbrFareClass_DashBeginLong);
  CPPUNIT_TEST(testUpdateFbrFareClass_AsteriskBegin);

  CPPUNIT_TEST(testCreateFBRPaxTypeFareRuleData);
  CPPUNIT_TEST(testCreateFBRPaxTypeFareRuleData_WithBaseFare);
  CPPUNIT_TEST(testCreateFBRPaxTypeFareRuleData_WithBaseFareAndBaseFareInfoBkcAvail);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  FareByRuleFareCreator* _creator;
  Itin* _itin;
  PricingTrx* _trx;
  FareMarket* _fareMarket;
  Loc* _locPL;
  Loc* _locUS;
  Loc* _locXX;
  FareByRuleItemInfo* _fbrItemInfo;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    PricingOptions* options = _memHandle.create<PricingOptions>();
    _trx->setOptions(options);
    _itin = _memHandle.create<Itin>();
    _fareMarket = _memHandle.create<FareMarket>();
    _creator = _memHandle.insert(new FareByRuleFareCreatorMock(*_trx, *_itin, *_fareMarket));

    _locPL = _memHandle.create<Loc>();
    _locUS = _memHandle.create<Loc>();
    _locXX = _memHandle.create<Loc>();
    _locPL->nation() = "PL";
    _locUS->nation() = "US";
    _locXX->nation() = "XX";
    _fareMarket->origin() = _locPL;
    _fareMarket->destination() = _locUS;

    FareByRuleProcessingInfo* fbrProcInfo = _memHandle.create<FareByRuleProcessingInfo>();
    fbrProcInfo->fbrApp() = _memHandle.create<FareByRuleApp>();
    fbrProcInfo->fbrCtrlInfo() = _memHandle.create<FareByRuleCtrlInfo>();
    _fbrItemInfo = _memHandle.create<FareByRuleItemInfo>();
    fbrProcInfo->fbrItemInfo() = _fbrItemInfo;

    _creator->initCreationData(fbrProcInfo,
                               _memHandle.create<std::vector<CategoryRuleItemInfo> >(),
                               _memHandle.create<CategoryRuleItemInfo>(),
                               _memHandle.create<CategoryRuleItemInfoSet>(),
                               false);
  }

  void tearDown() { _memHandle.clear(); }

  PaxTypeFare* createConstructedPtf(bool isSita)
  {
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    Fare* fare = _memHandle.create<Fare>();
    ptf->setFare(fare);
    fare->status().set(Fare::FS_ConstructedFare);

    FareInfo* info = _memHandle.create<FareInfo>();
    if (isSita)
      info->vendor() = SITA_VENDOR_CODE;
    fare->setFareInfo(info);

    FareClassAppInfo* fcAppInfo = _memHandle.create<FareClassAppInfo>();
    fcAppInfo->_segs.push_back(_memHandle.create<FareClassAppSegInfo>());

    ptf->fareClassAppInfo() = fcAppInfo;

    return ptf;
  }

  // TESTS

  void testCreateFareInfo()
  {
    CPPUNIT_ASSERT_EQUAL(
        FROM,
        _creator->createFareInfo(1, "PLN", "PLN", "PLN", DateTime(), DateTime())->directionality());
  }

  void testCreateFareInfo_Specified()
  {
    _fbrItemInfo->fareInd() = FareByRuleItemInfo::SPECIFIED;
    CPPUNIT_ASSERT_EQUAL(
        TO,
        _creator->createFareInfo(1, "PLN", "PLN", "PLN", DateTime(), DateTime())->directionality());
  }

  void testCreateFareInfo_RT()
  {
    _fbrItemInfo->resultowrt() = ROUND_TRIP_MAYNOT_BE_HALVED;
    CPPUNIT_ASSERT_EQUAL(
        BOTH,
        _creator->createFareInfo(1, "PLN", "PLN", "PLN", DateTime(), DateTime())->directionality());
  }

  void testCreateFareInfo_OW()
  {
    _fbrItemInfo->resultowrt() = ONE_WAY_MAY_BE_DOUBLED;
    CPPUNIT_ASSERT_EQUAL(
        BOTH,
        _creator->createFareInfo(1, "PLN", "PLN", "PLN", DateTime(), DateTime())->directionality());
  }

  void testCreateFareInfo_ConstructedSita()
  {
    CPPUNIT_ASSERT(dynamic_cast<SITAFareInfo*>(
        _creator->createFareInfo(DateTime(), DateTime(), createConstructedPtf(true))));
  }

  void testCreateFareInfo_ConstructedNotSita()
  {
    CPPUNIT_ASSERT(!dynamic_cast<SITAFareInfo*>(_creator->createFareInfo(
        DateTime(), DateTime(), createConstructedPtf(false))));
  }

  void testCloneFareClassAppInfo_OWRT()
  {
    _fbrItemInfo->resultowrt() = 'U';
    FareClassAppInfo appInfo;

    CPPUNIT_ASSERT_EQUAL(
        'U', _creator->cloneFareClassAppInfo(DateTime(), DateTime(), appInfo, "CLASS")->_owrt);
  }

  void testCloneFareClassAppInfo_Routing()
  {
    _fbrItemInfo->resultRouting() = "RTG";
    FareClassAppInfo appInfo;

    CPPUNIT_ASSERT_EQUAL(
        RoutingNumber("RTG"),
        _creator->cloneFareClassAppInfo(DateTime(), DateTime(), appInfo, "CLASS")->_routingNumber);
  }

  void testCloneFareClassAppInfo_FareType()
  {
    _fbrItemInfo->resultFareType1() = "FT";
    FareClassAppInfo appInfo;

    CPPUNIT_ASSERT_EQUAL(
        FareType("FT"),
        _creator->cloneFareClassAppInfo(DateTime(), DateTime(), appInfo, "CLASS")->_fareType);
  }

  void testCloneFareClassAppInfo_SeasonType()
  {
    _fbrItemInfo->resultseasonType() = 'W';
    FareClassAppInfo appInfo;

    CPPUNIT_ASSERT_EQUAL(
        'W',
        _creator->cloneFareClassAppInfo(DateTime(), DateTime(), appInfo, "CLASS")->_seasonType);
  }

  void testCloneFareClassAppInfo_DowType()
  {
    _fbrItemInfo->resultdowType() = 'V';
    FareClassAppInfo appInfo;

    CPPUNIT_ASSERT_EQUAL(
        'V', _creator->cloneFareClassAppInfo(DateTime(), DateTime(), appInfo, "CLASS")->_dowType);
  }

  void testCloneFareClassAppInfo_PricingCatType()
  {
    _fbrItemInfo->resultpricingcatType() = 'Y';
    FareClassAppInfo appInfo;

    CPPUNIT_ASSERT_EQUAL(
        'Y',
        _creator->cloneFareClassAppInfo(DateTime(), DateTime(), appInfo, "CLASS")->_pricingCatType);
  }

  void testCloneFareClassAppInfo_DisplayCatType()
  {
    _fbrItemInfo->resultDisplaycatType() = 'Z';
    FareClassAppInfo appInfo;

    CPPUNIT_ASSERT_EQUAL(
        'Z',
        _creator->cloneFareClassAppInfo(DateTime(), DateTime(), appInfo, "CLASS")->_displayCatType);
  }

  void testCloneFareClassAppSegInfo_TktCode()
  {
    _fbrItemInfo->tktCode() = "TKT";
    FareClassAppSegInfo appSegInfo;

    CPPUNIT_ASSERT_EQUAL(TktCode("TKT"), _creator->cloneFareClassAppSegInfo(appSegInfo)->_tktCode);
  }

  void testCloneFareClassAppSegInfo_TktCodeModifier()
  {
    _fbrItemInfo->tktCodeModifier() = 'A';
    FareClassAppSegInfo appSegInfo;

    CPPUNIT_ASSERT_EQUAL(TktCodeModifier("A"),
                         _creator->cloneFareClassAppSegInfo(appSegInfo)->_tktCodeModifier);
  }

  void testCloneFareClassAppSegInfo_TktDes()
  {
    _fbrItemInfo->tktDesignator() = "DES";
    FareClassAppSegInfo appSegInfo;

    CPPUNIT_ASSERT_EQUAL(TktDesignator("DES"),
                         _creator->cloneFareClassAppSegInfo(appSegInfo)->_tktDesignator);
  }

  void testCloneFareClassAppSegInfo_TktDesModifier()
  {
    _fbrItemInfo->tktDesignatorModifier() = 'B';
    FareClassAppSegInfo appSegInfo;

    CPPUNIT_ASSERT_EQUAL(TktDesignatorModifier("B"),
                         _creator->cloneFareClassAppSegInfo(appSegInfo)->_tktDesignatorModifier);
  }

  void testCloneFareClassAppSegInfo_BookingCode()
  {
    _fbrItemInfo->bookingCode1() = "BC";
    FareClassAppSegInfo appSegInfo;

    CPPUNIT_ASSERT_EQUAL(BookingCode("BC"),
                         _creator->cloneFareClassAppSegInfo(appSegInfo)->_bookingCode[0]);
  }

  void testGetDirectionalityReturnFROMWhenMatchCountryOfOrigin()
  {
    _fareMarket->origin() = _locPL;
    _fareMarket->destination() = _locXX;
    CPPUNIT_ASSERT(_creator->getDirectionality("PLN", false) == FROM);
  }

  void testGetDirectionalityReturnTOWhenMatchCountryOfDestination()
  {
    _fareMarket->origin() = _locXX;
    _fareMarket->destination() = _locPL;
    CPPUNIT_ASSERT(_creator->getDirectionality("PLN", false) == TO);
  }

  void testGetDirectionalityReturnTOWhenMatchCountryOfOriginInReverseDirection()
  {
    _fareMarket->origin() = _locPL;
    _fareMarket->destination() = _locXX;
    CPPUNIT_ASSERT(_creator->getDirectionality("PLN", true) == TO);
  }

  void testGetDirectionalityReturnFROMWhenMatchCountryOfDestinationInReverseDirection()
  {
    _fareMarket->origin() = _locXX;
    _fareMarket->destination() = _locPL;
    CPPUNIT_ASSERT(_creator->getDirectionality("PLN", true) == FROM);
  }

  void testGetDirectionalityReturnBOTHWhenMatchCountryOfOriginAndDestination()
  {
    _fareMarket->origin() = _locPL;
    _fareMarket->destination() = _locPL;
    CPPUNIT_ASSERT(_creator->getDirectionality("PLN", true) == BOTH);
  }

  void testGetDirectionalityReturnBOTHWhenFareMarketIsTransborder()
  {
    _fareMarket->origin() = _locXX;
    _fareMarket->destination() = _locPL;
    _fareMarket->geoTravelType() = GeoTravelType::Transborder;
    CPPUNIT_ASSERT(_creator->getDirectionality("XXX", true) == BOTH);
  }

  void testUpdateFbrFareClass_DashEnd()
  {
    CPPUNIT_ASSERT_EQUAL(FareClassCode("B26"),
                         FareByRuleFareCreator::updateFbrFareClass("Y26", "B-"));
  }

  void testUpdateFbrFareClass_DashEndLong()
  {
    CPPUNIT_ASSERT_EQUAL(FareClassCode("ABCDEFGHI26"),
                         FareByRuleFareCreator::updateFbrFareClass("Y26", "ABCDEFGHI-"));
  }

  void testUpdateFbrFareClass_DashBegin()
  {
    CPPUNIT_ASSERT_EQUAL(FareClassCode("YF"), FareByRuleFareCreator::updateFbrFareClass("Y", "-F"));
  }

  void testUpdateFbrFareClass_DashBeginLong()
  {
    CPPUNIT_ASSERT_EQUAL(FareClassCode("Y26ABCDE"),
                         FareByRuleFareCreator::updateFbrFareClass("Y26ABCDEF", "-XYZABC"));
  }

  void testUpdateFbrFareClass_AsteriskBegin()
  {
    CPPUNIT_ASSERT_EQUAL(FareClassCode("WABC"),
                         FareByRuleFareCreator::updateFbrFareClass("WAP3", "*ABC"));
  }

  void testCreateFBRPaxTypeFareRuleData()
  {
    std::map<PaxTypeFare*, std::set<BookingCode> > baseFareInfoBkcAvailMap;
    PaxTypeFare* ptf = NULL;

    FBRPaxTypeFareRuleData* ret = _creator->createFBRPaxTypeFareRuleData(
        *createConstructedPtf(false), baseFareInfoBkcAvailMap, false, ptf);
    CPPUNIT_ASSERT_EQUAL(ptf, ret->baseFare());
    CPPUNIT_ASSERT(!ret->isBaseFareAvailBkcMatched());
  }

  void testCreateFBRPaxTypeFareRuleData_WithBaseFare()
  {
    std::map<PaxTypeFare*, std::set<BookingCode> > baseFareInfoBkcAvailMap;
    PaxTypeFare* ptf = createConstructedPtf(false);

    FBRPaxTypeFareRuleData* ret = _creator->createFBRPaxTypeFareRuleData(
        *createConstructedPtf(false), baseFareInfoBkcAvailMap, false, ptf);
    CPPUNIT_ASSERT_EQUAL(ptf, ret->baseFare());
    CPPUNIT_ASSERT(!ret->isBaseFareAvailBkcMatched());
  }

  void testCreateFBRPaxTypeFareRuleData_WithBaseFareAndBaseFareInfoBkcAvail()
  {
    std::map<PaxTypeFare*, std::set<BookingCode> > baseFareInfoBkcAvailMap;
    std::set<BookingCode> bkgCodeSet;
    bkgCodeSet.insert("X");
    PaxTypeFare* ptf = createConstructedPtf(false);
    baseFareInfoBkcAvailMap.insert(std::make_pair(ptf, bkgCodeSet));

    FBRPaxTypeFareRuleData* ret = _creator->createFBRPaxTypeFareRuleData(
        *createConstructedPtf(false), baseFareInfoBkcAvailMap, false, ptf);
    CPPUNIT_ASSERT_EQUAL(ptf, ret->baseFare());
    CPPUNIT_ASSERT(ret->isBaseFareAvailBkcMatched());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareByRuleFareCreatorTest);
}
