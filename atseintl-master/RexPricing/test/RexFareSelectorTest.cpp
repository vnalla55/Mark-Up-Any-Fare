#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/Config/ConfigMan.h"
#include "Common/ErrorResponseException.h"
#include "Common/TravelSegAnalysis.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/Currency.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "RexPricing/RexFareSelector.h"
#include "Rules/RuleConst.h"
#include "Fares/BooleanFlagResetter.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/PrintCollection.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestFactoryManager.h"
#include "test/include/TestConfigInitializer.h"

#include <random>

namespace tse
{

using boost::assign::operator+=;

class MatchingParams
{
public:
  PaxType& _pt;
  FareClassCode _fcc;
  TktCode _tc;
  MoneyAmount _ma;
  CurrencyCode _cc;
  bool _isIndustry;

public:
  MatchingParams(
      PaxType& pt, FareClassCode fcc, TktCode tc, MoneyAmount ma, CurrencyCode cc, bool isIndustry)
    : _pt(pt), _fcc(fcc), _tc(tc), _ma(ma), _cc(cc), _isIndustry(isIndustry)
  {
  }
};

class FareMarketOverrideRec
{
public:
  PaxTypeBucket _ptc;
  PaxTypeFare _ptf;
  Fare _fare;
  FareInfo _fareInfo;
  FareClassAppSegInfo _fcasi;
};

class FareMarketOverride : public FareMarket
{
  PricingRequest _request;
  ExcItin _excItin;
  MinFarePlusUpItem _minFarePlusUpItem;
  FareCompInfo _fareCompInfo;

  std::vector<FareMarketOverrideRec*> _vfmer;
  std::vector<FareMarket*> _FareMarketList;

public:
  void Init(PaxType& pt,
            RexPricingTrx& trx,
            CurrencyCode& calculationCurrency,
            CurrencyCode& originationCurrency)
  {
    // RexPricingTrx data
    trx.exchangePaxType() = &pt;
    trx.paxType().push_back(&pt);
    trx.exchangeItin().push_back(&_excItin);
    trx.setRequest(&_request);
    trx.getRequest()->ticketingDT() = DateTime::localTime();
    trx.transactionStartTime() = DateTime::localTime();

    // ExcItin data
    _excItin.calculationCurrency() = calculationCurrency;
    _excItin.calcCurrencyOverride() = calculationCurrency;
    _excItin.originationCurrency() = originationCurrency;
    _excItin.calculationCurrencyNoDec() = calculationCurrency == "JPY" ? 0 : 2;
  }

  FareMarketOverride(PaxType& pt,
                     RexPricingTrx& trx,
                     CurrencyCode& calculationCurrency,
                     CurrencyCode& originationCurrency)
  {
    Init(pt, trx, calculationCurrency, originationCurrency);

    this->direction() = FMDirection::INBOUND;
  }

  FareMarketOverride(PaxType& pt,
                     RexPricingTrx& trx,
                     CurrencyCode& calculationCurrency,
                     CurrencyCode& originationCurrency,
                     DateTime departureDate)
  {
    Init(pt, trx, calculationCurrency, originationCurrency);

    if (departureDate != DateTime::emptyDate())
    {
      createTravelSeg(_excItin.travelSeg(),
                      "MIA",
                      "US",
                      "AA",
                      "NYC",
                      "US",
                      false,
                      departureDate.subtractDays(10));
      createTravelSeg(_excItin.travelSeg(), "NYC", "US", "AA", "WAS", "US", true, departureDate);
    }
  }

  ~FareMarketOverride()
  {
    std::vector<FareMarketOverrideRec*>::const_iterator pos = _vfmer.begin();
    for (; pos != _vfmer.end(); pos++)
      delete *pos;

    std::vector<FareMarket*>::const_iterator i = _FareMarketList.begin();
    for (; i != _FareMarketList.end(); i++)
      delete *i;
  }

  FareMarketOverrideRec* addFareWithVendor(PaxType& pt,
                                           FareClassCode fcc,
                                           TktCode tc,
                                           MoneyAmount ma,
                                           bool isIndustry,
                                           VendorCode vendor)
  {
    FareMarketOverrideRec* p = addFare(pt, fcc, tc, ma, isIndustry);
    p->_fareInfo.vendor() = vendor;
    p->_ptf.fareMarket() = new FareMarket();
    _FareMarketList.push_back(p->_ptf.fareMarket());
    return p;
  }

  FareMarketOverrideRec* addFareWithVendor(PaxType& pt,
                                           FareClassCode fcc,
                                           TktCode tc,
                                           MoneyAmount ma,
                                           CurrencyCode cc,
                                           bool isIndustry,
                                           VendorCode vendor)
  {
    FareMarketOverrideRec* p = addFare(pt, fcc, tc, ma, cc, isIndustry);
    p->_fareInfo.vendor() = vendor;
    p->_ptf.fareMarket() = new FareMarket();
    _FareMarketList.push_back(p->_ptf.fareMarket());
    return p;
  }

  FareMarketOverrideRec* addFare(PaxType& pt,
                                 FareClassCode fcc,
                                 TktCode tc,
                                 MoneyAmount ma,
                                 bool isIndustry,
                                 Directionality dir = TO)
  {
    FareMarketOverrideRec* pfmer = new FareMarketOverrideRec;
    pfmer->_ptf.fareMarket() = this;
    pfmer->_ptc.requestedPaxType() = &pt;
    pfmer->_ptc.actualPaxType().push_back(&pt);
    pfmer->_fareInfo.fareAmount() = ma;
    pfmer->_ptf.setCategoryValid(RuleConst::VOLUNTARY_EXCHANGE_RULE);
    pfmer->_fareInfo.fareClass() = fcc;
    pfmer->_fare.setFareInfo(&pfmer->_fareInfo);
    pfmer->_ptf.setFare(&pfmer->_fare);
    if (isIndustry)
      pfmer->_ptf.fare()->status().set(Fare::FS_IndustryFare);
    pfmer->_fcasi._tktCode = tc;
    pfmer->_ptf.fareClassAppSegInfo() = &pfmer->_fcasi;
    _vfmer.push_back(pfmer);

    this->paxTypeCortege().push_back(pfmer->_ptc);
    this->paxTypeCortege(&pt)->paxTypeFare().push_back(&pfmer->_ptf);

    return pfmer;
  }

  FareMarketOverrideRec* addFare(
      PaxType& pt, FareClassCode fcc, TktCode tc, MoneyAmount ma, CurrencyCode cc, bool isIndustry)
  {
    FareMarketOverrideRec* pfmer = addFare(pt, fcc, tc, ma, isIndustry);
    pfmer->_fareInfo.currency() = cc;

    return pfmer;
  }

  FareMarketOverrideRec* addFare(PaxType& pt,
                                 FareClassCode fcc,
                                 TktCode tc,
                                 MoneyAmount ma,
                                 CurrencyCode cc,
                                 bool isIndustry,
                                 Directionality dir)
  {
    FareMarketOverrideRec* pfmer = addFare(pt, fcc, tc, ma, cc, isIndustry);
    pfmer->_fareInfo.directionality() = dir;

    return pfmer;
  }

  void setFareCompInfo(FareClassCode fareBasisCode,
                       MoneyAmount moneyAmount,
                       uint16_t mileageSurchargePctg,
                       bool hip)
  {
    _fareCompInfo.fareMarket() = this;
    _fareCompInfo.fareBasisCode() = fareBasisCode;
    _fareCompInfo.tktFareCalcFareAmt() = moneyAmount;
    _fareCompInfo.mileageSurchargePctg() = mileageSurchargePctg;
    if (hip)
      _fareCompInfo.hip() = &_minFarePlusUpItem;

    if (find(_excItin.fareComponent().begin(), _excItin.fareComponent().end(), &_fareCompInfo) ==
        _excItin.fareComponent().end())
      _excItin.fareComponent().push_back(&_fareCompInfo);
  }

  FareCompInfo& fareCompInfo() { return _fareCompInfo; }

  void createTravelSeg(std::vector<TravelSeg*>& travelSeg,
                       const char* boardCity,
                       const char* boardNation,
                       const char* carrier,
                       const char* offCity,
                       const char* offNation,
                       bool unflown,
                       DateTime departureDate);
};

void
FareMarketOverride::createTravelSeg(std::vector<TravelSeg*>& travelSeg,
                                    const char* boardCity,
                                    const char* boardNation,
                                    const char* carrier,
                                    const char* offCity,
                                    const char* offNation,
                                    bool unflown,
                                    DateTime departureDate)
{
  AirSeg* as = new AirSeg;
  Loc* lc1 = new Loc();
  lc1->loc() = boardCity;
  lc1->nation() = boardNation;
  as->origin() = lc1;
  as->origAirport() = boardCity;
  Loc* lc2 = new Loc();
  lc2->loc() = offCity;
  lc2->nation() = offNation;
  as->destination() = lc2;
  as->destAirport() = offCity;
  as->carrier() = carrier;
  as->unflown() = unflown;
  as->departureDT() = departureDate;
  travelSeg.push_back(as);
}

class FareCompInfoOverride : public FareCompInfo
{
  MinFarePlusUpItem _minFarePlusUpItem;

public:
  FareCompInfoOverride(FareMarket& fareMarket,
                       FareClassCode fareBasisCode,
                       MoneyAmount moneyAmount,
                       uint16_t mileageSurchargePctg,
                       bool hip)
  {
    this->fareMarket() = &fareMarket;
    this->fareBasisCode() = fareBasisCode;
    this->tktFareCalcFareAmt() = moneyAmount;
    this->fareCalcFareAmt() = moneyAmount;
    this->mileageSurchargePctg() = mileageSurchargePctg;
    if (hip)
      this->hip() = &_minFarePlusUpItem;
  }

  std::vector<PaxTypeFare*> getMatchedFares()
  {
    std::vector<PaxTypeFare*> fares;
    for (MatchedFare& fare : matchedFares())
      fares.push_back(fare.get());
    return fares;
  }
};

class RexFareSelectorOverride : public RexFareSelector
{
  typedef std::multimap<int, MatchingParams> DateMPPar;
  typedef std::map<FareMarketOverride*, DateMPPar*> FMDMPPar;

