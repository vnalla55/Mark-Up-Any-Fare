//-------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include <gtest/gtest.h>

#include "Common/Global.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/Fare.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/FareRetailerResultingFareAttrInfo.h"
#include "DBAccess/FareRetailerRuleInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/Record2Types.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag335Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "Fares/NegFareCalc.h"
#include "Fares/NegFareSecurity.h"
#include "Fares/NegotiatedFareController.h"
#include "Rules/RuleApplicationBase.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
/*
struct FareRetailerRuleContext
{
  PseudoCityCode _sourcePcc;
  const FareRetailerRuleInfo* _frri;
  const std::vector<FareRetailerCalcInfo*>& _frciV;
  const FareRetailerResultingFareAttrInfo* _frrfai;
  const FareFocusSecurityInfo* _ffsi;
  const NegFareSecurityInfo* _t983;

  FareRetailerRuleContext(PseudoCityCode sourcePcc,
                          const FareRetailerRuleInfo* frri,
                          const std::vector<FareRetailerCalcInfo*>& frciV,
                          const FareRetailerResultingFareAttrInfo* frrfai,
                          const FareFocusSecurityInfo* ffsi)
    : _sourcePcc(sourcePcc),
      _frri(frri),
      _frciV(frciV),
      _frrfai(frrfai),
      _ffsi(ffsi),
      _t983(0) {}
};
*/
namespace // it prevents usage these mocks in other files
{
class MockNegotiatedFareController : public NegotiatedFareController
{
public:
  MarkupControl t_markupControl_1, t_markupControl_2, t_markupControl_3, t_markupControl_4;
  bool t_overrideCheckMultiCorpIdMatrix;
  int t_overrideInvokeCreateFareAndKeepMinAmt;
  int t_overrideCreateFareAndResetMinAmt;
  bool t_overrideValidate;
  int t_overrideProcessRule;
  bool t_overrideProcessRedistributions;
  bool t_createFaresFromCalcObj;
  std::vector<bool> t_overrideIsDuplicatedFare;

  std::vector<MarkupControl*> t_getMarkupBySecurityItemNo;
  std::vector<MarkupControl*> t_getMarkupBySecondSellerId;
  std::vector<NegFareCalcInfo*> t_getNegFareCalc;
  GeneralFareRuleInfoVec t_getGeneralFareRuleInfoElement;
  std::vector<NegFareSecurityInfo*> t_getNegFareSecurity;

  MockNegotiatedFareController(PricingTrx& trx, Itin& itin, FareMarket& market)
    : NegotiatedFareController(trx, itin, market),
      t_overrideCheckMultiCorpIdMatrix(false),
      t_overrideInvokeCreateFareAndKeepMinAmt(0),
      t_overrideCreateFareAndResetMinAmt(0),
      t_overrideValidate(false),
      t_overrideProcessRule(0),
      t_overrideProcessRedistributions(false),
      t_createFaresFromCalcObj(false)
  {
    MarkupCalculate* markupCalc;

    insertNegFareSecurityInfo('Y', 0, 100);

    t_markupControl_1.secondarySellerId() = 0;
    t_markupControl_1.sellTag() = YES;

    markupCalc = new MarkupCalculate();
    markupCalc->loc1().locType() = RuleConst::ANY_LOCATION_TYPE;
    markupCalc->loc2().locType() = RuleConst::ANY_LOCATION_TYPE;
    markupCalc->seasonType() = ' ';
    markupCalc->dowType() = ' ';
    markupCalc->sellingFareInd() = 'S';
    markupCalc->sellingPercent() = 100, markupCalc->sellingFareAmt1() = 90;
    markupCalc->sellingFareAmt2() = 90;
    markupCalc->sellingCur1() = "USD";
    markupCalc->sellingCur2() = "";
    markupCalc->sellingPercentNoDec() = 0;
    markupCalc->sellingNoDec1() = 0;

    t_markupControl_1.calcs().push_back(markupCalc);
    t_getMarkupBySecurityItemNo.push_back(&t_markupControl_1);

    t_markupControl_2.secondarySellerId() = 0;
    t_markupControl_2.sellTag() = YES;

    markupCalc = new MarkupCalculate();
    markupCalc->loc1().locType() = RuleConst::ANY_LOCATION_TYPE;
    markupCalc->loc2().locType() = RuleConst::ANY_LOCATION_TYPE;
    markupCalc->seasonType() = ' ';
    markupCalc->dowType() = ' ';
    markupCalc->sellingFareInd() = 'S';
    markupCalc->sellingPercent() = 100, markupCalc->sellingFareAmt1() = 50;
    markupCalc->sellingFareAmt2() = 50;
    markupCalc->sellingCur1() = "USD";
    markupCalc->sellingCur2() = "";
    markupCalc->sellingPercentNoDec() = 0;
    markupCalc->sellingNoDec1() = 0;

    t_markupControl_2.calcs().push_back(markupCalc);
    t_getMarkupBySecurityItemNo.push_back(&t_markupControl_2);

    t_markupControl_3.secondarySellerId() = 0;
    t_markupControl_3.sellTag() = YES;
    t_markupControl_3.accountCode() = "A123";

    markupCalc = new MarkupCalculate();
    markupCalc->loc1().locType() = RuleConst::ANY_LOCATION_TYPE;
    markupCalc->loc2().locType() = RuleConst::ANY_LOCATION_TYPE;
    markupCalc->seasonType() = ' ';
    markupCalc->dowType() = ' ';
    markupCalc->sellingFareInd() = 'S';
    markupCalc->sellingPercent() = 100, markupCalc->sellingFareAmt1() = 30;
    markupCalc->sellingFareAmt2() = 30;
    markupCalc->sellingCur1() = "USD";
    markupCalc->sellingCur2() = "";
    markupCalc->sellingPercentNoDec() = 0;
    markupCalc->sellingNoDec1() = 0;

    t_markupControl_3.calcs().push_back(markupCalc);
    t_getMarkupBySecurityItemNo.push_back(&t_markupControl_3);

    t_markupControl_4.secondarySellerId() = 0;
    t_markupControl_4.sellTag() = YES;
    t_markupControl_4.accountCode() = "B123";

    markupCalc = new MarkupCalculate();
    markupCalc->loc1().locType() = RuleConst::ANY_LOCATION_TYPE;
    markupCalc->loc2().locType() = RuleConst::ANY_LOCATION_TYPE;
    markupCalc->seasonType() = ' ';
    markupCalc->dowType() = ' ';
    markupCalc->sellingFareInd() = 'S';
    markupCalc->sellingPercent() = 100, markupCalc->sellingFareAmt1() = 130;
    markupCalc->sellingFareAmt2() = 130;
    markupCalc->sellingCur1() = "USD";
    markupCalc->sellingCur2() = "";
    markupCalc->sellingPercentNoDec() = 0;
    markupCalc->sellingNoDec1() = 0;

    t_markupControl_4.calcs().push_back(markupCalc);
    t_getMarkupBySecurityItemNo.push_back(&t_markupControl_4);
  }

  virtual ~MockNegotiatedFareController() { clearNegFareSecurityInfo(); }

  void insertNegFareSecurityInfo(Indicator redistributeInd, long secondarySellerId, int seqNo)
  {
    NegFareSecurityInfo* newInfo = new NegFareSecurityInfo();

    newInfo->redistributeInd() = redistributeInd;
    newInfo->secondarySellerId() = secondarySellerId;
    newInfo->seqNo() = seqNo;
    newInfo->loc1().locType() = LOCTYPE_NONE;
    newInfo->loc2().locType() = LOCTYPE_NONE;

    t_getNegFareSecurity.push_back(newInfo);
  }

  void clearNegFareSecurityInfo()
  {
    std::vector<NegFareSecurityInfo*>::iterator i = t_getNegFareSecurity.begin();
    std::vector<NegFareSecurityInfo*>::iterator ie = t_getNegFareSecurity.end();
    for (; i != ie; ++i)
    {
      delete *i;
      *i = NULL;
    }

    t_getNegFareSecurity.clear();
  }

  virtual bool
  checkMultiCorpIdMatrix(const MarkupControl& markupRec, const PaxTypeFare& paxTypeFare)
  {
    if (t_overrideCheckMultiCorpIdMatrix)
      return true;
    return NegotiatedFareController::checkMultiCorpIdMatrix(markupRec, paxTypeFare);
  }

  virtual void invokeCreateFare(PaxTypeFare& ptFare,
                                MoneyAmount fareAmount,
                                MoneyAmount nucFareAmount,
                                NegFareCalc& calcObj,
                                NegFareSecurityInfo* secRec,
                                Indicator fareDisplayCat35,
                                Indicator ticketingInd)
  {
    if (t_overrideInvokeCreateFareAndKeepMinAmt)
      t_overrideInvokeCreateFareAndKeepMinAmt += int(fareAmount);
    else
      NegotiatedFareController::invokeCreateFare(
          ptFare, fareAmount, nucFareAmount, calcObj, secRec, fareDisplayCat35, ticketingInd);
  }

  virtual void
  resetMinAmt(MoneyAmount& saveAmt, MoneyAmount& saveAmtNuc, NegPaxTypeFareRuleData*& ruleData)
  {
    if (!t_overrideCreateFareAndResetMinAmt)
      NegotiatedFareController::resetMinAmt(saveAmt, saveAmtNuc, ruleData);
  }

  virtual bool createFare(PaxTypeFare& ptFare,
                          NegPaxTypeFareRuleData* ruleData,
                          bool isLocationSwapped,
                          MoneyAmount fareAmount,
                          MoneyAmount fareAmountNuc,
                          bool pricingOption,
                          const Indicator* fareDisplayCat35)
  {
    if (t_overrideCreateFareAndResetMinAmt)
      t_overrideCreateFareAndResetMinAmt++;
    else
      return NegotiatedFareController::createFare(ptFare,
                                                  ruleData,
                                                  isLocationSwapped,
                                                  fareAmount,
                                                  fareAmountNuc,
                                                  pricingOption,
                                                  fareDisplayCat35);
    return false;
  }

  virtual Record3ReturnTypes validate(PaxTypeFare& ptFare)
  {
    if (t_overrideValidate)
      return PASS;
    return NegotiatedFareController::validate(ptFare);
  }

  virtual void processRule(PaxTypeFare& ptFare, bool isLocationSwapped)
  {
    if (t_overrideProcessRule)
    {
      t_overrideProcessRule++;
      if (isLocationSwapped)
        throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, "");
    }
    else
      NegotiatedFareController::processRule(ptFare, isLocationSwapped);
  }

  virtual void processRedistributions(PaxTypeFare& ptFare,
                                      NegPaxTypeFareRuleData* ruleData,
                                      MoneyAmount& amt,
                                      MoneyAmount& amtNuc,
                                      Money& base,
                                      NegFareCalc& calcObj,
                                      bool is979queried,
                                      int32_t seqNegMatch)
  {
    if (!t_overrideProcessRedistributions)
      NegotiatedFareController::processRedistributions(
          ptFare, ruleData, amt, amtNuc, base, calcObj, is979queried, seqNegMatch);
  }

  virtual bool createFaresFromCalcObj(PaxTypeFare& ptFare,
                                      NegFareCalc& calcObj,
                                      NegFareSecurityInfo* secRec,
                                      NegPaxTypeFareRuleData* ruleData,
                                      Money& base,
                                      MoneyAmount& amt,
                                      MoneyAmount& amtNuc,
                                      Indicator ticketingInd)
  {
    if (t_createFaresFromCalcObj)
      return amt > 0;
    return NegotiatedFareController::createFaresFromCalcObj(
        ptFare, calcObj, secRec, ruleData, base, amt, amtNuc, ticketingInd);
  }

  // overriding access to database
  const std::vector<MarkupControl*>& getMarkupBySecurityItemNo(const PaxTypeFare& ptFare) const
  {
    return t_getMarkupBySecurityItemNo;
  }

  const std::vector<MarkupControl*>&
  getMarkupBySecondSellerId(const PaxTypeFare& ptFare, uint32_t rec2SeqNum, long secondarySellerId)
      const
  {
    return t_getMarkupBySecondSellerId;
  }

  const std::vector<NegFareCalcInfo*>& getNegFareCalc() const { return t_getNegFareCalc; }

  void
  getGeneralFareRuleInfo(const PaxTypeFare& paxTypeFare, GeneralFareRuleInfoVec& gfrInfoVec) const
  {
    gfrInfoVec = t_getGeneralFareRuleInfoElement;
  }

  const std::vector<NegFareSecurityInfo*>& getNegFareSecurity() const
  {
    return t_getNegFareSecurity;
  }
};

class MockFareMarket : public FareMarket
{
public:
  MockFareMarket(PricingTrx& trx,
                 const LocCode& loc1,
                 const NationCode& nation1,
                 const IATAAreaCode& area1,
                 const LocCode& loc2,
                 const NationCode& nation2,
                 const IATAAreaCode& area2)
  {
    _loc1.loc() = loc1;
    _loc1.nation() = nation1;
    _loc1.area() = area1;
    _loc2.loc() = loc2;
    _loc2.nation() = nation2;
    _loc2.area() = area2;

    _airSeg.origin() = &_loc1;
    _airSeg.destination() = &_loc2;
    _airSeg.departureDT() = _departDT;

    origin() = &_loc1;
    destination() = &_loc2;
    travelSeg().push_back(&_airSeg);

    trx.travelSeg().push_back(&_airSeg);
  }

protected:
  Loc _loc1;
  Loc _loc2;
  AirSeg _airSeg;
  DateTime _departDT;
};

class MockPaxTypeFare : public PaxTypeFare
{
public:
  MockPaxTypeFare(FareMarket& fareMarket,
                  const PaxTypeCode& paxTypeCode,
                  const Indicator& displayCatType = ' ',
                  const TariffCategory& ruleTariffCat = 0,
                  const TariffNumber& ruleTariff = 0,
                  const CurrencyCode& currency = "",
                  const MoneyAmount& originalFareAmount = 0,
                  const CarrierCode& carrier = "CO",
                  PricingTrx* trx = 0)
  {
    _paxType.paxType() = paxTypeCode;

    _fareClassAppSegInfo._paxType = paxTypeCode;

    _fareClassAppInfo._displayCatType = displayCatType;
    _fareClassAppInfo._fareType = "";
    _fareClassAppInfo._seasonType = 0;
    _fareClassAppInfo._dowType = 0;

    _fareInfo._owrt = 'O';
    _fareInfo._market1 = fareMarket.origin()->loc();
    _fareInfo._market2 = fareMarket.destination()->loc();
    _fareInfo._fareClass = "NU310Q";
    _fareInfo._carrier = carrier;
    _fareInfo._ruleNumber = "AAAA";
    _fareInfo._vendor = "ATP";
    _fareInfo._currency = currency;
    _fareInfo._originalFareAmount = originalFareAmount;
    _fareInfo._fareAmount = originalFareAmount;
    _fareInfo._directionality = TO;
    _fareInfo.effDate() = DateTime(2011, 10, 10);
    _fareInfo.discDate() = DateTime(2011, 11, 10);

    _tariffCrossRefInfo._ruleTariff = ruleTariff;
    _tariffCrossRefInfo._tariffCat = ruleTariffCat;

    _fare.initialize(Fare::FS_ForeignDomestic, &_fareInfo, fareMarket, &_tariffCrossRefInfo);
    _fare._nucFareAmount = originalFareAmount;

    setFare(&_fare);
    paxType() = &_paxType;
    fareClassAppInfo() = &_fareClassAppInfo;
    fareClassAppSegInfo() = &_fareClassAppSegInfo;
    actualPaxType() = &_paxType;
    fareDisplayInfo() = &_fareDisplayInfo;
    _fareMarket = &fareMarket;

    // Add pax type fare to fare market
    fareMarket.allPaxTypeFare().push_back(this);
  }

protected:
  Fare _fare;
  FareInfo _fareInfo;
  FareDisplayInfo _fareDisplayInfo;
  TariffCrossRefInfo _tariffCrossRefInfo;
  PaxType _paxType;
  FareClassAppInfo _fareClassAppInfo;
  FareClassAppSegInfo _fareClassAppSegInfo;
};

class MockNegFareRest : public NegFareRest
{
public:
  MockNegFareRest(const PaxTypeCode& psgType,
                  uint32_t overrideDateTblItemNo = 0,
                  Indicator unavailTag = ' ',
                  const Indicator& netRemitMethod = RuleConst::NRR_METHOD_BLANK)
  {
    _psgType = psgType;
    _overrideDateTblItemNo = overrideDateTblItemNo;
    _unavailTag = unavailTag;
    _netRemitMethod = netRemitMethod;
    _negFareCalcTblItemNo = overrideDateTblItemNo;

    _tktFareDataInd1 = ' ';
    _owrt1 = ' ';
    _seasonType1 = ' ';
    _dowType1 = ' ';
    _globalDir1 = GlobalDirection::ZZ;
    _ruleTariff1 = 0;

    _tktFareDataInd2 = ' ';
    _owrt2 = ' ';
    _seasonType2 = ' ';
    _dowType2 = ' ';
    _globalDir2 = GlobalDirection::ZZ;
    _ruleTariff2 = 0;
  }

  MockNegFareRest(const LocCode& betwCity1,
                  const LocCode& andCity1,
                  const LocCode& betwCity2,
                  const LocCode& andCity2,
                  Indicator fareDataInd = 'N')
  {
    _betwCity1 = betwCity1;
    _andCity1 = andCity1;
    _betwCity2 = betwCity2;
    _andCity2 = andCity2;

    _tktFareDataInd1 = fareDataInd;
  }

  MockNegFareRest(const PaxTypeCode& psgType, const CarrierCode& carrier)
  {
    _psgType = psgType;
    _carrier = carrier;

    _tktAppl = ' ';
    _overrideDateTblItemNo = 0;
    _unavailTag = ' ';
    _netRemitMethod = RuleConst::NRR_METHOD_BLANK;

    _tktFareDataInd1 = ' ';
    _owrt1 = ' ';
    _seasonType1 = ' ';
    _dowType1 = ' ';
    _globalDir1 = GlobalDirection::ZZ;
    _ruleTariff1 = 0;

    _tktFareDataInd2 = ' ';
    _owrt2 = ' ';
    _seasonType2 = ' ';
    _dowType2 = ' ';
    _globalDir2 = GlobalDirection::ZZ;
    _ruleTariff2 = 0;
  }
};

class MockDiag335Collector : public Diag335Collector
{
public:
  MockDiag335Collector() : Diag335Collector() { enable(Diagnostic335); }

  virtual ~MockDiag335Collector() {}

  virtual void doNetRemitTkt(const NegFareRest&, bool, bool)
  {
    *this << "doNetRemitTkt\n";
  }

  virtual void displayFailCode(const NegFareRest&, const char*)
  {
    *this << "displayFailCode\n";
  }
};


CategoryRuleItemInfo dummyCRII(
    uint32_t relationalInd = CategoryRuleItemInfo::IF,
    Indicator directionality = NegotiatedFareController::BLANK,
    Indicator inOutInd = NegotiatedFareController::BLANK)
{
  CategoryRuleItemInfo crii;
  crii.setItemcat(35);
  crii.setOrderNo(0);
  crii.setRelationalInd(static_cast<CategoryRuleItemInfo::LogicalOperators>(relationalInd));
  crii.setInOutInd(inOutInd);
  crii.setDirectionality(directionality);
  crii.setItemNo(165502);
  return crii;
}

/*
class MockCategoryRuleItemInfo : public CategoryRuleItemInfo
{
public:
  MockCategoryRuleItemInfo(uint32_t relationalInd = CategoryRuleItemInfo::IF,
                           Indicator directionality = NegotiatedFareController::BLANK,
                           Indicator inOutInd = NegotiatedFareController::BLANK)
  {
    this->setItemcat(35);
    this->setOrderNo(0);
    this->setRelationalInd(static_cast<CategoryRuleItemInfo::LogicalOperators>(relationalInd));
    this->setInOutInd(inOutInd);
    this->setDirectionality(directionality);
    this->setItemNo(165502);
  }
};
*/
/*
class MockCategoryRuleItemInfoVec : public std::vector<CategoryRuleItemInfo>
{
public:
  void addNewItem(uint32_t relationalInd)
  {
    CategoryRuleItemInfo* itemInfo = new MockCategoryRuleItemInfo(relationalInd);
    push_back(itemInfo);
  }
};
*/

class MockNegFareCalcInfo : public NegFareCalcInfo
{
public:
  MockNegFareCalcInfo()
  {
    fareInd() = RuleConst::NF_ADD;
    loc1().locType() = RuleConst::ANY_LOCATION_TYPE;
    userDefZone1() = RuleConst::NOT_APPLICABLE_ZONE;
    loc2().locType() = RuleConst::ANY_LOCATION_TYPE;
    userDefZone2() = RuleConst::NOT_APPLICABLE_ZONE;
  }
};

class MockMarkupCalculate : public MarkupCalculate
{
public:
  MockMarkupCalculate(Indicator netSellingInd)
  {
    fareClass() = "";
    fareType() = "";
    seasonType() = 0;
    dowType() = 0;
    this->netSellingInd() = netSellingInd;
  }
};

class MockMarkupControl : public MarkupControl
{
public:
  MockMarkupControl(const char* accountCode) { this->accountCode() = accountCode; }

  MockMarkupControl(Indicator status,
                    Indicator sellTag = YES,
                    Indicator tktTag = YES,
                    Indicator netSellingInd = S_TYPE)
  {
    this->status() = status;
    this->sellTag() = sellTag;
    this->tktTag() = tktTag;
    MarkupCalculate* mc = new MockMarkupCalculate(netSellingInd);
    calcs().push_back(mc);
  }
};
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const std::vector<Customer*>& getCustomer(const PseudoCityCode& key)
  {
    std::vector<Customer*>* ret = _memHandle.create<std::vector<Customer*> >();
    Customer* c = _memHandle.create<Customer>();
    ret->push_back(c);
    c->pseudoCity() = key;
    c->arcNo() = "9999999";
    c->lbtCustomerGroup() = "9999999";
    c->branchAccInd() = 'N';
    c->curConvInd() = 'N';
    c->cadSubscriberInd() = 'N';
    c->webSubscriberInd() = 'N';
    c->btsSubscriberInd() = 'N';
    c->sellingFareInd() = 'N';
    c->tvlyInternetSubriber() = 'N';
    c->availabilityIgRul2St() = 'Y';
    c->availabilityIgRul3St() = 'Y';
    c->availIgRul2StWpnc() = 'Y';
    c->activateJourneyPricing() = 'Y';
    c->activateJourneyShopping() = 'Y';
    c->doNotApplySegmentFee() = 'N';
    c->optInAgency() = 'Y';
    c->privateFareInd() = 'Y';
    c->doNotApplyObTktFees() = 'N';
    c->fareQuoteCur() = 'Y';
    if (key == "5KAD")
    {
      c->homePseudoCity() = "5KAD";
      c->homeArcNo() = "9999999";
      c->requestCity() = "SIN";
      c->aaCity() = "SIN";
      c->defaultCur() = "SGD";
      c->agencyName() = "FARES AND PRICING 1";
      c->channelId() = 'T';
      c->crsCarrier() = "1B";
      c->ssgGroupNo() = 16;
      c->eTicketCapable() = 'N';
      c->hostName() = "ABAC";
      return *ret;
    }
    else if (key == "A0NC")
    {
      c->homePseudoCity() = "FC83";
      c->homeArcNo() = "1939119";
      c->requestCity() = "TYO";
      c->aaCity() = "TYO";
      c->defaultCur() = "JPY";
      c->agencyName() = "AXESS INTERNATION";
      c->channelId() = 'N';
      c->crsCarrier() = "1J";
      c->ssgGroupNo() = 19;
      c->eTicketCapable() = 'Y';
      c->hostName() = "AXES";
      return *ret;
    }
    return DataHandleMock::getCustomer(key);
  }
  const std::vector<tse::CorpId*>&
  getCorpId(const std::string& corpId, const CarrierCode& carrier, const DateTime& tvlDate)
  {
    if (corpId == "B")
      return *_memHandle.create<std::vector<tse::CorpId*> >();
    return DataHandleMock::getCorpId(corpId, carrier, tvlDate);
  }
  const std::vector<DateOverrideRuleItem*>&
  getDateOverrideRuleItem(const VendorCode& vendor,
                          int itemNumber,
                          const DateTime& applDate = DateTime::emptyDate())
  {
    if (itemNumber == 1000 || itemNumber == -1)
      return *_memHandle.create<std::vector<DateOverrideRuleItem*> >();
    return DataHandleMock::getDateOverrideRuleItem(vendor, itemNumber, applDate);
  }
  const NegFareRest* getNegFareRest(const VendorCode& vendor, int itemNo)
  {
    if (itemNo == 165502)
      return 0;
    return DataHandleMock::getNegFareRest(vendor, itemNo);
  }
  const std::vector<const PaxTypeMatrix*>& getPaxTypeMatrix(const PaxTypeCode& key)
  {
    if (key == "CNN" || key == "NEG" || key == "JCB" || key == "PFA" || key == "MIL")
      return *_memHandle.create<std::vector<const PaxTypeMatrix*> >();
    return DataHandleMock::getPaxTypeMatrix(key);
  }
  const bool isHistorical() { return false; }
  char getVendorType(const VendorCode& vendor)
  {
    if (vendor == "AAAA")
      return ' ';
    return DataHandleMock::getVendorType(vendor);
  }
};

} // end of namespace

class NegotiatedFareControllerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NegotiatedFareControllerTest);

  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testProcessRule);
  CPPUNIT_TEST(testProcessMultipleRules);

  CPPUNIT_TEST(testValidate_PaxType_Same);
  CPPUNIT_TEST(testValidate_PaxType_Different);
  CPPUNIT_TEST(testValidate_PaxType_FareAdult_Rec3Adult);
  CPPUNIT_TEST(testValidate_PaxType_FareBlank_Rec3Other);
  CPPUNIT_TEST(testValidate_PaxType_FareBlank_Rec3Other_OtherPTCRequested);
  CPPUNIT_TEST(testValidate_PaxType_FareNegGroup_Rec3NegGroup);
  CPPUNIT_TEST(testValidate_PaxType_FareNegGroup_Rec3NegGroup_OtherPTCRequested);
  CPPUNIT_TEST(testValidate_PaxType_FareNegGroup_Rec3Other);
  CPPUNIT_TEST(testValidate_PaxType_FareJcbGroup_Rec3JcbGroup);
  CPPUNIT_TEST(testValidate_PaxType_FareJcbGroup_Rec3JcbGroup_OtherPTCRequested);
  CPPUNIT_TEST(testValidate_PaxType_FareJcbGroup_Rec3Other);
  CPPUNIT_TEST(testValidate_PaxType_FarePfaGroup_Rec3PfaGroup);
  CPPUNIT_TEST(testValidate_PaxType_FarePfaGroup_Rec3PfaGroup_OtherPTCRequested);
  CPPUNIT_TEST(testValidate_PaxType_FarePfaGroup_Rec3Other);
  CPPUNIT_TEST(testValidate_PaxType_FareOther_Rec3Adult);
  CPPUNIT_TEST(testValidate_PaxType_FareAdult_Rec3Other);

  CPPUNIT_TEST(testValidate_OverrideDateItem_NotSet);
  CPPUNIT_TEST(testValidate_OverrideDateItem_Set);

  CPPUNIT_TEST(testValidate_Fail_UnavailableTag_NoData);
  CPPUNIT_TEST(testValidate_Skip_UnavailableTag_TextOnly);
  CPPUNIT_TEST(testValidate_Fail_PaxType);
  CPPUNIT_TEST(testValidate_Fail_OverrideTable);
  CPPUNIT_TEST(testValidate_Fail_Carrier);
//  CPPUNIT_TEST(testValidate_Pass_MultipleCarriers);
  CPPUNIT_TEST(testValidate_Fail_MultipleCarriers);
  CPPUNIT_TEST(testValidate_Fail_Cat35TktData);
  CPPUNIT_TEST(testValidate_Pass);

  CPPUNIT_TEST(testValidateCat35Data_FD_SellingFare);
  CPPUNIT_TEST(testValidateCat35Data_FD_NetSubmitFare_NoT979);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_SellingFare_MethodBlank_NoTktData);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_SellingFare_MethodBlank_TktData);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_SellingFare);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_NetSubmitFare_ValidCommission);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_NetSubmitFare_InvalidCommission);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_NetSubmitFareUpd_ValidCommission);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_NetSubmitFareUpd_InvalidCommission);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_NetSubmitFare_MethodBlank_TktData);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_NetSubmitFare_MethodBlank_NoTktData);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_NetSubmitFareUpd_Method2_TktData);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_NetSubmitFareUpd_Method2_NoTktData);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_NetSubmitFareUpd_Method3);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_NetSubmitFare_Method3);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_NetSubmitFareUpd_Method1);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_NetSubmitFare_Method1);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_NetSubmitFareUpd_Method2_TktData_FareDataIndBlank);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_NetSubmitFare_Method2_TktData_FareDataIndBlank);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_RecSegData_NoTktData);
  CPPUNIT_TEST(testValidateCat35Data_Pricing_RecSegData_TktData);
  CPPUNIT_TEST(testValidateCat35TktDataL_InfiniAgent_Method1_NoT979);
  CPPUNIT_TEST(testValidateCat35TktDataL_InfiniAgent_Method2_NoT979);
  CPPUNIT_TEST(testValidateCat35TktDataL_InfiniAgent_Method2_ExistingT979);
  CPPUNIT_TEST(testValidateCat35TktDataL_NoInfiniAgent_Method1_NoT979);
  CPPUNIT_TEST(testValidateCat35TktDataKorea_Pass_NotMethod3);
  CPPUNIT_TEST(testValidateCat35TktDataKorea_Pass_TktOrExchange);
  CPPUNIT_TEST(testValidateCat35TktDataKorea_Pass_Korea);
  CPPUNIT_TEST(testValidateCat35TktDataKorea_Fail);

  CPPUNIT_TEST(testValidCommissions);
  CPPUNIT_TEST(testInvalidCommissions);

  CPPUNIT_TEST(testIsSellingOrNet_SellingFare);
  CPPUNIT_TEST(testIsSellingOrNet_NetSubmitFare);
  CPPUNIT_TEST(testIsSellingOrNet_NetSubmitFareUpd);
  CPPUNIT_TEST(testIsSellingOrNet_Fail);

  CPPUNIT_TEST(testCheckFareJalAxess_Fail);
  CPPUNIT_TEST(testCheckFareJalAxess_Pass_FD);
  CPPUNIT_TEST(testCheckFareJalAxess_Pass_FareNotValid);
  CPPUNIT_TEST(testCheckFareJalAxess_Pass_FareSelling);
  CPPUNIT_TEST(testCheckFareJalAxess_Pass_NotAxess);
  CPPUNIT_TEST(testCheckFareJalAxess_Pass_NotNet);

  CPPUNIT_TEST(testCheckFare_JalAxessFail);
  CPPUNIT_TEST(testCheckFare_FareRexTrxFail);
  CPPUNIT_TEST(testCheckFare_FareTariffCarrierFail);
  CPPUNIT_TEST(testCheckFare_FareNegFail);
  CPPUNIT_TEST(testCheckFare_FareCat25WithCat35Fail);
  CPPUNIT_TEST(testCheckFare_Pass);

  CPPUNIT_TEST(testCheckFareRexTrx_Pass_FareValid);
  CPPUNIT_TEST(testCheckFareRexTrx_Pass_FareRexTrxAndNewItin);
  CPPUNIT_TEST(testCheckFareRexTrx_Fail_NeedRetrieveKeepFare);
  CPPUNIT_TEST(testCheckFareRexTrx_Fail_NewItin);

  CPPUNIT_TEST(testCheckFareTariffCarrier_Fail_NotPrivateFare);
  CPPUNIT_TEST(testCheckFareTariffCarrier_Fail_NotSellingOrNet);
  CPPUNIT_TEST(testCheckFareTariffCarrier_Fail_IndustryCarrier);
  CPPUNIT_TEST(testCheckFareTariffCarrier_Pass);

  CPPUNIT_TEST(testCheckFareNeg_FD_NET);
  CPPUNIT_TEST(testCheckFareNeg_NotFD_Selling);
  CPPUNIT_TEST(testCheckFareNeg_Pass);
  CPPUNIT_TEST(testCheckFareNeg_Fail);

  CPPUNIT_TEST(testCheckFareCat25WithCat35_Pass_Cat35NotAllowed);
  CPPUNIT_TEST(testCheckFareCat25WithCat35_Pass_IsFareByRule);
  CPPUNIT_TEST(testCheckFareCat25WithCat35_Fail);

  CPPUNIT_SKIP_TEST(testProcessRedistributionsWithAccCode);
  CPPUNIT_SKIP_TEST(testProcessRedistributionsNoAccCode);
  CPPUNIT_SKIP_TEST(testProcessRedistributionsXC);

  CPPUNIT_TEST(testMatchMUCAccCodeWithRequestedCorpId_emptyVector);
  CPPUNIT_TEST(testMatchMUCAccCodeWithRequestedCorpId_pass);
  CPPUNIT_TEST(testMatchMUCAccCodeWithRequestedCorpId_fail);

  CPPUNIT_TEST(testMatchMUCAccCodeWithRequestedAccCode_emptyVector);
  CPPUNIT_TEST(testMatchMUCAccCodeWithRequestedAccCode_pass);
  CPPUNIT_TEST(testMatchMUCAccCodeWithRequestedAccCode_fail);

  CPPUNIT_TEST(testDoDiag335Collector);
  CPPUNIT_TEST(testDoDiag335CollectorNotActive);
  CPPUNIT_TEST(testDoDiag335CollectorDisplayTkt);

  CPPUNIT_TEST(testShallKeepMinAmtFailNegativeNewAmt);
  CPPUNIT_TEST(testShallKeepMinAmtPassInvalidSaveAmt);
  CPPUNIT_TEST(testShallKeepMinAmtPassSaveAmtGreaterThanNewAmt);
  CPPUNIT_TEST(testShallKeepMinAmtPassNegGroup);
  CPPUNIT_TEST(testShallKeepMinAmtFailNegGroup);
  CPPUNIT_TEST(testShallKeepMinAmtPassJcbGroup);
  CPPUNIT_TEST(testShallKeepMinAmtFailJcbGroup);
  CPPUNIT_TEST(testShallKeepMinAmtPassPfaGroup);
  CPPUNIT_TEST(testShallKeepMinAmtFailPfaGroup);

  /* This test is not valid any more since we select first match fare and not lowest fare  *
   * This test expect false should be return since new amount is greater than saved amount.*/
  CPPUNIT_SKIP_TEST(testShallKeepMinAmtFailNoConditionMet);

  CPPUNIT_TEST(testKeepMinAmtAbacusVendorNotPublished);
  CPPUNIT_TEST(testKeepMinAmtAxessCat35);

  CPPUNIT_TEST(testGetSecIdsForRedistributionForRedistribute);
  CPPUNIT_TEST(testGetSecIdsForRedistributionSeqNegMatchZero);
  CPPUNIT_TEST(testGetSecIdsForRedistributionSeqNegMatchNotZero);

  CPPUNIT_TEST(testIfMarkupValidForRedistributionStatusDeclined);
  CPPUNIT_TEST(testIfMarkupValidForRedistributionStatusPending);
  CPPUNIT_TEST(testIfMarkupValidForRedistributionTicketingFail);
  CPPUNIT_TEST(testIfMarkupValidForRedistributionSellingFail);
  CPPUNIT_TEST(testIfMarkupValidForRedistributionTicketingPass);
  CPPUNIT_TEST(testIfMarkupValidForRedistributionSellingPass);

  CPPUNIT_TEST(testMatchMultiCorpIdAccCodeMUCPassEmptyAccountCode);
  CPPUNIT_TEST(testMatchMultiCorpIdAccCodeMUCFailEmptyCorpIDAndAccCode);
  CPPUNIT_TEST(testMatchMultiCorpIdAccCodeMUCPassCorpIDMatched);
  CPPUNIT_TEST(testMatchMultiCorpIdAccCodeMUCPassMultiCorpIDMatched);
  CPPUNIT_TEST(testMatchMultiCorpIdAccCodeMUCPassAccCodeMatched);
  CPPUNIT_TEST(testMatchMultiCorpIdAccCodeMUCFailNotMatched);
  CPPUNIT_TEST(testMatchMultiCorpIdAccCodeMUCFailForceCorpFares);

  CPPUNIT_TEST(testMatchCorpIdAccCodeMUCPassEmptyAccountCode);
  CPPUNIT_TEST(testMatchCorpIdAccCodeMUCFailEmptyCorpIDAndAccCode);
  CPPUNIT_TEST(testMatchCorpIdAccCodeMUCPassCorpIDMatched);
  CPPUNIT_TEST(testMatchCorpIdAccCodeMUCFailCorpIDNotMatched);
  CPPUNIT_TEST(testMatchCorpIdAccCodeMUCPassAccCodeMatched);
  CPPUNIT_TEST(testMatchCorpIdAccCodeMUCFailAccCodeNotMatched);
  CPPUNIT_TEST(testMatchCorpIdAccCodeMUCFailForceCorpFares);

  CPPUNIT_TEST(testPrepareNewSellingAmtFD);
  CPPUNIT_TEST(testPrepareNewSellingAmtNotFD);
  CPPUNIT_TEST(testPrepareNewSellingAmtNotSelling);
  CPPUNIT_TEST(testPrepareNewSellingAmtNotNegative);

//  CPPUNIT_TEST(testCreateFaresForFDSellingFare);
  CPPUNIT_TEST(testCreateFaresForFDNotSellingFare);
  CPPUNIT_TEST(testCreateFaresForFDWholesaler);
  CPPUNIT_TEST(testCreateFaresForFDFailFare);

  CPPUNIT_TEST(testProcessNegFareRest_Fail_NullPointer);
  CPPUNIT_TEST(testProcessNegFareRest_Fail_NotValidated);
  CPPUNIT_TEST(testProcessNegFareRest_Fail_T979NotExistsNet);
  CPPUNIT_TEST(testProcessNegFareRest_Fail_T979NotExistsSelling);
  CPPUNIT_TEST(testProcessNegFareRest_Pass);

  CPPUNIT_TEST(testProcessCategoryRuleItemInfo_Fail_NullPointer);
  CPPUNIT_TEST(testProcessCategoryRuleItemInfo_Fail_ReationalIndIf);
  CPPUNIT_TEST(testProcessCategoryRuleItemInfo_Fail_ReationalIndAnd);
  CPPUNIT_TEST(testProcessCategoryRuleItemInfo_Fail_DirectionCode);
  CPPUNIT_TEST(testProcessCategoryRuleItemInfo_Fail_FdDirectionCode);
  CPPUNIT_TEST(testProcessCategoryRuleItemInfo_Pass);

  CPPUNIT_SKIP_TEST(testCheckDirectionalityAndPaxType_Pass_DirFrom1To2);
  CPPUNIT_SKIP_TEST(testCheckDirectionalityAndPaxType_Pass_DirFrom2To1);
  CPPUNIT_TEST(testCheckDirectionalityAndPaxType_Pass_NotAlwayAppl);
  CPPUNIT_TEST(testCheckDirectionalityAndPaxType_Pass_PaxEmpty);
  CPPUNIT_TEST(testCheckDirectionalityAndPaxType_Pass_PaxAdult);
  CPPUNIT_TEST(testCheckDirectionalityAndPaxType_Fail);

  CPPUNIT_TEST(testCheckGeneralFareRuleInfo_Fail_Empty);
  CPPUNIT_TEST(testCheckGeneralFareRuleInfo_Fail_EmptyFD);
  CPPUNIT_TEST(testCheckGeneralFareRuleInfo_Pass_Empty);

  CPPUNIT_TEST(testCheckNotNetFareForAxess_Pass_NotAxess);
  CPPUNIT_TEST(testCheckNotNetFareForAxess_Pass_AxessNet);
  CPPUNIT_TEST(testCheckNotNetFareForAxess_Pass_FD);
  CPPUNIT_TEST(testCheckNotNetFareForAxess_Pass_NotNet);
  CPPUNIT_TEST(testCheckNotNetFareForAxess_Fail);

  CPPUNIT_TEST(testProcessCurrentRule_PrevAgent);
  CPPUNIT_TEST(testProcessCurrentRule_NotPrevAgent);
  CPPUNIT_TEST(testProcessCurrentRule_Exception);

  CPPUNIT_TEST(testProcessFare_FailOnCheckNotNetFareForAxess);
  CPPUNIT_TEST(testProcessFare_FailOnCheckGeneralFareRuleInfo);
  CPPUNIT_TEST(testProcessFare_FailOnApply);
  CPPUNIT_TEST(testProcessFare_FailOnDates);

  CPPUNIT_TEST(testSetNationFrance_Fail_LocType);
  CPPUNIT_TEST(testSetNationFrance_Fail_Loc);
  CPPUNIT_TEST(testSetNationFrance_Pass_Loc1);
  CPPUNIT_TEST(testSetNationFrance_Pass_Loc2);

  CPPUNIT_TEST(testcheckTktAuthorityAndUpdFlag_Pass);
  CPPUNIT_TEST(testcheckTktAuthorityAndUpdFlag_Fail_TktAuthiority);
  CPPUNIT_TEST(testcheckTktAuthorityAndUpdFlag_Fail_UpdFlag);

  CPPUNIT_TEST(testcheckUpdFlag_Fail_Net);
  CPPUNIT_TEST(testcheckUpdFlag_Pass_NetUpd);
  CPPUNIT_TEST(testcheckUpdFlag_Pass_Net);

  CPPUNIT_TEST(testIsJalAxessTypeC_Fail_FD);
  CPPUNIT_TEST(testIsJalAxessTypeC_Fail_Axess);
  CPPUNIT_TEST(testIsJalAxessTypeC_Fail_NetUpd);
  CPPUNIT_TEST(testIsJalAxessTypeC_Pass);

  CPPUNIT_TEST(testProcessCalcObj_Pass);
  CPPUNIT_TEST(testProcessCalcObj_Fail);
  CPPUNIT_TEST(testProcessCalcObj_TktAuthotity);
  CPPUNIT_TEST(testProcessCalcObj_JalAxessTypeC);
  CPPUNIT_TEST(testProcessCalcObj_TblItemInfoZero);
  CPPUNIT_TEST(testProcessCalcObj_TblItemInfoNotZero);
  CPPUNIT_TEST(testProcessCalcObj_CreateFaresFromCalcObj);

  CPPUNIT_TEST(testCreateFaresFromCalcObj_Pass_GetOneCalc);
  CPPUNIT_TEST(testCreateFaresFromCalcObj_Fail_FD);
  CPPUNIT_TEST(testCreateFaresFromCalcObj_Fail_UpdateInd);
  CPPUNIT_TEST(testCreateFaresFromCalcObj_Fail_Net);
  CPPUNIT_TEST(testCreateFaresFromCalcObj_Pass_DoNotIgnoreCType);
  CPPUNIT_TEST(testCreateFaresFromCalcObj_Selling);

  CPPUNIT_TEST(testProcessSecurityMatch_Fail_SecList);
  CPPUNIT_TEST(testProcessSecurityMatch_Pass_SecRec);
  CPPUNIT_TEST(testProcessSecurityMatch_Pass_IsMatch);
  CPPUNIT_TEST(testProcessSecurityMatch_Fail_ProcessCalcObj);
  CPPUNIT_TEST(testProcessSecurityMatch_Pass_ProcessCalcObj);

  CPPUNIT_TEST(testGet979_Pass_ItemCalc);
  CPPUNIT_TEST(testGet979_Fail_CalcRec);
  CPPUNIT_TEST(testGet979_Fail_Net);
  CPPUNIT_TEST(testGet979_Fail_IsValidCat);
  CPPUNIT_TEST(testGet979_Pass_HaveCalc);

  CPPUNIT_TEST(testCheckBetwAndCities_Fail_BetwCity1);
  CPPUNIT_TEST(testCheckBetwAndCities_Fail_BetwCity2);
  CPPUNIT_TEST(testCheckBetwAndCities_Fail_AndCity1);
  CPPUNIT_TEST(testCheckBetwAndCities_Fail_AndCity2);
  CPPUNIT_TEST(testCheckBetwAndCities_Pass);

  CPPUNIT_TEST(testValidateCat35SegData_Pass_NotReccuring);
  CPPUNIT_TEST(testValidateCat35SegData_Fail_TicketedFareData);
  CPPUNIT_TEST(testValidateCat35SegData_Fail_BetwAndCities);
  CPPUNIT_TEST(testValidateCat35SegData_Fail_FareDataInd);
  CPPUNIT_TEST(testValidateCat35SegData_Fail_Segments);
  CPPUNIT_TEST(testValidateCat35SegData_Pass_Reccuring);

  CPPUNIT_TEST(testCheckIfNeedNewFareAmt_Pass_Selling);
  CPPUNIT_TEST(testCheckIfNeedNewFareAmt_Fail_NotSelling);
  CPPUNIT_TEST(testCheckIfNeedNewFareAmt_Fail_JalAxessTypeC);

  CPPUNIT_TEST(testPrintMatchedFare_Selling_CreatorPCC);
  CPPUNIT_TEST(testPrintMatchedFare_Selling_CwtUser);

  CPPUNIT_TEST(testCreateFareInfoAndSetFlags_RexCreate);
  CPPUNIT_TEST(testCreateFareInfoAndSetFlags_Cat35Type);
  CPPUNIT_TEST(testCreateFareInfoAndSetFlags_NationFrance);
  CPPUNIT_TEST(testCreateFareInfoAndSetFlags_AccountCode);

  CPPUNIT_TEST(testSetAmounts_Selling);
  CPPUNIT_TEST(testSetAmounts_NotSelling);
  CPPUNIT_TEST(testSetAmounts_NoNeedNewAmt);
  CPPUNIT_TEST(testSetAmounts_NeedNewAmt);
  CPPUNIT_TEST(testSetAmounts_NeedNewAmtFD);

  CPPUNIT_TEST(testCreateFare_PricingOption);
  CPPUNIT_TEST(testCreateFare_NegativeAmount);
  CPPUNIT_TEST(testCreateFare_Processed);
  CPPUNIT_TEST(testCreateFare_FDRemoveDuplicate);
  CPPUNIT_TEST(testCreateFare_FDNoDuplicate);
  CPPUNIT_TEST(testCreateFare_SoftPass);
  CPPUNIT_TEST(testCreateFare_InitFDInfo);

  CPPUNIT_TEST(testCheckMarkupControl_Pass);
  CPPUNIT_TEST(testCheckMarkupControl_Fail_Pending);
  CPPUNIT_TEST(testCheckMarkupControl_Fail_Ticket);
  CPPUNIT_TEST(testCheckMarkupControl_Fail_Selling);

  CPPUNIT_TEST(testMatchedCorpIDInPricing_Fail);
  CPPUNIT_TEST(testMatchedCorpIDInPricing_Pass_Fd);
  CPPUNIT_TEST(testMatchedCorpIDInPricing_Pass_ForceCorpFares);
  CPPUNIT_TEST(testMatchedCorpIDInPricing_Pass_MatchCorpId);

  CPPUNIT_TEST(testGetOneCalc_Fail_Get979);
  CPPUNIT_TEST(testGetOneCalc_Pass_IsUpd);
  CPPUNIT_TEST(testGetOneCalc_Fail_CheckMarupControl);
  CPPUNIT_TEST(testGetOneCalc_Fail_NetSellingInd);
  CPPUNIT_SKIP_TEST(testGetOneCalc_Pass_NetSellingInd);

  CPPUNIT_TEST(testIsSoftPass_Pass_VectorNull);
  CPPUNIT_TEST(testIsSoftPass_Fail_DirectionalityBlank);
  CPPUNIT_TEST(testIsSoftPass_Fail_DirectionalityLoc1ToLoc2);
  CPPUNIT_TEST(testIsSoftPass_Fail_DirectionalityLoc2ToLoc1);
  CPPUNIT_TEST(testIsSoftPass_Pass_InOutInd);

  CPPUNIT_TEST(testInvokeCreateFare_LType);
  CPPUNIT_TEST(testInvokeCreateFare_NotLType);

  CPPUNIT_TEST(testFillSegInfo_Empty);
  CPPUNIT_TEST(testFillSegInfo_Adult);
  CPPUNIT_TEST(testFillSegInfo_NoChange);

  CPPUNIT_TEST(testGetValidCalc_Pass);
  CPPUNIT_TEST(testGetValidCalc_Fail_SecurityMatch);

  CPPUNIT_TEST(testUpdateFareMarket);

  CPPUNIT_TEST(testSelectNegFaresSameItemNumber);
  CPPUNIT_TEST(testSelectNegFaresDifferentItemNumber);
  CPPUNIT_TEST(testSelectNegFaresDifferentItemNumberMIP);
  CPPUNIT_TEST(testSelectNegFaresTwoBuckets);

  CPPUNIT_TEST(testGetFareRetailerRuleLookupInfo);
  CPPUNIT_TEST(testcreateAgentSourcePcc);
  CPPUNIT_SKIP_TEST(testprocessSourcePccSecurityMatch);
  CPPUNIT_TEST(testprocessPermissionIndicatorsFail);
  CPPUNIT_TEST(testprocessPermissionIndicatorsPass);
  CPPUNIT_TEST(test_isPtfValidForCat25Responsive);
  CPPUNIT_TEST(test_processFareRetailerRuleTypeCFbrResponsive);

  CPPUNIT_TEST_SUITE_END();

