#include <string>
#include <vector>

#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxSpecConfigReg.h"
#include "DBAccess/MultiAirportCity.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"

namespace tse
{

class MirrorImageTest : public CppUnit::TestFixture
{

private:
  CPPUNIT_TEST_SUITE(MirrorImageTest);
  CPPUNIT_TEST(testAirportMirrorNormal);
  CPPUNIT_TEST(testNationMirrorSurface);
  CPPUNIT_TEST(testMirrorOnStartSegment);
  CPPUNIT_TEST(testCityMirrorCheckPositive);
  CPPUNIT_TEST(testCityMirrorCheckNegative);
  CPPUNIT_TEST(testCityMirrorDontCheck);
  CPPUNIT_TEST_SUITE_END();

  class LocalDataHandle : public DataHandleMock
  {
  public:
    ~LocalDataHandle() { _memHandle.clear(); }

    const std::vector<MultiAirportCity*>& getMultiAirportCity(const LocCode& city)
    {
      std::vector<MultiAirportCity*>& cities =
          *_memHandle.create<std::vector<MultiAirportCity*> >();

      if (city == "EWR")
        cities.push_back(createMultiAirportCity(city, "JFK"));

      return cities;
    }

  private:
    TestMemHandle _memHandle;

    MultiAirportCity* createMultiAirportCity(std::string airport, std::string city)
    {
      MultiAirportCity* multiAirportCity = _memHandle.create<MultiAirportCity>();
      multiAirportCity->airportCode() = LocCode(airport);
      multiAirportCity->city() = LocCode(city);
      return multiAirportCity;
    }
  };

public:
  void setUp()
  {
    _localDataHandle = _memHandle.create<LocalDataHandle>();
    _mirrorImage = _memHandle.create<MirrorImage>();
    _taxCodeReg = _memHandle.create<TaxCodeReg>();

    _date = _memHandle.create<DateTime>();
    _pricingRequest = _memHandle.create<PricingRequest>();
    _pricingRequest->ticketingDT() = *_date;
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_pricingRequest);

    _itin = _memHandle.create<Itin>();

    _fp = _memHandle.create<FarePath>();
    _fp->itin() = _itin;

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->farePath() = _fp;

    _taxSpecConfigRegs = _memHandle.create<std::vector<TaxSpecConfigReg*> >();

    _startIndex = _memHandle.create<uint16_t>();

    _JFK = createLoc("JFK", "US");
    _EWR = createLoc("EWR", "US");
    _DFW = createLoc("DFW", "US");
    _LIM = createLoc("LIM", "PE");

    _JFKtoLIM = createSegment(_JFK, _LIM);
    _LIMtoJFK = createSegment(_LIM, _JFK);

    _JFKtoDFW = createSegment(_JFK, _DFW);
    _JFKtoEWR = createSegment(_JFK, _EWR);
    _EWRtoDFW = createSegment(_EWR, _DFW);
    _DFWtoEWR = createSegment(_DFW, _EWR);
  }

  void tearDown() { _memHandle.clear(); }

  /********************************* TEST CASES: ********************************/
  void testMirrorOnStartSegment()
  {
    _itin->travelSeg().push_back(_LIMtoJFK);
    _itin->travelSeg().push_back(_JFKtoLIM);
    _itin->travelSeg().push_back(_JFKtoDFW);
    *_startIndex = 0;
    bool result = _mirrorImage->isMirrorImage(*_trx, *_taxResponse, *_taxCodeReg, *_startIndex);

    CPPUNIT_ASSERT(result == false);
  }

  void testAirportMirrorNormal()
  {
    _itin->travelSeg().push_back(_LIMtoJFK);
    _itin->travelSeg().push_back(_JFKtoLIM);
    *_startIndex = 1;
    bool result = _mirrorImage->isMirrorImage(*_trx, *_taxResponse, *_taxCodeReg, *_startIndex);

    CPPUNIT_ASSERT(result == true);
  }

  void testNationMirrorSurface()
  {
    _itin->travelSeg().push_back(_LIMtoJFK);
    _itin->travelSeg().push_back(_JFKtoLIM);
    _itin->travelSeg().push_back(_JFKtoDFW);
    *_startIndex = 2;
    bool result = _mirrorImage->isMirrorImage(*_trx, *_taxResponse, *_taxCodeReg, *_startIndex);

    CPPUNIT_ASSERT(result == true);
  }

