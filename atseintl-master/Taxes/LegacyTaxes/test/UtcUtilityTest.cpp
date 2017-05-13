#include <string>
#include <iostream>
#include <vector>
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConverter.h"
#include "Common/DateTime.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/Fare.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxNation.h"
#include "DBAccess/TaxSpecConfigReg.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/MockTseServer.h"
#include "test/DBAccessMock/DataHandleMock.h"

#include "Taxes/LegacyTaxes/UtcUtility.h"

using namespace std;
using namespace tse::utc;

namespace tse
{
namespace utc
{

namespace
{

class Loc1TransferTypeMock : public Loc1TransferType
{
public:
  Loc1TransferTypeMock() {}

  bool isInLoc(const Loc& loc, const DateTime& ticketingDT) { return true; }

  void collectErrors(PricingTrx& trx,
                     TaxCodeReg& taxCodeReg,
                     TaxResponse& taxResponse,
                     const std::string& info)
  {
  }
};
}

class UtcUtilityTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(UtcUtilityTest);
  CPPUNIT_TEST(testLoc1TransferTypeEquipEmpty);
  CPPUNIT_TEST(testLoc1TransferTypeEquipTRN_TGV);
  CPPUNIT_TEST(testLoc1TransferTypeEquipTRN_BUS_TGV);
  CPPUNIT_TEST(testLoc1TransferTypeEquip_firstSegment);
  CPPUNIT_TEST(testLoc1TransferTypeEquip_arunkSecondSegment);
  CPPUNIT_TEST(testLoc1TransferTypeEquip);
  CPPUNIT_TEST(testValidateLocs_locExempted);
  CPPUNIT_TEST(testValidateLocs);
  CPPUNIT_TEST(testValidatePrevFltLocs_firstSegment);
  CPPUNIT_TEST(testValidatePrevFltLocs_arunkSecondSegment);
  CPPUNIT_TEST(testValidatePrevFltLocs);
  CPPUNIT_TEST(testValidateFareTypes_fareTypeNotSet);
  CPPUNIT_TEST(testValidateFareTypes_fareTypeSet);
  CPPUNIT_TEST(testValidateFareTypes_DOM_FARE);
  CPPUNIT_TEST(testValidateFareTypes_FDOM_FARE);
  CPPUNIT_TEST(testValidateFareTypes_INT_FARE);
  CPPUNIT_TEST(testValidateFareTypes_RDOM_FARE_fail);
  CPPUNIT_TEST(testValidateFareTypes_RDOM_FARE_pass);

  CPPUNIT_TEST_SUITE_END();

public:
  void testLoc1TransferTypeEquipEmpty() { CPPUNIT_ASSERT(!_loc1TransferType->validateEquip(BUS)); }

  void testLoc1TransferTypeEquipTRN_TGV()
  {
    std::string equipments = "TRN TGV";
    _loc1TransferType->setEquipments(equipments);
    CPPUNIT_ASSERT(!_loc1TransferType->validateEquip(BUS));
  }

  void testLoc1TransferTypeEquipTRN_BUS_TGV()
  {
    std::string equipments = "TRN " + BUS + " TGV";
    _loc1TransferType->setEquipments(equipments);
    CPPUNIT_ASSERT(_loc1TransferType->validateEquip(BUS));
  }

  void testLoc1TransferTypeEquip_firstSegment()
  {
    uint16_t startIndex = 0;
    CPPUNIT_ASSERT(
        !_loc1TransferType->validatePrevFltEquip(*_trx, *_taxCodeReg, *_taxResponse, startIndex));
  }

  void testLoc1TransferTypeEquip_arunkSecondSegment()
  {
    uint16_t startIndex = 2;
    CPPUNIT_ASSERT(
        !_loc1TransferType->validatePrevFltEquip(*_trx, *_taxCodeReg, *_taxResponse, startIndex));
  }

  void testLoc1TransferTypeEquip()
  {
    uint16_t startIndex = 3;
    CPPUNIT_ASSERT(
        _loc1TransferType->validatePrevFltEquip(*_trx, *_taxCodeReg, *_taxResponse, startIndex));
  }

  void testValidateLocs_locExempted()
  {
    //_loc1TransferType->_prevFltLocCodeExclInd = tse::YES;
    // CPPUNIT_ASSERT( !_loc1TransferType->validateLocs( *_trx, _travelSeg));
  }

  void testValidateLocs()
  {
    //_loc1TransferType->_prevFltLocCodeExclInd = tse::NO;
    // CPPUNIT_ASSERT( _loc1TransferType->validateLocs( *_trx, _travelSeg));
  }

  void testValidatePrevFltLocs_firstSegment()
  {
    uint16_t startIndex = 0;
    CPPUNIT_ASSERT(
        !_loc1TransferType->validatePrevFltLocs(*_trx, *_taxCodeReg, *_taxResponse, startIndex));
  }

  void testValidatePrevFltLocs_arunkSecondSegment()
  {
    uint16_t startIndex = 2;
    CPPUNIT_ASSERT(
        _loc1TransferType->validatePrevFltLocs(*_trx, *_taxCodeReg, *_taxResponse, startIndex));
  }

  void testValidatePrevFltLocs()
  {
    uint16_t startIndex = 3;
    CPPUNIT_ASSERT(
        _loc1TransferType->validatePrevFltLocs(*_trx, *_taxCodeReg, *_taxResponse, startIndex));
  }