public:
  NegotiatedFareControllerTest() {}
  ~NegotiatedFareControllerTest() {}

  // methods used in tests
  void enableAbacus()
  {
    Customer* agentTJR;

    _trx->dataHandle().get(agentTJR);
    agentTJR->crsCarrier() = "1B";
    agentTJR->hostName() = "ABAC";
    agentTJR->ssgGroupNo() = Agent::CWT_GROUP_NUMBER;

    _trx->getRequest()->ticketingAgent()->agentTJR() = agentTJR;
    _trx->getRequest()->ticketingAgent()->vendorCrsCode() = "1B"; // Abacus

    TrxUtil::enableAbacus();
  }

  void enableAxess()
  {
    const std::vector<Customer*> axess = _trx->dataHandle().getCustomer("A0NC");
    Agent* agent;
    _trx->dataHandle().get(agent);
    agent->agentTJR() = axess[0];
    agent->tvlAgencyPCC() = "A0NC";
    agent->mainTvlAgencyPCC() = "A0NC";
    _trx->getRequest()->ticketingAgent() = agent;

    Customer* agentTJR;

    _trx->dataHandle().get(agentTJR);
    agentTJR->crsCarrier() = "1J";
    agentTJR->hostName() = "AXES";

    _trx->getRequest()->ticketingAgent()->agentTJR() = agentTJR;
    _trx->getRequest()->ticketingAgent()->vendorCrsCode() = "1J"; // Axess
  }

  void setUp()
  {
    _trx = NULL;
    _itin = NULL;
    _fareMarket = NULL;
    _negotiatedFareController = NULL;
    _paxTypeFare = NULL;
    _negFareCalc = NULL;
    _negFareSecurityInfo = NULL;
    _negPaxTypeFareRuleData = NULL;
    _generalFareRuleInfo = NULL;
    _negFareRest = NULL;

    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
  }

  void tearDown() { _memHandle.clear(); }

  void setupPricingTrx()
  {
    Agent* agent;
    PricingRequest* request;
    PricingOptions* options;
    FareCalcConfig* fcConfig;

    _trx = _memHandle.create<PricingTrx>();

    _trx->dataHandle().get(agent);
    _trx->dataHandle().get(request);
    _trx->dataHandle().get(options);
    _trx->dataHandle().get(fcConfig);
    _trx->diagnostic().diagnosticType() = Diagnostic335;
    _trx->diagnostic().activate();
    _trx->setRequest(request);
    _trx->setOptions(options);
    _trx->fareCalcConfig() = fcConfig;
    request->ticketingAgent() = agent;
  }

  void setupFareDisplayTrx()
  {
    Agent* agent;
    FareDisplayRequest* request;
    FareDisplayOptions* options;
    _trx = _memHandle.create<FareDisplayTrx>();

    _trx->dataHandle().get(agent);
    _trx->dataHandle().get(request);
    _trx->dataHandle().get(options);
    _trx->diagnostic().diagnosticType() = Diagnostic335;
    _trx->diagnostic().activate();
    _trx->setRequest(request);
    _trx->setOptions(options);

    request->ticketingAgent() = agent;
  }

  void setupRexPricingTrx()
  {
    Agent* agent;
    PricingRequest* request;
    PricingOptions* options;
    FareCalcConfig* fcConfig;

    _trx = _memHandle.create<RexPricingTrx>();

    _trx->dataHandle().get(agent);
    _trx->dataHandle().get(request);
    _trx->dataHandle().get(options);
    _trx->dataHandle().get(fcConfig);
    _trx->diagnostic().diagnosticType() = Diagnostic335;
    _trx->diagnostic().activate();
    _trx->setRequest(request);
    _trx->setOptions(options);
    _trx->fareCalcConfig() = fcConfig;
    request->ticketingAgent() = agent;
  }

  void addReqPaxTypeToTrx(const PaxTypeCode& paxTypeCode)
  {
    PaxType* paxType = 0;
    PaxTypeCode copy(paxTypeCode);

    _trx->dataHandle().get(paxType);
    StateCode stateCode = "";
    PaxTypeUtil::initialize(*_trx, *paxType, copy, 1, 0, stateCode, 1);
    _trx->paxType().push_back(paxType);
  }

  void setupTestingData(const PricingTrx::TrxType trxType = PricingTrx::PRICING_TRX,
                        const PaxTypeCode& paxTypeCode = "ADT",
                        const Indicator& displayCatType = ' ',
                        const TariffCategory& ruleTariffCat = 0,
                        const TariffNumber& ruleTariff = 0,
                        const CurrencyCode& currency = "",
                        const MoneyAmount& originalFareAmount = 0,
                        const CarrierCode& carrier = "CO")
  {
    setupTestingData(false,
                     false,
                     trxType,
                     paxTypeCode,
                     displayCatType,
                     ruleTariffCat,
                     ruleTariff,
                     currency,
                     originalFareAmount,
                     carrier);
  }

  void setupTestingData(bool isAxess,
                        bool isWpNettRequested,
                        const PricingTrx::TrxType trxType = PricingTrx::PRICING_TRX,
                        const PaxTypeCode& paxTypeCode = "ADT",
                        const Indicator& displayCatType = ' ',
                        const TariffCategory& ruleTariffCat = 0,
                        const TariffNumber& ruleTariff = 0,
                        const CurrencyCode& currency = "",
                        const MoneyAmount& originalFareAmount = 0,
                        const CarrierCode& carrier = "CO")
  {
    switch (trxType)
    {
    case PricingTrx::PRICING_TRX:
      setupPricingTrx();
      break;
    case PricingTrx::FAREDISPLAY_TRX:
      setupFareDisplayTrx();
      break;
    default:
      setupRexPricingTrx();
      break;
    }

    if (isAxess)
      enableAxess();

    if (isWpNettRequested)
      _trx->getRequest()->wpNettRequested() = 'Y';

    _itin = _memHandle.create<Itin>();
    _fareMarket = _memHandle.insert(new MockFareMarket(*_trx, "BOS", "US", "1", "ELP", "US", "1"));
    _negotiatedFareController =
        _memHandle.insert(new MockNegotiatedFareController(*_trx, *_itin, *_fareMarket));
    _paxTypeFare = _memHandle.insert(new MockPaxTypeFare(*_fareMarket,
                                                         paxTypeCode,
                                                         displayCatType,
                                                         ruleTariffCat,
                                                         ruleTariff,
                                                         currency,
                                                         originalFareAmount,
                                                         carrier,
                                                         _trx));

    _negFareCalc = _memHandle.create<NegFareCalc>();
    _negFareSecurityInfo = _memHandle.create<NegFareSecurityInfo>();
    _negPaxTypeFareRuleData = _memHandle.create<NegPaxTypeFareRuleData>();
    _generalFareRuleInfo = _memHandle.create<GeneralFareRuleInfo>();
    _generalFareRuleInfo->effDate() = DateTime(2011, 10, 10);
    _generalFareRuleInfo->discDate() = DateTime(2011, 11, 10);

    _negotiatedFareController->_ruleInfo = _generalFareRuleInfo;
  }

  void setupTestingData(const NegFareRest& negFareRest,
                        const PricingTrx::TrxType trxType = PricingTrx::PRICING_TRX,
                        const PaxTypeCode& paxTypeCode = "ADT",
                        const Indicator& displayCatType = ' ',
                        const TariffCategory& ruleTariffCat = 0,
                        const TariffNumber& ruleTariff = 0,
                        const CurrencyCode& currency = "",
                        const MoneyAmount& originalFareAmount = 0,
                        const CarrierCode& carrier = "CO")
  {
    setupTestingData(trxType,
                     paxTypeCode,
                     displayCatType,
                     ruleTariffCat,
                     ruleTariff,
                     currency,
                     originalFareAmount,
                     carrier);
    _negFareRest = _memHandle.insert(new NegFareRest(negFareRest));
    _negotiatedFareController->_negFareRest = _negFareRest;
  }

  // TESTS

  void testProcess()
  {
    setupTestingData();

    CPPUNIT_ASSERT(_negotiatedFareController->process());
  }

  //-----------------------------------------------------------------
  // testProcessRule()
  //-----------------------------------------------------------------
  void testProcessRule()
  {
    setupTestingData();
    MockNegotiatedFareController* nfc = (MockNegotiatedFareController*)_negotiatedFareController;

    auto* iset = new CategoryRuleItemInfoSet();
    iset->push_back(dummyCRII(CategoryRuleItemInfo::THEN));

    _generalFareRuleInfo->vendorCode() = "ATP";
    _generalFareRuleInfo->addItemInfoSetNosync(iset);

    // we don't want to use original CreateFare
    nfc->t_overrideCreateFareAndResetMinAmt = 1;
    nfc->processRule(*_paxTypeFare, false);

    // only one fare was created (1+1)
    CPPUNIT_ASSERT_EQUAL(2, nfc->t_overrideCreateFareAndResetMinAmt);
  }

  //-----------------------------------------------------------------
  // testMultipleRule()
  //-----------------------------------------------------------------
  void testProcessMultipleRules()
  {
    setupTestingData();
    MockNegotiatedFareController* nfc = (MockNegotiatedFareController*)_negotiatedFareController;

    auto* iset = new CategoryRuleItemInfoSet();
    iset->push_back(dummyCRII(CategoryRuleItemInfo::THEN));
    iset->push_back(dummyCRII(CategoryRuleItemInfo::OR));
    iset->push_back(dummyCRII(CategoryRuleItemInfo::OR));

    _generalFareRuleInfo->vendorCode() = "ATP";
    _generalFareRuleInfo->addItemInfoSetNosync(iset);

    // we don't want to use original CreateFare
    nfc->t_overrideCreateFareAndResetMinAmt = 1;
    nfc->processRule(*_paxTypeFare, false);

    // only one fare was created (1+1)
    CPPUNIT_ASSERT_EQUAL(2, nfc->t_overrideCreateFareAndResetMinAmt);
  }

  //-----------------------------------------------------------------
  // testValidate_PaxType_FareAdult_Rec3Adult()
  // CASE: Validate fare and rec3 pax types set to ADT
  // Expected to PASS
  //-----------------------------------------------------------------
  void testValidate_PaxType_FareAdult_Rec3Adult()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "ADT", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(_negotiatedFareController->validatePaxType(*_paxTypeFare, "ADT"));
  }

  //-----------------------------------------------------------------
  // testValidate_PaxType_FareBlank_Rec3Other()
  // CASE: Validate fare pax type set to BLANK, rec3 pax type set to other
  // Expected to PASS
  //-----------------------------------------------------------------
  void testValidate_PaxType_FareBlank_Rec3Other()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "", RuleConst::SELLING_FARE);
    addReqPaxTypeToTrx("MIL");
    CPPUNIT_ASSERT(!_negotiatedFareController->validatePaxType(*_paxTypeFare, "MIL"));
  }

  //-----------------------------------------------------------------
  // testValidate_PaxType_FareBlank_Rec3Other_OtherPTCRequested()
  // CASE: Validate fare pax type set to BLANK, rec3 pax type set to other
  // but other PTC requested
  // Expected to FAIL
  //-----------------------------------------------------------------
  void testValidate_PaxType_FareBlank_Rec3Other_OtherPTCRequested()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "", RuleConst::SELLING_FARE);
    addReqPaxTypeToTrx("CNN");
    CPPUNIT_ASSERT(!_negotiatedFareController->validatePaxType(*_paxTypeFare, "MIL"));
  }

  //-----------------------------------------------------------------
  // testValidate_PaxType_Same()
  // CASE: Validate same pax types both in fare and in rec3
  // Expected to PASS
  //-----------------------------------------------------------------
  void testValidate_PaxType_Same()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "MIL", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(_negotiatedFareController->validatePaxType(*_paxTypeFare, "MIL"));
  }

  //-----------------------------------------------------------------
  // testValidate_PaxType_Different()
  // CASE: Validate different pax types both in fare and in rec3
  // Expected to FAIL
  //-----------------------------------------------------------------
  void testValidate_PaxType_Different()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "MIL", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->validatePaxType(*_paxTypeFare, "CNN"));
  }

  //-----------------------------------------------------------------
  // testValidate_PaxType_FareNegGroup_Rec3NegGroup()
  // CASE: Validate neg group pax type for both rec3 and fare
  // Expected to PASS
  //-----------------------------------------------------------------
  void testValidate_PaxType_FareNegGroup_Rec3NegGroup()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "CNE", RuleConst::SELLING_FARE);
    addReqPaxTypeToTrx("NEG");
    CPPUNIT_ASSERT(_negotiatedFareController->validatePaxType(*_paxTypeFare, "NEG"));
  }

  //-----------------------------------------------------------------
  // testValidate_PaxType_FareNegGroup_Rec3NegGroup_OtherPTCRequested()
  // CASE: Validate neg group pax type for both rec3 and fare
  // but other PTC requested
  // Expected to FAIL
  //-----------------------------------------------------------------
  void testValidate_PaxType_FareNegGroup_Rec3NegGroup_OtherPTCRequested()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "CNE", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->validatePaxType(*_paxTypeFare, "NEG"));
  }

  //-----------------------------------------------------------------
  // testValidate_PaxType_FareNegGroup_Rec3Other()
  // CASE: Validate neg group pax type for both rec3 and fare
  // Expected to FAIL
  //-----------------------------------------------------------------
  void testValidate_PaxType_FareNegGroup_Rec3Other()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "NEG", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->validatePaxType(*_paxTypeFare, "MIL"));
  }

  //-----------------------------------------------------------------
  // testValidate_PaxType_FareJcbGroup_Rec3JcbGroup()
  // CASE: Validate neg group pax type for both rec3 and fare
  // Expected to PASS
  //-----------------------------------------------------------------
  void testValidate_PaxType_FareJcbGroup_Rec3JcbGroup()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "JNN", RuleConst::SELLING_FARE);
    addReqPaxTypeToTrx("JCB");
    CPPUNIT_ASSERT(_negotiatedFareController->validatePaxType(*_paxTypeFare, "JCB"));
  }

  //-----------------------------------------------------------------
  // testValidate_PaxType_FareJcbGroup_Rec3JcbGroup_OtherPTCRequested()
  // CASE: Validate neg group pax type for both rec3 and fare
  // but other PTC requested
  // Expected to FAIL
  //-----------------------------------------------------------------
  void testValidate_PaxType_FareJcbGroup_Rec3JcbGroup_OtherPTCRequested()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "JNN", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->validatePaxType(*_paxTypeFare, "JCB"));
  }

  //-----------------------------------------------------------------
  // testValidate_PaxType_FareJcbGroup_Rec3Other()
  // CASE: Validate neg group pax type for both rec3 and fare
  // Expected to FAIL
  //-----------------------------------------------------------------
  void testValidate_PaxType_FareJcbGroup_Rec3Other()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "JCB", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->validatePaxType(*_paxTypeFare, "MIL"));
  }

  //-----------------------------------------------------------------
  // testValidate_PaxType_FarePfaGroup_Rec3PfaGroup()
  // CASE: Validate neg group pax type for both rec3 and fare
  // Expected to PASS
  //-----------------------------------------------------------------
  void testValidate_PaxType_FarePfaGroup_Rec3PfaGroup()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "CBC", RuleConst::SELLING_FARE);
    addReqPaxTypeToTrx("PFA");
    CPPUNIT_ASSERT(_negotiatedFareController->validatePaxType(*_paxTypeFare, "PFA"));
  }

  //-----------------------------------------------------------------
  // testValidate_PaxType_FarePfaGroup_Rec3PfaGroup_OtherPTCRequested()
  // CASE: Validate neg group pax type for both rec3 and fare
  // but other PTC requested
  // Expected to FAIL
  //-----------------------------------------------------------------
  void testValidate_PaxType_FarePfaGroup_Rec3PfaGroup_OtherPTCRequested()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "CBC", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->validatePaxType(*_paxTypeFare, "PFA"));
  }

  //-----------------------------------------------------------------
  // testValidate_PaxType_FarePfaGroup_Rec3Other()
  // CASE: Validate neg group pax type for both rec3 and fare
  // Expected to FAIL
  //-----------------------------------------------------------------
  void testValidate_PaxType_FarePfaGroup_Rec3Other()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "PFA", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->validatePaxType(*_paxTypeFare, "MIL"));
  }

  void testValidate_PaxType_FareOther_Rec3Adult()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "MIL", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->validatePaxType(*_paxTypeFare, ADULT));
  }

  void testValidate_PaxType_FareAdult_Rec3Other()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "ADT", RuleConst::SELLING_FARE);
    //addReqPaxTypeToTrx("MIL");
    CPPUNIT_ASSERT(!_negotiatedFareController->validatePaxType(*_paxTypeFare, "MIL"));
  }

  //-----------------------------------------------------------------
  // testValidate_OverrideDateItem()
  // CASE: Validate override date table item set to 0
  // Expected to PASS
  //-----------------------------------------------------------------
  void testValidate_OverrideDateItem_NotSet()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "ADT", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(_negotiatedFareController->validateDateOverrideTable(*_paxTypeFare, 0));
  }

  //-----------------------------------------------------------------
  // testValidate_OverrideDateItem()
  // CASE: Validate override date table item set to 1000
  // Expected to FAIL (there is no such item in database)
  //-----------------------------------------------------------------
  void testValidate_OverrideDateItem_Set()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "ADT", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->validateDateOverrideTable(*_paxTypeFare, 1000));
  }

  //-----------------------------------------------------------------
  // testValidate_Fail_UnavailableTag_NoData()
  // CASE: Validate when unavailable tag of rec3 is set to
  // data unavailable or to text only values
  // Expected FAIL
  //-----------------------------------------------------------------
  void testValidate_Fail_UnavailableTag_NoData()
  {
    setupTestingData(MockNegFareRest("ADT", 0, RuleApplicationBase::dataUnavailable),
                     PricingTrx::PRICING_TRX,
                     "ADT",
                     RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT_EQUAL(FAIL, _negotiatedFareController->validate(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidate_Skip_UnavailableTag_TextOnly()
  // CASE: Validate when unavailable tag of rec3 is set to text only value
  // Expected SKIP
  //-----------------------------------------------------------------
  void testValidate_Skip_UnavailableTag_TextOnly()
  {
    setupTestingData(MockNegFareRest("ADT", 0, RuleApplicationBase::textOnly),
                     PricingTrx::PRICING_TRX,
                     "ADT",
                     RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT_EQUAL(SKIP, _negotiatedFareController->validate(*_paxTypeFare));
  }

  void testValidate_Fail_PaxType()
  {
    setupTestingData(
        MockNegFareRest("MIL"), PricingTrx::PRICING_TRX, "ADT", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT_EQUAL(FAIL, _negotiatedFareController->validate(*_paxTypeFare));
  }

  void testValidate_Fail_OverrideTable()
  {
    setupTestingData(MockNegFareRest("ADT", 0xFFFFFFFF), // fail reason - not existing element
                     PricingTrx::PRICING_TRX,
                     "ADT",
                     RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT_EQUAL(FAIL, _negotiatedFareController->validate(*_paxTypeFare));
  }

  void testValidate_Fail_Carrier()
  {
    setupTestingData(MockNegFareRest("ADT", CarrierCode("AA")),
                     PricingTrx::PRICING_TRX,
                     "ADT",
                     RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT_EQUAL(FAIL, _negotiatedFareController->validate(*_paxTypeFare));
  }
  void testValidate_Pass_MultipleCarriers()
  {
    setupTestingData(MockNegFareRest("ADT", CarrierCode("AA")),
                     PricingTrx::PRICING_TRX,
                     "ADT",
                     RuleConst::SELLING_FARE);
    _trx->setValidatingCxrGsaApplicable(true);
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->validatingCarriers().push_back("AA");
    fm->validatingCarriers().push_back("LH");
    _paxTypeFare->fareMarket() = fm;
    CPPUNIT_ASSERT_EQUAL(PASS, _negotiatedFareController->validate(*_paxTypeFare));
  }
  void testValidate_Fail_MultipleCarriers()
  {
    setupTestingData(MockNegFareRest("ADT", CarrierCode("AA")),
                     PricingTrx::PRICING_TRX,
                     "ADT",
                     RuleConst::SELLING_FARE);
    _trx->setValidatingCxrGsaApplicable(true);
    FareMarket* fm = _memHandle.create<FareMarket>();
    fm->validatingCarriers().push_back("LH");
    fm->validatingCarriers().push_back("LO");
    _paxTypeFare->fareMarket() = fm;
    CPPUNIT_ASSERT_EQUAL(FAIL, _negotiatedFareController->validate(*_paxTypeFare));
  }
  void testValidate_Fail_Cat35TktData()
  {
    setupTestingData(
        MockNegFareRest("ADT", 0), PricingTrx::FAREDISPLAY_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    enableAbacus();
    CPPUNIT_ASSERT_EQUAL(FAIL, _negotiatedFareController->validate(*_paxTypeFare));
  }

  void testValidate_Pass()
  {
    setupTestingData(
        MockNegFareRest("ADT"), PricingTrx::PRICING_TRX, "ADT", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT_EQUAL(PASS, _negotiatedFareController->validate(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_FD_SellingFare()
  // CASE: Validate cat35 data for FareDisplayTrx for selling fare
  // Expected TRUE
  //-----------------------------------------------------------------
  void testValidateCat35Data_FD_SellingFare()
  {
    setupTestingData(
        MockNegFareRest("ADT"), PricingTrx::FAREDISPLAY_TRX, "ADT", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_FD_NetSubmitFare_NoT979()
  // CASE: Validate cat35 data for FareDisplayTrx for net submit fare
  // and no T979 available
  // Expected FALSE
  //-----------------------------------------------------------------
  void testValidateCat35Data_FD_NetSubmitFare_NoT979()
  {
    // fail reason for NET fare in FD
    setupTestingData(
        MockNegFareRest("ADT", 0), PricingTrx::FAREDISPLAY_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_SellingFare_MethodBlank_NoTktData()
  // CASE: Validate cat35 data for Pricing for selling fare, method blank
  // and no tkt data available
  // Expected TRUE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_SellingFare_MethodBlank_NoTktData()
  {
    setupTestingData(MockNegFareRest("ADT", 0, ' ', RuleConst::NRR_METHOD_BLANK),
                     PricingTrx::PRICING_TRX,
                     "ADT",
                     RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_SellingFare_MethodBlank_TktData()
  // CASE: Validate cat35 data for Pricing for selling fare, method blank
  // and tkt data available
  // Expected FALSE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_SellingFare_MethodBlank_TktData()
  {
    MockNegFareRest negFareRest("ADT", 0, ' ', RuleConst::NRR_METHOD_BLANK);
    // fail reason
    negFareRest.owrt1() = '1';
    setupTestingData(negFareRest, PricingTrx::PRICING_TRX, "ADT", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_SellingFare()
  // CASE: Validate cat35 data for Pricing for selling fare, method other than blank
  // and no tkt data
  // Expected FALSE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_SellingFare()
  {
    setupTestingData(MockNegFareRest("ADT", 0, ' ', RuleConst::NRR_METHOD_2),
                     PricingTrx::PRICING_TRX,
                     "ADT",
                     RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_NetSubmitFare_ValidCommission()
  // CASE: Validate cat35 data for Pricing for net submit fare, method other than blank
  // and no tkt data valid commission data and t979 available
  // Expected TRUE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_NetSubmitFare_ValidCommission()
  {
    MockNegFareRest negFareRest("ADT", 1111, ' ', RuleConst::NRR_METHOD_2);
    negFareRest.commPercent() = 300;
    setupTestingData(negFareRest, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    CPPUNIT_ASSERT(_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_NetSubmitFare_InvalidCommission()
  // CASE: Validate cat35 data for Pricing for net submit fare, method other than blank
  // and no tkt data invalid commission data and t979 available
  // Expected FALSE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_NetSubmitFare_InvalidCommission()
  {
    MockNegFareRest negFareRest("ADT", 1111, ' ', RuleConst::NRR_METHOD_2);
    negFareRest.commPercent() = 300;
    // fail reason
    negFareRest.cur1() = "USD";
    setupTestingData(negFareRest, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_NetSubmitFareUpd_ValidCommission()
  // CASE: Validate cat35 data for Pricing for net submit fare upd, method other than blank
  // valid commission data
  // Expected TRUE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_NetSubmitFareUpd_ValidCommission()
  {
    MockNegFareRest negFareRest("ADT", 0, ' ', RuleConst::NRR_METHOD_3);
    negFareRest.commPercent() = 300;
    setupTestingData(negFareRest, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD);
    CPPUNIT_ASSERT(_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_NetSubmitFareUpd_ValidCommission()
  // CASE: Validate cat35 data for Pricing for net submit fare upd, method other than blank
  // invalid commission data
  // Expected FALSE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_NetSubmitFareUpd_InvalidCommission()
  {
    MockNegFareRest negFareRest("ADT", 0, ' ', RuleConst::NRR_METHOD_3);
    negFareRest.commPercent() = 300;
    // fail reason for method3
    negFareRest.cur1() = "USD";
    setupTestingData(negFareRest, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD);
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_NetSubmitFare_MethodBlank_TktData()
  // CASE: Validate cat35 data for Pricing for net submit fare, method blank
  // and no tkt data
  // Expected FALSE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_NetSubmitFare_MethodBlank_TktData()
  {
    MockNegFareRest negFareRest("ADT", 0, ' ', RuleConst::NRR_METHOD_BLANK);
    // fail reason for method blank
    negFareRest.owrt1() = '1';
    setupTestingData(negFareRest, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_NetSubmitFare_MethodBlank_NoTktData()
  // CASE: Validate cat35 data for Pricing for net submit fare, method blank
  // and tkt data available
  // Expected TRUE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_NetSubmitFare_MethodBlank_NoTktData()
  {
    setupTestingData(MockNegFareRest("ADT", 0, ' ', RuleConst::NRR_METHOD_BLANK),
                     PricingTrx::PRICING_TRX,
                     "ADT",
                     RuleConst::NET_SUBMIT_FARE);
    CPPUNIT_ASSERT(_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_NetSubmitFareUpd_Method2_TktData()
  // CASE: Validate cat35 data for Pricing for net submit fare upd, method 2
  // and tkt data available
  // Expected TRUE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_NetSubmitFareUpd_Method2_TktData()
  {
    MockNegFareRest negFareRest("ADT", 0, ' ', RuleConst::NRR_METHOD_2);
    negFareRest.tktFareDataInd1() = 'X';
    negFareRest.owrt1() = '1';
    setupTestingData(negFareRest, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD);
    CPPUNIT_ASSERT(_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_NetSubmitFareUpd_Method2_NoTktData()
  // CASE: Validate cat35 data for Pricing for net submit fare upd, method 2
  // and no tkt data
  // Expected FALSE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_NetSubmitFareUpd_Method2_NoTktData()
  {
    MockNegFareRest negFareRest("ADT", 0, ' ', RuleConst::NRR_METHOD_2);
    // fail reason for NET fare and METHOD 2
    negFareRest.commPercent() = 300;
    setupTestingData(negFareRest, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD);
    CPPUNIT_ASSERT(_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_NetSubmitFareUpd_Method3()
  // CASE: Validate cat35 data for Pricing for net submit fare upd, method 3
  // Expected TRUE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_NetSubmitFareUpd_Method3()
  {
    MockNegFareRest negFareRest("ADT", 0, ' ', RuleConst::NRR_METHOD_3);
    // this not fails for NET fare and METHOD 3
    negFareRest.commPercent() = 300;
    setupTestingData(negFareRest, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD);
    CPPUNIT_ASSERT(_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_NetSubmitFare_Method3()
  // CASE: Validate cat35 data for Pricing for net submit fare, method 3
  // Expected TRUE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_NetSubmitFare_Method3()
  {
    MockNegFareRest negFareRest("ADT", 0, ' ', RuleConst::NRR_METHOD_3);
    // this not fails for NET fare and METHOD 3
    negFareRest.commPercent() = 300;
    setupTestingData(negFareRest, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    CPPUNIT_ASSERT(_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_NetSubmitFareUpd_Method1()
  // CASE: Validate cat35 data for Pricing for net submit fare upd, method 1
  // Expected FALSE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_NetSubmitFareUpd_Method1()
  {
    MockNegFareRest negFareRest("ADT", 0, ' ', RuleConst::NRR_METHOD_1);
    // fail reason for NET fare and METHOD 1
    negFareRest.commPercent() = 300;
    setupTestingData(negFareRest, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD);
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_NetSubmitFare_Method3()
  // CASE: Validate cat35 data for Pricing for net submit fare, method 1
  // Expected FALSE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_NetSubmitFare_Method1()
  {
    MockNegFareRest negFareRest("ADT", 0, ' ', RuleConst::NRR_METHOD_1);
    // fail reason for NET fare and METHOD 1
    negFareRest.commPercent() = 300;
    setupTestingData(negFareRest, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_NetSubmitFareUpd_Method2_TktData_FareDataIndBlank()
  // CASE: Validate cat35 data for Pricing for net submit fare upd, method 2
  // and tkt data available and tktFareDataInd1 set to blank
  // Expected FALSE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_NetSubmitFareUpd_Method2_TktData_FareDataIndBlank()
  {
    MockNegFareRest negFareRest("ADT", 0, ' ', RuleConst::NRR_METHOD_3);
    // fail reason for NET fare and METHOD 3
    negFareRest.commPercent() = 300;
    negFareRest.tktFareDataInd1() = ' ';
    negFareRest.owrt1() = '1';
    setupTestingData(negFareRest, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD);
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_NetSubmitFare_Method2_TktData_FareDataIndBlank()
  // CASE: Validate cat35 data for Pricing for net submit fare upd, method 2
  // and tkt data available and tktFareDataInd1 set to blank
  // Expected FALSE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_NetSubmitFare_Method2_TktData_FareDataIndBlank()
  {
    MockNegFareRest negFareRest("ADT", 0, ' ', RuleConst::NRR_METHOD_3);
    // fail reason for NET fare and METHOD 3
    negFareRest.commPercent() = 300;
    negFareRest.tktFareDataInd1() = ' ';
    negFareRest.owrt1() = '1';
    setupTestingData(negFareRest, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_NetSubmitFare_Method3()
  // CASE: Validate cat35 data in case of recuring segment data present
  // and ticketed data not present
  // Expected FALSE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_RecSegData_NoTktData()
  {
    MockNegFareRest negFareRest("ADT", 0, ' ', RuleConst::NRR_METHOD_BLANK);
    // fail reason for selling fare and METHOD BLANK
    negFareRest.owrt2() = 'X';
    setupTestingData(negFareRest, PricingTrx::PRICING_TRX, "ADT", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  //-----------------------------------------------------------------
  // testValidateCat35Data_Pricing_NetSubmitFare_Method3()
  // CASE: Validate cat35 data in case of recuring segment data present
  // and ticketed data present
  // Expected TRUE
  //-----------------------------------------------------------------
  void testValidateCat35Data_Pricing_RecSegData_TktData()
  {
    setupTestingData(MockNegFareRest("ADT", 0, ' ', RuleConst::NRR_METHOD_BLANK),
                     PricingTrx::PRICING_TRX,
                     "ADT",
                     RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(_negotiatedFareController->validateCat35TktData(*_paxTypeFare));
  }

  void testValidateCat35TktDataKorea_Pass_NotMethod3()
  {
    setupTestingData(MockNegFareRest("ADT"));
    // this is not method 3
    CPPUNIT_ASSERT(_negotiatedFareController->validateCat35TktDataKorea(
        RuleConst::NRR_METHOD_2, true, Agent()));
  }

  void testValidateCat35TktDataKorea_Pass_TktOrExchange()
  {
    setupTestingData(MockNegFareRest("ADT"));
    // ticket or exchange (2nd parameter) is false
    CPPUNIT_ASSERT(_negotiatedFareController->validateCat35TktDataKorea(
        RuleConst::NRR_METHOD_3, false, Agent()));
  }

  void testValidateCat35TktDataKorea_Pass_Korea()
  {
    setupTestingData(MockNegFareRest("ADT"));
    // create agent in Korea
    Loc korea;
    korea.nation() = KOREA;
    Agent agent;
    agent.agentLocation() = &korea;

    CPPUNIT_ASSERT(
        _negotiatedFareController->validateCat35TktDataKorea(RuleConst::NRR_METHOD_3, true, agent));
  }

  void testValidateCat35TktDataKorea_Fail()
  {
    setupTestingData(MockNegFareRest("ADT"));
    // create agent outside Korea
    Loc loc;
    loc.nation() = "AAA";
    Agent agent;
    agent.agentLocation() = &loc;
    // all conditions fails
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35TktDataKorea(
        RuleConst::NRR_METHOD_3, true, agent));
  }

  void testValidateCat35TktDataL_InfiniAgent_Method1_NoT979()
  {
    setupTestingData(MockNegFareRest("ADT"));
    Customer tjr;
    Agent agent;
    agent.agentTJR() = &tjr;
    tjr.crsCarrier() = "1F";
    tjr.hostName() = "INFI";
    _trx->getRequest()->ticketingAgent() = &agent;
    // all conditions fails
    CPPUNIT_ASSERT(_negotiatedFareController->validateCat35TktDataL(
        RuleConst::SELLING_FARE, RuleConst::NRR_METHOD_1, true));
  }

  void testValidateCat35TktDataL_InfiniAgent_Method2_NoT979()
  {
    setupTestingData(MockNegFareRest("ADT"));
    Customer tjr;
    Agent agent;
    agent.agentTJR() = &tjr;
    tjr.crsCarrier() = "1F";
    tjr.hostName() = "INFI";
    _trx->getRequest()->ticketingAgent() = &agent;
    // all conditions fails
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35TktDataL(
        RuleConst::SELLING_FARE, RuleConst::NRR_METHOD_2, true));
  }

  void testValidateCat35TktDataL_InfiniAgent_Method2_ExistingT979()
  {
    setupTestingData(MockNegFareRest("ADT", 1));
    Customer tjr;
    Agent agent;
    agent.agentTJR() = &tjr;
    tjr.crsCarrier() = "1F";
    tjr.hostName() = "INFI";
    _trx->getRequest()->ticketingAgent() = &agent;
    // all conditions fails
    CPPUNIT_ASSERT(_negotiatedFareController->validateCat35TktDataL(
        RuleConst::SELLING_FARE, RuleConst::NRR_METHOD_2, true));
  }

  void testValidateCat35TktDataL_NoInfiniAgent_Method1_NoT979()
  {
    setupTestingData(MockNegFareRest("ADT"));
    Customer tjr;
    Agent agent;
    agent.agentTJR() = &tjr;
    tjr.crsCarrier() = "1S";
    tjr.hostName() = "";
    _trx->getRequest()->ticketingAgent() = &agent;
    // all conditions fails
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35TktDataL(
        RuleConst::SELLING_FARE, RuleConst::NRR_METHOD_1, true));
  }

  void testValidCommissions()
  {
    setupTestingData(MockNegFareRest("ADT"));

    _negFareRest->commPercent() = RuleConst::PERCENT_NO_APPL;
    CPPUNIT_ASSERT(_negotiatedFareController->isValidCommissions());

    _negFareRest->commPercent() = 10;
    CPPUNIT_ASSERT(_negotiatedFareController->isValidCommissions());
  }

  void testInvalidCommissions()
  {
    setupTestingData(MockNegFareRest("ADT"));

    _negFareRest->commPercent() = 20;
    _negFareRest->cur1() = "USD";
    CPPUNIT_ASSERT(!_negotiatedFareController->isValidCommissions());

    _negFareRest->commPercent() = 30;
    _negFareRest->cur2() = "PLN";
    CPPUNIT_ASSERT(!_negotiatedFareController->isValidCommissions());
  }

  void testCheckFareJalAxess_Fail()
  {
    setupTestingData(true, true);
    CPPUNIT_ASSERT(!_negotiatedFareController->checkFareJalAxess(*_paxTypeFare));
  }

  void testCheckFareJalAxess_Pass_FD()
  {
    // Pass because here is FD
    setupTestingData(true, true, PricingTrx::FAREDISPLAY_TRX);
    CPPUNIT_ASSERT(_negotiatedFareController->checkFareJalAxess(*_paxTypeFare));
  }

  void testCheckFareJalAxess_Pass_FareNotValid()
  {
    // pass because fare is not valid
    setupTestingData(true, true);
    _paxTypeFare->invalidateFare(PaxTypeFare::FD_Cat35_Incomplete_Data);
    CPPUNIT_ASSERT(_negotiatedFareController->checkFareJalAxess(*_paxTypeFare));
  }

  void testCheckFareJalAxess_Pass_FareSelling()
  {
    // Pass because fare is selling or net
    setupTestingData(true, true, PricingTrx::PRICING_TRX, "ADT", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(_negotiatedFareController->checkFareJalAxess(*_paxTypeFare));
  }

  void testCheckFareJalAxess_Pass_NotAxess()
  {
    // Fail because Axess is not requested (1st parameter)
    setupTestingData(false, true);
    CPPUNIT_ASSERT(_negotiatedFareController->checkFareJalAxess(*_paxTypeFare));
  }

  void testCheckFareJalAxess_Pass_NotNet()
  {
    // Fail because wpnet is not requested (2nd parameter)
    setupTestingData(true, false);
    CPPUNIT_ASSERT(_negotiatedFareController->checkFareJalAxess(*_paxTypeFare));
  }

  void testIsSellingOrNet_SellingFare()
  {
    CPPUNIT_ASSERT(NegotiatedFareController::isSellingOrNet(RuleConst::SELLING_FARE));
  }

  void testIsSellingOrNet_NetSubmitFare()
  {
    CPPUNIT_ASSERT(NegotiatedFareController::isSellingOrNet(RuleConst::NET_SUBMIT_FARE));
  }

  void testIsSellingOrNet_NetSubmitFareUpd()
  {
    CPPUNIT_ASSERT(NegotiatedFareController::isSellingOrNet(RuleConst::NET_SUBMIT_FARE_UPD));
  }

  void testIsSellingOrNet_Fail() { CPPUNIT_ASSERT(!NegotiatedFareController::isSellingOrNet(' ')); }

  void testCheckFare_JalAxessFail()
  {
    // fail because axess (1st parameter) is requested with NET fare
    setupTestingData(true, false, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD);
    CPPUNIT_ASSERT(!_negotiatedFareController->checkFare(*_paxTypeFare));
  }

  void testCheckFare_FareRexTrxFail()
  {
    // trx is rex trx (by defult New Itin flag and needRetrieveKeepFare are switched off)
    setupTestingData(PricingTrx::REPRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD);
    // invalidate fare
    _paxTypeFare->invalidateFare(PaxTypeFare::FD_Cat35_Incomplete_Data);
    // and switch on needRetrieveKeepFare flag
    static_cast<RexPricingTrx*>(_trx)->markFareRetrievalMethodKeep();

    CPPUNIT_ASSERT(!_negotiatedFareController->checkFare(*_paxTypeFare));
  }

  void testCheckFare_FareTariffCarrierFail()
  {
    // it fails because ptf is not private
    setupTestingData(PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD);
    CPPUNIT_ASSERT(!_negotiatedFareController->checkFare(*_paxTypeFare));
  }

  void testCheckFare_FareNegFail()
  {
    setupTestingData(
        PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD, RuleConst::PRIVATE_TARIFF);
    // fail because _needByPassNegFares is true
    _negotiatedFareController->_needByPassNegFares = true;
    CPPUNIT_ASSERT(!_negotiatedFareController->checkFare(*_paxTypeFare));
  }

  void testCheckFare_FareCat25WithCat35Fail()
  {
    setupTestingData(
        PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD, RuleConst::PRIVATE_TARIFF);
    _negotiatedFareController->_needByPassNegFares = false;
    // fail because cat35NotAllowed is 'A'
    _trx->getOptions()->cat35NotAllowed() = 'A';
    CPPUNIT_ASSERT(!_negotiatedFareController->checkFare(*_paxTypeFare));
  }

  void testCheckFare_Pass()
  {
    setupTestingData(
        PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD, RuleConst::PRIVATE_TARIFF);
    _negotiatedFareController->_needByPassNegFares = false;
    CPPUNIT_ASSERT(_negotiatedFareController->checkFare(*_paxTypeFare));
  }

  void testCheckFareRexTrx_Pass_FareValid()
  {
    setupTestingData();
    CPPUNIT_ASSERT(_negotiatedFareController->checkFareRexTrx(*_paxTypeFare));
  }

  void testCheckFareRexTrx_Pass_FareRexTrxAndNewItin()
  {
    // trx is rex trx
    setupTestingData(PricingTrx::REPRICING_TRX);
    static_cast<RexPricingTrx*>(_trx)->trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    static_cast<RexPricingTrx*>(_trx)->markFareRetrievalMethodKeep();
    // so ignore invalid fare
    _paxTypeFare->invalidateFare(PaxTypeFare::FD_Cat35_Incomplete_Data);
    CPPUNIT_ASSERT(_negotiatedFareController->checkFareRexTrx(*_paxTypeFare));
  }

  void testCheckFareRexTrx_Fail_NeedRetrieveKeepFare()
  {
    // trx is rex trx // by default markFareRetrievalMethodKeep is switched off
    setupTestingData(PricingTrx::REPRICING_TRX);
    // and switch on needRetrieveKeepFare flag
    static_cast<RexPricingTrx*>(_trx)->markFareRetrievalMethodKeep();
    // so ignore invalid fare
    _paxTypeFare->invalidateFare(PaxTypeFare::FD_Cat35_Incomplete_Data);
    CPPUNIT_ASSERT(!_negotiatedFareController->checkFareRexTrx(*_paxTypeFare));
  }

  void testCheckFareRexTrx_Fail_NewItin()
  {
    // trx is rex trx (by defult New Itin flag and needRetrieveKeepFare are switched off)
    setupTestingData(PricingTrx::REPRICING_TRX);
    static_cast<RexPricingTrx*>(_trx)->trxPhase() = RexBaseTrx::PRICE_NEWITIN_PHASE;
    // so ignore invalid fare
    _paxTypeFare->invalidateFare(PaxTypeFare::FD_Cat35_Incomplete_Data);
    CPPUNIT_ASSERT(!_negotiatedFareController->checkFareRexTrx(*_paxTypeFare));
  }

  void testCheckFareTariffCarrier_Fail_NotPrivateFare()
  {
    // fare is not private
    setupTestingData();
    CPPUNIT_ASSERT(!NegotiatedFareController::checkFareTariffCarrier(*_paxTypeFare));
  }

  void testCheckFareTariffCarrier_Fail_NotSellingOrNet()
  {
    // fare is not selling/NET
    setupTestingData(PricingTrx::PRICING_TRX, "ADT", ' ', RuleConst::PRIVATE_TARIFF);
    CPPUNIT_ASSERT(!NegotiatedFareController::checkFareTariffCarrier(*_paxTypeFare));
  }

  void testCheckFareTariffCarrier_Fail_IndustryCarrier()
  {
    // carrier is industry
    setupTestingData(PricingTrx::PRICING_TRX,
                     "ADT",
                     RuleConst::NET_SUBMIT_FARE,
                     RuleConst::PRIVATE_TARIFF,
                     0,
                     "",
                     1,
                     INDUSTRY_CARRIER);
    CPPUNIT_ASSERT(!NegotiatedFareController::checkFareTariffCarrier(*_paxTypeFare));
  }

  void testCheckFareTariffCarrier_Pass()
  {
    setupTestingData(
        PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE, RuleConst::PRIVATE_TARIFF);
    CPPUNIT_ASSERT(NegotiatedFareController::checkFareTariffCarrier(*_paxTypeFare));
  }

  void testCheckFareNeg_FD_NET()
  {
    // this is Axess & FD & fare is NET upd
    setupTestingData(
        true, false, PricingTrx::FAREDISPLAY_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD);
    // after execution
    _negotiatedFareController->checkFareNeg(*_paxTypeFare);
    // category is not valid
    CPPUNIT_ASSERT(!_paxTypeFare->isCategoryValid(RuleConst::NEGOTIATED_RULE));
  }

  void testCheckFareNeg_NotFD_Selling()
  {
    // Pricing trx & fare is selling
    setupTestingData(PricingTrx::PRICING_TRX, "ADT", RuleConst::SELLING_FARE);
    // after execution
    _negotiatedFareController->checkFareNeg(*_paxTypeFare);
    // category is not valid
    CPPUNIT_ASSERT(!_paxTypeFare->isCategoryValid(RuleConst::NEGOTIATED_RULE));
  }

  void testCheckFareNeg_Pass()
  {
    setupTestingData();
    // if _needByPassNegFares is false return true
    _negotiatedFareController->_needByPassNegFares = false;
    CPPUNIT_ASSERT(_negotiatedFareController->checkFareNeg(*_paxTypeFare));
  }

  void testCheckFareNeg_Fail()
  {
    setupTestingData();
    // if _needByPassNegFares is true return false
    _negotiatedFareController->_needByPassNegFares = true;
    CPPUNIT_ASSERT(!_negotiatedFareController->checkFareNeg(*_paxTypeFare));
  }

  void testCheckFareCat25WithCat35_Pass_Cat35NotAllowed()
  {
    setupTestingData();
    // if cat35NotAllowed is not 'A' return true
    _trx->getOptions()->cat35NotAllowed() = ' ';
    CPPUNIT_ASSERT(_negotiatedFareController->checkFareCat25WithCat35(*_paxTypeFare));
  }

  void testCheckFareCat25WithCat35_Pass_IsFareByRule()
  {
    setupTestingData();
    // if ptf is cat25 return true
    _paxTypeFare->status().set(PaxTypeFare::PTF_FareByRule, true);
    CPPUNIT_ASSERT(_negotiatedFareController->checkFareCat25WithCat35(*_paxTypeFare));
  }

  void testCheckFareCat25WithCat35_Fail()
  {
    setupTestingData();
    // if cat35NotAllowed is 'A' return false
    _trx->getOptions()->cat35NotAllowed() = 'A';
    CPPUNIT_ASSERT(!_negotiatedFareController->checkFareCat25WithCat35(*_paxTypeFare));
  }

  void testProcessRedistributionsWithAccCode()
  {
    setupTestingData();

    _trx->getOptions()->forceCorpFares() = false;
    _trx->getRequest()->accountCode() = "A123";

    // Prepare GeneralFareRuleInfo
    auto* iset = new CategoryRuleItemInfoSet();
    iset->push_back(dummyCRII(CategoryRuleItemInfo::THEN));

    _negotiatedFareController->_catRuleItemInfo = &iset->front();
    _negotiatedFareController->_ruleItemInfoSet = iset;

    _generalFareRuleInfo->vendorCode() = "ATP";
    _generalFareRuleInfo->addItemInfoSetNosync(iset);

    MoneyAmount amt, amtNuc;
    NegPaxTypeFareRuleData* ruleData;

    _negotiatedFareController->resetMinAmt(amt, amtNuc, ruleData);
    NegFareRest nfr;
    _negotiatedFareController->_negFareRest = &nfr;

    int32_t seqNegMatch = 0;
    bool is979queried = false;
    Money base(_paxTypeFare->fare()->originalFareAmount(), _paxTypeFare->currency());

    amt = 100;
    amtNuc = 100;

    _negotiatedFareController->processRedistributions(*_paxTypeFare,
                                                      _negPaxTypeFareRuleData,
                                                      amt,
                                                      amtNuc,
                                                      base,
                                                      *_negFareCalc,
                                                      is979queried,
                                                      seqNegMatch);

    CPPUNIT_ASSERT_EQUAL(MoneyAmount(30), amt);
  }

  void testProcessRedistributionsNoAccCode()
  {
    setupTestingData();
    _trx->getOptions()->forceCorpFares() = false;

    // Prepare GeneralFareRuleInfo
    auto* iset = new CategoryRuleItemInfoSet();
    iset->push_back(dummyCRII(CategoryRuleItemInfo::THEN));
    _negotiatedFareController->_catRuleItemInfo = &iset->front();
    _negotiatedFareController->_ruleItemInfoSet = iset;

    _generalFareRuleInfo->vendorCode() = "ATP";
    _generalFareRuleInfo->addItemInfoSetNosync(iset);

    MoneyAmount amt, amtNuc;
    NegPaxTypeFareRuleData* ruleData;

    _negotiatedFareController->resetMinAmt(amt, amtNuc, ruleData);
    NegFareRest nfr;
    _negotiatedFareController->_negFareRest = &nfr;

    int32_t seqNegMatch = 0;
    bool is979queried = false;
    Money base(_paxTypeFare->fare()->originalFareAmount(), _paxTypeFare->currency());

    amt = 100;
    amtNuc = 100;

    _negotiatedFareController->processRedistributions(*_paxTypeFare,
                                                      _negPaxTypeFareRuleData,
                                                      amt,
                                                      amtNuc,
                                                      base,
                                                      *_negFareCalc,
                                                      is979queried,
                                                      seqNegMatch);

    CPPUNIT_ASSERT_EQUAL(MoneyAmount(50), amt);
  }

  void testProcessRedistributionsXC()
  {
    setupTestingData();
    _trx->getOptions()->forceCorpFares() = true;
    _trx->getRequest()->accountCode() = "B123";

    // Prepare GeneralFareRuleInfo
    auto* iset = new CategoryRuleItemInfoSet();
    iset->push_back(dummyCRII(CategoryRuleItemInfo::THEN));
    _negotiatedFareController->_catRuleItemInfo = &iset->front();
    _negotiatedFareController->_ruleItemInfoSet = iset;

    _generalFareRuleInfo->vendorCode() = "ATP";
    _generalFareRuleInfo->addItemInfoSetNosync(iset);

    MoneyAmount amt, amtNuc;
    NegPaxTypeFareRuleData* ruleData;

    _negotiatedFareController->resetMinAmt(amt, amtNuc, ruleData);
    NegFareRest nfr;
    _negotiatedFareController->_negFareRest = &nfr;

    int32_t seqNegMatch = 0;
    bool is979queried = false;
    Money base(_paxTypeFare->fare()->originalFareAmount(), _paxTypeFare->currency());

    amt = 150;
    amtNuc = 150;

    _negotiatedFareController->processRedistributions(*_paxTypeFare,
                                                      _negPaxTypeFareRuleData,
                                                      amt,
                                                      amtNuc,
                                                      base,
                                                      *_negFareCalc,
                                                      is979queried,
                                                      seqNegMatch);

    CPPUNIT_ASSERT_EQUAL(MoneyAmount(130), amt);
  }

  void testMatchMUCAccCodeWithRequestedCorpId_emptyVector()
  {
    setupTestingData();

    _trx->getOptions()->forceCorpFares() = true;
    _trx->getRequest()->isMultiAccCorpId() = true;
    MockMarkupControl markupRec("TST02");

    CPPUNIT_ASSERT(
        !_negotiatedFareController->matchMUCAccCode(markupRec, _trx->getRequest()->corpIdVec()));
  }

  void testMatchMUCAccCodeWithRequestedCorpId_pass()
  {
    setupTestingData();

    _trx->getOptions()->forceCorpFares() = true;
    _trx->getRequest()->isMultiAccCorpId() = true;
    _trx->getRequest()->corpIdVec().push_back("TST01");
    _trx->getRequest()->corpIdVec().push_back("TST02");
    _trx->getRequest()->corpIdVec().push_back("TST03");
    MockMarkupControl markupRec("TST02");

    CPPUNIT_ASSERT(
        _negotiatedFareController->matchMUCAccCode(markupRec, _trx->getRequest()->corpIdVec()));
  }

  void testMatchMUCAccCodeWithRequestedCorpId_fail()
  {
    setupTestingData();

    _trx->getOptions()->forceCorpFares() = true;
    _trx->getRequest()->isMultiAccCorpId() = true;
    _trx->getRequest()->corpIdVec().push_back("TST01");
    _trx->getRequest()->corpIdVec().push_back("TST02");
    _trx->getRequest()->corpIdVec().push_back("TST03");
    MockMarkupControl markupRec("TST05");

    CPPUNIT_ASSERT(
        !_negotiatedFareController->matchMUCAccCode(markupRec, _trx->getRequest()->corpIdVec()));
  }

  void testMatchMUCAccCodeWithRequestedAccCode_emptyVector()
  {
    setupTestingData();

    _trx->getOptions()->forceCorpFares() = true;
    _trx->getRequest()->isMultiAccCorpId() = true;
    MockMarkupControl markupRec("TST02");

    CPPUNIT_ASSERT(
        !_negotiatedFareController->matchMUCAccCode(markupRec, _trx->getRequest()->accCodeVec()));
  }

  void testMatchMUCAccCodeWithRequestedAccCode_pass()
  {
    setupTestingData();

    _trx->getOptions()->forceCorpFares() = true;
    _trx->getRequest()->isMultiAccCorpId() = true;
    _trx->getRequest()->accCodeVec().push_back("TST01");
    _trx->getRequest()->accCodeVec().push_back("TST02");
    _trx->getRequest()->accCodeVec().push_back("TST03");
    MockMarkupControl markupRec("TST02");

    CPPUNIT_ASSERT(
        _negotiatedFareController->matchMUCAccCode(markupRec, _trx->getRequest()->accCodeVec()));
  }

  void testMatchMUCAccCodeWithRequestedAccCode_fail()
  {
    setupTestingData();

    _trx->getOptions()->forceCorpFares() = true;
    _trx->getRequest()->isMultiAccCorpId() = true;
    _trx->getRequest()->accCodeVec().push_back("TST01");
    _trx->getRequest()->accCodeVec().push_back("TST02");
    _trx->getRequest()->accCodeVec().push_back("TST03");
    MockMarkupControl markupRec("TST05");

    CPPUNIT_ASSERT(
        !_negotiatedFareController->matchMUCAccCode(markupRec, _trx->getRequest()->accCodeVec()));
  }

  void testDoDiag335Collector()
  {
    setupTestingData(NegFareRest());

    MockDiag335Collector dc;
    _negotiatedFareController->_dc = &dc;

    dc.activate(); // diagnostic is active - response from MockDiag335Collector expected

    _negotiatedFareController->doDiag335Collector("");
    std::string ret = "displayFailCode\n";

    CPPUNIT_ASSERT_EQUAL(ret, _negotiatedFareController->_dc->str());
  }

  void testDoDiag335CollectorNotActive()
  {
    setupTestingData(NegFareRest());

    MockDiag335Collector dc;
    _negotiatedFareController->_dc = &dc;

    dc.deActivate(); // diagnostic is inactive - no response expected

    _negotiatedFareController->doDiag335Collector("");

    CPPUNIT_ASSERT_EQUAL(std::string(""), _negotiatedFareController->_dc->str());
  }

  void testDoDiag335CollectorDisplayTkt()
  {
    setupTestingData(NegFareRest());

    MockDiag335Collector dc;
    _negotiatedFareController->_dc = &dc;

    // TKT is specified so doNetRemitTkt should be executed at the end of displayFailCode method.
    dc.activate();
    std::pair<std::string, std::string> tkt(Diagnostic::DISPLAY_DETAIL, "TKT");
    _trx->diagnostic().diagParamMap().insert(tkt);

    _negotiatedFareController->doDiag335Collector("");
    std::string ret = "displayFailCode\ndoNetRemitTkt\n";

    CPPUNIT_ASSERT_EQUAL(ret, _negotiatedFareController->_dc->str());
  }

  void testShallKeepMinAmtFailNegativeNewAmt()
  {
    setupTestingData();

    MoneyAmount saveAmt;
    NegFareRest negFareRest;
    // new amount is negative, so we don't have to anything to keep
    MoneyAmount newAmt = -1;

    CPPUNIT_ASSERT(!_negotiatedFareController->shallKeepMinAmt(
        saveAmt, newAmt, *_negPaxTypeFareRuleData, *_paxTypeFare, negFareRest));
  }

  void testShallKeepMinAmtPassInvalidSaveAmt()
  {
    setupTestingData();

    NegFareRest negFareRest;
    // new amount is correct and old amount is invalid - return true
    MoneyAmount saveAmt = NegotiatedFareController::INVALID_AMT;
    MoneyAmount newAmt = 1;

    CPPUNIT_ASSERT(_negotiatedFareController->shallKeepMinAmt(
        saveAmt, newAmt, *_negPaxTypeFareRuleData, *_paxTypeFare, negFareRest));
  }

  void testShallKeepMinAmtPassSaveAmtGreaterThanNewAmt()
  {
    setupTestingData();

    NegFareRest negFareRest;
    // new amount is correct and old amount is greater than new one- return true
    MoneyAmount saveAmt = 2;
    MoneyAmount newAmt = 1;

    CPPUNIT_ASSERT(_negotiatedFareController->shallKeepMinAmt(
        saveAmt, newAmt, *_negPaxTypeFareRuleData, *_paxTypeFare, negFareRest));
  }

  void testShallKeepMinAmtPassNegGroup()
  {
    setupTestingData(PricingTrx::PRICING_TRX, CHILD);

    NegFareRest negFareRest;
    NegFareRest negFareRestForRuleData;
    _negPaxTypeFareRuleData->ruleItemInfo() = &negFareRestForRuleData;

    // new amount is greater than old one - should return false
    MoneyAmount saveAmt = 1;
    MoneyAmount newAmt = 2;
    // but precedence of types - return true
    negFareRest.psgType() = INFANT;
    negFareRestForRuleData.psgType() = CHILD;

    CPPUNIT_ASSERT(_negotiatedFareController->shallKeepMinAmt(
        saveAmt, newAmt, *_negPaxTypeFareRuleData, *_paxTypeFare, negFareRest));
  }

  void testShallKeepMinAmtFailNegGroup()
  {
    setupTestingData(PricingTrx::PRICING_TRX, CHILD);

    NegFareRest negFareRest;
    NegFareRest negFareRestForRuleData;
    _negPaxTypeFareRuleData->ruleItemInfo() = &negFareRestForRuleData;

    // new amount is greater than old one - should return false
    MoneyAmount saveAmt = 1;
    MoneyAmount newAmt = 2;
    // and precedence of types - return false also
    negFareRest.psgType() = CHILD;
    negFareRestForRuleData.psgType() = INFANT;

    CPPUNIT_ASSERT(!_negotiatedFareController->shallKeepMinAmt(
        saveAmt, newAmt, *_negPaxTypeFareRuleData, *_paxTypeFare, negFareRest));
  }

  void testShallKeepMinAmtPassJcbGroup()
  {
    setupTestingData(PricingTrx::PRICING_TRX, JCB);

    NegFareRest negFareRest;
    NegFareRest negFareRestForRuleData;
    _negPaxTypeFareRuleData->ruleItemInfo() = &negFareRestForRuleData;

    // new amount is greater than old one - should return false
    MoneyAmount saveAmt = 1;
    MoneyAmount newAmt = 2;
    // but precedence of types - return true
    negFareRest.psgType() = JNN;
    negFareRestForRuleData.psgType() = JCB;

    CPPUNIT_ASSERT(_negotiatedFareController->shallKeepMinAmt(
        saveAmt, newAmt, *_negPaxTypeFareRuleData, *_paxTypeFare, negFareRest));
  }

  void testShallKeepMinAmtFailJcbGroup()
  {
    setupTestingData(PricingTrx::PRICING_TRX, JCB);

    NegFareRest negFareRest;
    NegFareRest negFareRestForRuleData;
    _negPaxTypeFareRuleData->ruleItemInfo() = &negFareRestForRuleData;

    // new amount is greater than old one - should return false
    MoneyAmount saveAmt = 1;
    MoneyAmount newAmt = 2;
    // and precedence of types - return false also
    negFareRest.psgType() = JCB;
    negFareRestForRuleData.psgType() = JNN;

    CPPUNIT_ASSERT(!_negotiatedFareController->shallKeepMinAmt(
        saveAmt, newAmt, *_negPaxTypeFareRuleData, *_paxTypeFare, negFareRest));
  }

  void testShallKeepMinAmtPassPfaGroup()
  {
    setupTestingData(PricingTrx::PRICING_TRX, CBC);

    NegFareRest negFareRest;
    NegFareRest negFareRestForRuleData;
    _negPaxTypeFareRuleData->ruleItemInfo() = &negFareRestForRuleData;

    // new amount is greater than old one - should return false
    MoneyAmount saveAmt = 1;
    MoneyAmount newAmt = 2;
    // but precedence of types - return true
    negFareRest.psgType() = CBI;
    negFareRestForRuleData.psgType() = CBC;

    CPPUNIT_ASSERT(_negotiatedFareController->shallKeepMinAmt(
        saveAmt, newAmt, *_negPaxTypeFareRuleData, *_paxTypeFare, negFareRest));
  }

  void testShallKeepMinAmtFailPfaGroup()
  {
    setupTestingData(PricingTrx::PRICING_TRX, CBC);

    NegFareRest negFareRest;
    NegFareRest negFareRestForRuleData;
    _negPaxTypeFareRuleData->ruleItemInfo() = &negFareRestForRuleData;

    // new amount is greater than old one - should return false
    MoneyAmount saveAmt = 1;
    MoneyAmount newAmt = 2;
    // and precedence of types - return false also
    negFareRest.psgType() = CBC;
    negFareRestForRuleData.psgType() = CBI;

    CPPUNIT_ASSERT(!_negotiatedFareController->shallKeepMinAmt(
        saveAmt, newAmt, *_negPaxTypeFareRuleData, *_paxTypeFare, negFareRest));
  }

  /* This test is not valid any more since we select first match fare and not lowest fare  *
   * This test expect false should be return since new amount is greater than saved amount.*/
  void testShallKeepMinAmtFailNoConditionMet()
  {
    setupTestingData(PricingTrx::PRICING_TRX, ADULT);

    NegFareRest negFareRest;
    NegFareRest negFareRestForRuleData;
    _negPaxTypeFareRuleData->ruleItemInfo() = &negFareRestForRuleData;

    // new amount is greater than old one - should return false
    MoneyAmount saveAmt = 1;
    MoneyAmount newAmt = 2;
    // and ptf don't belong to correct group so ignore precedence of types - return false also
    negFareRest.psgType() = CBI;
    negFareRestForRuleData.psgType() = CBC;

    CPPUNIT_ASSERT(!_negotiatedFareController->shallKeepMinAmt(
        saveAmt, newAmt, *_negPaxTypeFareRuleData, *_paxTypeFare, negFareRest));
  }

  void testKeepMinAmtAbacusVendorNotPublished()
  {
    setupTestingData();

    FareInfo fi;
    fi.vendor() = "AAAA";
    _paxTypeFare->fare()->setFareInfo(&fi);
    _paxTypeFare->setFare(_paxTypeFare->fare());

    NegFareRest negFareRest;
    NegFareRest negFareRestForRuleData;
    _negFareCalc->creatorPCC() = "BBBB";
    Indicator ticketingInd = 'C';
    Indicator fareDisplayCat35 = 0;

    // this guarantte KeppMinAmt will be executed
    MoneyAmount saveAmt = NegotiatedFareController::INVALID_AMT;
    MoneyAmount newAmt = 1;

    // create abacus agent
    const std::vector<Customer*> abacus = _trx->dataHandle().getCustomer("5KAD");
    Agent* agent;
    _trx->dataHandle().get(agent);
    agent->agentTJR() = abacus[0];
    agent->mainTvlAgencyPCC() = "5KAD";
    _trx->getRequest()->ticketingAgent() = agent;
    TrxUtil::enableAbacus();

    _negotiatedFareController->keepMinAmt(saveAmt,
                                          newAmt,
                                          saveAmt,
                                          newAmt,
                                          _negPaxTypeFareRuleData,
                                          *_negFareCalc,
                                          *_paxTypeFare,
                                          ticketingInd,
                                          negFareRest,
                                          fareDisplayCat35);

    // check data was stored to ruleData
    CPPUNIT_ASSERT_EQUAL(ticketingInd, _negPaxTypeFareRuleData->tktIndicator());
    CPPUNIT_ASSERT_EQUAL(_negFareCalc->creatorPCC(), _negPaxTypeFareRuleData->creatorPCC());
  }

  void testKeepMinAmtAxessCat35()
  {
    setupTestingData();

    NegFareRest negFareRest;
    NegFareRest negFareRestForRuleData;
    _negFareCalc->creatorPCC() = "BBBB";
    Indicator ticketingInd = 'C';
    Indicator fareDisplayCat35 = 0;

    // this guarantte KeppMinAmt will be executed
    MoneyAmount saveAmt = NegotiatedFareController::INVALID_AMT;
    MoneyAmount newAmt = 1;

    // create axess agent
    const std::vector<Customer*> axess = _trx->dataHandle().getCustomer("A0NC");
    Agent* agent;
    _trx->dataHandle().get(agent);
    agent->agentTJR() = axess[0];
    agent->mainTvlAgencyPCC() = "A0NC";
    _trx->getRequest()->ticketingAgent() = agent;
    _trx->getRequest()->wpNettRequested() = 'Y';

    _negotiatedFareController->keepMinAmt(saveAmt,
                                          newAmt,
                                          saveAmt,
                                          newAmt,
                                          _negPaxTypeFareRuleData,
                                          *_negFareCalc,
                                          *_paxTypeFare,
                                          ticketingInd,
                                          negFareRest,
                                          fareDisplayCat35);

    // check data was stored to ruleData
    CPPUNIT_ASSERT_EQUAL(ticketingInd, _negPaxTypeFareRuleData->tktIndicator());
    CPPUNIT_ASSERT(_negPaxTypeFareRuleData->axessCat35Fare());
  }

  void testGetSecIdsForRedistributionForRedistribute()
  {
    setupTestingData();

    // N - ignore element
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->insertNegFareSecurityInfo('N', 1, 0);
    std::set<long> ret;
    _negotiatedFareController->getSecIdsForRedistribution(1000, ret);

    // so only one element was chosen
    CPPUNIT_ASSERT_EQUAL(1, (int)ret.size());
  }

  void testGetSecIdsForRedistributionSeqNegMatchZero()
  {
    setupTestingData();

    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->insertNegFareSecurityInfo('Y', 1, 0);
    std::set<long> ret;
    // 0  allow all.
    _negotiatedFareController->getSecIdsForRedistribution(0, ret);

    // so both elements were chosen
    CPPUNIT_ASSERT_EQUAL(2, (int)ret.size());
  }

  void testGetSecIdsForRedistributionSeqNegMatchNotZero()
  {
    setupTestingData();

    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->insertNegFareSecurityInfo('Y', 1, 0);
    std::set<long> ret;
    _negotiatedFareController->getSecIdsForRedistribution(1, ret);

    // 0 < 1 < 100 - so only one element was chosen
    CPPUNIT_ASSERT_EQUAL(1, (int)ret.size());
  }

  void testIfMarkupValidForRedistributionStatusDeclined()
  {
    // always fail for DECLINED status
    MockMarkupControl mc(NegotiatedFareController::DECLINED);
    CPPUNIT_ASSERT(!NegotiatedFareController::ifMarkupValidForRedistribution(mc, false));
    CPPUNIT_ASSERT(!NegotiatedFareController::ifMarkupValidForRedistribution(mc, true));
  }

  void testIfMarkupValidForRedistributionStatusPending()
  {
    // always fail for PENDING status
    MockMarkupControl mc(NegotiatedFareController::PENDING);
    CPPUNIT_ASSERT(!NegotiatedFareController::ifMarkupValidForRedistribution(mc, false));
    CPPUNIT_ASSERT(!NegotiatedFareController::ifMarkupValidForRedistribution(mc, true));
  }

  void testIfMarkupValidForRedistributionTicketingFail()
  {
    MockMarkupControl mc(' ', YES, NO);
    mc.tktTag() = NO;
    // return tkt tag
    CPPUNIT_ASSERT(!NegotiatedFareController::ifMarkupValidForRedistribution(mc, true));
  }

  void testIfMarkupValidForRedistributionSellingFail()
  {
    MockMarkupControl mc(' ', NO, YES);
    // return sell tag
    CPPUNIT_ASSERT(!NegotiatedFareController::ifMarkupValidForRedistribution(mc, false));
  }

  void testIfMarkupValidForRedistributionTicketingPass()
  {
    MockMarkupControl mc(' ', NO, YES);
    // return tkt tag
    CPPUNIT_ASSERT(NegotiatedFareController::ifMarkupValidForRedistribution(mc, true));
  }

  void testIfMarkupValidForRedistributionSellingPass()
  {
    MockMarkupControl mc(' ', YES, NO);
    // return sell tag
    CPPUNIT_ASSERT(NegotiatedFareController::ifMarkupValidForRedistribution(mc, false));
  }

  void testMatchMultiCorpIdAccCodeMUCPassEmptyAccountCode()
  {
    setupTestingData();
    MockMarkupControl mc(""); // it means return true
    CPPUNIT_ASSERT(_negotiatedFareController->matchMultiCorpIdAccCodeMUC(mc, *_paxTypeFare));
  }

  void testMatchMultiCorpIdAccCodeMUCFailEmptyCorpIDAndAccCode()
  {
    setupTestingData();
    MockMarkupControl mc("A");
    // look for "A" in corpIdVec & accCodeVec
    _trx->getRequest()->corpIdVec().clear();
    _trx->getRequest()->accCodeVec().clear();
    CPPUNIT_ASSERT(!_negotiatedFareController->matchMultiCorpIdAccCodeMUC(mc, *_paxTypeFare));
  }

  void testMatchMultiCorpIdAccCodeMUCPassCorpIDMatched()
  {
    setupTestingData();
    MockMarkupControl mc("A");
    // look for "A" in corpIdVec & accCodeVec
    _trx->getRequest()->corpIdVec().push_back("A");
    CPPUNIT_ASSERT(_negotiatedFareController->matchMultiCorpIdAccCodeMUC(mc, *_paxTypeFare));
  }

  void testMatchMultiCorpIdAccCodeMUCPassMultiCorpIDMatched()
  {
    setupTestingData();
    MockMarkupControl mc("A");
    // look for "A" in corpIdVec & accCodeVec - not found
    _trx->getRequest()->corpIdVec().push_back("B");
    // but checkMultiCorpIdMatrix returns true
    ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideCheckMultiCorpIdMatrix =
        true;
    CPPUNIT_ASSERT(_negotiatedFareController->matchMultiCorpIdAccCodeMUC(mc, *_paxTypeFare));
  }

  void testMatchMultiCorpIdAccCodeMUCPassAccCodeMatched()
  {
    setupTestingData();
    MockMarkupControl mc("A");
    // look for "A" in corpIdVec & accCodeVec
    _trx->getRequest()->accCodeVec().push_back("A");
    CPPUNIT_ASSERT(_negotiatedFareController->matchMultiCorpIdAccCodeMUC(mc, *_paxTypeFare));
  }

  void testMatchMultiCorpIdAccCodeMUCFailNotMatched()
  {
    setupTestingData();
    MockMarkupControl mc("A");
    // look for "A" in corpIdVec & accCodeVec
    _trx->getRequest()->corpIdVec().push_back("B");
    _trx->getRequest()->accCodeVec().push_back("B");
    CPPUNIT_ASSERT(!_negotiatedFareController->matchMultiCorpIdAccCodeMUC(mc, *_paxTypeFare));
  }

  void testMatchMultiCorpIdAccCodeMUCFailForceCorpFares()
  {
    setupTestingData();
    MockMarkupControl mc(""); // it means return true
    // unless matchEmptyCorpIdAccCodeMUC fails
    _trx->getOptions()->forceCorpFares() = true;
    CPPUNIT_ASSERT(!_negotiatedFareController->matchMultiCorpIdAccCodeMUC(mc, *_paxTypeFare));
  }

  void testMatchCorpIdAccCodeMUCPassEmptyAccountCode()
  {
    setupTestingData();
    MockMarkupControl mc(""); // it means return true
    CPPUNIT_ASSERT(_negotiatedFareController->matchCorpIdAccCodeMUC(mc, *_paxTypeFare));
  }

  void testMatchCorpIdAccCodeMUCFailEmptyCorpIDAndAccCode()
  {
    setupTestingData();
    MockMarkupControl mc("A");
    // look for "A" in corpIdVec & accCodeVec
    _trx->getRequest()->corporateID() = "";
    _trx->getRequest()->accountCode() = "";
    CPPUNIT_ASSERT(!_negotiatedFareController->matchCorpIdAccCodeMUC(mc, *_paxTypeFare));
  }

  void testMatchCorpIdAccCodeMUCPassCorpIDMatched()
  {
    setupTestingData();
    MockMarkupControl mc("A");
    // look for "A" in corpIdVec & accCodeVec
    _trx->getRequest()->corporateID() = "A";
    CPPUNIT_ASSERT(_negotiatedFareController->matchCorpIdAccCodeMUC(mc, *_paxTypeFare));
  }

  void testMatchCorpIdAccCodeMUCFailCorpIDNotMatched()
  {
    setupTestingData();
    MockMarkupControl mc("A");
    // look for "A" in corpIdVec & accCodeVec
    _trx->getRequest()->corporateID() = "B";
    CPPUNIT_ASSERT(!_negotiatedFareController->matchCorpIdAccCodeMUC(mc, *_paxTypeFare));
  }

  void testMatchCorpIdAccCodeMUCPassAccCodeMatched()
  {
    setupTestingData();
    MockMarkupControl mc("A");
    // look for "A" in corpIdVec & accCodeVec
    _trx->getRequest()->accountCode() = "A";
    CPPUNIT_ASSERT(_negotiatedFareController->matchCorpIdAccCodeMUC(mc, *_paxTypeFare));
  }

  void testMatchCorpIdAccCodeMUCFailAccCodeNotMatched()
  {
    setupTestingData();
    MockMarkupControl mc("A");
    // look for "A" in corpIdVec & accCodeVec
    _trx->getRequest()->accountCode() = "B";
    CPPUNIT_ASSERT(!_negotiatedFareController->matchCorpIdAccCodeMUC(mc, *_paxTypeFare));
  }

  void testMatchCorpIdAccCodeMUCFailForceCorpFares()
  {
    setupTestingData();
    MockMarkupControl mc(""); // it means return true
    _trx->getRequest()->corporateID() = "";
    _trx->getRequest()->accountCode() = "";
    // unless matchEmptyCorpIdAccCodeMUC fails
    _trx->getOptions()->forceCorpFares() = true;
    CPPUNIT_ASSERT(!_negotiatedFareController->matchCorpIdAccCodeMUC(mc, *_paxTypeFare));
  }

  void testPrepareNewSellingAmtFD()
  {
    // FD & selling fare
    setupTestingData(PricingTrx::FAREDISPLAY_TRX, "ADT", RuleConst::SELLING_FARE, 6, 1, "", 5);
    // ret amt shall be equal ptf.fareAmount - in this case 5
    MoneyAmount retAmt = -1;
    _negotiatedFareController->prepareNewSellingAmt(*_paxTypeFare, retAmt);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5, retAmt, 0.1);
  }

  void testPrepareNewSellingAmtNotFD()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "ADT", RuleConst::SELLING_FARE, 6, 1, "", 5);
    // for SELLING_FARE and for not fd trx ret amt shall be equal 0
    MoneyAmount retAmt = -1;
    _negotiatedFareController->prepareNewSellingAmt(*_paxTypeFare, retAmt);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0, retAmt, 0.1);
  }

  void testPrepareNewSellingAmtNotSelling()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE, 6, 1, "", 5);
    // for not SELLING_FARE don't change ret Amt
    MoneyAmount retAmt = -1;
    _negotiatedFareController->prepareNewSellingAmt(*_paxTypeFare, retAmt);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-1, retAmt, 0.1);
  }

  void testPrepareNewSellingAmtNotNegative()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "ADT", RuleConst::SELLING_FARE, 6, 1, "", 5);
    // for SELLING_FARE don't change positive retAmt
    MoneyAmount retAmt = 3;
    _negotiatedFareController->prepareNewSellingAmt(*_paxTypeFare, retAmt);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(3, retAmt, 0.1);
  }

  void testCreateFaresForFDSellingFare()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "ADT", RuleConst::SELLING_FARE, 6, 1, "", 2.01);

    MoneyAmount newAmtNuc = 0.0;
    Money base(NUC);
    MarkupControl markupRec;

    MoneyAmount newAmt = 1.01;
    // we don't wan't to use original invokeCreateFare
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_overrideInvokeCreateFareAndKeepMinAmt = 1;

    // for selling fare
    // 1, create NET (+2) + selling (+1) + wholesaler (+1) = 5
    _negotiatedFareController->createFaresForFD(
        *_paxTypeFare, newAmt, newAmtNuc, base, *_negFareCalc, *_negFareSecurityInfo, markupRec.tktTag());

    CPPUNIT_ASSERT_EQUAL(5,
                         ((MockNegotiatedFareController*)_negotiatedFareController)
                             ->t_overrideInvokeCreateFareAndKeepMinAmt);
  }

  void testCreateFaresForFDNotSellingFare()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE, 6, 1, "", 2.01);

    MoneyAmount newAmtNuc = 0.0;
    Money base(NUC);
    MarkupControl markupRec;

    MoneyAmount newAmt = 1.01;
    // we don't wan't to use original invokeCreateFare
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_overrideInvokeCreateFareAndKeepMinAmt = 1;
    // for not selling fare
    // 1, don't create NET (+0) + selling (+1) + wholesaler (+2) = 4
    _negotiatedFareController->createFaresForFD(
        *_paxTypeFare, newAmt, newAmtNuc, base, *_negFareCalc, *_negFareSecurityInfo, markupRec.tktTag());

    CPPUNIT_ASSERT_EQUAL(4,
                         ((MockNegotiatedFareController*)_negotiatedFareController)
                             ->t_overrideInvokeCreateFareAndKeepMinAmt);
  }

  void testCreateFaresForFDWholesaler()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE, 6, 1, "", 2.01);

    MoneyAmount newAmtNuc = 0.0;
    Money base(NUC);
    MarkupControl markupRec;
    VendorCode vendor;
    MarkupCalculate mc;
    Indicator viewNetInd = 0;
    PseudoCityCode creatorPCC;

    MoneyAmount newAmt = 1.01;
    // we don't wan't to use original invokeCreateFare
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_overrideInvokeCreateFareAndKeepMinAmt = 1;
    // for not selling fare
    // but wholesaler is on
    mc.wholesalerFareInd() = RuleConst::NF_ADD;
    _negFareCalc->load(vendor, &mc, viewNetInd, creatorPCC);

    // 1, don't create NET (+0) + selling (+1) + wholesaler (+0) = 2
    _negotiatedFareController->createFaresForFD(
        *_paxTypeFare, newAmt, newAmtNuc, base, *_negFareCalc, *_negFareSecurityInfo, markupRec.tktTag());

    CPPUNIT_ASSERT_EQUAL(2,
                         ((MockNegotiatedFareController*)_negotiatedFareController)
                             ->t_overrideInvokeCreateFareAndKeepMinAmt);
  }

  void testCreateFaresForFDFailFare()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE, 6, 1, "", 2.01);

    MoneyAmount newAmt = 1.01;
    MoneyAmount newAmtNuc = 0.0;
    Money base(NUC);
    MarkupControl markupRec;

    // we don't wan't to use original invokeCreateFare
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_overrideInvokeCreateFareAndKeepMinAmt = 1;

    // not selling fare
    // shall be set as not valid if viewNetInd != B
    _negFareCalc->viewNetInd() = 'A';

    _negotiatedFareController->createFaresForFD(
        *_paxTypeFare, newAmt, newAmtNuc, base, *_negFareCalc, *_negFareSecurityInfo, markupRec.tktTag());

    CPPUNIT_ASSERT(!_paxTypeFare->isValid());
  }

  void testProcessNegFareRest_Fail_NullPointer()
  {
    setupTestingData();
    // always return if _negFareRest is NULL
    _negotiatedFareController->_negFareRest = NULL;
    CPPUNIT_ASSERT(!_negotiatedFareController->processNegFareRest(*_paxTypeFare));
  }

  void testProcessNegFareRest_Fail_NotValidated()
  {
    setupTestingData(MockNegFareRest("MIL", 0), PricingTrx::PRICING_TRX, "ADT");
    // by default it Validate will fail
    CPPUNIT_ASSERT(!_negotiatedFareController->processNegFareRest(*_paxTypeFare));
  }

  void testProcessNegFareRest_Fail_T979NotExistsNet()
  {
    // T979 not exist & fare is NET
    setupTestingData(
        MockNegFareRest("ADT", 0), PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    // validate will pass
    ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideValidate = true;
    CPPUNIT_ASSERT(!_negotiatedFareController->processNegFareRest(*_paxTypeFare));
  }

  void testProcessNegFareRest_Fail_T979NotExistsSelling()
  {
    // this is Axess and NET is requested and fare is NET
    setupTestingData(true, true, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    // T979 not exist
    MockNegFareRest nfr("ADT", 0);
    _negotiatedFareController->_negFareRest = &nfr;
    // validate will pass
    ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideValidate = true;
    CPPUNIT_ASSERT(!_negotiatedFareController->processNegFareRest(*_paxTypeFare));
  }

  void testProcessNegFareRest_Pass()
  {
    // this is Axess and NET is requested and fare is NET
    setupTestingData(true, true, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    // T979 exists
    MockNegFareRest nfr("ADT", 1);
    _negotiatedFareController->_negFareRest = &nfr;
    // validate will pass
    ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideValidate = true;
    CPPUNIT_ASSERT(_negotiatedFareController->processNegFareRest(*_paxTypeFare));
  }

  void testProcessCategoryRuleItemInfo_Fail_NullPointer()
  {
    setupTestingData();
    // always return if _catRuleItemInfo is NULL
    _negotiatedFareController->_catRuleItemInfo = NULL;
    CPPUNIT_ASSERT(!_negotiatedFareController->processCategoryRuleItemInfo(*_paxTypeFare, false));
  }

  void testProcessCategoryRuleItemInfo_Fail_ReationalIndIf()
  {
    setupTestingData();
    auto catRuleInfo = dummyCRII(CategoryRuleItemInfo::IF); // fail in that case
    _negotiatedFareController->_catRuleItemInfo = &catRuleInfo;
    CPPUNIT_ASSERT(!_negotiatedFareController->processCategoryRuleItemInfo(*_paxTypeFare, false));
  }

  void testProcessCategoryRuleItemInfo_Fail_ReationalIndAnd()
  {
    setupTestingData();
    auto catRuleInfo = dummyCRII(CategoryRuleItemInfo::AND); // fail in that case
    _negotiatedFareController->_catRuleItemInfo = &catRuleInfo;
    CPPUNIT_ASSERT(!_negotiatedFareController->processCategoryRuleItemInfo(*_paxTypeFare, false));
  }

  void testProcessCategoryRuleItemInfo_Fail_DirectionCode()
  {
    setupTestingData();
    // this will fail Directioanilty if last parameter is true
    auto catRuleInfo = dummyCRII(0, RuleConst::FROM_LOC1_TO_LOC2);
    _negotiatedFareController->_catRuleItemInfo = &catRuleInfo;
    CPPUNIT_ASSERT(!_negotiatedFareController->processCategoryRuleItemInfo(*_paxTypeFare, true));
  }

  void testProcessCategoryRuleItemInfo_Fail_FdDirectionCode()
  {
    // trx is FD
    setupTestingData(PricingTrx::FAREDISPLAY_TRX);
    // this will fail Directioanilty if last parameter is true
    auto catRuleInfo = dummyCRII(0, RuleConst::FROM_LOC1_TO_LOC2);
    _negotiatedFareController->_catRuleItemInfo = &catRuleInfo;
    CPPUNIT_ASSERT(!_negotiatedFareController->processCategoryRuleItemInfo(*_paxTypeFare, true));
  }

  void testProcessCategoryRuleItemInfo_Pass()
  {
    setupTestingData();
    // this will pass Directioanilty if last parameter is false
    auto catRuleInfo = dummyCRII(CategoryRuleItemInfo::ELSE,
                                 RuleConst::FROM_LOC1_TO_LOC2,
                                 RuleConst::ALWAYS_APPLIES);
    _negotiatedFareController->_catRuleItemInfo = &catRuleInfo;
    CPPUNIT_ASSERT(_negotiatedFareController->processCategoryRuleItemInfo(*_paxTypeFare, false));
  }

  void testCheckDirectionalityAndPaxType_Pass_DirFrom1To2()
  {
    setupTestingData();
    // this condition forces returning true.
    auto catInfo = dummyCRII(0, RuleConst::ORIGIN_FROM_LOC1_TO_LOC2);
    _negotiatedFareController->_catRuleItemInfo = &catInfo;
    CPPUNIT_ASSERT(_negotiatedFareController->checkDirectionalityAndPaxType(CHILD));
  }

  void testCheckDirectionalityAndPaxType_Pass_DirFrom2To1()
  {
    setupTestingData();
    // this condition forces returning true.
    auto catInfo = dummyCRII(0, RuleConst::ORIGIN_FROM_LOC2_TO_LOC1);
    _negotiatedFareController->_catRuleItemInfo = &catInfo;
    CPPUNIT_ASSERT(_negotiatedFareController->checkDirectionalityAndPaxType(CHILD));
  }

  void testCheckDirectionalityAndPaxType_Pass_NotAlwayAppl()
  {
    setupTestingData();
    // directionality fails, but inOutInd Pass
    auto catInfo = dummyCRII(0, RuleConst::ALWAYS_APPLIES, RuleConst::ORIGIN_FROM_LOC2_TO_LOC1);
    _negotiatedFareController->_catRuleItemInfo = &catInfo;
    CPPUNIT_ASSERT(_negotiatedFareController->checkDirectionalityAndPaxType(CHILD));
  }

  void testCheckDirectionalityAndPaxType_Pass_PaxEmpty()
  {
    setupTestingData();
    // directionality & inOutInd fail
    auto catInfo = dummyCRII(CategoryRuleItemInfo::THEN, RuleConst::ALWAYS_APPLIES, RuleConst::ALWAYS_APPLIES);
    _negotiatedFareController->_catRuleItemInfo = &catInfo;
    // but pax type is empty
    CPPUNIT_ASSERT(_negotiatedFareController->checkDirectionalityAndPaxType(PaxTypeCode("")));
  }

  void testCheckDirectionalityAndPaxType_Pass_PaxAdult()
  {
    setupTestingData();
    // directionality & inOutInd fail
    auto catInfo = dummyCRII(0, RuleConst::ALWAYS_APPLIES, RuleConst::ALWAYS_APPLIES);
    _negotiatedFareController->_catRuleItemInfo = &catInfo;
    // but pax type is ADULT
    CPPUNIT_ASSERT(_negotiatedFareController->checkDirectionalityAndPaxType(ADULT));
  }

  void testCheckDirectionalityAndPaxType_Fail()
  {
    setupTestingData();
    // directionality & inOutInd fail
    auto catInfo = dummyCRII(0, RuleConst::ALWAYS_APPLIES, RuleConst::ALWAYS_APPLIES);
    _negotiatedFareController->_catRuleItemInfo = &catInfo;
    // and pax type also
    CPPUNIT_ASSERT(!_negotiatedFareController->checkDirectionalityAndPaxType(CHILD));
  }

  void testCheckGeneralFareRuleInfo_Fail_Empty()
  {
    setupTestingData();
    CategoryRuleItemInfo catInfo;
    _negotiatedFareController->_catRuleItemInfo = &catInfo;
    // this std::vector is empty so fail
    GeneralFareRuleInfoVec gfrInfoVec;
    CPPUNIT_ASSERT(!_negotiatedFareController->checkGeneralFareRuleInfo(*_paxTypeFare, gfrInfoVec));
  }

  void testCheckGeneralFareRuleInfo_Fail_EmptyFD()
  {
    setupTestingData(PricingTrx::FAREDISPLAY_TRX);
    // this vector is empty so fail
    GeneralFareRuleInfoVec gfrInfoVec;
    // so for FD fare shall be failed also
    _negotiatedFareController->checkGeneralFareRuleInfo(*_paxTypeFare, gfrInfoVec);
    CPPUNIT_ASSERT(!_paxTypeFare->isValid());
  }

  void testCheckGeneralFareRuleInfo_Pass_Empty()
  {
    setupTestingData();
    GeneralFareRuleInfoVec gfrInfoVec;
    std::pair<const tse::GeneralFareRuleInfo*, bool> element;
    // this vector is not empty so pass
    gfrInfoVec.push_back(element);
    CPPUNIT_ASSERT(_negotiatedFareController->checkGeneralFareRuleInfo(*_paxTypeFare, gfrInfoVec));
  }

  void testCheckNotNetFareForAxess_Pass_NotAxess()
  {
    // this is not axess
    setupTestingData(false, false);
    CPPUNIT_ASSERT(_negotiatedFareController->checkNotNetFareForAxess(*_paxTypeFare));
  }

  void testCheckNotNetFareForAxess_Pass_AxessNet()
  {
    // this is axess but Axess NET
    setupTestingData(true, true);
    CPPUNIT_ASSERT(_negotiatedFareController->checkNotNetFareForAxess(*_paxTypeFare));
  }

  void testCheckNotNetFareForAxess_Pass_FD()
  {
    // this is axess and not Axess NET but FD
    setupTestingData(true, false, PricingTrx::FAREDISPLAY_TRX);
    CPPUNIT_ASSERT(_negotiatedFareController->checkNotNetFareForAxess(*_paxTypeFare));
  }

  void testCheckNotNetFareForAxess_Pass_NotNet()
  {
    // this is axess and not Axess NET but fare is not NET_SUBMIT_FARE_UPD
    setupTestingData(true, false, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    CPPUNIT_ASSERT(_negotiatedFareController->checkNotNetFareForAxess(*_paxTypeFare));
  }

  void testCheckNotNetFareForAxess_Fail()
  {
    // this is axess and not Axess NET and fare is NET_SUBMIT_FARE_UPD
    setupTestingData(true, false, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD);
    CPPUNIT_ASSERT(!_negotiatedFareController->checkNotNetFareForAxess(*_paxTypeFare));
  }

  void testProcessCurrentRule_PrevAgent()
  {
    setupTestingData();
    // create fare for prev agent
    _negotiatedFareController->_rexCreateFaresForPrevAgent = true;
    // we don't want to use original processRule
    ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideProcessRule = 1;

    _negotiatedFareController->processCurrentRule(*_paxTypeFare, false);
    // We executed processRule twice (1+2)
    CPPUNIT_ASSERT_EQUAL(
        3, ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideProcessRule);
  }

  void testProcessCurrentRule_NotPrevAgent()
  {
    setupTestingData();
    // do not create fare for prev agent
    _negotiatedFareController->_rexCreateFaresForPrevAgent = false;
    // we don't want to use original processRule
    ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideProcessRule = 1;

    _negotiatedFareController->processCurrentRule(*_paxTypeFare, false);
    // We executed processRule twice (1+1)
    CPPUNIT_ASSERT_EQUAL(
        2, ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideProcessRule);
  }

  void testProcessCurrentRule_Exception()
  {
    setupTestingData();
    // create fare for prev agent
    _negotiatedFareController->_rexCreateFaresForPrevAgent = true;
    // we don't want to use original processRule
    ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideProcessRule = 1;

    // last parameter true causes throwing an exception in processRule
    _negotiatedFareController->processCurrentRule(*_paxTypeFare, true);
    // if exception was thrown & catched fare is invalid
    CPPUNIT_ASSERT(!_paxTypeFare->isValid());
  }

  void testProcessFare_FailOnCheckNotNetFareForAxess()
  {
    // this is axess and not Axess NET to fail on checkNotNetFareForAxess method
    setupTestingData(true, false);
    // we don't want to use original processRule
    ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideProcessRule = 1;

    _negotiatedFareController->processFare(*_paxTypeFare);
    // We didn't execute processRule (1 + 0)
    CPPUNIT_ASSERT_EQUAL(
        1, ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideProcessRule);
  }

  void testProcessFare_FailOnCheckGeneralFareRuleInfo()
  {
    // this is not axess
    setupTestingData(false, false);
    // empty vector causes failing on getGeneralFareRuleInfo.
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_getGeneralFareRuleInfoElement.clear();
    // we don't want to use original processRule
    ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideProcessRule = 1;

    _negotiatedFareController->processFare(*_paxTypeFare);
    // We didn't execute processRule (1 + 0)
    CPPUNIT_ASSERT_EQUAL(
        1, ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideProcessRule);
  }

  void testProcessFare_FailOnApply()
  {
    // this is not axess
    setupTestingData(false, false);
    std::pair<const tse::GeneralFareRuleInfo*, bool> element;
    element.first = _generalFareRuleInfo;
    // condition for failing
    _generalFareRuleInfo->applInd() = RuleConst::STRING_DOES_NOT_APPLY;
    // fill GeneralFareRuleInfo vector
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_getGeneralFareRuleInfoElement.push_back(element);
    // we don't want to use original processRule
    ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideProcessRule = 1;

    _negotiatedFareController->processFare(*_paxTypeFare);
    // We didn't execute processRule (1 + 0)
    CPPUNIT_ASSERT_EQUAL(
        1, ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideProcessRule);
  }

  void testProcessFare_FailOnDates()
  {
    // this is not axess
    setupTestingData(false, false);
    std::pair<const tse::GeneralFareRuleInfo*, bool> element;
    element.first = _generalFareRuleInfo;
    _generalFareRuleInfo->applInd() = RuleConst::ALWAYS_APPLIES;
    DateTime date1(2009, 06, 30);
    DateTime date2(2010, 06, 30);
    // condition for failing after 1-st element
    _trx->ticketingDate() = date2;
    _generalFareRuleInfo->createDate() = date1;
    // fill GeneralFareRuleInfo vector by 2 elements
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_getGeneralFareRuleInfoElement.push_back(element);
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_getGeneralFareRuleInfoElement.push_back(element);
    // we don't want to use original processRule
    ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideProcessRule = 1;

    _negotiatedFareController->processFare(*_paxTypeFare);
    // We executed processRule once (1 + 1) - second element was skipped
    CPPUNIT_ASSERT_EQUAL(
        2, ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideProcessRule);
  }

  void testSetNationFrance_Fail_LocType()
  {
    setupTestingData();
    LocKey loc1, loc2;
    // fail reason
    loc1.locType() = LOCTYPE_NONE;
    loc2.locType() = LOCTYPE_NONE;

    _negotiatedFareController->setNationFrance(loc1, loc2);
    CPPUNIT_ASSERT(!_negotiatedFareController->_nationFranceLocalInd);
  }

  void testSetNationFrance_Fail_Loc()
  {
    setupTestingData();
    LocKey loc1, loc2;
    loc1.locType() = LOCTYPE_NATION;
    loc2.locType() = LOCTYPE_NATION;
    // fail reason
    loc1.loc() = NATION_US;
    loc2.loc() = NATION_US;

    _negotiatedFareController->setNationFrance(loc1, loc2);
    CPPUNIT_ASSERT(!_negotiatedFareController->_nationFranceLocalInd);
  }

  void testSetNationFrance_Pass_Loc1()
  {
    setupTestingData();
    LocKey loc1, loc2;
    loc1.locType() = LOCTYPE_NATION;
    loc2.locType() = LOCTYPE_NATION;
    loc1.loc() = NATION_FRANCE;
    loc2.loc() = NATION_US;

    _negotiatedFareController->setNationFrance(loc1, loc2);
    CPPUNIT_ASSERT(_negotiatedFareController->_nationFranceLocalInd);
  }

  void testSetNationFrance_Pass_Loc2()
  {
    setupTestingData();
    LocKey loc1, loc2;
    loc1.locType() = LOCTYPE_NATION;
    loc2.locType() = LOCTYPE_NATION;
    // fail reason
    loc1.loc() = NATION_US;
    loc2.loc() = NATION_FRANCE;

    _negotiatedFareController->setNationFrance(loc1, loc2);
    CPPUNIT_ASSERT(_negotiatedFareController->_nationFranceLocalInd);
  }

  void testcheckTktAuthorityAndUpdFlag_Pass()
  {
    setupTestingData();
    NegFareSecurity negFareSecurity(_negFareSecurityInfo);
    negFareSecurity.ticketInd() = 'Y';
    _negFareSecurityInfo->updateInd() = 'N';
    CPPUNIT_ASSERT(_negotiatedFareController->checkTktAuthorityAndUpdFlag(
        negFareSecurity, _negFareSecurityInfo, 'Y', *_paxTypeFare));
  }

  void testcheckTktAuthorityAndUpdFlag_Fail_TktAuthiority()
  {
    setupTestingData();
    NegFareSecurity negFareSecurity(_negFareSecurityInfo);
    negFareSecurity.ticketInd() = 'N';
    _negFareSecurityInfo->updateInd() = 'N';
    CPPUNIT_ASSERT(!_negotiatedFareController->checkTktAuthorityAndUpdFlag(
        negFareSecurity, _negFareSecurityInfo, 'Y', *_paxTypeFare));
  }

  void testcheckTktAuthorityAndUpdFlag_Fail_UpdFlag()
  {
    setupTestingData();
    NegFareSecurity negFareSecurity(_negFareSecurityInfo);
    negFareSecurity.ticketInd() = 'Y';
    _negFareSecurityInfo->updateInd() = 'Y';
    CPPUNIT_ASSERT(!_negotiatedFareController->checkTktAuthorityAndUpdFlag(
        negFareSecurity, _negFareSecurityInfo, 'Y', *_paxTypeFare));
  }

  void testcheckUpdFlag_Fail_Net()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    _negFareSecurityInfo->updateInd() = 'Y';
    CPPUNIT_ASSERT(!_negotiatedFareController->checkUpdFlag(_negFareSecurityInfo, *_paxTypeFare));
  }

  void testcheckUpdFlag_Pass_NetUpd()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD);
    _negFareSecurityInfo->updateInd() = 'Y';
    CPPUNIT_ASSERT(_negotiatedFareController->checkUpdFlag(_negFareSecurityInfo, *_paxTypeFare));
  }

  void testcheckUpdFlag_Pass_Net()
  {
    setupTestingData(PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    _negFareSecurityInfo->updateInd() = 'N';
    CPPUNIT_ASSERT(_negotiatedFareController->checkUpdFlag(_negFareSecurityInfo, *_paxTypeFare));
  }

  void testIsJalAxessTypeC_Fail_FD()
  {
    setupTestingData(PricingTrx::FAREDISPLAY_TRX);
    CPPUNIT_ASSERT(!_negotiatedFareController->isJalAxessTypeC(*_paxTypeFare));
  }

  void testIsJalAxessTypeC_Fail_Axess()
  {
    setupTestingData(false, false, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD);
    CPPUNIT_ASSERT(!_negotiatedFareController->isJalAxessTypeC(*_paxTypeFare));
  }

  void testIsJalAxessTypeC_Fail_NetUpd()
  {
    setupTestingData(true, false, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->isJalAxessTypeC(*_paxTypeFare));
  }

  void testIsJalAxessTypeC_Pass()
  {
    setupTestingData(true, false, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD);
    CPPUNIT_ASSERT(_negotiatedFareController->isJalAxessTypeC(*_paxTypeFare));
  }

  void testProcessCalcObj_Pass()
  {
    // necessary parameters
    bool isPosMatch;
    bool is979queried;
    int32_t seqNegMatch;
    Money base(NUC);

    setupTestingData();
    NegFareSecurity negFareSecurity(_negFareSecurityInfo);
    negFareSecurity.applInd() = RuleConst::REQUIRED;

    CPPUNIT_ASSERT(_negotiatedFareController->processCalcObj(isPosMatch,
                                                             is979queried,
                                                             seqNegMatch,
                                                             *_paxTypeFare,
                                                             *_negFareCalc,
                                                             _negFareSecurityInfo,
                                                             _negPaxTypeFareRuleData,
                                                             base,
                                                             base.value(),
                                                             base.value(),
                                                             negFareSecurity,
                                                             'Y'));
  }

  void testProcessCalcObj_Fail()
  {
    // necessary parameters
    bool isPosMatch;
    bool is979queried;
    int32_t seqNegMatch;
    Money base(NUC);

    setupTestingData();
    NegFareSecurity negFareSecurity(_negFareSecurityInfo);
    negFareSecurity.applInd() = RuleConst::NOT_ALLOWED;

    CPPUNIT_ASSERT(!_negotiatedFareController->processCalcObj(isPosMatch,
                                                              is979queried,
                                                              seqNegMatch,
                                                              *_paxTypeFare,
                                                              *_negFareCalc,
                                                              _negFareSecurityInfo,
                                                              _negPaxTypeFareRuleData,
                                                              base,
                                                              base.value(),
                                                              base.value(),
                                                              negFareSecurity,
                                                              'Y'));
  }

  void testProcessCalcObj_TktAuthotity()
  {
    // necessary parameters
    bool isPosMatch;
    bool is979queried = false;
    int32_t seqNegMatch;
    Money base(NUC);

    setupTestingData();
    NegFareSecurity negFareSecurity(_negFareSecurityInfo);
    negFareSecurity.applInd() = RuleConst::REQUIRED;
    // this will fail TktAuth
    negFareSecurity.ticketInd() = 'N';
    _negFareSecurityInfo->updateInd() = 'N';

    _negotiatedFareController->processCalcObj(isPosMatch,
                                              is979queried,
                                              seqNegMatch,
                                              *_paxTypeFare,
                                              *_negFareCalc,
                                              _negFareSecurityInfo,
                                              _negPaxTypeFareRuleData,
                                              base,
                                              base.value(),
                                              base.value(),
                                              negFareSecurity,
                                              'Y');

    CPPUNIT_ASSERT(!is979queried);
  }

  void testProcessCalcObj_JalAxessTypeC()
  {
    // necessary parameters
    bool isPosMatch;
    bool is979queried = false;
    int32_t seqNegMatch;
    Money base(NUC);

    // is JalAxess C
    setupTestingData(true, false, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD);
    NegFareSecurity negFareSecurity(_negFareSecurityInfo);
    negFareSecurity.applInd() = RuleConst::REQUIRED;
    negFareSecurity.ticketInd() = 'Y';
    _negFareSecurityInfo->updateInd() = 'Y';

    _negotiatedFareController->processCalcObj(isPosMatch,
                                              is979queried,
                                              seqNegMatch,
                                              *_paxTypeFare,
                                              *_negFareCalc,
                                              _negFareSecurityInfo,
                                              _negPaxTypeFareRuleData,
                                              base,
                                              base.value(),
                                              base.value(),
                                              negFareSecurity,
                                              'Y');

    CPPUNIT_ASSERT(!is979queried);
  }

  void testProcessCalcObj_TblItemInfoZero()
  {
    // necessary parameters
    bool isPosMatch;
    bool is979queried = false;
    int32_t seqNegMatch;
    Money base(NUC);

    // TblItemInfoZero
    setupTestingData(MockNegFareRest("ADT", 0));
    NegFareSecurity negFareSecurity(_negFareSecurityInfo);
    negFareSecurity.applInd() = RuleConst::REQUIRED;
    negFareSecurity.ticketInd() = 'Y';
    _negFareSecurityInfo->updateInd() = 'N';

    _negotiatedFareController->processCalcObj(isPosMatch,
                                              is979queried,
                                              seqNegMatch,
                                              *_paxTypeFare,
                                              *_negFareCalc,
                                              _negFareSecurityInfo,
                                              _negPaxTypeFareRuleData,
                                              base,
                                              base.value(),
                                              base.value(),
                                              negFareSecurity,
                                              'Y');
    CPPUNIT_ASSERT(is979queried);
  }

  void testProcessCalcObj_TblItemInfoNotZero()
  {
    // necessary parameters
    bool isPosMatch;
    bool is979queried = false;
    int32_t seqNegMatch;
    Money base(5, NUC);

    // TblItemInfoZero
    setupTestingData(MockNegFareRest("ADT", 1));
    NegFareSecurity negFareSecurity(_negFareSecurityInfo);
    _negFareSecurityInfo->updateInd() = 'N';
    negFareSecurity.applInd() = RuleConst::REQUIRED;
    negFareSecurity.ticketInd() = 'Y';
    // do not use original createFaresFromCalcObj
    ((MockNegotiatedFareController*)_negotiatedFareController)->t_createFaresFromCalcObj = true;

    _negotiatedFareController->processCalcObj(isPosMatch,
                                              is979queried,
                                              seqNegMatch,
                                              *_paxTypeFare,
                                              *_negFareCalc,
                                              _negFareSecurityInfo,
                                              _negPaxTypeFareRuleData,
                                              base,
                                              base.value(),
                                              base.value(),
                                              negFareSecurity,
                                              'Y');
    CPPUNIT_ASSERT(is979queried);
  }

  void testProcessCalcObj_CreateFaresFromCalcObj()
  {
    // necessary parameters
    bool isPosMatch;
    bool is979queried = false;
    int32_t seqNegMatch;
    Money base(-5, NUC);

    setupTestingData(MockNegFareRest("ADT", 1));
    NegFareSecurity negFareSecurity(_negFareSecurityInfo);
    negFareSecurity.applInd() = RuleConst::REQUIRED;
    negFareSecurity.ticketInd() = 'Y';
    _negFareSecurityInfo->updateInd() = 'N';
    // do not use original createFaresFromCalcObj
    ((MockNegotiatedFareController*)_negotiatedFareController)->t_createFaresFromCalcObj = true;

    _negotiatedFareController->processCalcObj(isPosMatch,
                                              is979queried,
                                              seqNegMatch,
                                              *_paxTypeFare,
                                              *_negFareCalc,
                                              _negFareSecurityInfo,
                                              _negPaxTypeFareRuleData,
                                              base,
                                              base.value(),
                                              base.value(),
                                              negFareSecurity,
                                              'Y');
    CPPUNIT_ASSERT(!is979queried);
  }

  void testCreateFaresFromCalcObj_Pass_GetOneCalc()
  {
    // conditions will pass getOneCalc
    setupTestingData(MockNegFareRest("ADT", 0));
    Money base(NUC);
    _negFareSecurityInfo->updateInd() = 'N';

    // we don't want to use original methods
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_overrideInvokeCreateFareAndKeepMinAmt = 1;

    CPPUNIT_ASSERT(_negotiatedFareController->createFaresFromCalcObj(*_paxTypeFare,
                                                                     *_negFareCalc,
                                                                     _negFareSecurityInfo,
                                                                     _negPaxTypeFareRuleData,
                                                                     base,
                                                                     base.value(),
                                                                     base.value(),
                                                                     'Y'));
  }

  void testCreateFaresFromCalcObj_Fail_FD()
  {
    setupTestingData(MockNegFareRest("ADT", 1)); // is not FD so Fail
    Money base(NUC);

    // we don't want to use original methods
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_overrideInvokeCreateFareAndKeepMinAmt = 1;

    CPPUNIT_ASSERT(!_negotiatedFareController->createFaresFromCalcObj(*_paxTypeFare,
                                                                      *_negFareCalc,
                                                                      _negFareSecurityInfo,
                                                                      _negPaxTypeFareRuleData,
                                                                      base,
                                                                      base.value(),
                                                                      base.value(),
                                                                      'Y'));
  }

  void testCreateFaresFromCalcObj_Fail_UpdateInd()
  {
    setupTestingData(MockNegFareRest("ADT", 1), PricingTrx::FAREDISPLAY_TRX);
    Money base(NUC);
    _negFareSecurityInfo->updateInd() = 'N'; // reason of failing

    // we don't want to use original methods
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_overrideInvokeCreateFareAndKeepMinAmt = 1;

    CPPUNIT_ASSERT(!_negotiatedFareController->createFaresFromCalcObj(*_paxTypeFare,
                                                                      *_negFareCalc,
                                                                      _negFareSecurityInfo,
                                                                      _negPaxTypeFareRuleData,
                                                                      base,
                                                                      base.value(),
                                                                      base.value(),
                                                                      'Y'));
  }

  void testCreateFaresFromCalcObj_Fail_Net()
  {
    setupTestingData(MockNegFareRest("ADT", 1),
                     PricingTrx::FAREDISPLAY_TRX,
                     "ADT",
                     RuleConst::SELLING_FARE); // fare is not NET
    Money base(NUC);
    _negFareSecurityInfo->updateInd() = 'N';

    // we don't want to use original methods
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_overrideInvokeCreateFareAndKeepMinAmt = 1;

    CPPUNIT_ASSERT(!_negotiatedFareController->createFaresFromCalcObj(*_paxTypeFare,
                                                                      *_negFareCalc,
                                                                      _negFareSecurityInfo,
                                                                      _negPaxTypeFareRuleData,
                                                                      base,
                                                                      base.value(),
                                                                      base.value(),
                                                                      'Y'));
  }

  void testCreateFaresFromCalcObj_Pass_DoNotIgnoreCType()
  {
    setupTestingData(MockNegFareRest("ADT", 0),
                     PricingTrx::FAREDISPLAY_TRX,
                     "ADT",
                     RuleConst::NET_SUBMIT_FARE_UPD);
    Money base(3, NUC);
    _negFareSecurityInfo->updateInd() = 'Y';

    // we don't want to use original methods
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_overrideInvokeCreateFareAndKeepMinAmt = 1;

    CPPUNIT_ASSERT(_negotiatedFareController->createFaresFromCalcObj(*_paxTypeFare,
                                                                     *_negFareCalc,
                                                                     _negFareSecurityInfo,
                                                                     _negPaxTypeFareRuleData,
                                                                     base,
                                                                     base.value(),
                                                                     base.value(),
                                                                     'Y'));
    // 4 = 1 + invoke(3) + saveCandidate(0)
    CPPUNIT_ASSERT_EQUAL(4,
                         ((MockNegotiatedFareController*)_negotiatedFareController)
                             ->t_overrideInvokeCreateFareAndKeepMinAmt);
  }

  void testCreateFaresFromCalcObj_Selling()
  {
    setupTestingData(MockNegFareRest("ADT", 0),
                     PricingTrx::FAREDISPLAY_TRX,
                     "ADT",
                     RuleConst::SELLING_FARE,
                     6,
                     1,
                     "",
                     2.01);
    Money base(3, NUC);
    _negFareSecurityInfo->updateInd() = 'N';

    // we don't want to use original methods
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_overrideInvokeCreateFareAndKeepMinAmt = 1;

    _negotiatedFareController->createFaresFromCalcObj(*_paxTypeFare,
                                                      *_negFareCalc,
                                                      _negFareSecurityInfo,
                                                      _negPaxTypeFareRuleData,
                                                      base,
                                                      base.value(),
                                                      base.value(),
                                                      'Y');

    // 3 = 1 + saveCandidate(0)
    CPPUNIT_ASSERT_EQUAL(1,
                         ((MockNegotiatedFareController*)_negotiatedFareController)
                             ->t_overrideInvokeCreateFareAndKeepMinAmt);
  }

  void testProcessSecurityMatch_Fail_SecList()
  {
    bool isPosMatch;
    bool is979queried;
    Money base(NUC);
    int32_t seqNegMatch;

    setupTestingData();
    ((MockNegotiatedFareController*)_negotiatedFareController)->clearNegFareSecurityInfo();

    CPPUNIT_ASSERT(!_negotiatedFareController->processSecurityMatch(isPosMatch,
                                                                    is979queried,
                                                                    base.value(),
                                                                    base.value(),
                                                                    base,
                                                                    _negPaxTypeFareRuleData,
                                                                    *_paxTypeFare,
                                                                    seqNegMatch,
                                                                    *_negFareCalc));
  }

  void testProcessSecurityMatch_Pass_SecRec()
  {
    bool isPosMatch = false;
    bool is979queried;
    Money base(NUC);
    int32_t seqNegMatch;

    setupTestingData();
    ((MockNegotiatedFareController*)_negotiatedFareController)->clearNegFareSecurityInfo();
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_getNegFareSecurity.push_back(NULL);

    CPPUNIT_ASSERT(_negotiatedFareController->processSecurityMatch(isPosMatch,
                                                                   is979queried,
                                                                   base.value(),
                                                                   base.value(),
                                                                   base,
                                                                   _negPaxTypeFareRuleData,
                                                                   *_paxTypeFare,
                                                                   seqNegMatch,
                                                                   *_negFareCalc));
  }

  void testProcessSecurityMatch_Pass_IsMatch()
  {
    bool isPosMatch;
    bool is979queried;
    Money base(NUC);
    int32_t seqNegMatch;

    setupTestingData();

    CPPUNIT_ASSERT(_negotiatedFareController->processSecurityMatch(isPosMatch,
                                                                   is979queried,
                                                                   base.value(),
                                                                   base.value(),
                                                                   base,
                                                                   _negPaxTypeFareRuleData,
                                                                   *_paxTypeFare,
                                                                   seqNegMatch,
                                                                   *_negFareCalc));
  }

  void testProcessSecurityMatch_Fail_ProcessCalcObj()
  {
    bool isPosMatch;
    bool is979queried;
    Money base(NUC);
    int32_t seqNegMatch;

    setupTestingData(true, false);
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_getNegFareSecurity.front()
        ->localeType() = RuleConst::BLANK;
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_getNegFareSecurity.front()
        ->applInd() = RuleConst::NOT_ALLOWED;

    CPPUNIT_ASSERT(!_negotiatedFareController->processSecurityMatch(isPosMatch,
                                                                    is979queried,
                                                                    base.value(),
                                                                    base.value(),
                                                                    base,
                                                                    _negPaxTypeFareRuleData,
                                                                    *_paxTypeFare,
                                                                    seqNegMatch,
                                                                    *_negFareCalc));
  }

  void testProcessSecurityMatch_Pass_ProcessCalcObj()
  {
    bool isPosMatch;
    bool is979queried;
    Money base(NUC);
    int32_t seqNegMatch;

    setupTestingData(true, false);
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_getNegFareSecurity.front()
        ->localeType() = RuleConst::BLANK;
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_getNegFareSecurity.front()
        ->applInd() = RuleConst::REQUIRED;

    CPPUNIT_ASSERT(_negotiatedFareController->processSecurityMatch(isPosMatch,
                                                                   is979queried,
                                                                   base.value(),
                                                                   base.value(),
                                                                   base,
                                                                   _negPaxTypeFareRuleData,
                                                                   *_paxTypeFare,
                                                                   seqNegMatch,
                                                                   *_negFareCalc));
  }

  void testGet979_Pass_ItemCalc()
  {
    setupTestingData(MockNegFareRest("ADT", 0));
    CPPUNIT_ASSERT(_negotiatedFareController->get979(*_paxTypeFare, false, *_negFareCalc));
  }

  void testGet979_Fail_CalcRec()
  {
    setupTestingData(MockNegFareRest("ADT", 1));
    ((MockNegotiatedFareController*)_negotiatedFareController)->t_getNegFareCalc.push_back(NULL);
    CPPUNIT_ASSERT(!_negotiatedFareController->get979(*_paxTypeFare, false, *_negFareCalc));
  }

  void testGet979_Fail_Net()
  {
    setupTestingData(
        MockNegFareRest("ADT", 1), PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    NegFareCalcInfo nfcInfo;
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_getNegFareCalc.push_back(&nfcInfo);
    CPPUNIT_ASSERT(!_negotiatedFareController->get979(*_paxTypeFare, false, *_negFareCalc));
  }

  void testGet979_Fail_IsValidCat()
  {
    setupTestingData(MockNegFareRest("ADT", 1));
    MockNegFareCalcInfo nfcInfo;
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_getNegFareCalc.push_back(&nfcInfo);
    CPPUNIT_ASSERT(!_negotiatedFareController->get979(*_paxTypeFare, true, *_negFareCalc));
  }

  void testGet979_Pass_HaveCalc()
  {
    setupTestingData(MockNegFareRest("ADT", 1));
    MockNegFareCalcInfo nfcInfo;
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_getNegFareCalc.push_back(&nfcInfo);
    CPPUNIT_ASSERT(_negotiatedFareController->get979(*_paxTypeFare, false, *_negFareCalc));
  }

  void testCheckBetwAndCities_Fail_BetwCity1()
  {
    setupTestingData(MockNegFareRest("", "BBB", "CCC", "DDD"));
    CPPUNIT_ASSERT(!_negotiatedFareController->checkBetwAndCities());
  }

  void testCheckBetwAndCities_Fail_BetwCity2()
  {
    setupTestingData(MockNegFareRest("AAA", "", "CCC", "DDD"));
    CPPUNIT_ASSERT(!_negotiatedFareController->checkBetwAndCities());
  }

  void testCheckBetwAndCities_Fail_AndCity1()
  {
    setupTestingData(MockNegFareRest("AAA", "BBB", "", "DDD"));
    CPPUNIT_ASSERT(!_negotiatedFareController->checkBetwAndCities());
  }

  void testCheckBetwAndCities_Fail_AndCity2()
  {
    setupTestingData(MockNegFareRest("AAA", "BBB", "CCC", ""));
    CPPUNIT_ASSERT(!_negotiatedFareController->checkBetwAndCities());
  }

  void testCheckBetwAndCities_Pass()
  {
    setupTestingData(MockNegFareRest("AAA", "BBB", "CCC", "DDD"));
    CPPUNIT_ASSERT(_negotiatedFareController->checkBetwAndCities());
  }

  void testValidateCat35SegData_Pass_NotReccuring()
  {
    setupTestingData(MockNegFareRest("ADT"));
    CPPUNIT_ASSERT(_negotiatedFareController->validateCat35SegData(false));
  }

  void testValidateCat35SegData_Fail_TicketedFareData()
  {
    setupTestingData(MockNegFareRest("AAA", "BBB", "CCC", "DDD"));
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35SegData(false));
  }

  void testValidateCat35SegData_Fail_BetwAndCities()
  {
    setupTestingData(MockNegFareRest("AAA", "BBB", "CCC", ""));
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35SegData(true));
  }

  void testValidateCat35SegData_Fail_FareDataInd()
  {
    setupTestingData(MockNegFareRest("AAA", "BBB", "CCC", "DDD", 'Y'));
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35SegData(true));
  }

  void testValidateCat35SegData_Fail_Segments()
  {
    setupTestingData(MockNegFareRest("AAA", "BBB", "CCC", "DDD"));
    _itin->travelSeg().clear();
    CPPUNIT_ASSERT(!_negotiatedFareController->validateCat35SegData(true));
  }

  void testValidateCat35SegData_Pass_Reccuring()
  {
    setupTestingData(MockNegFareRest("AAA", "BBB", "CCC", "DDD"));
    _itin->travelSeg().push_back(NULL); // travelseg has to have 4 segs
    _itin->travelSeg().push_back(NULL);
    _itin->travelSeg().push_back(NULL);
    _itin->travelSeg().push_back(NULL);
    _fareMarket->travelSeg().push_back(NULL); // fareMarket has to have 2 segs
    CPPUNIT_ASSERT(_negotiatedFareController->validateCat35SegData(true));
  }

  void testCheckIfNeedNewFareAmt_Pass_Selling()
  {
    setupTestingData(PricingTrx::FAREDISPLAY_TRX, "ADT", RuleConst::SELLING_FARE);
    CPPUNIT_ASSERT(_negotiatedFareController->checkIfNeedNewFareAmt(
        *_paxTypeFare, *_paxTypeFare, &RuleConst::NET_FARE));
  }

  void testCheckIfNeedNewFareAmt_Fail_NotSelling()
  {
    setupTestingData(PricingTrx::FAREDISPLAY_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    CPPUNIT_ASSERT(!_negotiatedFareController->checkIfNeedNewFareAmt(
        *_paxTypeFare, *_paxTypeFare, &RuleConst::NET_FARE));
  }

  void testCheckIfNeedNewFareAmt_Fail_JalAxessTypeC()
  {
    setupTestingData(true, false, PricingTrx::PRICING_TRX, "ADT", RuleConst::NET_SUBMIT_FARE_UPD);
    CPPUNIT_ASSERT(!_negotiatedFareController->checkIfNeedNewFareAmt(
        *_paxTypeFare, *_paxTypeFare, &RuleConst::NET_FARE));
  }

  void testPrintMatchedFare_Selling_CreatorPCC()
  {
    setupTestingData(MockNegFareRest("ADT", 0));
    _negPaxTypeFareRuleData->creatorPCC() = "AAAA";
    _negPaxTypeFareRuleData->ruleItemInfo() = _negotiatedFareController->_negFareRest;
    _negotiatedFareController->_dc->activate();
    _negotiatedFareController->printMatchedFare(*_negPaxTypeFareRuleData, *_paxTypeFare, true);
    CPPUNIT_ASSERT(_negotiatedFareController->_dc->str().find("CREATOR PCC-AAAA") != (size_t) - 1);
  }

  void testPrintMatchedFare_Selling_CwtUser()
  {
    setupTestingData(MockNegFareRest("ADT", 0));
    _negPaxTypeFareRuleData->ruleItemInfo() = _negotiatedFareController->_negFareRest;
    _negotiatedFareController->_dc->activate();
    enableAbacus();
    _negotiatedFareController->printMatchedFare(*_negPaxTypeFareRuleData, *_paxTypeFare, true);
    std::string s = _negotiatedFareController->_dc->str();
    CPPUNIT_ASSERT(_negotiatedFareController->_dc->str().find("NATION FRANCE INDICATOR-") !=
                   (size_t) - 1);
  }

  void testCreateFareInfoAndSetFlags_RexCreate()
  {
    setupTestingData(MockNegFareRest("ADT", 0));
    PaxTypeFare newFare;

    _negotiatedFareController->_rexCreateFaresForPrevAgent = true;
    _negotiatedFareController->_processPrevAgentForRex = true;

    _negotiatedFareController->createFareInfoAndSetFlags(*_paxTypeFare,
                                                         newFare,
                                                         *_negPaxTypeFareRuleData,
                                                         *(_negotiatedFareController->_negFareRest),
                                                         NULL);
    CPPUNIT_ASSERT(_negPaxTypeFareRuleData->rexCat35FareUsingPrevAgent());
  }

  void testCreateFareInfoAndSetFlags_Cat35Type()
  {
    setupTestingData(MockNegFareRest("ADT", 0));
    PaxTypeFare newFare;
    Indicator ind = 'X';

    _negotiatedFareController->createFareInfoAndSetFlags(*_paxTypeFare,
                                                         newFare,
                                                         *_negPaxTypeFareRuleData,
                                                         *(_negotiatedFareController->_negFareRest),
                                                         &ind);
    CPPUNIT_ASSERT_EQUAL(ind, newFare.fareDisplayCat35Type());
  }

  void testCreateFareInfoAndSetFlags_NationFrance()
  {
    setupTestingData(MockNegFareRest("ADT", 0));
    PaxTypeFare newFare;

    _negotiatedFareController->_nationFranceFare = true;

    _negotiatedFareController->createFareInfoAndSetFlags(*_paxTypeFare,
                                                         newFare,
                                                         *_negPaxTypeFareRuleData,
                                                         *(_negotiatedFareController->_negFareRest),
                                                         NULL);
    CPPUNIT_ASSERT(newFare.isNationFRInCat35());
  }

  void testCreateFareInfoAndSetFlags_AccountCode()
  {
    setupTestingData(MockNegFareRest("ADT", 0));
    PaxTypeFare newFare;
    _negotiatedFareController->_accCodeMatch = true;
    _negotiatedFareController->_accCodeValue = "ABC";

    _negotiatedFareController->createFareInfoAndSetFlags(*_paxTypeFare,
                                                         newFare,
                                                         *_negPaxTypeFareRuleData,
                                                         *(_negotiatedFareController->_negFareRest),
                                                         NULL);
    CPPUNIT_ASSERT_EQUAL(_negotiatedFareController->_accCodeValue, newFare.matchedAccCode());
  }

  void testSetAmounts_Selling()
  {
    setupTestingData(
        MockNegFareRest("ADT", 1), PricingTrx::PRICING_TRX, "ADT", RuleConst::SELLING_FARE);
    MockPaxTypeFare newFare(*_fareMarket, "ADT");
    FareInfo fareInfo;

    _negotiatedFareController->setAmounts(*_paxTypeFare,
                                          newFare,
                                          *_negPaxTypeFareRuleData,
                                          *(_negotiatedFareController->_negFareRest),
                                          fareInfo,
                                          1.0,
                                          2.0,
                                          NULL);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, _negPaxTypeFareRuleData->netAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, _negPaxTypeFareRuleData->nucNetAmount(), 0.01);
  }

  void testSetAmounts_NotSelling()
  {
    setupTestingData(MockNegFareRest("ADT", 1),
                     PricingTrx::PRICING_TRX,
                     "ADT",
                     RuleConst::NET_SUBMIT_FARE,
                     6,
                     1,
                     "",
                     2.0);
    MockPaxTypeFare newFare(*_fareMarket, "ADT");
    FareInfo fareInfo;

    _negotiatedFareController->setAmounts(*_paxTypeFare,
                                          newFare,
                                          *_negPaxTypeFareRuleData,
                                          *(_negotiatedFareController->_negFareRest),
                                          fareInfo,
                                          1.0,
                                          2.0,
                                          NULL);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, _negPaxTypeFareRuleData->netAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, _negPaxTypeFareRuleData->nucNetAmount(), 0.01);
  }

  void testSetAmounts_NoNeedNewAmt()
  {
    setupTestingData(
        MockNegFareRest("ADT", 1), PricingTrx::FAREDISPLAY_TRX, "ADT", RuleConst::NET_SUBMIT_FARE);
    MockPaxTypeFare newFare(*_fareMarket, "ADT");
    FareInfo fareInfo;
    fareInfo.fareAmount() = 7.0;

    _negotiatedFareController->setAmounts(*_paxTypeFare,
                                          newFare,
                                          *_negPaxTypeFareRuleData,
                                          *(_negotiatedFareController->_negFareRest),
                                          fareInfo,
                                          1.0,
                                          2.0,
                                          &RuleConst::NET_FARE);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.0, fareInfo.fareAmount(), 0.01);
  }

  void testSetAmounts_NeedNewAmt()
  {
    setupTestingData(MockNegFareRest("ADT", 1),
                     PricingTrx::FAREDISPLAY_TRX,
                     "ADT",
                     RuleConst::SELLING_FARE,
                     6,
                     1,
                     "",
                     2.0);
    MockPaxTypeFare newFare(*_fareMarket, "ADT");
    FareInfo fareInfo;
    fareInfo.fareAmount() = 7.0;

    _negotiatedFareController->setAmounts(*_paxTypeFare,
                                          newFare,
                                          *_negPaxTypeFareRuleData,
                                          *(_negotiatedFareController->_negFareRest),
                                          fareInfo,
                                          1.0,
                                          2.0,
                                          &RuleConst::NET_FARE);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, fareInfo.fareAmount(), 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, newFare.nucFareAmount(), 0.01);
  }

  void testSetAmounts_NeedNewAmtFD()
  {
    setupTestingData(MockNegFareRest("ADT", 1),
                     PricingTrx::FAREDISPLAY_TRX,
                     "ADT",
                     RuleConst::SELLING_FARE,
                     6,
                     1,
                     "",
                     2.0);
    MockPaxTypeFare newFare(*_fareMarket, "ADT");
    FareInfo fareInfo;
    fareInfo.fareAmount() = 7.0;

    _negotiatedFareController->setAmounts(*_paxTypeFare,
                                          newFare,
                                          *_negPaxTypeFareRuleData,
                                          *(_negotiatedFareController->_negFareRest),
                                          fareInfo,
                                          1.0,
                                          4.0,
                                          &RuleConst::NET_FARE);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, newFare.nucOriginalFareAmount(), 0.01);
  }

  void testCreateFare_PricingOption()
  {
    setupTestingData(PricingTrx::FAREDISPLAY_TRX);
    _negotiatedFareController->createFare(
        *_paxTypeFare, _negPaxTypeFareRuleData, false, 0.0, 0.0, true, NULL);
    CPPUNIT_ASSERT_EQUAL(0, (int)_negotiatedFareController->_negPaxTypeFares.size());
  }

  void testCreateFare_NegativeAmount()
  {
    setupTestingData();
    _negotiatedFareController->createFare(
        *_paxTypeFare, _negPaxTypeFareRuleData, false, -1.0, 0.0, false, NULL);
    CPPUNIT_ASSERT_EQUAL(0, (int)_negotiatedFareController->_negPaxTypeFares.size());
  }

  void testCreateFare_Processed()
  {
    setupTestingData(MockNegFareRest("ADT", 1));
    _negPaxTypeFareRuleData->ruleItemInfo() = _negotiatedFareController->_negFareRest;
    std::vector<CategoryRuleItemInfo> itemInfoVec;
    _negPaxTypeFareRuleData->categoryRuleItemInfoVec() = &itemInfoVec;
    _negotiatedFareController->createFare(
        *_paxTypeFare, _negPaxTypeFareRuleData, false, 1.0, 0.0, false, &RuleConst::NET_FARE);
    ASSERT_EQ(1u, _negotiatedFareController->_negPtfBucketContainer->size());
  }

  void testCreateFare_FDRemoveDuplicate()
  {
    setupTestingData(MockNegFareRest("ADT", 1), PricingTrx::FAREDISPLAY_TRX);
    _negPaxTypeFareRuleData->ruleItemInfo() = _negotiatedFareController->_negFareRest;
    (*_paxTypeFare->paxTypeFareRuleDataMap())[RuleConst::NEGOTIATED_RULE] = nullptr;
    std::vector<CategoryRuleItemInfo> itemInfoVec;
    _negPaxTypeFareRuleData->categoryRuleItemInfoVec() = &itemInfoVec;
    _negotiatedFareController->createFare(
        *_paxTypeFare, _negPaxTypeFareRuleData, false, 1.0, 0.0, false, &RuleConst::NET_FARE);
    auto pftRuleDataMap = _negotiatedFareController->_negPtfBucketContainer->begin()->begin()->begin()->ptf->paxTypeFareRuleDataMap();
    auto nCats = std::count_if(pftRuleDataMap->begin(),
                               pftRuleDataMap->end(),
                               [](PaxTypeFare::PaxTypeFareRuleDataByCatNo::const_reference cat) -> bool { return cat.load(std::memory_order_relaxed);});
    CPPUNIT_ASSERT_EQUAL(1L, nCats);
  }

  void testCreateFare_FDNoDuplicate()
  {
    setupTestingData(MockNegFareRest("ADT", 1), PricingTrx::FAREDISPLAY_TRX);
    _negPaxTypeFareRuleData->ruleItemInfo() = _negotiatedFareController->_negFareRest;
    PaxTypeFare::PaxTypeFareAllRuleData cat1;
    (*_paxTypeFare->paxTypeFareRuleDataMap())[RuleConst::ELIGIBILITY_RULE] = &cat1;
    std::vector<CategoryRuleItemInfo> itemInfoVec;
    _negPaxTypeFareRuleData->categoryRuleItemInfoVec() = &itemInfoVec;
    _negotiatedFareController->createFare(
        *_paxTypeFare, _negPaxTypeFareRuleData, false, 1.0, 0.0, false, &RuleConst::NET_FARE);
    auto pftRuleDataMap = _negotiatedFareController->_negPtfBucketContainer->begin()->begin()->begin()->ptf->paxTypeFareRuleDataMap();
    auto nCats = std::count_if(pftRuleDataMap->begin(),
                               pftRuleDataMap->end(),
                               [](PaxTypeFare::PaxTypeFareRuleDataByCatNo::const_reference cat) -> bool { return cat.load(std::memory_order_relaxed);});
    CPPUNIT_ASSERT_EQUAL(2L, nCats);
  }

  void testCreateFare_SoftPass()
  {
    setupTestingData(MockNegFareRest("ADT", 1));
    _negPaxTypeFareRuleData->ruleItemInfo() = _negotiatedFareController->_negFareRest;
    std::vector<CategoryRuleItemInfo> itemInfoVec;
    _negPaxTypeFareRuleData->categoryRuleItemInfoVec() = &itemInfoVec;
    _negotiatedFareController->createFare(
        *_paxTypeFare, _negPaxTypeFareRuleData, false, 1.0, 0.0, false, &RuleConst::NET_FARE);
    CPPUNIT_ASSERT(_negotiatedFareController->_negPtfBucketContainer->begin()
                       ->begin()
                       ->begin()
                       ->ptf->isCategorySoftPassed(RuleConst::NEGOTIATED_RULE));
  }

  void testCreateFare_InitFDInfo()
  {
    setupTestingData(MockNegFareRest("ADT", 1), PricingTrx::FAREDISPLAY_TRX);
    _negPaxTypeFareRuleData->ruleItemInfo() = _negotiatedFareController->_negFareRest;
    std::vector<CategoryRuleItemInfo> itemInfoVec;
    _negPaxTypeFareRuleData->categoryRuleItemInfoVec() = &itemInfoVec;
    _negotiatedFareController->createFare(
        *_paxTypeFare, _negPaxTypeFareRuleData, false, 1.0, 0.0, false, &RuleConst::NET_FARE);
    CPPUNIT_ASSERT(_negotiatedFareController->_negPtfBucketContainer->begin()
                       ->begin()
                       ->begin()
                       ->ptf->fareDisplayInfo());
  }

  void testCheckMarkupControl_Pass()
  {
    MockMarkupControl markupRec(NegotiatedFareController::DECLINED, YES, YES);
    CPPUNIT_ASSERT(NegotiatedFareController::checkMarkupControl(markupRec, true));
  }

  void testCheckMarkupControl_Fail_Pending()
  {
    MockMarkupControl markupRec(NegotiatedFareController::PENDING);
    CPPUNIT_ASSERT(!NegotiatedFareController::checkMarkupControl(markupRec, false));
  }

  void testCheckMarkupControl_Fail_Ticket()
  {
    MockMarkupControl markupRec(NegotiatedFareController::DECLINED, NO, NO);
    CPPUNIT_ASSERT(!NegotiatedFareController::checkMarkupControl(markupRec, true));
  }

  void testCheckMarkupControl_Fail_Selling()
  {
    MockMarkupControl markupRec(NegotiatedFareController::DECLINED, NO, NO);
    CPPUNIT_ASSERT(!NegotiatedFareController::checkMarkupControl(markupRec, false));
  }

  void testMatchedCorpIDInPricing_Fail()
  {
    setupTestingData(PricingTrx::PRICING_TRX);
    _trx->getOptions()->forceCorpFares() = true;
    _paxTypeFare->setMatchedCorpID(false);
    CPPUNIT_ASSERT(!_negotiatedFareController->matchedCorpIDInPricing(*_paxTypeFare));
  }

  void testMatchedCorpIDInPricing_Pass_Fd()
  {
    setupTestingData(PricingTrx::FAREDISPLAY_TRX);
    CPPUNIT_ASSERT(_negotiatedFareController->matchedCorpIDInPricing(*_paxTypeFare));
  }

  void testMatchedCorpIDInPricing_Pass_ForceCorpFares()
  {
    setupTestingData(PricingTrx::PRICING_TRX);
    _trx->getOptions()->forceCorpFares() = false;
    CPPUNIT_ASSERT(_negotiatedFareController->matchedCorpIDInPricing(*_paxTypeFare));
  }

  void testMatchedCorpIDInPricing_Pass_MatchCorpId()
  {
    setupTestingData(PricingTrx::PRICING_TRX);
    _trx->getOptions()->forceCorpFares() = true;
    _paxTypeFare->setMatchedCorpID(true);
    CPPUNIT_ASSERT(_negotiatedFareController->matchedCorpIDInPricing(*_paxTypeFare));
  }

  void testGetOneCalc_Fail_Get979()
  {
    setupTestingData(MockNegFareRest("ADT", 1));
    ((MockNegotiatedFareController*)_negotiatedFareController)->t_getNegFareCalc.push_back(NULL);
    Money base(_paxTypeFare->fare()->originalFareAmount(), _paxTypeFare->currency());
    MoneyAmount amt, amtNuc;
    amt = 100;
    amtNuc = 100;
    CPPUNIT_ASSERT(!_negotiatedFareController->getOneCalc(*_paxTypeFare, false, 0, *_negFareCalc,
                                                          _negPaxTypeFareRuleData, base, amt, amtNuc, _negFareSecurityInfo));
  }

  void testGetOneCalc_Pass_IsUpd()
  {
    setupTestingData(MockNegFareRest("ADT", 0));
    Money base(_paxTypeFare->fare()->originalFareAmount(), _paxTypeFare->currency());
    MoneyAmount amt, amtNuc;
    amt = 100;
    amtNuc = 100;
    CPPUNIT_ASSERT(_negotiatedFareController->getOneCalc(*_paxTypeFare, false, 0, *_negFareCalc,
                                                          _negPaxTypeFareRuleData, base, amt, amtNuc, _negFareSecurityInfo));
  }

  void testGetOneCalc_Fail_CheckMarupControl()
  {
    setupTestingData(MockNegFareRest("ADT", 0));
    MockMarkupControl mc(NegotiatedFareController::PENDING);
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_getMarkupBySecondSellerId.push_back(&mc);
    Money base(_paxTypeFare->fare()->originalFareAmount(), _paxTypeFare->currency());
    MoneyAmount amt, amtNuc;
    amt = 100;
    amtNuc = 100;
    CPPUNIT_ASSERT(!_negotiatedFareController->getOneCalc(*_paxTypeFare, true, 0, *_negFareCalc,
                                                          _negPaxTypeFareRuleData, base, amt, amtNuc, _negFareSecurityInfo));
  }

  void testGetOneCalc_Fail_NetSellingInd()
  {
    setupTestingData(MockNegFareRest("ADT", 0));
    MockMarkupControl mc(NegotiatedFareController::DECLINED, YES, YES, ' ');
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_getMarkupBySecondSellerId.push_back(&mc);
    Money base(_paxTypeFare->fare()->originalFareAmount(), _paxTypeFare->currency());
    MoneyAmount amt, amtNuc;
    amt = 100;
    amtNuc = 100;
    CPPUNIT_ASSERT(!_negotiatedFareController->getOneCalc(*_paxTypeFare, true, 0, *_negFareCalc,
                                                          _negPaxTypeFareRuleData, base, amt, amtNuc, _negFareSecurityInfo));
  }

  void testGetOneCalc_Pass_NetSellingInd()
  {
    setupTestingData(MockNegFareRest("ADT", 0));
    MockMarkupControl mc(NegotiatedFareController::DECLINED, YES, YES, S_TYPE);
    ((MockNegotiatedFareController*)_negotiatedFareController)
        ->t_getMarkupBySecondSellerId.push_back(&mc);
    Money base(_paxTypeFare->fare()->originalFareAmount(), _paxTypeFare->currency());
    MoneyAmount amt, amtNuc;
    amt = 100;
    amtNuc = 100;
    CPPUNIT_ASSERT(_negotiatedFareController->getOneCalc(*_paxTypeFare, true, 0, *_negFareCalc,
                                                          _negPaxTypeFareRuleData, base, amt, amtNuc, _negFareSecurityInfo));
  }

  void testIsSoftPass_Pass_VectorNull()
  {
    setupTestingData();
    std::vector<CategoryRuleItemInfo> itemInfoVec;
    _negPaxTypeFareRuleData->categoryRuleItemInfoVec() = &itemInfoVec;
    CPPUNIT_ASSERT(_negotiatedFareController->isSoftPass(_negPaxTypeFareRuleData));
  }

  void testIsSoftPass_Fail_DirectionalityBlank()
  {
    setupTestingData();
    auto catInfo = dummyCRII(
        0, NegotiatedFareController::BLANK, NegotiatedFareController::BLANK);
    _negPaxTypeFareRuleData->categoryRuleItemInfo() = &catInfo;
    CPPUNIT_ASSERT(!_negotiatedFareController->isSoftPass(_negPaxTypeFareRuleData));
  }

  void testIsSoftPass_Fail_DirectionalityLoc1ToLoc2()
  {
    setupTestingData();
    auto catInfo = dummyCRII(
        0, NegotiatedFareController::LOC1_TO_LOC2, NegotiatedFareController::BLANK);
    _negPaxTypeFareRuleData->categoryRuleItemInfo() = &catInfo;
    CPPUNIT_ASSERT(!_negotiatedFareController->isSoftPass(_negPaxTypeFareRuleData));
  }

  void testIsSoftPass_Fail_DirectionalityLoc2ToLoc1()
  {
    setupTestingData();
    auto catInfo = dummyCRII(
        0, NegotiatedFareController::LOC2_TO_LOC1, NegotiatedFareController::BLANK);
    _negPaxTypeFareRuleData->categoryRuleItemInfo() = &catInfo;
    CPPUNIT_ASSERT(!_negotiatedFareController->isSoftPass(_negPaxTypeFareRuleData));
  }

  void testIsSoftPass_Pass_InOutInd()
  {
    setupTestingData();
    auto catInfo = dummyCRII(
        0, NegotiatedFareController::BLANK, NegotiatedFareController::LOC1_TO_LOC2);
    _negPaxTypeFareRuleData->categoryRuleItemInfo() = &catInfo;
    CPPUNIT_ASSERT(_negotiatedFareController->isSoftPass(_negPaxTypeFareRuleData));
  }

  void testInvokeCreateFare_LType()
  {
    setupTestingData(
        MockNegFareRest("ADT", 1), PricingTrx::FAREDISPLAY_TRX, "ADT", RuleConst::SELLING_FARE);
    ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideCreateFareAndResetMinAmt =
        1;
    _negotiatedFareController->invokeCreateFare(
        *_paxTypeFare, 0.0, 0.0, *_negFareCalc, _negFareSecurityInfo, 'X', 'Y');
    CPPUNIT_ASSERT(!((*_paxTypeFare->paxTypeFareRuleDataMap())[RuleConst::NEGOTIATED_RULE]));
  }

  void testInvokeCreateFare_NotLType()
  {
    setupTestingData(
        MockNegFareRest("ADT", 1), PricingTrx::FAREDISPLAY_TRX, "ADT", RuleConst::NET_FARE);
    ((MockNegotiatedFareController*)_negotiatedFareController)->t_overrideCreateFareAndResetMinAmt =
        1;
    _negotiatedFareController->invokeCreateFare(
        *_paxTypeFare, 0.0, 0.0, *_negFareCalc, _negFareSecurityInfo, 'X', 'Y');
    CPPUNIT_ASSERT((*_paxTypeFare->paxTypeFareRuleDataMap())[RuleConst::NEGOTIATED_RULE]);
  }

  void testFillSegInfo_Empty()
  {
    setupTestingData(MockNegFareRest("MIL", 0), PricingTrx::PRICING_TRX, "");
    MockPaxTypeFare ptf(*_fareMarket, "ADT");
    addReqPaxTypeToTrx("MIL");
    _negotiatedFareController->fillSegInfo(
        *_paxTypeFare, ptf, *_negotiatedFareController->_negFareRest);
    CPPUNIT_ASSERT_EQUAL(PaxTypeCode("MIL"), ptf.actualPaxType()->paxType());
  }

  void testFillSegInfo_Adult()
  {
    setupTestingData(MockNegFareRest("MIL", 0), PricingTrx::PRICING_TRX, "ADT");
    MockPaxTypeFare ptf(*_fareMarket, "ADT");
    addReqPaxTypeToTrx("MIL");
    _negotiatedFareController->fillSegInfo(
        *_paxTypeFare, ptf, *_negotiatedFareController->_negFareRest);
    CPPUNIT_ASSERT_EQUAL(PaxTypeCode("MIL"), ptf.actualPaxType()->paxType());
  }

  void testFillSegInfo_NoChange()
  {
    setupTestingData(MockNegFareRest("MIL", 0), PricingTrx::PRICING_TRX, "MIL");
    MockPaxTypeFare ptf(*_fareMarket, "ADT");
    addReqPaxTypeToTrx("MIL");
    _negotiatedFareController->fillSegInfo(
        *_paxTypeFare, ptf, *_negotiatedFareController->_negFareRest);
    CPPUNIT_ASSERT_EQUAL(PaxTypeCode("ADT"), ptf.actualPaxType()->paxType());
  }

  void testGetValidCalc_Pass()
  {
    setupTestingData();
    MoneyAmount amount;
    MockNegotiatedFareController* nfc = (MockNegotiatedFareController*)_negotiatedFareController;
    nfc->clearNegFareSecurityInfo();
    nfc->t_getNegFareSecurity.push_back(NULL);
    nfc->t_overrideProcessRedistributions = true;

    CPPUNIT_ASSERT(_negotiatedFareController->getValidCalc(
        *_paxTypeFare, _negPaxTypeFareRuleData, amount, amount));
  }

  void testGetValidCalc_Fail_SecurityMatch()
  {
    setupTestingData();
    MoneyAmount amount;
    MockNegotiatedFareController* nfc = (MockNegotiatedFareController*)_negotiatedFareController;
    nfc->clearNegFareSecurityInfo();
    nfc->t_overrideProcessRedistributions = true;

    CPPUNIT_ASSERT(!_negotiatedFareController->getValidCalc(
        *_paxTypeFare, _negPaxTypeFareRuleData, amount, amount));
  }

  void testUpdateFareMarket()
  {
    setupTestingData();
    _negotiatedFareController->_negPaxTypeFares.push_back(_paxTypeFare);
    _negotiatedFareController->updateFareMarket();
    CPPUNIT_ASSERT_EQUAL((size_t)0, _negotiatedFareController->_negPaxTypeFares.size());
  }

  void initOneBucketOneR3TwoFares(NegPaxTypeFareData& ptfData1, NegPaxTypeFareData& ptfData2)
  {
    CategoryRuleItemInfo catRuleItemInfo;
    ptfData1.isPricingOption = ptfData2.isPricingOption = true;
    ptfData1.r2SubSetNum = ptfData2.r2SubSetNum = 1u;
    ptfData1.isDir3 = ptfData2.isDir3 = true;
    ptfData1.catRuleItemInfo = ptfData2.catRuleItemInfo = &catRuleItemInfo;

    _negotiatedFareController->_negPtfBucketContainer->insert(ptfData1);
    _negotiatedFareController->_negPtfBucketContainer->insert(ptfData2);
  }

  void testSelectNegFaresSameItemNumber()
  {
    setupTestingData();
    PaxTypeFare ptf1, ptf2;
    NegPaxTypeFareData ptfData1, ptfData2;
    ptfData1.ptf = &ptf1;
    ptfData2.ptf = &ptf2;
    initOneBucketOneR3TwoFares(ptfData1, ptfData2);

    _negotiatedFareController->selectNegFares();

    ASSERT_EQ(2u, _negotiatedFareController->_negPaxTypeFares.size());
  }

  void testSelectNegFaresDifferentItemNumber()
  {
    setupTestingData();
    PaxTypeFare ptf1, ptf2;
    NegPaxTypeFareData ptfData1, ptfData2;
    CategoryRuleItemInfo catRuleItemInfo1, catRuleItemInfo2;
    ptfData1.ptf = &ptf1;
    catRuleItemInfo1.setItemNo(1u);
    ptfData1.catRuleItemInfo = &catRuleItemInfo1;
    ptfData2.ptf = &ptf2;
    catRuleItemInfo2.setItemNo(2u);
    ptfData2.catRuleItemInfo = &catRuleItemInfo2;

    ptfData1.psgType = ptfData2.psgType = ADULT;
    ptfData1.isPricingOption = ptfData2.isPricingOption = true;
    ptfData1.isQualifierPresent = ptfData2.isQualifierPresent = false;
    ptfData1.isDirectionalityApplied = ptfData2.isDirectionalityApplied = true;
    ptfData1.r2SubSetNum = ptfData2.r2SubSetNum = 1u;
    ptfData1.isDir3 = ptfData2.isDir3 = true;

    _negotiatedFareController->_negPtfBucketContainer->insert(ptfData1);
    _negotiatedFareController->_negPtfBucketContainer->insert(ptfData2);

    _negotiatedFareController->selectNegFares();

    ASSERT_TRUE(_negotiatedFareController->_negPtfBucketContainer->getComparator().areEquivalent(
        ptfData1, ptfData2));
    ASSERT_EQ(1u, _negotiatedFareController->_negPtfBucketContainer->size());
    ASSERT_EQ(1u, _negotiatedFareController->_negPaxTypeFares.size());
  }

  void testSelectNegFaresTwoBuckets()
  {
    setupTestingData();
    PaxTypeFare ptf1, ptf2;
    NegPaxTypeFareData ptfData1, ptfData2;
    CategoryRuleItemInfo catRuleItemInfo1, catRuleItemInfo2;
    ptfData1.ptf = &ptf1;
    ptfData1.itemNo = 1u;
    ptfData1.catRuleItemInfo = &catRuleItemInfo1;
    ptfData1.isPricingOption = false;
    ptfData2.ptf = &ptf2;
    ptfData2.itemNo = 2u;
    ptfData2.catRuleItemInfo = &catRuleItemInfo2;
    ptfData2.r2SubSetNum = 1u;
    ptfData2.isPricingOption = true;

    ptfData1.isDir3 = ptfData2.isDir3 = true;

    _negotiatedFareController->_negPtfBucketContainer->insert(ptfData1);
    _negotiatedFareController->_negPtfBucketContainer->insert(ptfData2);

    _negotiatedFareController->selectNegFares();

    ASSERT_EQ(2u, _negotiatedFareController->_negPaxTypeFares.size());
  }

  void testSelectNegFaresDifferentItemNumberMIP()
  {
    setupTestingData();
    _fareMarket->setHasDuplicates(true);

    PaxTypeFare ptf1, ptf2;
    NegPaxTypeFareData ptfData1, ptfData2;
    CategoryRuleItemInfo catRuleItemInfo1, catRuleItemInfo2;
    ptfData1.ptf = &ptf1;
    catRuleItemInfo1.setItemNo(1u);
    ptfData1.catRuleItemInfo = &catRuleItemInfo1;
    ptfData2.ptf = &ptf2;
    catRuleItemInfo2.setItemNo(2u);
    ptfData2.catRuleItemInfo = &catRuleItemInfo2;

    ptfData1.psgType = ptfData2.psgType = ADULT;
    ptfData1.isPricingOption = ptfData2.isPricingOption = true;
    ptfData1.isQualifierPresent = ptfData2.isQualifierPresent = false;
    ptfData1.isDirectionalityApplied = ptfData2.isDirectionalityApplied = true;
    ptfData1.r2SubSetNum = ptfData2.r2SubSetNum = 1u;
    ptfData1.isDir3 = ptfData2.isDir3 = true;

    _negotiatedFareController->_negPtfBucketContainer->insert(ptfData1);
    _negotiatedFareController->_negPtfBucketContainer->insert(ptfData2);

    _negotiatedFareController->selectNegFares();

    ASSERT_TRUE(_negotiatedFareController->_negPtfBucketContainer->getComparator().areEquivalent(
        ptfData1, ptfData2));
    ASSERT_EQ(1u, _negotiatedFareController->_negPtfBucketContainer->size());
    ASSERT_EQ(2u, _negotiatedFareController->_negPaxTypeFares.size());
    ASSERT_EQ(1u, _negotiatedFareController->_fareMarket.getAllNegFaresBuckets().size());
    ASSERT_EQ(_negotiatedFareController->_negPtfBucketContainer,
              _negotiatedFareController->_fareMarket.getAllNegFaresBuckets().front());
  }

  void testGetFareRetailerRuleLookupInfo()
  {
    setupTestingData();
    _trx->setRequest(0);

    CPPUNIT_ASSERT(!_negotiatedFareController->getFareRetailerRuleLookupInfo());

    PricingRequest request;
    request.ticketingAgent() = 0;
    _trx->setRequest(&request);

    CPPUNIT_ASSERT(!_negotiatedFareController->getFareRetailerRuleLookupInfo());

    Agent agent;
    agent.tvlAgencyPCC() = "";

    request.ticketingAgent() = &agent;

    CPPUNIT_ASSERT(!_negotiatedFareController->getFareRetailerRuleLookupInfo());

    agent.tvlAgencyPCC() = "ABCD";
    CPPUNIT_ASSERT(_negotiatedFareController->getFareRetailerRuleLookupInfo());
  }

  void testcreateAgentSourcePcc()
  {
    setupTestingData();
    _trx->setRequest(0);
    const PseudoCityCode& sourcePcc = "5KAD";
    Agent* sourceAGT = _negotiatedFareController->createAgentSourcePcc(sourcePcc);
    CPPUNIT_ASSERT_EQUAL(PseudoCityCode("5KAD"), sourceAGT->tvlAgencyPCC());
  }

  void testprocessSourcePccSecurityMatch()
  {
    setupTestingData();
    _trx->setRequest(0);

  }

  void testprocessPermissionIndicatorsFail()
  {
    setupTestingData();
    PricingRequest* request;
    _trx->dataHandle().get(request);
    _trx->setRequest(request);
    Agent* agent;
    _trx->dataHandle().get(agent);
    agent->tvlAgencyPCC() = "NL43";
    agent->mainTvlAgencyPCC() = "AC41";
    _trx->getRequest()->ticketingAgent() = agent;
    const Indicator& displayCatType = ' ';
    _frri = _memHandle.create<FareRetailerRuleInfo>();
    PseudoCityCode sourcePcc = "AC41";
    _frrfai = _memHandle.create<FareRetailerResultingFareAttrInfo>();
    _frrfai->sellInd() = 'N';
    _context=_memHandle.insert(new FareRetailerRuleContext(sourcePcc,
                                                           _frri,
                                                           _frciV,
                                                           _frrfai,
                                                           _ffsi));
    _negFareSecurityInfo->sellInd() = 'N';
    _context->_t983 = _negFareSecurityInfo;

    CPPUNIT_ASSERT(!_negotiatedFareController->processPermissionIndicators(*_context, _negFareSecurityInfo, _negPaxTypeFareRuleData, displayCatType));
  }

  void testprocessPermissionIndicatorsPass()
  {
    setupTestingData();
    PricingRequest* request;
    _trx->dataHandle().get(request);
    _trx->setRequest(request);
    Agent* agent;
    _trx->dataHandle().get(agent);
    agent->tvlAgencyPCC() = "NL43";
    agent->mainTvlAgencyPCC() = "AC41";
    _trx->getRequest()->ticketingAgent() = agent;
    const Indicator& displayCatType = ' ';
    _frri = _memHandle.create<FareRetailerRuleInfo>();
    PseudoCityCode sourcePcc = "AC41";
    _frrfai = _memHandle.create<FareRetailerResultingFareAttrInfo>();
    _frrfai->updateInd() = 'N';
    _frrfai->sellInd() = 'Y';
    _frrfai->ticketInd() = 'Y';
    _context=_memHandle.insert(new FareRetailerRuleContext(sourcePcc,
                                                           _frri,
                                                           _frciV,
                                                           _frrfai,
                                                           _ffsi));

    _negFareSecurityInfo->updateInd()= 'Y';
    _negFareSecurityInfo->redistributeInd()= 'Y';
    _negFareSecurityInfo->sellInd() = 'Y';
    _negFareSecurityInfo->ticketInd() = 'Y';
    _context->_t983 = _negFareSecurityInfo;

    CPPUNIT_ASSERT(_negotiatedFareController->processPermissionIndicators(*_context, _negFareSecurityInfo, _negPaxTypeFareRuleData, displayCatType));
    Indicator ind = 'N';
    CPPUNIT_ASSERT_EQUAL(ind, _negPaxTypeFareRuleData->updateInd());
  }

  void test_isPtfValidForCat25Responsive()
  {
    setupTestingData();
    PaxTypeFare ptf;

    FareByRuleItemInfo fbrRuleItemInfo;
    fbrRuleItemInfo.fareInd() = 'z'; // not 'S', 'K', 'E', 'F' so FBRPaxTypeFareRuleData::isSpecifiedFare
                                     // returns false

    FBRPaxTypeFareRuleData fareRuleData;
    fareRuleData.ruleItemInfo() = dynamic_cast<const RuleItemInfo*> (&fbrRuleItemInfo);

    PaxTypeFare::PaxTypeFareAllRuleData allRuleData;
    allRuleData.fareRuleData = &fareRuleData;
    (*ptf.paxTypeFareRuleDataMap())[RuleConst::FARE_BY_RULE] = &allRuleData;
    ptf.status().set(PaxTypeFare::PTF_Discounted, false);
    CPPUNIT_ASSERT(!_negotiatedFareController->isPtfValidForCat25Responsive(ptf));

    ptf.status().set(PaxTypeFare::PTF_FareByRule, true);
    CPPUNIT_ASSERT(_negotiatedFareController->isPtfValidForCat25Responsive(ptf));

    ptf.status().set(PaxTypeFare::PTF_Discounted, true);
    CPPUNIT_ASSERT(!_negotiatedFareController->isPtfValidForCat25Responsive(*_paxTypeFare));
  }
 
  void test_processFareRetailerRuleTypeCFbrResponsive()
  {
    setupTestingData();
    PricingRequest* request;
    _trx->dataHandle().get(request);
    _trx->setRequest(request);
    Agent* agent;
    _trx->dataHandle().get(agent);
    agent->tvlAgencyPCC() = "NL43";
    agent->mainTvlAgencyPCC() = "AC41";
    _trx->getRequest()->ticketingAgent() = agent;
    _frri = _memHandle.create<FareRetailerRuleInfo>();
    PseudoCityCode sourcePcc = "AC41";
    _frrfai = _memHandle.create<FareRetailerResultingFareAttrInfo>();
    _frrfai->updateInd() = 'N';
    _frrfai->sellInd() = 'Y';
    _frrfai->ticketInd() = 'Y';
    _context=_memHandle.insert(new FareRetailerRuleContext(sourcePcc,
                                                           _frri,
                                                           _frciV,
                                                           _frrfai,
                                                           _ffsi));

    _negFareSecurityInfo->updateInd()= 'Y';
    _negFareSecurityInfo->redistributeInd()= 'Y';
    _negFareSecurityInfo->sellInd() = 'Y';
    _negFareSecurityInfo->ticketInd() = 'Y';
    _context->_t983 = _negFareSecurityInfo;

    _trx->getOptions()->forceCorpFares() = false;
    _trx->getRequest()->accountCode() = "A123";

    // Prepare GeneralFareRuleInfo
    auto* iset = new CategoryRuleItemInfoSet();
    iset->push_back(dummyCRII(CategoryRuleItemInfo::THEN));

    _negotiatedFareController->_catRuleItemInfo = &iset->front();
    _negotiatedFareController->_ruleItemInfoSet = iset;

    _generalFareRuleInfo->vendorCode() = "ATP";
    _generalFareRuleInfo->addItemInfoSetNosync(iset);

    MoneyAmount amt, amtNuc;
    NegPaxTypeFareRuleData* ruleData;

    _negotiatedFareController->resetMinAmt(amt, amtNuc, ruleData);
    NegFareRest nfr;
    _negotiatedFareController->_negFareRest = &nfr;

    bool is979queried = false;
    Money base(_paxTypeFare->fare()->originalFareAmount(), _paxTypeFare->currency());

    FareByRuleItemInfo fbrRuleItemInfo;
    fbrRuleItemInfo.fareInd() = 'S';
    FBRPaxTypeFareRuleData fareRuleData;
    fareRuleData.ruleItemInfo() = dynamic_cast<const RuleItemInfo*> (&fbrRuleItemInfo);

    PaxTypeFare::PaxTypeFareAllRuleData allRuleData;
    allRuleData.fareRuleData = &fareRuleData;
    (*_paxTypeFare->paxTypeFareRuleDataMap())[RuleConst::FARE_BY_RULE] = &allRuleData;

    _negFareSecurityInfo->updateInd()='Y';
    amt = 100;
    amtNuc = 100;
    bool rc=_negotiatedFareController->
              processFareRetailerRuleTypeCFbrResponsive(*_paxTypeFare,
                                                        _negPaxTypeFareRuleData,
                                                        amt,
                                                        amtNuc,
                                                        base,
                                                        *_negFareCalc,
                                                        is979queried,
                                                        true,
                                                        _negFareSecurityInfo);
     CPPUNIT_ASSERT(!rc);
  }
private:
  PricingTrx* _trx;
  Itin* _itin;
  FareMarket* _fareMarket;
  NegotiatedFareController* _negotiatedFareController;
  PaxTypeFare* _paxTypeFare;
  NegFareCalc* _negFareCalc;
  NegFareSecurityInfo* _negFareSecurityInfo;
  NegPaxTypeFareRuleData* _negPaxTypeFareRuleData;
  GeneralFareRuleInfo* _generalFareRuleInfo;
  NegFareRest* _negFareRest;
  TestMemHandle _memHandle;
  std::vector<const FareRetailerRuleLookupInfo*> _frrlVecTypeC, _frrlVecTypeLT;
  std::vector<FareRetailerRuleContext> _frrcVecTypeC, _frrcVecTypeLT;
  FareRetailerCalcDetailInfo* _frCalcDetInfo;
  struct FareRetailerRuleContext* _context;
  std::vector<FareRetailerCalcInfo*> _frciV;
  FareRetailerResultingFareAttrInfo* _frrfai;
  FareFocusSecurityInfo* _ffsi;
  FareRetailerRuleInfo* _frri;

}; // class

CPPUNIT_TEST_SUITE_REGISTRATION(NegotiatedFareControllerTest);

} // tse
