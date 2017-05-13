//----------------------------------------------------------------------------
//  Copyright Sabre 2016
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

#include "Common/SpanishResidentFaresEnhancementUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/SpanishReferenceFareInfo.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"
#include "Pricing/YFlexiValidator.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{
class SRFMock : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const std::vector<SpanishReferenceFareInfo*>&
  getSpanishReferenceFare(const CarrierCode& tktCarrier,
                          const CarrierCode& fareCarrier,
                          const LocCode& sourceLoc,
                          const LocCode& destLoc,
                          const DateTime& date)
  {
    if(tktCarrier == "IB")
    {
      std::vector<SpanishReferenceFareInfo*>* srfInfos =
          _memHandle.create<std::vector<SpanishReferenceFareInfo*>>();

      SpanishReferenceFareInfo* srfInfo1 = _memHandle.create<SpanishReferenceFareInfo>();
      srfInfo1->get<tag::SCurrencyCode>() = "EUR";
      srfInfo1->get<tag::FareAmount>() = 95.00;
      srfInfo1->get<tag::ViaPointOneAirport>() = "AGP";
      srfInfos->push_back(srfInfo1);

      SpanishReferenceFareInfo* srfInfo2 = _memHandle.create<SpanishReferenceFareInfo>();
      srfInfo2->get<tag::SCurrencyCode>() = "EUR";
      srfInfo2->get<tag::FareAmount>() = 84.50;
      srfInfo2->get<tag::ViaPointOneAirport>() = "PMI";
      srfInfos->push_back(srfInfo2);
      return *srfInfos;
    }
    else if (tktCarrier == "AU")
    {
      return *_memHandle.create<std::vector<SpanishReferenceFareInfo*>>();
    }
    return DataHandleMock::getSpanishReferenceFare(tktCarrier, fareCarrier, sourceLoc, destLoc, date);
  }
};
class YFlexiValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(YFlexiValidatorTest);

  CPPUNIT_TEST(testValidateVIApointsWhenVia1);
  CPPUNIT_TEST(testValidateVIApointsWhenVia2);
  CPPUNIT_TEST(testValidateVIApointsWhenNoVia);

  CPPUNIT_TEST(testGetAmount);
  CPPUNIT_TEST(testSetMaxAmount);
  CPPUNIT_TEST(testUpdDiscAmountBoundary);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<SRFMock>();
    _trx = _memHandle.create<PricingTrx>();
    _calcCurrCode = _memHandle.create<CurrencyCode>();
    *_calcCurrCode = "EUR";
    _puPath = _memHandle.create<PUPath>();
    _yFlexi = _memHandle.create<YFlexiValidator>(*_trx, *_puPath, *_calcCurrCode);
    _agp = _memHandle.create<Loc>();
    _agp->loc() = "AGP";
    _pmi = _memHandle.create<Loc>();
    _pmi->loc() = "PMI";
    _mad = _memHandle.create<Loc>();
    _mad->loc() = "MAD";
    _segAGP = _memHandle.create<AirSeg>();
    _segAGP->destination() = _agp;
    _segPMI = _memHandle.create<AirSeg>();
    _segPMI->destination() = _pmi;
    _segMAD = _memHandle.create<AirSeg>();
    _segMAD->destination() = _mad;
    _tvlSegs = _memHandle.create<std::vector<TravelSeg*>>();
    _validatingCarrier = _memHandle.create<CarrierCode>();
    *_validatingCarrier = "AU";
    _fp = _memHandle.create<FarePath>();
    _ptf = _memHandle.create<PaxTypeFare>();
    _fu = _memHandle.create<FareUsage>();
  }

  void tearDown() { _memHandle.clear(); }

  void testValidateVIApointsWhenVia1()
  {
    SpanishReferenceFareInfo srfInfo;

    _tvlSegs->push_back(_segAGP);
    _tvlSegs->push_back(_segPMI);

    srfInfo.get<tag::ViaPointOneAirport>() = _agp->loc();
    srfInfo.get<tag::ViaPointTwoAirport>() = "";

    CPPUNIT_ASSERT(_yFlexi->validateVIApoints(srfInfo, *_tvlSegs));
  }

  void testValidateVIApointsWhenVia2()
  {
    SpanishReferenceFareInfo srfInfo;

    _tvlSegs->push_back(_segAGP);
    _tvlSegs->push_back(_segPMI);

    srfInfo.get<tag::ViaPointOneAirport>() = "";
    srfInfo.get<tag::ViaPointTwoAirport>() = _agp->loc();

    CPPUNIT_ASSERT(!_yFlexi->validateVIApoints(srfInfo, *_tvlSegs));
  }

  void testValidateVIApointsWhenNoVia()
  {
    SpanishReferenceFareInfo srfInfo;

    _tvlSegs->push_back(_segAGP);
    _tvlSegs->push_back(_segPMI);

    srfInfo.get<tag::ViaPointOneAirport>() = _agp->loc();
    srfInfo.get<tag::ViaPointTwoAirport>() = _pmi->loc();

    CPPUNIT_ASSERT(!_yFlexi->validateVIApoints(srfInfo, *_tvlSegs));
  }

  void prepareDataForGetAmount(CarrierCode govCarrier)
  {
    if(govCarrier == "IB")
    {
      _fm = _memHandle.create<FareMarket>();
      _fm->governingCarrier() = "IB";
      _fm->origin() = _pmi;
      _fm->destination() = _agp;
      _fm->travelSeg().push_back(_segPMI);
      _fm->travelSeg().push_back(_segAGP);
      _fm->validatingCarriers().push_back("IB");
    }
    else if (govCarrier == "AU")
    {
      _fm = _memHandle.create<FareMarket>();
      _fm->governingCarrier() = "AU";
      _fm->origin() = _agp;
      _fm->destination() = _mad;
      _fm->travelSeg().push_back(_segAGP);
      _fm->travelSeg().push_back(_segMAD);
      _fm->validatingCarriers().push_back("IB");
    }
  }

  void testGetAmount()
  {
    prepareDataForGetAmount("IB");
    *_validatingCarrier = "IB";

    MoneyAmount expectedAmount = 84.50;
    MoneyAmount amount = _yFlexi->getAmount(*_fm, *_validatingCarrier);
    CPPUNIT_ASSERT((amount > expectedAmount - EPSILON) && (amount < expectedAmount + EPSILON));
  }

  void testSetMaxAmount()
  {
    MergedFareMarket mfm;

    prepareDataForGetAmount("IB");
    mfm.mergedFareMarket().push_back(_fm);

    prepareDataForGetAmount("AU");
    mfm.mergedFareMarket().push_back(_fm);

    MoneyAmount expectedAmount = 95.00;
    MoneyAmount maxAmount = _yFlexi->setMaxAmount(mfm, "IB");
    CPPUNIT_ASSERT((maxAmount > expectedAmount - EPSILON) &&
                   (maxAmount < expectedAmount + EPSILON));
  }

  void testUpdDiscAmountBoundary()
  {
    PU pu;
    pu.fareDirectionality().push_back(FROM);
    pu.fareDirectionality().push_back(TO);

    MergedFareMarket mfm1, mfm2;

    prepareDataForGetAmount("IB");
    mfm1.mergedFareMarket().push_back(_fm);

    prepareDataForGetAmount("AU");
    mfm1.mergedFareMarket().push_back(_fm);

    prepareDataForGetAmount("IB");
    mfm2.mergedFareMarket().push_back(_fm);

    pu.fareMarket().push_back(&mfm1);
    pu.fareMarket().push_back(&mfm2);

    _puPath->puPath().push_back(&pu);

    CPPUNIT_ASSERT(_yFlexi->updDiscAmountBoundary());
    CPPUNIT_ASSERT(_puPath->getSpanishResidentAmount().size() == 4);
    MoneyAmount val = _puPath->findSpanishResidentAmount(*mfm1.mergedFareMarket()[0],
                                                          ANY_CARRIER,
                                                          "IB");
    CPPUNIT_ASSERT_EQUAL(95.00, val);

    val = _puPath->findSpanishResidentAmount(*mfm1.mergedFareMarket()[1], "AU", "IB");
    CPPUNIT_ASSERT_EQUAL(95.00, val);

    val = _puPath->findSpanishResidentAmount(*mfm1.mergedFareMarket()[1], ANY_CARRIER, "IB");
    CPPUNIT_ASSERT_EQUAL(95.00, val);

    val = _puPath->findSpanishResidentAmount(*mfm1.mergedFareMarket()[0], "IB", "IB");
    CPPUNIT_ASSERT_EQUAL(84.50, val);
  }

private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  CurrencyCode* _calcCurrCode;
  PUPath* _puPath;
  YFlexiValidator* _yFlexi;
  Loc* _agp;
  Loc* _pmi;
  Loc* _mad;
  AirSeg* _segAGP;
  AirSeg* _segPMI;
  AirSeg* _segMAD;
  std::vector<TravelSeg*>* _tvlSegs;
  FareMarket* _fm;
  CarrierCode* _validatingCarrier;
  FarePath* _fp;
  FareUsage* _fu;
  PaxTypeFare* _ptf;
};
CPPUNIT_TEST_SUITE_REGISTRATION(YFlexiValidatorTest);
} // end tse