  FMDMPPar _allFares;
  TestMemHandle _memHandle;

public:
  RexFareSelectorOverride(RexPricingTrx& trx, FareMarket& fareMarket) : RexFareSelector(trx)
  {
    trx.setOptions(_memHandle.create<PricingOptions>());
    trx.getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    this->setInitialFaresStatus(
        fareMarket.allPaxTypeFare(), RuleConst::VOLUNTARY_EXCHANGE_RULE, false);
  }

  void addFareForDate(FareMarketOverride& fareMarket,
                      int dateSequence,
                      PaxType& pt,
                      FareClassCode fcc,
                      TktCode tc,
                      MoneyAmount ma,
                      CurrencyCode cc,
                      bool isIndustry)
  {
    if (dateSequence == ORIGINAL_TICKET_DATE)
    {
      fareMarket.addFare(pt, fcc, tc, ma, cc, isIndustry);
      return;
    }

    FMDMPPar::const_iterator pos;
    pos = _allFares.find(&fareMarket);

    DateMPPar* par;

    if (pos == _allFares.end())
    {
      par = _memHandle.create<DateMPPar>();
      _allFares.insert(FMDMPPar::value_type(&fareMarket, par));
    }
    else
    {
      par = _allFares[&fareMarket];
    }

    MatchingParams mp(pt, fcc, tc, ma, cc, isIndustry);

    par->insert(DateMPPar::value_type(dateSequence, mp));
  }

protected:
  void setInitialFaresStatus(std::vector<PaxTypeFare*>& fares,
                             const unsigned int category,
                             const bool catIsValid) const
  {
    std::vector<PaxTypeFare*>::const_iterator faresIter = fares.begin();

    for (; faresIter != fares.end(); ++faresIter)
      (*faresIter)->setCategoryValid(category, catIsValid);
  }

  bool runFareCollector(RexPricingTrx& trx)
  {
    std::vector<FareCompInfo*>::iterator fcIter = trx.exchangeItin()[0]->fareComponent().begin(),
                                         fcIterEnd = trx.exchangeItin()[0]->fareComponent().end();

    for (; fcIter != fcIterEnd; ++fcIter)
    {
      FareCompInfo& fc = *(*fcIter);
      FareMarketOverride* fmo = (FareMarketOverride*)fc.fareMarket();

      FMDMPPar::const_iterator pos = _allFares.find(fmo);

      if (pos == _allFares.end())
        return false;

      DateMPPar::const_iterator i = pos->second->begin();
      DateMPPar::const_iterator ie = pos->second->end();

      for (; i != ie; i++)
      {
        if (i->first == trx.getCurrTktDateSeqStatus())
        {
          fmo->addFare(i->second._pt,
                       i->second._fcc,
                       i->second._tc,
                       i->second._ma,
                       i->second._cc,
                       i->second._isIndustry);
        }
      }
    }

    return true;
  }
};

void operator+=(std::vector<PaxTypeFare*>& expect, FareMarketOverrideRec* rec)
{
  expect.push_back(&rec->_ptf);
}

class RexFareSelectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RexFareSelectorTest);

  CPPUNIT_TEST(testNoMatchCalcCurrencyAndFaresCurrency);
  CPPUNIT_TEST(testNoMatchFareClassForJPYCurrency);
  CPPUNIT_TEST(testNoMatchAmountForJPYCurrency);
  CPPUNIT_TEST(testMatchAmountForJPYCurrency);
  CPPUNIT_TEST(testMatchAmountPlusOneForJPYCurrency);
  CPPUNIT_TEST(testNoMatchFareClassForUSDCurrency);
  CPPUNIT_TEST(testNoMatchAmountForUSDCurrency);
  CPPUNIT_TEST(testMatchAmountForUSDCurrency);
  CPPUNIT_TEST(testMatchAmountPlusOneForUSDCurrency);
  CPPUNIT_TEST(testMatchLowerLimitVarianceAmountForUSDCurrency);
  CPPUNIT_TEST(testMatchUpperLimitVarianceAmountForUSDCurrency);
  CPPUNIT_TEST(testMatchVarianceAmountForUSDCurrency);
  CPPUNIT_TEST(testMatchYYFare);
  CPPUNIT_TEST(testNoMatchHIP);
  CPPUNIT_TEST(testMatchHIP);
  CPPUNIT_TEST(testMatchHIPForYY);
  CPPUNIT_TEST(testMatchProcess);
  CPPUNIT_TEST(testMatchProcessYY);
  CPPUNIT_TEST(testMatchProcessHIP);
  CPPUNIT_TEST(testMatchProcessHIPandYY);
  CPPUNIT_TEST(testMatchProcessVariance);
  CPPUNIT_TEST(testMatchProcessVarianceYY);
  CPPUNIT_TEST(testNoMatchProcessHIPAndVariance);
  CPPUNIT_TEST(testMatchWhenFareCalcTimesSurchargeEqualsFareAmt);
  CPPUNIT_TEST(testMatchForHIPWhenFareCalcTimesSurchargeEqualsFareAmt);
  CPPUNIT_TEST(testMatchWhenFareCalcMinusVarianceTimesSurchargeEqualsFareAmt);

  CPPUNIT_TEST(testMatchHIP_SameFaresAtpAndSita);
  CPPUNIT_TEST(testMatchHIP_FaresAtpAndSitaDiffAmount);
  CPPUNIT_TEST(testMatchHIP_FaresAtpAndSita);
  CPPUNIT_TEST(testMatchVariance_SameFaresAtpAndSita);
  CPPUNIT_TEST(testMatchVariance_FaresAtpAndSitaDiffAmount);
  CPPUNIT_TEST(testMatchVariance_FaresAtpAndSita);
  CPPUNIT_TEST(testMatchAmount_SameFaresAtpAndSita);
  CPPUNIT_TEST(testMatchAmount_FaresAtpAndSita_SFBetterMatch);

  CPPUNIT_TEST(testMatchOutboundFareForSideTripOutboundFareMarket);
  CPPUNIT_TEST(testMatchOutboundFareForSideTripInboundFareMarket);
  CPPUNIT_TEST(testMatchInboundFareForSideTripOutboundFareMarket);
  CPPUNIT_TEST(testMatchInboundFareForSideTripInboundFareMarket);

  CPPUNIT_TEST(testStoreVCTR_NotStoredForOrigTcktDate);
  CPPUNIT_TEST(testStoreVCTR_NotStoredFcDateEqOrigDT);
  CPPUNIT_TEST(testStoreVCTR_StoredForFcDateDifferentThanOrigDT);
  CPPUNIT_TEST(testStoreRestoreVCTR_StoredAndRestoredForDateDifferentThanOrigDT);

  CPPUNIT_TEST(testInvalidateClone);
  CPPUNIT_TEST(testSelectSecondaryFareMarket_SubArea21);
  CPPUNIT_TEST(testSelectSecondaryFareMarket_USCA);
  CPPUNIT_TEST(testSelectSecondaryFareMarket_Except_USCA);
  CPPUNIT_TEST(testSelectSecondaryFareMarket_No_VCTR);
  CPPUNIT_TEST(testSelectSecondaryFareMarket_VCTR);

  CPPUNIT_TEST(testRAII);

  CPPUNIT_TEST(markupPercentTest);
  CPPUNIT_TEST(markupAmountTest);
  CPPUNIT_TEST(discountPercentTest);
  CPPUNIT_TEST(discountAmountTest);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<RexPricingTrx>();
    _trx->fareCalcConfig() = _memHandle.create<FareCalcConfig>();
    _trx->setRequest(_memHandle.create<RexBaseRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    Loc* agentLoc = _memHandle.create<Loc>();
    agentLoc->nation() = "GER";
    agentLoc->loc() = "BER";
    agentLoc->city() = "BER";
    _trx->getRequest()->ticketingAgent()->agentLocation() = agentLoc;

    _ptADT.paxType() = "ADT";
    _ptINF.paxType() = "INF";

    _localTime = DateTime::localTime();
    _localTimeOneDayBefore = _localTime.subtractDays(1);
    _localTimeTwoDaysBefore = _localTime.subtractDays(2);
    _localTimeOneDayAfter = _localTime.addDays(1);
    _localTimeTwoDaysAfter = _localTime.addDays(2);
    _currencyJPY = "JPY";
    _currencyUSD = "USD";
    _YYFare = true;
    _notYYFare = false;

    _HIP = true;
    _noHIP = false;

    _directionalityTO = TO;
    _directionalityFROM = FROM;
    _zeroMileageSurcharge = 0;
  }

  void tearDown()
  {
    _memHandle.clear();
    TestFactoryManager::instance()->destroyAll();
  }