  void testValidateFareTypes_fareTypeNotSet() { CPPUNIT_ASSERT(!FareType().mustBeValidate()); }

  void testValidateFareTypes_fareTypeSet()
  {
    FareType fareType;
    std::string types = "DOM";
    fareType.setFareType(types);
    CPPUNIT_ASSERT(fareType.mustBeValidate());
  }

  void testValidateFareTypes_DOM_FARE()
  {
    FareUsage fu;
    PaxTypeFare ptf;
    Fare fare;
    fare.status().set(Fare::FS_Domestic);

    PaxType actualPaxType;
    FareMarket fareMarket;
    ptf.initialize(&fare, &actualPaxType, &fareMarket);
    fu.paxTypeFare() = &ptf;

    FareType fareType;

    std::string types = "DOM";
    fareType.setFareType(types);

    CPPUNIT_ASSERT(fareType.validateFareTypes(fu));
  }
  void testValidateFareTypes_FDOM_FARE()
  {
    FareUsage fu;
    PaxTypeFare ptf;
    Fare fare;
    fare.status().set(Fare::FS_ForeignDomestic);

    PaxType actualPaxType;
    FareMarket fareMarket;
    ptf.initialize(&fare, &actualPaxType, &fareMarket);
    fu.paxTypeFare() = &ptf;

    FareType fareType;

    std::string types = "DOM FDOM";
    fareType.setFareType(types);

    CPPUNIT_ASSERT(fareType.validateFareTypes(fu));
  }
  void testValidateFareTypes_INT_FARE()
  {
    FareUsage fu;
    PaxTypeFare ptf;
    Fare fare;
    fare.status().set(Fare::FS_International);

    PaxType actualPaxType;
    FareMarket fareMarket;
    ptf.initialize(&fare, &actualPaxType, &fareMarket);
    fu.paxTypeFare() = &ptf;

    FareType fareType;

    std::string types = "DOM FDOM";
    fareType.setFareType(types);

    CPPUNIT_ASSERT(!fareType.validateFareTypes(fu));
  }
  void testValidateFareTypes_RDOM_FARE_fail()
  {
    FareUsage fu;
    PaxTypeFare ptf;
    Fare fare;
    NationCode nation("SC");
    Loc loc;
    loc.nation() = "DE";
    fare.status().set(Fare::FS_ForeignDomestic);

    PaxType actualPaxType;
    FareMarket fareMarket;
    fareMarket.origin() = &loc;
    fareMarket.destination() = &loc;
    ptf.initialize(&fare, &actualPaxType, &fareMarket);
    fu.paxTypeFare() = &ptf;

    FareType fareType;

    std::string types = "RDOM";
    fareType.setFareType(types);

    CPPUNIT_ASSERT(!fareType.validateFareTypes(fu, nation));
  }
  void testValidateFareTypes_RDOM_FARE_pass()
  {
    FareUsage fu;
    PaxTypeFare ptf;
    Fare fare;
    NationCode nation("SC");
    Loc loc;
    loc.nation() = "SC";
    fare.status().set(Fare::FS_ForeignDomestic);

    PaxType actualPaxType;
    FareMarket fareMarket;
    fareMarket.origin() = &loc;
    fareMarket.destination() = &loc;
    ptf.initialize(&fare, &actualPaxType, &fareMarket);
    fu.paxTypeFare() = &ptf;

    FareType fareType;

    std::string types = "RDOM";
    fareType.setFareType(types);

    CPPUNIT_ASSERT(fareType.validateFareTypes(fu, nation));
  }

  Loc1TransferType* _loc1TransferType;

  PricingTrx* _trx;
  PricingRequest* _request;
  TaxCodeReg* _taxCodeReg;
  TaxResponse* _taxResponse;
  Loc* _origin;
  Loc* _destination;
  TravelSeg* _travelSeg;
  TravelSeg* _arunkSeg;
  Itin* _itin;
  FarePath* _farePath;

  void setUp()
  {
    _loc1TransferType = new Loc1TransferTypeMock();
    _trx = new PricingTrx();
    _request = new PricingRequest();
    _taxCodeReg = new TaxCodeReg();
    _taxResponse = new TaxResponse();
    _origin = new Loc();
    _destination = new Loc();
    _travelSeg = new AirSeg();
    _arunkSeg = new ArunkSeg();
    _itin = new Itin();
    _farePath = new FarePath();

    _trx->setRequest(_request);
    _trx->getRequest()->ticketingDT() = DateTime(2011, 7, 14, 11, 0, 0);
    _origin->loc() = std::string("DFW");
    _origin->nation() = std::string("US");
    _destination->loc() = std::string("MIA");
    _destination->nation() = std::string("US");

    _farePath->itin() = _itin;
    _farePath->itin()->travelSeg().push_back(_travelSeg);
    _farePath->itin()->travelSeg().push_back(_arunkSeg);
    _farePath->itin()->travelSeg().push_back(_travelSeg);
    _farePath->itin()->travelSeg().push_back(_travelSeg);
    _taxResponse->farePath() = _farePath;
  }

  void tearDown()
  {

    delete _origin;
    delete _destination;
    delete _travelSeg;
    delete _arunkSeg;
    delete _taxCodeReg;
    delete _request;
    delete _trx;
    delete _loc1TransferType;
    delete _farePath;
    delete _itin;
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(UtcUtilityTest);
}
}
