//----------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include <algorithm>
#include <initializer_list>

#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/MockGlobal.h"
#include "test/include/MockTseServer.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

#include "test/testdata/TestLocFactory.h"
#include "test/testdata/TestTicketingFeesInfoFactory.h"

#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/BrandInfo.h"
#include "Common/BaggageTripType.h"
#include "Common/ErrorResponseException.h"
#include "Common/SpecifyMaximumPenaltyCommon.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/XMLConstruct.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "DBAccess/FareProperties.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/ServicesDescription.h"
#include "DBAccess/ServicesSubGroup.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"
#include "DBAccess/TaxText.h"
#include "DBAccess/TicketEndorsementsInfo.h"
#include "DBAccess/TicketingFeesInfo.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "DBAccess/BankerSellRate.h"
#include "DataModel/Agent.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/Billing.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "DataModel/MultiExchangeTrx.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/RefundPenalty.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/StructuredRuleTrx.h"
#include "DataModel/SurfaceSeg.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcConsts.h"
#include "FareCalc/FareCalcCollector.h"
#include "Rules/RuleConst.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OCFeesUsage.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "Xform/PricingResponseFormatter.h"
#include "Xform/PricingResponseXMLTags.h"

namespace tse
{
using boost::assign::list_of;
using boost::assign::operator+=;
using ::testing::_;
using ::testing::Return;

const std::string LOC_DFW = "/vobs/atseintl/test/testdata/data/LocDFW.xml";
const std::string LOC_LON = "/vobs/atseintl/test/testdata/data/LocLON.xml";
const std::string LOC_CHI = "/vobs/atseintl/test/testdata/data/LocCHI.xml";

class PaxTypeFareTest : public PaxTypeFare
{
public:
  void setCarrier(const CarrierCode& cxr) { _carrier = cxr; }
  void
  getValidBrands(const PricingTrx& trx, std::vector<BrandCode>& brands,
    bool hardPassedOnly = false)
      const
  {
    brands.push_back("FD");
  }
  MOCK_CONST_METHOD3(getValidBrandIndex, int(const PricingTrx& trx, const BrandCode* brandCode,
                     Direction fareUsageDirection));
};

class FareCalcConfigBuilder
{
private:
  TestMemHandle* _memHandle;
  FareCalcConfig* _fareCalcConfig;

public:
  FareCalcConfigBuilder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _fareCalcConfig = _memHandle->create<FareCalcConfig>();
  }

  FareCalcConfig* build() { return _fareCalcConfig; }
};

class CalcTotalsBuilder
{
private:
  TestMemHandle* _memHandle;
  CalcTotals* _calcTotals;

public:
  CalcTotalsBuilder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _calcTotals = _memHandle->create<CalcTotals>();
  }

  CalcTotalsBuilder& withFarePath(FarePath* farePath)
  {
    _calcTotals->farePath = farePath;
    return *this;
  }

  CalcTotalsBuilder& withTruePaxType(PaxTypeCode paxType)
  {
    _calcTotals->truePaxType = paxType;
    return *this;
  }

  CalcTotalsBuilder& withRequestedPaxType(PaxTypeCode paxType)
  {
    _calcTotals->requestedPaxType = paxType;
    return *this;
  }

  CalcTotals* build() { return _calcTotals; }
};

class FarePathBuilder
{
private:
  TestMemHandle* _memHandle;
  FarePath* _farePath;

public:
  FarePathBuilder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _farePath = _memHandle->create<FarePath>();
    _farePath->paxType() = _memHandle->create<PaxType>();
    _farePath->paxType()->number() = 1;
    PricingUnit* pu = _memHandle->create<PricingUnit>();
    _farePath->pricingUnit().push_back(pu);
    pu = _memHandle->create<PricingUnit>();
    _farePath->pricingUnit().push_back(pu);
  }

  FarePathBuilder& withFareUsageOnPricingUnit(unsigned int pricingUnitNumber, FareUsage* fareUsage)
  {
    _farePath->pricingUnit()[pricingUnitNumber]->fareUsage().push_back(fareUsage);
    return *this;
  }

  FarePathBuilder& withPaxTypeCode(PaxTypeCode& paxType)
  {
    _farePath->paxType()->paxType() = paxType;
    return *this;
  }

  FarePathBuilder& withItin(Itin* itinerary)
  {
    _farePath->itin() = itinerary;
    return *this;
  }

  FarePathBuilder& withSegmentNumber(int segmentNumber, bool usDot = false)
  {
    std::vector<TravelSeg*>* travelSegments = _memHandle->create<std::vector<TravelSeg*> >();
    for (int i = 0; i < segmentNumber; ++i)
    {
      AirSeg* airSeg = _memHandle->create<AirSeg>();
      airSeg->pnrSegment() = i + 1;
      travelSegments->push_back(airSeg);
    }
    Itin* itin = _memHandle->create<Itin>();
    itin->setBaggageTripType(usDot ? BaggageTripType::TO_FROM_US : BaggageTripType::OTHER);
    itin->travelSeg() = *travelSegments;
    _farePath->itin() = itin;
    itin->farePath().push_back(_farePath);
    return *this;
  }

  FarePathBuilder& withBaggageResponse(std::string& baggageResponse)
  {
    _farePath->baggageResponse() = baggageResponse;
    return *this;
  }

  FarePathBuilder& withBaggageEmbargoesResponse(std::string& baggageEmbargoesResponse)
  {
    _farePath->baggageEmbargoesResponse() = baggageEmbargoesResponse;
    return *this;
  }

  FarePath* build() { return _farePath; }
};

class ExcItinBuilder
{
  TestMemHandle* _memHandle;
  ExcItin* _itin;

public:
  ExcItinBuilder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _itin = _memHandle->create<ExcItin>();
    setCalculationCurrency("USD");
  }

  ExcItinBuilder& setCalculationCurrency(const CurrencyCode& currencyCode)
  {
    _itin->calculationCurrency() = currencyCode;
    return *this;
  }

  ExcItinBuilder& addFarePath(FarePath* farePath)
  {
    _itin->farePath().push_back(farePath);
    return *this;
  }

  ExcItin* build() { return _itin; }
};

template <typename BuiltType>
class Builder
{
protected:
  BuiltType* _builtTypePtr;
  TestMemHandle* _memHandle;

public:
  Builder(TestMemHandle* memHandle)
  {
    _memHandle = memHandle;
    _builtTypePtr = _memHandle->create<BuiltType>();
  }

  BuiltType* build() { return _builtTypePtr; }
};

class ItinBuilder : public Builder<Itin>
{
public:
  ItinBuilder(TestMemHandle* memHandle) : Builder<Itin>(memHandle) {}

  ItinBuilder& addNSegmentAndComponents(unsigned int number, bool usDot = false)
  {
    std::vector<TravelSeg*>* travelSegments = _memHandle->create<std::vector<TravelSeg*>>();
    for (unsigned int i = 0; i < number; i++)
    {
      _builtTypePtr->fareComponent().push_back(_memHandle->create<FareCompInfo>());
      _builtTypePtr->fareComponent()[i]->fareCompNumber() = i + 1;
      FareMarket* fareMarket = _memHandle->create<FareMarket>();
      AirSeg* airSeg = _memHandle->create<AirSeg>();
      airSeg->pnrSegment() = i + 1;
      travelSegments->push_back(airSeg);
      fareMarket->travelSeg().push_back(airSeg);
      _builtTypePtr->fareComponent()[i]->fareMarket() = fareMarket;
    _builtTypePtr->setBaggageTripType(usDot ? BaggageTripType::TO_FROM_US : BaggageTripType::OTHER);
    _builtTypePtr->travelSeg() = *travelSegments;
    }
    return *this;
  }
};

class FareUsageBuilder : public Builder<FareUsage>
{
public:
  FareUsageBuilder(TestMemHandle* memHandle) : Builder<FareUsage>(memHandle) {}

  template <typename... Args>
  FareUsageBuilder& addTravelSegments(TravelSeg* segment, Args&&... args)
  {
    _builtTypePtr->travelSeg().push_back(segment);
    addTravelSegments(std::forward<Args>(args)...);
    return *this;
  }

  FareUsageBuilder& addTravelSegments(TravelSeg* segment)
  {
    _builtTypePtr->travelSeg().push_back(segment);
    return *this;
  }

  FareUsageBuilder& addTicketEndorseItem(PaxTypeFare* ptf, std::vector<TravelSeg*>& vts)
  {
    TicketEndorsementsInfo teInfo;
    TicketEndorsementsInfo::dummyData(teInfo);
    TicketEndorseItem teItem(teInfo, *ptf);
    _builtTypePtr->tktEndorsement().push_back(teItem);

    FareMarket* fm = _memHandle->create<FareMarket>();
    for (TravelSeg* ts : vts)
      fm->travelSeg().push_back(ts);
    ptf->fareMarket() = fm;

    return *this;
  }
};

class BaggageTravelBuilder
{
private:
  TestMemHandle* _memHandle;
  BaggageTravel* _bgTravel;
  std::vector<TravelSeg*>* _travelSegs;

public:
  BaggageTravelBuilder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _bgTravel = _memHandle->create<BaggageTravel>();
  }

  BaggageTravelBuilder& withTrx(PricingTrx* trx)
  {
    _bgTravel->_trx = trx;
    return *this;
  }

  BaggageTravelBuilder& withAllowance(OCFees* ocFees)
  {
    _bgTravel->_allowance = ocFees;
    return *this;
  }

  BaggageTravelBuilder& addCharges(OCFees* ocFees)
  {
    BaggageCharge* bgCharge = _memHandle->create<BaggageCharge>();
    bgCharge->optFee() = ocFees->optFee();
    bgCharge->subCodeInfo() = ocFees->subCodeInfo();

    _bgTravel->_chargeVector.push_back(bgCharge);
    return *this;
  }

  BaggageTravelBuilder& withSegment(FarePath* farePath, int segmentNumber)
  {
    _bgTravel->setupTravelData(*farePath);
    _bgTravel->_MSS = farePath->itin()->travelSeg().begin() + segmentNumber - 1;
    _bgTravel->updateSegmentsRange(_bgTravel->_MSS, _bgTravel->_MSS + 1);
    return *this;
  }

  BaggageTravel* build() { return _bgTravel; }
};

class OCFeesBuilder
{
private:
  TestMemHandle* _memHandle;
  OCFees* _ocFees;
  SubCodeInfo* _s5;
  OptionalServicesInfo* _s7;

public:
  OCFeesBuilder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _ocFees = _memHandle->create<OCFees>();
    _s5 = _memHandle->create<SubCodeInfo>();
    SubCodeInfo::dummyData(*_s5);
    _s7 = _memHandle->create<OptionalServicesInfo>();
    OptionalServicesInfo::dummyData(*_s7);
  }

  OCFeesBuilder& withCarrier(const CarrierCode& carrier)
  {
    _s7->carrier() = carrier;
    return *this;
  }

  OCFeesBuilder& withServiceSubTypeCode(const ServiceSubTypeCode& subType)
  {
    _s5->serviceSubTypeCode() = subType;
    return *this;
  }

  OCFeesBuilder& withFreeBaggagePcs(int32_t freeBaggagePcs)
  {
    _s7->freeBaggagePcs() = freeBaggagePcs;
    return *this;
  }

  OCFeesBuilder& withBaggageWeightUnit(int32_t baggageWeightUnit)
  {
    _s7->baggageWeightUnit() = baggageWeightUnit;
    return *this;
  }

  OCFeesBuilder& withBaggageOccurrenceFirstPc(int32_t baggageOccurrenceFirstPc)
  {
    _s7->baggageOccurrenceFirstPc() = baggageOccurrenceFirstPc;
    return *this;
  }

  OCFeesBuilder& withBaggageOccurrenceLastPc(int32_t baggageOccurrenceLastPc)
  {
    _s7->baggageOccurrenceLastPc() = baggageOccurrenceLastPc;
    return *this;
  }

  OCFeesBuilder& withAdvPurchTktIssue(const Indicator& advPurchTktIssue)
  {
    _s7->advPurchTktIssue() = advPurchTktIssue;
    return *this;
  }

  OCFeesBuilder& withNoAvailNoChargeInd(const Indicator& noAvailNoChargeInd)
  {
    _s7->notAvailNoChargeInd() = noAvailNoChargeInd;
    return *this;
  }

  OCFeesBuilder& withFormOfFeeRefundInd(const Indicator& formOfFeeRefundInd)
  {
    _s7->formOfFeeRefundInd() = formOfFeeRefundInd;
    return *this;
  }

  OCFeesBuilder& withRefundReissueInd(const Indicator& refundReissueInd)
  {
    _s7->refundReissueInd() = refundReissueInd;
    return *this;
  }

  OCFeesBuilder& withCommissionInd(const Indicator& commissionInd)
  {
    _s7->commissionInd() = commissionInd;
    return *this;
  }

  OCFeesBuilder& withInterlineInd(const Indicator& interlineInd)
  {
    _s7->interlineInd() = interlineInd;
    return *this;
  }

  OCFeesBuilder& withPaxType(const PaxTypeCode& paxType)
  {
    FarePath* fp = _memHandle->create<FarePath>();
    fp->paxType() = _memHandle->create<PaxType>();
    fp->paxType()->paxType() = paxType;
    _ocFees->farePath() = fp;
    return *this;
  }

  OCFeesBuilder& withAmount(const MoneyAmount& feeAmount, const CurrencyCode& currencyCode)
  {
    _ocFees->feeAmount() = feeAmount;
    _ocFees->feeCurrency() = currencyCode;
    return *this;
  }

  OCFeesBuilder& withOrginalAmount(const MoneyAmount& feeAmount, const CurrencyCode& currencyCode)
  {
    _ocFees->orginalFeeAmount() = feeAmount;
    _ocFees->orginalFeeCurrency() = currencyCode;
    return *this;
  }

  OCFeesBuilder& withfrequentFlyerStatus(uint16_t value)
  {
    _s7->frequentFlyerStatus() = value;
    return *this;
  }

  OCFees* build()
  {
    _ocFees->subCodeInfo() = _s5;
    _ocFees->optFee() = _s7;
    return _ocFees;
  }
};

template <class Transaction>
class TransactionBuilder
{
  Transaction* _trx;
  TestMemHandle* _memHandle;

  PricingRequest* getRequest()
  {
    if (_trx->getRequest() == 0)
      _trx->setRequest(_memHandle->create<PricingRequest>());

    return _trx->getRequest();
  }

public:
  TransactionBuilder(TestMemHandle* memHandle) : _memHandle(memHandle)
  {
    _trx = _memHandle->create<Transaction>();
    PaxType* paxType = _memHandle->create<PaxType>();
    paxType->paxType() = "ABC";
    _trx->paxType().push_back(paxType);

    Billing* billing = _memHandle->create<Billing>();
    _trx->billing() = billing;

    withAbacusAgent();
    setUsDot(false);
  }

  TransactionBuilder& withAbacusAgent()
  {
    Agent* agent = _memHandle->create<Agent>();
    Customer* customer = _memHandle->create<Customer>();
    customer->hostName() = "ABAC";
    customer->crsCarrier() = "1B";
    agent->agentTJR() = customer;
    getRequest()->ticketingAgent() = agent;
    return *this;
  }

  TransactionBuilder& setUsDot(bool isUsDot)
  {
    if (_trx->itin().empty())
      _trx->itin().push_back(_memHandle->create<Itin>());

    _trx->itin().front()->setBaggageTripType(isUsDot ? BaggageTripType::TO_FROM_US
                                                     : BaggageTripType::OTHER);
    return *this;
  }

  TransactionBuilder& setUpExcInit()
  {
    _trx->exchangeItin().front()->rexTrx() = _trx;
    _trx->setExcNonRefAmount(Money(NUC));

    return *this;
  }

  TransactionBuilder& dummyTransaction();

  TransactionBuilder& setExcTrxType(const PricingTrx::ExcTrxType& type)
  {
    _trx->setExcTrxType(type);
    return *this;
  }

  TransactionBuilder& setSchemaVersion(uint16_t major, uint16_t minor, uint16_t revision)
  {
    getRequest()->majorSchemaVersion() = major;
    getRequest()->minorSchemaVersion() = minor;
    getRequest()->revisionSchemaVersion() = revision;

    return *this;
  }

  Transaction* build() { return _trx; }
};

template <class Transaction>
TransactionBuilder<Transaction>&
TransactionBuilder<Transaction>::dummyTransaction()
{
  return *this;
}

template <>
TransactionBuilder<RefundPricingTrx>&
TransactionBuilder<RefundPricingTrx>::dummyTransaction()
{
  FarePath* farePath = FarePathBuilder(_memHandle).build();
  _trx->exchangeItin().push_back(ExcItinBuilder(_memHandle).addFarePath(farePath).build());
  RefundPermutation* refundPermutation = _memHandle->create<RefundPermutation>();

  for (PricingUnit* pu : farePath->pricingUnit())
    refundPermutation->penaltyFees()[pu] = _memHandle->create<RefundPenalty>();

  VoluntaryRefundsInfo* vri = _memHandle->insert(new VoluntaryRefundsInfo);
  vri->formOfRefund() = RefundPermutation::ANY_FORM_OF_PAYMENT;
  RefundProcessInfo* info = _memHandle->insert(new RefundProcessInfo);
  info->assign(vri, _memHandle->insert(new PaxTypeFare()), 0);
  refundPermutation->processInfos().push_back(info);

  _trx->setFullRefundWinningPermutation(*refundPermutation);

  return *this;
}

class PricingResponseFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PricingResponseFormatterTest);

  CPPUNIT_TEST(testFormatErrorResponse);
  CPPUNIT_TEST(testElectronicTicketBlank);
  CPPUNIT_TEST(testElectronicTicketR);
  CPPUNIT_TEST(testElectronicTicketN);
  CPPUNIT_TEST(testFormatResponseWhenCAT3XHasNoError);
  CPPUNIT_TEST(testFormatResponseWhenCAT3XGotError);
  CPPUNIT_TEST(testFormatResponseWhenCAT3XHasDiagnosticAndGotError);
  CPPUNIT_TEST(testFormatResponseWhenCAT3XAndPortGotError);
  CPPUNIT_TEST(testFormatResponseWhenCAT3XAndPortGotErrorAndHasDiagnostic);
  CPPUNIT_TEST(testPrepareOBFee);
  CPPUNIT_TEST(testprepareAllOBFees);
  CPPUNIT_TEST(testFormatResponseOBFees);
  CPPUNIT_TEST(testFormatResponseOBFeesRT);
  CPPUNIT_TEST(testFormatResponseOBFeesMax);
  CPPUNIT_TEST(testFormatResponseOBFeesSTA);
  CPPUNIT_TEST(testbuildTotalTTypeOBFeeAmount);
  CPPUNIT_TEST(testbuildTotalTTypeOBFeeAmountMax);
  CPPUNIT_TEST(testGetConfigOBFeeOptionMaxLimit);
  CPPUNIT_TEST(testOBFeesForBALimitNotHit);
  CPPUNIT_TEST(testOBFeesForBAZeroNoFop);
  CPPUNIT_TEST(testOBFeesForBAZeroFop);
  CPPUNIT_TEST(testOBFeesForBANonZero);
  CPPUNIT_TEST(testOBFeesForBAMultiPTCOneMax);
  CPPUNIT_TEST(testOBFeesForBAMultiPTCOneMultiMaxSecond);
  CPPUNIT_TEST(testOBFeesForBAMultiPTCOneMultiMaxFirst);
  CPPUNIT_TEST(testOBFeesForBANoChargeY);
  CPPUNIT_TEST(testOBFeesForBAClearAndGetLast);
  CPPUNIT_TEST(testSetFopBinNumber);
  CPPUNIT_TEST(testGetFeeRoundingCallControllingNationCodeReturnTrue);
  CPPUNIT_TEST(testGetFeeRoundingDoNotCallControllingNationCodeReturnTrue);
  CPPUNIT_TEST(testCabinCharReturnSame);
  CPPUNIT_TEST(testCabinCharReturnP);
  CPPUNIT_TEST(testCabinCharReturnS);
  CPPUNIT_TEST(testPrepareDifferentialwithNOdata);
  CPPUNIT_TEST(testPrepareDifferentialwithDiffDataFail);
  CPPUNIT_TEST(testPrepareDifferentialwithDiffDataPASSnoAmount);
  CPPUNIT_TEST(testPrepareDifferentialwithDiffDataPASSwithAmount);
  CPPUNIT_TEST(testPrepareDifferentialwithDiffDataPASSwithHIPAmount);
  CPPUNIT_TEST(testPrepareDifferentialwithDiffDataPASSwithHIPhigherOrigin);
  CPPUNIT_TEST(testPrepareDifferentialwithDiffDataPASSwithHIPlowOrigin);
  CPPUNIT_TEST(testPrepareHPUForNet);
  CPPUNIT_TEST(testPrepareHPUForAdj);
  CPPUNIT_TEST(testPreparePassengerInfoRetailerCode);
  CPPUNIT_TEST(testPrepareCat27TourCodeWhenCat35EmptyTourCode);
  CPPUNIT_TEST(testPrepareCat27TourCodeWhenCat35TourCode);
  CPPUNIT_TEST(testPrepareCat27TourCodeWhenEmptyTourCode);
  CPPUNIT_TEST(testPrepareCat27TourCodeWhenTourCode);
  CPPUNIT_TEST(testPrepareCat27PaxFareNumber);
  CPPUNIT_TEST(testChangeFeesForRefund_minimum);
  CPPUNIT_TEST(testChangeFeesForRefund_zero);
  CPPUNIT_TEST(testChangeFeesForRefund_nonRef);
  CPPUNIT_TEST(testChangeFeesForRefund_multiple);
  CPPUNIT_TEST(testFormatOCFeesResponse_ValidGroup);
  CPPUNIT_TEST(testFormatOCFeesResponse_EmptyGroup);
  CPPUNIT_TEST(testFormatOCFeesResponse_InvalidGroup);
  CPPUNIT_TEST(testFormatOCFeesResponse_NAGroup);
  CPPUNIT_TEST(testFormatOCFees);
  CPPUNIT_TEST(testFormatOCFees_UN);
  CPPUNIT_TEST(testFormatOCFees_SAGroup);
  CPPUNIT_TEST(testFormatOCFees_XXGroup);
  CPPUNIT_TEST(testFormatOCFees_Historical);
  CPPUNIT_TEST(testFormatOCFees_Historical_EmptyGroup);
  CPPUNIT_TEST(testFormatOCFees_Historical_SAGroup);
  CPPUNIT_TEST(testFormatOCFees_DispOnly);
  CPPUNIT_TEST(testFormatOCFees_DispOnly_SAGroup);
  CPPUNIT_TEST(testFormatOCFees_NumberOfFeesBelowMax);
  CPPUNIT_TEST(testFormatOCFees_NumberOfFeesEqualToMax);
  CPPUNIT_TEST(testFormatOCFees_NumberOfFeesAboveMax);
  CPPUNIT_TEST(testFormatOCFees_AllSegsConfirmed);
  CPPUNIT_TEST(testFormatOCFees_AllSegsUnconfirmed);
  CPPUNIT_TEST(testFormatOCFees_NotAllSegsConfirmed);
  CPPUNIT_TEST(testFormatOCFeesGroups);
  CPPUNIT_TEST(testFormatOCFeesGroups_SAGroup);
  CPPUNIT_TEST(testFormatOCFeesGroups_AllDispOnly);
  CPPUNIT_TEST(testFormatOCFeesGroups_AllDispOnly_SAGroup);
  CPPUNIT_TEST(testFormatOCFeesGroups_DispOnly);
  CPPUNIT_TEST(testFormatOCFeesGroups_DispOnly_SAGroup_2Fees);
  CPPUNIT_TEST(testFormatOCFeesGroups_DispOnly_SAGroup);
  CPPUNIT_TEST(testFormatOCFeesGroups_AdvPurchase);
  CPPUNIT_TEST(testFormatOCFeesGroups_NotAdvPurchase);
  CPPUNIT_TEST(testFormatOCFeesGroups_AdvPurchase_SA);
  CPPUNIT_TEST(testFormatOCFeesGroups_NonRefundable_N);
  CPPUNIT_TEST(testFormatOCFeesGroups_NonRefundable_R);
  CPPUNIT_TEST(testFormatOCFeesGroups_Refundable);
  CPPUNIT_TEST(testFormatOCFeesGroups_NonRefundable_SA);
  CPPUNIT_TEST(test_addPrefixWarningForOCTrailer_False);
  CPPUNIT_TEST(testIsOcFeesTrxDisplayOnly);
  CPPUNIT_TEST(testIsOcFeesTrxDisplayOnly_DateOverride);
  CPPUNIT_TEST(testIsOcFeesTrxDisplayOnly_LowFare);

  CPPUNIT_TEST(testDisplayTfdPsc_Pass_NegFareWithNegFareRestExtSeq);
  CPPUNIT_TEST(testDisplayTfdPsc_Fail_NegFareWithNoNegFareRestExtSeq);

  CPPUNIT_TEST(testPrepareTfdpsc_FareBasisIndA);
  CPPUNIT_TEST(testPrepareTfdpsc_FareBasisIndF);
  CPPUNIT_TEST(testPrepareTfdpsc_FareBasisIndA_MatchOnLast);
  CPPUNIT_TEST(testPrepareTfdpsc_FareBasisIndA_MatchOnMiddle);
  CPPUNIT_TEST(testPrepareTfdpsc_FareBasisIndA_DontMatch);

  CPPUNIT_TEST(testPrepareTfdpsc_FareBasisIndBWithUnfbc);
  CPPUNIT_TEST(testPrepareTfdpsc_FareBasisIndBWithNoUnfbc);

  CPPUNIT_TEST(testPrepareFareVendorSource_NonCat35Fare);
  CPPUNIT_TEST(testPrepareFareVendorSource_Cat35WithNRTicketIndFalse);
  CPPUNIT_TEST(testPrepareFareVendorSource_FareBasisIndNWithNoExtSeq);
  CPPUNIT_TEST(testPrepareFareVendorSource_FareBasisIndBWithUnfbc);
  CPPUNIT_TEST(testPrepareFareVendorSource_FareBasisIndBWithNoUnfbc);
  CPPUNIT_TEST(testPrepareFareVendorSource_ATP);
  CPPUNIT_TEST(testPrepareFareVendorSource_5KAD);
  CPPUNIT_TEST(testPrepareFareVendorSource_SMFA);
  CPPUNIT_TEST(testPrepareFareVendorSource_SMFC);

  CPPUNIT_TEST(testIsTimeOutBeforeStartOCFees_True);
  CPPUNIT_TEST(testIsTimeOutBeforeStartOCFees_True_ABACUS);
  CPPUNIT_TEST(testTimeOutMaxCharCountOCFeesStatusALL_WP_True);
  CPPUNIT_TEST(testTimeOutMaxCharCountOCFeesStatusALL_WPAE_True);

  CPPUNIT_TEST(testTimeOutMaxCharCountOCFeesStatusUQ_True);
  CPPUNIT_TEST(testAnyTimeOutMaxCharCountIssue_OCFee_returned);
  CPPUNIT_TEST(testAnyTimeOutMaxCharCountIssue_NO_OCFee_returned);
  CPPUNIT_TEST(test_isR7TuningAndWP_flase);
  CPPUNIT_TEST(test_isR7TuningAndWP_true);
  CPPUNIT_TEST(test_isR7TuningAndWP_true_withItinTrue);
  CPPUNIT_TEST(test_isR7TuningAndWP_true_prefixTrue);
  CPPUNIT_TEST(test_isR7TuningAndWP_true_withItinTrue_prefixTrue);
  CPPUNIT_TEST(test_formatOCHeaderMsg);
  CPPUNIT_TEST(test_isGenericTrailer_Empty);
  CPPUNIT_TEST(test_isGenericTrailer_True_No_Valid_Group);
  CPPUNIT_TEST(test_isGenericTrailer_true_Unconfirm);
  CPPUNIT_TEST(test_builTrailerOCF);
  CPPUNIT_TEST(test_createOCGSection);
  CPPUNIT_TEST(test_checkIfAnyGroupValid_false);
  CPPUNIT_TEST(test_checkIfAnyGroupValid_true);
  CPPUNIT_TEST(test_replaceSpecialCharInDisplay);
  CPPUNIT_TEST(test_buildST1data_Empty);
  CPPUNIT_TEST(test_buildST1data_Not_Empty);
  CPPUNIT_TEST(test_buildST1data_Same_Ind);
  CPPUNIT_TEST(testGetFootnoteByHirarchyOrderWhenInformationOnly);
  CPPUNIT_TEST(testGetFootnoteByHirarchyOrderWhenDisplayOnly);
  CPPUNIT_TEST(testGetFootnoteByHirarchyOrderWhenNonRefundableN);
  CPPUNIT_TEST(testGetFootnoteByHirarchyOrderWhenNonRefundableR);
  CPPUNIT_TEST(testGetFootnoteByHirarchyOrderWhenPurchSameTimeAsTicket);
  CPPUNIT_TEST(testGetFootnoteByHirarchyOrderForEachItem);
  CPPUNIT_TEST(testGetFootnoteByHirarchyOrderForEntireUnit);
  CPPUNIT_TEST(testGetFootnoteByHirarchyOrderForEntireTicket);
  CPPUNIT_TEST(test_fotmatOCTrailer);
  CPPUNIT_TEST(test_formatOCGenericMsg);
  CPPUNIT_TEST(test_checkPNRSegmentNumberLogic_without_Arunk);
  CPPUNIT_TEST(test_checkPNRSegmentNumberLogic_with_Arunk);
  CPPUNIT_TEST(test_checkPNRSegmentNumberLogic_with_middle_Arunk);
  CPPUNIT_TEST(test_checkPNRSegmentNumberLogic_with_three_segs);

  CPPUNIT_TEST(test_buildPNRDataWithIndex_0);
  CPPUNIT_TEST(test_buildPNRDataWithIndex_0_WithBaseCurr);
  CPPUNIT_TEST(test_buildPNRData_with_index_value);

  CPPUNIT_TEST(test_buildPNRDataForNonDisplayFees_AX1_Tag);

  CPPUNIT_TEST(testPrepareValidatingCarrierAttr_EffDate);
  CPPUNIT_TEST(testPrepareValidatingCarrierAttr_EffDate_BadCarrier);

  CPPUNIT_TEST(testPreparePricingUnitPlusUps_hrt);

  CPPUNIT_TEST(testIsAncillariesGuarantee_For_AutoPriced_Solution);
  CPPUNIT_TEST(testIsAncillariesNonGuarantee_For_WPNQF_IgnoreFuelSurcharge);
  CPPUNIT_TEST(testIsAncillariesNonGuarantee_For_WPQFareBasisCode_3_point_5);
  CPPUNIT_TEST(testIsAncillariesNonGuarantee_For_WPOC_Bx_BookingCodeOverride_request);
  CPPUNIT_TEST(testIsAncillariesNonGuarantee_For_WPR_BSR_SecondRateAmountOverride);
  CPPUNIT_TEST(testIsAncillariesNonGuarantee_For_WPR_BSR_FirstRateAmountOverride);
  CPPUNIT_TEST(testIsAncillariesNonGuarantee_For_WPXPTS_PaperTktSurchargeOverride);
  CPPUNIT_TEST(testIsAncillariesNonGuarantee_For_WPQDP_3_point_75);
  CPPUNIT_TEST(testIsAncillariesNonGuarantee_For_WPQDA_3_point_75);
  CPPUNIT_TEST(testIsAncillariesNonGuarantee_For_WPC_AA_GoverningCxrOverride);
  CPPUNIT_TEST(testIsAncillariesNonGuarantee_For_WPC_YY_IndustryCxrOverride);

  CPPUNIT_TEST(testIsTaxExempted_TN);
  CPPUNIT_TEST(testIsTaxExempted_TE);
  CPPUNIT_TEST(testIsTaxExempted_TE_UO);
  CPPUNIT_TEST(testIsTaxExempted_TE_UOE);
  CPPUNIT_TEST(testIsTaxExempted_TE_UOD);

  CPPUNIT_TEST(testSetEndTaxOnOcIterator_OneItem);
  CPPUNIT_TEST(testSetEndTaxOnOcIterator_SixteenItems);

  CPPUNIT_TEST(test_buildPNRData_PADIS_WPAE);
  CPPUNIT_TEST(test_formatOCFeesLine);

  CPPUNIT_TEST(test_buildCarryOnAllowanceBDI);
  CPPUNIT_TEST(test_buildCarryOnAllowanceBDI_different_carrier);
  CPPUNIT_TEST(test_buildCarryOnAllowanceBDI_different_freeBaggagePcs);
  CPPUNIT_TEST(test_buildCarryOnAllowanceBDI_different_baggageWeightUnit);
  CPPUNIT_TEST(test_buildCarryOnAllowanceBDI_different_baggageOccurrenceFirstPc);
  CPPUNIT_TEST(test_buildCarryOnAllowanceBDI_different_baggageOccurrenceLastPc);
  CPPUNIT_TEST(test_buildCarryOnChargesBDI);
  CPPUNIT_TEST(test_buildCarryOnChargesBDI_different_advPurchTktIssue);
  CPPUNIT_TEST(test_buildCarryOnChargesBDI_different_notAvailNoChargeInd);
  CPPUNIT_TEST(test_buildCarryOnChargesBDI_different_formOfFeeRefundInd);
  CPPUNIT_TEST(test_buildCarryOnChargesBDI_different_refundReissueInd);
  CPPUNIT_TEST(test_buildCarryOnChargesBDI_different_commissionInd);
  CPPUNIT_TEST(test_buildCarryOnChargesBDI_different_interlineInd);

  CPPUNIT_TEST(buildOSC_mandatory);
  CPPUNIT_TEST(buildOSC_subGroup_emptyDefinition);
  CPPUNIT_TEST(buildOSC_subGroup);
  CPPUNIT_TEST(buildOSC_optional);

  CPPUNIT_TEST(test_buildExtendedSubCodeKey_baggageProvision);
  CPPUNIT_TEST(test_buildExtendedSubCodeKey);
  CPPUNIT_TEST(test_buildExtendedSubCodeKey_merchManager);

  CPPUNIT_TEST(test_buildSubCodeDescription_firstEmpty);
  CPPUNIT_TEST(test_buildSubCodeDescription_firstNotFound);
  CPPUNIT_TEST(test_buildSubCodeDescription_first);
  CPPUNIT_TEST(test_buildSubCodeDescription_secondNotFound);
  CPPUNIT_TEST(test_buildSubCodeDescription_firstAndSecond);

  CPPUNIT_TEST(test_buildSubCodeRestrictions);

  CPPUNIT_TEST(test_buildSubCodeRestrictionWithAlternativeUnits_none);
  CPPUNIT_TEST(test_buildSubCodeRestrictionWithAlternativeUnits_one);
  CPPUNIT_TEST(test_buildSubCodeRestrictionWithAlternativeUnits_both);

  CPPUNIT_TEST(test_buildSubCodeRestriction);

  CPPUNIT_TEST(test_getSubCodeRestrictions);
  CPPUNIT_TEST(test_getSubCodeRestrictions_empty);
  CPPUNIT_TEST(test_getSubCodeRestrictions_notFound);

  CPPUNIT_TEST(test_SubCodeRestrictionsComparator_true);
  CPPUNIT_TEST(test_SubCodeRestrictionsComparator_true_alternative);
  CPPUNIT_TEST(test_SubCodeRestrictionsComparator_false_unit);
  CPPUNIT_TEST(test_SubCodeRestrictionsComparator_false_limit);

  CPPUNIT_TEST(test_buildBDI_BaggageChargesWithFrequentFlyerWarning);
  CPPUNIT_TEST(test_buildBDI_A_schema_1_1_0);
  CPPUNIT_TEST(test_buildBDI_A_schema_1_1_1);
  CPPUNIT_TEST(test_buildBDI_CC_schema_1_1_0);
  CPPUNIT_TEST(test_buildBDI_CC_schema_1_1_1);
  CPPUNIT_TEST(test_buildPFF_schema_1_1_0);
  CPPUNIT_TEST(test_buildPFF_schema_1_1_1);
  CPPUNIT_TEST(test_buildQ00);

  CPPUNIT_TEST(test_USI_usDot_schema_1_1_0);
  CPPUNIT_TEST(test_USI_usDot_schema_1_1_1);
  CPPUNIT_TEST(test_USI_NotUsDot_schema_1_1_0);
  CPPUNIT_TEST(test_USI_NotUsDot_schema_1_1_1);

  CPPUNIT_TEST(test_structuredFareRuleResponse_AppropriateNumberOfFareComponentTagsExists);

  CPPUNIT_TEST(test_preparePassengerForFullRefund_USI_usDot);
  CPPUNIT_TEST(test_preparePassengerForFUllRefund_USI_notUsDot);

  CPPUNIT_TEST(test_addAdditionalPaxInfo_USI_usDot);
  CPPUNIT_TEST(test_addAdditionalPaxInfo_USI_notUsDot);

  CPPUNIT_TEST(test_addUsDotItinIndicator_TO_FROM_US);
  CPPUNIT_TEST(test_addUsDotItinIndicator_TO_FROM_CA);
  CPPUNIT_TEST(test_addUsDotItinIndicator_WHOLLY_WITHIN_US);
  CPPUNIT_TEST(test_addUsDotItinIndicator_WHOLLY_WITHIN_CA);
  CPPUNIT_TEST(test_addUsDotItinIndicator_BETWEEN_US_CA);
  CPPUNIT_TEST(test_addUsDotItinIndicator_OTHER);

  CPPUNIT_TEST(test_validSchamaVersion_AR_EXC_TRX);
  CPPUNIT_TEST(test_validSchamaVersion_AF_EXC_TRX);
  CPPUNIT_TEST(test_validSchamaVersion_PORT_EXC_TRX);
  CPPUNIT_TEST(test_validSchamaVersion_ME_DIAG_TRX);
  CPPUNIT_TEST(test_validSchamaVersion_EXC1_WITHIN_ME);
  CPPUNIT_TEST(test_validSchamaVersion_EXC2_WITHIN_ME);
  CPPUNIT_TEST(test_validSchamaVersion_NEW_WITHIN_ME);
  CPPUNIT_TEST(test_validSchemaVersion_notExchangeTrx);

  CPPUNIT_TEST(test_isOldPricingTrx_PricingTrx);
  CPPUNIT_TEST(test_isOldPricingTrx_MultiExchangeTrx_newPricingTrx);
  CPPUNIT_TEST(test_isOldPricingTrx_MultiExchangeTrx_excPricingTrx1);
  CPPUNIT_TEST(test_isOldPricingTrx_MultiExchangeTrx_excPricingTrx2);

  CPPUNIT_TEST(test_sideTrip_arunk_seg);
  CPPUNIT_TEST(test_sideTrip_seg_seg);
  CPPUNIT_TEST(test_sideTrip_seg_arunk);
  CPPUNIT_TEST(test_sideTrip_seg_arunk_seg);
  CPPUNIT_TEST(test_sideTrip_arunk_seg_arunk);

  CPPUNIT_TEST(test_checkForStructuredData_XML_RESPONSE_OFF_US_DOT);
  CPPUNIT_TEST(test_checkForStructuredData_XML_RESPONSE_OFF_NON_US_DOT);
  CPPUNIT_TEST(test_checkForStructuredData_XML_RESPONSE_ITIN_EMPTY);
  CPPUNIT_TEST(test_checkForStructuredData_XML_RESPONSE_US_DOT_PricingTrx_1_0_1);
  CPPUNIT_TEST(test_checkForStructuredData_XML_RESPONSE_US_DOT_PricingTrx_1_1_0);
  CPPUNIT_TEST(test_checkForStructuredData_XML_RESPONSE_US_DOT_PricingTrx_1_1_1);
  CPPUNIT_TEST(test_checkForStructuredData_XML_RESPONSE_US_DOT_ExchangeTrx);
  CPPUNIT_TEST(test_checkForStructuredData_XML_RESPONSE_NON_US_DOT_PricingTrx_1_0_1);
  CPPUNIT_TEST(test_checkForStructuredData_XML_RESPONSE_NON_US_DOT_PricingTrx_1_1_0);
  CPPUNIT_TEST(test_checkForStructuredData_XML_RESPONSE_NON_US_DOT_PricingTrx_1_1_1);
  CPPUNIT_TEST(test_checkForStructuredData_XML_RESPONSE_NON_US_DOT_EchangeTrx);

  CPPUNIT_TEST(test_formatGreenScreenResponse_usdot);
  CPPUNIT_TEST(test_formatGreenScreenResponse_nonusdot);
  CPPUNIT_TEST(test_formatGreenScreenResponse_maxBuffSize1);
  CPPUNIT_TEST(test_formatGreenScreenResponse_maxBuffSize2);

  CPPUNIT_TEST(testIsCategory25AppliesWhenHasBaseCat1);
  CPPUNIT_TEST(testIsCategory25AppliesWhenHasBaseCat1ButNoCatRuleInfo);
  CPPUNIT_TEST(testIsCategory25AppliesWhenHasntBaseCat1);
  CPPUNIT_TEST(testIsCategory25AppliesWhenHasBaseCat10);
  CPPUNIT_TEST(testIsCategory25AppliesWhenHasntBaseCat10);
  CPPUNIT_TEST(testIsCategory25AppliesWhenNoBaseCategory);

  CPPUNIT_TEST(testGetLowestObFeeAmountWhenNoCurrency);
  CPPUNIT_TEST(testGetLowestObFeeAmountWhenCurrency);
  CPPUNIT_TEST(testCalculateResidualObFeeAmountWhenNoResidual);
  CPPUNIT_TEST(testCalculateResidualObFeeAmountWhenResidualFop);
  CPPUNIT_TEST(testCalculateResidualObFeeAmountWhenResidual);
  CPPUNIT_TEST(testCalculateResidualObFeeAmountWhenResidualFopNegative);

  CPPUNIT_TEST(testCalculateObFeeAmountFromAmountMaxWhenSameCur);
  CPPUNIT_TEST(testComputeMaximumOBFeesPercentWhenNoOB);
  CPPUNIT_TEST(testComputeMaximumOBFeesPercentWhenMax);

  CPPUNIT_TEST(test_buildParticipatingCarrier_STD);
  CPPUNIT_TEST(test_buildParticipatingCarrier_3PT);
  CPPUNIT_TEST(test_buildParticipatingCarrier_PPR);
  CPPUNIT_TEST(test_buildValidatingCarrier_Default);
  CPPUNIT_TEST(test_buildValidatingCarrier_MaxPcx);
  CPPUNIT_TEST(test_buildValidatingCarrier_Alternate);
  CPPUNIT_TEST(test_prepareValidatingCarrierLists_OneSettlementPlan);
  CPPUNIT_TEST(test_prepareValidatingCarrierLists_MultiSp);
  CPPUNIT_TEST(test_prepareValidatingCarrierLists_InhibitPq);
  CPPUNIT_TEST(test_buildValidatingCarrierList_VC_Vector_Empty);
  CPPUNIT_TEST(test_buildValidatingCarrierList_DefaultVC_Empty_VCvector_HasOneVC);
  CPPUNIT_TEST(test_buildValidatingCarrierList_DefaultVC_Empty_VCvector_HasMoreThanOneVC);
  CPPUNIT_TEST(test_buildValidatingCarrierList_DefaultVC_And_VCvector_HasMoreThanOneVC);
  CPPUNIT_TEST(test_buildValidatingCarrierList_DefaultVC_Empty_VCvector_HasMoreThanOneNeutralVC);
  CPPUNIT_TEST(test_buildValidatingCarrierList_NoValidatingCxr);
  CPPUNIT_TEST(test_buildValidatingCarrierList_NoValidatingCxrMap);

  CPPUNIT_TEST(test_buildValidatingCarrier_Default_MultiSp);
  CPPUNIT_TEST(test_buildValidatingCarrier_MaxPcx_MultiSp);
  CPPUNIT_TEST(test_buildValidatingCarrier_Alternate_MultiSp);
  CPPUNIT_TEST(test_buildValidatingCarrierList_NoItin_MultiSp);
  CPPUNIT_TEST(test_buildValidatingCarrierList_NoGsaDataForSp_MultiSp);
  CPPUNIT_TEST(test_buildValidatingCarrierList_DefaultVC_And_VCvector_HasMoreThanOneVC_MultiSp);
  CPPUNIT_TEST(test_buildValidatingCarrierList_DefaultAndNoAlternates_MultiSP);
  CPPUNIT_TEST(test_buildValidatingCarrierList_DefaultAndNoAlternates2_MultiSP);
  //CPPUNIT_TEST(test_buildValidatingCarrierList_AlternateOnly_MultiSP);
  CPPUNIT_TEST(test_buildValidatingCarrierList_NoValidatingCxr_MultiSp);
  CPPUNIT_TEST(test_isAnyDefaultCarrierEmpty);

  CPPUNIT_TEST(testAddFareBrandDetailsNotPbbRequest);
  CPPUNIT_TEST(testAddFareBrandDetailsPbbRequest);
  CPPUNIT_TEST(testAddFareBrandDetailsBrandProgramEpmpty);
  CPPUNIT_TEST(testPrepareMaxPenaltyResponse1);
  CPPUNIT_TEST(testPrepareMaxPenaltyResponse2);

  CPPUNIT_TEST(testAbacusWPwithSpecificFBC_DefaultErrorResponse);
  CPPUNIT_TEST(testAbacusWPwithSpecificFBC_AllFaresFailOnRules);
  CPPUNIT_TEST(testAbacusWPwithSpecificFBC_BookingCodeValidationFailed);
  CPPUNIT_TEST(testAbacusWPwithSpecificFBC_FareBasisNotAvailable);

  CPPUNIT_TEST(test_constructElementVCC_WithNoDec);
  CPPUNIT_TEST(test_constructElementVCC_WithDec);
  CPPUNIT_TEST(test_prepareMarkupAndCommissionAmount_WithMarkUpAndCommission);
  CPPUNIT_TEST(test_prepareMarkupAndCommissionAmount_WithMarkUpOnly);
  CPPUNIT_TEST(test_prepareMarkupAndCommissionAmount_WithCommissionOnly);
  CPPUNIT_TEST(test_prepareMarkupAndCommissionAmount_WithIncorrectMarkUp);
  CPPUNIT_TEST(test_prepareMarkupAndCommissionAmount_WithMarkUpNoComm);
  CPPUNIT_TEST(test_prepareCommissionForValidatingCarriers_NoAgencyCommission);
  CPPUNIT_TEST(test_prepareCommissionForValidatingCarriers_OneAgencyCommission);
  CPPUNIT_TEST(test_prepareCommissionForValidatingCarriers_OneAgencyCommission_withKP);
  CPPUNIT_TEST(test_prepareCommissionForValidatingCarriers_TwoSameAgencyCommission);
  CPPUNIT_TEST(test_prepareCommissionForValidatingCarriers_TwoSameAgencyCommission_withKP);
  CPPUNIT_TEST(test_prepareCommissionForValidatingCarriers_TwoDiffAgencyCommission);
  CPPUNIT_TEST(test_prepareCommissionForValidatingCarriers_TwoDiffAgencyCommission_withKP);
  CPPUNIT_TEST(test_prepareCommissionForValidatingCarriers_TwoDiffAgencyWithSameCommission);

  CPPUNIT_TEST(testPrepareAdjustedCalcTotalWithORG);
  CPPUNIT_TEST(testPrepareAdjustedCalcTotalWithoutORG);
  CPPUNIT_TEST(testPrepareAdjustedCalcTotalDiffCurrency);
  CPPUNIT_TEST(testPrepareAdjustedCalcTotalWithTax);
  CPPUNIT_TEST(testPrepareHPSItemsSingleItem);
  CPPUNIT_TEST(testPrepareHPSItemsDualItems);
  CPPUNIT_TEST(testPrepareHPSItemsMultiItems);

  CPPUNIT_TEST(testGetCommissionSourceIndicator_AMC);
  CPPUNIT_TEST(testGetCommissionSourceIndicator_Cat35CommissionPercent);
  CPPUNIT_TEST(testGetCommissionSourceIndicator_Cat35CommissionAmount_withKP);
  CPPUNIT_TEST(testGetCommissionSourceIndicator_Manual);
  CPPUNIT_TEST(testGetCommissionSourceIndicator_NoCommission);

  CPPUNIT_TEST(testprepareSegments_WithServiceFee);
  CPPUNIT_TEST(testprepareSegments_WithoutServiceFee);

  CPPUNIT_TEST_SUITE_END();

private:
  PricingResponseFormatter* _formatter;
  XMLConstruct* _construct;
  PricingResponseFormatter* _respFormatter;
  RexPricingTrx* _rexTrx;
  StructuredRuleTrx* _structRuleTrx;
  ExchangePricingTrx* _excTrx;
  FareCalcCollector* _rexFareCalcCollector;
  FareCalcCollector* _excFareCalcCollector;
  PricingTrx* _pTrx;
  CountrySettlementPlanInfo* _cspi;
  Itin* _itin;
  FarePath* _farePath;
  Agent* _pAgent;
  PricingRequest* _pRequest;
  PricingOptions* _pOptions;
  MockTseServer* _mTseServer;
  TestMemHandle _memH;
  Billing* _pBilling;
  TicketingFeesInfo* _feeInfo;
  CalcTotals* _calcTotals;

public:
  class PricingResponseFormatterMock : public PricingResponseFormatter
  {
  public:
    void prepareSummary(PricingTrx& pricingTrx,
                        FareCalcCollector& fareCalcCollector,
                        XMLConstruct& construct,
                        const char* itinNumber = 0)
    {
      construct.openElement(xml2::SummaryInfo);
      construct.closeElement();
    }

    void prepareRexSummary(RexPricingTrx* rexTrx,
                           XMLConstruct& construct,
                           FareCalcCollector& originalFCC)
    {
      construct.openElement(xml2::SummaryInfo);
      construct.closeElement();
    }

    void prepareAgent(PricingTrx& pricingTrx, XMLConstruct& construct)
    {
      construct.openElement(xml2::TicketingAgentInfo);
      construct.closeElement();
    }

    void prepareBilling(PricingTrx& pricingTrx,
                        FareCalcCollector& fareCalcCollector,
                        XMLConstruct& construct)
    {
      construct.openElement(xml2::BillingInformation);
      construct.closeElement();
    }
    void prepareUnflownItinPriceInfo(RexPricingTrx& rexTrx, XMLConstruct& construct)
    {
      construct.openElement(xml2::UnflownItinPriceInfo);
      construct.closeElement();
    }
    void prepareErrorMessage(PricingTrx& pricingTrx,
                             XMLConstruct& construct,
                             ErrorResponseException::ErrorResponseCode errCode,
                             const std::string& msgText)
    {
      construct.openElement(xml2::MessageInformation);
      construct.addAttributeShort(xml2::MessageFailCode, Message::errCode(errCode));
      construct.closeElement();
    }
    void prepareResponseText(const std::string& responseString,
                             XMLConstruct& construct,
                             bool noSizelimit = false)
    {
      construct.openElement(xml2::MessageInformation);
      construct.addAttributeChar(xml2::MessageType, Message::TYPE_GENERAL);
      construct.addAttribute(xml2::MessageText, responseString);
      construct.closeElement();
    }

    void getGlobalDirection(const PricingTrx* trx,
                            DateTime travelDate,
                            const std::vector<TravelSeg*>& tvlSegs,
                            GlobalDirection& globalDir) const
    {
    }

    bool addPrefixWarningForOCTrailer(PricingTrx& pricingTrx, bool warning) { return false; }
    DateTime calculatePurchaseByDate(const PricingTrx& pricingTrx) { return DateTime::localTime(); }
  };

  void setUp()
  {
    _mTseServer = _memH.insert(new MockTseServer);

    Message::fillMsgErrCodeMap();
    _memH.create<MyDataHandle>();
    _formatter = _memH.insert(new PricingResponseFormatter);
    _construct = _memH.insert(new XMLConstruct);
    _respFormatter = _memH.insert(new PricingResponseFormatterMock);
    _rexTrx = _memH.insert(new RexPricingTrx);
    _structRuleTrx = _memH.insert(new StructuredRuleTrx);
    _excTrx = _memH.insert(new ExchangePricingTrx);
    _rexFareCalcCollector = _memH.insert(new FareCalcCollector);
    _excFareCalcCollector = _memH.insert(new FareCalcCollector);
    _pTrx = _memH.insert(new PricingTrx);
    _pRequest = _memH.insert(new PricingRequest);
    _pOptions = _memH.insert(new PricingOptions);
    _pAgent = _memH.insert(new Agent);
    _pBilling = _memH.insert(new Billing);
    _pOptions->currencyOverride() = "USD";
    _itin = _memH.insert(new Itin);
    _farePath = _memH.insert(new FarePath);
    _cspi = _memH.insert(new CountrySettlementPlanInfo);
    _pRequest->ticketingAgent() = _pAgent;
    _rexTrx->setOptions(_pOptions);
    _rexTrx->setRequest(_pRequest);

    _pTrx->fareCalcConfig() = FareCalcConfigBuilder(&_memH).build();
    _pTrx->setRequest(_pRequest);
    _pTrx->setOptions(_pOptions);
    _pTrx->itin().push_back(_itin);
    _pTrx->billing() = _pBilling;
    _pTrx->ticketingDate() = DateTime::localTime();
    _pTrx->countrySettlementPlanInfo() = _cspi;
    _pTrx->setOptions(_pOptions);
    _pTrx->setRequest(_pRequest);

    _structRuleTrx->fareCalcConfig() = FareCalcConfigBuilder(&_memH).build();
    _structRuleTrx->setRequest(_pRequest);
    _structRuleTrx->setOptions(_pOptions);
    _structRuleTrx->itin().push_back(_itin);
    _structRuleTrx->billing() = _pBilling;
    _structRuleTrx->ticketingDate() = DateTime::localTime();
    _structRuleTrx->countrySettlementPlanInfo() = _cspi;
    _structRuleTrx->setOptions(_memH.create<PricingOptions>());
    _structRuleTrx->getOptions()->currencyOverride() = "USD";
    _structRuleTrx->setRequest(_memH.create<PricingRequest>());
    _structRuleTrx->getRequest()->markAsSFR();
    _structRuleTrx->getRequest()->ticketingAgent() = _pAgent;

    _pRequest->ticketingDT() = DateTime::localTime();

    _itin->farePath().push_back(new FarePath);

    _feeInfo = _memH.create<TicketingFeesInfo>();
    _calcTotals = _memH.create<CalcTotals>();
    TestConfigInitializer::setValue("MAX_NUMBER_OF_FEES", 3, "SERVICE_FEES_SVC", true);
  }

  void tearDown() { _memH.clear(); }

  void initAllowanceBaggageTravels(std::vector<const BaggageTravel*>& baggageTravels,
                                   OCFees* ocFees1,
                                   OCFees* ocFees2,
                                   OCFees* ocFees3)
  {
    FarePath* farePath = FarePathBuilder(&_memH).withSegmentNumber(3).build();

    BaggageTravel* bgTravel1 = BaggageTravelBuilder(&_memH)
                                   .withSegment(farePath, 1)
                                   .withAllowance(ocFees1)
                                   .withTrx(_pTrx)
                                   .build();
    BaggageTravel* bgTravel2 = BaggageTravelBuilder(&_memH)
                                   .withSegment(farePath, 2)
                                   .withAllowance(ocFees2)
                                   .withTrx(_pTrx)
                                   .build();
    BaggageTravel* bgTravel3 = BaggageTravelBuilder(&_memH)
                                   .withSegment(farePath, 3)
                                   .withAllowance(ocFees3)
                                   .withTrx(_pTrx)
                                   .build();

    baggageTravels.push_back(bgTravel1);
    baggageTravels.push_back(bgTravel2);
    baggageTravels.push_back(bgTravel3);
  }

  void initChargesBaggageTravels(std::vector<const BaggageTravel*>& baggageTravels,
                                 OCFees* ocFees1,
                                 OCFees* ocFees2,
                                 OCFees* ocFees3)
  {
    FarePath* farePath = FarePathBuilder(&_memH).withSegmentNumber(3).build();

    BaggageTravel* bgTravel1 = BaggageTravelBuilder(&_memH)
                                   .withSegment(farePath, 1)
                                   .addCharges(ocFees1)
                                   .addCharges(ocFees2)
                                   .withTrx(_pTrx)
                                   .build();
    BaggageTravel* bgTravel2 = BaggageTravelBuilder(&_memH)
                                   .withSegment(farePath, 2)
                                   .addCharges(ocFees2)
                                   .addCharges(ocFees3)
                                   .withTrx(_pTrx)
                                   .build();
    BaggageTravel* bgTravel3 = BaggageTravelBuilder(&_memH)
                                   .withSegment(farePath, 3)
                                   .addCharges(ocFees3)
                                   .withTrx(_pTrx)
                                   .build();

    baggageTravels.push_back(bgTravel1);
    baggageTravels.push_back(bgTravel2);
    baggageTravels.push_back(bgTravel3);
  }

  void testFormatErrorResponse()
  {
    _pTrx->ticketingDate() = DateTime::localTime();

    ErrorResponseException ex1(ErrorResponseException::NO_FARE_FOR_CLASS_USED);
    ErrorResponseException ex2(ErrorResponseException::MAX_NUMBER_COMBOS_EXCEEDED);
    ErrorResponseException ex3(ErrorResponseException::NO_FARE_VALID_FOR_PSGR_TYPE);
    ErrorResponseException ex4(ErrorResponseException::EXCEEDS_ALLOWED_SEGS_CHANGED_CLASS_FLIGHTS);

    // Negotiated Fare IT/BT ticketing codes start at 5500

    ErrorResponseException ex5(ErrorResponseException::UTAT_NET_SELLING_AMOUNTS_CONFLICT);
    ErrorResponseException ex6(ErrorResponseException::UTAT_MULTIPLE_TOUR_CODES);
    ErrorResponseException ex7(ErrorResponseException::UTAT_TOUR_CODE_NOT_FOUND);

    // SAX parser
    ErrorResponseException ex8(ErrorResponseException::SAX_PARSER_FAILURE);

    // Limitations error start at 9501
    ErrorResponseException ex9(ErrorResponseException::LMT_ISSUE_SEP_TKTS_INTL_SURFACE_RESTR);
    // All system related exceptions like std::bad_aloc
    // can be passed as unknow exception
    ErrorResponseException ex10(ErrorResponseException::UNKNOWN_EXCEPTION);
    ErrorResponseException ex11(ErrorResponseException::SYSTEM_EXCEPTION);

    std::string respStr;
    respStr = _formatter->formatResponse(ex1.message(), false, *_pTrx, 0, ex1.code());
    CPPUNIT_ASSERT(respStr[35] != '0');

    respStr = _formatter->formatResponse(ex2.message(), false, *_pTrx, 0, ex2.code());
    CPPUNIT_ASSERT(respStr[35] != '0');

    respStr = _formatter->formatResponse(ex3.message(), false, *_pTrx, 0, ex3.code());
    CPPUNIT_ASSERT(respStr[35] != '0');

    respStr = _formatter->formatResponse(ex4.message(), false, *_pTrx, 0, ex4.code());
    CPPUNIT_ASSERT(respStr[35] != '0');

    //_msgErrCodeMap[ ErrorResponseException::UTAT_NET_SELLING_AMOUNTS_CONFLICT          ] = 0;
    respStr = _formatter->formatResponse(ex5.message(), false, *_pTrx, 0, ex5.code());
    CPPUNIT_ASSERT(respStr[35] == '0');

    //_msgErrCodeMap[ ErrorResponseException::UTAT_MULTIPLE_TOUR_CODES                   ] = 0;
    respStr = _formatter->formatResponse(ex6.message(), false, *_pTrx, 0, ex6.code());
    CPPUNIT_ASSERT(respStr[35] == '0');

    //_msgErrCodeMap[ ErrorResponseException::UTAT_TOUR_CODE_NOT_FOUND                   ] = 0;
    respStr = _formatter->formatResponse(ex7.message(), false, *_pTrx, 0, ex7.code());
    CPPUNIT_ASSERT(respStr[35] == '0');

    //_msgErrCodeMap[ ErrorResponseException::SAX_PARSER_FAILURE           ] = 0;
    respStr = _formatter->formatResponse(ex8.message(), false, *_pTrx, 0, ex8.code());
    CPPUNIT_ASSERT(respStr[35] == '0');

    respStr = _formatter->formatResponse(ex9.message(), false, *_pTrx, 0, ex9.code());
    CPPUNIT_ASSERT(respStr[35] != '0');

    //_msgErrCodeMap[ ErrorResponseException::UNKNOWN_EXCEPTION           ] = 0;
    respStr = _formatter->formatResponse(ex10.message(), false, *_pTrx, 0, ex10.code());
    CPPUNIT_ASSERT(respStr[35] == '0');

    //_msgErrCodeMap[ ErrorResponseException::SYSTEM_EXCEPTION            ] = 0;
    respStr = _formatter->formatResponse(ex11.message(), false, *_pTrx, 0, ex11.code());
    CPPUNIT_ASSERT(respStr[35] == '0');
  }

  class ProcessTagPermutationMock : public ProcessTagPermutation
  {
  public:
    Indicator _electronicTicketValue;
    ProcessTagPermutationMock(Indicator electronicTkt) : _electronicTicketValue(electronicTkt) {}
    const Indicator electronicTicket() const { return _electronicTicketValue; }
  };

  void setUpElectronicTicket(char indicator)
  {
    CalcTotals calcTotals;
    FarePath* farePath = _memH.insert(new FarePath);
    calcTotals.farePath = farePath;
    _construct->openElement(xml2::ReissueExchange);
    _formatter->electronicTicketInd(indicator, *_construct);
    _construct->closeElement();
  }

  void setSchemaVersion(short major, short minor, short revision)
  {
    _pTrx->getRequest()->majorSchemaVersion() = major;
    _pTrx->getRequest()->minorSchemaVersion() = minor;
    _pTrx->getRequest()->revisionSchemaVersion() = revision;
  }

  void enableBaggageCTA() { _pTrx->ticketingDate() = DateTime(2100, 1, 1); }

  void call_prepareCommissionForValidatingCarriers(
      XMLConstruct& xml,
      const FarePath& farePath,
      const CurrencyNoDec noDec) const
  {
    FuFcIdMap fuFcIdCol;
    _formatter->prepareCommissionForValidatingCarriers(*_pTrx, xml, farePath, fuFcIdCol, noDec);
  }

  void call_constructElementVCC(
      XMLConstruct& xml,
      const CarrierCode& cxrs,
      const MoneyAmount& commAmt,
      const CurrencyNoDec noDec) const
  {
    FuFcIdMap fuFcIdCol;
    FarePath fp;
    uint16_t fcId=0;
    _formatter->constructElementVCC(*_pTrx, xml, fp, cxrs, commAmt, noDec, fuFcIdCol, fcId);
  }

  void testElectronicTicketBlank()
  {
    setUpElectronicTicket(' ');
    CPPUNIT_ASSERT(_construct->getXMLData() == "<REX PXY=\"F\" PXZ=\"F\"/>");
  }

  void testElectronicTicketR()
  {
    setUpElectronicTicket('R');
    CPPUNIT_ASSERT(_construct->getXMLData() == "<REX PXY=\"T\" PXZ=\"F\"/>");
  }

  void testElectronicTicketN()
  {
    setUpElectronicTicket('N');
    CPPUNIT_ASSERT(_construct->getXMLData() == "<REX PXY=\"F\" PXZ=\"T\"/>");
  }

  bool setRedirectionCases(RexPricingTrx& rexTrx,
                           ExchangePricingTrx*& excTrx,
                           std::string& secondaryExcReqType,
                           bool isRedirectedToPortExchange,
                           bool redirectResult,
                           FareCalcCollector*& rexFareCalcCollector,
                           FareCalcCollector*& excFareCalcCollector)
  {
    rexTrx.exchangePricingTrxForRedirect() = excTrx;
    rexTrx.secondaryExcReqType() = secondaryExcReqType;
    rexTrx.redirected() = isRedirectedToPortExchange;
    rexTrx.redirectResult() = redirectResult;
    if (excFareCalcCollector)
      rexTrx.exchangePricingTrxForRedirect()->fareCalcCollector().push_back(excFareCalcCollector);

    return !rexFareCalcCollector && isRedirectedToPortExchange && rexTrx.redirectResult() &&
           !rexTrx.exchangePricingTrxForRedirect()->fareCalcCollector().empty() &&
           rexTrx.exchangePricingTrxForRedirect()->fareCalcCollector().front();
  }

  void testFormatResponseWhenCAT3XHasNoError()
  {
    bool displayOnly = false;
    ExchangePricingTrx* excTrx = 0;
    std::string secondaryExcReqType = "FE";
    bool isRedirectedToPortExchange = false, redirectResult = false;
    FareCalcCollector* excFareCalcCollector = 0;
    bool isSuccessfullyRedirected = setRedirectionCases(*_rexTrx,
                                                        excTrx,
                                                        secondaryExcReqType,
                                                        isRedirectedToPortExchange,
                                                        redirectResult,
                                                        _rexFareCalcCollector,
                                                        excFareCalcCollector);

    std::string respStr;
    respStr = _respFormatter->formatResponse(
        respStr, displayOnly, *_rexTrx, _rexFareCalcCollector, ErrorResponseException::NO_ERROR);

    std::ostringstream expectedStr;
    expectedStr << "<RexPricingResponse " << xml2::RequestType << "=\"" << _rexTrx->reqType()
                << "\"><" << xml2::TicketingAgentInfo << "/><" << xml2::BillingInformation << "/><"
                << xml2::SummaryInfo << "/><" << xml2::UnflownItinPriceInfo
                << "/></RexPricingResponse>";

    CPPUNIT_ASSERT(!isSuccessfullyRedirected);
    CPPUNIT_ASSERT_EQUAL(expectedStr.str(), respStr);
  }

  void testFormatResponseWhenCAT3XGotError()
  {
    std::string secondaryExcReqType = "FE";
    bool isRedirectedToPortExchange = true, redirectResult = true;
    ErrorResponseException rexPricingError = ErrorResponseException::UNABLE_TO_MATCH_FARE,
                           portExchangeError = ErrorResponseException::NO_ERROR;

    FareCalcCollector* rexFareCalcCollector = 0;
    bool isSuccessfullyRedirected = setRedirectionCases(*_rexTrx,
                                                        _excTrx,
                                                        secondaryExcReqType,
                                                        isRedirectedToPortExchange,
                                                        redirectResult,
                                                        rexFareCalcCollector,
                                                        _excFareCalcCollector);
    _rexTrx->redirectReasonError() = rexPricingError;

    std::string respStr;
    _respFormatter->formatResponseIfRedirected(
        *_construct, respStr, *_rexTrx, rexFareCalcCollector, portExchangeError.code());

    std::ostringstream expectedStr;
    expectedStr << "<" << xml2::TicketingAgentInfo << "/><" << xml2::BillingInformation << "/><"
                << xml2::MessageInformation << " " << xml2::MessageFailCode << "=\""
                << Message::errCode(rexPricingError.code()) << "\"/><" << xml2::SummaryInfo << "/>";

    CPPUNIT_ASSERT(isSuccessfullyRedirected);
    CPPUNIT_ASSERT_EQUAL(expectedStr.str(), _construct->getXMLData());
  }

  void testFormatResponseWhenCAT3XHasDiagnosticAndGotError()
  {
    std::string secondaryExcReqType = "FE";
    bool isRedirectedToPortExchange = true, redirectResult = true;
    ErrorResponseException rexPricingError = ErrorResponseException::UNABLE_TO_MATCH_FARE,
                           portExchangeError = ErrorResponseException::NO_ERROR;

    FareCalcCollector* rexFareCalcCollector = 0;
    bool isSuccessfullyRedirected = setRedirectionCases(*_rexTrx,
                                                        _excTrx,
                                                        secondaryExcReqType,
                                                        isRedirectedToPortExchange,
                                                        redirectResult,
                                                        rexFareCalcCollector,
                                                        _excFareCalcCollector);
    _rexTrx->redirectReasonError() = rexPricingError;
    _rexTrx->diagnostic().diagnosticType() = AllFareDiagnostic;

    std::string respStr = "DIAGNOSTIC 200 RETURNED NO DATA";
    _respFormatter->formatResponseIfRedirected(
        *_construct, respStr, *_rexTrx, rexFareCalcCollector, portExchangeError.code());

    std::ostringstream expectedStr;
    expectedStr << "<" << xml2::TicketingAgentInfo << "/><" << xml2::BillingInformation << "/><"
                << xml2::SummaryInfo << "/><" << xml2::MessageInformation << " "
                << xml2::MessageType << "=\"" << Message::TYPE_GENERAL << "\" "
                << xml2::MessageFailCode << "=\"" << 3 << "\" " << xml2::MessageText << "=\""
                << respStr << "\"/>";

    CPPUNIT_ASSERT(isSuccessfullyRedirected);
    CPPUNIT_ASSERT_EQUAL(expectedStr.str(), _construct->getXMLData());
  }

  void testFormatResponseWhenCAT3XAndPortGotError()
  {
    std::string secondaryExcReqType = "FE";
    bool isRedirectedToPortExchange = true, redirectResult = false;
    ErrorResponseException rexPricingError = ErrorResponseException::UNABLE_TO_MATCH_FARE,
                           portExchangeError = ErrorResponseException::NO_FARE_FOR_CLASS_USED;
    FareCalcCollector* rexFareCalcCollector = 0;
    FareCalcCollector* excFareCalcCollector = 0;
    bool isSuccessfullyRedirected = setRedirectionCases(*_rexTrx,
                                                        _excTrx,
                                                        secondaryExcReqType,
                                                        isRedirectedToPortExchange,
                                                        redirectResult,
                                                        rexFareCalcCollector,
                                                        excFareCalcCollector);
    _rexTrx->redirectReasonError() = rexPricingError;

    std::string respStr;
    _respFormatter->formatResponseIfRedirected(
        *_construct, respStr, *_rexTrx, rexFareCalcCollector, portExchangeError.code());

    std::ostringstream expectedStr;
    expectedStr << "<" << xml2::MessageInformation << " " << xml2::MessageFailCode << "=\""
                << Message::errCode(rexPricingError.code()) << "\"/><" << xml2::MessageInformation
                << " " << xml2::MessageFailCode << "=\""
                << Message::errCode(portExchangeError.code()) << "\"/>";

    CPPUNIT_ASSERT(!isSuccessfullyRedirected);
    CPPUNIT_ASSERT_EQUAL(expectedStr.str(), _construct->getXMLData());
  }

  void testFormatResponseWhenCAT3XAndPortGotErrorAndHasDiagnostic()
  {
    std::string secondaryExcReqType = "FE";
    bool isRedirectedToPortExchange = true, redirectResult = false;
    ErrorResponseException rexPricingError = ErrorResponseException::UNABLE_TO_MATCH_FARE,
                           portExchangeError = ErrorResponseException::NO_FARE_FOR_CLASS_USED;
    FareCalcCollector* rexFareCalcCollector = 0;
    FareCalcCollector* excFareCalcCollector = 0;
    bool isSuccessfullyRedirected = setRedirectionCases(*_rexTrx,
                                                        _excTrx,
                                                        secondaryExcReqType,
                                                        isRedirectedToPortExchange,
                                                        redirectResult,
                                                        rexFareCalcCollector,
                                                        excFareCalcCollector);
    _rexTrx->redirectReasonError() = rexPricingError;
    _rexTrx->diagnostic().diagnosticType() = AllFareDiagnostic;
    _rexTrx->diagnostic().activate();
    _rexTrx->diagnostic().insertDiagMsg("DIAGNOSTIC 200 RETURNED NO DATA");

    std::string respStr;
    _respFormatter->formatResponseIfRedirected(
        *_construct, respStr, *_rexTrx, rexFareCalcCollector, portExchangeError.code());

    std::ostringstream expectedStr;
    expectedStr << "<" << xml2::MessageInformation << " " << xml2::MessageType << "=\""
                << Message::TYPE_GENERAL << "\" " << xml2::MessageFailCode << "=\"" << 3 << "\" "
                << xml2::MessageText << "=\"DIAGNOSTIC 200 RETURNED NO DATA\"/><"
                << xml2::MessageInformation << " " << xml2::MessageFailCode << "=\""
                << Message::errCode(rexPricingError.code()) << "\"/><" << xml2::MessageInformation
                << " " << xml2::MessageFailCode << "=\""
                << Message::errCode(portExchangeError.code()) << "\"/>";

    CPPUNIT_ASSERT(!isSuccessfullyRedirected);
    CPPUNIT_ASSERT_EQUAL(expectedStr.str(), _construct->getXMLData());
  }

  void createAgent(Loc& loc, Agent& agent)
  {
    loc.loc() = "DFW";
    loc.subarea() = "1";
    loc.area() = "2";
    loc.nation() = "US";
    loc.state() = "TX";
    loc.cityInd() = true;

    agent.agentLocation() = &loc;
    agent.agentCity() = "DFW";
    agent.tvlAgencyPCC() = "HDQ";
    agent.mainTvlAgencyPCC() = "HDQ";
    agent.tvlAgencyIATA() = "XYZ";
    agent.homeAgencyIATA() = "XYZ";
    agent.agentFunctions() = "XYZ";
    agent.agentDuty() = "XYZ";
    agent.airlineDept() = "XYZ";
    agent.cxrCode() = "AA";
    agent.currencyCodeAgent() = "USD";
    agent.coHostID() = 9;
    agent.agentCommissionType() = "PERCENT";
    agent.agentCommissionAmount() = 10;
  }

  void testPrepareOBFee()
  {
    Loc loc;
    createAgent(loc, *_pAgent);
    _pRequest->ticketingAgent() = _pAgent;

    _calcTotals->equivCurrencyCode = "EUR";
    _calcTotals->convertedBaseFareCurrencyCode = "";
    _calcTotals->equivFareAmount = 1000;

    _feeInfo->feeAmount() = 1.55;
    _feeInfo->noDec() = 2;
    _feeInfo->cur() = "EUR";
    _feeInfo->serviceTypeCode() = "OB";
    _feeInfo->serviceSubTypeCode() = "FDA";
    _feeInfo->fopBinNumber() = "528159";

    _pTrx->getRequest()->collectOBFee() = 'Y';
    _pTrx->getRequest()->setCollectTTypeOBFee(true);

    _formatter->prepareOBFee(*_pTrx, *_calcTotals, *_construct, _feeInfo);

    std::string expected = "<OBF SF0=\"OBFDA\" SF1=\"1.55\" STA=\"1001.55\" "
                           "SF2=\"528159\" SF3=\"   \" SF4=\" \"";

    CPPUNIT_ASSERT(_construct->getXMLData().find(expected) != std::string::npos);
  }

  void testprepareAllOBFees()
  {
    Loc loc;
    createAgent(loc, *_pAgent);
    _pRequest->ticketingAgent() = _pAgent;

    _calcTotals->equivCurrencyCode = "USD";
    _calcTotals->convertedBaseFareCurrencyCode = "";
    _calcTotals->equivFareAmount = 1000;

    std::vector<TicketingFeesInfo*> obFeeVect;

    obFeeVect.push_back(_memH(new TicketingFeesInfo));
    obFeeVect.back()->feeAmount() = 1.00;
    obFeeVect.back()->noDec() = 2;
    obFeeVect.back()->cur() = "USD";
    obFeeVect.back()->serviceTypeCode() = "OB";
    obFeeVect.back()->serviceSubTypeCode() = "FDA";
    obFeeVect.back()->fopBinNumber() = "528159";

    obFeeVect.push_back(_memH(new TicketingFeesInfo));
    obFeeVect.back()->feeAmount() = 3.00;
    obFeeVect.back()->noDec() = 2;
    obFeeVect.back()->cur() = "USD";
    obFeeVect.back()->serviceTypeCode() = "OB";
    obFeeVect.back()->serviceSubTypeCode() = "FDA";
    obFeeVect.back()->fopBinNumber() = "524848";

    _pTrx->getRequest()->collectOBFee() = 'Y';
    _pTrx->getRequest()->setCollectTTypeOBFee(true);

    _formatter->prepareAllOBFees(*_pTrx, *_calcTotals, *_construct, 50, obFeeVect);

    std::string expected = "<OBF SF0=\"OBFDA\" SF1=\"1.00\" STA=\"1001.00\" "
                           "SF2=\"528159\" SF3=\"   \" SF4=\" \"/>"
                           "<OBF SF0=\"OBFDA\" SF1=\"3.00\" STA=\"1003.00\" "
                           "SF2=\"524848\" SF3=\"   \" SF4=\" \"/>";

    CPPUNIT_ASSERT(_construct->getXMLData().find(expected) != std::string::npos);

  }

  void testFormatResponseOBFees()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;

    CalcTotals calcTotals;
    calcTotals.equivCurrencyCode = "EUR";
    calcTotals.convertedBaseFareCurrencyCode = "";

    std::vector<TicketingFeesInfo*> info;
    FarePath* farePath = _memH.insert(new FarePath);

    TicketingFeesInfo* tInfo = TestTicketingFeesInfoFactory::create(
        "/vobs/atseintl/test/testdata/data/TicketingInfoOne.xml");
    farePath->collectedTktOBFees().push_back(tInfo);

    TicketingFeesInfo* tInfo1 = TestTicketingFeesInfoFactory::create(
        "/vobs/atseintl/test/testdata/data/TicketingInfoTwo.xml");
    farePath->collectedTktOBFees().push_back(tInfo1);

    TicketingFeesInfo* tInfo2 = TestTicketingFeesInfoFactory::create(
        "/vobs/atseintl/test/testdata/data/TicketingInfoAmountZero.xml");
    farePath->collectedTktOBFees().push_back(tInfo2);

    calcTotals.farePath = farePath;
    _formatter->prepareOBFees(*_pTrx, calcTotals, *_construct);

    std::string configValue;
    Global::config().getValue("PRICING_OPTION_MAX_LIMIT", configValue, "TICKETING_FEE_OPT_MAX");

    if (TestConfigInitializer::setValue("PRICING_OPTION_MAX_LIMIT", "2", "TICKETING_FEE_OPT_MAX", true))
    {
      XMLConstruct _construct1;
      _formatter->prepareOBFees(*_pTrx, calcTotals, _construct1);

      TicketingFeesInfo* tInfo3 = TestTicketingFeesInfoFactory::create(
          "/vobs/atseintl/test/testdata/data/TicketingInfoFOPBINblank.xml");
      farePath->collectedTktOBFees().push_back(tInfo3);

      XMLConstruct _construct2;
      _formatter->prepareOBFees(*_pTrx, calcTotals, _construct2);
    }
    if (configValue != "")
    {
      TestConfigInitializer::setValue(
          "PRICING_OPTION_MAX_LIMIT", configValue, "TICKETING_FEE_OPT_MAX", true);
    }
  }

  void testFormatResponseOBFeesRT()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;

    CalcTotals calcTotals;
    calcTotals.equivCurrencyCode = "EUR";
    calcTotals.convertedBaseFareCurrencyCode = "";

    FarePath* farePath = _memH.insert(new FarePath);

    createOBFeeVector(farePath->collectedTTypeOBFee(), 'T', 1);
    _pTrx->getRequest()->setCollectTTypeOBFee(true);

    createOBFeeVector(farePath->collectedRTypeOBFee(), 'R', 1);
    _pTrx->getRequest()->setCollectRTypeOBFee(true);

    calcTotals.farePath = farePath;
    _formatter->prepareOBFees(*_pTrx, calcTotals, *_construct);

    std::string CompareString;
    CompareString = "<OBF SF0=\"OBT01\" SF1=\"4.00\" SF3=\"   \" SF4=\" \" ";
    CompareString += "SDD=\"CARRIER TICKETING FEE01\"/>";
    CompareString += "<OBF SF0=\"OBR01\" SF1=\"4.00\" SF3=\"   \" SF4=\" \" ";
    CompareString += "SDD=\"OVERNIGHT DELIVERY CHARGE\"/>";

    CPPUNIT_ASSERT(_construct->getXMLData() == CompareString);
  }

  void testFormatResponseOBFeesMax()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;

    TestConfigInitializer::setValue("PRICING_OPTION_MAX_LIMIT", "50", "TICKETING_FEE_OPT_MAX", true);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(50), TrxUtil::getConfigOBFeeOptionMaxLimit());

    CalcTotals calcTotals;
    calcTotals.equivCurrencyCode = "EUR";
    calcTotals.convertedBaseFareCurrencyCode = "";
    calcTotals.equivFareAmount = 1000;

    FarePath* farePath = _memH.insert(new FarePath);

    createOBFeeVector(farePath->collectedTTypeOBFee(), 'T', 38);
    _pTrx->getRequest()->setCollectTTypeOBFee(true);

    createOBFeeVector(farePath->collectedRTypeOBFee(), 'R', 20);
    _pTrx->getRequest()->setCollectRTypeOBFee(true);

    calcTotals.farePath = farePath;
    _formatter->prepareOBFees(*_pTrx, calcTotals, *_construct);

    // Should cut off at 48 so, we will see R10 but not R11 (38 + 10)
    CPPUNIT_ASSERT(_construct->getXMLData().find("OBR10") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("OBR11") == std::string::npos);
  }

  void testFormatResponseOBFeesSTA()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;

    CalcTotals calcTotals;
    calcTotals.equivCurrencyCode = "EUR";
    calcTotals.convertedBaseFareCurrencyCode = "";
    calcTotals.equivFareAmount = 1000;

    FarePath* farePath = _memH.insert(new FarePath);

    farePath->collectedTktOBFees().push_back(_memH(new TicketingFeesInfo));
    farePath->collectedTktOBFees().back()->feeAmount() = 1.00;
    farePath->collectedTktOBFees().back()->cur() = "USD";
    farePath->collectedTktOBFees().back()->serviceTypeCode() = "OB";
    farePath->collectedTktOBFees().back()->serviceSubTypeCode() = "FCA";
    farePath->collectedTktOBFees().back()->fopBinNumber() = "111111";
    farePath->collectedTktOBFees().push_back(_memH(new TicketingFeesInfo));
    farePath->collectedTktOBFees().back()->feeAmount() = 2.00;
    farePath->collectedTktOBFees().back()->cur() = "USD";
    farePath->collectedTktOBFees().back()->serviceTypeCode() = "OB";
    farePath->collectedTktOBFees().back()->serviceSubTypeCode() = "FDA";
    farePath->collectedTktOBFees().back()->fopBinNumber() = "222222";
    _pTrx->getRequest()->collectOBFee() = 'Y';

    createOBFeeVector(farePath->collectedTTypeOBFee(), 'T', 40);
    _pTrx->getRequest()->setCollectTTypeOBFee(true);

    calcTotals.farePath = farePath;
    _formatter->prepareOBFees(*_pTrx, calcTotals, *_construct);

    // F Type adds the fee to the total fare for STA, but does not accumulate
    CPPUNIT_ASSERT(getTaggedSubValue(_construct->getXMLData(), "OBFCA", "STA") == "1000.80");
    CPPUNIT_ASSERT(getTaggedSubValue(_construct->getXMLData(), "OBFDA", "STA") == "1001.60");

    // T Type adds the fee to the total fare for STA, and accumulates it
    // CPPUNIT_ASSERT(getTaggedSubValue(_construct->getXMLData(), "OBT01", "STA") == "1005.00");
    // CPPUNIT_ASSERT(getTaggedSubValue(_construct->getXMLData(), "OBT02", "STA") == "1010.00");
    // CPPUNIT_ASSERT(getTaggedSubValue(_construct->getXMLData(), "OBT40", "STA") == "1200.00");
  }

  void createOBFeeVector(std::vector<TicketingFeesInfo*>& obFeeVect, char type, size_t num)
  {
    for (size_t i = 1; i <= num; ++i)
    {
      obFeeVect.push_back(_memH(new TicketingFeesInfo));
      obFeeVect.back()->feeAmount() = 5.00;
      obFeeVect.back()->cur() = "USD";
      obFeeVect.back()->serviceTypeCode() = "OB";
      std::stringstream ss;
      ss << type << std::setw(2) << std::setfill('0') << i;
      obFeeVect.back()->serviceSubTypeCode() = ss.str();
    }
  }

  std::string
  getTaggedSubValue(const std::string& inputStr, const std::string& tag, const std::string& subTag)
  {
    size_t start, end;

    start = inputStr.find(subTag, inputStr.find(tag)) + 5;
    end = inputStr.find('"', start);
    return inputStr.substr(start, end - start);
  }

  void testbuildTotalTTypeOBFeeAmount()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;

    CalcTotals calcTotals;
    calcTotals.equivCurrencyCode = "EUR";
    calcTotals.convertedBaseFareCurrencyCode = "";
    calcTotals.equivFareAmount = 1000;
    calcTotals.convertedBaseFare = 1000;
    FarePath* farePath = _memH.insert(new FarePath);

    createOBFeeVector(farePath->collectedTTypeOBFee(), 'T', 40);
    _pTrx->getRequest()->setCollectTTypeOBFee(true);

    calcTotals.farePath = farePath;
    _construct->openElement(xml2::PassengerInfo);
    _formatter->buildTotalTTypeOBFeeAmount(*_pTrx, calcTotals, *_construct);
    _construct->closeElement();

    CPPUNIT_ASSERT(_construct->getXMLData() == "<PXI C81=\"1160.00\" C82=\"160.00\"/>");
  }

  void testbuildTotalTTypeOBFeeAmountMax()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;

    CalcTotals calcTotals;
    calcTotals.equivCurrencyCode = "EUR";
    calcTotals.convertedBaseFareCurrencyCode = "";
    calcTotals.equivFareAmount = 1000;
    calcTotals.convertedBaseFare = 1000;
    FarePath* farePath = _memH.insert(new FarePath);

    createOBFeeVector(farePath->collectedTTypeOBFee(), 'T', 60);
    _pTrx->getRequest()->setCollectTTypeOBFee(true);

    calcTotals.farePath = farePath;
    _construct->openElement(xml2::PassengerInfo);
    _formatter->buildTotalTTypeOBFeeAmount(*_pTrx, calcTotals, *_construct);
    _construct->closeElement();

    // We will only display a max number of records in the response, but we sum all
    CPPUNIT_ASSERT(_construct->getXMLData() == "<PXI C81=\"1240.00\" C82=\"240.00\"/>");
  }

  void testGetConfigOBFeeOptionMaxLimit()
  {
    std::string configValue;
    std::string configValueStore;
    uint32_t max = 0;

    if (Global::config().getValue("PRICING_OPTION_MAX_LIMIT", configValue, "TICKETING_FEE_OPT_MAX"))
    {
      configValueStore = configValue;
    }

    if (TestConfigInitializer::setValue("PRICING_OPTION_MAX_LIMIT", "10", "TICKETING_FEE_OPT_MAX", true))
    {
      max = 10;
      CPPUNIT_ASSERT_EQUAL(max, TrxUtil::getConfigOBFeeOptionMaxLimit());
    }

    if (TestConfigInitializer::setValue(
            "PRICING_OPTION_MAX_LIMIT", configValueStore, "TICKETING_FEE_OPT_MAX", true))
    {
      max = atoi(configValueStore.c_str());
      CPPUNIT_ASSERT_EQUAL(max, TrxUtil::getConfigOBFeeOptionMaxLimit());
    }
  }

  void makeCalcTotals(const std::vector<MoneyAmount>& amounts,
                      const std::vector<CurrencyCode>& currencies,
                      const std::vector<FopBinNumber>& fops)
  {
    FarePath* fp = _memH(new FarePath);
    fp->processed() = true;
    std::vector<MoneyAmount>::const_iterator ai = amounts.begin();
    std::vector<MoneyAmount>::const_iterator aie = amounts.end();
    for (unsigned i = 0; ai != aie; ++ai, ++i)
    {
      fp->collectedTktOBFees().push_back(_memH(new TicketingFeesInfo));
      fp->collectedTktOBFees().back()->feeAmount() = amounts[i];
      fp->collectedTktOBFees().back()->cur() = currencies[i];
      fp->collectedTktOBFees().back()->fopBinNumber() = fops[i];
    }

    CalcTotals* ct = _memH(new CalcTotals);
    ct->equivCurrencyCode = NUC;
    ct->farePath = fp;

    _rexFareCalcCollector->passengerCalcTotals().push_back(ct);
  }

  void OBFeesForBASetUp()
  {
    std::string configValue;
    Global::config().getValue("PRICING_OPTION_MAX_LIMIT", configValue, "TICKETING_FEE_OPT_MAX");
    TestConfigInitializer::setValue("PRICING_OPTION_MAX_LIMIT", "2", "TICKETING_FEE_OPT_MAX", true);
    CPPUNIT_ASSERT_EQUAL(static_cast<uint32_t>(2), TrxUtil::getConfigOBFeeOptionMaxLimit());

    _pRequest->collectOBFee() = 'Y';
  }

  void testOBFeesForBALimitNotHit()
  {
    OBFeesForBASetUp();
    makeCalcTotals(list_of(0.0)(0.0), list_of(NUC)(NUC), list_of("")(""));
    _formatter->checkLimitOBFees(*_pTrx, *_rexFareCalcCollector);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::vector<TicketingFeesInfo*>::size_type>(2),
                         _rexFareCalcCollector->passengerCalcTotals()
                             .front()
                             ->farePath->collectedTktOBFees()
                             .size());
  }

  void testOBFeesForBAZeroNoFop()
  {
    OBFeesForBASetUp();
    makeCalcTotals(list_of(0.0)(0.0)(0.0), list_of(NUC)(NUC)(NUC), list_of("")("")(""));
    _formatter->checkLimitOBFees(*_pTrx, *_rexFareCalcCollector);
    CPPUNIT_ASSERT_EQUAL(static_cast<std::vector<TicketingFeesInfo*>::size_type>(1),
                         _rexFareCalcCollector->passengerCalcTotals()
                             .front()
                             ->farePath->collectedTktOBFees()
                             .size());
  }

  void testOBFeesForBAZeroFop()
  {
    OBFeesForBASetUp();
    makeCalcTotals(list_of(0.0)(0.0)(0.0), list_of(NUC)(NUC)(NUC), list_of("")("")("221B"));
    _formatter->checkLimitOBFees(*_pTrx, *_rexFareCalcCollector);
    CPPUNIT_ASSERT(_rexFareCalcCollector->passengerCalcTotals()
                       .front()
                       ->farePath->collectedTktOBFees()
                       .empty());
  }

  void testOBFeesForBANonZero()
  {
    OBFeesForBASetUp();
    makeCalcTotals(list_of(1.0)(2.0)(0.5), list_of(NUC)(NUC)(NUC), list_of("")("")(""));
    _formatter->checkLimitOBFees(*_pTrx, *_rexFareCalcCollector);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        2.0,
        _rexFareCalcCollector->passengerCalcTotals().front()->farePath->maximumObFee()->feeAmount(),
        EPSILON);
  }

  void testOBFeesForBAMultiPTCOneMax()
  {
    OBFeesForBASetUp();
    makeCalcTotals(list_of(0.0)(0.0)(0.0), list_of(NUC)(NUC)(NUC), list_of("")("")(""));
    makeCalcTotals(list_of(0.0)(5.0), list_of(NUC)(NUC), list_of("")("314"));
    _formatter->checkLimitOBFees(*_pTrx, *_rexFareCalcCollector);
    CPPUNIT_ASSERT(_rexFareCalcCollector->passengerCalcTotals()
                       .front()
                       ->farePath->collectedTktOBFees()
                       .empty());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        5.0,
        _rexFareCalcCollector->passengerCalcTotals().back()->farePath->maximumObFee()->feeAmount(),
        EPSILON);
  }

  void testOBFeesForBAMultiPTCOneMultiMaxSecond()
  {
    OBFeesForBASetUp();
    makeCalcTotals(list_of(0.0)(3.0)(0.0), list_of(NUC)(NUC)(NUC), list_of("")("271")(""));
    makeCalcTotals(list_of(0.0)(5.0), list_of(NUC)(NUC), list_of("")("314"));
    makeCalcTotals(list_of(3.0)(4.5), list_of(NUC)(NUC), list_of("")(""));
    _formatter->checkLimitOBFees(*_pTrx, *_rexFareCalcCollector);
    CPPUNIT_ASSERT(_rexFareCalcCollector->passengerCalcTotals()
                       .front()
                       ->farePath->collectedTktOBFees()
                       .empty());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        5.0,
        _rexFareCalcCollector->passengerCalcTotals().back()->farePath->maximumObFee()->feeAmount(),
        EPSILON);
  }

  void testOBFeesForBAMultiPTCOneMultiMaxFirst()
  {
    OBFeesForBASetUp();
    makeCalcTotals(list_of(0.0)(3.0)(8.0), list_of(NUC)(NUC)(NUC), list_of("")("271")(""));
    makeCalcTotals(list_of(0.0)(5.0), list_of(NUC)(NUC), list_of("")("314"));
    _formatter->checkLimitOBFees(*_pTrx, *_rexFareCalcCollector);
    CPPUNIT_ASSERT(_rexFareCalcCollector->passengerCalcTotals()
                       .front()
                       ->farePath->collectedTktOBFees()
                       .empty());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        8.0,
        _rexFareCalcCollector->passengerCalcTotals().back()->farePath->maximumObFee()->feeAmount(),
        EPSILON);
  }

  void testOBFeesForBANoChargeY()
  {
    OBFeesForBASetUp();
    makeCalcTotals(list_of(1.0)(0.0)(0.5), list_of(NUC)(NUC)(NUC), list_of("")("")(""));
    _rexFareCalcCollector->passengerCalcTotals().front()->farePath->collectedTktOBFees()
        [1]->noCharge() = 'Y';
    _formatter->checkLimitOBFees(*_pTrx, *_rexFareCalcCollector);
    // Two ASSERTs below commented out due to code change in the PricingResponseFormatter.cpp 0n
    // Apr-28-2011
    //        CPPUNIT_ASSERT_EQUAL('Y',
    // _rexFareCalcCollector->passengerCalcTotals().front()->farePath->collectedTktOBFees().front()->noCharge());
    //        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0,
    // _rexFareCalcCollector->passengerCalcTotals().front()->farePath->collectedTktOBFees().front()->feeAmount(),
    // EPSILON);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(
        1.0,
        _rexFareCalcCollector->passengerCalcTotals().front()->farePath->maximumObFee()->feeAmount(),
        EPSILON);
    CPPUNIT_ASSERT(_rexFareCalcCollector->passengerCalcTotals()
                       .front()
                       ->farePath->collectedTktOBFees()
                       .empty());
  }

  void testOBFeesForBAClearAndGetLast()
  {
    std::vector<CalcTotals*> ct;

    for (int i = 0; i < 4; ++i)
    {
      ct.push_back(_memH(new CalcTotals));
      FarePath* fp = _memH(new FarePath);
      fp->processed() = true;
      fp->collectedTktOBFees().push_back(_memH(new TicketingFeesInfo));
      ct.back()->farePath = fp;
    }

    CPPUNIT_ASSERT_EQUAL(ct.back()->farePath, _formatter->clearAllFeesAndGetLastPTC(ct));
    for (int i = 0; i < 4; ++i)
      CPPUNIT_ASSERT(ct[i]->farePath->collectedTktOBFees().empty());
  }

  void testSetFopBinNumber()
  {
    const FopBinNumber formOfPayment = "123456";
    const std::string attributeName = xml2::FopBINNumberApplied;

    _construct->openElement("XXX");
    _respFormatter->setFopBinNumber(formOfPayment, *_construct, attributeName);
    _construct->closeElement();

    CPPUNIT_ASSERT(_construct->getXMLData().find(attributeName + "=\"" + formOfPayment + "\"") !=
                   std::string::npos);
  }

  void testCalculateObFeeAmountFromAmountMaxWhenSameCur()
  {
    MoneyAmount feeAmt = 10.0;
    CalcTotals calcTotals;
    calcTotals.equivCurrencyCode = "NUC";
    _feeInfo->feePercent() = 5;
    _feeInfo->feeAmount() = feeAmt;
    _feeInfo->cur() = "NUC";

    MoneyAmount amt =
        _respFormatter->calculateObFeeAmountFromAmountMax(*_pTrx, calcTotals, _feeInfo);
    CPPUNIT_ASSERT_EQUAL(feeAmt, amt);
  }

  void testComputeMaximumOBFeesPercentWhenNoOB()
  {
    CalcTotals calcTotals;
    FarePath fp;
    calcTotals.farePath = &fp;

    std::pair<const TicketingFeesInfo*, MoneyAmount> pair =
        _formatter->computeMaximumOBFeesPercent(*_pTrx, calcTotals);
    CPPUNIT_ASSERT(!pair.first);
  }

  void testComputeMaximumOBFeesPercentWhenMax()
  {
    OBFeesForBASetUp();
    MoneyAmount amtMax = 5.0;
    makeCalcTotals(list_of(amtMax)(3.0), list_of(NUC)(NUC), list_of("123")("271"));
    CalcTotals& calcTotals = *(_rexFareCalcCollector->passengerCalcTotals().front());

    std::pair<const TicketingFeesInfo*, MoneyAmount> pair =
        _formatter->computeMaximumOBFeesPercent(*_pTrx, calcTotals);
    CPPUNIT_ASSERT(pair.first);
    CPPUNIT_ASSERT_EQUAL(pair.second, amtMax);
  }

  void testGetFeeRoundingCallControllingNationCodeReturnTrue()
  {
    _pTrx->ticketingDate() = DateTime::localTime();
    CurrencyCode curr = "ZWR";
    RoundingFactor rF = 0;
    CurrencyNoDec rND = 0;
    RoundingRule rR = NONE;
    CPPUNIT_ASSERT(_formatter->getFeeRounding(*_pTrx, curr, rF, rND, rR));
  }

  void testGetFeeRoundingDoNotCallControllingNationCodeReturnTrue()
  {
    _pTrx->ticketingDate() = DateTime::localTime();
    CurrencyCode curr = "JPY";
    RoundingFactor rF = 0;
    CurrencyNoDec rND = 0;
    RoundingRule rR = NONE;
    CPPUNIT_ASSERT(_formatter->getFeeRounding(*_pTrx, curr, rF, rND, rR));
  }

  void testCabinCharReturnSame()
  {
    Indicator cabin = 'P';
    CPPUNIT_ASSERT_EQUAL('P', _formatter->cabinChar(cabin));
  }

  void testCabinCharReturnP()
  {
    Indicator cabin = 'R';
    CPPUNIT_ASSERT_EQUAL('P', _formatter->cabinChar(cabin));
  }

  void testCabinCharReturnS()
  {
    Indicator cabin = 'W';
    CPPUNIT_ASSERT_EQUAL('S', _formatter->cabinChar(cabin));
  }

  void testPrepareDifferentialwithNOdata()
  {
    _pTrx->ticketingDate() = DateTime::localTime();
    FareUsage fu1;
    uint16_t noDecCalc = 2;

    _formatter->prepareDifferential(*_pTrx, fu1, noDecCalc, *_construct);
    CPPUNIT_ASSERT(_construct->getXMLData().empty());
  }

  void testPrepareDifferentialwithDiffDataFail()
  {
    _pTrx->ticketingDate() = DateTime::localTime();
    FareUsage fu1;
    uint16_t noDecCalc = 2;
    fu1.differentialAmt() = 200;
    DifferentialData dd;
    fu1.differentialPlusUp().push_back(&dd);

    dd.status() = DifferentialData::SC_FAILED;

    _formatter->prepareDifferential(*_pTrx, fu1, noDecCalc, *_construct);
    CPPUNIT_ASSERT(_construct->getXMLData().empty());
  }

  void testPrepareDifferentialwithDiffDataPASSnoAmount()
  {
    uint16_t noDecCalc = 2;
    FareUsage fu1;
    AirSeg ts;
    FareMarket mk;
    PaxTypeFareTest* pf = getPTFareForDifferential();
    DifferentialData dd;

    prepareDifferentialData(fu1, ts, mk, *pf, dd);

    dd.amount() = 0;

    std::string expected =
        "<ZDF C50=\"0.00\" A13=\"DFW\" A14=\"LON\" Q4S=\"00\" Q4T=\"00\" B02=\"AA\"/>";
    _formatter->prepareDifferential(*_pTrx, fu1, noDecCalc, *_construct);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testPrepareDifferentialwithDiffDataPASSwithAmount()
  {
    uint16_t noDecCalc = 2;
    FareUsage fu1;
    AirSeg ts;
    FareMarket mk;
    PaxTypeFareTest* pf = getPTFareForDifferential();
    DifferentialData dd;

    prepareDifferentialData(fu1, ts, mk, *pf, dd);

    dd.fareClassHigh() = "R";

    std::string expected = "<HIP C50=\"200.00\" A13=\"DFW\" A14=\"LON\" BJ0=\"R\" N04=\"R\" "
                           "Q4S=\"00\" Q4T=\"00\" B02=\"AA\"/>";
    _formatter->prepareDifferential(*_pTrx, fu1, noDecCalc, *_construct);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testPrepareDifferentialwithDiffDataPASSwithHIPAmount()
  {
    uint16_t noDecCalc = 2;
    FareUsage fu1;
    AirSeg ts;
    FareMarket mk;
    PaxTypeFareTest* pf = getPTFareForDifferential();
    DifferentialData dd;

    prepareDifferentialData(fu1, ts, mk, *pf, dd);

    fu1.inbound() = true;
    dd.hipAmount() = 300;
    dd.fareClassHigh() = "R";

    std::string expected = "<HIP C50=\"300.00\" A13=\"LON\" A14=\"DFW\" BJ0=\"R\" N04=\"R\" "
                           "Q4S=\"00\" Q4T=\"00\" B02=\"AA\"/>";
    _formatter->prepareDifferential(*_pTrx, fu1, noDecCalc, *_construct);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testPrepareDifferentialwithDiffDataPASSwithHIPhigherOrigin()
  {
    uint16_t noDecCalc = 2;
    FareUsage fu1;
    AirSeg ts;
    FareMarket mk;
    PaxTypeFareTest* pf = getPTFareForDifferential();
    DifferentialData dd;

    prepareDifferentialData(fu1, ts, mk, *pf, dd);

    fu1.inbound() = true;
    dd.hipAmount() = 300;
    dd.fareClassHigh() = "R";
    dd.fareClassLow() = "F0";
    dd.hipHighOrigin() = "CHI";
    dd.hipCabinHigh() = DifferentialData::PREMIUM_FIRST_HIP;
    pf->mileageSurchargePctg() = 10;

    std::string expected = "<HIP C50=\"300.00\" A13=\"LON\" A14=\"DFW\" B30=\"F0\" BJ0=\"R\" "
                           "Q48=\"10\" A03=\"CHI\" N04=\"P\" Q4S=\"00\" Q4T=\"00\" B02=\"AA\"/>";
    _formatter->prepareDifferential(*_pTrx, fu1, noDecCalc, *_construct);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void testPrepareDifferentialwithDiffDataPASSwithHIPlowOrigin()
  {
    uint16_t noDecCalc = 2;
    FareUsage fu1;
    AirSeg ts;
    FareMarket mk;
    PaxTypeFareTest* pf = getPTFareForDifferential();
    DifferentialData dd;

    prepareDifferentialData(fu1, ts, mk, *pf, dd);

    fu1.inbound() = true;
    dd.hipAmount() = 300;
    dd.fareClassHigh() = "R9";
    dd.fareClassLow() = "F0";
    dd.hipLowOrigin() = "CHI";
    dd.hipCabinHigh() = DifferentialData::PREMIUM_FIRST_HIP;
    dd.hipCabinLow() = DifferentialData::PREMIUM_BUSINESS_HIP;
    pf->mileageSurchargePctg() = 10;

    std::string expected = "<HIP C50=\"300.00\" A13=\"LON\" A14=\"DFW\" B30=\"F0\" BJ0=\"R9\" "
                           "Q48=\"10\" A01=\"CHI\" N00=\"J\" N04=\"R\" Q4S=\"00\" Q4T=\"00\" "
                           "B02=\"AA\"/>";
    _formatter->prepareDifferential(*_pTrx, fu1, noDecCalc, *_construct);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void prepareDifferentialData(
      FareUsage& fu1, AirSeg& ts, FareMarket& mk, PaxTypeFare& pf, DifferentialData& dd)
  {
    _pTrx->ticketingDate() = DateTime::localTime();

    ts.origin() = TestLocFactory::create(LOC_DFW);
    ts.destination() = TestLocFactory::create(LOC_LON);
    ts.origAirport() = "DFW";
    ts.destAirport() = "LON";
    ts.pnrSegment() = 0;

    mk.boardMultiCity() = "DFW";
    mk.offMultiCity() = "LON";
    mk.travelSeg().push_back(&ts);

    fu1.differentialAmt() = 200;
    fu1.differentialPlusUp().push_back(&dd);

    dd.travelSeg().push_back(&ts);
    dd.fareMarket().push_back(&mk);
    dd.status() = DifferentialData::SC_PASSED;
    dd.amount() = 200;
    dd.fareHigh() = &pf;
  }

  PaxTypeFareTest* getPTFareForDifferential()
  {
    PaxTypeFareTest* ptf = _memH.create<PaxTypeFareTest>();
    ptf->setCarrier("AA");
    return ptf;
  }

  void testPreparePassengerInfoRetailerCode()
  {
    XMLConstruct xml;

    FareUsage fu1;
    PaxTypeFareTest* pf = getPTFareForDifferential();

    NegPaxTypeFareRuleData negRuleData;
    negRuleData.sourcePseudoCity() = "DFW";
    negRuleData.fareRetailerCode() = "FARES";
    pf->setRuleData(NEGOTIATED_RULE, _pTrx->dataHandle(), &negRuleData, true);
    fu1.paxTypeFare() = pf;

    FarePath* farePath = FarePathBuilder(&_memH).withSegmentNumber(1, true)
                                                .withFareUsageOnPricingUnit(0, &fu1)
                                                .build();
    FareCalcConfig* fcConfig = FareCalcConfigBuilder(&_memH).build();
    CalcTotals* calcTotals = CalcTotalsBuilder(&_memH).withFarePath(farePath).build();
    uint16_t paxNumber = 0;

    pf->fareDisplayCat35Type() = 'a';
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;

    setSchemaVersion(1, 1, 0);

    PaxType* paxType = _memH.create<PaxType>();
    paxType->paxType() = "ABC";
    _pTrx->paxType().push_back(paxType);

    _respFormatter->preparePassengerInfo(*_pTrx, *fcConfig, *calcTotals, paxNumber, *_construct);
    CPPUNIT_ASSERT(_construct->getXMLData().find("RC1=\"FARES\"") !=
                   std::string::npos);
  }

  void testPrepareHPUForNet()
  {
    FareUsage fu1;
    PaxTypeFareTest* pf = getPTFareForDifferential();

    NegPaxTypeFareRuleData negRuleData;
    NegFareRest negFareRest;
    prepareHPUForNetData(fu1, *pf, negRuleData, negFareRest);

    std::string expected = "<HPU APP=\"NT\" C51=\"100\" C52=\"100\" C53=\"USD\" A52=\"FRA\" "
                           "R52=\"1\"/>";
    _formatter->prepareHPUForNet(*_pTrx, fu1, *_construct);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void prepareHPUForNetData(FareUsage& fu1, PaxTypeFare& pf, NegPaxTypeFareRuleData &negRuleData, NegFareRest &negFareRest)
  {
    FareInfo* fareInfo = _memH.create<FareInfo>();
    CurrencyCode curr = "USD";
    fareInfo->currency() = curr;
    fareInfo->fareAmount() = 200;
    pf.fare()->setFareInfo(fareInfo);

    negRuleData.netAmount() = 100;
    negRuleData.sourcePseudoCity() = "FRA";
    negRuleData.fareRetailerRuleId() = 1;
    pf.setRuleData(NEGOTIATED_RULE, _pTrx->dataHandle(), &negRuleData, true);

    pf.paxTypeFareRuleData(35)->ruleItemInfo() = &negFareRest;

    pf.fareDisplayCat35Type() = 'a';
    FareClassAppInfo *fareClassAppInfo = _memH.create<FareClassAppInfo>();
    FareClassAppInfo::dummyData(*fareClassAppInfo);
    pf.fareClassAppInfo() = fareClassAppInfo;
    fu1.paxTypeFare() = &pf;
  }

  void testPrepareHPUForAdj()
  {
    FareUsage fu1;
    PaxTypeFareTest* pf = getPTFareForDifferential();

    AdjustedSellingCalcData adjSellingCalcData;
    prepareHPUForAdjData(fu1, *pf, adjSellingCalcData);

    std::string expected = "<HPU APP=\"AJ\" C51=\"100\" C52=\"50\" C53=\"USD\" A52=\"DFW\" "
                           "R52=\"1\"/>";
    _formatter->prepareHPUForAdjusted(*_pTrx, fu1, *_construct);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void prepareHPUForAdjData(FareUsage& fu1, PaxTypeFare& pf, AdjustedSellingCalcData &adjSellingCalcData)
  {
    FareInfo* fareInfo = _memH.create<FareInfo>();
    CurrencyCode curr = "USD";
    fareInfo->currency() = curr;
    fareInfo->fareAmount() = 50;
    pf.fare()->setFareInfo(fareInfo);

    adjSellingCalcData.setCalculatedAmt(100);
    adjSellingCalcData.setSourcePcc("DFW");
    adjSellingCalcData.setFareRetailerRuleId(1);

    pf.setAdjustedSellingCalcData(&adjSellingCalcData);
    fu1.paxTypeFare() = &pf;
  }

  void testPrepareCat27TourCodeWhenCat35EmptyTourCode()
  {
    CollectedNegFareData negFareData;
    negFareData.indicatorCat35() = true;
    FarePath farePath;
    farePath.cat27TourCode() = "TOURCODE";
    farePath.collectedNegFareData() = &negFareData;

    _construct->openElement("T");
    CPPUNIT_ASSERT(_formatter->prepareCat27TourCode(farePath, 0, *_construct));
    _construct->closeElement();

    const std::string XML = "<T S02=\"TOURCODE\"/>";
    CPPUNIT_ASSERT_EQUAL(XML, _construct->getXMLData());
  }

  void testPrepareCat27TourCodeWhenCat35TourCode()
  {
    CollectedNegFareData negFareData;
    negFareData.indicatorCat35() = true;
    negFareData.tourCode() = "CAT35TOURCODE";
    FarePath farePath;
    farePath.collectedNegFareData() = &negFareData;

    CPPUNIT_ASSERT(!_formatter->prepareCat27TourCode(farePath, 0, *_construct));

    CPPUNIT_ASSERT(_construct->getXMLData().empty());
  }

  void testPrepareCat27TourCodeWhenEmptyTourCode()
  {
    FarePath farePath;
    farePath.cat27TourCode() = "";

    CPPUNIT_ASSERT(!_formatter->prepareCat27TourCode(farePath, 0, *_construct));

    CPPUNIT_ASSERT(_construct->getXMLData().empty());
  }

  void testPrepareCat27TourCodeWhenTourCode()
  {
    const std::string TOURCODE = "TOURCODE";

    FarePath farePath;
    farePath.cat27TourCode() = TOURCODE;

    _construct->openElement("T");
    CPPUNIT_ASSERT(_formatter->prepareCat27TourCode(farePath, 0, *_construct));
    _construct->closeElement();

    const std::string XML = "<T S02=\"TOURCODE\" Q0W=\"0\"/>";
    CPPUNIT_ASSERT_EQUAL(XML, _construct->getXMLData());
  }

  void testPrepareCat27PaxFareNumber()
  {
    const std::string TOURCODE = "TOURCODE";

    FarePath farePath;
    farePath.cat27TourCode() = TOURCODE;

    _construct->openElement("T");
    CPPUNIT_ASSERT(_formatter->prepareCat27TourCode(farePath, 1, *_construct));
    _construct->closeElement();

    const std::string XML = "<T S02=\"TOURCODE\" Q0W=\"1\"/>";
    CPPUNIT_ASSERT_EQUAL(XML, _construct->getXMLData());
  }

  void testChangeFeesForRefund_minimum()
  {
    PricingTrx* trx = TransactionBuilder<RexPricingTrx>(&_memH).dummyTransaction().build();
    RefundPermutation perm;
    perm.minimumPenalty() = Money(10.0, NUC);

    _formatter->addChangeFeesForRefund(*trx, *_construct, perm);
    const std::string XML = "<CHG C77=\"10.00\" C78=\"NUC\" PXJ=\"T\" NRI=\"F\"/>";
    CPPUNIT_ASSERT_EQUAL(XML, _construct->getXMLData());
  }

  void testChangeFeesForRefund_zero()
  {
    PricingTrx* trx = TransactionBuilder<RexPricingTrx>(&_memH).dummyTransaction().build();
    RefundPermutation perm;

    _formatter->addChangeFeesForRefund(*trx, *_construct, perm);
    const std::string XML = "<CHG C77=\"0.00\" PXL=\"T\" PXK=\"F\" PXJ=\"T\" NRI=\"F\"/>";
    CPPUNIT_ASSERT_EQUAL(XML, _construct->getXMLData());
  }

  RefundPermutation createPenalty(const std::vector<RefundPenalty::Fee>& fees, int puCount = 1)
  {
    RefundPermutation perm;

    RefundPenalty* rp = _memH.create<RefundPenalty>();
    rp->fee() = fees;

    for (int i = 0; i < puCount; i++)
    {
      PricingUnit* pu = (PricingUnit*)(int64_t)i;
      perm.penaltyFees()[pu] = rp;
    }

    MoneyAmount totalAmt = 0.0;
    for (const RefundPenalty::Fee& fee : fees)
      totalAmt += fee.amount().value();

    perm.totalPenalty().value() = totalAmt;

    return perm;
  }

  void testChangeFeesForRefund_nonRef()
  {
    PricingTrx* trx = TransactionBuilder<RexPricingTrx>(&_memH).dummyTransaction().build();

    std::vector<RefundPenalty::Fee> fees;
    fees.push_back(RefundPenalty::Fee(Money(20.0, NUC)));
    fees.push_back(RefundPenalty::Fee(Money(10.0, NUC), false, true));

    const RefundPermutation& perm = createPenalty(fees);

    _formatter->addChangeFeesForRefund(*trx, *_construct, perm);
    const std::string XML("<CHG C77=\"20.00\" C78=\"NUC\" PXJ=\"F\" NRI=\"F\"/>"
                          "<CHG C77=\"10.00\" C78=\"NUC\" PXJ=\"F\" NRI=\"T\"/>");
    CPPUNIT_ASSERT_EQUAL(XML, _construct->getXMLData());
  }

  void testChangeFeesForRefund_multiple()
  {
    PricingTrx* trx = TransactionBuilder<RexPricingTrx>(&_memH).dummyTransaction().build();

    std::vector<RefundPenalty::Fee> fees;
    fees.push_back(RefundPenalty::Fee());
    fees.push_back(RefundPenalty::Fee(Money(10.0, NUC)));

    const RefundPermutation& perm = createPenalty(fees, 2);

    _formatter->addChangeFeesForRefund(*trx, *_construct, perm);
    const std::string XML("<CHG C77=\"10.00\" C78=\"NUC\" PXJ=\"F\" NRI=\"F\"/>"
                          "<CHG C77=\"10.00\" C78=\"NUC\" PXJ=\"F\" NRI=\"F\"/>");
    CPPUNIT_ASSERT_EQUAL(XML, _construct->getXMLData());
  }

  FarePath* createFarePath()
  {
    FarePath* farePath = _memH.insert(new FarePath);
    PaxType* paxType = _memH.insert(new PaxType);
    paxType->paxType() = "ADT";
    farePath->paxType() = paxType;
    return farePath;
  }

  void fillOCFeesUsage(OCFees* ocFees, const std::string padisTranslation)
  {
    OCFeesUsage* ocfUsage = _memH.create<OCFeesUsage>();
    ocfUsage->oCFees() = ocFees;
    ocfUsage->setSegIndex(0);
    ocfUsage->upgradeT198Sequence() = _memH.create<SvcFeesResBkgDesigInfo>();
    ocfUsage->upgradeT198CommercialName() = padisTranslation;
    ocFees->ocfeeUsage().push_back(ocfUsage);
  }

  OCFees* createOCFees(bool displayOnly = false,
                       ServiceGroup serviceGroup = "BG",
                       std::string padisTranslation = EMPTY_STRING())
  {
    OCFees* ocFees = _memH.insert(new OCFees);
    ocFees->setDisplayOnly(displayOnly);
    ocFees->carrierCode() = "AA";
    ocFees->feeNoDec() = 0;

    SubCodeInfo* subCodeInfo = _memH.insert(new SubCodeInfo);
    subCodeInfo->commercialName() = "BAGGAGE";
    subCodeInfo->serviceGroup() = serviceGroup;
    ocFees->subCodeInfo() = subCodeInfo;

    OptionalServicesInfo* optServicesInfo = _memH.insert(new OptionalServicesInfo);
    ocFees->optFee() = optServicesInfo;

    Loc* originLoc = _memH.insert(new Loc);
    originLoc->loc() = "KRK";
    AirSeg* travelStartSeg = _memH.insert(new AirSeg);
    travelStartSeg->origin() = originLoc;
    ocFees->travelStart() = travelStartSeg;

    Loc* destLoc = _memH.insert(new Loc);
    destLoc->loc() = "MUC";
    AirSeg* travelEndSeg = _memH.insert(new AirSeg);
    travelEndSeg->destination() = destLoc;
    ocFees->travelEnd() = travelEndSeg;

    fillOCFeesUsage(ocFees, padisTranslation);

    return ocFees;
  }

  PaxOCFeesUsages* createPaxOCFeesUsages(bool displayOnly = false, ServiceGroup serviceGroup = "BG")
  {
    OCFees* ocFees = createOCFees(displayOnly, serviceGroup);
    OCFeesUsage* ocFeesUsage = _memH.create<OCFeesUsage>();
    ocFeesUsage->oCFees() = ocFees;
    ocFees->ocfeeUsage().push_back(ocFeesUsage);
    FPOCFeesUsages* fpOcFeesUsages =
        _memH.insert(new FPOCFeesUsages(createFarePath(), ocFeesUsage));
    PaxOCFeesUsages* paxOcFeesUsages = _memH.insert(new PaxOCFeesUsages(*fpOcFeesUsages));
    return paxOcFeesUsages;
  }

  OCFees* createOCFeesWithCurrecnyAndAmount(bool displayOnly = false,
                                            ServiceGroup serviceGroup = "BG",
                                            std::string padisTranslation = EMPTY_STRING())
  {
    OCFees* ocFees = _memH.insert(new OCFees);
    ocFees->setDisplayOnly(displayOnly);
    ocFees->feeAmount() = 15;
    ocFees->feeCurrency() = "EUR";
    ocFees->carrierCode() = "AA";
    ocFees->feeNoDec() = 2;
    //    ocFees->purchaseByDate() = DateTime(2011, 3, 17);
    ocFees->purchaseByDate() = DateTime::localTime();

    SubCodeInfo* subCodeInfo = _memH.insert(new SubCodeInfo);
    subCodeInfo->emdType() = 'E';
    subCodeInfo->commercialName() = "BAGGAGE";
    subCodeInfo->serviceGroup() = serviceGroup;

    ocFees->subCodeInfo() = subCodeInfo;

    OptionalServicesInfo* optServicesInfo = _memH.insert(new OptionalServicesInfo);
    ocFees->optFee() = optServicesInfo;

    Loc* originLoc = _memH.insert(new Loc);
    originLoc->loc() = "KRK";
    AirSeg* travelStartSeg = _memH.insert(new AirSeg);
    travelStartSeg->origin() = originLoc;
    ocFees->travelStart() = travelStartSeg;

    Loc* destLoc = _memH.insert(new Loc);
    destLoc->loc() = "MUC";
    AirSeg* travelEndSeg = _memH.insert(new AirSeg);
    travelEndSeg->destination() = destLoc;
    ocFees->travelEnd() = travelEndSeg;

    fillOCFeesUsage(ocFees, padisTranslation);

    return ocFees;
  }

  PaxOCFeesUsages* createPaxOCFeesUsagesWithCurrencyAndAmount(bool displayOnly = false,
                                                              ServiceGroup serviceGroup = "BG")
  {
    OCFees* ocFees = createOCFeesWithCurrecnyAndAmount(displayOnly, serviceGroup);
    OCFeesUsage* ocFeesUsage = _memH.create<OCFeesUsage>();
    ocFeesUsage->oCFees() = ocFees;
    ocFees->ocfeeUsage().push_back(ocFeesUsage);
    FPOCFeesUsages* fpOcFeesUsages =
        _memH.insert(new FPOCFeesUsages(createFarePath(), ocFeesUsage));
    PaxOCFeesUsages* paxOcFeesUsages = _memH.insert(new PaxOCFeesUsages(*fpOcFeesUsages));
    return paxOcFeesUsages;
  }

  ServiceFeesGroup* createServiceFeesGroup(uint16_t numOfFees = 0,
                                           bool displayOnly = false,
                                           ServiceGroup serviceGroup = "BG")
  {
    ServiceFeesGroup* srvFeesGroup = _memH.insert(new ServiceFeesGroup);
    srvFeesGroup->state() = ServiceFeesGroup::VALID;
    srvFeesGroup->groupCode() = "ML";
    _itin->ocFeesGroup().push_back(srvFeesGroup);

    std::vector<OCFees*> ocFees;

    for (uint16_t i = 0; i < numOfFees; ++i)
      ocFees.push_back(createOCFees(displayOnly, serviceGroup));

    FarePath* farePath = createFarePath();
    farePath->itin() = _itin;

    srvFeesGroup->ocFeesMap().insert(std::make_pair(farePath, ocFees));

    return srvFeesGroup;
  }

  void testFormatOCFeesResponse_ValidGroup()
  {
    ServiceFeesGroup* srvFeesGroup = createServiceFeesGroup();
    srvFeesGroup->groupCode() = "XX";
    _construct->openElement("PricingResponse");

    _respFormatter->formatOCFeesResponse(*_construct, *_pTrx);
    _construct->closeElement();
    const std::string expectedXML = "<PricingResponse><OCM><OCH><MSG S18=\"AIR "
                                    "EXTRAS\"/></OCH><OCG SF0=\"XX\"/></OCM></PricingResponse>";

    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testFormatOCFeesResponse_EmptyGroup()
  {
    ServiceFeesGroup* srvFeesGroup = createServiceFeesGroup();
    srvFeesGroup->state() = ServiceFeesGroup::EMPTY;
    srvFeesGroup->groupCode() = "XX";
    _construct->openElement("PricingResponse");

    _respFormatter->formatOCFeesResponse(*_construct, *_pTrx);
    _construct->closeElement();
    const std::string expectedXML =
        "<PricingResponse><OCM><OCG SF0=\"XX\"/></OCM></PricingResponse>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testFormatOCFeesResponse_InvalidGroup()
  {
    ServiceFeesGroup* srvFeesGroup = createServiceFeesGroup();
    srvFeesGroup->state() = ServiceFeesGroup::INVALID;
    srvFeesGroup->groupCode() = "XX";
    _construct->openElement("PricingResponse");

    _respFormatter->formatOCFeesResponse(*_construct, *_pTrx);
    _construct->closeElement();
    const std::string expectedXML =
        "<PricingResponse><OCM><OCG SF0=\"XX\"/></OCM></PricingResponse>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testFormatOCFeesResponse_NAGroup()
  {
    ServiceFeesGroup* srvFeesGroup = createServiceFeesGroup();
    srvFeesGroup->state() = ServiceFeesGroup::NOT_AVAILABLE;
    srvFeesGroup->groupCode() = "XX";

    _construct->openElement("PricingResponse");

    _respFormatter->formatOCFeesResponse(*_construct, *_pTrx);

    _construct->closeElement();

    const std::string expectedXML =
        "<PricingResponse><OCM><OCG SF0=\"XX\"/></OCM></PricingResponse>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testFormatOCFees()
  {
    ServiceFeesGroup srvFeesGroup;
    _itin->ocFeesGroup().push_back(&srvFeesGroup);

    _formatter->formatOCFees(*_pTrx, *_construct);
    const std::string expectedXML = "<OCG/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testFormatOCFees_UN()
  {
    ServiceFeesGroup* srvFeesGroup = createServiceFeesGroup();
    srvFeesGroup->groupCode() = "UN";
    srvFeesGroup->groupDescription() = "UNACCOMPANIED TRAVEL (ESCORT)";

    _formatter->formatOCFees(*_pTrx, *_construct);
    const std::string expectedXML = "<OCG SF0=\"UN\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testFormatOCFees_SAGroup()
  {
    ServiceFeesGroup* srvFeesGroup = createServiceFeesGroup(1, false, "SA");
    srvFeesGroup->groupCode() = "SA";

    _formatter->formatOCFees(*_pTrx, *_construct);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"SA\" ST1=\"@\">") !=
                   std::string::npos);
  }

  void testFormatOCFees_XXGroup()
  {
    //    _pTrx->ticketingDate() = DateTime::localTime();
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;
    //    _pRequest->ticketingDT() = DateTime::localTime();

    ServiceFeesGroup* srvFeesGroup = createServiceFeesGroup(1);
    srvFeesGroup->groupCode() = "XX";

    _respFormatter->formatOCFees(*_pTrx, *_construct);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"XX\" ST1=\"*\">") ==
                   std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"XX\" ST1=\"@\">") ==
                   std::string::npos);
  }

  void testFormatOCFees_Historical()
  {
   // TestConfigInitializer::setValue("MAX_NUMBER_OF_FEES", 3, "SERVICE_FEES_SVC", true);
    _pTrx->getOptions()->isOCHistorical() = true;

    createServiceFeesGroup(1);

    _formatter->formatOCFees(*_pTrx, *_construct);
    CPPUNIT_ASSERT(_construct->getXMLData().find("ST0=\"AL\"") != std::string::npos);
  }

  void testFormatOCFees_Historical_EmptyGroup()
  {
    _pTrx->getOptions()->isOCHistorical() = true;

    ServiceFeesGroup* srvFeesGroup = createServiceFeesGroup();
    srvFeesGroup->state() = ServiceFeesGroup::EMPTY;

    _formatter->formatOCFees(*_pTrx, *_construct);
    CPPUNIT_ASSERT(_construct->getXMLData().find("FEES ARE FOR INFORMATION / DISPLAY ONLY") ==
                   std::string::npos);
  }

  void testFormatOCFees_Historical_SAGroup()
  {
    _pTrx->getOptions()->isOCHistorical() = true;

    ServiceFeesGroup* srvFeesGroup = createServiceFeesGroup(1);
    srvFeesGroup->groupCode() = "SA";

    _formatter->formatOCFees(*_pTrx, *_construct);
    CPPUNIT_ASSERT(_construct->getXMLData().find("ST0=\"AL\"") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"SA\">") != std::string::npos);
  }

  void testFormatOCFees_DispOnly()
  {
    //    _pTrx->ticketingDate() = DateTime::localTime();
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;
    //    _pRequest->ticketingDT() = DateTime::localTime();

    createServiceFeesGroup(1, true);
    createServiceFeesGroup(1, false);

    _respFormatter->formatOCFees(*_pTrx, *_construct);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"ML\" ST1=\"*\">") !=
                   std::string::npos);
  }

  void testFormatOCFees_DispOnly_SAGroup()
  {
    //    _pTrx->ticketingDate() = DateTime::localTime();
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;
    //    _pRequest->ticketingDT() = DateTime::localTime();

    ServiceFeesGroup* srvFeesGroup = createServiceFeesGroup(1, true);

    srvFeesGroup->groupCode() = "SA";
    createServiceFeesGroup(1, false);

    _respFormatter->formatOCFees(*_pTrx, *_construct);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"SA\" ST1=\"*\">") !=
                   std::string::npos);
  }

  void testFormatOCFeesGroups()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;
    std::vector<PaxOCFeesUsages> paxOcFeesUsages;
    ServiceFeesGroup srvFeesGroup;
    std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages> > > groupFeesVector;

    paxOcFeesUsages.push_back(*(createPaxOCFeesUsages()));
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFeesUsages));

    _respFormatter->formatOCFeesGroups(*_pTrx, *_construct, groupFeesVector, false, true);

    CPPUNIT_ASSERT(_construct->getXMLData().find("1  ADT-BAGGAGE") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("AA -1-KRKMUC") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("*") == std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("@") == std::string::npos);
  }

  void testFormatOCFees_NumberOfFeesBelowMax()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;
    createServiceFeesGroup(2);

    // TestConfigInitializer::setValue("MAX_NUMBER_OF_FEES", 3, "SERVICE_FEES_SVC", true);

    _respFormatter->formatOCFees(*_pTrx, *_construct);

    CPPUNIT_ASSERT(
        _construct->getXMLData().find(
            "<OCF><MSG S18=\"MORE AIR EXTRAS AVAILABLE - USE WPAE WITH QUALIFIERS\"/></OCF>") ==
        std::string::npos);
  }

  void testFormatOCFees_NumberOfFeesEqualToMax()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;
    createServiceFeesGroup(3);

    //TestConfigInitializer::setValue("MAX_NUMBER_OF_FEES", 3, "SERVICE_FEES_SVC", true);

    _respFormatter->formatOCFees(*_pTrx, *_construct);

    CPPUNIT_ASSERT(
        _construct->getXMLData().find(
            "<OCF><MSG S18=\"MORE AIR EXTRAS AVAILABLE - USE WPAE WITH QUALIFIERS\"/></OCF>") ==
        std::string::npos);
  }

  void testFormatOCFees_NumberOfFeesAboveMax()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;
    createServiceFeesGroup(5);

    //TestConfigInitializer::setValue("MAX_NUMBER_OF_FEES", 3, "SERVICE_FEES_SVC", true);

    _respFormatter->formatOCFees(*_pTrx, *_construct);

    CPPUNIT_ASSERT(
        _construct->getXMLData().find(
            "<OCF><MSG S18=\"MORE AIR EXTRAS AVAILABLE - USE WPAE WITH QUALIFIERS\"/></OCF>") !=
        std::string::npos);
  }

  void testFormatOCFees_AllSegsConfirmed()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;
    _itin->allSegsConfirmed() = true;
    _itin->allSegsUnconfirmed() = false;

    createServiceFeesGroup(2);

    _respFormatter->formatOCFees(*_pTrx, *_construct);

    CPPUNIT_ASSERT(_construct->getXMLData().find(
                       "AIR EXTRAS APPLICABLE TO CONFIRMED SEGMENTS ONLY") == std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"ML\">") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCT ST0=\"UF\"/>") == std::string::npos);
  }

  void testFormatOCFees_AllSegsUnconfirmed()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;
    _itin->allSegsUnconfirmed() = true;
    _itin->allSegsConfirmed() = false;

    createServiceFeesGroup(2);

    _respFormatter->formatOCFees(*_pTrx, *_construct);

    // CPPUNIT_ASSERT(_construct->getXMLData().find("AIR EXTRAS APPLICABLE TO CONFIRMED SEGMENTS
    // ONLY") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"ML\">") != std::string::npos);
  }

  void testFormatOCFees_NotAllSegsConfirmed()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;
    _itin->allSegsUnconfirmed() = false;
    _itin->allSegsConfirmed() = false;

    createServiceFeesGroup(2);

    _respFormatter->formatOCFees(*_pTrx, *_construct);

    CPPUNIT_ASSERT(_construct->getXMLData().find(
                       "AIR EXTRAS APPLICABLE TO CONFIRMED SEGMENTS ONLY") == std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"ML\">") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCT ST0=\"UF\"/>") != std::string::npos);
  }

  void testFormatOCFeesGroups_SAGroup()
  {
    std::vector<PaxOCFeesUsages> paxOcFeesUsages;
    ServiceFeesGroup srvFeesGroup;
    std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages> > > groupFeesVector;

    srvFeesGroup.groupCode() = "SA";
    paxOcFeesUsages.push_back(*(createPaxOCFeesUsages(false, srvFeesGroup.groupCode())));
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFeesUsages));

    _formatter->formatOCFeesGroups(*_pTrx, *_construct, groupFeesVector, false, true);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"SA\" ST1=\"@\">") !=
                   std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("-- ADT-BAGGAGE") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("AA -1-KRKMUC") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("@") != std::string::npos);
  }

  void testFormatOCFeesGroups_AllDispOnly()
  {
    std::vector<PaxOCFeesUsages> paxOcFeesUsages;
    ServiceFeesGroup srvFeesGroup;
    std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages> > > groupFeesVector;

    paxOcFeesUsages.push_back(*(createPaxOCFeesUsages()));
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFeesUsages));

    _formatter->formatOCFeesGroups(*_pTrx, *_construct, groupFeesVector, true, true);
    CPPUNIT_ASSERT(_construct->getXMLData().find("-- ADT-BAGGAGE") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("AA -1-KRKMUC") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("@") == std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("*") == std::string::npos);
  }

  void testFormatOCFeesGroups_AllDispOnly_SAGroup()
  {
    std::vector<PaxOCFeesUsages> paxOcFeesUsages;
    ServiceFeesGroup srvFeesGroup;
    std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages> > > groupFeesVector;

    srvFeesGroup.groupCode() = "SA";
    paxOcFeesUsages.push_back(*(createPaxOCFeesUsages()));
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFeesUsages));

    _formatter->formatOCFeesGroups(*_pTrx, *_construct, groupFeesVector, true, true);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"SA\" ST1=\"@\">") ==
                   std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("-- ADT-BAGGAGE") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("AA -1-KRKMUC") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("@") == std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("*") == std::string::npos);
  }

  void testFormatOCFeesGroups_DispOnly()
  {
    std::vector<PaxOCFeesUsages> paxOcFeesUsages;
    ServiceFeesGroup srvFeesGroup;
    std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages> > > groupFeesVector;

    paxOcFeesUsages.push_back(*(createPaxOCFeesUsages(true)));
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFeesUsages));

    _formatter->formatOCFeesGroups(*_pTrx, *_construct, groupFeesVector, false, true);
    CPPUNIT_ASSERT(_construct->getXMLData().find("-- ADT-BAGGAGE") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("AA -1-KRKMUC") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("*") != std::string::npos);
  }

  void testFormatOCFeesGroups_DispOnly_SAGroup_2Fees()
  {
    std::vector<PaxOCFeesUsages> paxOcFeesUsages;
    ServiceFeesGroup srvFeesGroup;
    std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages> > > groupFeesVector;

    srvFeesGroup.groupCode() = "SA";
    paxOcFeesUsages.push_back(*(createPaxOCFeesUsages(true)));
    paxOcFeesUsages.push_back(*(createPaxOCFeesUsages(false, srvFeesGroup.groupCode())));
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFeesUsages));

    _formatter->formatOCFeesGroups(*_pTrx, *_construct, groupFeesVector, false, true);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"SA\" ST1=\"*@\">") !=
                   std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("-- ADT-BAGGAGE") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("AA -1-KRKMUC") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("@") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("*") != std::string::npos);
  }

  void testFormatOCFeesGroups_DispOnly_SAGroup()
  {
    std::vector<PaxOCFeesUsages> paxOcFeesUsages;
    ServiceFeesGroup srvFeesGroup;
    std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages> > > groupFeesVector;

    srvFeesGroup.groupCode() = "SA";
    paxOcFeesUsages.push_back(*(createPaxOCFeesUsages(true)));
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFeesUsages));

    _formatter->formatOCFeesGroups(*_pTrx, *_construct, groupFeesVector, false, true);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"SA\" ST1=\"@\">") ==
                   std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("-- ADT-BAGGAGE") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("AA -1-KRKMUC") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("*") != std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("@") == std::string::npos);
  }

  void testFormatOCFeesGroups_AdvPurchase()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;
    std::vector<PaxOCFeesUsages> paxOcFeesUsages;
    ServiceFeesGroup srvFeesGroup;
    std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages> > > groupFeesVector;

    srvFeesGroup.groupCode() = "ML";
    PaxOCFeesUsages* paxFeesUsages = createPaxOCFeesUsages();
    (const_cast<OptionalServicesInfo*>(paxFeesUsages->fees()->optFee()))->advPurchTktIssue() = 'X';
    paxOcFeesUsages.push_back(*paxFeesUsages);
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFeesUsages));

    _respFormatter->formatOCFeesGroups(*_pTrx, *_construct, groupFeesVector, false, true);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"ML\" ST1=\"P\">") !=
                   std::string::npos);
  }

  void testFormatOCFeesGroups_NotAdvPurchase()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;
    std::vector<PaxOCFeesUsages> paxOcFeesUsages;
    ServiceFeesGroup srvFeesGroup;
    std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages> > > groupFeesVector;

    srvFeesGroup.groupCode() = "ML";
    paxOcFeesUsages.push_back(*(createPaxOCFeesUsages()));
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFeesUsages));

    _respFormatter->formatOCFeesGroups(*_pTrx, *_construct, groupFeesVector, false, true);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"ML\" ST1=\"P\">") ==
                   std::string::npos);
  }

  void testFormatOCFeesGroups_AdvPurchase_SA()
  {
    std::vector<PaxOCFeesUsages> paxOcFeesUsages;
    ServiceFeesGroup srvFeesGroup;
    std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages> > > groupFeesVector;

    srvFeesGroup.groupCode() = "SA";
    PaxOCFeesUsages* paxFeesUsages = createPaxOCFeesUsages(false, srvFeesGroup.groupCode());
    (const_cast<OptionalServicesInfo*>(paxFeesUsages->fees()->optFee()))->advPurchTktIssue() = 'X';
    paxOcFeesUsages.push_back(*paxFeesUsages);
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFeesUsages));

    _formatter->formatOCFeesGroups(*_pTrx, *_construct, groupFeesVector, false, true);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"SA\" ST1=\"P\">") ==
                   std::string::npos);
  }

  void testFormatOCFeesGroups_NonRefundable_N()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;
    std::vector<PaxOCFeesUsages> paxOcFeesUsages;
    ServiceFeesGroup srvFeesGroup;
    std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages> > > groupFeesVector;

    srvFeesGroup.groupCode() = "ML";
    PaxOCFeesUsages* paxFeesUsages = createPaxOCFeesUsages();
    (const_cast<OptionalServicesInfo*>(paxFeesUsages->fees()->optFee()))->refundReissueInd() = 'N';
    paxOcFeesUsages.push_back(*paxFeesUsages);
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFeesUsages));

    _respFormatter->formatOCFeesGroups(*_pTrx, *_construct, groupFeesVector, false, true);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"ML\" ST1=\"N\">") !=
                   std::string::npos);
  }

  void testFormatOCFeesGroups_NonRefundable_R()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;
    std::vector<PaxOCFeesUsages> paxOcFeesUsages;
    ServiceFeesGroup srvFeesGroup;
    std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages> > > groupFeesVector;

    srvFeesGroup.groupCode() = "ML";
    PaxOCFeesUsages* paxFeesUsages = createPaxOCFeesUsages();
    (const_cast<OptionalServicesInfo*>(paxFeesUsages->fees()->optFee()))->refundReissueInd() = 'R';
    paxOcFeesUsages.push_back(*paxFeesUsages);
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFeesUsages));

    _respFormatter->formatOCFeesGroups(*_pTrx, *_construct, groupFeesVector, false, true);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"ML\" ST1=\"N\">") !=
                   std::string::npos);
  }

  void testFormatOCFeesGroups_Refundable()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;
    std::vector<PaxOCFeesUsages> paxOcFeesUsages;
    ServiceFeesGroup srvFeesGroup;
    std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages> > > groupFeesVector;

    srvFeesGroup.groupCode() = "ML";
    PaxOCFeesUsages* paxFeesUsages = createPaxOCFeesUsages();
    (const_cast<OptionalServicesInfo*>(paxFeesUsages->fees()->optFee()))->refundReissueInd() = 'Y';
    paxOcFeesUsages.push_back(*paxFeesUsages);
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFeesUsages));

    _respFormatter->formatOCFeesGroups(*_pTrx, *_construct, groupFeesVector, false, true);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"ML\" ST1=\"N\">") ==
                   std::string::npos);
  }

  void testFormatOCFeesGroups_NonRefundable_SA()
  {
    std::vector<PaxOCFeesUsages> paxOcFeesUsages;
    ServiceFeesGroup srvFeesGroup;
    std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages> > > groupFeesVector;

    srvFeesGroup.groupCode() = "SA";
    PaxOCFeesUsages* paxFeesUsages = createPaxOCFeesUsages(false, srvFeesGroup.groupCode());
    (const_cast<OptionalServicesInfo*>(paxFeesUsages->fees()->optFee()))->refundReissueInd() = 'N';
    paxOcFeesUsages.push_back(*paxFeesUsages);
    groupFeesVector.push_back(std::make_pair(&srvFeesGroup, paxOcFeesUsages));

    _formatter->formatOCFeesGroups(*_pTrx, *_construct, groupFeesVector, false, true);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<OCG SF0=\"SA\" ST1=\"N\">") ==
                   std::string::npos);
  }

  void test_addPrefixWarningForOCTrailer_False()
  {
    bool ret = _formatter->addPrefixWarningForOCTrailer(*_pTrx, false);
    CPPUNIT_ASSERT_EQUAL(false, ret);
  }

  void testIsOcFeesTrxDisplayOnly()
  {
    _pOptions->isTicketingDateOverrideEntry() = false;
    _pRequest->lowFareNoRebook() = 'N';
    CPPUNIT_ASSERT(!_formatter->isOcFeesTrxDisplayOnly(*_pTrx));
  }

  void testIsOcFeesTrxDisplayOnly_DateOverride()
  {
    _pOptions->isOCHistorical() = true;
    CPPUNIT_ASSERT(_formatter->isOcFeesTrxDisplayOnly(*_pTrx));
  }

  void testIsOcFeesTrxDisplayOnly_LowFare()
  {
    _pRequest->lowFareNoRebook() = 'Y';
    CPPUNIT_ASSERT(_formatter->isOcFeesTrxDisplayOnly(*_pTrx));
  }

  FareUsage* setupPrepareTfdpsc(Indicator fareBasisAmtInd, bool uniqueFareBasis = false)
  {
    TestConfigInitializer::setValue("ACTIVATE_OPTIMUS_NET_REMIT", "Y", "PRICING_SVC", true);
    TrxUtil::enableAbacus();

    Agent* agent = _memH.create<Agent>();
    _pRequest->ticketingAgent() = agent;

    Customer* customer = _memH.create<Customer>();
    customer->crsCarrier() = "1B";
    customer->hostName() = "ABAC";
    agent->agentTJR() = customer;

    FareUsage* fu = _memH.create<FareUsage>();
    AirSeg* first = _memH.create<AirSeg>();
    AirSeg* second = _memH.create<AirSeg>();
    AirSeg* last = _memH.create<AirSeg>();

    fu->travelSeg().push_back(first);
    fu->travelSeg().push_back(second);
    fu->travelSeg().push_back(last);

    // create cat 35 fare
    PaxTypeFare* ptf = _memH.create<PaxTypeFare>();
    fu->paxTypeFare() = ptf;

    Fare* fare = _memH.create<Fare>();
    ptf->setFare(fare);

    FareInfo* fareInfo = _memH.create<FareInfo>();
    fareInfo->vendor() = "ATP";
    fare->setFareInfo(fareInfo);

    ptf->status().set(PaxTypeFare::PTF_Negotiated);
    NegPaxTypeFareRuleData* ruleData = _memH.create<NegPaxTypeFareRuleData>();
    PaxTypeFare::PaxTypeFareAllRuleData* allRules =
        _memH.create<PaxTypeFare::PaxTypeFareAllRuleData>();
    (*ptf->paxTypeFareRuleDataMap())[RuleConst::NEGOTIATED_RULE] = allRules;

    allRules->chkedRuleData = true;
    allRules->chkedGfrData = false;
    allRules->fareRuleData = ruleData;
    allRules->gfrRuleData = 0;
    PaxTypeFare* baseFare = _memH.create<PaxTypeFare>();
    NegFareRestExt* nfrExt = _memH.create<NegFareRestExt>();
    ruleData->ruleItemInfo() = _memH.create<NegFareRest>();
    ruleData->negFareRestExt() = nfrExt;
    ruleData->baseFare() = baseFare;
    fareInfo->fareClass() = "ABC";
    baseFare->fare()->setFareInfo(fareInfo);
    nfrExt->fareBasisAmtInd() = fareBasisAmtInd;

    NegFareRestExtSeq* restExtSeq = _memH.create<NegFareRestExtSeq>();
    nfrExt->tktFareDataSegExistInd() = 'Y';
    if (uniqueFareBasis)
      restExtSeq->uniqueFareBasis() = "U2345678";

    fu->netRemitPscResults().push_back(
        FareUsage::TktNetRemitPscResult(first, last, restExtSeq, ptf));

    return fu;
  }

  void testDisplayTfdPsc_Pass_NegFareWithNegFareRestExtSeq()
  {
    FareUsage* fu = setupPrepareTfdpsc(RuleConst::NR_VALUE_N);
    CPPUNIT_ASSERT(_formatter->displayTfdPsc(*_pTrx, *fu));
  }

  void testDisplayTfdPsc_Fail_NegFareWithNoNegFareRestExtSeq()
  {
    FareUsage* fu = setupPrepareTfdpsc(RuleConst::NR_VALUE_N);
    fu->netRemitPscResults().clear();
    CPPUNIT_ASSERT(!_formatter->displayTfdPsc(*_pTrx, *fu));
  }

  void testPrepareTfdpsc_FareBasisIndA()
  {
    FareUsage* fu = setupPrepareTfdpsc('A');

    _construct->openElement("A");
    _formatter->prepareTfdpsc(*_pTrx, fu->travelSeg().front(), *fu, *_construct);
    _construct->closeElement();

    const std::string expectedXML = "<A B51=\"ABC\" Q10=\"3\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testPrepareTfdpsc_FareBasisIndF()
  {
    FareUsage* fu = setupPrepareTfdpsc('F');

    _construct->openElement("F");
    _formatter->prepareTfdpsc(*_pTrx, fu->travelSeg().front(), *fu, *_construct);
    _construct->closeElement();

    const std::string expectedXML = "<F B51=\"ABC\" Q10=\"3\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testPrepareTfdpsc_FareBasisIndA_MatchOnLast()
  {
    FareUsage* fu = setupPrepareTfdpsc('A');

    _construct->openElement("A");
    _formatter->prepareTfdpsc(*_pTrx, fu->travelSeg().back(), *fu, *_construct);
    _construct->closeElement();

    const std::string expectedXML = "<A B51=\"ABC\" Q10=\"3\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testPrepareTfdpsc_FareBasisIndA_MatchOnMiddle()
  {
    FareUsage* fu = setupPrepareTfdpsc('A');

    _construct->openElement("A");
    _formatter->prepareTfdpsc(*_pTrx, *(fu->travelSeg().begin() + 1), *fu, *_construct);
    _construct->closeElement();

    const std::string expectedXML = "<A B51=\"ABC\" Q10=\"3\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testPrepareTfdpsc_FareBasisIndA_DontMatch()
  {
    FareUsage* fu = setupPrepareTfdpsc('A');
    fu->netRemitPscResults().clear();
    AirSeg seg;

    _construct->openElement("A");
    _formatter->prepareTfdpsc(*_pTrx, &seg, *fu, *_construct);
    _construct->closeElement();

    const std::string expectedXML = "<A/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testPrepareTfdpsc_FareBasisIndBWithUnfbc()
  {
    FareUsage* fu = setupPrepareTfdpsc('B', true);

    _construct->openElement("B");
    _formatter->prepareTfdpsc(*_pTrx, fu->travelSeg().front(), *fu, *_construct);
    _construct->closeElement();

    const std::string expectedXML = "<B B51=\"U2345678\" Q10=\"8\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testPrepareTfdpsc_FareBasisIndBWithNoUnfbc()
  {
    FareUsage* fu = setupPrepareTfdpsc('B');

    _construct->openElement("B");
    _formatter->prepareTfdpsc(*_pTrx, fu->travelSeg().front(), *fu, *_construct);
    _construct->closeElement();

    const std::string expectedXML = "<B/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testPrepareFareVendorSource_NonCat35Fare()
  {
    FarePath farePath;
    FareUsage* fu = setupPrepareTfdpsc(RuleConst::NR_VALUE_N);

    _construct->openElement("A");
    _formatter->prepareNetRemitTrailerMsg(*_pTrx, farePath, *fu, *_construct, *_calcTotals);
    _construct->closeElement();

    const std::string XML = "<A/>";
    CPPUNIT_ASSERT_EQUAL(XML, _construct->getXMLData());
  }

  void testPrepareFareVendorSource_Cat35WithNRTicketIndFalse()
  {
    CollectedNegFareData negFareData;
    FarePath farePath;
    farePath.collectedNegFareData() = &negFareData;
    FareUsage* fu = setupPrepareTfdpsc(RuleConst::NR_VALUE_N);

    _construct->openElement("A");
    _formatter->prepareNetRemitTrailerMsg(*_pTrx, farePath, *fu, *_construct, *_calcTotals);
    _construct->closeElement();

    const std::string XML = "<A/>";
    CPPUNIT_ASSERT_EQUAL(XML, _construct->getXMLData());
  }

  void testPrepareFareVendorSource_FareBasisIndNWithNoExtSeq()
  {
    CollectedNegFareData negFareData;
    negFareData.netRemitTicketInd() = true;
    FarePath farePath;
    farePath.collectedNegFareData() = &negFareData;
    FareUsage* fu = setupPrepareTfdpsc(RuleConst::NR_VALUE_N);
    fu->netRemitPscResults().clear();

    _construct->openElement("A");
    _formatter->prepareNetRemitTrailerMsg(*_pTrx, farePath, *fu, *_construct, *_calcTotals);
    _construct->closeElement();

    const std::string XML = "<A/>";
    CPPUNIT_ASSERT_EQUAL(XML, _construct->getXMLData());
  }

  void testPrepareFareVendorSource_FareBasisIndBWithUnfbc()
  {
    CollectedNegFareData negFareData;
    negFareData.netRemitTicketInd() = true;
    FarePath farePath;
    farePath.collectedNegFareData() = &negFareData;
    FareUsage* fu = setupPrepareTfdpsc('B', true);

    _construct->openElement("A");
    _formatter->prepareNetRemitTrailerMsg(*_pTrx, farePath, *fu, *_construct, *_calcTotals);
    _construct->closeElement();

    const std::string XML = "<A S83=\"ATPC\"/>";
    CPPUNIT_ASSERT_EQUAL(XML, _construct->getXMLData());
  }

  void testPrepareFareVendorSource_FareBasisIndBWithNoUnfbc()
  {
    CollectedNegFareData negFareData;
    negFareData.netRemitTicketInd() = true;
    FarePath farePath;
    farePath.collectedNegFareData() = &negFareData;
    FareUsage* fu = setupPrepareTfdpsc('B');

    _construct->openElement("A");
    _formatter->prepareNetRemitTrailerMsg(*_pTrx, farePath, *fu, *_construct, *_calcTotals);
    _construct->closeElement();

    const std::string XML = "<A/>";
    CPPUNIT_ASSERT_EQUAL(XML, _construct->getXMLData());
  }

  FareUsage* setupFareUsage(VendorCode vendor, bool needFareProperties)
  {
    FareUsage* fu = _memH.create<FareUsage>();
    PaxTypeFare* ptf = _memH.create<PaxTypeFare>();
    Fare* fare = _memH.create<Fare>();
    ptf->setFare(fare);
    FareInfo* fareInfo = _memH.create<FareInfo>();
    fare->setFareInfo(fareInfo);
    fareInfo->vendor() = vendor;
    fu->paxTypeFare() = ptf;
    if (needFareProperties)
    {
      ptf->status().set(PaxTypeFare::PTF_Negotiated);
      NegPaxTypeFareRuleData* ruleData = _memH.create<NegPaxTypeFareRuleData>();
      PaxTypeFare::PaxTypeFareAllRuleData* allRules =
          _memH.create<PaxTypeFare::PaxTypeFareAllRuleData>();
      (*ptf->paxTypeFareRuleDataMap())[RuleConst::NEGOTIATED_RULE] = allRules;

      allRules->chkedRuleData = true;
      allRules->chkedGfrData = false;
      allRules->fareRuleData = ruleData;
      allRules->gfrRuleData = 0;

      FareProperties* fareProperties = _memH.create<FareProperties>();
      ruleData->fareProperties() = fareProperties;

      fareProperties->fareSource() = "SQSIN";
    }
    AirSeg* first = _memH.create<AirSeg>();
    fu->travelSeg().push_back(first);
    return fu;
  }

  void testPrepareFareVendorSource_ATP()
  {
    CollectedNegFareData negFareData;
    negFareData.netRemitTicketInd() = true;
    FarePath farePath;
    farePath.collectedNegFareData() = &negFareData;
    FareUsage* fu = setupFareUsage("ATP", false);

    _construct->openElement("A");
    _formatter->prepareNetRemitTrailerMsg(*_pTrx, farePath, *fu, *_construct, *_calcTotals);
    _construct->closeElement();

    const std::string XML = "<A S83=\"ATPC\"/>";
    CPPUNIT_ASSERT_EQUAL(XML, _construct->getXMLData());
  }

  void testPrepareFareVendorSource_5KAD()
  {
    CollectedNegFareData negFareData;
    negFareData.netRemitTicketInd() = true;
    FarePath farePath;
    farePath.collectedNegFareData() = &negFareData;
    FareUsage* fu = setupFareUsage("5KAD", false);

    _construct->openElement("A");
    _formatter->prepareNetRemitTrailerMsg(*_pTrx, farePath, *fu, *_construct, *_calcTotals);
    _construct->closeElement();

    const std::string XML = "<A S83=\"5KAD\"/>";
    CPPUNIT_ASSERT_EQUAL(XML, _construct->getXMLData());
  }

  void testPrepareFareVendorSource_SMFA()
  {
    CollectedNegFareData negFareData;
    negFareData.netRemitTicketInd() = true;
    FarePath farePath;
    farePath.collectedNegFareData() = &negFareData;
    FareUsage* fu = setupFareUsage("SMFA", true);

    _construct->openElement("A");
    _formatter->prepareNetRemitTrailerMsg(*_pTrx, farePath, *fu, *_construct, *_calcTotals);
    _construct->closeElement();

    const std::string XML = "<A S83=\"SQSIN\"/>";
    CPPUNIT_ASSERT_EQUAL(XML, _construct->getXMLData());
  }

  void testPrepareFareVendorSource_SMFC()
  {
    CollectedNegFareData negFareData;
    negFareData.netRemitTicketInd() = true;
    FarePath farePath;
    farePath.collectedNegFareData() = &negFareData;
    FareUsage* fu = setupFareUsage("SMFC", true);

    _construct->openElement("A");
    _formatter->prepareNetRemitTrailerMsg(*_pTrx, farePath, *fu, *_construct, *_calcTotals);
    _construct->closeElement();

    const std::string XML = "<A S83=\"SQSIN\"/>";
    CPPUNIT_ASSERT_EQUAL(XML, _construct->getXMLData());
  }

  void testIsTimeOutBeforeStartOCFees_True()
  {
    _itin->timeOutForExceeded() = true;
    _respFormatter->isTimeOutBeforeStartOCFees(*_pTrx, *_construct, false);

    const std::string expectedXML = "<OCM><OCF><MSG S18=\"MAX AIR EXTRAS EXCEEDED/USE AE SVC "
                                    "QUALIFIER OR SEG SELECT\"/></OCF></OCM>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testIsTimeOutBeforeStartOCFees_True_ABACUS()
  {
    _itin->timeOutForExceeded() = true;
    _respFormatter->isTimeOutBeforeStartOCFees(*_pTrx, *_construct, true);
    const std::string expectedXML = "<OCM SF8=\"T\"><OCF><MSG S18=\"MAX AIR EXTRAS EXCEEDED/USE AE "
                                    "SVC QUALIFIER OR SEG SELECT\"/></OCF></OCM>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testTimeOutMaxCharCountOCFeesStatusALL_WP_True()
  {
    _itin->timeOutOCFForWP() = true;
    createServiceFeesGroup();
    _respFormatter->timeOutMaxCharCountNoOCFeesReturned(*_pTrx, *_construct, false);
    const std::string expectedXML = "<OCM><OCF><MSG S18=\"AIR EXTRAS MAY APPLY - USE WPAE WITH "
                                    "SERVICE QUALIFIER\"/></OCF><OCG SF0=\"ML\"/></OCM>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testTimeOutMaxCharCountOCFeesStatusALL_WPAE_True()
  {
    _pTrx->getOptions()->isProcessAllGroups() = true;
    createServiceFeesGroup();
    _respFormatter->timeOutMaxCharCountNoOCFeesReturned(*_pTrx, *_construct, false);
    const std::string expectedXML = "<OCM><OCF><MSG S18=\"MAX AIR EXTRAS EXCEEDED/USE AE SVC "
                                    "QUALIFIER OR SEG SELECT\"/></OCF><OCG SF0=\"ML\"/></OCM>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testTimeOutMaxCharCountOCFeesStatusUQ_True()
  {
    _pTrx->getOptions()->isProcessAllGroups() = true;
    createServiceFeesGroup();
    _construct->openElement("PricingResponse");
    _respFormatter->timeOutMaxCharCountRequestedOCFeesReturned(*_pTrx, *_construct, false);
    _construct->closeElement();
    const std::string expectedXML =
        "<PricingResponse><OCM><OCH><MSG S18=\"AIR EXTRAS\"/></OCH><OCF><MSG S18=\"MORE AIR EXTRAS "
        "AVAILABLE - USE WPAE WITH QUALIFIERS\"/></OCF><OCG SF0=\"ML\"/></OCM></PricingResponse>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testAnyTimeOutMaxCharCountIssue_OCFee_returned()
  {
    _itin->timeOutForExceededSFGpresent() = true;
    createServiceFeesGroup();
    _construct->openElement("PricingResponse");
    _respFormatter->anyTimeOutMaxCharCountIssue(*_pTrx, *_construct, false);
    _construct->closeElement();
    const std::string expectedXML =
        "<PricingResponse><OCM><OCH><MSG S18=\"AIR EXTRAS\"/></OCH><OCF><MSG S18=\"MORE AIR EXTRAS "
        "AVAILABLE - USE WPAE WITH QUALIFIERS\"/></OCF><OCG SF0=\"ML\"/></OCM></PricingResponse>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void testAnyTimeOutMaxCharCountIssue_NO_OCFee_returned()
  {
    _itin->timeOutForExceeded() = true;
    _pTrx->getOptions()->isProcessAllGroups() = true;
    createServiceFeesGroup();
    _respFormatter->anyTimeOutMaxCharCountIssue(*_pTrx, *_construct, false);
    const std::string expectedXML = "<OCM><OCF><MSG S18=\"MAX AIR EXTRAS EXCEEDED/USE AE SVC "
                                    "QUALIFIER OR SEG SELECT\"/></OCF><OCG SF0=\"ML\"/></OCM>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_isR7TuningAndWP_flase()
  {
    CPPUNIT_ASSERT(!_respFormatter->isR7TuningAndWP(*_pTrx, *_construct, false));
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_isR7TuningAndWP_true()
  {
    _pTrx->getOptions()->isWPwithOutAE() = true;
    CPPUNIT_ASSERT(!_respFormatter->isR7TuningAndWP(*_pTrx, *_construct, false));
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_isR7TuningAndWP_true_withItinTrue()
  {
    _itin->moreFeesAvailable() = true;
    _pTrx->getOptions()->isWPwithOutAE() = true;
    CPPUNIT_ASSERT(_respFormatter->isR7TuningAndWP(*_pTrx, *_construct, false));
    const std::string expectedXML = "<OCM ST0=\"AE\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_isR7TuningAndWP_true_prefixTrue()
  {
    _pTrx->getOptions()->isWPwithOutAE() = true;
    CPPUNIT_ASSERT(!_respFormatter->isR7TuningAndWP(*_pTrx, *_construct, true));
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_isR7TuningAndWP_true_withItinTrue_prefixTrue()
  {
    _itin->moreFeesAvailable() = true;
    _pTrx->getOptions()->isWPwithOutAE() = true;
    CPPUNIT_ASSERT(_respFormatter->isR7TuningAndWP(*_pTrx, *_construct, true));
    const std::string expectedXML = "<OCM SF8=\"T\" ST0=\"AE\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatOCHeaderMsg()
  {
    _respFormatter->formatOCHeaderMsg(*_construct);
    const std::string expectedXML = "<OCH><MSG S18=\"AIR EXTRAS\"/></OCH>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_isGenericTrailer_Empty()
  {
    createServiceFeesGroup();
    _pTrx->getOptions()->isProcessAllGroups() = true;
    _respFormatter->isGenericTrailer(*_pTrx, *_construct, false);
    const std::string expectedXML = "";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_isGenericTrailer_True_No_Valid_Group()
  {
    _pTrx->getOptions()->isProcessAllGroups() = true;
    _respFormatter->isGenericTrailer(*_pTrx, *_construct, false);
    const std::string expectedXML = "<OCM><OCF><MSG S18=\"AIR EXTRAS NOT FOUND\"/></OCF></OCM>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_isGenericTrailer_true_Unconfirm()
  {
    _itin->allSegsUnconfirmed() = true;
    _pTrx->getOptions()->isProcessAllGroups() = true;
    _respFormatter->isGenericTrailer(*_pTrx, *_construct, false);
    const std::string expectedXML =
        "<OCM><OCF><MSG S18=\"AIR EXTRAS APPLICABLE TO CONFIRMED SEGMENTS ONLY\"/></OCF></OCM>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_builTrailerOCF()
  {
    _respFormatter->builTrailerOCF(*_pTrx, *_construct, "AIR EXTRAS NOT FOUND", true);
    const std::string expectedXML =
        "<OCM SF8=\"T\"><OCF><MSG S18=\"AIR EXTRAS NOT FOUND\"/></OCF></OCM>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_createOCGSection()
  {
    ServiceFeesGroup* srvFeesGroup = _memH.insert(new ServiceFeesGroup);
    srvFeesGroup->groupCode() = "BG";
    _itin->ocFeesGroup().push_back(srvFeesGroup);
    ServiceFeesGroup* srvFeesGroup1 = _memH.insert(new ServiceFeesGroup);
    srvFeesGroup1->groupCode() = "ML";
    _itin->ocFeesGroup().push_back(srvFeesGroup1);
    ServiceFeesGroup* srvFeesGroup2 = _memH.insert(new ServiceFeesGroup);
    srvFeesGroup2->groupCode() = "SA";
    _itin->ocFeesGroup().push_back(srvFeesGroup2);
    _respFormatter->createOCGSection(*_pTrx, *_construct);
    const std::string expectedXML = "<OCG SF0=\"BG\"/><OCG SF0=\"ML\"/><OCG SF0=\"SA\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_checkIfAnyGroupValid_false()
  {
    CPPUNIT_ASSERT_EQUAL(false, _respFormatter->checkIfAnyGroupValid(*_pTrx));
  }

  void test_checkIfAnyGroupValid_true()
  {
    createServiceFeesGroup();
    CPPUNIT_ASSERT_EQUAL(true, _respFormatter->checkIfAnyGroupValid(*_pTrx));
  }

  void test_replaceSpecialCharInDisplay()
  {
    ServiceFeesGroup* srvFeesGroup = _memH.insert(new ServiceFeesGroup);
    srvFeesGroup->groupCode() = "UN";
    srvFeesGroup->groupDescription() = "UNACCOMPANIED TRAVEL (ESCORT)";
    ;
    _respFormatter->replaceSpecialCharInDisplay(srvFeesGroup->groupDescription());
    std::string grpDesc = "UNACCOMPANIED TRAVEL -ESCORT-";
    CPPUNIT_ASSERT_EQUAL(grpDesc, srvFeesGroup->groupDescription());
  }

  void test_buildST1data_Empty()
  {
    std::string st1Ind = "";
    char indicator = '@';
    _respFormatter->buildST1data(st1Ind, &indicator);
    const std::string expected = "@";
    CPPUNIT_ASSERT(!st1Ind.empty());
    CPPUNIT_ASSERT_EQUAL(expected, st1Ind);
  }

  void test_buildST1data_Not_Empty()
  {
    std::string st1Ind = "X./";
    char indicator = '@';
    _respFormatter->buildST1data(st1Ind, &indicator);
    CPPUNIT_ASSERT(!st1Ind.empty());
    const std::string expected = "X./@";
    CPPUNIT_ASSERT_EQUAL(expected, st1Ind);
  }

  void test_buildST1data_Same_Ind()
  {
    std::string st1Ind = "@";
    char indicator = '@';
    _respFormatter->buildST1data(st1Ind, &indicator);
    CPPUNIT_ASSERT(!st1Ind.empty());
    const std::string expected = "@";
    CPPUNIT_ASSERT_EQUAL(expected, st1Ind);
  }

  void testGetFootnoteByHirarchyOrderWhenInformationOnly()
  {
    OCFees fee;
    SubCodeInfo sci;
    fee.subCodeInfo() = &sci;
    sci.serviceGroup() = "SA";
    OptionalServicesInfo osi;
    fee.optFee() = &osi;

    CPPUNIT_ASSERT_EQUAL('@', _respFormatter->getFootnoteByHirarchyOrder(fee));
  }

  void testGetFootnoteByHirarchyOrderWhenDisplayOnly()
  {
    OCFees fee;
    SubCodeInfo sci;
    fee.subCodeInfo() = &sci;
    fee.setDisplayOnly(true);

    CPPUNIT_ASSERT_EQUAL('*', _respFormatter->getFootnoteByHirarchyOrder(fee));
  }

  void testGetFootnoteByHirarchyOrderWhenNonRefundableN()
  {
    OCFees fee;
    SubCodeInfo sci;
    fee.subCodeInfo() = &sci;
    OptionalServicesInfo osi;
    fee.optFee() = &osi;
    osi.refundReissueInd() = 'N';

    CPPUNIT_ASSERT_EQUAL('N', _respFormatter->getFootnoteByHirarchyOrder(fee));
  }

  void testGetFootnoteByHirarchyOrderWhenNonRefundableR()
  {
    OCFees fee;
    SubCodeInfo sci;
    fee.subCodeInfo() = &sci;
    OptionalServicesInfo osi;
    fee.optFee() = &osi;
    osi.refundReissueInd() = 'R';

    CPPUNIT_ASSERT_EQUAL('N', _respFormatter->getFootnoteByHirarchyOrder(fee));
  }

  void testGetFootnoteByHirarchyOrderWhenPurchSameTimeAsTicket()
  {
    OCFees fee;
    SubCodeInfo sci;
    fee.subCodeInfo() = &sci;
    OptionalServicesInfo osi;
    fee.optFee() = &osi;
    osi.advPurchTktIssue() = 'X';

    CPPUNIT_ASSERT_EQUAL('P', _respFormatter->getFootnoteByHirarchyOrder(fee));
  }

  void testGetFootnoteByHirarchyOrderForEachItem()
  {
    OCFees fee;
    SubCodeInfo sci;
    fee.subCodeInfo() = &sci;
    OptionalServicesInfo osi;
    fee.optFee() = &osi;
    osi.frequentFlyerMileageAppl() = '3';

    CPPUNIT_ASSERT_EQUAL('/', _respFormatter->getFootnoteByHirarchyOrder(fee));
  }

  void testGetFootnoteByHirarchyOrderForEntireUnit()
  {
    OCFees fee;
    SubCodeInfo sci;
    fee.subCodeInfo() = &sci;
    OptionalServicesInfo osi;
    fee.optFee() = &osi;
    osi.frequentFlyerMileageAppl() = '4';

    CPPUNIT_ASSERT_EQUAL('X', _respFormatter->getFootnoteByHirarchyOrder(fee));
  }

  void testGetFootnoteByHirarchyOrderForEntireTicket()
  {
    OCFees fee;
    SubCodeInfo sci;
    fee.subCodeInfo() = &sci;
    OptionalServicesInfo osi;
    fee.optFee() = &osi;
    osi.frequentFlyerMileageAppl() = '5';

    CPPUNIT_ASSERT_EQUAL('-', _respFormatter->getFootnoteByHirarchyOrder(fee));
  }
  /*void test_setFeeIndicator_ForEachItem()
    {
    char feeApplyInd = ' ';
    bool isFeeApplFlag = false;
    _respFormatter->setFeeIndicator('3', feeApplyInd, isFeeApplFlag);
    CPPUNIT_ASSERT_EQUAL('/', feeApplyInd);
    CPPUNIT_ASSERT_EQUAL(true, isFeeApplFlag);
    }

    void test_setFeeIndicator_ForEntireUnit()
    {
    char feeApplyInd = ' ';
    bool isFeeApplFlag = false;
    _respFormatter->setFeeIndicator('4', feeApplyInd, isFeeApplFlag);
    CPPUNIT_ASSERT_EQUAL('X', feeApplyInd);
    CPPUNIT_ASSERT_EQUAL(true, isFeeApplFlag);
    }

    void test_setFeeIndicator_ForEntireTicket()
    {
    char feeApplyInd = ' ';
    bool isFeeApplFlag = false;
    _respFormatter->setFeeIndicator('5', feeApplyInd, isFeeApplFlag);
    CPPUNIT_ASSERT_EQUAL('-', feeApplyInd);
    CPPUNIT_ASSERT_EQUAL(true, isFeeApplFlag);
    }*/

  void test_fotmatOCTrailer()
  {
    _pTrx->getOptions()->isOCHistorical() = true;
    _respFormatter->formatOCTrailer(*_construct, "AD");
    const std::string expectedXML = "<OCT ST0=\"AD\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatOCGenericMsg()
  {
    _pTrx->getOptions()->isOCHistorical() = true;
    _respFormatter->formatOCGenericMsg(*_construct,
                                       "MORE AIR EXTRAS AVAILABLE - USE WPAE WITH QUALIFIERS");
    const std::string expectedXML =
        "<OCF><MSG S18=\"MORE AIR EXTRAS AVAILABLE - USE WPAE WITH QUALIFIERS\"/></OCF>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void createTravelSeg(int i)
  {
    AirSeg* airSeg = _memH.create<AirSeg>();
    airSeg->pnrSegment() = i;
    airSeg->segmentType() = Air;
    _itin->travelSeg().push_back(airSeg);
  }

  void createArunkSeg(int i)
  {
    AirSeg* airSeg = _memH.create<AirSeg>();
    airSeg->pnrSegment() = i;
    airSeg->segmentType() = tse::Arunk;
    _itin->travelSeg().push_back(airSeg);
  }

  void test_checkPNRSegmentNumberLogic_without_Arunk()
  {
    createTravelSeg(1);
    createTravelSeg(2);

    _construct->openElement("OCP");
    _respFormatter->checkPNRSegmentNumberLogic(*_pTrx, *_construct, 1, 2);
    _construct->closeElement();
    const std::string expectedXML = "<OCP SSG=\"1/2\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_checkPNRSegmentNumberLogic_with_Arunk()
  {
    createTravelSeg(1);
    createArunkSeg(2);

    _construct->openElement("OCP");
    _respFormatter->checkPNRSegmentNumberLogic(*_pTrx, *_construct, 1, 2);
    _construct->closeElement();
    const std::string expectedXML = "<OCP SSG=\"1\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_checkPNRSegmentNumberLogic_with_middle_Arunk()
  {
    createTravelSeg(1);
    createArunkSeg(2);
    createTravelSeg(3);

    _construct->openElement("OCP");
    _respFormatter->checkPNRSegmentNumberLogic(*_pTrx, *_construct, 1, 3);
    _construct->closeElement();
    const std::string expectedXML = "<OCP SSG=\"1/3\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_checkPNRSegmentNumberLogic_with_three_segs()
  {
    createTravelSeg(1);
    createTravelSeg(2);
    createTravelSeg(3);

    _construct->openElement("OCP");
    _respFormatter->checkPNRSegmentNumberLogic(*_pTrx, *_construct, 1, 3);
    _construct->closeElement();
    const std::string expectedXML = "<OCP SSG=\"1/2/3\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  // Adding SFH tag in PNR section
  void test_buildPNRDataWithIndex_0()
  {
    ServiceFeesGroup srvFeesGroup;
    PaxOCFeesUsages* paxOcFeesUsages = createPaxOCFeesUsages(true);
    Money equiv(0, USD);

    _respFormatter->buildPNRData(*_pTrx, *_construct, *paxOcFeesUsages, 0, equiv);
    const std::string expectedXML = "<OCP SHI=\"--\" SFA=\"0.00\" SFH=\"USD\" SFE=\"0.00\" "
                                    "SFF=\"BAGGAGE\" SSG=\"-1\" SHF=\"ADT\" SFD=\"Y\" SFU=\"BG\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildPNRDataWithIndex_0_WithBaseCurr()
  {
    ServiceFeesGroup srvFeesGroup;
    PaxOCFeesUsages* paxOcFeesUsages = createPaxOCFeesUsagesWithCurrencyAndAmount(true, "ML");
    Money equiv(20, USD);

    _respFormatter->buildPNRData(*_pTrx, *_construct, *paxOcFeesUsages, 0, equiv);
    const std::string expectedXML = "<OCP SHI=\"--\" SFA=\"15.00\" SFB=\"EUR\" SFC=\"20.00\" "
                                    "SFH=\"USD\" SFE=\"20.00\" SFF=\"BAGGAGE\" SSG=\"-1\" "
                                    "SHF=\"ADT\" SFD=\"Y\" SFU=\"ML\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildPNRData_with_index_value()
  {
    DateTime time = DateTime::localTime();
    std::string shm = time.dateToString(YYYYMMDD, "-").c_str();
    ServiceFeesGroup srvFeesGroup;
    PaxOCFeesUsages* paxOcFeesUsages = createPaxOCFeesUsagesWithCurrencyAndAmount("UN");
    Money equiv(20, USD);
    (const_cast<OptionalServicesInfo*>(paxOcFeesUsages->fees()->optFee()))->sectorPortionInd() =
        'P';

    _respFormatter->buildPNRData(*_pTrx, *_construct, *paxOcFeesUsages, 3, equiv);
    //    const std::string expectedXML = "<OCP SHI=\"3\" SFA=\"15.00\" SFB=\"EUR\" SFC=\"20.00\"
    // SFH=\"USD\" SFF=\"BAGGAGE\" SSG=\"-1\" SHF=\"ADT\" SFD=\"N\" SFU=\"BG\" SFJ=\"E\" SFM=\"P\"
    // SFR=\"T\" SFZ=\"80/01/01\" SHA=\"00/00/00\" SHM=\"2011-03-17\" SHN=\"KRKMUC\"/>";
    std::string xml = "<OCP SHI=\"3\" SFA=\"15.00\" SFB=\"EUR\" SFC=\"20.00\" SFH=\"USD\" "
                      "SFE=\"20.00\" SFF=\"BAGGAGE\" SSG=\"-1\" SHF=\"ADT\" SFD=\"N\" SFU=\"BG\" "
                      "SFJ=\"E\" SFM=\"P\" SFR=\"T\" SFZ=\"1980-01-01\" SHA=\"0000-00-00\" SHM=\"";
    std::string ending = "\" SHN=\"KRKMUC\"/>";
    std::string expectedXML = xml + shm + ending;

    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_buildPNRDataForNonDisplayFees_AX1_Tag()
  {
    DateTime time = DateTime::localTime();
    std::string shm = time.dateToString(YYYYMMDD, "-").c_str();
    _construct->openElement(xml2::OCFeesPNRInfo);
    ServiceFeesGroup srvFeesGroup;
    PaxOCFeesUsages* paxOcFeesUsages = createPaxOCFeesUsagesWithCurrencyAndAmount("UN");
    Money equiv(20, USD);
    (const_cast<OptionalServicesInfo*>(paxOcFeesUsages->fees()->optFee()))->sectorPortionInd() =
        'P';
    (const_cast<OptionalServicesInfo*>(paxOcFeesUsages->fees()->optFee()))->availabilityInd() = 'Y';
    (const_cast<OptionalServicesInfo*>(paxOcFeesUsages->fees()->optFee()))->notAvailNoChargeInd() =
        'F';

    _respFormatter->buildPNRDataForNonDisplayFees(*_pTrx, *_construct, *paxOcFeesUsages);
    _construct->closeElement();
    std::string xml =
        "<OCP SFJ=\"E\" SFM=\"P\" SFR=\"T\" SFZ=\"1980-01-01\" SHA=\"0000-00-00\" SHM=\"";
    std::string ending = "\" SHN=\"KRKMUC\" AX1=\"T\"/>";
    std::string expectedXML = xml + shm + ending;

    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  XMLConstruct& getExpectResult(const CarrierCode& carrier)
  {
    XMLConstruct& xml = *_memH(new XMLConstruct);
    xml.openElement(xml2::SummaryInfo);
    if (!carrier.empty())
      xml.addAttribute(xml2::ValidatingCarrier, carrier);
    xml.closeElement();
    return xml;
  }

  void testPrepareValidatingCarrierAttr_EffDate()
  {
    _pRequest->validatingCarrier() = CARRIER_9B;
    _pTrx->ticketingDate() = DateTime(2026, 1, 1);

    _construct->openElement(xml2::SummaryInfo);
    _respFormatter->prepareValidatingCarrierAttr(*_pTrx, *_construct);
    _construct->closeElement();

    CPPUNIT_ASSERT_EQUAL(getExpectResult(CARRIER_9B).getXMLData(), _construct->getXMLData());
  }

  void testPrepareValidatingCarrierAttr_EffDate_BadCarrier()
  {
    _pRequest->validatingCarrier() = EMPTY_STRING();
    _pTrx->ticketingDate() = DateTime(2026, 1, 1);

    _construct->openElement(xml2::SummaryInfo);
    _respFormatter->prepareValidatingCarrierAttr(*_pTrx, *_construct);
    _construct->closeElement();

    CPPUNIT_ASSERT_EQUAL(getExpectResult(EMPTY_STRING()).getXMLData(), _construct->getXMLData());
  }

  void testPreparePricingUnitPlusUps_hrt()
  {
    CurrencyNoDec noDecCalc = 2;
    PricingUnit pu;
    MinFarePlusUpItem hrtPlusUp;
    hrtPlusUp.constructPoint = "";
    hrtPlusUp.boardPoint = "ABC";
    hrtPlusUp.offPoint = "DEF";
    hrtPlusUp.plusUpAmount = 20.00;

    pu.minFarePlusUp().addItem(HRT, &hrtPlusUp);
    _respFormatter->preparePricingUnitPlusUps(pu, noDecCalc, *_pTrx, *_construct);
    CPPUNIT_ASSERT_EQUAL(std::string("<PUP C6L=\"20.00\" A11=\"ABC\" A12=\"DEF\" S68=\"HRTC\"/>"),
                         _construct->getXMLData());
  }

  void testIsAncillariesGuarantee_For_AutoPriced_Solution()
  {
    FarePath path;
    path.fuelSurchargeIgnored() = false;
    CPPUNIT_ASSERT(!_formatter->isAncillaryNonGuarantee(*_pTrx, &path));
  }

  void testIsAncillariesNonGuarantee_For_WPNQF_IgnoreFuelSurcharge()
  {
    FarePath path;
    path.fuelSurchargeIgnored() = true;
    CPPUNIT_ASSERT(_formatter->isAncillaryNonGuarantee(*_pTrx, &path));
  }

  void testIsAncillariesNonGuarantee_For_WPQFareBasisCode_3_point_5()
  {
    FarePath path;
    _pOptions->fbcSelected() = true;
    CPPUNIT_ASSERT(_formatter->isAncillaryNonGuarantee(*_pTrx, &path));
  }

  void testIsAncillariesNonGuarantee_For_WPOC_Bx_BookingCodeOverride_request()
  {
    FarePath path;
    _pOptions->bookingCodeOverride() = true;
    CPPUNIT_ASSERT(_formatter->isAncillaryNonGuarantee(*_pTrx, &path));
  }

  void testIsAncillariesNonGuarantee_For_WPR_BSR_SecondRateAmountOverride()
  {
    FarePath path;
    _pRequest->secondRateAmountOverride() = true;
    CPPUNIT_ASSERT(_formatter->isAncillaryNonGuarantee(*_pTrx, &path));
  }

  void testIsAncillariesNonGuarantee_For_WPR_BSR_FirstRateAmountOverride()
  {
    FarePath path;
    _pRequest->rateAmountOverride() = true;
    CPPUNIT_ASSERT(_formatter->isAncillaryNonGuarantee(*_pTrx, &path));
  }

  void testIsAncillariesNonGuarantee_For_WPXPTS_PaperTktSurchargeOverride()
  {
    FarePath path;
    const char tr = 'T';
    _pRequest->ptsOverride() = tr;
    CPPUNIT_ASSERT(_formatter->isAncillaryNonGuarantee(*_pTrx, &path));
  }

  void testIsAncillariesNonGuarantee_For_WPQDP_3_point_75()
  {
    FarePath path;
    _pRequest->addDiscountPercentage(1, 20.0);

    CPPUNIT_ASSERT(_formatter->isAncillaryNonGuarantee(*_pTrx, &path));
  }

  void testIsAncillariesNonGuarantee_For_WPQDA_3_point_75()
  {
    FarePath path;
    _pRequest->addDiscountAmountNew(1, 1, MoneyAmount(100.00), "USD");
    CPPUNIT_ASSERT(_formatter->isAncillaryNonGuarantee(*_pTrx, &path));
  }

  void testIsAncillariesNonGuarantee_For_WPC_AA_GoverningCxrOverride()
  {
    FarePath path;
    _pRequest->governingCarrierOverrides().insert(std::pair<int16_t, CarrierCode>('1', "AA"));
    CPPUNIT_ASSERT(_formatter->isAncillaryNonGuarantee(*_pTrx, &path));
  }

  void testIsAncillariesNonGuarantee_For_WPC_YY_IndustryCxrOverride()
  {
    FarePath path;
    _pRequest->industryFareOverrides().push_back('1');
    CPPUNIT_ASSERT(_formatter->isAncillaryNonGuarantee(*_pTrx, &path));
  }

  void testIsTaxExempted_TN()
  {
    std::string taxCode = "UOE";
    bool isExemptAllTaxes = true;
    bool isExemptSpecificTaxes = false;
    std::vector<std::string> taxIdExempted;

    CPPUNIT_ASSERT(
        _formatter->isTaxExempted(taxCode, isExemptAllTaxes, isExemptSpecificTaxes, taxIdExempted));
  }

  void testIsTaxExempted_TE()
  {
    std::string taxCode = "UOE";
    bool isExemptAllTaxes = false;
    bool isExemptSpecificTaxes = true;
    std::vector<std::string> taxIdExempted;

    CPPUNIT_ASSERT(
        _formatter->isTaxExempted(taxCode, isExemptAllTaxes, isExemptSpecificTaxes, taxIdExempted));
  }

  void testIsTaxExempted_TE_UO()
  {
    std::string taxCode = "UOE";
    bool isExemptAllTaxes = false;
    bool isExemptSpecificTaxes = true;
    std::vector<std::string> taxIdExempted;

    taxIdExempted.push_back("UO");

    CPPUNIT_ASSERT(
        _formatter->isTaxExempted(taxCode, isExemptAllTaxes, isExemptSpecificTaxes, taxIdExempted));
  }

  void testIsTaxExempted_TE_UOE()
  {
    std::string taxCode = "UOE";
    bool isExemptAllTaxes = false;
    bool isExemptSpecificTaxes = true;
    std::vector<std::string> taxIdExempted;

    taxIdExempted.push_back("UOE");

    CPPUNIT_ASSERT(
        _formatter->isTaxExempted(taxCode, isExemptAllTaxes, isExemptSpecificTaxes, taxIdExempted));
  }

  void testIsTaxExempted_TE_UOD()
  {
    std::string taxCode = "UOE";
    bool isExemptAllTaxes = false;
    bool isExemptSpecificTaxes = true;
    std::vector<std::string> taxIdExempted;

    taxIdExempted.push_back("UOD");

    CPPUNIT_ASSERT(!_formatter->isTaxExempted(
        taxCode, isExemptAllTaxes, isExemptSpecificTaxes, taxIdExempted));
  }

  void testSetEndTaxOnOcIterator_OneItem()
  {
    std::vector<OCFees::TaxItem> taxItems;
    OCFees::TaxItem taxItem;
    taxItems.push_back(taxItem);

    std::vector<OCFees::TaxItem>::const_iterator itEnd = taxItems.end();

    CPPUNIT_ASSERT(_formatter->setEndTaxOnOcIterator(taxItems) == itEnd);
  }

  void testSetEndTaxOnOcIterator_SixteenItems()
  {
    std::vector<OCFees::TaxItem> taxItems;
    OCFees::TaxItem taxItem;

    for (int i = 0; i != 16; i++)
      taxItems.push_back(taxItem);

    std::vector<OCFees::TaxItem>::const_iterator itEnd = taxItems.end();

    CPPUNIT_ASSERT(_formatter->setEndTaxOnOcIterator(taxItems) != itEnd);
  }

  void fillOcFeesUsage(OCFees* ocFees, bool fill = false)
  {
    OCFeesUsage* ocfUsage = _memH.create<OCFeesUsage>();
    if (!ocfUsage)
      return;

    ocfUsage->oCFees() = ocFees;
    ocfUsage->setSegIndex(1);
    ocfUsage->upgradeT198Sequence() = _memH.create<SvcFeesResBkgDesigInfo>();
    ocfUsage->upgradeT198CommercialName() = "PADIS TEST";

    ocFees->ocfeeUsage().push_back(ocfUsage);
  }

  void test_buildPNRData_PADIS_WPAE()
  {
    OCFees* ocFee = createOCFees(false, "SA", "RIGHT SIDE");
    OCFeesUsage* ocFeesUsage = ocFee->ocfeeUsage().front();
    FPOCFeesUsages* fpOcFeesUsages =
        _memH.insert(new FPOCFeesUsages(createFarePath(), ocFeesUsage));
    PaxOCFeesUsages* paxOcFeesUsages = _memH.insert(new PaxOCFeesUsages(*fpOcFeesUsages));

    AncRequest* ancReq = _memH.create<AncRequest>();
    ancReq->ancRequestType() = AncRequest::WPAERequest;
    _pTrx->setRequest(ancReq);

    Money equiv(0, USD);

    _respFormatter->buildPNRData(*_pTrx, *_construct, *paxOcFeesUsages, 0, equiv);
    const std::string expectedXML = "<OCP SHI=\"--\" SFA=\"0.00\" SFH=\"USD\" SFE=\"0.00\" "
                                    "SFF=\"RIGHT SIDE\" SSG=\"-1\" SHF=\"ADT\" SFD=\"Y\" "
                                    "SFU=\"SA\"/>";
    CPPUNIT_ASSERT_EQUAL(expectedXML, _construct->getXMLData());
  }

  void test_formatOCFeesLine()
  {
    OCFees* ocFee = createOCFees(false, "SA", "RIGHT SIDE");
    OCFeesUsage* ocFeesUsage = ocFee->ocfeeUsage().front();
    FPOCFeesUsages* fpOcFeesUsages =
        _memH.insert(new FPOCFeesUsages(createFarePath(), ocFeesUsage));
    PaxOCFeesUsages* paxOcFeesUsages = _memH.insert(new PaxOCFeesUsages(*fpOcFeesUsages));

    AncRequest* ancReq = _memH.create<AncRequest>();
    ancReq->ancRequestType() = AncRequest::WPAERequest;
    _pTrx->setRequest(ancReq);
    _respFormatter->formatOCFeesLine(*_pTrx, *_construct, *paxOcFeesUsages, 0, EMPTY);
    CPPUNIT_ASSERT(_construct->getXMLData().find("SFF=\"RIGHT SIDE\"") != std::string::npos);
  }

  void test_buildCarryOnAllowanceBDI()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    OCFees* ocFees1 = OCFeesBuilder(&_memH).build();
    OCFees* ocFees2 = OCFeesBuilder(&_memH).build();
    OCFees* ocFees3 = OCFeesBuilder(&_memH).build();

    initAllowanceBaggageTravels(baggageTravels, ocFees1, ocFees2, ocFees3);

    _respFormatter->buildCarryOnAllowanceBDI(*_pTrx, *_construct, baggageTravels);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>2</Q00><Q00>3</Q00></BDI>") !=
                   std::string::npos);
  }

  void test_buildCarryOnAllowanceBDI_different_carrier()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    OCFees* ocFees1 = OCFeesBuilder(&_memH).withCarrier("LT").build();
    OCFees* ocFees2 = OCFeesBuilder(&_memH).build();
    OCFees* ocFees3 = OCFeesBuilder(&_memH).withCarrier("LT").build();

    initAllowanceBaggageTravels(baggageTravels, ocFees1, ocFees2, ocFees3);

    _respFormatter->buildCarryOnAllowanceBDI(*_pTrx, *_construct, baggageTravels);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>3</Q00></BDI>") !=
                   std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>2</Q00></BDI>") != std::string::npos);
  }

  void test_buildCarryOnAllowanceBDI_different_freeBaggagePcs()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    OCFees* ocFees1 = OCFeesBuilder(&_memH).withFreeBaggagePcs(2).build();
    OCFees* ocFees2 = OCFeesBuilder(&_memH).build();
    OCFees* ocFees3 = OCFeesBuilder(&_memH).withFreeBaggagePcs(2).build();

    initAllowanceBaggageTravels(baggageTravels, ocFees1, ocFees2, ocFees3);

    _respFormatter->buildCarryOnAllowanceBDI(*_pTrx, *_construct, baggageTravels);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>3</Q00></BDI>") !=
                   std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>2</Q00></BDI>") != std::string::npos);
  }

  void test_buildCarryOnAllowanceBDI_different_baggageWeightUnit()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    OCFees* ocFees1 = OCFeesBuilder(&_memH).withBaggageWeightUnit('K').build();
    OCFees* ocFees2 = OCFeesBuilder(&_memH).build();
    OCFees* ocFees3 = OCFeesBuilder(&_memH).withBaggageWeightUnit('K').build();

    initAllowanceBaggageTravels(baggageTravels, ocFees1, ocFees2, ocFees3);

    _respFormatter->buildCarryOnAllowanceBDI(*_pTrx, *_construct, baggageTravels);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>3</Q00></BDI>") !=
                   std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>2</Q00></BDI>") != std::string::npos);
  }

  void test_buildCarryOnAllowanceBDI_different_baggageOccurrenceFirstPc()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    OCFees* ocFees1 = OCFeesBuilder(&_memH).withBaggageOccurrenceFirstPc(1).build();
    OCFees* ocFees2 = OCFeesBuilder(&_memH).build();
    OCFees* ocFees3 = OCFeesBuilder(&_memH).withBaggageOccurrenceFirstPc(1).build();

    initAllowanceBaggageTravels(baggageTravels, ocFees1, ocFees2, ocFees3);

    _respFormatter->buildCarryOnAllowanceBDI(*_pTrx, *_construct, baggageTravels);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>3</Q00></BDI>") !=
                   std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>2</Q00></BDI>") != std::string::npos);
  }

  void test_buildCarryOnAllowanceBDI_different_baggageOccurrenceLastPc()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    OCFees* ocFees1 = OCFeesBuilder(&_memH).withBaggageOccurrenceLastPc(1).build();
    OCFees* ocFees2 = OCFeesBuilder(&_memH).build();
    OCFees* ocFees3 = OCFeesBuilder(&_memH).withBaggageOccurrenceLastPc(1).build();

    initAllowanceBaggageTravels(baggageTravels, ocFees1, ocFees2, ocFees3);

    _respFormatter->buildCarryOnAllowanceBDI(*_pTrx, *_construct, baggageTravels);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>3</Q00></BDI>") !=
                   std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>2</Q00></BDI>") != std::string::npos);
  }

  void test_buildCarryOnChargesBDI()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    OCFees* ocFees1 = OCFeesBuilder(&_memH).build();
    OCFees* ocFees2 = OCFeesBuilder(&_memH).build();
    OCFees* ocFees3 = OCFeesBuilder(&_memH).build();

    initChargesBaggageTravels(baggageTravels, ocFees1, ocFees2, ocFees3);

    _respFormatter->buildCarryOnChargesBDI(*_pTrx, *_construct, baggageTravels);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>2</Q00><Q00>3</Q00></BDI>") !=
                   std::string::npos);
  }

  void test_buildCarryOnChargesBDI_different_advPurchTktIssue()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    OCFees* ocFees1 = OCFeesBuilder(&_memH).withAdvPurchTktIssue('X').build();
    OCFees* ocFees2 = OCFeesBuilder(&_memH).build();
    OCFees* ocFees3 = OCFeesBuilder(&_memH).withAdvPurchTktIssue('X').build();

    initChargesBaggageTravels(baggageTravels, ocFees1, ocFees2, ocFees3);

    _respFormatter->buildCarryOnChargesBDI(*_pTrx, *_construct, baggageTravels);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>2</Q00><Q00>3</Q00></BDI>") !=
                   std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>2</Q00></BDI>") !=
                   std::string::npos);
  }

  void test_buildCarryOnChargesBDI_different_notAvailNoChargeInd()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    OCFees* ocFees1 = OCFeesBuilder(&_memH).withNoAvailNoChargeInd('X').build();
    OCFees* ocFees2 = OCFeesBuilder(&_memH).build();
    OCFees* ocFees3 = OCFeesBuilder(&_memH).withNoAvailNoChargeInd('X').build();

    initChargesBaggageTravels(baggageTravels, ocFees1, ocFees2, ocFees3);

    _respFormatter->buildCarryOnChargesBDI(*_pTrx, *_construct, baggageTravels);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>2</Q00><Q00>3</Q00></BDI>") !=
                   std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>2</Q00></BDI>") !=
                   std::string::npos);
  }

  void test_buildCarryOnChargesBDI_different_formOfFeeRefundInd()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    OCFees* ocFees1 = OCFeesBuilder(&_memH).withFormOfFeeRefundInd('X').build();
    OCFees* ocFees2 = OCFeesBuilder(&_memH).build();
    OCFees* ocFees3 = OCFeesBuilder(&_memH).withFormOfFeeRefundInd('X').build();

    initChargesBaggageTravels(baggageTravels, ocFees1, ocFees2, ocFees3);

    _respFormatter->buildCarryOnChargesBDI(*_pTrx, *_construct, baggageTravels);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>2</Q00><Q00>3</Q00></BDI>") !=
                   std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>2</Q00></BDI>") !=
                   std::string::npos);
  }

  void test_buildCarryOnChargesBDI_different_refundReissueInd()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    OCFees* ocFees1 = OCFeesBuilder(&_memH).withRefundReissueInd('X').build();
    OCFees* ocFees2 = OCFeesBuilder(&_memH).build();
    OCFees* ocFees3 = OCFeesBuilder(&_memH).withRefundReissueInd('X').build();

    initChargesBaggageTravels(baggageTravels, ocFees1, ocFees2, ocFees3);

    _respFormatter->buildCarryOnChargesBDI(*_pTrx, *_construct, baggageTravels);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>2</Q00><Q00>3</Q00></BDI>") !=
                   std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>2</Q00></BDI>") !=
                   std::string::npos);
  }

  void test_buildCarryOnChargesBDI_different_commissionInd()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    OCFees* ocFees1 = OCFeesBuilder(&_memH).withCommissionInd('X').build();
    OCFees* ocFees2 = OCFeesBuilder(&_memH).build();
    OCFees* ocFees3 = OCFeesBuilder(&_memH).withCommissionInd('X').build();

    initChargesBaggageTravels(baggageTravels, ocFees1, ocFees2, ocFees3);

    _respFormatter->buildCarryOnChargesBDI(*_pTrx, *_construct, baggageTravels);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>2</Q00><Q00>3</Q00></BDI>") !=
                   std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>2</Q00></BDI>") !=
                   std::string::npos);
  }

  void test_buildCarryOnChargesBDI_different_interlineInd()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    OCFees* ocFees1 = OCFeesBuilder(&_memH).withInterlineInd('M').build();
    OCFees* ocFees2 = OCFeesBuilder(&_memH).build();
    OCFees* ocFees3 = OCFeesBuilder(&_memH).withInterlineInd('M').build();

    initChargesBaggageTravels(baggageTravels, ocFees1, ocFees2, ocFees3);

    _respFormatter->buildCarryOnChargesBDI(*_pTrx, *_construct, baggageTravels);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>2</Q00><Q00>3</Q00></BDI>") !=
                   std::string::npos);
    CPPUNIT_ASSERT(_construct->getXMLData().find("<Q00>1</Q00><Q00>2</Q00></BDI>") !=
                   std::string::npos);
  }

  void buildOSC_mandatory()
  {
    SubCodeInfo s5;
    s5.fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    s5.serviceSubTypeCode() = "ABC";
    s5.vendor() = ATPCO_VENDOR_CODE;
    s5.carrier() = "AA";
    s5.commercialName() = "COMMERCIAL NAME";
    s5.serviceGroup() = "DEF";

    _respFormatter->buildOSC(*_pTrx, &s5, *_construct);

    std::string expected = "<OSC SHK=\"ABCAFAA\" SFF=\"COMMERCIAL NAME\" SF0=\"DEF\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void buildOSC_subGroup_emptyDefinition()
  {
    SubCodeInfo s5;
    s5.fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    s5.serviceSubTypeCode() = "ABC";
    s5.vendor() = ATPCO_VENDOR_CODE;
    s5.carrier() = "AA";
    s5.commercialName() = "COMMERCIAL NAME";
    s5.serviceGroup() = "XXX";
    s5.serviceSubGroup() = "XXX";

    _respFormatter->buildOSC(*_pTrx, &s5, *_construct);

    std::string expected = "<OSC SHK=\"ABCAFAA\" SFF=\"COMMERCIAL NAME\" SF0=\"XXX\" ASG=\"XXX\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void buildOSC_subGroup()
  {
    SubCodeInfo s5;
    s5.fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    s5.serviceSubTypeCode() = "ABC";
    s5.vendor() = ATPCO_VENDOR_CODE;
    s5.carrier() = "AA";
    s5.commercialName() = "COMMERCIAL NAME";
    s5.serviceGroup() = "GGG";
    s5.serviceSubGroup() = "SSS";

    _respFormatter->buildOSC(*_pTrx, &s5, *_construct);

    std::string expected = "<OSC SHK=\"ABCAFAA\" SFF=\"COMMERCIAL NAME\" SF0=\"GGG\" ASG=\"SSS\" "
                           "ASD=\"DEFINITION\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void buildOSC_optional()
  {
    SubCodeInfo s5;
    s5.fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    s5.serviceSubTypeCode() = "ABC";
    s5.vendor() = ATPCO_VENDOR_CODE;
    s5.carrier() = "AA";
    s5.commercialName() = "COMMERCIAL NAME";
    s5.serviceGroup() = "GGG";
    s5.serviceSubGroup() = "SSS";
    s5.description1() = "01";
    s5.description2() = "02";
    s5.rfiCode() = 'R';
    s5.ssrCode() = "SSRC";
    s5.emdType() = 'E';
    s5.bookingInd() = "BI";

    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("01", 0, 'I', 'U');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("02", 0, 'C', 'U');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("01", 0, 'L', 'U');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("02", 0, 'K', 'U');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("01", 0, 'L', 'O');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("02", 0, 'K', 'O');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("01", 0, 'I', 'O');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("02", 0, 'C', 'O');

    _respFormatter->buildOSC(*_pTrx, &s5, *_construct);

    std::string expected = "<OSC SHK=\"ABCAFAA\" SFF=\"COMMERCIAL NAME\" "
                           "SF0=\"GGG\" ASG=\"SSS\" ASD=\"DEFINITION\" "
                           "DC1=\"01\" D01=\"DESCRIPTION 01\" "
                           "DC2=\"02\" D02=\"DESCRIPTION 02\" "
                           "W01=\"0\" WU1=\"L\" W02=\"0\" WU2=\"K\" "
                           "W03=\"0\" WU3=\"L\" W04=\"0\" WU4=\"K\" "
                           "S01=\"0\" SU1=\"I\" S02=\"0\" SU2=\"C\" "
                           "S03=\"0\" SU3=\"I\" S04=\"0\" SU4=\"C\" "
                           "N01=\"R\" SHL=\"SSRC\" N02=\"E\" SFN=\"BI\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_buildExtendedSubCodeKey_baggageProvision()
  {
    SubCodeInfo s5;
    s5.fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    s5.serviceSubTypeCode() = "ABC";
    s5.vendor() = ATPCO_VENDOR_CODE;
    s5.carrier() = "AA";

    std::string expected = "ABCAXXAA";
    CPPUNIT_ASSERT_EQUAL(expected, _respFormatter->buildExtendedSubCodeKey(&s5, BaggageProvisionType("XX")));
  }

  void test_buildExtendedSubCodeKey()
  {
    SubCodeInfo s5;
    s5.fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    s5.serviceSubTypeCode() = "ABC";
    s5.vendor() = ATPCO_VENDOR_CODE;
    s5.carrier() = "AA";

    std::string expected = "ABCAFAA";
    CPPUNIT_ASSERT_EQUAL(expected, _respFormatter->buildExtendedSubCodeKey(&s5, BaggageProvisionType(s5.fltTktMerchInd())));
  }

  void test_buildExtendedSubCodeKey_merchManager()
  {
    SubCodeInfo s5;
    s5.fltTktMerchInd() = FLIGHT_RELATED_SERVICE;
    s5.serviceSubTypeCode() = "ABC";
    s5.vendor() = MERCH_MANAGER_VENDOR_CODE;
    s5.carrier() = "AA";

    std::string expected = "ABCMFAA";
    CPPUNIT_ASSERT_EQUAL(expected, _respFormatter->buildExtendedSubCodeKey(&s5, BaggageProvisionType(s5.fltTktMerchInd())));
  }

  void test_buildSubCodeDescription_firstEmpty()
  {
    SubCodeInfo subCodeInfo;
    subCodeInfo.description1() = "";
    subCodeInfo.description2() = "02";

    _construct->openElement("A");
    _respFormatter->buildSubCodeDescription(*_pTrx, &subCodeInfo, *_construct);
    _construct->closeElement();

    std::string expected = "<A/>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_buildSubCodeDescription_firstNotFound()
  {
    SubCodeInfo subCodeInfo;
    subCodeInfo.description1() = "XX";
    subCodeInfo.description2() = "02";

    _construct->openElement("A");
    _respFormatter->buildSubCodeDescription(*_pTrx, &subCodeInfo, *_construct);
    _construct->closeElement();

    std::string expected = "<A/>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_buildSubCodeDescription_first()
  {
    SubCodeInfo subCodeInfo;
    subCodeInfo.description1() = "01";
    subCodeInfo.description2() = "";

    _construct->openElement("A");
    _respFormatter->buildSubCodeDescription(*_pTrx, &subCodeInfo, *_construct);
    _construct->closeElement();

    std::string expected = "<A DC1=\"01\" D01=\"DESCRIPTION 01\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_buildSubCodeDescription_secondNotFound()
  {
    SubCodeInfo subCodeInfo;
    subCodeInfo.description1() = "01";
    subCodeInfo.description2() = "XX";

    _construct->openElement("A");
    _respFormatter->buildSubCodeDescription(*_pTrx, &subCodeInfo, *_construct);
    _construct->closeElement();

    std::string expected = "<A DC1=\"01\" D01=\"DESCRIPTION 01\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_buildSubCodeDescription_firstAndSecond()
  {
    SubCodeInfo subCodeInfo;
    subCodeInfo.description1() = "01";
    subCodeInfo.description2() = "02";

    _construct->openElement("A");
    _respFormatter->buildSubCodeDescription(*_pTrx, &subCodeInfo, *_construct);
    _construct->closeElement();

    std::string expected = "<A DC1=\"01\" D01=\"DESCRIPTION 01\" "
                           "DC2=\"02\" D02=\"DESCRIPTION 02\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_buildSubCodeRestrictions()
  {
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("AB", 0, 'I', 'U');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("XX", 0, 'I', 'U');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("CD", 0, 'C', 'U');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("AB", 0, 'X', 'O');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("AB", 0, 'L', 'U');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("CD", 0, 'K', 'U');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("AB", 0, 'L', 'O');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("XX", 0, 'K', 'O');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("CD", 0, 'K', 'O');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("AB", 0, 'I', 'X');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("AB", 0, 'I', 'O');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("CD", 0, 'C', 'O');

    SubCodeInfo s5;
    s5.description1() = "AB";
    s5.description2() = "CD";

    _construct->openElement("A");
    _respFormatter->buildSubCodeRestrictions(&s5, *_construct);
    _construct->closeElement();

    std::string expected = "<A "
                           "W01=\"0\" WU1=\"L\" W02=\"0\" WU2=\"K\" "
                           "W03=\"0\" WU3=\"L\" W04=\"0\" WU4=\"K\" "
                           "S01=\"0\" SU1=\"I\" S02=\"0\" SU2=\"C\" "
                           "S03=\"0\" SU3=\"I\" S04=\"0\" SU4=\"C\""
                           "/>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_buildSubCodeRestrictionWithAlternativeUnits_none()
  {
    PricingResponseFormatter::SubCodeRestrictionsComparator comparator('A', 'B', 'C');

    std::vector<const PricingResponseFormatter::BaggageSizeWeightDescription*> descriptions;
    PricingResponseFormatter::BaggageSizeWeightDescription description1("XX", 0, 'X', 'X');
    descriptions += &description1;

    _construct->openElement("A");
    _respFormatter->buildSubCodeRestrictionWithAlternativeUnits(
        "VA1", "UA1", "VA2", "UA2", comparator, descriptions, *_construct);
    _construct->closeElement();

    std::string expected = "<A/>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_buildSubCodeRestrictionWithAlternativeUnits_one()
  {
    PricingResponseFormatter::SubCodeRestrictionsComparator comparator('A', 'B', 'C');

    std::vector<const PricingResponseFormatter::BaggageSizeWeightDescription*> descriptions;
    PricingResponseFormatter::BaggageSizeWeightDescription description1("XX", 0, 'A', 'C');
    descriptions += &description1;
    PricingResponseFormatter::BaggageSizeWeightDescription description2("XX", 0, 'X', 'C');
    descriptions += &description2;

    _construct->openElement("A");
    _respFormatter->buildSubCodeRestrictionWithAlternativeUnits(
        "VA1", "UA1", "VA2", "UA2", comparator, descriptions, *_construct);
    _construct->closeElement();

    std::string expected = "<A VA1=\"0\" UA1=\"A\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_buildSubCodeRestrictionWithAlternativeUnits_both()
  {
    PricingResponseFormatter::SubCodeRestrictionsComparator comparator('A', 'B', 'C');

    std::vector<const PricingResponseFormatter::BaggageSizeWeightDescription*> descriptions;
    PricingResponseFormatter::BaggageSizeWeightDescription description1("XX", 0, 'A', 'C');
    descriptions += &description1;
    PricingResponseFormatter::BaggageSizeWeightDescription description2("XX", 0, 'X', 'C');
    descriptions += &description2;
    PricingResponseFormatter::BaggageSizeWeightDescription description3("XX", 0, 'B', 'C');
    descriptions += &description3;

    _construct->openElement("A");
    _respFormatter->buildSubCodeRestrictionWithAlternativeUnits(
        "VA1", "UA1", "VA2", "UA2", comparator, descriptions, *_construct);
    _construct->closeElement();

    std::string expected = "<A VA1=\"0\" UA1=\"A\" VA2=\"0\" UA2=\"B\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_buildSubCodeRestriction()
  {
    _construct->openElement("A");
    _respFormatter->buildSubCodeRestriction("AT1", 123.45, "AT2", 'A', *_construct);
    _construct->closeElement();

    std::string expected = "<A AT1=\"123.45\" AT2=\"A\"/>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_getSubCodeRestrictions()
  {
    std::vector<const PricingResponseFormatter::BaggageSizeWeightDescription*> restrictions;

    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("AB", 0, 'X', 'X');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("XX", 0, 'X', 'X');
    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("AB", 0, 'X', 'X');

    _respFormatter->getSubCodeRestrictions("AB", restrictions);
    CPPUNIT_ASSERT_EQUAL((size_t)2, restrictions.size());
  }

  void test_getSubCodeRestrictions_empty()
  {
    std::vector<const PricingResponseFormatter::BaggageSizeWeightDescription*> restrictions;

    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("", 0, 'X', 'X');

    _respFormatter->getSubCodeRestrictions("", restrictions);
    CPPUNIT_ASSERT(restrictions.empty());
  }

  void test_getSubCodeRestrictions_notFound()
  {
    std::vector<const PricingResponseFormatter::BaggageSizeWeightDescription*> restrictions;

    _respFormatter->_baggageSizeWeightRestrictions +=
        PricingResponseFormatter::BaggageSizeWeightDescription("AB", 0, 'X', 'X');

    _respFormatter->getSubCodeRestrictions("XX", restrictions);
    CPPUNIT_ASSERT(restrictions.empty());
  }

  void test_SubCodeRestrictionsComparator_true()
  {
    PricingResponseFormatter::BaggageSizeWeightDescription desc("XX", 123, 'K', 'O');
    PricingResponseFormatter::SubCodeRestrictionsComparator comparator('K', 'L', 'O');

    CPPUNIT_ASSERT(comparator(&desc));
  }

  void test_SubCodeRestrictionsComparator_true_alternative()
  {
    PricingResponseFormatter::BaggageSizeWeightDescription desc("XX", 123, 'L', 'O');
    PricingResponseFormatter::SubCodeRestrictionsComparator comparator('K', 'L', 'O');

    CPPUNIT_ASSERT(comparator(&desc));
  }

  void test_SubCodeRestrictionsComparator_false_unit()
  {
    PricingResponseFormatter::BaggageSizeWeightDescription desc("XX", 123, 'X', 'O');
    PricingResponseFormatter::SubCodeRestrictionsComparator comparator('K', 'L', 'O');

    CPPUNIT_ASSERT(!comparator(&desc));
  }

  void test_SubCodeRestrictionsComparator_false_limit()
  {
    PricingResponseFormatter::BaggageSizeWeightDescription desc("XX", 123, 'K', 'X');
    PricingResponseFormatter::SubCodeRestrictionsComparator comparator('K', 'L', 'O');

    CPPUNIT_ASSERT(!comparator(&desc));
  }

  void test_buildBDI_BaggageChargesWithFrequentFlyerWarning()
  {
    OCFeesBuilder ocBuilder = OCFeesBuilder(&_memH);

    ocBuilder.withAmount(15, "USD");
    ocBuilder.withfrequentFlyerStatus(2);
    OCFees* ocFees = ocBuilder.build();
    _pTrx->getOptions()->isWPwithOutAE() = true;
    _pTrx->getRequest()->setSpecificAgencyText(false);
    TestConfigInitializer::setValue("FF_TIER_ACTIVATION_DATE", DateTime(2015, 1, 1), "PRICING_SVC");
    std::set<int> segmentNumbers;
    setSchemaVersion(1, 1, 0);

    _respFormatter->buildBDI(
        *_pTrx, *_construct, ocFees, BaggageProvisionType(BAGGAGE_CHARGE), segmentNumbers);
    std::string expected = xml2::FrequentFlyerWarning + "=\"P\"";
    CPPUNIT_ASSERT(_construct->getXMLData().find(expected) != std::string::npos);
  }

  void test_buildBDI_A_schema_1_1_0()
  {
    OCFeesBuilder ocBuilder = OCFeesBuilder(&_memH);
    OCFees* ocFees = ocBuilder.build();
    std::set<int> segmentNumbers;

    setSchemaVersion(1, 1, 0);

    std::string expected = "<BDI BPT=\"A\" SFK=\"EF\" SHK=\"IJKMAEF\" BPC=\"30\" "
                           "B20=\"33\" N0D=\"L\" OC1=\"31\" OC2=\"32\"/>";

    _respFormatter->buildBDI(*_pTrx, *_construct, ocFees, BaggageProvisionType(BAGGAGE_ALLOWANCE), segmentNumbers);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_buildBDI_A_schema_1_1_1()
  {
    OCFeesBuilder ocBuilder = OCFeesBuilder(&_memH);
    OCFees* ocFees = ocBuilder.build();
    std::set<int> segmentNumbers;

    setSchemaVersion(1, 1, 1);

    std::string expected = "<BDI BPT=\"A\" SFK=\"EF\" SHK=\"IJKMAEF\" BPC=\"30\" "
                           "B20=\"33\" N0D=\"L\" OC1=\"31\" OC2=\"32\" Q7D=\"7\"/>";

    _respFormatter->buildBDI(*_pTrx, *_construct, ocFees, "A", segmentNumbers);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_buildBDI_CC_schema_1_1_0()
  {
    OCFeesBuilder ocBuilder = OCFeesBuilder(&_memH);
    ocBuilder.withPaxType("ADT");
    ocBuilder.withAmount(15, "USD");
    OCFees* ocFees = ocBuilder.build();
    std::set<int> segmentNumbers;

    setSchemaVersion(1, 1, 0);

    std::string expected = "<BDI BPT=\"CC\" SFK=\"EF\" SHK=\"IJKMCEF\" OC1=\"31\" OC2=\"32\">"
                           "<PFF N41=\"Q\" N42=\"I\" N43=\"N\" N44=\"M\" Q7D=\"7\" "
                           "N45=\"J\" P03=\"K\" P04=\"L\" B70=\"ADT\" C50=\"15.00\" "
                           "C51=\"15\" C5A=\"USD\" C52=\"15.00\" C5B=\"USD\" N21=\"R\"/></BDI>";

    _respFormatter->buildBDI(*_pTrx, *_construct, ocFees, CARRY_ON_CHARGE, segmentNumbers);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_buildBDI_CC_schema_1_1_1()
  {
    OCFeesBuilder ocBuilder = OCFeesBuilder(&_memH);
    ocBuilder.withPaxType("ADT");
    ocBuilder.withAmount(15, "USD");
    OCFees* ocFees = ocBuilder.build();
    std::set<int> segmentNumbers;

    setSchemaVersion(1, 1, 1);

    std::string expected =
        "<BDI BPT=\"CC\" SFK=\"EF\" SHK=\"IJKMCEF\" OC1=\"31\" OC2=\"32\" Q7D=\"7\">"
        "<PFF N41=\"Q\" N42=\"I\" N43=\"N\" N44=\"M\" "
        "N45=\"J\" P03=\"K\" P04=\"L\" B70=\"ADT\" C50=\"15.00\" "
        "C51=\"15\" C5A=\"USD\" C52=\"15.00\" C5B=\"USD\" N21=\"R\"/></BDI>";

    _respFormatter->buildBDI(*_pTrx, *_construct, ocFees, CARRY_ON_CHARGE, segmentNumbers);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_buildPFF_schema_1_1_0()
  {
    OCFeesBuilder ocBuilder = OCFeesBuilder(&_memH);
    ocBuilder.withPaxType("ADT");
    ocBuilder.withAmount(15, "USD");
    ocBuilder.withOrginalAmount(45, "PLN");
    OCFees* ocFees = ocBuilder.build();

    setSchemaVersion(1, 1, 0);

    std::string expected = "<PFF N41=\"Q\" N42=\"I\" N43=\"N\" N44=\"M\" Q7D=\"7\" "
                           "N45=\"J\" P03=\"K\" P04=\"L\" B70=\"ADT\" C50=\"15.00\" "
                           "C51=\"45\" C5A=\"PLN\" C52=\"15.00\" C5B=\"USD\" N21=\"R\"/>";

    _respFormatter->buildPFF(*_pTrx, *_construct, ocFees);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_buildPFF_schema_1_1_1()
  {
    OCFeesBuilder ocBuilder = OCFeesBuilder(&_memH);
    ocBuilder.withPaxType("ADT");
    ocBuilder.withAmount(15, "USD");
    ocBuilder.withOrginalAmount(45, "PLN");
    OCFees* ocFees = ocBuilder.build();

    setSchemaVersion(1, 1, 1);

    std::string expected = "<PFF N41=\"Q\" N42=\"I\" N43=\"N\" N44=\"M\" "
                           "N45=\"J\" P03=\"K\" P04=\"L\" B70=\"ADT\" C50=\"15.00\" "
                           "C51=\"45\" C5A=\"PLN\" C52=\"15.00\" C5B=\"USD\" N21=\"R\"/>";

    _respFormatter->buildPFF(*_pTrx, *_construct, ocFees);
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_buildQ00()
  {
    _respFormatter->buildQ00(*_pTrx, *_construct, 1);
    std::string expected = "<Q00>1</Q00>";
    CPPUNIT_ASSERT_EQUAL(expected, _construct->getXMLData());
  }

  void test_USI_usDot_schema_1_1_0()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;

    setSchemaVersion(1, 1, 0);

    PaxType* paxType = _memH.create<PaxType>();
    paxType->paxType() = "ABC";
    _pTrx->paxType().push_back(paxType);
    FarePath* farePath = FarePathBuilder(&_memH).withSegmentNumber(1, true).build();
    FareCalcConfig* fcConfig = FareCalcConfigBuilder(&_memH).build();
    CalcTotals* calcTotals = CalcTotalsBuilder(&_memH).withFarePath(farePath).build();
    uint16_t paxNumber = 0;
    _respFormatter->preparePassengerInfo(*_pTrx, *fcConfig, *calcTotals, paxNumber, *_construct);

    std::string expected = "USI=\"T\"";
    CPPUNIT_ASSERT(_construct->getXMLData().find(expected) == std::string::npos);
  }

  void test_USI_usDot_schema_1_1_1()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;

    setSchemaVersion(1, 1, 1);

    PaxType* paxType = _memH.create<PaxType>();
    paxType->paxType() = "ABC";
    _pTrx->paxType().push_back(paxType);
    FarePath* farePath = FarePathBuilder(&_memH).withSegmentNumber(1, true).build();
    FareCalcConfig* fcConfig = FareCalcConfigBuilder(&_memH).build();
    CalcTotals* calcTotals = CalcTotalsBuilder(&_memH).withFarePath(farePath).build();
    uint16_t paxNumber = 0;
    _respFormatter->preparePassengerInfo(*_pTrx, *fcConfig, *calcTotals, paxNumber, *_construct);

    std::string expected = "USI=\"T\"";
    CPPUNIT_ASSERT(_construct->getXMLData().find(expected) != std::string::npos);
  }

  void test_USI_NotUsDot_schema_1_1_0()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;

    setSchemaVersion(1, 1, 0);

    PaxType* paxType = _memH.create<PaxType>();
    paxType->paxType() = "ABC";
    _pTrx->paxType().push_back(paxType);
    FarePath* farePath = FarePathBuilder(&_memH).withSegmentNumber(1).build();
    FareCalcConfig* fcConfig = FareCalcConfigBuilder(&_memH).build();
    CalcTotals* calcTotals = CalcTotalsBuilder(&_memH).withFarePath(farePath).build();
    uint16_t paxNumber = 0;
    _respFormatter->preparePassengerInfo(*_pTrx, *fcConfig, *calcTotals, paxNumber, *_construct);

    std::string expected = "USI=\"F\"";
    CPPUNIT_ASSERT(_construct->getXMLData().find(expected) == std::string::npos);
  }

  void test_USI_NotUsDot_schema_1_1_1()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _pRequest->ticketingAgent() = &agent;

    setSchemaVersion(1, 1, 1);

    PaxType* paxType = _memH.create<PaxType>();
    paxType->paxType() = "ABC";
    _pTrx->paxType().push_back(paxType);
    FarePath* farePath = FarePathBuilder(&_memH).withSegmentNumber(1).build();
    FareCalcConfig* fcConfig = FareCalcConfigBuilder(&_memH).build();
    CalcTotals* calcTotals = CalcTotalsBuilder(&_memH).withFarePath(farePath).build();
    uint16_t paxNumber = 0;
    _respFormatter->preparePassengerInfo(*_pTrx, *fcConfig, *calcTotals, paxNumber, *_construct);

    std::string expected = "USI=\"F\"";
    CPPUNIT_ASSERT(_construct->getXMLData().find(expected) != std::string::npos);
  }

  void test_structuredFareRuleResponse_AppropriateNumberOfFareComponentTagsExists()
  {
    Loc loc;
    Agent agent;
    createAgent(loc, agent);
    _structRuleTrx->getRequest()->ticketingAgent() = &agent;

    setSchemaVersion(1, 1, 1);

    PaxType* paxType = _memH.create<PaxType>();
    paxType->paxType() = "ABC";
    _structRuleTrx->paxType().push_back(paxType);

    Itin* itinerary = ItinBuilder(&_memH).addNSegmentAndComponents(4).build();

    PaxTypeFare* ptf = _memH.create<PaxTypeFare>();
    FareUsage* fareUsage = FareUsageBuilder(&_memH)
                               .addTravelSegments(itinerary->travelSeg()[0])
                               .addTicketEndorseItem(ptf, itinerary->travelSeg())
                               .build();
    fareUsage->createStructuredRuleDataIfNonexistent();
    fareUsage->paxTypeFare() = ptf;

    PaxTypeFare* ptf2 = _memH.create<PaxTypeFare>();
    FareUsage* fareUsage2 = FareUsageBuilder(&_memH)
                                .addTravelSegments(itinerary->travelSeg()[1])
                                .addTicketEndorseItem(ptf, itinerary->travelSeg())
                                .build();
    fareUsage->createStructuredRuleDataIfNonexistent();
    fareUsage2->paxTypeFare() = ptf2;

    PaxTypeFare* ptf3 = _memH.create<PaxTypeFare>();
    FareUsage* fareUsage3 = FareUsageBuilder(&_memH)
                                .addTravelSegments(itinerary->travelSeg()[2])
                                .addTicketEndorseItem(ptf, itinerary->travelSeg())
                                .build();
    fareUsage->createStructuredRuleDataIfNonexistent();
    fareUsage3->paxTypeFare() = ptf3;

    PaxTypeFare* ptf4 = _memH.create<PaxTypeFare>();
    FareUsage* fareUsage4 = FareUsageBuilder(&_memH)
                                .addTravelSegments(itinerary->travelSeg()[3])
                                .addTicketEndorseItem(ptf, itinerary->travelSeg())
                                .build();
    fareUsage->createStructuredRuleDataIfNonexistent();
    fareUsage4->paxTypeFare() = ptf4;

    FarePath* farePath = FarePathBuilder(&_memH)
                             .withItin(itinerary)
                             .withFareUsageOnPricingUnit(0, fareUsage)
                             .withFareUsageOnPricingUnit(0, fareUsage2)
                             .withFareUsageOnPricingUnit(0, fareUsage3)
                             .withFareUsageOnPricingUnit(0, fareUsage4)
                             .build();
    itinerary->farePath().push_back(farePath);

    FareCalcConfig* fcConfig = FareCalcConfigBuilder(&_memH).build();
    CalcTotals* calcTotals = CalcTotalsBuilder(&_memH).withFarePath(farePath).build();
    uint16_t paxNumber = 4;

    _respFormatter->preparePassengerInfo(
        *_structRuleTrx, *fcConfig, *calcTotals, paxNumber, *_construct);

    std::string expected = "<FCD Q6D=\"1\"";
    CPPUNIT_ASSERT(_construct->getXMLData().find(expected) != std::string::npos);
    expected = "<FCD Q6D=\"2\"";
    CPPUNIT_ASSERT(_construct->getXMLData().find(expected) != std::string::npos);
    expected = "<FCD Q6D=\"3\"";
    CPPUNIT_ASSERT(_construct->getXMLData().find(expected) != std::string::npos);
    expected = "<FCD Q6D=\"4\"";
    CPPUNIT_ASSERT(_construct->getXMLData().find(expected) != std::string::npos);
    expected = "</FCD>";
    CPPUNIT_ASSERT(_construct->getXMLData().find(expected) == std::string::npos);
  }

  void test_preparePassengerForFullRefund_USI_usDot()
  {
    RefundPricingTrx* trx = TransactionBuilder<RefundPricingTrx>(&_memH)
                                .dummyTransaction()
                                .setUsDot(true)
                                .setUpExcInit()
                                .build();
    trx->setOptions(_memH.create<RexPricingOptions>());
    trx->setExcTrxType(PricingTrx::AF_EXC_TRX);
    _respFormatter->preparePassengerForFullRefund(*trx, *_construct);
    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("USI=\"T\""));
  }

  void test_preparePassengerForFUllRefund_USI_notUsDot()
  {
    RefundPricingTrx* trx = TransactionBuilder<RefundPricingTrx>(&_memH)
                                .dummyTransaction()
                                .setUsDot(false)
                                .setUpExcInit()
                                .build();
    trx->setOptions(_memH.create<RexPricingOptions>());
    trx->setExcTrxType(PricingTrx::AF_EXC_TRX);
    _respFormatter->preparePassengerForFullRefund(*trx, *_construct);
    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("USI=\"F\""));
  }

  void test_addAdditionalPaxInfo_USI_usDot()
  {
    FarePath* farePath = FarePathBuilder(&_memH).withSegmentNumber(1, true).build();
    PricingTrx* trx =
        TransactionBuilder<RexPricingTrx>(&_memH).dummyTransaction().setUsDot(true).build();
    CalcTotals* calcTotals = CalcTotalsBuilder(&_memH).withFarePath(farePath).build();
    _construct->openElement(xml2::PassengerInfo);
    _respFormatter->addAdditionalPaxInfo(*trx, *calcTotals, 0, *_construct);
    _construct->closeElement();

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("USI=\"T\""));
  }

  void test_addAdditionalPaxInfo_USI_notUsDot()
  {
    FarePath* farePath = FarePathBuilder(&_memH).withSegmentNumber(1).build();
    PricingTrx* trx =
        TransactionBuilder<RexPricingTrx>(&_memH).dummyTransaction().setUsDot(false).build();
    CalcTotals* calcTotals = CalcTotalsBuilder(&_memH).withFarePath(farePath).build();
    _construct->openElement(xml2::PassengerInfo);
    _respFormatter->addAdditionalPaxInfo(*trx, *calcTotals, 0, *_construct);
    _construct->closeElement();

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("USI=\"F\""));
  }

  void test_addUsDotItinIndicator_TO_FROM_US()
  {
    setSchemaVersion(1, 1, 1);
    enableBaggageCTA();
    _itin->setBaggageTripType(BaggageTripType::TO_FROM_US);
    _construct->openElement(xml2::PassengerInfo);
    _respFormatter->addUsDotItinIndicator(*_construct, _itin, *_pTrx);
    _construct->closeElement();

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("USI=\"1\""));
  }

  void test_addUsDotItinIndicator_TO_FROM_CA()
  {
    setSchemaVersion(1, 1, 1);
    enableBaggageCTA();
    _itin->setBaggageTripType(BaggageTripType::TO_FROM_CA);
    _construct->openElement(xml2::PassengerInfo);
    _respFormatter->addUsDotItinIndicator(*_construct, _itin, *_pTrx);
    _construct->closeElement();

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("USI=\"2\""));
  }

  void test_addUsDotItinIndicator_WHOLLY_WITHIN_US()
  {
    setSchemaVersion(1, 1, 1);
    enableBaggageCTA();
    _itin->setBaggageTripType(BaggageTripType::WHOLLY_WITHIN_US);
    _construct->openElement(xml2::PassengerInfo);
    _respFormatter->addUsDotItinIndicator(*_construct, _itin, *_pTrx);
    _construct->closeElement();

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("USI=\"3\""));
  }

  void test_addUsDotItinIndicator_WHOLLY_WITHIN_CA()
  {
    setSchemaVersion(1, 1, 1);
    enableBaggageCTA();
    _itin->setBaggageTripType(BaggageTripType::WHOLLY_WITHIN_CA);
    _construct->openElement(xml2::PassengerInfo);
    _respFormatter->addUsDotItinIndicator(*_construct, _itin, *_pTrx);
    _construct->closeElement();

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("USI=\"4\""));
  }

  void test_addUsDotItinIndicator_BETWEEN_US_CA()
  {
    setSchemaVersion(1, 1, 1);
    enableBaggageCTA();
    _itin->setBaggageTripType(BaggageTripType::BETWEEN_US_CA);
    _construct->openElement(xml2::PassengerInfo);
    _respFormatter->addUsDotItinIndicator(*_construct, _itin, *_pTrx);
    _construct->closeElement();

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("USI=\"5\""));
  }

  void test_addUsDotItinIndicator_OTHER()
  {
    setSchemaVersion(1, 1, 1);
    enableBaggageCTA();
    _itin->setBaggageTripType(BaggageTripType::OTHER);
    _construct->openElement(xml2::PassengerInfo);
    _respFormatter->addUsDotItinIndicator(*_construct, _itin, *_pTrx);
    _construct->closeElement();

    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find("USI=\"6\""));
  }

  void test_validSchamaVersion_AR_EXC_TRX()
  {
    RexPricingTrx* trx = TransactionBuilder<RexPricingTrx>(&_memH).dummyTransaction().build();
    CPPUNIT_ASSERT(_respFormatter->validSchemaVersion(*trx));
  }

  void test_validSchamaVersion_AF_EXC_TRX()
  {
    RefundPricingTrx* trx = TransactionBuilder<RefundPricingTrx>(&_memH).dummyTransaction().build();
    CPPUNIT_ASSERT(_respFormatter->validSchemaVersion(*trx));
  }

  void test_validSchamaVersion_PORT_EXC_TRX()
  {
    ExchangePricingTrx* trx = TransactionBuilder<ExchangePricingTrx>(&_memH)
                                  .dummyTransaction()
                                  .setExcTrxType(PricingTrx::PORT_EXC_TRX)
                                  .build();
    CPPUNIT_ASSERT(_respFormatter->validSchemaVersion(*trx));
  }

  void test_validSchamaVersion_ME_DIAG_TRX()
  {
    ExchangePricingTrx* trx = TransactionBuilder<ExchangePricingTrx>(&_memH)
                                  .dummyTransaction()
                                  .setExcTrxType(PricingTrx::ME_DIAG_TRX)
                                  .build();
    CPPUNIT_ASSERT(_respFormatter->validSchemaVersion(*trx));
  }

  void test_validSchamaVersion_EXC1_WITHIN_ME()
  {
    ExchangePricingTrx* trx = TransactionBuilder<ExchangePricingTrx>(&_memH)
                                  .dummyTransaction()
                                  .setExcTrxType(PricingTrx::EXC1_WITHIN_ME)
                                  .build();
    CPPUNIT_ASSERT(_respFormatter->validSchemaVersion(*trx));
  }

  void test_validSchamaVersion_EXC2_WITHIN_ME()
  {
    ExchangePricingTrx* trx = TransactionBuilder<ExchangePricingTrx>(&_memH)
                                  .dummyTransaction()
                                  .setExcTrxType(PricingTrx::EXC2_WITHIN_ME)
                                  .build();
    CPPUNIT_ASSERT(_respFormatter->validSchemaVersion(*trx));
  }

  void test_validSchamaVersion_NEW_WITHIN_ME()
  {
    ExchangePricingTrx* trx = TransactionBuilder<ExchangePricingTrx>(&_memH)
                                  .dummyTransaction()
                                  .setExcTrxType(PricingTrx::NEW_WITHIN_ME)
                                  .build();
    CPPUNIT_ASSERT(_respFormatter->validSchemaVersion(*trx));
  }

  void test_validSchemaVersion_notExchangeTrx()
  {
    PricingTrx* trx = TransactionBuilder<PricingTrx>(&_memH)
                          .dummyTransaction()
                          .setExcTrxType(PricingTrx::NOT_EXC_TRX)
                          .build();
    CPPUNIT_ASSERT(!_respFormatter->validSchemaVersion(*trx));
  }

  void test_isOldPricingTrx_PricingTrx()
  {
    PricingTrx* pricingTrx = _memH.create<PricingTrx>();
    CPPUNIT_ASSERT(!_respFormatter->isOldPricingTrx(*pricingTrx));
  }

  MultiExchangeTrx* createMultiExchangeTrx()
  {
    MultiExchangeTrx* multiExchangeTrx = _memH.create<MultiExchangeTrx>();
    multiExchangeTrx->newPricingTrx() = _memH.create<PricingTrx>();
    multiExchangeTrx->newPricingTrx()->setParentTrx(multiExchangeTrx);
    multiExchangeTrx->excPricingTrx1() = _memH.create<PricingTrx>();
    multiExchangeTrx->excPricingTrx1()->setParentTrx(multiExchangeTrx);
    multiExchangeTrx->excPricingTrx2() = _memH.create<PricingTrx>();
    multiExchangeTrx->excPricingTrx2()->setParentTrx(multiExchangeTrx);
    return multiExchangeTrx;
  }

  void test_isOldPricingTrx_MultiExchangeTrx_newPricingTrx()
  {
    CPPUNIT_ASSERT(!_respFormatter->isOldPricingTrx(*createMultiExchangeTrx()->newPricingTrx()));
  }

  void test_isOldPricingTrx_MultiExchangeTrx_excPricingTrx1()
  {
    CPPUNIT_ASSERT(_respFormatter->isOldPricingTrx(*createMultiExchangeTrx()->excPricingTrx1()));
  }

  void test_isOldPricingTrx_MultiExchangeTrx_excPricingTrx2()
  {
    CPPUNIT_ASSERT(_respFormatter->isOldPricingTrx(*createMultiExchangeTrx()->excPricingTrx2()));
  }

  class SideTripObjects
  {
  public:
    SideTripObjects(TestMemHandle* memHandle, Itin* itin) : pItin(itin), _myMemHandle(memHandle)
    {
      fm0 = _myMemHandle->create<FareMarket>();
      fm1 = _myMemHandle->create<FareMarket>();
      pu0 = _myMemHandle->create<PricingUnit>();
      pu1 = _myMemHandle->create<PricingUnit>();
      fu0 = _myMemHandle->create<FareUsage>();
      fu1 = _myMemHandle->create<FareUsage>();
      ptf = _myMemHandle->create<PaxTypeFare>();
      farePath = FarePathBuilder(_myMemHandle).withSegmentNumber(0).build();
      pItin->travelSeg().clear();
      vSideTrip0 = _myMemHandle->create<std::vector<TravelSeg*> >();
      vSideTrip1 = _myMemHandle->create<std::vector<TravelSeg*> >();
    }

    AirSeg* createAirSeg(unsigned i)
    {
      AirSeg* airSeg = _myMemHandle->create<AirSeg>();
      airSeg->pnrSegment() = i;
      airSeg->segmentType() = Air;
      airSeg->setOperatingCarrierCode("CM");
      airSeg->setMarketingCarrierCode("AA");
      pItin->travelSeg().push_back(airSeg);
      return airSeg;
    }

    SurfaceSeg* createSurfaceSeg(unsigned i)
    {
      SurfaceSeg* surfaceSeg = _myMemHandle->create<SurfaceSeg>();
      surfaceSeg->pnrSegment() = i;
      surfaceSeg->segmentType() = tse::Arunk;
      pItin->travelSeg().push_back(surfaceSeg);
      return surfaceSeg;
    }

  public:
    FareMarket* fm0;
    FareMarket* fm1;
    PricingUnit* pu0;
    PricingUnit* pu1;
    FareUsage* fu0;
    FareUsage* fu1;
    PaxTypeFare* ptf;
    FarePath* farePath;
    Itin* pItin;
    std::vector<TravelSeg*>* vSideTrip0;
    std::vector<TravelSeg*>* vSideTrip1;

  private:
    TestMemHandle* _myMemHandle;
  };

  void test_sideTrip_arunk_seg()
  {
    TestMemHandle _myMemH;
    SideTripObjects sto(&_myMemH, _itin);

    AirSeg* const a1 = sto.createAirSeg(1);
    SurfaceSeg* const s2 = sto.createSurfaceSeg(2);
    AirSeg* const a3 = sto.createAirSeg(3);
    AirSeg* const a4 = sto.createAirSeg(4);

    sto.fu0->paxTypeFare() = sto.ptf;
    sto.fu0->paxTypeFare()->fareMarket() = sto.fm0;
    sto.fu0->travelSeg().push_back(a1);
    sto.fu0->travelSeg().push_back(a4);
    sto.pu0->travelSeg().push_back(a1);
    sto.pu0->travelSeg().push_back(a4);
    sto.pu0->fareUsage().push_back(sto.fu0);
    sto.pu0->isSideTripPU() = false;
    sto.farePath->pricingUnit().push_back(sto.pu0);

    // Side trip.
    sto.fu1->paxTypeFare() = sto.ptf;
    sto.fu1->paxTypeFare()->fareMarket() = sto.fm1;
    sto.fu1->travelSeg().push_back(s2);
    sto.fu1->travelSeg().push_back(a3);
    sto.pu1->travelSeg().push_back(s2);
    sto.pu1->travelSeg().push_back(a3);
    sto.pu1->fareUsage().push_back(sto.fu1);
    sto.pu1->isSideTripPU() = true;
    sto.fm1->sideTripTravelSeg().push_back(sto.pu1->travelSeg());
    sto.farePath->pricingUnit().push_back(sto.pu1);

    const FarePath fp = *sto.farePath;
    const PricingUnit pu0 = *sto.pu0;
    const PricingUnit pu1 = *sto.pu1;
    const FareUsage fu0 = *sto.fu0;
    const FareUsage fu1 = *sto.fu1;
    const Itin itin = *_itin;

    uint16_t segOrder;
    XMLConstruct* construct = _myMemH.insert(new XMLConstruct);
    construct->openElement(xml2::MessageInformation);
    construct->addAttributeChar(xml2::MessageType, Message::TYPE_GENERAL);
    std::string* xml = const_cast<std::string*>(&construct->getXMLData());

    // A segment prior to starting side trip segment.
    segOrder = 1;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu0, &itin, *a1, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *a1, *construct, fu0);
    _respFormatter->prepareArunk(*_pTrx, fp, fu0, pu0, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08") == std::string::npos);
    xml->clear();

    // Start segment of side trip.
    segOrder = 2;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu1, &itin, *s2, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *s2, *construct,fu1);
    _respFormatter->prepareArunk(*_pTrx, fp, fu1, pu1, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08") == std::string::npos);
    xml->clear();

    // End segment of side trip.
    segOrder = 3;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu1, &itin, *a3, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *a3, *construct, fu1);
    _respFormatter->prepareArunk(*_pTrx, fp, fu1, pu1, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10") == std::string::npos);
    xml->clear();

    // Segment after end of side trip segment.
    segOrder = 4;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu0, &itin, *a4, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *a4, *construct, fu0);
    _respFormatter->prepareArunk(*_pTrx, fp, fu0, pu0, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08") == std::string::npos);
    xml->clear();

    construct->closeElement();
  }

  void test_sideTrip_seg_arunk()
  {
    TestMemHandle _myMemH;
    SideTripObjects sto(&_myMemH, _itin);

    AirSeg* const a1 = sto.createAirSeg(1);
    AirSeg* const a2 = sto.createAirSeg(2);
    SurfaceSeg* const s3 = sto.createSurfaceSeg(3);
    AirSeg* const a4 = sto.createAirSeg(4);

    sto.fu0->paxTypeFare() = sto.ptf;
    sto.fu0->paxTypeFare()->fareMarket() = sto.fm0;
    sto.fu0->travelSeg().push_back(a1);
    sto.fu0->travelSeg().push_back(a4);
    sto.pu0->travelSeg().push_back(a1);
    sto.pu0->travelSeg().push_back(a4);
    sto.pu0->fareUsage().push_back(sto.fu0);
    sto.pu0->isSideTripPU() = false;
    sto.farePath->pricingUnit().push_back(sto.pu0);

    // Side trip.
    sto.fu1->paxTypeFare() = sto.ptf;
    sto.fu1->paxTypeFare()->fareMarket() = sto.fm1;
    sto.fu1->travelSeg().push_back(a2);
    sto.fu1->travelSeg().push_back(s3);
    sto.pu1->travelSeg().push_back(a2);
    sto.pu1->travelSeg().push_back(s3);
    sto.pu1->fareUsage().push_back(sto.fu1);
    sto.pu1->isSideTripPU() = true;
    sto.fm1->sideTripTravelSeg().push_back(sto.pu1->travelSeg());
    sto.farePath->pricingUnit().push_back(sto.pu1);

    const FarePath fp = *sto.farePath;
    const PricingUnit pu0 = *sto.pu0;
    const PricingUnit pu1 = *sto.pu1;
    const FareUsage fu0 = *sto.fu0;
    const FareUsage fu1 = *sto.fu1;
    const Itin itin = *_itin;

    uint16_t segOrder;
    XMLConstruct* construct = _myMemH.insert(new XMLConstruct);
    construct->openElement(xml2::MessageInformation);
    construct->addAttributeChar(xml2::MessageType, Message::TYPE_GENERAL);
    std::string* xml = const_cast<std::string*>(&construct->getXMLData());

    // A segment prior to starting side trip segment.
    segOrder = 1;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu0, &itin, *a1, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *a1, *construct, fu0);
    _respFormatter->prepareArunk(*_pTrx, fp, fu0, pu0, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10") == std::string::npos);
    xml->clear();

    // Start segment of side trip.
    segOrder = 2;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu1, &itin, *a2, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *a2, *construct, fu1);
    _respFormatter->prepareArunk(*_pTrx, fp, fu1, pu1, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10") == std::string::npos);
    xml->clear();

    // End segment of side trip.
    segOrder = 3;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu1, &itin, *s3, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *s3, *construct, fu1);
    _respFormatter->prepareArunk(*_pTrx, fp, fu1, pu1, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09=\"F\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10=\"T\"") != std::string::npos);
    xml->clear();

    // Segment after end of side trip segment.
    segOrder = 4;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu0, &itin, *a4, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *a4, *construct, fu0);
    _respFormatter->prepareArunk(*_pTrx, fp, fu0, pu0, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10") == std::string::npos);
    xml->clear();

    construct->closeElement();
  }

  void test_sideTrip_seg_seg()
  {
    TestMemHandle _myMemH;
    SideTripObjects sto(&_myMemH, _itin);

    AirSeg* const a1 = sto.createAirSeg(1);
    AirSeg* const a2 = sto.createAirSeg(2);
    AirSeg* const a3 = sto.createAirSeg(3);
    AirSeg* const a4 = sto.createAirSeg(4);

    sto.fu0->paxTypeFare() = sto.ptf;
    sto.fu0->paxTypeFare()->fareMarket() = sto.fm0;
    sto.fu0->hasSideTrip() = false;
    sto.pu0->travelSeg().push_back(a1);
    sto.pu0->travelSeg().push_back(a4);
    sto.pu0->fareUsage().push_back(sto.fu0);
    sto.fm0->sideTripTravelSeg().clear();
    sto.farePath->pricingUnit().push_back(sto.pu0);

    // The side trip
    sto.fu1->paxTypeFare() = sto.ptf;
    sto.fu1->paxTypeFare()->fareMarket() = sto.fm1;
    sto.fu1->hasSideTrip() = true;
    sto.fu1->travelSeg().push_back(a2);
    sto.fu1->travelSeg().push_back(a3);

    sto.pu1->travelSeg().push_back(a2);
    sto.pu1->travelSeg().push_back(a3);
    sto.pu1->fareUsage().push_back(sto.fu1);
    sto.pu1->isSideTripPU() = true;

    sto.fm1->sideTripTravelSeg().push_back(sto.pu1->travelSeg());
    sto.farePath->pricingUnit().push_back(sto.pu1);

    const FarePath fp = *sto.farePath;
    const PricingUnit pu0 = *sto.pu0;
    const PricingUnit pu1 = *sto.pu1;
    const FareUsage fu0 = *sto.fu0;
    const FareUsage fu1 = *sto.fu1;
    const Itin itin = *_itin;

    uint16_t segOrder;
    XMLConstruct* construct = _myMemH.insert(new XMLConstruct);
    construct->openElement(xml2::MessageInformation);
    construct->addAttributeChar(xml2::MessageType, Message::TYPE_GENERAL);
    std::string* xml = const_cast<std::string*>(&construct->getXMLData());

    // Segment prior to starting side trip segment.
    segOrder = 1;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu0, &itin, *a1, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *a1, *construct, fu0);
    _respFormatter->prepareArunk(*_pTrx, fp, fu0, pu0, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10") == std::string::npos);
    xml->clear();

    // Start segment of side trip.
    segOrder = 2;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu1, &itin, *a2, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *a2, *construct, fu1);
    _respFormatter->prepareArunk(*_pTrx, fp, fu1, pu1, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10") == std::string::npos);
    xml->clear();

    // End segment of side trip.
    segOrder = 3;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu1, &itin, *a3, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *a3, *construct, fu1);
    _respFormatter->prepareArunk(*_pTrx, fp, fu1, pu1, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10") == std::string::npos);
    xml->clear();

    // Segment after end of side trip segment.
    segOrder = 4;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu0, &itin, *a4, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *a4, *construct, fu0);
    _respFormatter->prepareArunk(*_pTrx, fp, fu0, pu0, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10") == std::string::npos);
    xml->clear();

    construct->closeElement();
  }

  void test_sideTrip_seg_arunk_seg()
  {
    TestMemHandle _myMemH;
    SideTripObjects sto(&_myMemH, _itin);

    AirSeg* const a1 = sto.createAirSeg(1);
    AirSeg* const a2 = sto.createAirSeg(2);
    SurfaceSeg* const s3 = sto.createSurfaceSeg(3);
    AirSeg* const a4 = sto.createAirSeg(4);
    AirSeg* const a5 = sto.createAirSeg(5);

    // Side trip boundary.
    sto.fu0->paxTypeFare() = sto.ptf;
    sto.fu0->paxTypeFare()->fareMarket() = sto.fm0;
    sto.fu0->hasSideTrip() = false;
    sto.pu0->travelSeg().push_back(a1);
    sto.pu0->travelSeg().push_back(a5);
    sto.pu0->fareUsage().push_back(sto.fu0);
    sto.fm0->sideTripTravelSeg().clear();
    sto.farePath->pricingUnit().push_back(sto.pu0);

    // The side trip
    sto.fu1->paxTypeFare() = sto.ptf;
    sto.fu1->paxTypeFare()->fareMarket() = sto.fm1;
    sto.fu1->hasSideTrip() = true;
    sto.fu1->travelSeg().push_back(a2);
    sto.fu1->travelSeg().push_back(s3);
    sto.fu1->travelSeg().push_back(a4);

    sto.pu1->travelSeg().push_back(a2);
    sto.pu1->travelSeg().push_back(s3);
    sto.pu1->travelSeg().push_back(a4);
    sto.pu1->fareUsage().push_back(sto.fu1);
    sto.pu1->isSideTripPU() = true;

    sto.fm1->sideTripTravelSeg().push_back(sto.pu1->travelSeg());
    sto.farePath->pricingUnit().push_back(sto.pu1);

    const FarePath fp = *sto.farePath;
    const PricingUnit pu0 = *sto.pu0;
    const PricingUnit pu1 = *sto.pu1;
    const FareUsage fu0 = *sto.fu0;
    const FareUsage fu1 = *sto.fu1;
    const Itin itin = *_itin;

    uint16_t segOrder;
    XMLConstruct* construct = _myMemH.insert(new XMLConstruct);
    construct->openElement(xml2::MessageInformation);
    construct->addAttributeChar(xml2::MessageType, Message::TYPE_GENERAL);
    std::string* xml = const_cast<std::string*>(&construct->getXMLData());

    // Segment prior to starting side trip segment.
    segOrder = 1;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu0, &itin, *a1, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *a1, *construct, fu0 );
    _respFormatter->prepareArunk(*_pTrx, fp, fu0, pu0, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10") == std::string::npos);
    xml->clear();

    // Start segment of side trip.
    segOrder = 2;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu1, &itin, *a2, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *a2, *construct, fu1);
    _respFormatter->prepareArunk(*_pTrx, fp, fu1, pu1, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10") == std::string::npos);
    xml->clear();

    // Interior segment of side trip.
    segOrder = 3;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu1, &itin, *s3, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *s3, *construct,fu1);
    _respFormatter->prepareArunk(*_pTrx, fp, fu1, pu1, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09=\"F\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10=\"T\"") != std::string::npos);
    xml->clear();

    // End segment of side trip.
    segOrder = 4;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu1, &itin, *a4, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *a4, *construct, fu1);
    _respFormatter->prepareArunk(*_pTrx, fp, fu1, pu1, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10") == std::string::npos);
    xml->clear();

    // Segment after end of side trip segment.
    segOrder = 5;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu0, &itin, *a5, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *a5, *construct, fu0);
    _respFormatter->prepareArunk(*_pTrx, fp, fu0, pu0, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10") == std::string::npos);
    xml->clear();

    construct->closeElement();
  }

  void test_sideTrip_arunk_seg_arunk()
  {
    TestMemHandle _myMemH;
    SideTripObjects sto(&_myMemH, _itin);

    AirSeg* const a1 = sto.createAirSeg(1);
    SurfaceSeg* const s2 = sto.createSurfaceSeg(2);
    AirSeg* const a3 = sto.createAirSeg(3);
    SurfaceSeg* const s4 = sto.createSurfaceSeg(4);
    AirSeg* const a5 = sto.createAirSeg(5);

    // Side trip boundary.
    sto.fu0->paxTypeFare() = sto.ptf;
    sto.fu0->paxTypeFare()->fareMarket() = sto.fm0;
    sto.fu0->hasSideTrip() = false;
    sto.pu0->travelSeg().push_back(a1);
    sto.pu0->travelSeg().push_back(a5);
    sto.pu0->fareUsage().push_back(sto.fu0);
    sto.fm0->sideTripTravelSeg().clear();
    sto.farePath->pricingUnit().push_back(sto.pu0);

    // The side trip
    sto.fu1->paxTypeFare() = sto.ptf;
    sto.fu1->paxTypeFare()->fareMarket() = sto.fm1;
    sto.fu1->hasSideTrip() = true;
    sto.fu1->travelSeg().push_back(s2);
    sto.fu1->travelSeg().push_back(a3);
    sto.fu1->travelSeg().push_back(s4);

    sto.pu1->travelSeg().push_back(s2);
    sto.pu1->travelSeg().push_back(a3);
    sto.pu1->travelSeg().push_back(s4);
    sto.pu1->fareUsage().push_back(sto.fu1);
    sto.pu1->isSideTripPU() = true;

    sto.fm1->sideTripTravelSeg().push_back(sto.pu1->travelSeg());
    sto.farePath->pricingUnit().push_back(sto.pu1);

    const FarePath fp = *sto.farePath;
    const PricingUnit pu0 = *sto.pu0;
    const PricingUnit pu1 = *sto.pu1;
    const FareUsage fu0 = *sto.fu0;
    const FareUsage fu1 = *sto.fu1;
    const Itin itin = *_itin;

    uint16_t segOrder;
    XMLConstruct* construct = _myMemH.insert(new XMLConstruct);
    construct->openElement(xml2::MessageInformation);
    construct->addAttributeChar(xml2::MessageType, Message::TYPE_GENERAL);
    std::string* xml = const_cast<std::string*>(&construct->getXMLData());

    // Segment prior to starting side trip segment.
    segOrder = 1;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu0, &itin, *a1, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *a1, *construct, fu0);
    _respFormatter->prepareArunk(*_pTrx, fp, fu0, pu0, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10") == std::string::npos);
    xml->clear();

    // Start segment of side trip.
    segOrder = 2;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu1, &itin, *s2, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *s2, *construct, fu1);
    _respFormatter->prepareArunk(*_pTrx, fp, fu1, pu1, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09=\"F\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10=\"T\"") != std::string::npos);
    xml->clear();

    // Interior segment of side trip.
    segOrder = 3;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu1, &itin, *a3, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *a3, *construct, fu1);
    _respFormatter->prepareArunk(*_pTrx, fp, fu1, pu1, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10") == std::string::npos);
    xml->clear();

    // End segment of side trip.
    segOrder = 4;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu1, &itin, *s4, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *s4, *construct, fu1);
    _respFormatter->prepareArunk(*_pTrx, fp, fu1, pu1, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08=\"T\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09=\"F\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10=\"T\"") != std::string::npos);
    xml->clear();

    // Segment after end of side trip segment.
    segOrder = 5;

    _respFormatter->prepareSegmentSideTrips(*_pTrx, fp, pu0, &itin, *a5, segOrder, *construct);
    _respFormatter->prepareScheduleInfo(*_pTrx, *a5, *construct, fu0);
    _respFormatter->prepareArunk(*_pTrx, fp, fu0, pu0, &itin, segOrder, *construct);
    CPPUNIT_ASSERT(xml->find("B00=\"AA\" B01=\"CM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("P2N") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S07") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S08") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S09") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("S10") == std::string::npos);
    xml->clear();

    construct->closeElement();
  }

  void test_checkForStructuredData_XML_RESPONSE_OFF_US_DOT()
  {
    bool isBaggageXmlResponse = false;
    bool isBaggageExchange = true;
    bool isBaggageGlobalDisclosure = true;

    PricingTrx* pricingTrx =
        TransactionBuilder<PricingTrx>(&_memH).setUsDot(true).setSchemaVersion(1, 1, 1).build();

    CPPUNIT_ASSERT(!_respFormatter->checkForStructuredData(
        *pricingTrx, isBaggageXmlResponse, isBaggageExchange, isBaggageGlobalDisclosure));
  }

  void test_checkForStructuredData_XML_RESPONSE_OFF_NON_US_DOT()
  {
    bool isBaggageXmlResponse = false;
    bool isBaggageExchange = true;
    bool isBaggageGlobalDisclosure = true;

    PricingTrx* pricingTrx =
        TransactionBuilder<PricingTrx>(&_memH).setUsDot(false).setSchemaVersion(1, 1, 1).build();

    CPPUNIT_ASSERT(!_respFormatter->checkForStructuredData(
        *pricingTrx, isBaggageXmlResponse, isBaggageExchange, isBaggageGlobalDisclosure));
  }

  void test_checkForStructuredData_XML_RESPONSE_ITIN_EMPTY()
  {
    bool isBaggageXmlResponse = false;
    bool isBaggageExchange = true;
    bool isBaggageGlobalDisclosure = true;

    PricingTrx* pricingTrx = TransactionBuilder<PricingTrx>(&_memH).build();
    CPPUNIT_ASSERT(!_respFormatter->checkForStructuredData(
        *pricingTrx, isBaggageXmlResponse, isBaggageExchange, isBaggageGlobalDisclosure));
  }

  void test_checkForStructuredData_XML_RESPONSE_US_DOT_PricingTrx_1_0_1()
  {
    bool isBaggageXmlResponse = true;
    bool isBaggageExchange = true;
    bool isBaggageGlobalDisclosure = true;

    PricingTrx* pricingTrx =
        TransactionBuilder<PricingTrx>(&_memH).setUsDot(true).setSchemaVersion(1, 0, 1).build();
    CPPUNIT_ASSERT(!_respFormatter->checkForStructuredData(
        *pricingTrx, isBaggageXmlResponse, isBaggageExchange, isBaggageGlobalDisclosure));
  }

  void test_checkForStructuredData_XML_RESPONSE_US_DOT_PricingTrx_1_1_0()
  {
    bool isBaggageXmlResponse = true;
    bool isBaggageExchange = true;
    bool isBaggageGlobalDisclosure = true;

    PricingTrx* pricingTrx =
        TransactionBuilder<PricingTrx>(&_memH).setUsDot(true).setSchemaVersion(1, 1, 0).build();
    CPPUNIT_ASSERT(_respFormatter->checkForStructuredData(
        *pricingTrx, isBaggageXmlResponse, isBaggageExchange, isBaggageGlobalDisclosure));
  }

  void test_checkForStructuredData_XML_RESPONSE_US_DOT_PricingTrx_1_1_1()
  {
    bool isBaggageXmlResponse = true;
    bool isBaggageExchange = true;
    bool isBaggageGlobalDisclosure = true;

    PricingTrx* pricingTrx =
        TransactionBuilder<PricingTrx>(&_memH).setUsDot(true).setSchemaVersion(1, 1, 1).build();
    CPPUNIT_ASSERT(_respFormatter->checkForStructuredData(
        *pricingTrx, isBaggageXmlResponse, isBaggageExchange, isBaggageGlobalDisclosure));
  }

  void test_checkForStructuredData_XML_RESPONSE_US_DOT_ExchangeTrx()
  {
    bool isBaggageXmlResponse = true;
    bool isBaggageExchange = true;
    bool isBaggageGlobalDisclosure = true;

    ExchangePricingTrx* exchangePricingTrx = TransactionBuilder<ExchangePricingTrx>(&_memH)
                                                 .dummyTransaction()
                                                 .setUsDot(true)
                                                 .setExcTrxType(PricingTrx::EXC2_WITHIN_ME)
                                                 .build();
    CPPUNIT_ASSERT(_respFormatter->checkForStructuredData(
        *exchangePricingTrx, isBaggageXmlResponse, isBaggageExchange, isBaggageGlobalDisclosure));
  }

  void test_checkForStructuredData_XML_RESPONSE_NON_US_DOT_PricingTrx_1_0_1()
  {
    bool isBaggageXmlResponse = true;
    bool isBaggageExchange = true;
    bool isBaggageGlobalDisclosure = false;

    PricingTrx* pricingTrx =
        TransactionBuilder<PricingTrx>(&_memH).setUsDot(false).setSchemaVersion(1, 0, 1).build();
    CPPUNIT_ASSERT(!_respFormatter->checkForStructuredData(
        *pricingTrx, isBaggageXmlResponse, isBaggageExchange, isBaggageGlobalDisclosure));
  }

  void test_checkForStructuredData_XML_RESPONSE_NON_US_DOT_PricingTrx_1_1_0()
  {
    bool isBaggageXmlResponse = true;
    bool isBaggageExchange = true;
    bool isBaggageGlobalDisclosure = false;

    PricingTrx* pricingTrx =
        TransactionBuilder<PricingTrx>(&_memH).setUsDot(false).setSchemaVersion(1, 1, 0).build();
    CPPUNIT_ASSERT(!_respFormatter->checkForStructuredData(
        *pricingTrx, isBaggageXmlResponse, isBaggageExchange, isBaggageGlobalDisclosure));
  }

  void test_checkForStructuredData_XML_RESPONSE_NON_US_DOT_PricingTrx_1_1_1()
  {
    bool isBaggageXmlResponse = true;
    bool isBaggageExchange = true;
    bool isBaggageGlobalDisclosure = true;

    PricingTrx* pricingTrx =
        TransactionBuilder<PricingTrx>(&_memH).setUsDot(false).setSchemaVersion(1, 1, 1).build();
    CPPUNIT_ASSERT(_respFormatter->checkForStructuredData(
        *pricingTrx, isBaggageXmlResponse, isBaggageExchange, isBaggageGlobalDisclosure));
  }

  void test_checkForStructuredData_XML_RESPONSE_NON_US_DOT_EchangeTrx()
  {
    bool isBaggageXmlResponse = true;
    bool isBaggageExchange = true;
    bool isBaggageGlobalDisclosure = true;

    ExchangePricingTrx* exchangePricingTrx = TransactionBuilder<ExchangePricingTrx>(&_memH)
                                                 .dummyTransaction()
                                                 .setUsDot(false)
                                                 .setExcTrxType(PricingTrx::EXC2_WITHIN_ME)
                                                 .build();
    CPPUNIT_ASSERT(_respFormatter->checkForStructuredData(
        *exchangePricingTrx, isBaggageXmlResponse, isBaggageExchange, isBaggageGlobalDisclosure));
  }

  CalcTotals* buildCalcTotals(PaxTypeCode paxType,
                              std::string baggageResponse = "",
                              std::string baggageEmbargoesResponse = "")
  {
    FarePath* farePaht = FarePathBuilder(&_memH)
                             .withBaggageResponse(baggageResponse)
                             .withBaggageEmbargoesResponse(baggageEmbargoesResponse)
                             .withPaxTypeCode(paxType)
                             .build();
    CalcTotals* calcTotals = CalcTotalsBuilder(&_memH)
                                 .withFarePath(farePaht)
                                 .withTruePaxType(paxType)
                                 .withRequestedPaxType("INF")
                                 .build();
    return calcTotals;
  }

  const FareCalcCollector* greenScreeSetup(bool embargoesFlag = true)
  {
    std::string baggageMIL =
        "CARRY ON ALLOWANCE\n MIADCA-02P\n01/UP TO 45 LINEAR INCHES/115 LINEAR CENTIMETERS\n<ADD>";
    std::string baggageADT = "CARRY ON ALLOWANCE\nMIADCA-02P\n";
    std::string embargoes;
    if (embargoesFlag)
      embargoes = " \nEMBARGOES-APPLY TO EACH PASSENGER\n FRANRT-LH\n";

    CalcTotals* calcTotalsADT = buildCalcTotals("ADT", baggageADT, embargoes);
    CalcTotals* calcTotalsMIL = buildCalcTotals("MIL", baggageMIL, embargoes);

    FareCalcCollector* fareCalcCollector = _memH.create<FareCalcCollector>();
    fareCalcCollector->baggageTagMap().insert(std::make_pair("BAGGAGETEXT001", calcTotalsADT));
    fareCalcCollector->baggageTagMap().insert(std::make_pair("BAGGAGETEXT002", calcTotalsMIL));

    _pTrx->fareCalcConfig() = FareCalcConfigBuilder(&_memH).build();
    ;
    _pTrx->fareCalcConfig()->truePsgrTypeInd() = FareCalcConsts::FC_YES;

    return fareCalcCollector;
  }

  void test_formatGreenScreenResponse_usdot()
  {
    const FareCalcCollector* fareCalcCollector = greenScreeSetup();
    _pTrx->itin().front()->setBaggageTripType(BaggageTripType::TO_FROM_US);
    setSchemaVersion(1, 1, 1);

    std::string endos = "\nENDOS*VLDFLTDTE/BLKOUDTE/CONDAPY/NOPARTIALRFND/RBKGSGD30/XSGBY30JUN14"
                        "VLDFLTDTE/BLKOUDTE/CONDAPY/NOPARTIALRFND/RBKGSGD30/XSGBY30JUN14"
                        "VLDFLTDTE/BLKOUDTE/CONDAPY/NOPARTIALRFND/RBKGSGD30/XSGBY30JUN14"
                        "VLDFLTDTE/BLKOUDTE/CONDAPY/NOPARTIALRFND/RBKGSGD30/XSGBY30JUN14";

    std::string response = "BASE FARE  \nBAGGAGETEXT002\n 1-  PLN41119.00    USD12870.00     "
                           "356.90XT     USD13226.90ADT\nBAGGAGETEXT001" +
                           endos;

    std::string expected =
        "<MSG N06=\"X\" Q0K=\"3\" S18=\"BASE FARE  \"/>"
        "<MSG N06=\"X\" Q0K=\"4\" S18=\"CARRY ON ALLOWANCE\"/>"
        "<MSG N06=\"X\" Q0K=\"5\" S18=\"MIADCA-02P\"/>"
        "<MSG N06=\"X\" Q0K=\"6\" S18=\"01/UP TO 45 LINEAR INCHES/115 LINEAR CENTIMETERS\"/>"
        "<MSG N06=\"X\" Q0K=\"7\" S18=\"ADDITIONAL ALLOWANCES AND/OR DISCOUNTS MAY APPLY\"/>"
        "<MSG N06=\"X\" Q0K=\"8\" S18=\" 1-  PLN41119.00    USD12870.00     356.90XT     "
        "USD13226.90ADT\"/>"
        "<MSG N06=\"X\" Q0K=\"9\" S18=\"CARRY ON ALLOWANCE\"/>"
        "<MSG N06=\"X\" Q0K=\"10\" S18=\"MIADCA-02P\"/>"
        "<MSG N06=\"X\" Q0K=\"11\" S18=\" \"/>"
        "<MSG N06=\"X\" Q0K=\"12\" S18=\"EMBARGOES-APPLY TO EACH PASSENGER\"/>"
        "<MSG N06=\"X\" Q0K=\"13\" S18=\"FRANRT-LH\"/>";

    XMLConstruct construct;
    _respFormatter->formatGreenScreenMsg(construct, response, *_pTrx, fareCalcCollector);
    CPPUNIT_ASSERT_EQUAL(expected, construct.getXMLData());
  }

  void test_formatGreenScreenResponse_nonusdot()
  {
    const FareCalcCollector* fareCalcCollector = greenScreeSetup();
    _pTrx->itin().front()->setBaggageTripType(BaggageTripType::OTHER);
    setSchemaVersion(1, 1, 1);

    std::string endos = "\nENDOS*VLDFLTDTE/BLKOUDTE/CONDAPY/NOPARTIALRFND/RBKGSGD30/XSGBY30JUN14";
    std::string response = "BASE FARE  \nBAGGAGETEXT002\n 1-  PLN41119.00    USD12870.00     "
                           "356.90XT     USD13226.90ADT\nBAGGAGETEXT001" +
                           endos;

    std::string expected =
        "<MSG N06=\"X\" Q0K=\"3\" S18=\"BASE FARE  \"/>"
        "<MSG N06=\"Y\" Q0K=\"4\" S18=\"MIL-01\"/>"
        "<MSG N06=\"Y\" Q0K=\"5\" S18=\"CARRY ON ALLOWANCE\"/>"
        "<MSG N06=\"Y\" Q0K=\"6\" S18=\"MIADCA-02P\"/>"
        "<MSG N06=\"Y\" Q0K=\"7\" S18=\"01/UP TO 45 LINEAR INCHES/115 LINEAR CENTIMETERS\"/>"
        "<MSG N06=\"Y\" Q0K=\"8\" S18=\"ADDITIONAL ALLOWANCES AND/OR DISCOUNTS MAY APPLY\"/>"
        "<MSG N06=\"X\" Q0K=\"9\" S18=\" 1-  PLN41119.00    USD12870.00     356.90XT     "
        "USD13226.90ADT\"/>"
        "<MSG N06=\"Y\" Q0K=\"10\" S18=\"ADT-01\"/>"
        "<MSG N06=\"Y\" Q0K=\"11\" S18=\"CARRY ON ALLOWANCE\"/>"
        "<MSG N06=\"Y\" Q0K=\"12\" S18=\"MIADCA-02P\"/>"
        "<MSG N06=\"Y\" Q0K=\"13\" S18=\" \"/>"
        "<MSG N06=\"Y\" Q0K=\"14\" S18=\"EMBARGOES-APPLY TO EACH PASSENGER\"/>"
        "<MSG N06=\"Y\" Q0K=\"15\" S18=\"FRANRT-LH\"/>"
        "<MSG N06=\"X\" Q0K=\"16\" "
        "S18=\"ENDOS*VLDFLTDTE/BLKOUDTE/CONDAPY/NOPARTIALRFND/RBKGSGD30/XSGBY30JUN14\"/>";

    XMLConstruct construct;
    _respFormatter->formatGreenScreenMsg(construct, response, *_pTrx, fareCalcCollector);
    CPPUNIT_ASSERT_EQUAL(expected, construct.getXMLData());
  }

  void test_formatGreenScreenResponse_maxBuffSize1()
  {
    const FareCalcCollector* fareCalcCollector = greenScreeSetup();
    _pTrx->itin().front()->setBaggageTripType(BaggageTripType::OTHER);
    setSchemaVersion(1, 1, 1);
    _respFormatter->_maxTotalBuffSize = 100;

    std::string endos = "\nENDOS*VLDFLTDTE/BLKOUDTE/CONDAPY/NOPARTIALRFND/RBKGSGD30/XSGBY30JUN14";
    std::string response = "BASE FARE  \nBAGGAGETEXT002\n 1-  PLN41119.00    USD12870.00     "
                           "356.90XT     USD13226.90ADT\n " +
                           endos;

    std::string expected = "<MSG N06=\"X\" Q0K=\"3\" S18=\"BASE FARE  \"/>"
                           "<MSG N06=\"Y\" Q0K=\"4\" S18=\"MIL-01\"/>"
                           "<MSG N06=\"Y\" Q0K=\"5\" S18=\"CARRY ON ALLOWANCE\"/>"
                           "<MSG N06=\"X\" Q0K=\"6\" S18=\"RESPONSE TOO LONG FOR CRT\"/>";

    XMLConstruct construct;
    _respFormatter->formatGreenScreenMsg(construct, response, *_pTrx, fareCalcCollector);
    CPPUNIT_ASSERT_EQUAL(expected, construct.getXMLData());
  }

  void test_formatGreenScreenResponse_maxBuffSize2()
  {
    const FareCalcCollector* fareCalcCollector = greenScreeSetup(false);
    _pTrx->itin().front()->setBaggageTripType(BaggageTripType::OTHER);
    setSchemaVersion(1, 1, 1);
    _respFormatter->_maxTotalBuffSize = 320;

    std::string endos = "\nENDOS*VLDFLTDTE/BLKOUDTE/CONDAPY/NOPARTIALRFND/RBKGSGD30/XSGBY30JUN14";
    std::string response = "BASE FARE  \nBAGGAGETEXT002\n 1-  PLN41119.00    USD12870.00     "
                           "356.90XT     USD13226.90ADT\n JOG GA  B   11NOV TEE14DSG              "
                           "25NOV 20K" +
                           endos;

    std::string expected =
        "<MSG N06=\"X\" Q0K=\"3\" S18=\"BASE FARE  \"/>"
        "<MSG N06=\"Y\" Q0K=\"4\" S18=\"MIL-01\"/>"
        "<MSG N06=\"Y\" Q0K=\"5\" S18=\"CARRY ON ALLOWANCE\"/>"
        "<MSG N06=\"Y\" Q0K=\"6\" S18=\"MIADCA-02P\"/>"
        "<MSG N06=\"Y\" Q0K=\"7\" S18=\"01/UP TO 45 LINEAR INCHES/115 LINEAR CENTIMETERS\"/>"
        "<MSG N06=\"Y\" Q0K=\"8\" S18=\"ADDITIONAL ALLOWANCES AND/OR DISCOUNTS MAY APPLY\"/>"
        "<MSG N06=\"X\" Q0K=\"9\" S18=\" 1-  PLN41119.00    USD12870.00     356.90XT     "
        "USD13226.90ADT\"/>"
        "<MSG N06=\"X\" Q0K=\"10\" S18=\"RESPONSE TOO LONG FOR CRT\"/>";

    XMLConstruct construct;
    _respFormatter->formatGreenScreenMsg(construct, response, *_pTrx, fareCalcCollector);
    CPPUNIT_ASSERT_EQUAL(expected, construct.getXMLData());
  }

  void prepareIsCategory25Applies(uint16_t category,
                                  PaxTypeFare& ptf,
                                  PaxTypeFareRuleData& ptfRD,
                                  FareByRuleItemInfo& fbrItemInfo,
                                  PaxTypeFare& baseFare,
                                  PaxTypeFareRuleData& basePtfRD,
                                  Indicator cat1,
                                  Indicator cat10)
  {
    ptf.setRuleData(25, _pTrx->dataHandle(), &ptfRD, true);
    ptfRD.ruleItemInfo() = &fbrItemInfo;
    fbrItemInfo.ovrdcat1() = cat1;
    fbrItemInfo.ovrdcat10() = cat10;
    ptfRD.baseFare() = &baseFare;
    baseFare.setRuleData(category, _pTrx->dataHandle(), &basePtfRD, true);
    basePtfRD.ruleItemInfo() = &fbrItemInfo;
  }

  void testIsCategory25AppliesWhenHasBaseCat1()
  {
    PaxTypeFare ptf, baseFare;
    PaxTypeFareRuleData ptfRD, basePtfRD;
    FareByRuleItemInfo rbrItemInfo;
    uint16_t category = 1;
    Indicator cat1 = 'B';
    Indicator cat10 = 'X';

    prepareIsCategory25Applies(category, ptf, ptfRD, rbrItemInfo, baseFare, basePtfRD, cat1, cat10);

    CPPUNIT_ASSERT(_respFormatter->isCategory25Applies(category, ptf));
  }

  void testIsCategory25AppliesWhenHasBaseCat1ButNoCatRuleInfo()
  {
    PaxTypeFare ptf, baseFare;
    PaxTypeFareRuleData ptfRD, basePtfRD;
    FareByRuleItemInfo rbrItemInfo;
    uint16_t category = 1;
    Indicator cat1 = 'B';
    Indicator cat10 = 'X';

    prepareIsCategory25Applies(category, ptf, ptfRD, rbrItemInfo, baseFare, basePtfRD, cat1, cat10);
    basePtfRD.ruleItemInfo() = 0;

    CPPUNIT_ASSERT(!_respFormatter->isCategory25Applies(category, ptf));
  }

  void testIsCategory25AppliesWhenHasntBaseCat1()
  {
    PaxTypeFare ptf, baseFare;
    PaxTypeFareRuleData ptfRD, basePtfRD;
    FareByRuleItemInfo rbrItemInfo;
    uint16_t category = 1;
    Indicator cat1 = 'X';
    Indicator cat10 = 'B';

    prepareIsCategory25Applies(category, ptf, ptfRD, rbrItemInfo, baseFare, basePtfRD, cat1, cat10);

    CPPUNIT_ASSERT(!_respFormatter->isCategory25Applies(category, ptf));
  }

  void testIsCategory25AppliesWhenHasBaseCat10()
  {
    PaxTypeFare ptf, baseFare;
    PaxTypeFareRuleData ptfRD, basePtfRD;
    FareByRuleItemInfo rbrItemInfo;
    uint16_t category = 10;
    Indicator cat1 = 'X';
    Indicator cat10 = 'B';
    CombinabilityRuleInfo cRuleInfo;

    prepareIsCategory25Applies(category, ptf, ptfRD, rbrItemInfo, baseFare, basePtfRD, cat1, cat10);
    baseFare.rec2Cat10() = &cRuleInfo;

    CPPUNIT_ASSERT(_respFormatter->isCategory25Applies(category, ptf));
  }

  void testIsCategory25AppliesWhenHasntBaseCat10()
  {
    PaxTypeFare ptf, baseFare;
    PaxTypeFareRuleData ptfRD, basePtfRD;
    FareByRuleItemInfo rbrItemInfo;
    uint16_t category = 10;
    Indicator cat1 = 'B';
    Indicator cat10 = 'X';

    prepareIsCategory25Applies(category, ptf, ptfRD, rbrItemInfo, baseFare, basePtfRD, cat1, cat10);
    baseFare.rec2Cat10() = 0;

    CPPUNIT_ASSERT(!_respFormatter->isCategory25Applies(category, ptf));
  }

  void testIsCategory25AppliesWhenNoBaseCategory()
  {
    PaxTypeFare ptf, baseFare;
    PaxTypeFareRuleData ptfRD, basePtfRD;
    FareByRuleItemInfo rbrItemInfo;
    uint16_t category = 1;
    Indicator cat1 = 'B';
    Indicator cat10 = ' ';

    prepareIsCategory25Applies(category, ptf, ptfRD, rbrItemInfo, baseFare, basePtfRD, cat1, cat10);
    ptfRD.baseFare() = 0;

    CPPUNIT_ASSERT(!_respFormatter->isCategory25Applies(category, ptf));
  }

  void testGetLowestObFeeAmountWhenNoCurrency()
  {
    CurrencyCode maxFeeCur;
    MoneyAmount calcAmount = 100.00;
    MoneyAmount maxAmount = 20.00;

    MoneyAmount lowestAmt = _respFormatter->getLowestObFeeAmount(maxFeeCur, calcAmount, maxAmount);
    CPPUNIT_ASSERT_EQUAL(lowestAmt, calcAmount);
  }

  void testGetLowestObFeeAmountWhenCurrency()
  {
    CurrencyCode maxFeeCur = "GBP";
    MoneyAmount calcAmount = 100.00;
    MoneyAmount maxAmount = 20.00;

    MoneyAmount lowestAmt = _respFormatter->getLowestObFeeAmount(maxFeeCur, calcAmount, maxAmount);
    CPPUNIT_ASSERT_EQUAL(lowestAmt, maxAmount);
  }

  void testCalculateResidualObFeeAmountWhenNoResidual()
  {
    _pTrx->getRequest()->chargeResidualInd() = false;
    _pTrx->getRequest()->paymentAmountFop() = 1000;
    _feeInfo->feePercent() = 5;
    MoneyAmount totalAmt = 200;
    MoneyAmount testAmount = 50;

    MoneyAmount calcAmt = _respFormatter->calculateResidualObFeeAmount(*_pTrx, totalAmt, _feeInfo);

    CPPUNIT_ASSERT_EQUAL(calcAmt, testAmount);
  }

  void testCalculateResidualObFeeAmountWhenResidualFop()
  {
    _pTrx->getRequest()->chargeResidualInd() = true;
    _pTrx->getRequest()->paymentAmountFop() = 100;
    _feeInfo->feePercent() = 5;
    MoneyAmount totalAmt = 300;
    MoneyAmount testAmount = 10;

    MoneyAmount calcAmt = _respFormatter->calculateResidualObFeeAmount(*_pTrx, totalAmt, _feeInfo);

    CPPUNIT_ASSERT_EQUAL(calcAmt, testAmount);
  }

  void testCalculateResidualObFeeAmountWhenResidualFopNegative()
  {
    _pTrx->getRequest()->chargeResidualInd() = true;
    _pTrx->getRequest()->paymentAmountFop() = 100;
    _feeInfo->feePercent() = 5;
    MoneyAmount totalAmt = 30;
    MoneyAmount testAmount = 0.0;

    MoneyAmount calcAmt = _respFormatter->calculateResidualObFeeAmount(*_pTrx, totalAmt, _feeInfo);

    CPPUNIT_ASSERT_EQUAL(calcAmt, testAmount);
  }

  void testCalculateResidualObFeeAmountWhenResidual()
  {
    _pTrx->getRequest()->chargeResidualInd() = true;
    _pTrx->getRequest()->paymentAmountFop() = 0;
    _feeInfo->feePercent() = 5;
    MoneyAmount totalAmt = 200;
    MoneyAmount testAmount = 10;

    MoneyAmount calcAmt = _respFormatter->calculateResidualObFeeAmount(*_pTrx, totalAmt, _feeInfo);

    CPPUNIT_ASSERT_EQUAL(calcAmt, testAmount);
  }

  void test_buildParticipatingCarrier_STD()
  {
    vcx::ParticipatingCxr pcx("PC", vcx::STANDARD);
    XMLConstruct construct;
    _respFormatter->buildPartcipatingCarrier(construct, pcx);

    std::stringstream expectedXml;
    expectedXml << "<PCX B00=\"PC\" VC1=\"STD\"/>";

    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_buildParticipatingCarrier_3PT()
  {
    vcx::ParticipatingCxr pcx("PC", vcx::THIRD_PARTY);
    XMLConstruct construct;
    _respFormatter->buildPartcipatingCarrier(construct, pcx);

    std::stringstream expectedXml;
    expectedXml << "<PCX B00=\"PC\" VC1=\"3PT\"/>";

    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_buildParticipatingCarrier_PPR()
  {
    vcx::ParticipatingCxr pcx("PC", vcx::PAPER_ONLY);
    XMLConstruct construct;
    _respFormatter->buildPartcipatingCarrier(construct, pcx);

    std::stringstream expectedXml;
    expectedXml << "<PCX B00=\"PC\" VC1=\"PPR\"/>";

    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_buildValidatingCarrier_Default()
  {
    FarePath* farePath = FarePathBuilder(&_memH).build();
    farePath->itin() = _itin;

    const CarrierCode vcxr = "VC";
    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::PAPER_TKT_PREF;

    ValidatingCxrGSAData v;
    v.validatingCarriersData()[vcxr] = vcxrData;
    _itin->validatingCxrGsaData() = &v;

    XMLConstruct construct;
    _respFormatter->buildValidatingCarrier(construct, *_itin, vcxr, true);

    std::stringstream expectedXml;
    expectedXml << "<DCX B00=\"VC\" TT0=\"PTKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</DCX>";

    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_buildValidatingCarrier_Alternate()
  {
    FarePath* farePath = FarePathBuilder(&_memH).build();
    farePath->itin() = _itin;

    const CarrierCode vcxr = "VC";
    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::PAPER_TKT_PREF;

    ValidatingCxrGSAData v;
    v.validatingCarriersData()[vcxr] = vcxrData;
    _itin->validatingCxrGsaData() = &v;

    XMLConstruct construct;
    _respFormatter->buildValidatingCarrier(construct, *_itin, vcxr);

    std::stringstream expectedXml;
    expectedXml << "<ACX B00=\"VC\" TT0=\"PTKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>";

    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_buildValidatingCarrier_MaxPcx()
  {
    FarePath* farePath = FarePathBuilder(&_memH).build();
    farePath->defaultValidatingCarrier() = "VC";
    farePath->processed() = true;
    farePath->itin() = _itin;

    const CarrierCode vcxr = "VC";
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::PAPER_TKT_PREF;
    _pTrx->countrySettlementPlanInfo()->setSettlementPlanTypeCode("BSP");
    vcx::ParticipatingCxr pcx;
    for (uint16_t i = 1; i < vcx::MAX_PARTICIPATING_CARRIERS + 5; ++i)
    {
      pcx.cxrName = "P" + boost::lexical_cast<std::string>(i);
      vcxrData.participatingCxrs.push_back(pcx);
    }

    ValidatingCxrGSAData v;
    v.validatingCarriersData()[vcxr] = vcxrData;
    _itin->validatingCxrGsaData() = &v;

    XMLConstruct construct;
    _respFormatter->buildValidatingCarrier(construct, *_itin, vcxr, true);

    std::stringstream expectedXml;
    expectedXml << "<DCX B00=\"VC\" TT0=\"PTKTPREF\">"
                << "<PCX B00=\"P1\"/>"
                << "<PCX B00=\"P2\"/>"
                << "<PCX B00=\"P3\"/>"
                << "<PCX B00=\"P4\"/>"
                << "<PCX B00=\"P5\"/>"
                << "<PCX B00=\"P6\"/>"
                << "<PCX B00=\"P7\"/>"
                << "<PCX B00=\"P8\"/>"
                << "<PCX B00=\"P9\"/>"
                << "<PCX B00=\"P10\"/>"
                << "<PCX B00=\"P11\"/>"
                << "<PCX B00=\"P12\"/>"
                << "<PCX B00=\"P13\"/>"
                << "<PCX B00=\"P14\"/>"
                << "<PCX B00=\"P15\"/>"
                << "<PCX B00=\"P16\"/>"
                << "<PCX B00=\"P17\"/>"
                << "<PCX B00=\"P18\"/>"
                << "<PCX B00=\"P19\"/>"
                << "<PCX B00=\"P20\"/>"
                << "<PCX B00=\"P21\"/>"
                << "<PCX B00=\"P22\"/>"
                << "<PCX B00=\"P23\"/>"
                << "<PCX B00=\"P24\"/>"
                << "<PCX B00=\"P25\"/>"
                << "<PCX B00=\"P26\"/>"
                << "<PCX B00=\"P27\"/>"
                << "<PCX B00=\"P28\"/>"
                << "<PCX B00=\"P29\"/>"
                << "<PCX B00=\"P30\"/>"
                << "<PCX B00=\"P31\"/>"
                << "<PCX B00=\"P32\"/>"
                << "<PCX B00=\"P33\"/>"
                << "<PCX B00=\"P34\"/>"
                << "<PCX B00=\"P35\"/>"
                << "<PCX B00=\"P36\"/>"
                << "<PCX B00=\"P37\"/>"
                << "<PCX B00=\"P38\"/>"
                << "<PCX B00=\"P39\"/>"
                << "<PCX B00=\"P40\"/>"
                << "<PCX B00=\"P41\"/>"
                << "<PCX B00=\"P42\"/>"
                << "<PCX B00=\"P43\"/>"
                << "<PCX B00=\"P44\"/>"
                << "<PCX B00=\"P45\"/>"
                << "<PCX B00=\"P46\"/>"
                << "<PCX B00=\"P47\"/>"
                << "<PCX B00=\"P48\"/>"
                << "</DCX>";

    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_buildValidatingCarrierList_NoValidatingCxr()
  {
    FarePath* farePath = FarePathBuilder(&_memH).build();
    farePath->processed() = true;
    farePath->itin() = _itin;

    XMLConstruct construct;
    _respFormatter->buildValidatingCarrierList(*_pTrx, construct, *farePath);

    std::stringstream expectedXml;
    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_buildValidatingCarrierList_NoValidatingCxrMap()
  {
    FarePath* farePath = FarePathBuilder(&_memH).build();
    farePath->defaultValidatingCarrier() = "VC";
    farePath->processed() = true;
    farePath->itin() = _itin;

    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::ETKT_PREF;

    ValidatingCxrGSAData v;
    _itin->validatingCxrGsaData() = &v;

    XMLConstruct construct;
    _respFormatter->buildValidatingCarrierList(*_pTrx, construct, *farePath);

    std::stringstream expectedXml;
    expectedXml << "<VCL P3L=\"F\" VC0=\"T\">"
                << "<DCX B00=\"VC\"/>"
                << "</VCL>";

    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_buildValidatingCarrierList_VC_Vector_Empty()
  {
    FarePath* farePath = FarePathBuilder(&_memH).build();
    farePath->defaultValidatingCarrier() = "VC";
    farePath->processed() = true;
    farePath->itin() = _itin;

    const CarrierCode validatingCxr = "VC";

    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::ETKT_PREF;
    _pTrx->countrySettlementPlanInfo()->setSettlementPlanTypeCode("GEN");

    ValidatingCxrGSAData v;
    v.validatingCarriersData()[validatingCxr] = vcxrData;

    _itin->validatingCxrGsaData() = &v;
    XMLConstruct construct;
    _respFormatter->buildValidatingCarrierList(*_pTrx, construct, *farePath);

    std::stringstream expectedXml;
    expectedXml << "<VCL P3L=\"F\" SM0=\"GEN\" VC0=\"T\">"
                << "<DCX B00=\"VC\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</DCX>"
                << "</VCL>";

    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_buildValidatingCarrierList_DefaultVC_Empty_VCvector_HasOneVC()
  {
    FarePath* farePath = FarePathBuilder(&_memH).build();
    farePath->validatingCarriers().push_back("VC");
    farePath->processed() = true;
    farePath->itin() = _itin;

    const CarrierCode validatingCxr = "VC";

    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::ETKT_PREF;
    _pTrx->countrySettlementPlanInfo()->setSettlementPlanTypeCode("GEN");

    ValidatingCxrGSAData v;
    v.validatingCarriersData()[validatingCxr] = vcxrData;

    _itin->validatingCxrGsaData() = &v;
    XMLConstruct construct;
    _respFormatter->buildValidatingCarrierList(*_pTrx, construct, *farePath);

    std::stringstream expectedXml;
    expectedXml << "<VCL P3L=\"F\" SM0=\"GEN\" VC0=\"T\">"
                << "<ACX B00=\"VC\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "</VCL>";

    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_buildValidatingCarrierList_DefaultVC_Empty_VCvector_HasMoreThanOneVC()
  {
    FarePath* farePath = FarePathBuilder(&_memH).build();
    farePath->validatingCarriers().push_back("VC");
    farePath->validatingCarriers().push_back("WN");
    farePath->processed() = true;
    farePath->itin() = _itin;

    const CarrierCode validatingCxr1 = "VC";
    const CarrierCode validatingCxr2 = "WN";

    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::ETKT_PREF;
    _pTrx->countrySettlementPlanInfo()->setSettlementPlanTypeCode("GEN");

    ValidatingCxrGSAData v;
    v.validatingCarriersData()[validatingCxr1] = vcxrData;
    v.validatingCarriersData()[validatingCxr2] = vcxrData;

    _itin->validatingCxrGsaData() = &v;
    XMLConstruct construct;
    _respFormatter->buildValidatingCarrierList(*_pTrx, construct, *farePath);

    std::stringstream expectedXml;
    expectedXml << "<VCL P3L=\"T\" SM0=\"GEN\" VC0=\"T\">"
                << "<ACX B00=\"VC\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "<ACX B00=\"WN\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "</VCL>";

    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_buildValidatingCarrierList_DefaultVC_Empty_VCvector_HasMoreThanOneNeutralVC()
  {
    FarePath* farePath = FarePathBuilder(&_memH).build();
    farePath->validatingCarriers().push_back("VC");
    farePath->validatingCarriers().push_back("WN");
    farePath->processed() = true;
    farePath->itin() = _itin;
    const CarrierCode validatingCxr1 = "VC";
    const CarrierCode validatingCxr2 = "WN";
    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::ETKT_PREF;

    _pTrx->countrySettlementPlanInfo()->setSettlementPlanTypeCode("GEN");

    ValidatingCxrGSAData v;
    v.validatingCarriersData()[validatingCxr1] = vcxrData;
    v.validatingCarriersData()[validatingCxr2] = vcxrData;
    v.isNeutralValCxr() = true;

    _itin->validatingCxrGsaData() = &v;
    XMLConstruct construct;
    _respFormatter->buildValidatingCarrierList(*_pTrx, construct, *farePath);

    std::stringstream expectedXml;
    expectedXml << "<VCL P3L=\"T\" SM0=\"GEN\" VC0=\"T\" MNV=\"T\">"
                << "<ACX B00=\"VC\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "<ACX B00=\"WN\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "</VCL>";
    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_buildValidatingCarrierList_DefaultVC_And_VCvector_HasMoreThanOneVC()
  {
    FarePath* farePath = FarePathBuilder(&_memH).build();
    farePath->defaultValidatingCarrier() = "DF";
    farePath->validatingCarriers().push_back("VC");
    farePath->validatingCarriers().push_back("WN");
    farePath->processed() = true;
    farePath->itin() = _itin;

    const CarrierCode validatingCxr1 = "DF";
    const CarrierCode validatingCxr2 = "VC";
    const CarrierCode validatingCxr3 = "WN";

    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::ETKT_PREF;
    _pTrx->countrySettlementPlanInfo()->setSettlementPlanTypeCode("GEN");

    ValidatingCxrGSAData v;
    v.validatingCarriersData()[validatingCxr1] = vcxrData;
    v.validatingCarriersData()[validatingCxr2] = vcxrData;
    v.validatingCarriersData()[validatingCxr3] = vcxrData;

    _itin->validatingCxrGsaData() = &v;
    XMLConstruct construct;
    _respFormatter->buildValidatingCarrierList(*_pTrx, construct, *farePath);

    std::stringstream expectedXml;
    expectedXml << "<VCL P3L=\"F\" SM0=\"GEN\" VC0=\"T\">"
                << "<DCX B00=\"DF\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</DCX>"
                << "<ACX B00=\"VC\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "<ACX B00=\"WN\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "</VCL>";

    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_buildValidatingCarrier_Default_MultiSp()
  {
    const CarrierCode vcxr = "VC";
    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::PAPER_TKT_PREF;
    const bool isDefaultVcxr = true;

    XMLConstruct construct;
    _respFormatter->buildValidatingCarrier(construct, vcxr, vcxrData, isDefaultVcxr);

    std::stringstream expectedXml;
    expectedXml << "<DCX B00=\"VC\" TT0=\"PTKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</DCX>";

    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_buildValidatingCarrier_Alternate_MultiSp()
  {
    const CarrierCode vcxr = "VC";
    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::PAPER_TKT_PREF;
    const bool isDefaultVcxr = false;

    XMLConstruct construct;
    _respFormatter->buildValidatingCarrier(construct, vcxr, vcxrData, isDefaultVcxr);

    std::stringstream expectedXml;
    expectedXml << "<ACX B00=\"VC\" TT0=\"PTKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>";

    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_buildValidatingCarrier_MaxPcx_MultiSp()
  {
    const CarrierCode vcxr = "VC";
    vcx::ValidatingCxrData vcxrData;
    vcxrData.ticketType = vcx::PAPER_TKT_PREF;
    _pTrx->countrySettlementPlanInfo()->setSettlementPlanTypeCode("BSP");
    vcx::ParticipatingCxr pcx;
    for (uint16_t i = 1; i < vcx::MAX_PARTICIPATING_CARRIERS + 5; ++i)
    {
      pcx.cxrName = "P" + boost::lexical_cast<std::string>(i);
      vcxrData.participatingCxrs.push_back(pcx);
    }

    const bool isDefaultVcxr = true;
    XMLConstruct construct;
    _respFormatter->buildValidatingCarrier(construct, vcxr, vcxrData, isDefaultVcxr);

    std::stringstream expectedXml;
    expectedXml << "<DCX B00=\"VC\" TT0=\"PTKTPREF\">"
                << "<PCX B00=\"P1\"/>"
                << "<PCX B00=\"P2\"/>"
                << "<PCX B00=\"P3\"/>"
                << "<PCX B00=\"P4\"/>"
                << "<PCX B00=\"P5\"/>"
                << "<PCX B00=\"P6\"/>"
                << "<PCX B00=\"P7\"/>"
                << "<PCX B00=\"P8\"/>"
                << "<PCX B00=\"P9\"/>"
                << "<PCX B00=\"P10\"/>"
                << "<PCX B00=\"P11\"/>"
                << "<PCX B00=\"P12\"/>"
                << "<PCX B00=\"P13\"/>"
                << "<PCX B00=\"P14\"/>"
                << "<PCX B00=\"P15\"/>"
                << "<PCX B00=\"P16\"/>"
                << "<PCX B00=\"P17\"/>"
                << "<PCX B00=\"P18\"/>"
                << "<PCX B00=\"P19\"/>"
                << "<PCX B00=\"P20\"/>"
                << "<PCX B00=\"P21\"/>"
                << "<PCX B00=\"P22\"/>"
                << "<PCX B00=\"P23\"/>"
                << "<PCX B00=\"P24\"/>"
                << "<PCX B00=\"P25\"/>"
                << "<PCX B00=\"P26\"/>"
                << "<PCX B00=\"P27\"/>"
                << "<PCX B00=\"P28\"/>"
                << "<PCX B00=\"P29\"/>"
                << "<PCX B00=\"P30\"/>"
                << "<PCX B00=\"P31\"/>"
                << "<PCX B00=\"P32\"/>"
                << "<PCX B00=\"P33\"/>"
                << "<PCX B00=\"P34\"/>"
                << "<PCX B00=\"P35\"/>"
                << "<PCX B00=\"P36\"/>"
                << "<PCX B00=\"P37\"/>"
                << "<PCX B00=\"P38\"/>"
                << "<PCX B00=\"P39\"/>"
                << "<PCX B00=\"P40\"/>"
                << "<PCX B00=\"P41\"/>"
                << "<PCX B00=\"P42\"/>"
                << "<PCX B00=\"P43\"/>"
                << "<PCX B00=\"P44\"/>"
                << "<PCX B00=\"P45\"/>"
                << "<PCX B00=\"P46\"/>"
                << "<PCX B00=\"P47\"/>"
                << "<PCX B00=\"P48\"/>"
                << "</DCX>";

    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void setSpValidatingCxrGsaData(FarePath& fp,
      ValidatingCxrGSAData* valCxrGsaData,
      std::initializer_list<SettlementPlanType> sp_list)
  {
    if (!fp.itin()->spValidatingCxrGsaDataMap())
      fp.itin()->spValidatingCxrGsaDataMap() = _pTrx->dataHandle().create<SpValidatingCxrGSADataMap>();

    SpValidatingCxrGSADataMap* ref = fp.itin()->spValidatingCxrGsaDataMap();
    for (auto sp : sp_list)
      ref->insert(std::pair<SettlementPlanType, const ValidatingCxrGSAData*>(sp, valCxrGsaData));
  }

  void test_prepareValidatingCarrierLists_OneSettlementPlan()
  {
    const CarrierCode VC1 = "DC";
    const CarrierCode VC2 = "A1";
    const CarrierCode VC3 = "A2";
    const SettlementPlanType SP = "BSP";

    FarePath* fp = FarePathBuilder(&_memH).build();
    fp->defaultValCxrPerSp()[SP] = VC1;
    fp->validatingCarriers().push_back(VC1);
    fp->validatingCarriers().push_back(VC2);
    fp->validatingCarriers().push_back(VC3);
    fp->processed() = true;
    fp->itin() = _itin;
    fp->settlementPlanValidatingCxrs()[SP].push_back(VC1);
    fp->settlementPlanValidatingCxrs()[SP].push_back(VC2);
    fp->settlementPlanValidatingCxrs()[SP].push_back(VC3);

    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::ETKT_PREF;

    ValidatingCxrGSAData gsaData;
    gsaData.validatingCarriersData()[VC1] = vcxrData;
    gsaData.validatingCarriersData()[VC2] = vcxrData;
    gsaData.validatingCarriersData()[VC3] = vcxrData;

    setSpValidatingCxrGsaData(*fp, &gsaData, {SP});

    XMLConstruct construct;
    _respFormatter->prepareValidatingCarrierLists( *_pTrx, construct, *fp );

    std::stringstream expectedXml;
    expectedXml << "<VCL P3L=\"F\" SM0=\"BSP\" VC0=\"T\">"
                << "<DCX B00=\"DC\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</DCX>"
                << "<ACX B00=\"A1\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "<ACX B00=\"A2\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "</VCL>";

    CPPUNIT_ASSERT_EQUAL( expectedXml.str(), construct.getXMLData() );
  }

  void test_prepareValidatingCarrierLists_MultiSp()
  {
    const CarrierCode VC1 = "DC";
    const CarrierCode VC2 = "A1";
    const CarrierCode VC3 = "A2";
    const SettlementPlanType SP1 = "BSP";
    const SettlementPlanType SP2 = "GEN";

    FarePath* fp = FarePathBuilder(&_memH).build();
    fp->defaultValCxrPerSp()[SP1] = VC1;
    fp->defaultValCxrPerSp()[SP2] = VC2;
    fp->validatingCarriers().push_back(VC3);
    fp->validatingCarriers().push_back(VC2);
    fp->validatingCarriers().push_back(VC1);
    fp->processed() = true;
    fp->itin() = _itin;

    fp->settlementPlanValidatingCxrs()[SP1].push_back(VC1);
    fp->settlementPlanValidatingCxrs()[SP1].push_back(VC2);
    fp->settlementPlanValidatingCxrs()[SP1].push_back(VC3);

    fp->settlementPlanValidatingCxrs()[SP2].push_back(VC1);
    fp->settlementPlanValidatingCxrs()[SP2].push_back(VC2);
    fp->settlementPlanValidatingCxrs()[SP2].push_back(VC3);

    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::ETKT_PREF;

    ValidatingCxrGSAData gsaData;
    gsaData.validatingCarriersData()[VC1] = vcxrData;
    gsaData.validatingCarriersData()[VC2] = vcxrData;
    gsaData.validatingCarriersData()[VC3] = vcxrData;

    setSpValidatingCxrGsaData(*fp, &gsaData, {SP1, SP2});

    XMLConstruct construct;
    _respFormatter->prepareValidatingCarrierLists( *_pTrx, construct, *fp );

    std::stringstream expectedXml;
    expectedXml << "<VCL P3L=\"F\" SM0=\"BSP\" VC0=\"T\">"
                << "<DCX B00=\"DC\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</DCX>"
                << "<ACX B00=\"A2\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "<ACX B00=\"A1\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "</VCL>"
                << "<VCL P3L=\"F\" SM0=\"GEN\" VC0=\"T\">"
                << "<DCX B00=\"A1\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</DCX>"
                << "<ACX B00=\"A2\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "<ACX B00=\"DC\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "</VCL>";

    CPPUNIT_ASSERT_EQUAL( expectedXml.str(), construct.getXMLData() );
  }

  // At least one empty default carrier, means we inhibit PQ
  void test_prepareValidatingCarrierLists_InhibitPq()
  {
    const CarrierCode VC1 = "DC";
    const CarrierCode VC2 = "A1";
    const CarrierCode VC3 = "A2";
    const SettlementPlanType SP1 = "BSP";
    const SettlementPlanType SP2 = "GTC";

    FarePath* fp = FarePathBuilder(&_memH).build();
    fp->defaultValCxrPerSp()[SP1] = VC1;
    fp->defaultValCxrPerSp()[SP2] = "";
    fp->validatingCarriers().push_back(VC3);
    fp->validatingCarriers().push_back(VC2);
    fp->validatingCarriers().push_back(VC1);
    fp->processed() = true;
    fp->itin() = _itin;

    fp->settlementPlanValidatingCxrs()[SP1].push_back(VC1);
    fp->settlementPlanValidatingCxrs()[SP1].push_back(VC2);
    fp->settlementPlanValidatingCxrs()[SP1].push_back(VC3);

    fp->settlementPlanValidatingCxrs()[SP2].push_back(VC1);
    fp->settlementPlanValidatingCxrs()[SP2].push_back(VC2);
    fp->settlementPlanValidatingCxrs()[SP2].push_back(VC3);

    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::ETKT_PREF;

    ValidatingCxrGSAData gsaData;
    gsaData.validatingCarriersData()[VC1] = vcxrData;
    gsaData.validatingCarriersData()[VC2] = vcxrData;
    gsaData.validatingCarriersData()[VC3] = vcxrData;

    setSpValidatingCxrGsaData(*fp, &gsaData, {SP1, SP2});

    XMLConstruct construct;
    _respFormatter->prepareValidatingCarrierLists( *_pTrx, construct, *fp );

    std::stringstream expectedXml;
    expectedXml << "<VCL P3L=\"T\" SM0=\"BSP\" VC0=\"T\">"
                << "<DCX B00=\"DC\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</DCX>"
                << "<ACX B00=\"A2\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "<ACX B00=\"A1\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "</VCL>"
                << "<VCL P3L=\"T\" SM0=\"GTC\" VC0=\"T\">"
                << "<ACX B00=\"A2\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "<ACX B00=\"A1\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "<ACX B00=\"DC\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "</VCL>";

    CPPUNIT_ASSERT_EQUAL( expectedXml.str(), construct.getXMLData() );
  }

  // Default carrier and multiple alternates
  void test_buildValidatingCarrierList_DefaultVC_And_VCvector_HasMoreThanOneVC_MultiSp()
  {
    const CarrierCode VC1 = "DC";
    const CarrierCode VC2 = "A1";
    const CarrierCode VC3 = "A2";
    const SettlementPlanType sp = "BSP";
    const CarrierCode defaultVcxr = VC1;
    const std::string inhibitPq = "F";

    FarePath* farePath = FarePathBuilder(&_memH).build();
    farePath->validatingCarriers().push_back(VC1);
    farePath->validatingCarriers().push_back(VC2);
    farePath->validatingCarriers().push_back(VC3);
    farePath->processed() = true;
    farePath->itin() = _itin;

    farePath->settlementPlanValidatingCxrs()[sp].push_back(VC1);
    farePath->settlementPlanValidatingCxrs()[sp].push_back(VC2);
    farePath->settlementPlanValidatingCxrs()[sp].push_back(VC3);

    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::ETKT_PREF;

    ValidatingCxrGSAData gsaData;
    gsaData.validatingCarriersData()[VC1] = vcxrData;
    gsaData.validatingCarriersData()[VC2] = vcxrData;
    gsaData.validatingCarriersData()[VC3] = vcxrData;

    setSpValidatingCxrGsaData(*farePath, &gsaData, {sp});

    XMLConstruct construct;
    _respFormatter->buildValidatingCarrierList( construct, *farePath, sp, defaultVcxr, inhibitPq );

    std::stringstream expectedXml;
    expectedXml << "<VCL P3L=\"F\" SM0=\"BSP\" VC0=\"T\">"
                << "<DCX B00=\"DC\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</DCX>"
                << "<ACX B00=\"A1\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "<ACX B00=\"A2\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "</VCL>";

    CPPUNIT_ASSERT_EQUAL( expectedXml.str(), construct.getXMLData() );
  }

  // Default carrier and no alternates. Only 1 carrier on the list.
  void test_buildValidatingCarrierList_DefaultAndNoAlternates_MultiSP()
  {
    const CarrierCode VC1 = "DC";
    const CarrierCode VC2 = "A1";
    const CarrierCode VC3 = "A2";
    const SettlementPlanType sp = "BSP";
    const CarrierCode defaultVcxr = VC1;
    const std::string inhibitPq = "F";

    FarePath* farePath = FarePathBuilder(&_memH).build();
    farePath->validatingCarriers().push_back(VC1);
    farePath->processed() = true;
    farePath->itin() = _itin;

    farePath->settlementPlanValidatingCxrs()[sp].push_back(VC1);
    farePath->settlementPlanValidatingCxrs()[sp].push_back(VC2);
    farePath->settlementPlanValidatingCxrs()[sp].push_back(VC3);

    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::ETKT_PREF;

    ValidatingCxrGSAData gsaData;
    gsaData.validatingCarriersData()[VC1] = vcxrData;
    gsaData.validatingCarriersData()[VC2] = vcxrData;
    gsaData.validatingCarriersData()[VC3] = vcxrData;

    setSpValidatingCxrGsaData(*farePath, &gsaData, {sp});

    XMLConstruct construct;
    _respFormatter->buildValidatingCarrierList( construct, *farePath, sp, defaultVcxr, inhibitPq );

    std::stringstream expectedXml;
    expectedXml << "<VCL P3L=\"F\" SM0=\"BSP\" VC0=\"T\">"
                << "<DCX B00=\"DC\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</DCX>"
                << "</VCL>";

    CPPUNIT_ASSERT_EQUAL( expectedXml.str(), construct.getXMLData() );
  }

  // Default carrier and no alternates.
  // Multiple carriers on the list but only one is applicable to the given settlement plan.
  void test_buildValidatingCarrierList_DefaultAndNoAlternates2_MultiSP()
  {
    const CarrierCode VC1 = "DC";
    const CarrierCode VC2 = "A1";
    const CarrierCode VC3 = "A2";
    const SettlementPlanType sp = "BSP";
    const CarrierCode defaultVcxr = VC1;
    const std::string inhibitPq = "F";

    FarePath* farePath = FarePathBuilder(&_memH).build();
    farePath->validatingCarriers().push_back(VC1);
    farePath->validatingCarriers().push_back(VC2);
    farePath->validatingCarriers().push_back(VC3);
    farePath->processed() = true;
    farePath->itin() = _itin;
    farePath->settlementPlanValidatingCxrs()[sp].push_back(VC1);

    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::ETKT_PREF;

    ValidatingCxrGSAData gsaData;
    gsaData.validatingCarriersData()[VC1] = vcxrData;
    gsaData.validatingCarriersData()[VC2] = vcxrData;
    gsaData.validatingCarriersData()[VC3] = vcxrData;

    setSpValidatingCxrGsaData(*farePath, &gsaData, {sp});

    XMLConstruct construct;
    _respFormatter->buildValidatingCarrierList( construct, *farePath, sp, defaultVcxr, inhibitPq );

    std::stringstream expectedXml;
    expectedXml << "<VCL P3L=\"F\" SM0=\"BSP\" VC0=\"T\">"
                << "<DCX B00=\"DC\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</DCX>"
                << "<ACX B00=\"A1\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "<ACX B00=\"A2\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "</VCL>";

    CPPUNIT_ASSERT_EQUAL( expectedXml.str(), construct.getXMLData() );
  }

  // Alternate carrier only.
  void test_buildValidatingCarrierList_AlternateOnly_MultiSP()
  {
    const CarrierCode VC1 = "A1";
    const SettlementPlanType sp = "BSP";
    const CarrierCode defaultVcxr; // No default
    const std::string inhibitPq = "F";

    FarePath* farePath = FarePathBuilder(&_memH).build();
    farePath->validatingCarriers().push_back(VC1);
    farePath->processed() = true;
    farePath->itin() = _itin;
    farePath->settlementPlanValidatingCxrs()[sp].push_back(VC1);

    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);
    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::ETKT_PREF;

    ValidatingCxrGSAData gsaData;
    gsaData.validatingCarriersData()[VC1] = vcxrData;

    setSpValidatingCxrGsaData(*farePath, &gsaData, {sp});

    XMLConstruct construct;
    _respFormatter->buildValidatingCarrierList( construct, *farePath, sp, defaultVcxr, inhibitPq );

    std::stringstream expectedXml;
    expectedXml << "<VCL P3L=\"F\" SM0=\"BSP\" VC0=\"T\">"
                << "<ACX B00=\"A1\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</ACX>"
                << "</VCL>";

    CPPUNIT_ASSERT_EQUAL( expectedXml.str(), construct.getXMLData() );
  }

  void test_buildValidatingCarrierList_NoItin_MultiSp()
  {
    FarePath* farePath = FarePathBuilder(&_memH).build();
    farePath->processed() = true;
    farePath->itin() = 0;
    const SettlementPlanType sp = "BSP";
    const CarrierCode defaultVcxr = "VC";
    const std::string inhibitPq = "F";

    XMLConstruct construct;
    _respFormatter->buildValidatingCarrierList( construct, *farePath, sp, defaultVcxr, inhibitPq );

    std::stringstream expectedXml;
    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_buildValidatingCarrierList_NoGsaDataForSp_MultiSp()
  {
    FarePath* farePath = FarePathBuilder(&_memH).build();
    farePath->processed() = true;
    farePath->itin() = _itin;
    const SettlementPlanType sp = "BSP";
    const CarrierCode defaultVcxr = "VC";
    const std::string inhibitPq = "F";

    const std::stringstream expectedXml;
    XMLConstruct construct;

    // Settlement plan is not within the GSA data map
    _respFormatter->buildValidatingCarrierList( construct, *farePath, sp, defaultVcxr, inhibitPq );
    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());

    // Settlement plan is in the map but GSA pointer is null
    setSpValidatingCxrGsaData(*farePath, 0, {sp});

    _respFormatter->buildValidatingCarrierList( construct, *farePath, sp, defaultVcxr, inhibitPq );
    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_buildValidatingCarrierList_NoValidatingCxr_MultiSp()
  {
    const CarrierCode VC1 = "DC";
    const CarrierCode VC2 = "A1";
    const CarrierCode VC3 = "A2";
    const SettlementPlanType sp = "BSP";
    const CarrierCode defaultVcxr = VC1;
    const std::string inhibitPq = "F";

    FarePath* farePath = FarePathBuilder(&_memH).build();
    farePath->validatingCarriers().clear(); // No validating carrier
    farePath->processed() = true;
    farePath->itin() = _itin;
    farePath->settlementPlanValidatingCxrs()[sp].push_back(VC1);
    farePath->settlementPlanValidatingCxrs()[sp].push_back(VC2);
    farePath->settlementPlanValidatingCxrs()[sp].push_back(VC3);

    vcx::ParticipatingCxr pc1("P1", vcx::PAPER_ONLY);
    vcx::ParticipatingCxr pc2("P2", vcx::THIRD_PARTY);
    vcx::ParticipatingCxr pc3("P3", vcx::STANDARD);

    vcx::ValidatingCxrData vcxrData;
    vcxrData.participatingCxrs.push_back(pc1);
    vcxrData.participatingCxrs.push_back(pc2);
    vcxrData.participatingCxrs.push_back(pc3);
    vcxrData.ticketType = vcx::ETKT_PREF;

    ValidatingCxrGSAData gsaData;
    gsaData.validatingCarriersData()[VC1] = vcxrData;
    gsaData.validatingCarriersData()[VC2] = vcxrData;
    gsaData.validatingCarriersData()[VC3] = vcxrData;

    setSpValidatingCxrGsaData(*farePath, &gsaData, {sp});

    XMLConstruct construct;
    _respFormatter->buildValidatingCarrierList( construct, *farePath, sp, defaultVcxr, inhibitPq );

    std::stringstream expectedXml;
    expectedXml << "<VCL P3L=\"F\" SM0=\"BSP\" VC0=\"T\">"
                << "<DCX B00=\"DC\" TT0=\"ETKTPREF\">"
                << "<PCX B00=\"P1\" VC1=\"PPR\"/>"
                << "<PCX B00=\"P2\" VC1=\"3PT\"/>"
                << "<PCX B00=\"P3\" VC1=\"STD\"/>"
                << "</DCX>"
                << "</VCL>";

    CPPUNIT_ASSERT_EQUAL(expectedXml.str(), construct.getXMLData());
  }

  void test_isAnyDefaultCarrierEmpty()
  {
    FarePath* fp = FarePathBuilder(&_memH).build();
    fp->defaultValCxrPerSp()["BSP"] = "AA";
    fp->defaultValCxrPerSp()["GTC"] = "";
    fp->defaultValCxrPerSp()["RUT"] = "CC";

    CPPUNIT_ASSERT( _respFormatter->isAnyDefaultVcxrEmpty( *fp ) );

    fp->defaultValCxrPerSp()["GTC"] = "BB";
    CPPUNIT_ASSERT( !_respFormatter->isAnyDefaultVcxrEmpty( *fp ) );
  }

  void setupBrand()
  {
    QualifiedBrand brandElement;
    brandElement.first = _memH.create<BrandProgram>();
    brandElement.second = _memH.create<BrandInfo>();

    brandElement.first->programCode() = "Program";
    brandElement.first->programName() = "VA Super Program";
    brandElement.second->brandName() = "Free Drinks";
    _pTrx->brandProgramVec().push_back(brandElement);
  }

  void testAddFareBrandDetailsPbbRequest()
  {
    setupBrand();
    createTravelSeg(1);
    _pTrx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);

    PaxTypeFareTest mPax;
    EXPECT_CALL(mPax, getValidBrandIndex(_, _, _)).Times(1).WillOnce(Return(0));

    _construct->openElement(xml2::FareCalcInformation);
    _formatter->addFareBrandDetails(*_pTrx, mPax, *_construct, Direction::BOTHWAYS);
    _construct->closeElement();

    std::string* xml = const_cast<std::string*>(&_construct->getXMLData());
    CPPUNIT_ASSERT(xml->find("SB2=\"FD\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("SB3=\"FREE DRINKS\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("SC0=\"PROGRAM\"") != std::string::npos);
    CPPUNIT_ASSERT(xml->find("SC2=\"VA SUPER PROGRAM\"") != std::string::npos);
  }

  void testAddFareBrandDetailsBrandProgramEpmpty()
  {
    createTravelSeg(1);
    _pTrx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);

    PaxTypeFareTest mPax;
    EXPECT_CALL(mPax, getValidBrandIndex(_, _, _)).Times(1).WillOnce(Return(0));

    _construct->openElement(xml2::FareCalcInformation);
    _formatter->addFareBrandDetails(*_pTrx, mPax, *_construct, Direction::BOTHWAYS);
    _construct->closeElement();

    std::string* xml = const_cast<std::string*>(&_construct->getXMLData());
    CPPUNIT_ASSERT(xml->find("SB2") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("SB3") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("SC0") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("SC2") == std::string::npos);
  }

  void testAddFareBrandDetailsNotPbbRequest()
  {
    setupBrand();
    createTravelSeg(1);
    _pTrx->setPbbRequest(NOT_PBB_RQ);

    PaxTypeFareTest mPax;
    _construct->openElement(xml2::FareCalcInformation);
    _formatter->addFareBrandDetails(*_pTrx, mPax, *_construct, Direction::BOTHWAYS);
    _construct->closeElement();

    std::string* xml = const_cast<std::string*>(&_construct->getXMLData());
    CPPUNIT_ASSERT(xml->find("SB2") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("SB3") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("SC0") == std::string::npos);
    CPPUNIT_ASSERT(xml->find("SC2") == std::string::npos);
  }

  void testPrepareMaxPenaltyResponse1()
  {
    MaxPenaltyResponse response;
    XMLConstruct xml;
    PricingResponseFormatter prf;
    FarePath* farePath = _memH.insert(new FarePath);
    prf.prepareMaxPenaltyResponse(*_pTrx, *farePath, response, DateTime::localTime(), xml);
    CPPUNIT_ASSERT_EQUAL(std::string("<PEN>"
                                     "<CPB NON=\"T\"/>"
                                     "<CPA NON=\"T\"/>"
                                     "<RPB NON=\"T\"/>"
                                     "<RPA NON=\"T\"/>"
                                     "</PEN>"),
                         xml.getXMLData());
  }

  void testPrepareMaxPenaltyResponse2()
  {
    MaxPenaltyResponse response;
    response._changeFees._before = MaxPenaltyResponse::Fee{{}, true};
    response._changeFees._after = MaxPenaltyResponse::Fee{{}, true};
    response._changeFees._cat16 = smp::BOTH;
    response._refundFees._before = MaxPenaltyResponse::Fee{Money(100.0, "USD"), false};
    response._refundFees._after = MaxPenaltyResponse::Fee{Money(150.0, "USD"), true};
    XMLConstruct xml;
    PricingResponseFormatter prf;
    FarePath* farePath = _memH.insert(new FarePath);
    prf.prepareMaxPenaltyResponse(*_pTrx, *farePath, response, DateTime::localTime(), xml);
    CPPUNIT_ASSERT_EQUAL(std::string("<PEN>"
                                     "<CPB NON=\"T\" C16=\"T\"/>"
                                     "<CPA NON=\"T\" C16=\"T\"/>"
                                     "<RPB MPA=\"100.00\" MPC=\"USD\"/>"
                                     "<RPA MPA=\"150.00\" MPC=\"USD\" NON=\"T\"/>"
                                     "</PEN>"),
                         xml.getXMLData());
  }

  void buildValidFare(Fare* fare)
  {
    fare->status().set(Fare::FS_ScopeIsDefined);
    fare->setMissingFootnote(false);
    fare->setFareInfo(_memH.create<FareInfo>());
    fare->setTariffCrossRefInfo(_memH.create<TariffCrossRefInfo>());
    fare->setCat15SecurityValid(true);
    fare->setGlobalDirectionValid(true);
    fare->setCalcCurrForDomItinValid(true);
    fare->setFareNotValidForDisplay(false);
  }

  void buildValidPaxTypeFare(PaxTypeFare* ptf)
  {
    ptf->actualPaxType() = _memH.create<PaxType>();
    ptf->fareClassAppInfo() = _memH.create<FareClassAppInfo>();
    ptf->fareClassAppSegInfo() = _memH.create<FareClassAppSegInfo>();
    ptf->setFare(_memH.create<Fare>());
    buildValidFare(ptf->fare());
    for (unsigned int ci = 1; ci < 36; ++ci)
    {
      ptf->setCategoryProcessed(ci);
      ptf->setCategoryValid(ci);
    }
    ptf->setIsMipUniqueFare(true);
    ptf->setRoutingValid(true);
    ptf->setIsValidForBranding(true);
    ptf->bookingCodeStatus().set(PaxTypeFare::BKS_PASS);
    ptf->setRequestedFareBasisInvalid(false);
  }

  void buildTravelSegments(std::vector<TravelSeg*>& container, int orderMin, int orderMax)
  {
    TravelSeg::RequestedFareBasis rfb {"FARECLASSCODE", "ADT", "VEND", "CAR", "12", "RULE", 999.99};
    for (int ti = orderMin; ti <= orderMax; ++ti)
    {
      TravelSeg* ts = _memH.create<AirSeg>();
      ts->requestedFareBasis().push_back(rfb);
      ts->segmentOrder() = ti;
      container.push_back(ts);
    }
  }

  void buildEssentials(PricingTrx* trx)
  {
    trx->fareCalcConfig() = _memH.insert(new FareCalcConfig);
    trx->setExcTrxType(PricingTrx::NOT_EXC_TRX);
    buildTravelSegments(trx->travelSeg(), 1, 6);
    for (int fi = 1; fi <= 3; ++fi)
    {
      FareMarket* fm = _memH.create<FareMarket>();
      buildTravelSegments(fm->travelSeg(), fi, fi+3);
      for (int pi = 0; pi < 10; ++pi)
      {
        PaxTypeFare* ptf = _memH.create<PaxTypeFare>();
        buildValidPaxTypeFare(ptf);
        ptf->fareMarket() = fm;
        fm->allPaxTypeFare().push_back(ptf);
      }
      trx->fareMarket().push_back(fm);
    }
    TrxUtil::enableAbacus();
  }

  void testAbacusWPwithSpecificFBC_DefaultErrorResponse()
  {
    ErrorResponseException::ErrorResponseCode errCode;
    PricingTrx* trx = TransactionBuilder<PricingTrx>(&_memH).build();
    XMLConstruct xml;
    std::string msgText = "DUMMY";

    buildEssentials(trx);
    errCode = ErrorResponseException::NO_FARES_RBD_CARRIER;
    _formatter->prepareErrorMessage(*trx, xml, errCode, msgText);
    CPPUNIT_ASSERT_EQUAL(std::string("<MSG N06=\"X\" Q0K=\"3\" S18=\"NO FARES/RBD/CARRIER\"/>"),
                         xml.getXMLData());
  }

  void testAbacusWPwithSpecificFBC_AllFaresFailOnRules()
  {
    ErrorResponseException::ErrorResponseCode errCode;
    PricingTrx* trx = TransactionBuilder<PricingTrx>(&_memH).build();
    XMLConstruct xml;
    std::string msgText = "DUMMY";

    buildEssentials(trx);
    errCode = ErrorResponseException::SFB_RULE_VALIDATION_FAILED;
    for (FareMarket* fm : trx->fareMarket())
      for (PaxTypeFare* ptf : fm->allPaxTypeFare())
      {
        ptf->setRequestedFareBasisInvalid(true);
      }
    _formatter->prepareErrorMessage(*trx, xml, errCode, msgText);
    CPPUNIT_ASSERT_EQUAL(std::string("<MSG N06=\"X\" Q0K=\"3\" S18=\"ATTN* RULE VALIDATION FAILED,"
                                     " TRY COMMAND PRICING WPQ\"/>"), xml.getXMLData());
  }

  void testAbacusWPwithSpecificFBC_BookingCodeValidationFailed()
  {
    ErrorResponseException::ErrorResponseCode errCode;
    PricingTrx* trx = TransactionBuilder<PricingTrx>(&_memH).build();
    XMLConstruct xml;
    std::string msgText = "DUMMY";

    buildEssentials(trx);
    errCode = ErrorResponseException::SFB_RULE_VALIDATION_FAILED;
    for (FareMarket* fm : trx->fareMarket())
      for (PaxTypeFare* ptf : fm->allPaxTypeFare())
      {
        ptf->bookingCodeStatus().setNull();
        ptf->bookingCodeStatus().set(PaxTypeFare::BKS_FAIL);
      }
    _formatter->prepareErrorMessage(*trx, xml, errCode, msgText);
    CPPUNIT_ASSERT_EQUAL(std::string("<MSG N06=\"X\" Q0K=\"3\" S18=\"ATTN* RULE VALIDATION FAILED,"
                                     " TRY COMMAND PRICING WPQ\"/>"), xml.getXMLData());
  }

  void testAbacusWPwithSpecificFBC_FareBasisNotAvailable()
  {
    ErrorResponseException::ErrorResponseCode errCode;
    PricingTrx* trx = TransactionBuilder<PricingTrx>(&_memH).build();
    XMLConstruct xml;
    std::string msgText = "DUMMY";

    buildEssentials(trx);
    errCode = ErrorResponseException::FARE_BASIS_NOT_AVAIL;
    trx->fareMarket().back()->allPaxTypeFare().clear();
    _formatter->prepareErrorMessage(*trx, xml, errCode, msgText);
    CPPUNIT_ASSERT_EQUAL(std::string("<MSG N06=\"X\" Q0K=\"3\" S18=\"$FORMAT$ FARE BASIS NOT "
                                     "AVAILABLE\"/>"), xml.getXMLData());
  }

  void test_constructElementVCC_WithNoDec()
  {
    XMLConstruct xml;
    const CarrierCode valCxr("AA");
    const MoneyAmount commAmt(100.12);
    const CurrencyNoDec noDec = 0;
    call_constructElementVCC(xml, valCxr, commAmt, noDec);
    CPPUNIT_ASSERT_EQUAL(std::string("<VCC B00=\"AA\" C58=\"100\"/>"), xml.getXMLData());
  }

  void test_constructElementVCC_WithDec()
  {
    XMLConstruct xml;
    const CarrierCode valCxr("AA");
    const MoneyAmount commAmt(100.12);
    const CurrencyNoDec noDec = 2;
    call_constructElementVCC(xml, valCxr, commAmt, noDec);
    CPPUNIT_ASSERT_EQUAL(std::string("<VCC B00=\"AA\" C58=\"100.12\"/>"), xml.getXMLData());
  }

  void test_prepareMarkupAndCommissionAmount_WithMarkUpAndCommission()
  {
    XMLConstruct xml;
    const CurrencyNoDec noDec = 2;

    FarePath* farePath = _memH.insert(new FarePath);
    CPPUNIT_ASSERT(farePath != nullptr);
    farePath->commissionAmount() = 100;

    CollectedNegFareData negFareData;
    negFareData.markUpAmount() = 10;
    farePath->collectedNegFareData() = &negFareData;

    xml.openElement(xml2::PassengerInfo);
    _formatter->prepareMarkupAndCommissionAmount(*_pTrx, xml, *farePath, noDec);
    xml.closeElement();
    CPPUNIT_ASSERT_EQUAL(std::string("<PXI C57=\"10.00\" C58=\"90.00\"/>"), xml.getXMLData());
  }

  void test_prepareMarkupAndCommissionAmount_WithMarkUpOnly()
  {
    XMLConstruct xml;
    const CurrencyNoDec noDec = 2;

    FarePath* farePath = _memH.insert(new FarePath);
    CPPUNIT_ASSERT(farePath != nullptr);
    farePath->commissionAmount() = 100.00;

    CollectedNegFareData negFareData;
    negFareData.markUpAmount() = 100.00;
    farePath->collectedNegFareData() = &negFareData;

    xml.openElement(xml2::PassengerInfo);
    _formatter->prepareMarkupAndCommissionAmount(*_pTrx, xml, *farePath, noDec);
    xml.closeElement();
    CPPUNIT_ASSERT_EQUAL(std::string("<PXI C57=\"100.00\" C58=\"0.00\"/>"), xml.getXMLData());
  }

  void test_prepareMarkupAndCommissionAmount_WithCommissionOnly()
  {
    XMLConstruct xml;
    const CurrencyNoDec noDec = 2;

    FarePath* farePath = _memH.insert(new FarePath);
    CPPUNIT_ASSERT(farePath != nullptr);
    farePath->commissionAmount() = 100;

    CollectedNegFareData negFareData;
    farePath->collectedNegFareData() = &negFareData;

    xml.openElement(xml2::PassengerInfo);
    _formatter->prepareMarkupAndCommissionAmount(*_pTrx, xml, *farePath, noDec);
    xml.closeElement();
    CPPUNIT_ASSERT_EQUAL(std::string("<PXI C57=\"0.00\" C58=\"100.00\"/>"), xml.getXMLData());
  }

  void test_prepareMarkupAndCommissionAmount_WithIncorrectMarkUp()
  {
    XMLConstruct xml;
    const CurrencyNoDec noDec = 2;

    FarePath* farePath = _memH.insert(new FarePath);
    CPPUNIT_ASSERT(farePath != nullptr);
    farePath->commissionAmount() = 100;

    CollectedNegFareData negFareData;
    negFareData.markUpAmount() = 100.50;
    farePath->collectedNegFareData() = &negFareData;

    xml.openElement(xml2::PassengerInfo);
    _formatter->prepareMarkupAndCommissionAmount(*_pTrx, xml, *farePath, noDec);
    xml.closeElement();

    CPPUNIT_ASSERT_EQUAL(std::string("<PXI C57=\"100.50\"/>"), xml.getXMLData());
  }

  void test_prepareMarkupAndCommissionAmount_WithMarkUpNoComm()
  {
    XMLConstruct xml;
    const CurrencyNoDec noDec = 2;

    FarePath* farePath = _memH.insert(new FarePath);
    CPPUNIT_ASSERT(farePath != nullptr);
    farePath->commissionAmount() = 0;

    CollectedNegFareData negFareData;
    negFareData.markUpAmount() = 100;
    farePath->collectedNegFareData() = &negFareData;

    xml.openElement(xml2::PassengerInfo);
    _formatter->prepareMarkupAndCommissionAmount(*_pTrx, xml, *farePath, noDec);
    xml.closeElement();

    CPPUNIT_ASSERT_EQUAL(std::string("<PXI C57=\"100.00\"/>"), xml.getXMLData());
  }

  void test_prepareCommissionForValidatingCarriers_NoAgencyCommission()
  {
    XMLConstruct xml;
    const CurrencyNoDec noDec = 2;

    FarePath* farePath = _memH.insert(new FarePath);
    CPPUNIT_ASSERT(farePath != nullptr);

    call_prepareCommissionForValidatingCarriers(xml, *farePath, noDec);
    CPPUNIT_ASSERT_EQUAL(std::string(""), xml.getXMLData());
  }

  void test_prepareCommissionForValidatingCarriers_OneAgencyCommission()
  {
    XMLConstruct xml;
    const CurrencyNoDec noDec = 2;

    FarePath* farePath = _memH.insert(new FarePath);
    CPPUNIT_ASSERT(farePath != nullptr);
    farePath->defaultValidatingCarrier()="AA";
    farePath->storeCommissionForValidatingCarrier("AA", 10);

    call_prepareCommissionForValidatingCarriers(xml, *farePath, noDec);
    CPPUNIT_ASSERT_EQUAL(std::string("<VCC B00=\"AA\" C58=\"10.00\"/>"), xml.getXMLData());
  }

  void test_prepareCommissionForValidatingCarriers_OneAgencyCommission_withKP()
  {
    XMLConstruct xml;
    const CurrencyNoDec noDec = 2;
    _pAgent->agentCommissionType() = "KP10";
    FarePath* farePath = _memH.insert(new FarePath);
    CPPUNIT_ASSERT(farePath != nullptr);
    farePath->defaultValidatingCarrier()="AA";
    farePath->storeCommissionForValidatingCarrier("AA", 10);
    xml.openElement(xml2::PassengerInfo);
    _pTrx->setValidatingCxrGsaApplicable(true);

    call_prepareCommissionForValidatingCarriers(xml, *farePath, noDec);
    CPPUNIT_ASSERT_EQUAL(std::string("<PXI Q0V=\"174\"><VCC B00=\"AA\" C58=\"10.00\"/>"), xml.getXMLData());
  }

  void test_prepareCommissionForValidatingCarriers_TwoSameAgencyCommission()
  {
    XMLConstruct xml;
    const CurrencyNoDec noDec = 2;

    FarePath* farePath = _memH.insert(new FarePath);
    CPPUNIT_ASSERT(farePath != nullptr);
    farePath->defaultValidatingCarrier()="AA";
    farePath->storeCommissionForValidatingCarrier("AA", 10);
    farePath->storeCommissionForValidatingCarrier("BA", 10);

    call_prepareCommissionForValidatingCarriers(xml, *farePath, noDec);
    CPPUNIT_ASSERT_EQUAL(std::string("<VCC B00=\"AA\" C58=\"10.00\"/><VCC B00=\"BA\" C58=\"10.00\"/>"), xml.getXMLData());
  }

  void test_prepareCommissionForValidatingCarriers_TwoSameAgencyCommission_withKP()
  {
    XMLConstruct xml;
    const CurrencyNoDec noDec = 2;
    _pAgent->agentCommissionType() = "KP10";

    FarePath* farePath = _memH.insert(new FarePath);
    CPPUNIT_ASSERT(farePath != nullptr);
    farePath->defaultValidatingCarrier()="AA";
    farePath->storeCommissionForValidatingCarrier("AA", 10);
    farePath->storeCommissionForValidatingCarrier("BA", 10);
    xml.openElement(xml2::PassengerInfo);
    _pTrx->setValidatingCxrGsaApplicable(true);

    call_prepareCommissionForValidatingCarriers(xml, *farePath, noDec);
    CPPUNIT_ASSERT_EQUAL(std::string("<PXI Q0V=\"174\">"\
                                     "<VCC B00=\"AA\" C58=\"10.00\"/>"\
                                     "<VCC B00=\"BA\" C58=\"10.00\"/>"), xml.getXMLData());
  }

  void test_prepareCommissionForValidatingCarriers_TwoDiffAgencyCommission()
  {
    XMLConstruct xml;
    const CurrencyNoDec noDec = 2;

    FarePath* farePath = _memH.insert(new FarePath);
    CPPUNIT_ASSERT(farePath != nullptr);
    farePath->defaultValidatingCarrier()="BA";
    farePath->storeCommissionForValidatingCarrier("AA", 10);
    farePath->storeCommissionForValidatingCarrier("BA", 20);
    farePath->validatingCarriers().push_back("AA");

    xml.openElement(xml2::PassengerInfo);
    _pTrx->setValidatingCxrGsaApplicable(true);
    call_prepareCommissionForValidatingCarriers(xml, *farePath, noDec);
    xml.closeElement();
    std::string expRes =
    "<PXI><VCC B00=\"BA\" C58=\"20.00\"/><VCC B00=\"AA\" C58=\"10.00\"/></PXI>";

    CPPUNIT_ASSERT_EQUAL(expRes, xml.getXMLData());
  }

  void test_prepareCommissionForValidatingCarriers_TwoDiffAgencyCommission_withKP()
  {
    XMLConstruct xml;
    const CurrencyNoDec noDec = 2;
    _pAgent->agentCommissionType() = "KP10";

    FarePath* farePath = _memH.insert(new FarePath);
    CPPUNIT_ASSERT(farePath != nullptr);
    farePath->defaultValidatingCarrier()="BA";
    farePath->storeCommissionForValidatingCarrier("AA", 10);
    farePath->storeCommissionForValidatingCarrier("BA", 20);
    farePath->validatingCarriers().push_back("AA");

    xml.openElement(xml2::PassengerInfo);
    _pTrx->setValidatingCxrGsaApplicable(true);
    call_prepareCommissionForValidatingCarriers(xml, *farePath, noDec);
    xml.closeElement();

    std::string expRes =
    "<PXI Q0V=\"174\"><VCC B00=\"BA\" C58=\"20.00\"/><VCC B00=\"AA\" C58=\"10.00\"/></PXI>";

    CPPUNIT_ASSERT_EQUAL(expRes, xml.getXMLData());
  }

  void test_prepareCommissionForValidatingCarriers_TwoDiffAgencyWithSameCommission()
  {
    XMLConstruct xml;
    const CurrencyNoDec noDec = 2;

    FarePath* farePath = _memH.insert(new FarePath);
    CPPUNIT_ASSERT(farePath != nullptr);
    farePath->defaultValidatingCarrier()="BA";
    farePath->storeCommissionForValidatingCarrier("AA", 10);
    farePath->storeCommissionForValidatingCarrier("BA", 20);
    farePath->storeCommissionForValidatingCarrier("UA", 10);
    farePath->validatingCarriers().push_back("AA");
    farePath->validatingCarriers().push_back("UA");

    xml.openElement(xml2::PassengerInfo);
    _pTrx->setValidatingCxrGsaApplicable(true);
    call_prepareCommissionForValidatingCarriers(xml, *farePath, noDec);
    xml.closeElement();

    std::string expRes =
    "<PXI><VCC B00=\"BA\" C58=\"20.00\"/>"\
    "<VCC B00=\"AA\" C58=\"10.00\"/>"\
    "<VCC B00=\"UA\" C58=\"10.00\"/></PXI>";

    CPPUNIT_ASSERT_EQUAL(expRes, xml.getXMLData());
  }

  void test_prepareCommissionForValidatingCarriers_TwoDiffAgencyWithSameCommission_withKP()
  {
    XMLConstruct xml;
    const CurrencyNoDec noDec = 2;
    _pAgent->agentCommissionType() = "KP10";

    FarePath* farePath = _memH.insert(new FarePath);
    CPPUNIT_ASSERT(farePath != nullptr);
    farePath->defaultValidatingCarrier()="BA";
    farePath->storeCommissionForValidatingCarrier("AA", 10);
    farePath->storeCommissionForValidatingCarrier("BA", 20);
    farePath->storeCommissionForValidatingCarrier("UA", 10);

    xml.openElement(xml2::PassengerInfo);
    _pTrx->setValidatingCxrGsaApplicable(true);
    call_prepareCommissionForValidatingCarriers(xml, *farePath, noDec);
    xml.closeElement();

    std::string expRes =
    "<PXI Q0V=\"174\"><VCC B00=\"BA\" C58=\"20.00\"/>"\
    "<VCC B00=\"AA\" C58=\"10.00\"/>"\
    "<VCC B00=\"UA\" C58=\"10.00\"/></PXI>";

    CPPUNIT_ASSERT_EQUAL(expRes, xml.getXMLData());
  }

  void testPrepareAdjustedCalcTotalWithORG()
  {
    CalcTotals calcTotalsADJ;
    _calcTotals->adjustedCalcTotal = &calcTotalsADJ;

    _pOptions->setPDOForFRRule(true);
    _formatter->prepareAdjustedCalcTotal(*_pTrx, *_construct, *_calcTotals);

    CPPUNIT_ASSERT(_construct->getXMLData().find(
      "<SFD TYP=\"ADS\"/>" ) != std::string::npos);

    AdjustedSellingDiffInfo a1("ADJT AMT", "J", "100.00");
    _calcTotals->adjustedCalcTotal->adjustedSellingDiffInfo.push_back(a1);

    _formatter->prepareAdjustedCalcTotal(*_pTrx, *_construct, *_calcTotals);

   CPPUNIT_ASSERT(_construct->getXMLData().find(
     "<SFD TYP=\"ADS\"><HPS T52=\"J\" N52=\"ADJT AMT\" C52=\"100.00\"/></SFD>")
        != std::string::npos);
  }

  void testPrepareAdjustedCalcTotalWithoutORG()
  {
    CalcTotals calcTotalsADJ;
    _calcTotals->adjustedCalcTotal = &calcTotalsADJ;

    _pOptions->setPDOForFRRule(false);

    PaxTypeCode pt("ADT");
    FarePath* fp = FarePathBuilder(&_memH).withPaxTypeCode(pt).build();
    FarePath* adjFp = FarePathBuilder(&_memH).withPaxTypeCode(pt).build();
    adjFp->calculationCurrency() = "USD";
    adjFp->setTotalNUCAmount(888.88);

    _calcTotals->farePath = fp;
    _calcTotals->equivCurrencyCode = "USD";
    _calcTotals->adjustedCalcTotal->farePath = adjFp;
    _calcTotals->adjustedCalcTotal->convertedBaseFareCurrencyCode = "USD";
    _calcTotals->adjustedCalcTotal->equivCurrencyCode = "USD";
    _calcTotals->adjustedCalcTotal->convertedBaseFare = 888.88;
    _calcTotals->adjustedCalcTotal->setTaxAmount(111.11);

    _formatter->prepareAdjustedCalcTotal(*_pTrx, *_construct, *_calcTotals);

    CPPUNIT_ASSERT(_construct->getXMLData() ==
      "<SFD TYP=\"ADS\" C5E=\"888.88\" C5A=\"888.88\" C66=\"999.99\" C65=\"111.11\"/>");

    AdjustedSellingDiffInfo a1("ADJT AMT", "J", "100.00");
    _calcTotals->adjustedCalcTotal->adjustedSellingDiffInfo.push_back(a1);

    _formatter->prepareAdjustedCalcTotal(*_pTrx, *_construct, *_calcTotals);
    CPPUNIT_ASSERT(_construct->getXMLData().find(
      "<SFD TYP=\"ADS\" C5E=\"888.88\" C5A=\"888.88\" C66=\"999.99\" C65=\"111.11\"><HPS T52=\"J\" N52=\"ADJT AMT\" C52=\"100.00\"/></SFD>")
        != std::string::npos);
  }

  void testPrepareAdjustedCalcTotalWithTax()
  {
    CalcTotals calcTotalsADJ;
    _calcTotals->adjustedCalcTotal = &calcTotalsADJ;

    _pOptions->setPDOForFRRule(false);

    PaxTypeCode pt("ADT");
    FarePath* fp = FarePathBuilder(&_memH).withPaxTypeCode(pt).build();
    FarePath* adjFp = FarePathBuilder(&_memH).withPaxTypeCode(pt).build();
    adjFp->calculationCurrency() = "USD";
    adjFp->setTotalNUCAmount(888.88);

    _calcTotals->farePath = fp;
    _calcTotals->equivCurrencyCode = "USD";
    _calcTotals->adjustedCalcTotal->farePath = adjFp;
    _calcTotals->adjustedCalcTotal->convertedBaseFareCurrencyCode = "USD";
    _calcTotals->adjustedCalcTotal->equivCurrencyCode = "USD";
    _calcTotals->adjustedCalcTotal->convertedBaseFare = 888.88;
    _calcTotals->adjustedCalcTotal->setTaxAmount(111.11);

    TaxRecord t1;
    t1.setTaxAmount(10);
    t1.taxNoDec() = 2;
    t1.taxCurrencyCode() = "USD";
    t1._publishedAmount = 15;
    t1._publishedNoDec = 2;
    t1._publishedCurrencyCode = "USD";

    TaxResponse taxResponse;
    taxResponse.taxRecordVector().push_back(&t1);

    FareCalc::FcTaxInfo& fcTaxInfo =  _calcTotals->adjustedCalcTotal->getMutableFcTaxInfo();
    FareCalcConfig* fcConfig = FareCalcConfigBuilder(&_memH).build();
    fcTaxInfo.initialize(_pTrx, _calcTotals->adjustedCalcTotal, fcConfig, &taxResponse);

    _formatter->prepareAdjustedCalcTotal(*_pTrx, *_construct, *_calcTotals);

    CPPUNIT_ASSERT(_construct->getXMLData() ==
      "<SFD TYP=\"ADS\" C5E=\"888.88\" C5A=\"888.88\" C66=\"898.88\" C65=\"10.00\"><TAX C6B=\"10.00\" C40=\"USD\" C6A=\"15.00\" C41=\"USD\"/></SFD>");
  }

  void testPrepareAdjustedCalcTotalDiffCurrency()
  {
    CalcTotals calcTotalsADJ;
    _calcTotals->adjustedCalcTotal = &calcTotalsADJ;

    _pOptions->setPDOForFRRule(false);

    PaxTypeCode pt("ADT");
    FarePath* fp = FarePathBuilder(&_memH).withPaxTypeCode(pt).build();
    FarePath* adjFp = FarePathBuilder(&_memH).withPaxTypeCode(pt).build();
    adjFp->calculationCurrency() = "USD";
    adjFp->setTotalNUCAmount(888.88);

    _calcTotals->farePath = fp;
    _calcTotals->equivCurrencyCode = "GBP";
    _calcTotals->adjustedCalcTotal->farePath = adjFp;
    _calcTotals->adjustedCalcTotal->convertedBaseFareCurrencyCode = "USD";
    _calcTotals->adjustedCalcTotal->equivCurrencyCode = "GBP";
    _calcTotals->adjustedCalcTotal->equivFareAmount = 333.33;
    _calcTotals->adjustedCalcTotal->convertedBaseFare = 777.77;
    _calcTotals->adjustedCalcTotal->setTaxAmount(111.11);

    _formatter->prepareAdjustedCalcTotal(*_pTrx, *_construct, *_calcTotals);

    CPPUNIT_ASSERT(_construct->getXMLData() ==
      "<SFD TYP=\"ADS\" C5E=\"888.88\" C5A=\"777.77\" C5F=\"333.33\" C66=\"444.44\" C65=\"111.11\"/>");
  }

  void testPrepareHPSItemsSingleItem()
  {
    _formatter->prepareHPSItems(*_pTrx, *_construct, *_calcTotals);
    CPPUNIT_ASSERT(_construct->getXMLData().find("ADJT AMT") == std::string::npos);

    AdjustedSellingDiffInfo a1("ADJT AMT", "J", "100.00");
    _calcTotals->adjustedSellingDiffInfo.push_back(a1);

    _formatter->prepareHPSItems(*_pTrx, *_construct, *_calcTotals);

    CPPUNIT_ASSERT(_construct->getXMLData().find(
      "<HPS T52=\"J\" N52=\"ADJT AMT\" C52=\"100.00\"/>") != std::string::npos);
  }

  void testPrepareHPSItemsDualItems()
  {
    _formatter->prepareHPSItems(*_pTrx, *_construct, *_calcTotals);
    CPPUNIT_ASSERT(_construct->getXMLData().find("ADJT AMT") == std::string::npos);

    AdjustedSellingDiffInfo a1("ADJT AMT", "J", "100.00");
    _calcTotals->adjustedSellingDiffInfo.push_back(a1);

    AdjustedSellingDiffInfo a2("A2", "G", "88.88");
    _calcTotals->adjustedSellingDiffInfo.push_back(a2);

    _formatter->prepareHPSItems(*_pTrx, *_construct, *_calcTotals);
    CPPUNIT_ASSERT(_construct->getXMLData().find(
      "<HPS T52=\"J\" N52=\"ADJT AMT\" C52=\"100.00\"/><HPS T52=\"G\" N52=\"A2\" C52=\"88.88\"/>")
        != std::string::npos);
  }

  void testPrepareHPSItemsMultiItems()
  {
    _formatter->prepareHPSItems(*_pTrx, *_construct, *_calcTotals);
    CPPUNIT_ASSERT(_construct->getXMLData().find("ADJT AMT") == std::string::npos);

    AdjustedSellingDiffInfo a1("ADJT AMT", "J", "100.00");
    _calcTotals->adjustedSellingDiffInfo.push_back(a1);

    AdjustedSellingDiffInfo a2("A2", "G", "88.88");
    _calcTotals->adjustedSellingDiffInfo.push_back(a2);

    AdjustedSellingDiffInfo a3("X2", "G", "7.7");
    _calcTotals->adjustedSellingDiffInfo.push_back(a3);

    _formatter->prepareHPSItems(*_pTrx, *_construct, *_calcTotals);
    CPPUNIT_ASSERT(_construct->getXMLData().find(
      "<HPS T52=\"J\" N52=\"ADJT AMT\" C52=\"100.00\"/><HPS T52=\"G\" N52=\"A2\" C52=\"88.88\"/><HPS T52=\"G\" N52=\"X2\" C52=\"7.7\"/>")
        != std::string::npos);
  }

  void testGetCommissionSourceIndicator_AMC()
  {
    FarePath* farePath = _memH.insert(new FarePath);
    CPPUNIT_ASSERT(farePath != nullptr);
    farePath->defaultValidatingCarrier()="AA";
    farePath->storeCommissionForValidatingCarrier("AA", 10);
    _calcTotals->farePath = farePath;

    CPPUNIT_ASSERT_EQUAL(_formatter->getCommissionSourceIndicator(*_pTrx, *_calcTotals), 'A');
  }

  void testGetCommissionSourceIndicator_Cat35CommissionPercent()
  {
    FarePath* farePath = _memH.insert(new FarePath);
    CPPUNIT_ASSERT(farePath != nullptr);
    CollectedNegFareData negFareData;
    negFareData.indicatorCat35() = true;
    negFareData.comPercent() = 9;
    farePath->collectedNegFareData() = &negFareData;
    _calcTotals->farePath = farePath;

    CPPUNIT_ASSERT_EQUAL(_formatter->getCommissionSourceIndicator(*_pTrx, *_calcTotals), 'C');
  }

  void testGetCommissionSourceIndicator_Cat35CommissionAmount_withKP()
  {
    FarePath* farePath = _memH.insert(new FarePath);
    CPPUNIT_ASSERT(farePath != nullptr);
    CollectedNegFareData negFareData;
    negFareData.indicatorCat35() = true;
    negFareData.comAmount() = 10.00;
    farePath->collectedNegFareData() = &negFareData;
    _calcTotals->farePath = farePath;
    _pAgent->agentCommissionType() = "K10";

    CPPUNIT_ASSERT_EQUAL(_formatter->getCommissionSourceIndicator(*_pTrx, *_calcTotals), 'C');
  }

  void testGetCommissionSourceIndicator_Manual()
  {
    _pAgent->agentCommissionType() = "KP10";

    CPPUNIT_ASSERT_EQUAL(_formatter->getCommissionSourceIndicator(*_pTrx, *_calcTotals), 'M');
  }

  void testGetCommissionSourceIndicator_NoCommission()
  {
    CPPUNIT_ASSERT_EQUAL(_formatter->getCommissionSourceIndicator(*_pTrx, *_calcTotals), ' ');
  }

  void mockPrepareSegments()
  {
    FarePath fp;
    FareUsage fu;
    PricingUnit pu;
    uint16_t segmentOrder = 1;
    AirSeg as;
    as.departureDT() = DateTime(2020, 2, 2, 12, 59);
    as.arrivalDT() = DateTime(2020, 2, 3, 18, 01);
    fu.travelSeg().push_back(&as);
    _calcTotals->farePath = &fp;
    fp.itin() = _itin;
    _itin->travelSeg().push_back(&as);
    PaxTypeFare ptf;
    fu.paxTypeFare() = &ptf;
    FareMarket fm;
    ptf.fareMarket() = &fm;
    _calcTotals->bkgCodeSegStatus.resize(2);
    _calcTotals->bkgCodeSegStatus[1].set(PaxTypeFare::BKSS_PASS, true);
    _calcTotals->fcConfig = FareCalcConfigBuilder(&_memH).build();
    _pTrx->altTrxType() = PricingTrx::WP;
    _pRequest->majorSchemaVersion() = 9;
    _pRequest->minorSchemaVersion() = 9;
    _pRequest->revisionSchemaVersion() = 9;
    _respFormatter->prepareSegments(*_pTrx, *_calcTotals, fu, fp, pu, segmentOrder, 2,
                                    *_construct);
  }

  void testprepareSegments_WithServiceFee()
  {
    _pOptions->setServiceFeesTemplateRequested();
    mockPrepareSegments();
    CPPUNIT_ASSERT(std::string::npos != _construct->getXMLData().find(
        " D71=\"2020-02-02\" D72=\"1259\" D73=\"2020-02-03\" D74=\"1801\""));
  }

  void testprepareSegments_WithoutServiceFee()
  {
    mockPrepareSegments();
    CPPUNIT_ASSERT_EQUAL(std::string::npos, _construct->getXMLData().find(" D72="));
    CPPUNIT_ASSERT_EQUAL(std::string::npos, _construct->getXMLData().find(" D73="));
    CPPUNIT_ASSERT_EQUAL(std::string::npos, _construct->getXMLData().find(" D74="));
  }

  class MyDataHandle : public DataHandleMock
  {
  private:
    TestMemHandle _memHandle;

  public:
    const std::vector<BankerSellRate*>&
    getBankerSellRate(const CurrencyCode& primeCur, const CurrencyCode& cur, const DateTime& date)
    {
      std::vector<BankerSellRate*>* ret = _memHandle.create<std::vector<BankerSellRate*> >();
      BankerSellRate* bsr = _memHandle.create<BankerSellRate>();
      bsr->primeCur() = primeCur;
      bsr->cur() = cur;
      bsr->rateType() = 'B';
      bsr->agentSine() = "FXR";
      if (primeCur == "USD" && cur == "EUR")
      {
        bsr->rate() = 0.8;
        bsr->rateNodec() = 2;
      }
      else
      {
        bsr->rate() = 0.1;
        bsr->rateNodec() = 1;
      }

      ret->push_back(bsr);
      return *ret;
    }

    const LocCode getMultiTransportCity(const LocCode& locCode)
    {
      static LocCode codes[] = { "DFW", "LON" };
      static LocCode* endCodes = codes + sizeof(codes) / sizeof(LocCode);

      if (std::find(codes, endCodes, locCode) != endCodes)
        return locCode;

      return DataHandleMock::getMultiTransportCity(locCode);
    }

    const TaxText* getTaxText(const VendorCode& vendor, int itemNo)
    {
      TaxText* taxText = _memHandle.create<TaxText>();
      taxText->txtMsgs().push_back("//01/0F3");
      return taxText;
    }

    const std::vector<SubCodeInfo*>& getSubCode(const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const ServiceTypeCode& serviceTypeCode,
                                                const ServiceGroup& serviceGroup,
                                                const DateTime& date)
    {
      std::vector<SubCodeInfo*>* subCodes = _memHandle.create<std::vector<SubCodeInfo*> >();
      SubCodeInfo* s5 = _memHandle.create<SubCodeInfo>();
      SubCodeInfo::dummyData(*s5);
      s5->vendor() = ATPCO_VENDOR_CODE;
      s5->serviceTypeCode() = "OC";
      s5->serviceGroup() = "BG";
      s5->carrier() = "EF";
      s5->serviceSubTypeCode() = "0F3";
      s5->fltTktMerchInd() = BAGGAGE_CHARGE;
      subCodes->push_back(s5);
      return *subCodes;
    }

    const ServicesDescription* getServicesDescription(const ServiceGroupDescription& value)
    {
      if (value == "01" || value == "02")
      {
        ServicesDescription* description = _memHandle.create<ServicesDescription>();
        description->value() = value;
        description->description() = "DESCRIPTION " + value;
        return description;
      }
      else
        return DataHandleMock::getServicesDescription(value);
    }

    const ServicesSubGroup*
    getServicesSubGroup(const ServiceGroup& serviceGroup, const ServiceGroup& serviceSubGroup)
    {
      if (!serviceGroup.empty() && !serviceSubGroup.empty())
      {
        ServicesSubGroup* subGroup = _memHandle.create<ServicesSubGroup>();
        if (serviceGroup == "GGG" && serviceSubGroup == "SSS")
        {
          subGroup->definition() = "DEFINITION";
        }
        return subGroup;
      }
      else
        return DataHandleMock::getServicesSubGroup(serviceGroup, serviceSubGroup);
    }

    const PaxTypeInfo* getPaxType(const PaxTypeCode& paxTypeCode, const VendorCode& vendor)
    {
      PaxTypeInfo* paxTypeInfo = _memHandle.create<PaxTypeInfo>();
      return paxTypeInfo;
    }

    const Loc* getLoc(const LocCode& locCode, const DateTime& date)
    {
      return _memHandle.create<Loc>();
    }
  };
};
CPPUNIT_TEST_SUITE_REGISTRATION(PricingResponseFormatterTest);

} // tse