TravelSeg*
createAirSeg(const LocCode& origAirport, const LocCode& destAirport, const CarrierCode& cxr)
{
  AirSeg* seg = _memHandle.create<AirSeg>();
  DataHandle dataHandle;

  seg->origin() = dataHandle.getLoc( origAirport, DateTime::localTime() );
  seg->destination() = dataHandle.getLoc( destAirport, DateTime::localTime() );
  seg->carrier() = cxr;
  seg->departureDT() = DateTime::localTime();

  return seg;
}

  void testNoMatchCalcCurrencyAndFaresCurrency()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyJPY, _currencyJPY);
    fareMarket.addFare(_ptADT, "CX", "", 100, _currencyUSD, _notYYFare);
    fareMarket.addFare(_ptINF, "CX", "", 150, _currencyUSD, _notYYFare);
    fareMarket.addFare(_ptADT, "CX", "-/CAAS", 200, _currencyUSD, _notYYFare);
    fareMarket.addFare(_ptADT, "CX", "-/CAAS", 1200, _currencyJPY, _notYYFare);

    FareCompInfoOverride fareCompInfo(fareMarket, "CX/CAAS", 200, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT(fareCompInfo.matchedFares().empty());
  }

  // We should failed because none fare have right basis code for ADT fares
  void testNoMatchFareClassForJPYCurrency()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyJPY, _currencyJPY);
    fareMarket.addFare(_ptADT, "CX", "", 1253.984, _currencyJPY, _notYYFare);
    fareMarket.addFare(_ptINF, "CX", "", 1253.986, _currencyJPY, _notYYFare);
    fareMarket.addFare(_ptADT, "CX", "-/CAAS", 200, _currencyUSD, _notYYFare);

    FareCompInfoOverride fareCompInfo(fareMarket, "CX/CAAS", 1200, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT(fareCompInfo.matchedFares().empty());
  }

  // We should failed because none fare have right fare amount for ADT fares
  void testNoMatchAmountForJPYCurrency()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyJPY, _currencyJPY);
    fareMarket.addFare(_ptADT, "CX", "", 1198.984, _currencyJPY, _notYYFare);
    fareMarket.addFare(_ptINF, "CX", "", 1202.186, _currencyJPY, _notYYFare);
    fareMarket.addFare(_ptADT, "CX", "-/CAAS", 200, _currencyUSD, _notYYFare);

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 1200, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT(fareCompInfo.matchedFares().empty());
  }

  // We should passed for 1 fare with JPY currency for calc amount 1200
  void testMatchAmountForJPYCurrency()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyJPY, _currencyJPY);
    PaxTypeFare& validFare1 =
        fareMarket.addFare(_ptADT, "CX", "-/CAAS", 1198.984, _currencyJPY, _notYYFare)->_ptf;
    PaxTypeFare& validFare2 =
        fareMarket.addFare(_ptADT, "CX", "-/CAAS", 1202.186, _currencyJPY, _notYYFare)->_ptf;
    PaxTypeFare& validFare3 =
        fareMarket.addFare(_ptADT, "CX", "-/CAAS", 1200.476, _currencyJPY, _notYYFare)->_ptf;

    FareCompInfoOverride fareCompInfo(fareMarket, "CX/CAAS", 1200, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    std::vector<PaxTypeFare*> expect;
    expect += &validFare3;
    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(validFare1.isCategoryValid(31) == false);
    CPPUNIT_ASSERT(validFare2.isCategoryValid(31) == false);

    CPPUNIT_ASSERT(validFare3.isCategoryValid(31) == true);
    CPPUNIT_ASSERT_EQUAL(validFare3.createFareBasis(NULL),
                         std::string(fareCompInfo.fareBasisCode().c_str()));
  }

  // We should passed for 1 fare with JPY currency for calc amount 1201
  void testMatchAmountPlusOneForJPYCurrency()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyJPY, _currencyJPY);
    PaxTypeFare& validFare1 =
        fareMarket.addFare(_ptADT, "CX", "-/CAAS", 1198.984, _currencyJPY, _notYYFare)->_ptf;
    PaxTypeFare& validFare2 =
        fareMarket.addFare(_ptADT, "CX", "-/CAAS", 1202.186, _currencyJPY, _notYYFare)->_ptf;
    PaxTypeFare& validFare3 =
        fareMarket.addFare(_ptADT, "CX", "-/CAAS", 1200.476, _currencyJPY, _notYYFare)->_ptf;

    FareCompInfoOverride fareCompInfo(fareMarket, "CX/CAAS", 1201, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    std::vector<PaxTypeFare*> expect;
    expect += &validFare3;
    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(validFare1.isCategoryValid(31) == false);
    CPPUNIT_ASSERT(validFare2.isCategoryValid(31) == false);

    CPPUNIT_ASSERT(validFare3.isCategoryValid(31) == true);
    CPPUNIT_ASSERT_EQUAL(validFare3.createFareBasis(NULL),
                         std::string(fareCompInfo.fareBasisCode().c_str()));
    CPPUNIT_ASSERT_EQUAL(std::string(fareCompInfo.fareBasisCode().c_str()),
                         validFare3.createFareBasis(const_cast<RexPricingTrx&>(*_trx)));
  }

  // We should failed because none fare have right basis code for ADT fares
  void testNoMatchFareClassForUSDCurrency()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    fareMarket.addFare(_ptADT, "CX", "", 200.47867, _currencyUSD, _notYYFare);
    fareMarket.addFare(_ptINF, "CX", "", 200.50156, _currencyUSD, _notYYFare);
    fareMarket.addFare(_ptADT, "CX", "-/CAAS", 200.48717, _currencyJPY, _notYYFare);

    FareCompInfoOverride fareCompInfo(fareMarket, "CX/CAAS", 200, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT(fareCompInfo.matchedFares().empty());
  }

  // We should failed because none fare have right fare amount for ADT fares
  void testNoMatchAmountForUSDCurrency()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    fareMarket.addFare(_ptADT, "CX", "", 200.47867, _currencyUSD, _notYYFare);
    fareMarket.addFare(_ptINF, "CX", "", 200.50156, _currencyUSD, _notYYFare);
    fareMarket.addFare(_ptADT, "CX", "-/CAAS", 200.48717, _currencyUSD, _notYYFare);

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 200, _zeroMileageSurcharge, _noHIP);
    fareCompInfo.fareMarket() = &fareMarket;
    fareCompInfo.fareBasisCode() = "CX";
    fareCompInfo.tktFareCalcFareAmt() = 200;

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT(fareCompInfo.matchedFares().empty());
  }

  // We should passed for 1 fare with USD currency for calc amount 1200
  void testMatchAmountForUSDCurrency()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    PaxTypeFare& validFare1 =
        fareMarket.addFare(_ptADT, "CX", "-/CAAS", 200.46867, _currencyUSD, _notYYFare)->_ptf;
    PaxTypeFare& validFare2 =
        fareMarket.addFare(_ptADT, "CX", "-/CAAS", 200.50156, _currencyUSD, _notYYFare)->_ptf;
    PaxTypeFare& validFare3 =
        fareMarket.addFare(_ptADT, "CX", "-/CAAS", 200.48717, _currencyUSD, _notYYFare)->_ptf;

    FareCompInfoOverride fareCompInfo(fareMarket, "CX/CAAS", 200.48, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    std::vector<PaxTypeFare*> expect;
    expect += &validFare3;
    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(validFare1.isCategoryValid(31) == false);
    CPPUNIT_ASSERT(validFare2.isCategoryValid(31) == false);

    CPPUNIT_ASSERT(validFare3.isCategoryValid(31) == true);
    CPPUNIT_ASSERT_EQUAL(std::string(fareCompInfo.fareBasisCode().c_str()),
                         validFare3.createFareBasis(NULL));
    CPPUNIT_ASSERT_EQUAL(std::string(fareCompInfo.fareBasisCode().c_str()),
                         validFare3.createFareBasis(const_cast<RexPricingTrx&>(*_trx)));
  }

  // We should passed for 1 fare with USD currency for calc amount 1201
  void testMatchAmountPlusOneForUSDCurrency()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    PaxTypeFare& validFare1 =
        fareMarket.addFare(_ptADT, "CX", "-/CAAS", 200.46867, _currencyUSD, _notYYFare)->_ptf;
    PaxTypeFare& validFare2 =
        fareMarket.addFare(_ptADT, "CX", "-/CAAS", 200.50156, _currencyUSD, _notYYFare)->_ptf;
    PaxTypeFare& validFare3 =
        fareMarket.addFare(_ptADT, "CX", "-/CAAS", 200.48717, _currencyUSD, _notYYFare)->_ptf;

    FareCompInfoOverride fareCompInfo(fareMarket, "CX/CAAS", 200.49, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    //    rexFareSelector.selectFare(fareCompInfo, fares, *_trx, _notYYFare, _directionalityTO);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    std::vector<PaxTypeFare*> expect;
    expect += &validFare3;
    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(validFare1.isCategoryValid(31) == false);
    CPPUNIT_ASSERT(validFare2.isCategoryValid(31) == false);

    CPPUNIT_ASSERT(validFare3.isCategoryValid(31) == true);
    CPPUNIT_ASSERT_EQUAL(std::string(fareCompInfo.fareBasisCode().c_str()),
                         validFare3.createFareBasis(NULL));
    CPPUNIT_ASSERT_EQUAL(std::string(fareCompInfo.fareBasisCode().c_str()),
                         validFare3.createFareBasis(const_cast<RexPricingTrx&>(*_trx)));
  }

  // We should passed for 1 fare with 9% variance
  void testMatchLowerLimitVarianceAmountForUSDCurrency()
  {
    std::vector<PaxTypeFare*> expect;
    double varianceFactor = 0.09;
    MoneyAmount baseFareAmt = 899.13;
    MoneyAmount fareCalcFareAmt = baseFareAmt * (1 + varianceFactor);

    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    fareMarket.addFare(_ptADT, "CX", "-/CAAS", baseFareAmt - 0.01, _currencyUSD, _notYYFare);
    expect += fareMarket.addFare(_ptADT, "CX", "-/CAAS", baseFareAmt, _currencyUSD, _notYYFare);

    FareCompInfoOverride fareCompInfo(
        fareMarket, "CX/CAAS", fareCalcFareAmt, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector._lastChanceForVariance = true;
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT_EQUAL(std::string(fareCompInfo.fareBasisCode().c_str()),
                         fareCompInfo.getMatchedFares().front()->createFareBasis(NULL));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        baseFareAmt, fareCompInfo.getMatchedFares().front()->fareAmount(), EPSILON);
  }

  // We should passed for 1 fare with 9% variance
  void testMatchUpperLimitVarianceAmountForUSDCurrency()
  {
    std::vector<PaxTypeFare*> expect;
    double varianceFactor = 0.09;
    MoneyAmount baseFareAmt = 899.13;
    MoneyAmount fareCalcFareAmt = baseFareAmt * (1 - varianceFactor);

    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    expect += fareMarket.addFare(_ptADT, "CX", "-/CAAS", baseFareAmt, _currencyUSD, _notYYFare);
    fareMarket.addFare(_ptADT, "CX", "-/CAAS", baseFareAmt + 0.01, _currencyUSD, _notYYFare);

    FareCompInfoOverride fareCompInfo(
        fareMarket, "CX/CAAS", fareCalcFareAmt, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector._lastChanceForVariance = true;
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT_EQUAL(std::string(fareCompInfo.fareBasisCode().c_str()),
                         fareCompInfo.getMatchedFares().front()->createFareBasis(NULL));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        baseFareAmt, fareCompInfo.getMatchedFares().front()->fareAmount(), EPSILON);
  }

  // We should passed for 1 fare with 9% variance
  void testMatchVarianceAmountForUSDCurrency()
  {
    std::vector<PaxTypeFare*> expect;
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    fareMarket.addFare(_ptADT, "CX", "-/CAAS", 182.4367, _currencyUSD, _notYYFare);
    fareMarket.addFare(_ptADT, "CX", "-/CAAS", 182.4369, _currencyUSD, _notYYFare);
    fareMarket.addFare(_ptADT, "CX", "-/CAAS", 218.5449, _currencyUSD, _notYYFare);
    expect += fareMarket.addFare(_ptADT, "CX", "-/CAAS", 202.5451, _currencyUSD, _notYYFare);
    fareMarket.addFare(_ptADT, "CX", "-/CAAS", 218.5451, _currencyUSD, _notYYFare);

    FareCompInfoOverride fareCompInfo(fareMarket, "CX/CAAS", 200.49, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector._lastChanceForVariance = true;
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT_EQUAL(std::string(fareCompInfo.fareBasisCode().c_str()),
                         fareCompInfo.getMatchedFares().front()->createFareBasis(NULL));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        202.5451, fareCompInfo.getMatchedFares().front()->fareAmount(), EPSILON);
  }

  // only one valid YY fare withe valid paxType, FBC, currency, amount
  void testMatchYYFare()
  {
    std::vector<PaxTypeFare*> expect;
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    fareMarket.addFare(_ptADT, "CX", "", 100, _currencyUSD, _notYYFare);
    fareMarket.addFare(_ptADT, "CX", "", 150, _currencyUSD, _notYYFare);
    expect += fareMarket.addFare(_ptADT, "YX", "", 200, _currencyUSD, _YYFare);

    FareCompInfoOverride fareCompInfo(fareMarket, "YX", 200, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(fareCompInfo.getMatchedFares().front()->isCategoryValid(31) == true);
    CPPUNIT_ASSERT(fareCompInfo.getMatchedFares().front()->fare()->isIndustry() == _YYFare);
  }

  // test fail all fares for HIP
  void testNoMatchHIP()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    fareMarket.addFare(_ptADT, "CX", "-/CAAS", 300, _currencyUSD, _notYYFare);
    fareMarket.addFare(_ptADT, "CX", "-/CAAS", 350, _currencyUSD, _notYYFare);
    fareMarket.addFare(_ptADT, "CX", "", 400, _currencyUSD, _notYYFare);

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 400, _zeroMileageSurcharge, _HIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT(fareCompInfo.matchedFares().empty());
  }

  // test match one carrier fare for HIP
  void testMatchHIP()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    PaxTypeFare* fare300 =
        &fareMarket.addFare(_ptADT, "CX", "", 300, _currencyUSD, _notYYFare)->_ptf;
    PaxTypeFare* fare350 =
        &fareMarket.addFare(_ptADT, "CX", "", 350, _currencyUSD, _notYYFare)->_ptf;
    fareMarket.addFare(_ptADT, "CX", "", 400, _currencyUSD, _notYYFare);

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 400, _zeroMileageSurcharge, _HIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    std::vector<PaxTypeFare*> expected{fare350, fare300};
    CPPUNIT_ASSERT_EQUAL(expected, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(fareCompInfo.getMatchedFares().front()->isCategoryValid(31) == true);
    CPPUNIT_ASSERT(fareCompInfo.getMatchedFares().front()->fareAmount() <
                   fareCompInfo.tktFareCalcFareAmt());
  }

  // test match one YY fare for HIP
  void testMatchHIPForYY()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    PaxTypeFare* fare200 = &fareMarket.addFare(_ptADT, "YX", "", 200, _currencyUSD, _YYFare)->_ptf;
    PaxTypeFare* fare300 = &fareMarket.addFare(_ptADT, "YX", "", 300, _currencyUSD, _YYFare)->_ptf;
    fareMarket.addFare(_ptADT, "CX", "", 350, _currencyUSD, _YYFare);
    fareMarket.addFare(_ptADT, "YX", "", 400, _currencyUSD, _YYFare);

    FareCompInfoOverride fareCompInfo(fareMarket, "YX", 400, _zeroMileageSurcharge, _HIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    std::vector<PaxTypeFare*> expected{fare300, fare200};
    CPPUNIT_ASSERT_EQUAL(expected, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(fareCompInfo.getMatchedFares().front()->isCategoryValid(31) == true);
    CPPUNIT_ASSERT(fareCompInfo.getMatchedFares().front()->fare()->isIndustry() == _YYFare);
    CPPUNIT_ASSERT(fareCompInfo.getMatchedFares().front()->fareAmount() <
                   fareCompInfo.tktFareCalcFareAmt());
  }

  // match process for carrier fare
  void testMatchProcess()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    PaxTypeFare& validFare1 =
        fareMarket.addFare(_ptADT, "CX", "", 400, _currencyUSD, _YYFare)->_ptf;
    PaxTypeFare& validFare2 =
        fareMarket.addFare(_ptADT, "CX", "", 300, _currencyUSD, _notYYFare)->_ptf;
    PaxTypeFare& validFare3 =
        fareMarket.addFare(_ptADT, "CX", "", 300, _currencyUSD, _YYFare)->_ptf;
    PaxTypeFare& validFare4 =
        fareMarket.addFare(_ptADT, "CX", "", 400, _currencyUSD, _notYYFare)->_ptf;

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 300, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    std::vector<PaxTypeFare*> expect;
    expect += &validFare2;
    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());
    CPPUNIT_ASSERT(validFare1.isCategoryValid(31) == false);
    CPPUNIT_ASSERT(validFare2.isCategoryValid(31) == true);
    CPPUNIT_ASSERT(validFare3.isCategoryValid(31) == false);
    CPPUNIT_ASSERT(validFare4.isCategoryValid(31) == false);
    CPPUNIT_ASSERT_EQUAL(&validFare2, fareCompInfo.getMatchedFares().front());
  }

  // match process for YY fare
  void testMatchProcessYY()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    PaxTypeFare& validFare1 =
        fareMarket.addFare(_ptADT, "YX", "", 200, _currencyUSD, _notYYFare)->_ptf;
    PaxTypeFare& validFare2 =
        fareMarket.addFare(_ptADT, "YX", "", 300, _currencyUSD, _notYYFare)->_ptf;
    PaxTypeFare& validFare3 =
        fareMarket.addFare(_ptADT, "YX", "", 350, _currencyUSD, _YYFare)->_ptf;
    PaxTypeFare& validFare4 =
        fareMarket.addFare(_ptADT, "YX", "", 400, _currencyUSD, _YYFare)->_ptf;

    FareCompInfoOverride fareCompInfo(fareMarket, "YX", 350, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    std::vector<PaxTypeFare*> expect;
    expect += &validFare3;
    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(validFare1.isCategoryValid(31) == false);
    CPPUNIT_ASSERT(validFare2.isCategoryValid(31) == false);
    CPPUNIT_ASSERT(validFare3.isCategoryValid(31) == true);
    CPPUNIT_ASSERT(validFare3.fare()->isIndustry() == _YYFare);
    CPPUNIT_ASSERT(validFare4.isCategoryValid(31) == false);
  }

  // match process for carrier fare with HIP
  void testMatchProcessHIP()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    PaxTypeFare& validFare1 = fareMarket.addFare(_ptADT, "CX", "", 300, "USD", _notYYFare)->_ptf;
    PaxTypeFare& validFare2 = fareMarket.addFare(_ptADT, "CX", "", 301, "USD", _notYYFare)->_ptf;
    PaxTypeFare& validFare3 = fareMarket.addFare(_ptADT, "CX", "", 250, "USD", _notYYFare)->_ptf;
    PaxTypeFare& validFare4 = fareMarket.addFare(_ptADT, "CX", "", 402, "USD", _notYYFare)->_ptf;

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 300, _zeroMileageSurcharge, _HIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    std::vector<PaxTypeFare*> expect;
    expect += &validFare3;
    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(!validFare1.isCategoryValid(31));
    CPPUNIT_ASSERT(!validFare2.isCategoryValid(31));
    CPPUNIT_ASSERT(validFare3.nucFareAmount() < fareCompInfo.tktFareCalcFareAmt());
    CPPUNIT_ASSERT(validFare3.isCategoryValid(31));
    CPPUNIT_ASSERT(!validFare4.isCategoryValid(31));
  }

  // match process for YY fare with HIP
  void testMatchProcessHIPandYY()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    PaxTypeFare& validFare1 =
        fareMarket.addFare(_ptADT, "YX", "", 200, _currencyUSD, _YYFare)->_ptf;
    PaxTypeFare& validFare2 =
        fareMarket.addFare(_ptADT, "YX", "", 300, _currencyUSD, _notYYFare)->_ptf;
    PaxTypeFare& validFare3 =
        fareMarket.addFare(_ptADT, "YX", "", 402, _currencyUSD, _YYFare)->_ptf;
    PaxTypeFare& validFare4 =
        fareMarket.addFare(_ptADT, "YX", "", 400, _currencyUSD, _notYYFare)->_ptf;

    FareCompInfoOverride fareCompInfo(fareMarket, "YX", 400, _zeroMileageSurcharge, _HIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    std::vector<PaxTypeFare*> expect;
    expect += &validFare2;
    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(!validFare1.isCategoryValid(31));
    CPPUNIT_ASSERT(validFare2.isCategoryValid(31));
    CPPUNIT_ASSERT(!validFare2.fare()->isIndustry());
    CPPUNIT_ASSERT(validFare2.nucFareAmount() < fareCompInfo.tktFareCalcFareAmt());
    CPPUNIT_ASSERT(!validFare3.isCategoryValid(31));
    CPPUNIT_ASSERT(!validFare4.isCategoryValid(31));
  }

  // match process for carrier fare with 9% variance
  void testMatchProcessVariance()
  {
    std::vector<PaxTypeFare*> expect;
    MoneyAmount baseFareAmount = 294;
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyJPY, _currencyJPY);
    fareMarket.addFare(_ptADT, "CX", "", baseFareAmount - 24, _currencyJPY, _notYYFare);
    expect += fareMarket.addFare(_ptADT, "CX", "", baseFareAmount, _currencyJPY, _notYYFare);
    fareMarket.addFare(_ptADT, "CX", "", baseFareAmount + 8, _currencyJPY, _YYFare);
    fareMarket.addFare(_ptADT, "CX", "", baseFareAmount + 26, _currencyJPY, _notYYFare);

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 300, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector._lastChanceForVariance = true;
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        baseFareAmount, fareCompInfo.getMatchedFares().front()->fareAmount(), EPSILON);
  }

  // match process for YY fare with 9% variance
  void testMatchProcessVarianceYY()
  {
    std::vector<PaxTypeFare*> expect;
    MoneyAmount baseFareAmount = 325;
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyJPY, _currencyJPY);
    fareMarket.addFare(_ptADT, "YX", "", baseFareAmount - 125, _currencyJPY, _YYFare);
    fareMarket.addFare(_ptADT, "YX", "", baseFareAmount - 25, _currencyJPY, _YYFare);
    expect += fareMarket.addFare(_ptADT, "YX", "", baseFareAmount, _currencyJPY, _YYFare);
    fareMarket.addFare(_ptADT, "YX", "", baseFareAmount + 50, _currencyJPY, _notYYFare);

    FareCompInfoOverride fareCompInfo(fareMarket, "YX", 320, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector._lastChanceForVariance = true;
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());
    CPPUNIT_ASSERT_EQUAL(_YYFare, fareCompInfo.getMatchedFares().front()->fare()->isIndustry());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        baseFareAmount, fareCompInfo.getMatchedFares().front()->fareAmount(), EPSILON);
  }

  // no match process for carrier fare with HIP and variance
  void testNoMatchProcessHIPAndVariance()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    fareMarket.addFare(_ptADT, "CX", "", 400 * 0.9, _currencyUSD, _notYYFare);

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 400, _zeroMileageSurcharge, _HIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector._lastChanceForVariance = true;
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT(fareCompInfo.matchedFares().empty());
  }

  void testMatchWhenFareCalcTimesSurchargeEqualsFareAmt()
  {
    std::vector<PaxTypeFare*> expect;
    MoneyAmount baseFareAmount = 899.13;

    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    expect += fareMarket.addFare(_ptADT, "CX", "", baseFareAmount, _currencyUSD, _notYYFare);
    fareMarket.addFare(_ptADT, "CX", "", 1057, _currencyUSD, _notYYFare);

    uint16_t mileageSurchargePctg = 15;
    MoneyAmount fareAmountWithMileage = ((100.0 + mileageSurchargePctg) * baseFareAmount) / 100.0;

    FareCompInfoOverride fareCompInfo(
        fareMarket, "CX", fareAmountWithMileage, mileageSurchargePctg, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(fareCompInfo.getMatchedFares().front()->isCategoryValid(31) == true);
  }

  void testMatchForHIPWhenFareCalcTimesSurchargeEqualsFareAmt()
  {
    std::vector<PaxTypeFare*> expect;
    MoneyAmount baseFareAmount = 899.13;

    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    fareMarket.addFare(_ptADT, "CX", "", baseFareAmount, _currencyUSD, _notYYFare);
    expect += fareMarket.addFare(_ptADT, "CX", "", baseFareAmount - 1, _currencyUSD, _notYYFare);
    fareMarket.addFare(_ptADT, "CX", "", 899.15, _currencyUSD, _notYYFare);

    uint16_t mileageSurchargePctg = 15;
    MoneyAmount fareAmountWithMileage = ((100 + mileageSurchargePctg) * baseFareAmount) / 100;

    FareCompInfoOverride fareCompInfo(
        fareMarket, "CX", fareAmountWithMileage, mileageSurchargePctg, _HIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(fareCompInfo.getMatchedFares().front()->isCategoryValid(31) == true);
    CPPUNIT_ASSERT(fareCompInfo.getMatchedFares().front()->fareAmount() < baseFareAmount);
  }

  void testMatchWhenFareCalcMinusVarianceTimesSurchargeEqualsFareAmt()
  {
    std::vector<PaxTypeFare*> expect;
    double varianceFactor = 0.09;
    MoneyAmount baseFareAmount = 899.13;
    MoneyAmount fareCalcFareAmt = baseFareAmount * (1 + varianceFactor);

    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    fareMarket.addFare(_ptADT, "CX", "", baseFareAmount - 1, _currencyUSD, _notYYFare);
    expect += fareMarket.addFare(_ptADT, "CX", "", baseFareAmount, _currencyUSD, _notYYFare);
    fareMarket.addFare(_ptADT, "CX", "", 818.19, _currencyUSD, _notYYFare);

    uint16_t mileageSurchargePctg = 15;
    MoneyAmount fareCalcFareAmtPlusMileage = ((100 + mileageSurchargePctg) * fareCalcFareAmt) / 100;

    FareCompInfoOverride fareCompInfo(
        fareMarket, "CX", fareCalcFareAmtPlusMileage, mileageSurchargePctg, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector._lastChanceForVariance = true;
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        baseFareAmount, fareCompInfo.getMatchedFares().front()->fareAmount(), EPSILON);
  }

  bool matchForDate(std::vector<int> dateSequence, int date)
  {
    MoneyAmount baseFareAmount = 200;
    MoneyAmount fareCalcFareAmt = baseFareAmount;
    _trx->setOriginalTktIssueDT() = _localTime;
    _trx->lastTktReIssueDT() = _localTime;
    DateTime commenceDate = _localTime.subtractDays(5);

    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD, commenceDate);
    fareMarket.setFareCompInfo("CX", fareCalcFareAmt, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);

    std::vector<int>::iterator pos = dateSequence.begin();
    for (; pos != dateSequence.end(); pos++)
      rexFareSelector.addFareForDate(
          fareMarket, *pos, _ptADT, "CX", "", baseFareAmount, _currencyUSD, _notYYFare);

    rexFareSelector.process();
    const PaxTypeFare* validFare = fareMarket.fareCompInfo().matchedFares().front().get();

    return validFare && (validFare->createFareBasis(const_cast<RexPricingTrx&>(*_trx)) ==
                         fareMarket.fareCompInfo().fareBasisCode()) &&
           fabs(validFare->nucFareAmount() - baseFareAmount) < EPSILON &&
           fareMarket.fareCompInfo().fareMatchingPhase() == date;
  }

  void testMatchHIP_SameFaresAtpAndSita()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    PaxTypeFare& validFare1 =
        fareMarket.addFareWithVendor(_ptADT, "CX", "", 380, _currencyUSD, _notYYFare, "SITA")->_ptf;
    PaxTypeFare& validFare2 =
        fareMarket.addFareWithVendor(_ptADT, "CX", "", 379, _currencyUSD, _notYYFare, "SITA")->_ptf;
    PaxTypeFare& validFare3 =
        fareMarket.addFareWithVendor(_ptADT, "CX", "", 400, _currencyUSD, _notYYFare, "ATP")->_ptf;
    PaxTypeFare& validFare4 =
        fareMarket.addFareWithVendor(_ptADT, "CX", "", 380, _currencyUSD, _notYYFare, "ATP")->_ptf;

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 400, _zeroMileageSurcharge, _HIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    std::vector<PaxTypeFare*> expect;
    expect += &validFare4;
    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(!validFare1.isCategoryValid(31));
    CPPUNIT_ASSERT(!validFare2.isCategoryValid(31));
    CPPUNIT_ASSERT(!validFare3.isCategoryValid(31));
    CPPUNIT_ASSERT(validFare4.isCategoryValid(31));
    CPPUNIT_ASSERT(validFare4.nucFareAmount() < fareCompInfo.tktFareCalcFareAmt());
  }

  void testMatchHIP_FaresAtpAndSitaDiffAmount()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);

    PaxTypeFare& validFare1 =
        fareMarket.addFareWithVendor(_ptADT, "CX", "", 381, _currencyUSD, _notYYFare, "SITA")->_ptf;
    PaxTypeFare& validFare2 =
        fareMarket.addFareWithVendor(_ptADT, "CX", "", 379, _currencyUSD, _YYFare, "ATP")->_ptf;
    PaxTypeFare& validFare3 =
        fareMarket.addFareWithVendor(_ptADT, "CX", "", 400, _currencyUSD, _notYYFare, "ATP")->_ptf;
    PaxTypeFare& validFare4 =
        fareMarket.addFareWithVendor(_ptADT, "CX", "", 380, _currencyUSD, _YYFare, "ATP")->_ptf;

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 400, _zeroMileageSurcharge, _HIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    //    rexFareSelector.selectFare(fareCompInfo, fares, *_trx, _notYYFare, _directionalityTO);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    std::vector<PaxTypeFare*> expect;
    expect += &validFare1;
    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(validFare1.isCategoryValid(31));
    CPPUNIT_ASSERT(!validFare2.isCategoryValid(31));
    CPPUNIT_ASSERT(!validFare3.isCategoryValid(31));
    CPPUNIT_ASSERT(!validFare4.isCategoryValid(31));
    CPPUNIT_ASSERT(validFare1.nucFareAmount() < fareCompInfo.tktFareCalcFareAmt());
  }

  void testMatchHIP_FaresAtpAndSita()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    PaxTypeFare& validFare1 =
        fareMarket.addFareWithVendor(_ptADT, "CX", "", 380, _currencyUSD, _notYYFare, "SITA")->_ptf;
    PaxTypeFare& validFare2 =
        fareMarket.addFareWithVendor(_ptADT, "CX", "", 379, _currencyUSD, _YYFare, "ATP")->_ptf;
    PaxTypeFare& validFare3 =
        fareMarket.addFareWithVendor(_ptADT, "CX", "", 400, _currencyUSD, _notYYFare, "ATP")->_ptf;
    PaxTypeFare& validFare4 =
        fareMarket.addFareWithVendor(_ptADT, "CX", "", 380, _currencyUSD, _notYYFare, "ATP")->_ptf;

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 400, _zeroMileageSurcharge, _HIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    std::vector<PaxTypeFare*> expect;
    expect += &validFare4;
    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(!validFare1.isCategoryValid(31));
    CPPUNIT_ASSERT(!validFare2.isCategoryValid(31));
    CPPUNIT_ASSERT(!validFare3.isCategoryValid(31));
    CPPUNIT_ASSERT(validFare4.isCategoryValid(31));
    CPPUNIT_ASSERT(validFare4.nucFareAmount() < fareCompInfo.tktFareCalcFareAmt());
  }

  void testMatchVariance_SameFaresAtpAndSita()
  {
    MoneyAmount baseFareAmount = 294;
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyJPY, _currencyJPY);
    fareMarket.addFareWithVendor(
        _ptADT, "CX", "", baseFareAmount - 24, _currencyJPY, _notYYFare, "ATP");
    fareMarket.addFareWithVendor(
        _ptADT, "CX", "", baseFareAmount, _currencyJPY, _notYYFare, "SITA");
    fareMarket.addFareWithVendor(
        _ptADT, "CX", "", baseFareAmount + 8, _currencyJPY, _YYFare, "ATP");
    fareMarket.addFareWithVendor(
        _ptADT, "CX", "", baseFareAmount + 26, _currencyJPY, _notYYFare, "ATP");
    PaxTypeFare& validFare =
        fareMarket.addFareWithVendor(
                       _ptADT, "CX", "", baseFareAmount, _currencyJPY, _notYYFare, "ATP")->_ptf;

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 300, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector._lastChanceForVariance = true;
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    std::vector<PaxTypeFare*> expect;
    expect += &validFare;
    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());
  }

  void testMatchVariance_FaresAtpAndSitaDiffAmount()
  {
    std::vector<PaxTypeFare*> expect;

    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyJPY, _currencyJPY);
    MoneyAmount baseFareAmount = 294;

    fareMarket.addFareWithVendor(
        _ptADT, "CX", "", baseFareAmount - 24, _currencyJPY, _notYYFare, "ATP");
    fareMarket.addFareWithVendor(
        _ptADT, "CX", "", baseFareAmount, _currencyJPY, _notYYFare, "SITA");
    fareMarket.addFareWithVendor(
        _ptADT, "CX", "", baseFareAmount + 8, _currencyJPY, _YYFare, "ATP");
    fareMarket.addFareWithVendor(
        _ptADT, "CX", "", baseFareAmount + 26, _currencyJPY, _notYYFare, "ATP");
    expect += fareMarket.addFareWithVendor(
        _ptADT, "CX", "", baseFareAmount - 1, _currencyJPY, _notYYFare, "ATP");

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 300, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector._lastChanceForVariance = true;
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());
  }

  void testMatchVariance_FaresAtpAndSita()
  {
    std::vector<PaxTypeFare*> expect;
    MoneyAmount baseFareAmount = 294;
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyJPY, _currencyJPY);
    fareMarket.addFareWithVendor(
        _ptADT, "CX", "", baseFareAmount - 24, _currencyJPY, _notYYFare, "ATP");
    fareMarket.addFareWithVendor(
        _ptADT, "CX", "", baseFareAmount, _currencyJPY, _notYYFare, "SITA");
    fareMarket.addFareWithVendor(
        _ptADT, "CX", "", baseFareAmount + 8, _currencyJPY, _YYFare, "ATP");
    fareMarket.addFareWithVendor(
        _ptADT, "CX", "", baseFareAmount + 26, _currencyJPY, _notYYFare, "ATP");
    expect += fareMarket.addFareWithVendor(
        _ptADT, "CX", "", baseFareAmount - 8, _currencyJPY, _notYYFare, "ATP");

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 300, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector._lastChanceForVariance = true;
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());
  }

  void testMatchAmount_SameFaresAtpAndSita()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    PaxTypeFare& validFare1 =
        fareMarket.addFareWithVendor(
                       _ptADT, "CX", "-/CAAS", 200.46867, _currencyUSD, _notYYFare, "ATP")->_ptf;
    PaxTypeFare& validFare2 =
        fareMarket.addFareWithVendor(
                       _ptADT, "CX", "-/CAAS", 200.48717, _currencyUSD, _notYYFare, "SITA")
            ->_ptf; // OK
    PaxTypeFare& validFare3 =
        fareMarket.addFareWithVendor(
                       _ptADT, "CX", "-/CAAS", 200.50156, _currencyUSD, _notYYFare, "ATP")->_ptf;
    PaxTypeFare& validFare4 =
        fareMarket.addFareWithVendor(
                       _ptADT, "CX", "-/CAAS", 200.48700, _currencyUSD, _notYYFare, "ATP")
            ->_ptf; // OK
    PaxTypeFare& validFare5 =
        fareMarket.addFareWithVendor(
                       _ptADT, "CX", "-/CAAS", 200.48704, _currencyUSD, _notYYFare, "SITA")
            ->_ptf; // OK

    FareCompInfoOverride fareCompInfo(fareMarket, "CX/CAAS", 200.48, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    std::vector<PaxTypeFare*> expect;
    expect += &validFare4;
    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(validFare1.isCategoryValid(31) == false);
    CPPUNIT_ASSERT(validFare2.isCategoryValid(31) == false);
    CPPUNIT_ASSERT(validFare3.isCategoryValid(31) == false);
    CPPUNIT_ASSERT(validFare4.isCategoryValid(31) == true);
    CPPUNIT_ASSERT(validFare5.isCategoryValid(31) == false);

    CPPUNIT_ASSERT_EQUAL(std::string(fareCompInfo.fareBasisCode().c_str()),
                         validFare4.createFareBasis(const_cast<RexPricingTrx&>(*_trx)));
    CPPUNIT_ASSERT_EQUAL(std::string(fareCompInfo.fareBasisCode().c_str()),
                         validFare4.createFareBasis(NULL));
  }

  void testMatchAmount_FaresAtpAndSita_SFBetterMatch()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    PaxTypeFare& validFare1 =
        fareMarket.addFareWithVendor(
                       _ptADT, "CX", "-/CAAS", 200.46867, _currencyUSD, _notYYFare, "ATP")->_ptf;
    PaxTypeFare& validFare2 =
        fareMarket.addFareWithVendor(
                       _ptADT, "CX", "-/CAAS", 200.48104, _currencyUSD, _notYYFare, "SITA")->_ptf;
    PaxTypeFare& validFare3 =
        fareMarket.addFareWithVendor(
                       _ptADT, "CX", "-/CAAS", 200.50156, _currencyUSD, _notYYFare, "ATP")->_ptf;
    PaxTypeFare& validFare4 =
        fareMarket.addFareWithVendor(
                       _ptADT, "CX", "-/CAAS", 200.49001, _currencyUSD, _notYYFare, "ATP")->_ptf;
    PaxTypeFare& validFare5 =
        fareMarket.addFareWithVendor(
                       _ptADT, "CX", "-/CAAS", 200.46017, _currencyUSD, _notYYFare, "SITA")->_ptf;

    FareCompInfoOverride fareCompInfo(fareMarket, "CX/CAAS", 200.48, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    std::vector<PaxTypeFare*> expect;
    expect += &validFare2;
    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(validFare1.isCategoryValid(31) == false);
    CPPUNIT_ASSERT(validFare2.isCategoryValid(31) == true);
    CPPUNIT_ASSERT(validFare3.isCategoryValid(31) == false);
    CPPUNIT_ASSERT(validFare4.isCategoryValid(31) == false);
    CPPUNIT_ASSERT(validFare5.isCategoryValid(31) == false);

    CPPUNIT_ASSERT_EQUAL(std::string(fareCompInfo.fareBasisCode().c_str()),
                         validFare2.createFareBasis(const_cast<RexPricingTrx&>(*_trx)));
    CPPUNIT_ASSERT_EQUAL(std::string(fareCompInfo.fareBasisCode().c_str()),
                         validFare2.createFareBasis(NULL));
  }

  void testMatchOutboundFareForSideTripOutboundFareMarket()
  {
    std::vector<PaxTypeFare*> expect;
    Directionality fareDirection = _directionalityFROM;

    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    fareMarket.direction() = FMDirection::OUTBOUND;

    expect += fareMarket.addFare(_ptADT, "CX", "", 200, _currencyUSD, _notYYFare, fareDirection);

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 200, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());

    CPPUNIT_ASSERT(fareCompInfo.getMatchedFares().front()->isCategoryValid(31) == true);
  }

  void testMatchOutboundFareForSideTripInboundFareMarket()
  {
    std::vector<PaxTypeFare*> expect;
    Directionality fareDirection = _directionalityFROM;

    AirSeg ts1;
    AirSeg ts2;
    ts1.forcedSideTrip() = 'T';
    ts2.forcedSideTrip() = 'T';

    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    fareMarket.direction() = FMDirection::INBOUND;
    expect += fareMarket.addFare(_ptADT, "CX", "", 200, _currencyUSD, _notYYFare, fareDirection);
    fareMarket.travelSeg().push_back(&ts1);
    fareMarket.travelSeg().push_back(&ts2);

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 200, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());
    CPPUNIT_ASSERT(fareCompInfo.getMatchedFares().front()->isCategoryValid(31) == true);
  }

  void testMatchInboundFareForSideTripOutboundFareMarket()
  {
    std::vector<PaxTypeFare*> expect;
    Directionality fareDirection = _directionalityTO;

    AirSeg ts1;
    AirSeg ts2;
    ts1.forcedSideTrip() = 'T';
    ts2.forcedSideTrip() = 'T';

    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    fareMarket.direction() = FMDirection::OUTBOUND;

    expect += fareMarket.addFare(_ptADT, "CX", "", 200, _currencyUSD, _notYYFare, fareDirection);
    fareMarket.travelSeg().push_back(&ts1);
    fareMarket.travelSeg().push_back(&ts2);

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 200, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());
    CPPUNIT_ASSERT(fareCompInfo.getMatchedFares().front()->isCategoryValid(31) == true);
  }

  void testMatchInboundFareForSideTripInboundFareMarket()
  {
    std::vector<PaxTypeFare*> expect;
    Directionality fareDirection = _directionalityTO;

    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyUSD, _currencyUSD);
    fareMarket.direction() = FMDirection::INBOUND;

    expect += fareMarket.addFare(_ptADT, "CX", "", 200, _currencyUSD, _notYYFare, fareDirection);

    FareCompInfoOverride fareCompInfo(fareMarket, "CX", 200, _zeroMileageSurcharge, _noHIP);

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.retrieveVCTRTask(fareCompInfo);

    CPPUNIT_ASSERT_EQUAL(expect, fareCompInfo.getMatchedFares());
    CPPUNIT_ASSERT(fareCompInfo.getMatchedFares().front()->isCategoryValid(31) == true);
  }

  void testStoreVCTR_NotStoredForOrigTcktDate()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyJPY, _currencyJPY);

    FareComponentInfo fci;
    VCTRInfo vctrI;
    fci.vctrInfo() = &vctrI;
    fci.vctrInfo()->vctr() = VCTR("ATP", "LH", 22, "26", 0);
    fci.vctrInfo()->retrievalDate() = DateTime(2008, 11, 13);

    FareCompInfo fc;
    fc.VCTR() = VCTR("ATP", "AA", 11, "13", 0);
    fc.hasVCTR() = true;

    _trx->excFareCompInfo().push_back(&fci);
    _trx->exchangeItin()[0]->fareComponent().push_back(&fc);

    _trx->setOriginalTktIssueDT() = DateTime(2008, 11, 12);
    _trx->setupDateSeq();
    _trx->setupMipDateSeq();

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.storeVCTR();

    // current tkt date seq is original tkt date
    CPPUNIT_ASSERT(rexFareSelector._vctrStorage.empty());
  }

  void testStoreVCTR_NotStoredFcDateEqOrigDT()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyJPY, _currencyJPY);

    VCTR originalVCTR("ATP", "AA", 11, "13", 0);
    VCTR newVCTR("ATP", "LH", 22, "26", 0);

    VCTRInfo vctrI;
    vctrI.vctr() = newVCTR;
    vctrI.retrievalDate() = DateTime(2008, 11, 12);

    FareComponentInfo fci;
    fci.vctrInfo() = &vctrI;

    FareCompInfo fc;
    fc.VCTR() = originalVCTR;
    fc.hasVCTR() = true;

    _trx->excFareCompInfo().push_back(&fci);
    _trx->exchangeItin()[0]->fareComponent().push_back(&fc);

    _trx->setOriginalTktIssueDT() = DateTime(2008, 11, 12);
    _trx->setupDateSeq();
    _trx->setupMipDateSeq();

    _trx->nextTktDateSeq();

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.storeVCTR();

    CPPUNIT_ASSERT(rexFareSelector._vctrStorage.empty());
  }

  void testStoreVCTR_StoredForFcDateDifferentThanOrigDT()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyJPY, _currencyJPY);

    VCTR originalVCTR("ATP", "AA", 11, "13", 0);
    VCTR newVCTR("ATP", "LH", 22, "26", 0);
    bool originalStatus = true;
    bool newStatus = true;

    VCTRInfo vctrI;
    vctrI.vctr() = newVCTR;
    vctrI.retrievalDate() = DateTime(2008, 11, 13);

    FareComponentInfo fci;
    fci.vctrInfo() = &vctrI;

    FareCompInfo fc;
    fc.VCTR() = originalVCTR;
    fc.hasVCTR() = true;

    _trx->excFareCompInfo().push_back(&fci);
    _trx->exchangeItin()[0]->fareComponent().push_back(&fc);

    _trx->setOriginalTktIssueDT() = DateTime(2008, 11, 12);
    _trx->setupDateSeq();
    _trx->setupMipDateSeq();

    _trx->nextTktDateSeq();

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.storeVCTR();

    CPPUNIT_ASSERT(rexFareSelector._vctrStorage.size() == 1);

    CPPUNIT_ASSERT(originalVCTR == rexFareSelector._vctrStorage[0].first);
    CPPUNIT_ASSERT(originalStatus == rexFareSelector._vctrStorage[0].second);

    CPPUNIT_ASSERT(newVCTR == fc.VCTR());
    CPPUNIT_ASSERT(newStatus == fc.hasVCTR());
  }

  void testStoreRestoreVCTR_StoredAndRestoredForDateDifferentThanOrigDT()
  {
    FareMarketOverride fareMarket(_ptADT, *_trx, _currencyJPY, _currencyJPY);

    VCTR originalVCTR("ATP", "AA", 11, "13", 0);
    VCTR newVCTR("ATP", "LH", 22, "26", 0);
    bool originalStatus = true;
    bool newStatus = true;

    VCTRInfo vctrI;
    vctrI.vctr() = newVCTR;
    vctrI.retrievalDate() = DateTime(2008, 11, 13);

    FareComponentInfo fci;
    fci.vctrInfo() = &vctrI;

    FareCompInfo fc;
    fc.VCTR() = originalVCTR;
    fc.hasVCTR() = true;

    _trx->excFareCompInfo().push_back(&fci);
    _trx->exchangeItin()[0]->fareComponent().push_back(&fc);

    _trx->setOriginalTktIssueDT() = DateTime(2008, 11, 12);
    _trx->setupDateSeq();
    _trx->setupMipDateSeq();

    _trx->nextTktDateSeq();

    RexFareSelectorOverride rexFareSelector(*_trx, fareMarket);
    rexFareSelector.storeVCTR();

    CPPUNIT_ASSERT(rexFareSelector._vctrStorage.size() == 1);

    CPPUNIT_ASSERT(originalVCTR == rexFareSelector._vctrStorage[0].first);
    CPPUNIT_ASSERT(originalStatus == rexFareSelector._vctrStorage[0].second);

    CPPUNIT_ASSERT(newVCTR == fc.VCTR());
    CPPUNIT_ASSERT(newStatus == fc.hasVCTR());

    rexFareSelector.restoreVCTR();

    CPPUNIT_ASSERT(originalVCTR == fc.VCTR());
    CPPUNIT_ASSERT(originalStatus == fc.hasVCTR());
  }

  void testInvalidateClone()
  {
    RexFareSelector rexFareSelector(*_trx);
    FareMarket fm1, fm2, fm3;
    AirSeg as;
    fm2.travelSeg().push_back(&as);
    std::vector<FareMarket*> fmv;
    fmv.push_back(&fm1);
    fmv.push_back(&fm2);
    fmv.push_back(&fm1);
    fmv.push_back(&fm3);

    for (const FareMarket* fm : fmv)
      CPPUNIT_ASSERT(!fm->breakIndicator());

    rexFareSelector.invalidateClones(fm1, fmv.begin(), fmv.end());

    CPPUNIT_ASSERT(!fmv[0]->breakIndicator());
    CPPUNIT_ASSERT(!fmv[1]->breakIndicator());
    CPPUNIT_ASSERT(!fmv[2]->breakIndicator());
    CPPUNIT_ASSERT(fmv[3]->breakIndicator());
  }

  void testSelectSecondaryFareMarket_SubArea21()
  {
    FareMarket fmCrossingCxr;
    fmCrossingCxr.governingCarrier() = "AA";
    fmCrossingCxr.setHighTPMGoverningCxr( false );
    fmCrossingCxr.travelSeg().push_back(createAirSeg("MOW", "FRA", "AA")); // 1264 EH
    fmCrossingCxr.travelSeg().push_back(createAirSeg("FRA", "WAW", "BB")); // 5360 TS

    FareMarket fmTpmCxr;
    fmCrossingCxr.clone( fmTpmCxr );
    fmTpmCxr.governingCarrier() = "BB";
    fmTpmCxr.setHighTPMGoverningCxr( true );

    ExcItin exchangeItin;
    exchangeItin.fareMarket().push_back( &fmCrossingCxr );
    exchangeItin.fareMarket().push_back( &fmTpmCxr );

    FareCompInfo fci;
    fci.fareMarket() = &fmCrossingCxr;
    exchangeItin.fareComponent().push_back( &fci );

    _trx->exchangeItin().push_back( &exchangeItin );
    _trx->setIataFareSelectionApplicable( false );

    const Boundary boundary =
        TravelSegAnalysis::selectTravelBoundary( fci.fareMarket()->travelSeg() );
    CPPUNIT_ASSERT(boundary == Boundary::AREA_21);

    RexFareSelector rfs(*_trx);
    rfs.selectSecondaryFareMarket();
    CPPUNIT_ASSERT( &fmTpmCxr == fci.secondaryFareMarket() );

    fci.secondaryFareMarket() = 0;
    _trx->setIataFareSelectionApplicable( true );
    rfs.selectSecondaryFareMarket();
    CPPUNIT_ASSERT( &fmTpmCxr == fci.secondaryFareMarket() );
  }

  void testSelectSecondaryFareMarket_USCA()
  {
    FareMarket fmCrossingCxr;
    fmCrossingCxr.governingCarrier() = "AA";
    fmCrossingCxr.setHighTPMGoverningCxr( false );
    fmCrossingCxr.travelSeg().push_back(createAirSeg("DFW", "DEN", "AA"));
    fmCrossingCxr.travelSeg().push_back(createAirSeg("DEN", "BOS", "BB"));

    FareMarket fmTpmCxr;
    fmCrossingCxr.clone( fmTpmCxr );
    fmTpmCxr.governingCarrier() = "BB";
    fmTpmCxr.setHighTPMGoverningCxr( true );

    ExcItin exchangeItin;
    exchangeItin.fareMarket().push_back( &fmCrossingCxr );
    exchangeItin.fareMarket().push_back( &fmTpmCxr );

    FareCompInfo fci;
    fci.fareMarket() = &fmCrossingCxr;
    exchangeItin.fareComponent().push_back( &fci );

    _trx->exchangeItin().push_back( &exchangeItin );
    _trx->setIataFareSelectionApplicable( true );

    const Boundary boundary =
        TravelSegAnalysis::selectTravelBoundary( fci.fareMarket()->travelSeg() );
    CPPUNIT_ASSERT(boundary == Boundary::USCA);

    RexFareSelector rfs(*_trx);
    rfs.selectSecondaryFareMarket();
    CPPUNIT_ASSERT( 0 == fci.secondaryFareMarket() );
  }

  // Same nation to same nation, but not any combination of US and CA
  void testSelectSecondaryFareMarket_Except_USCA()
  {
    FareMarket fmCrossingCxr;
    fmCrossingCxr.governingCarrier() = "AA";
    fmCrossingCxr.setHighTPMGoverningCxr( false );
    fmCrossingCxr.travelSeg().push_back(createAirSeg("COR", "BUE", "AA"));
    fmCrossingCxr.travelSeg().push_back(createAirSeg("BUE", "EZE", "BB"));

    FareMarket fmTpmCxr;
    fmCrossingCxr.clone( fmTpmCxr );
    fmTpmCxr.governingCarrier() = "BB";
    fmTpmCxr.setHighTPMGoverningCxr( true );

    ExcItin exchangeItin;
    exchangeItin.fareMarket().push_back( &fmCrossingCxr );
    exchangeItin.fareMarket().push_back( &fmTpmCxr );

    FareCompInfo fci;
    fci.fareMarket() = &fmCrossingCxr;
    exchangeItin.fareComponent().push_back( &fci );

    _trx->exchangeItin().push_back( &exchangeItin );
    _trx->setIataFareSelectionApplicable( true );

    const Boundary boundary =
        TravelSegAnalysis::selectTravelBoundary( fci.fareMarket()->travelSeg() );
    CPPUNIT_ASSERT(boundary == Boundary::EXCEPT_USCA);

    RexFareSelector rfs(*_trx);
    rfs.selectSecondaryFareMarket();
    CPPUNIT_ASSERT( 0 == fci.secondaryFareMarket() );
  }

  void testSelectSecondaryFareMarket_No_VCTR()
  {
    FareMarket fmCrossingCxr;
    fmCrossingCxr.governingCarrier() = "AA";
    fmCrossingCxr.setHighTPMGoverningCxr( false );
    fmCrossingCxr.travelSeg().push_back(createAirSeg("MOW", "FRA", "AA")); // 1264 EH
    fmCrossingCxr.travelSeg().push_back(createAirSeg("FRA", "WAW", "BB")); // 5360 TS

    FareMarket fmTpmCxr;
    fmCrossingCxr.clone( fmTpmCxr );
    fmTpmCxr.governingCarrier() = "BB";
    fmTpmCxr.setHighTPMGoverningCxr( true );

    ExcItin exchangeItin;
    exchangeItin.fareMarket().push_back( &fmCrossingCxr );
    exchangeItin.fareMarket().push_back( &fmTpmCxr );

    FareCompInfo fci;
    fci.hasVCTR() = false;
    fci.fareMarket() = &fmCrossingCxr;
    exchangeItin.fareComponent().push_back( &fci );

    _trx->exchangeItin().push_back( &exchangeItin );
    _trx->setIataFareSelectionApplicable( false );

    const Boundary boundary =
        TravelSegAnalysis::selectTravelBoundary( fci.fareMarket()->travelSeg() );
    CPPUNIT_ASSERT(boundary == Boundary::AREA_21);

    RexFareSelector rfs(*_trx);
    rfs.selectSecondaryFareMarket();
    CPPUNIT_ASSERT( &fmCrossingCxr == fci.fareMarket() );
    CPPUNIT_ASSERT( &fmTpmCxr == fci.secondaryFareMarket() );

    fci.secondaryFareMarket() = 0;
    _trx->setIataFareSelectionApplicable( true );
    rfs.selectSecondaryFareMarket();
    CPPUNIT_ASSERT( &fmCrossingCxr == fci.fareMarket() );
    CPPUNIT_ASSERT( &fmTpmCxr == fci.secondaryFareMarket() );
  }

  void testSelectSecondaryFareMarket_VCTR()
  {
    FareMarket fmCrossingCxr;
    fmCrossingCxr.governingCarrier() = "AA";
    fmCrossingCxr.setHighTPMGoverningCxr( false );
    fmCrossingCxr.travelSeg().push_back(createAirSeg("MOW", "FRA", "AA")); // 1264 EH
    fmCrossingCxr.travelSeg().push_back(createAirSeg("FRA", "WAW", "BB")); // 5360 TS

    FareMarket fmTpmCxr;
    fmCrossingCxr.clone( fmTpmCxr );
    fmTpmCxr.governingCarrier() = "BB";
    fmTpmCxr.setHighTPMGoverningCxr( true );

    ExcItin exchangeItin;
    exchangeItin.fareMarket().push_back( &fmCrossingCxr );
    exchangeItin.fareMarket().push_back( &fmTpmCxr );

    FareCompInfo fci;
    fci.hasVCTR() = true;
    fci.VCTR().carrier() = fmTpmCxr.governingCarrier();
    fci.fareMarket() = &fmCrossingCxr;
    exchangeItin.fareComponent().push_back( &fci );

    _trx->exchangeItin().push_back( &exchangeItin );
    _trx->setIataFareSelectionApplicable( false );

    const Boundary boundary =
        TravelSegAnalysis::selectTravelBoundary( fci.fareMarket()->travelSeg() );
    CPPUNIT_ASSERT(boundary == Boundary::AREA_21);

    RexFareSelector rfs(*_trx);
    rfs.selectSecondaryFareMarket();
    CPPUNIT_ASSERT( &fmCrossingCxr == fci.fareMarket() );
    CPPUNIT_ASSERT( &fmTpmCxr == fci.secondaryFareMarket() );

    fci.secondaryFareMarket() = 0;
    _trx->setIataFareSelectionApplicable( true );
    rfs.selectSecondaryFareMarket();
    CPPUNIT_ASSERT( &fmTpmCxr == fci.fareMarket() );
    CPPUNIT_ASSERT( &fmCrossingCxr == fci.secondaryFareMarket() );
  }

  void testRAII()
  {
    bool var = false;
    {
      BoolRAIISwitchTrue raiiToTrue(var);
    }
    CPPUNIT_ASSERT(var);
    {
      BoolRAIISwitchFalse raiiToFalse(var);
    }
    CPPUNIT_ASSERT(!var);
  }

  void azPercentTooHighTest()
  {
    prepareAZdata();
    FareCompInfo* fci = _trx->exchangeItin().front()->fareComponent().front();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> randomRealGen(0.1, 100.0);

    static_cast<RexBaseRequest*>(_trx->getRequest())->excDiscounts().addPercentage(
        1, 100.0 + randomRealGen(gen));

    RexFareSelector rfs(*_trx);
    CPPUNIT_ASSERT_THROW(rfs.updateFCAmount(*fci), ErrorResponseException);
  }

  void markupPercentTest()
  {
    prepareAZdata();
    FareCompInfo* fci = _trx->exchangeItin().front()->fareComponent().front();
    fci->fareCalcFareAmt() = 110.0;
    fci->tktFareCalcFareAmt() = 220.0;

    static_cast<RexBaseRequest*>(_trx->getRequest())->excDiscounts().addPercentage(1, -10.0);

    RexFareSelector rfs(*_trx);
    rfs.updateFCAmount(*fci);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, fci->fareCalcFareAmt(), 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(200.0, fci->tktFareCalcFareAmt(), 0.1);
  }

  void markupAmountTest()
  {
    prepareAZdata();
    FareCompInfo* fci = _trx->exchangeItin().front()->fareComponent().front();
    _trx->exchangeItin().front()->calcCurrencyOverride() = USD;
    _trx->exchangeItin().front()->calculationCurrency() = USD;
    fci->fareCalcFareAmt() = 110.0;
    fci->tktFareCalcFareAmt() = 210.0;

    static_cast<RexBaseRequest*>(_trx->getRequest())->excDiscounts().addAmount(0, 1, -10, "EUR");

    RexFareSelector rfs(*_trx);
    rfs.updateFCAmount(*fci);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, fci->fareCalcFareAmt(), 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(200.0, fci->tktFareCalcFareAmt(), 0.1);
  }

  void discountPercentTest()
  {
    prepareAZdata();
    FareCompInfo* fci = _trx->exchangeItin().front()->fareComponent().front();
    fci->fareCalcFareAmt() = 90.0;
    fci->tktFareCalcFareAmt() = 180.0;

    static_cast<RexBaseRequest*>(_trx->getRequest())->excDiscounts().addPercentage(1, 10.0);

    RexFareSelector rfs(*_trx);
    rfs.updateFCAmount(*fci);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, fci->fareCalcFareAmt(), 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(200.0, fci->tktFareCalcFareAmt(), 0.1);
  }

  void discountAmountTest()
  {
    prepareAZdata();
    FareCompInfo* fci = _trx->exchangeItin().front()->fareComponent().front();
    _trx->exchangeItin().front()->calcCurrencyOverride() = USD;
    _trx->exchangeItin().front()->calculationCurrency() = USD;
    fci->fareCalcFareAmt() = 90.0;
    fci->tktFareCalcFareAmt() = 190.0;

    static_cast<RexBaseRequest*>(_trx->getRequest())->excDiscounts().addAmount(0, 1, 10, "EUR");

    RexFareSelector rfs(*_trx);
    rfs.updateFCAmount(*fci);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(100.0, fci->fareCalcFareAmt(), 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(200.0, fci->tktFareCalcFareAmt(), 0.1);
  }

  void prepareAZdata()
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->governingCarrier() = "AA";
    TravelSeg* seg = createAirSeg("MOW", "FRA", "AA");
    seg->pnrSegment() = 1;
    seg->segmentOrder() = 1;
    fm->travelSeg().push_back(seg);

    ExcItin* exchangeItin = _memHandle.create<ExcItin>();
    exchangeItin->fareMarket().push_back(fm);

    FareCompInfo* fci = _memHandle.create<FareCompInfo>();

    fci->hasVCTR() = false;
    fci->fareMarket() = fm;
    exchangeItin->fareComponent().push_back(fci);

    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(exchangeItin);
  }

  class MyDataHandle : public DataHandleMock
  {
  public:
    const DateTime& getVoluntaryChangesConfig(const CarrierCode& carrier,
                                              const DateTime& currentTktDate,
                                              const DateTime& originalTktIssueDate)
    {
      if (carrier == "")
        return DateTime::emptyDate();
      return DataHandleMock::getVoluntaryChangesConfig(
          carrier, currentTktDate, originalTktIssueDate);
    }

    const std::vector<BankerSellRate*>&
    getBankerSellRate(const CurrencyCode&, const CurrencyCode&, const DateTime&)
    {
      return _bsrVect;
    }

    const Nation*
    getNation(const NationCode& nationCode, const DateTime&)
    {
      return _nation;
    }

    MyDataHandle()
    {
      BankerSellRate* bsr = _memHandle.create<BankerSellRate>();
      bsr->primeCur() = "EUR";
      bsr->cur() = "USD";
      bsr->rate() = 1.0;
      _bsrVect.push_back(bsr);

      _nation = _memHandle.create<Nation>();
      _nation->nation() = "GER";
      _nation->primeCur() = "EUR";
      _nation->alternateCur() = "EUR";
      _nation->conversionCur() = "EUR";
    }

  private:
    TestMemHandle _memHandle;
    std::vector<BankerSellRate*> _bsrVect;
    Nation* _nation;
  };

private:
  RexPricingTrx* _trx;
  DateTime _localTime;
  DateTime _localTimeOneDayBefore;
  DateTime _localTimeTwoDaysBefore;
  DateTime _localTimeOneDayAfter;
  DateTime _localTimeTwoDaysAfter;
  PaxType _ptADT;
  PaxType _ptINF;
  CurrencyCode _currencyJPY;
  CurrencyCode _currencyUSD;
  bool _YYFare;
  bool _notYYFare;
  bool _HIP;
  bool _noHIP;
  Directionality _directionalityFROM;
  Directionality _directionalityTO;
  uint16_t _zeroMileageSurcharge;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RexFareSelectorTest);

} // end of namespace tse