  void testCityMirrorCheckPositive()
  {
    setCityMirrorPositiveItin();
    addUtcParam("CHECKCITYMIRROR", "Y");
    bool result = _mirrorImage->isMirrorImage(*_trx, *_taxResponse, *_taxCodeReg, *_startIndex);

    CPPUNIT_ASSERT(result == true);
  }

  void testCityMirrorCheckNegative()
  {
    setCityMirrorNegativeItin();
    addUtcParam("CHECKCITYMIRROR", "Y");
    bool result = _mirrorImage->isMirrorImage(*_trx, *_taxResponse, *_taxCodeReg, *_startIndex);

    CPPUNIT_ASSERT(result == false);
  }

  void testCityMirrorDontCheck()
  {
    setCityMirrorPositiveItin();
    // no UTC param setting
    bool result = _mirrorImage->isMirrorImage(*_trx, *_taxResponse, *_taxCodeReg, *_startIndex);

    CPPUNIT_ASSERT(result == false);
  }

  /************************** SPECIAL CONFIGURATION: ****************************/
  void setCityMirrorPositiveItin()
  {
    _itin->travelSeg().clear();
    _itin->travelSeg().push_back(_JFKtoDFW);
    _itin->travelSeg().push_back(_DFWtoEWR);
    *_startIndex = 1;
  }

  void setCityMirrorNegativeItin()
  {
    _itin->travelSeg().clear();
    _itin->travelSeg().push_back(_JFKtoEWR);
    _itin->travelSeg().push_back(_EWRtoDFW);
    *_startIndex = 1;
  }

  void addUtcParam(std::string paramName, std::string paramValue)
  {
    TaxSpecConfigReg::TaxSpecConfigRegSeq* taxSpecConfigRegSeq =
        new TaxSpecConfigReg::TaxSpecConfigRegSeq(); // pointer used by TaxSpecConfigReg destructor

    taxSpecConfigRegSeq->paramName() = paramName;
    taxSpecConfigRegSeq->paramValue() = paramValue;

    TaxSpecConfigReg* taxSpecConfigReg = _memHandle.create<TaxSpecConfigReg>();

    taxSpecConfigReg->effDate() = *_date;
    taxSpecConfigReg->effDate() = *_date;
    taxSpecConfigReg->discDate() = *_date;

    taxSpecConfigReg->seqs().push_back(taxSpecConfigRegSeq);

    _taxSpecConfigRegs->push_back(taxSpecConfigReg);
    _mirrorImage = _memHandle.create<MirrorImage>(*_date, _taxSpecConfigRegs);
  }

  /*************************** GENERAL CONFIGURATION: ***************************/
  AirSeg* createSegment(Loc* loc1, Loc* loc2)
  {
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->origin() = loc1;
    seg->destination() = loc2;
    return seg;
  }

  Loc* createLoc(std::string locCode, std::string nationCode)
  {
    Loc* loc = _memHandle.create<Loc>();
    loc->loc() = LocCode(locCode);
    loc->nation() = NationCode(nationCode);
    return loc;
  }

protected:
  // TESTS METADATA:
  TestMemHandle _memHandle;
  LocalDataHandle* _localDataHandle;

  // GENERAL CONFIGURATION:
  MirrorImage* _mirrorImage;
  TaxCodeReg* _taxCodeReg;

  DateTime* _date;
  PricingRequest* _pricingRequest;
  PricingTrx* _trx;
  TaxResponse* _taxResponse;
  FarePath* _fp;
  Itin* _itin;
  uint16_t* _startIndex;

  Loc* _JFK;
  Loc* _LIM;
  Loc* _EWR;
  Loc* _DFW;

  AirSeg* _JFKtoLIM;
  AirSeg* _LIMtoJFK;
  AirSeg* _JFKtoDFW;
  AirSeg* _JFKtoEWR;
  AirSeg* _EWRtoDFW;
  AirSeg* _DFWtoEWR;

  // SPECIAL CONFIGURATION:
  std::vector<TaxSpecConfigReg*>* _taxSpecConfigRegs;
};
CPPUNIT_TEST_SUITE_REGISTRATION(MirrorImageTest);
}
