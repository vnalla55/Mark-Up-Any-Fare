// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "DataModel/AncRequest.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Cabin.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/Loc.h"
#include "Xform/BaggageRequestHandler.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestDataBuilders.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class BaggageRequestBuilderDataHandleMock : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  BaggageRequestBuilderDataHandleMock() {}
  ~BaggageRequestBuilderDataHandleMock() {}

  const Cabin*
  getCabin(const CarrierCode& carrier, const BookingCode& classOfService, const DateTime& date)
  {
    return _memHandle.create<Cabin>();
  }

  const Loc* getLoc(const LocCode& locCode, const DateTime& date)
  {
    Loc* loc = _memHandle.create<Loc>();
    loc->loc() = locCode;
    return loc;
  }

  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    return "KRK";
  }

  const std::vector<const PaxTypeMatrix*>& getPaxTypeMatrix(const PaxTypeCode& paxTypeCode)
  {
    if (paxTypeCode == "ADT" || paxTypeCode == "CNN" || paxTypeCode == "NEG")
      return *_memHandle.create<std::vector<const PaxTypeMatrix*> >();
    return DataHandleMock::getPaxTypeMatrix(paxTypeCode);
  }

  const LocCode getMultiTransportCity(const LocCode& locCode)
  {
    if (locCode == "LGA")
      return "LGA";
    return DataHandleMock::getMultiTransportCity(locCode);
  }

  const std::vector<FareCalcConfig*>& getFareCalcConfig(const Indicator& userApplType,
                                                        const UserApplCode& userAppl,
                                                        const PseudoCityCode& pseudoCity)
  {
    std::vector<FareCalcConfig*>& fareCalcConfigs =
        *_memHandle.create<std::vector<FareCalcConfig*> >();
    fareCalcConfigs.push_back(_memHandle.create<FareCalcConfig>());
    return fareCalcConfigs;
  }

  bool getUtcOffsetDifference(const DSTGrpCode& dstgrp1,
                              const DSTGrpCode& dstgrp2,
                              short& utcOffset,
                              const DateTime& dateTime1,
                              const DateTime& dateTime2)
  {
    utcOffset = 0;
    return true;
  }
};

class BaggageRequestHandlerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaggageRequestHandlerTest);
  CPPUNIT_TEST(testOnStartHSP);
  CPPUNIT_TEST(testOnStartEQP);
  CPPUNIT_TEST(testonStartBaggageRequest_101);
  CPPUNIT_TEST(testonStartBaggageRequest_222);
  CPPUNIT_TEST(testDetectRtw_True);
  CPPUNIT_TEST(testDetectRtw_False);
  CPPUNIT_TEST_SUITE_END();

private:
  TestMemHandle _memHandle;
  BaggageRequestBuilderDataHandleMock* _dataHandleMock;
  BaggageTrx* _pricingTrx;
  Trx* _trx;
  AncRequest* _req;
  DataHandle _dataHandle;
  BaggageRequestHandler* _baggageRequestHandler;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _pricingTrx = _memHandle.create<BaggageTrx>();
    _pricingTrx->setRequest(_req = _memHandle(new AncRequest));
    _pricingTrx->setOptions(_memHandle(new PricingOptions));
    _baggageRequestHandler = _memHandle(new BaggageRequestHandler(_trx));
    _baggageRequestHandler->_pricingTrx = _pricingTrx;
    _baggageRequestHandler->_request = _req;
    _dataHandleMock = _memHandle.create<BaggageRequestBuilderDataHandleMock>();
  }

  void tearDown() { _memHandle.clear(); }

  void testOnStartHSP()
  {
    std::string request =
        "<BaggageRequest>"
        "<AGI A10=\"LGA\" C40=\"USD\"/>"
        "<ITN Q00=\"2\">"
        "<IRO>"
        "<PXI B70=\"ADT\" Q0U=\"1\"/>"
        "</IRO>"
        "<SGI Q00=\"1\">"
        "<FLI A01=\"LGA\" A02=\"MSP\" D01=\"2025-11-03\" D31=\"1115\" D02=\"2025-11-03\" "
        "D32=\"1640\" B00=\"AA\" B01=\"AA\" B30=\"Y\" Q0B=\"0321\" S95=\"S80\">"
        "<HSP A03=\"ORD\"/>"
        "</FLI>"
        "<FBA Q6D=\"1\"/>"
        "</SGI>"
        "<FBI Q00=\"1\" B02=\"AA\" C50=\"810.23\" B50=\"YA2AA\" S53=\"EU\" S37=\"ATP\" "
        "S90=\"2000\" FTY=\"00\"/>"
        "</ITN>"
        "</BaggageRequest>";

    _baggageRequestHandler->parse(_dataHandle, request);
    LocCode expectedLocCode = LocCode("ORD");

    CPPUNIT_ASSERT_EQUAL(
        expectedLocCode,
        static_cast<PricingTrx*>(_trx)->travelSeg().front()->hiddenStops().front()->loc());
  }

  void testOnStartEQP()
  {
    std::string request =
        "<BaggageRequest>"
        "<AGI A10=\"LGA\" C40=\"USD\"/>"
        "<ITN Q00=\"2\">"
        "<IRO>"
        "<PXI B70=\"ADT\" Q0U=\"1\"/>"
        "</IRO>"
        "<SGI Q00=\"1\">"
        "<FLI A01=\"LGA\" A02=\"MSP\" D01=\"2025-11-03\" D31=\"1115\" D02=\"2025-11-03\" "
        "D32=\"1640\" B00=\"AA\" B01=\"AA\" B30=\"Y\" Q0B=\"0321\" S95=\"S80\">"
        "<HSP A03=\"ORD\"/>"
        "<EQP S95=\"777\"/>"
        "</FLI>"
        "<FBA Q6D=\"1\"/>"
        "</SGI>"
        "<FBI Q00=\"1\" B02=\"AA\" C50=\"810.23\" B50=\"YA2AA\" S53=\"EU\" S37=\"ATP\" "
        "S90=\"2000\" FTY=\"00\"/>"
        "</ITN>"
        "</BaggageRequest>";

    _baggageRequestHandler->parse(_dataHandle, request);
    EquipmentType expectedEquipment1 = EquipmentType("S80");
    EquipmentType expectedEquipment2 = EquipmentType("777");

    CPPUNIT_ASSERT_EQUAL(
        expectedEquipment1,
        static_cast<PricingTrx*>(_trx)->travelSeg().front()->equipmentTypes().front());
    CPPUNIT_ASSERT_EQUAL(
        expectedEquipment2,
        static_cast<PricingTrx*>(_trx)->travelSeg().front()->equipmentTypes().back());
  }

  void testonStartBaggageRequest_101()
  {
    std::string request =
        "<BaggageRequest Version=\"1.0.1\">"
        "<AGI A10=\"LGA\" C40=\"USD\"/>"
        "<ITN Q00=\"2\">"
        "<IRO>"
        "<PXI B70=\"ADT\" Q0U=\"1\"/>"
        "</IRO>"
        "<SGI Q00=\"1\">"
        "<FLI A01=\"LGA\" A02=\"MSP\" D01=\"2025-11-03\" D31=\"1115\" D02=\"2025-11-03\" "
        "D32=\"1640\" B00=\"AA\" B01=\"AA\" B30=\"Y\" Q0B=\"0321\" S95=\"S80\">"
        "<HSP A03=\"ORD\"/>"
        "<EQP S95=\"777\"/>"
        "</FLI>"
        "<FBA Q6D=\"1\"/>"
        "</SGI>"
        "<FBI Q00=\"1\" B02=\"AA\" C50=\"810.23\" B50=\"YA2AA\" S53=\"EU\" S37=\"ATP\" "
        "S90=\"2000\" FTY=\"00\"/>"
        "</ITN>"
        "</BaggageRequest>";

    _baggageRequestHandler->parse(_dataHandle, request);

    CPPUNIT_ASSERT(static_cast<PricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT_EQUAL((uint16_t)1,
                         static_cast<PricingTrx*>(_trx)->getRequest()->majorSchemaVersion());
    CPPUNIT_ASSERT_EQUAL((uint16_t)0,
                         static_cast<PricingTrx*>(_trx)->getRequest()->minorSchemaVersion());
    CPPUNIT_ASSERT_EQUAL((uint16_t)1,
                         static_cast<PricingTrx*>(_trx)->getRequest()->revisionSchemaVersion());
  }

  void testonStartBaggageRequest_222()
  {
    std::string request =
        "<BaggageRequest Version=\"2.2.2\">"
        "<AGI A10=\"LGA\" C40=\"USD\"/>"
        "<ITN Q00=\"2\">"
        "<IRO>"
        "<PXI B70=\"ADT\" Q0U=\"1\"/>"
        "</IRO>"
        "<SGI Q00=\"1\">"
        "<FLI A01=\"LGA\" A02=\"MSP\" D01=\"2025-11-03\" D31=\"1115\" D02=\"2025-11-03\" "
        "D32=\"1640\" B00=\"AA\" B01=\"AA\" B30=\"Y\" Q0B=\"0321\" S95=\"S80\">"
        "<HSP A03=\"ORD\"/>"
        "<EQP S95=\"777\"/>"
        "</FLI>"
        "<FBA Q6D=\"1\"/>"
        "</SGI>"
        "<FBI Q00=\"1\" B02=\"AA\" C50=\"810.23\" B50=\"YA2AA\" S53=\"EU\" S37=\"ATP\" "
        "S90=\"2000\" FTY=\"00\"/>"
        "</ITN>"
        "</BaggageRequest>";

    _baggageRequestHandler->parse(_dataHandle, request);

    CPPUNIT_ASSERT(static_cast<PricingTrx*>(_trx)->getRequest());
    CPPUNIT_ASSERT_EQUAL((uint16_t)2,
                         static_cast<PricingTrx*>(_trx)->getRequest()->majorSchemaVersion());
    CPPUNIT_ASSERT_EQUAL((uint16_t)2,
                         static_cast<PricingTrx*>(_trx)->getRequest()->minorSchemaVersion());
    CPPUNIT_ASSERT_EQUAL((uint16_t)2,
                         static_cast<PricingTrx*>(_trx)->getRequest()->revisionSchemaVersion());
  }

  AncRequest::AncFareBreakAssociation* createFba(SequenceNumber farecompId)
  {
    AncRequest::AncFareBreakAssociation* fba = _memHandle(new AncRequest::AncFareBreakAssociation);
    fba->fareComponentID() = farecompId;
    return fba;
  }

  void prepareRtw(std::initializer_list<SequenceNumber> fareCompId)
  {
    AirSegBuilder bld(_memHandle);
    Itin* itin = _memHandle(new Itin);
    itin->travelSeg().push_back(bld.withLocs("AAA", "BBB").build());
    itin->travelSeg().push_back(bld.withLocs("BBB", "AAA").build());
    _pricingTrx->itin().push_back(itin);
    std::vector<AncRequest::AncFareBreakAssociation*>& fba =
        _req->fareBreakAssociationPerItin()[itin];
    for (SequenceNumber fcid : fareCompId)
      fba.push_back(createFba(fcid));
  }

  void testDetectRtw_True()
  {
    prepareRtw({1,1});
    _baggageRequestHandler->detectRtw();
    CPPUNIT_ASSERT(_pricingTrx->getOptions()->isRtw());
  }

  void testDetectRtw_False()
  {
    prepareRtw({1,2});
    _baggageRequestHandler->detectRtw();
    CPPUNIT_ASSERT(!_pricingTrx->getOptions()->isRtw());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BaggageRequestHandlerTest);
}
