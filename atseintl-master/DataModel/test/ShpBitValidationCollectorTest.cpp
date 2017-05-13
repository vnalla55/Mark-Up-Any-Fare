// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
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

#include "Common/Global.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/ShpBitValidationCollector.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/ItinIndex.h"
#include "DBAccess/DataHandle.h"
#include "Routing/MileageInfo.h"

#include "test/include/CppUnitHelperMacros.h"

#include <memory>
#include <vector>

namespace tse
{

class ShpBitValidationCollectorDerived : public ShpBitValidationCollector
{
public:
  class FMValidationSharedDataDerived : public ShpBitValidationCollector::FMValidationSharedData
  {
  public:
    BitValidationShareMap& getSharedData()
    {
      return ShpBitValidationCollector::FMValidationSharedData::getSharedData();
    };
    SharedElements& getBitSharedData(uint32_t bit)
    {
      return ShpBitValidationCollector::FMValidationSharedData::getBitSharedData(bit);
    };
  };

  CxrBitValidationShareMap& getCxrValidationShareMap()
  {
    return ShpBitValidationCollector::getCxrValidationShareMap();
  }
  FMValidationSharedDataDerived*
  getFMSharedData(const ItinIndex::Key carrierKey, const FareMarket* fareMarket)
  {
    return (FMValidationSharedDataDerived*)ShpBitValidationCollector::getFMSharedData(carrierKey,
                                                                                      fareMarket);
  }
};

class ShpBitValidationCollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ShpBitValidationCollectorTest);
  CPPUNIT_TEST(testEmptyShareCollector);
  CPPUNIT_TEST(testGetFMSharedDataEmpty);
  CPPUNIT_TEST(testGetFMSharedDataOneElement_Outbound);
  CPPUNIT_TEST(testGetFMSharedDataOneElement_OutboundInbound);
  CPPUNIT_TEST(testGetFMSharedDataOneElement_OutboundInbound_Empty);
  CPPUNIT_TEST(testGetFMSharedDataTwoElements_OutboundInbound);
  CPPUNIT_TEST(testGetFMSharedDataTwoFMTwoElements_OutboundInbound);
  CPPUNIT_TEST(testGetFMSharedDataTwoCxrsOneElements_OutboundInbound);
  CPPUNIT_TEST_SUITE_END();

private:
  typedef std::shared_ptr<FareMarket> FareMarketPtr;
  typedef std::vector<FareMarketPtr> FMVector;

  static const uint16_t LocNumber = 4;
  static const uint16_t CxrNumber = 2;

  FMVector _fmVector;
  DataHandle _dataHandle;
  uint32_t _cxr[CxrNumber];
  CarrierCode _cxrCode[CxrNumber];
  std::vector<Loc*> _loc;
  ShpBitValidationCollectorDerived* _collector;

private:
  FareMarketPtr newFareMarket(CarrierCode& cxr, const Loc* origin, const Loc* destination)
  {
    FareMarketPtr fm(new FareMarket());
    fm->origin() = origin;
    fm->destination() = destination;
    fm->governingCarrier() = cxr;
    fm->boardMultiCity() = origin->loc();
    fm->offMultiCity() = destination->loc();
    return fm;
  }

  void createFareMarkets()
  {
    for (uint16_t indOrig = 0; indOrig < LocNumber; ++indOrig)
    {
      for (uint16_t indDest = indOrig + 1; indDest < LocNumber; ++indDest)
      {
        _fmVector.push_back(newFareMarket(_cxrCode[0], _loc[indOrig], _loc[indDest]));
        _fmVector.push_back(newFareMarket(_cxrCode[1], _loc[indOrig], _loc[indDest]));
      }
    }
  }

  void addLocationToCollection(const std::string city)
  {
    Loc* location = _dataHandle.create<Loc>();
    location->loc() = city;
    _loc.push_back(location);
  }

public:
  ShpBitValidationCollectorTest() : _collector(0)
  {
    addLocationToCollection("JFK");
    addLocationToCollection("DFW");
    addLocationToCollection("LAX");
    addLocationToCollection("SFA");

    ItinIndex::Key cxrKey;
    ShoppingUtil::createCxrKey("DL", cxrKey);
    _cxrCode[0] = "DL";
    _cxr[0] = cxrKey;
    ShoppingUtil::createCxrKey("LH", cxrKey);
    _cxrCode[1] = "LH";
    _cxr[1] = cxrKey;

    createFareMarkets();
  }

  void setUp() { _collector = new ShpBitValidationCollectorDerived; }

  void tearDown()
  {
    delete _collector;
    _collector = 0;
    _dataHandle.clear();
  }

  void testEmptyShareCollector()
  {
    CPPUNIT_ASSERT_MESSAGE("Carrier validation share map non empty",
                           _collector->getCxrValidationShareMap().empty());
  }

  void testGetFMSharedDataEmpty()
  {
    ShpBitValidationCollectorDerived::FMValidationSharedDataDerived* sharedData =
        _collector->getFMSharedData(_cxr[0], _fmVector[0].get());
    CPPUNIT_ASSERT_MESSAGE("FM shared data non empty", sharedData->getSharedData().empty());

    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage inbound non empty",
                           sharedData->getBitSharedData(0).getMileageInfInbound() == 0);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage outbound non empty",
                           sharedData->getBitSharedData(0).getMileageInfOutbound() == 0);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit RTG map non empty",
                           sharedData->getBitSharedData(0).getRoutingMap().empty());
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit BCE vector non empty",
                           sharedData->getBitSharedData(0).getBCEData().empty());

    CPPUNIT_ASSERT_MESSAGE("FM shared data size not as expected",
                           sharedData->getSharedData().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage inbound non empty",
                           sharedData->getBitSharedData(1).getMileageInfInbound() == 0);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage outbound non empty",
                           sharedData->getBitSharedData(1).getMileageInfOutbound() == 0);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit RTG map non empty",
                           sharedData->getBitSharedData(1).getRoutingMap().empty());
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit BCE vector non empty",
                           sharedData->getBitSharedData(1).getBCEData().empty());

    CPPUNIT_ASSERT_MESSAGE("FM shared data size not as expected",
                           sharedData->getSharedData().size() == 2);
  }

  void testGetFMSharedDataOneElement_Outbound()
  {
    FareMarket* fm = _fmVector[0].get();
    uint32_t bit = 0;

    ShpBitValidationCollectorDerived::FMValidationSharedDataDerived* sharedData =
        _collector->getFMSharedData(_cxr[0], fm);
    CPPUNIT_ASSERT_MESSAGE("FM shared data non empty", sharedData->getSharedData().empty());

    MileageInfo* mileageInfoOutbound;
    MileageInfo* mileageInfoInbound;
    _dataHandle.get(mileageInfoOutbound);
    _dataHandle.get(mileageInfoInbound);

    fm->mileageInfo(true) = mileageInfoOutbound;

    sharedData->collectFareMarketData(fm, bit);

    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage inbound non empty",
                           sharedData->getBitSharedData(bit).getMileageInfInbound() == 0);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage outbound not as expected",
                           sharedData->getBitSharedData(bit).getMileageInfOutbound() ==
                               mileageInfoOutbound);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit RTG map non empty",
                           sharedData->getBitSharedData(bit).getRoutingMap().empty());
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit BCE vector non empty",
                           sharedData->getBitSharedData(bit).getBCEData().empty());

    fm->mileageInfo(true) = 0;

    sharedData->updateFareMarketData(fm, bit);

    CPPUNIT_ASSERT_MESSAGE("FM mileage info inbound not set correctly after update",
                           fm->mileageInfo(false) == 0);
    CPPUNIT_ASSERT_MESSAGE("FM mileage info outbound not set correctly after update",
                           fm->mileageInfo(true) == mileageInfoOutbound);

    CPPUNIT_ASSERT_MESSAGE("FM shared data size not as expected",
                           sharedData->getSharedData().size() == 1);
  }

  void testGetFMSharedDataOneElement_OutboundInbound()
  {
    FareMarket* fm = _fmVector[0].get();
    uint32_t bit = 0;

    ShpBitValidationCollectorDerived::FMValidationSharedDataDerived* sharedData =
        _collector->getFMSharedData(_cxr[0], fm);
    CPPUNIT_ASSERT_MESSAGE("FM shared data non empty", sharedData->getSharedData().empty());

    MileageInfo* mileageInfoOutbound;
    MileageInfo* mileageInfoInbound;
    _dataHandle.get(mileageInfoOutbound);
    _dataHandle.get(mileageInfoInbound);

    fm->mileageInfo(true) = mileageInfoOutbound;
    fm->mileageInfo(false) = mileageInfoInbound;

    sharedData->collectFareMarketData(fm, bit);

    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage inbound not as expected",
                           sharedData->getBitSharedData(bit).getMileageInfInbound() ==
                               mileageInfoInbound);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage outbound not as expected",
                           sharedData->getBitSharedData(bit).getMileageInfOutbound() ==
                               mileageInfoOutbound);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit RTG map non empty",
                           sharedData->getBitSharedData(bit).getRoutingMap().empty());
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit BCE vector non empty",
                           sharedData->getBitSharedData(bit).getBCEData().empty());

    fm->mileageInfo(true) = 0;
    fm->mileageInfo(false) = 0;

    sharedData->updateFareMarketData(fm, bit);

    CPPUNIT_ASSERT_MESSAGE("FM mileage info inbound not set correctly after update",
                           fm->mileageInfo(false) == mileageInfoInbound);
    CPPUNIT_ASSERT_MESSAGE("FM mileage info outbound not set correctly after update",
                           fm->mileageInfo(true) == mileageInfoOutbound);

    CPPUNIT_ASSERT_MESSAGE("FM shared data size not as expected",
                           sharedData->getSharedData().size() == 1);
  }

  void testGetFMSharedDataOneElement_OutboundInbound_Empty()
  {
    FareMarket* fm = _fmVector[0].get();
    uint32_t bit = 0;

    ShpBitValidationCollectorDerived::FMValidationSharedDataDerived* sharedData =
        _collector->getFMSharedData(_cxr[0], fm);
    CPPUNIT_ASSERT_MESSAGE("FM shared data non empty", sharedData->getSharedData().empty());

    CPPUNIT_ASSERT_MESSAGE("FM initiall data not correct", fm->mileageInfo(false) == 0);
    CPPUNIT_ASSERT_MESSAGE("FM initiall data not correct", fm->mileageInfo(true) == 0);

    sharedData->collectFareMarketData(fm, bit);

    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage inbound non empty",
                           sharedData->getBitSharedData(bit).getMileageInfInbound() == 0);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage outbound not as expected",
                           sharedData->getBitSharedData(bit).getMileageInfOutbound() == 0);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit RTG map non empty",
                           sharedData->getBitSharedData(bit).getRoutingMap().empty());
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit BCE vector non empty",
                           sharedData->getBitSharedData(bit).getBCEData().empty());

    sharedData->updateFareMarketData(fm, bit);

    CPPUNIT_ASSERT_MESSAGE("FM mileage info inbound not set correctly after update",
                           fm->mileageInfo(false) == 0);
    CPPUNIT_ASSERT_MESSAGE("FM mileage info outbound not set correctly after update",
                           fm->mileageInfo(true) == 0);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit RTG map non empty",
                           sharedData->getBitSharedData(bit).getRoutingMap().empty());
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit BCE vector non empty",
                           sharedData->getBitSharedData(bit).getBCEData().empty());

    CPPUNIT_ASSERT_MESSAGE("FM shared data size not as expected",
                           sharedData->getSharedData().size() == 1);
  }

  void testGetFMSharedDataTwoElements_OutboundInbound()
  {
    FareMarket* fm = _fmVector[0].get();
    const uint32_t bit1 = 0;
    const uint32_t bit2 = 1;

    ShpBitValidationCollectorDerived::FMValidationSharedDataDerived* sharedData =
        _collector->getFMSharedData(_cxr[0], fm);
    CPPUNIT_ASSERT_MESSAGE("FM shared data non empty", sharedData->getSharedData().empty());

    MileageInfo* mileageInfoOutbound1(0), *mileageInfoInbound1(0), *mileageInfoOutbound2(0),
        *mileageInfoInbound2(0);
    _dataHandle.get(mileageInfoOutbound1);
    _dataHandle.get(mileageInfoInbound1);
    _dataHandle.get(mileageInfoOutbound2);
    _dataHandle.get(mileageInfoInbound2);

    fm->mileageInfo(true) = mileageInfoOutbound1;
    fm->mileageInfo(false) = mileageInfoInbound1;

    sharedData->collectFareMarketData(fm, bit1);

    fm->mileageInfo(true) = mileageInfoOutbound2;
    fm->mileageInfo(false) = mileageInfoInbound2;

    sharedData->collectFareMarketData(fm, bit2);

    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage inbound not as expected",
                           sharedData->getBitSharedData(bit1).getMileageInfInbound() ==
                               mileageInfoInbound1);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage outbound not as expected",
                           sharedData->getBitSharedData(bit1).getMileageInfOutbound() ==
                               mileageInfoOutbound1);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit RTG map non empty",
                           sharedData->getBitSharedData(bit1).getRoutingMap().empty());
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit BCE vector non empty",
                           sharedData->getBitSharedData(bit1).getBCEData().empty());

    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage inbound not as expected",
                           sharedData->getBitSharedData(bit2).getMileageInfInbound() ==
                               mileageInfoInbound2);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage outbound not as expected",
                           sharedData->getBitSharedData(bit2).getMileageInfOutbound() ==
                               mileageInfoOutbound2);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit RTG map non empty",
                           sharedData->getBitSharedData(bit2).getRoutingMap().empty());
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit BCE vector non empty",
                           sharedData->getBitSharedData(bit2).getBCEData().empty());

    fm->mileageInfo(true) = 0;
    fm->mileageInfo(false) = 0;

    sharedData->updateFareMarketData(fm, bit1);

    CPPUNIT_ASSERT_MESSAGE("FM mileage info inbound not set correctly after update",
                           fm->mileageInfo(false) == mileageInfoInbound1);
    CPPUNIT_ASSERT_MESSAGE("FM mileage info outbound not set correctly after update",
                           fm->mileageInfo(true) == mileageInfoOutbound1);

    sharedData->updateFareMarketData(fm, bit2);

    CPPUNIT_ASSERT_MESSAGE("FM mileage info inbound not set correctly after update",
                           fm->mileageInfo(false) == mileageInfoInbound2);
    CPPUNIT_ASSERT_MESSAGE("FM mileage info outbound not set correctly after update",
                           fm->mileageInfo(true) == mileageInfoOutbound2);

    CPPUNIT_ASSERT_MESSAGE("FM shared data size not as expected",
                           sharedData->getSharedData().size() == 2);
  }

  void testGetFMSharedDataTwoFMTwoElements_OutboundInbound()
  {
    FareMarket* fm1 = _fmVector[0].get();
    FareMarket* fm2 = _fmVector[1].get();
    const uint32_t bit1 = 0;
    const uint32_t bit2 = 1;

    ShpBitValidationCollectorDerived::FMValidationSharedDataDerived* sharedData1 =
        _collector->getFMSharedData(_cxr[0], fm1);
    CPPUNIT_ASSERT_MESSAGE("FM shared data non empty", sharedData1->getSharedData().empty());
    ShpBitValidationCollectorDerived::FMValidationSharedDataDerived* sharedData2 =
        _collector->getFMSharedData(_cxr[0], fm2);
    CPPUNIT_ASSERT_MESSAGE("FM shared data non empty", sharedData2->getSharedData().empty());

    MileageInfo* mileageInfoOutbound1(0), *mileageInfoInbound1(0), *mileageInfoOutbound2(0),
        *mileageInfoInbound2(0);
    _dataHandle.get(mileageInfoOutbound1);
    _dataHandle.get(mileageInfoInbound1);
    _dataHandle.get(mileageInfoOutbound2);
    _dataHandle.get(mileageInfoInbound2);

    fm1->mileageInfo(true) = mileageInfoOutbound1;
    fm1->mileageInfo(false) = mileageInfoInbound1;

    sharedData1->collectFareMarketData(fm1, bit1);

    fm2->mileageInfo(true) = mileageInfoOutbound2;
    fm2->mileageInfo(false) = mileageInfoInbound2;

    sharedData2->collectFareMarketData(fm2, bit2);

    CPPUNIT_ASSERT_MESSAGE("FM shared data size not as expected",
                           sharedData1->getSharedData().size() == 1);
    CPPUNIT_ASSERT_MESSAGE("FM shared data size not as expected",
                           sharedData2->getSharedData().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage inbound not as expected",
                           sharedData1->getBitSharedData(bit1).getMileageInfInbound() ==
                               mileageInfoInbound1);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage outbound not as expected",
                           sharedData1->getBitSharedData(bit1).getMileageInfOutbound() ==
                               mileageInfoOutbound1);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit RTG map non empty",
                           sharedData1->getBitSharedData(bit1).getRoutingMap().empty());
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit BCE vector non empty",
                           sharedData1->getBitSharedData(bit1).getBCEData().empty());

    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage inbound not as expected",
                           sharedData2->getBitSharedData(bit2).getMileageInfInbound() ==
                               mileageInfoInbound2);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage outbound not as expected",
                           sharedData2->getBitSharedData(bit2).getMileageInfOutbound() ==
                               mileageInfoOutbound2);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit RTG map non empty",
                           sharedData2->getBitSharedData(bit2).getRoutingMap().empty());
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit BCE vector non empty",
                           sharedData2->getBitSharedData(bit2).getBCEData().empty());

    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage inbound not as expected",
                           sharedData1->getBitSharedData(bit2).getMileageInfInbound() == 0);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage outbound not as expected",
                           sharedData1->getBitSharedData(bit2).getMileageInfOutbound() == 0);

    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage inbound not as expected",
                           sharedData2->getBitSharedData(bit1).getMileageInfInbound() == 0);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage outbound not as expected",
                           sharedData2->getBitSharedData(bit1).getMileageInfOutbound() == 0);

    CPPUNIT_ASSERT_MESSAGE("FM shared data size not as expected",
                           sharedData1->getSharedData().size() == 2);
    CPPUNIT_ASSERT_MESSAGE("FM shared data size not as expected",
                           sharedData2->getSharedData().size() == 2);

    fm1->mileageInfo(true) = 0;
    fm1->mileageInfo(false) = 0;

    sharedData1->updateFareMarketData(fm1, bit1);

    CPPUNIT_ASSERT_MESSAGE("FM mileage info inbound not set correctly after update",
                           fm1->mileageInfo(false) == mileageInfoInbound1);
    CPPUNIT_ASSERT_MESSAGE("FM mileage info outbound not set correctly after update",
                           fm1->mileageInfo(true) == mileageInfoOutbound1);

    fm2->mileageInfo(true) = 0;
    fm2->mileageInfo(false) = 0;

    sharedData2->updateFareMarketData(fm2, bit2);

    CPPUNIT_ASSERT_MESSAGE("FM mileage info inbound not set correctly after update",
                           fm2->mileageInfo(false) == mileageInfoInbound2);
    CPPUNIT_ASSERT_MESSAGE("FM mileage info outbound not set correctly after update",
                           fm2->mileageInfo(true) == mileageInfoOutbound2);

    sharedData1->updateFareMarketData(fm1, bit2);

    CPPUNIT_ASSERT_MESSAGE("FM mileage info inbound not set correctly after update",
                           fm1->mileageInfo(false) == 0);
    CPPUNIT_ASSERT_MESSAGE("FM mileage info outbound not set correctly after update",
                           fm1->mileageInfo(true) == 0);

    sharedData2->updateFareMarketData(fm2, bit1);

    CPPUNIT_ASSERT_MESSAGE("FM mileage info inbound not set correctly after update",
                           fm2->mileageInfo(false) == 0);
    CPPUNIT_ASSERT_MESSAGE("FM mileage info outbound not set correctly after update",
                           fm2->mileageInfo(true) == 0);
  }

  void testGetFMSharedDataTwoCxrsOneElements_OutboundInbound()
  {
    FareMarket* fm1 = _fmVector[0].get();
    FareMarket* fm2 = _fmVector[1].get();
    const uint32_t bit1 = 0;
    const uint32_t bit2 = 1;

    ShpBitValidationCollectorDerived::FMValidationSharedDataDerived* sharedData1 =
        _collector->getFMSharedData(_cxr[0], fm1);
    CPPUNIT_ASSERT_MESSAGE("FM shared data non empty", sharedData1->getSharedData().empty());
    ShpBitValidationCollectorDerived::FMValidationSharedDataDerived* sharedData2 =
        _collector->getFMSharedData(_cxr[1], fm2);
    CPPUNIT_ASSERT_MESSAGE("FM shared data non empty", sharedData2->getSharedData().empty());

    CPPUNIT_ASSERT_MESSAGE("Collector map size not as expected",
                           _collector->getCxrValidationShareMap().size() == 2);

    MileageInfo* mileageInfoOutbound1(0), *mileageInfoInbound1(0), *mileageInfoOutbound2(0),
        *mileageInfoInbound2(0);
    _dataHandle.get(mileageInfoOutbound1);
    _dataHandle.get(mileageInfoInbound1);
    _dataHandle.get(mileageInfoOutbound2);
    _dataHandle.get(mileageInfoInbound2);

    fm1->mileageInfo(true) = mileageInfoOutbound1;
    fm1->mileageInfo(false) = mileageInfoInbound1;

    sharedData1->collectFareMarketData(fm1, bit1);

    fm2->mileageInfo(true) = mileageInfoOutbound2;
    fm2->mileageInfo(false) = mileageInfoInbound2;

    sharedData2->collectFareMarketData(fm2, bit1);

    CPPUNIT_ASSERT_MESSAGE("FM shared data size not as expected",
                           sharedData1->getSharedData().size() == 1);
    CPPUNIT_ASSERT_MESSAGE("FM shared data size not as expected",
                           sharedData2->getSharedData().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage inbound not as expected",
                           sharedData1->getBitSharedData(bit1).getMileageInfInbound() ==
                               mileageInfoInbound1);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage outbound not as expected",
                           sharedData1->getBitSharedData(bit1).getMileageInfOutbound() ==
                               mileageInfoOutbound1);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit RTG map non empty",
                           sharedData1->getBitSharedData(bit1).getRoutingMap().empty());
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit BCE vector non empty",
                           sharedData1->getBitSharedData(bit1).getBCEData().empty());

    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage inbound not as expected",
                           sharedData2->getBitSharedData(bit1).getMileageInfInbound() ==
                               mileageInfoInbound2);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage outbound not as expected",
                           sharedData2->getBitSharedData(bit1).getMileageInfOutbound() ==
                               mileageInfoOutbound2);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit RTG map non empty",
                           sharedData2->getBitSharedData(bit1).getRoutingMap().empty());
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit BCE vector non empty",
                           sharedData2->getBitSharedData(bit1).getBCEData().empty());

    CPPUNIT_ASSERT_MESSAGE("FM shared data size not as expected",
                           sharedData1->getSharedData().size() == 1);
    CPPUNIT_ASSERT_MESSAGE("FM shared data size not as expected",
                           sharedData2->getSharedData().size() == 1);

    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage inbound not as expected",
                           sharedData1->getBitSharedData(bit2).getMileageInfInbound() == 0);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage outbound not as expected",
                           sharedData1->getBitSharedData(bit2).getMileageInfOutbound() == 0);

    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage inbound not as expected",
                           sharedData2->getBitSharedData(bit2).getMileageInfInbound() == 0);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage outbound not as expected",
                           sharedData2->getBitSharedData(bit2).getMileageInfOutbound() == 0);

    CPPUNIT_ASSERT_MESSAGE("FM shared data size not as expected",
                           sharedData1->getSharedData().size() == 2);
    CPPUNIT_ASSERT_MESSAGE("FM shared data size not as expected",
                           sharedData2->getSharedData().size() == 2);

    ShpBitValidationCollectorDerived::FMValidationSharedDataDerived* sharedData =
        _collector->getFMSharedData(_cxr[0], fm2);
    CPPUNIT_ASSERT_MESSAGE("FM shared data non empty", sharedData->getSharedData().empty());

    sharedData = _collector->getFMSharedData(_cxr[1], fm1);
    CPPUNIT_ASSERT_MESSAGE("FM shared data non empty", sharedData->getSharedData().empty());

    sharedData = _collector->getFMSharedData(_cxr[0], fm1);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage outbound not as expected",
                           sharedData->getBitSharedData(bit1).getMileageInfOutbound() ==
                               mileageInfoOutbound1);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage inbound not as expected",
                           sharedData->getBitSharedData(bit1).getMileageInfInbound() ==
                               mileageInfoInbound1);

    sharedData = _collector->getFMSharedData(_cxr[1], fm2);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage outbound not as expected",
                           sharedData->getBitSharedData(bit1).getMileageInfOutbound() ==
                               mileageInfoOutbound2);
    CPPUNIT_ASSERT_MESSAGE("Shared data for bit Mileage inbound not as expected",
                           sharedData->getBitSharedData(bit1).getMileageInfInbound() ==
                               mileageInfoInbound2);

    fm1->mileageInfo(true) = 0;
    fm1->mileageInfo(false) = 0;

    sharedData1->updateFareMarketData(fm1, bit1);

    CPPUNIT_ASSERT_MESSAGE("FM mileage info inbound not set correctly after update",
                           fm1->mileageInfo(false) == mileageInfoInbound1);
    CPPUNIT_ASSERT_MESSAGE("FM mileage info outbound not set correctly after update",
                           fm1->mileageInfo(true) == mileageInfoOutbound1);

    fm2->mileageInfo(true) = 0;
    fm2->mileageInfo(false) = 0;

    sharedData2->updateFareMarketData(fm2, bit1);

    CPPUNIT_ASSERT_MESSAGE("FM mileage info inbound not set correctly after update",
                           fm2->mileageInfo(false) == mileageInfoInbound2);
    CPPUNIT_ASSERT_MESSAGE("FM mileage info outbound not set correctly after update",
                           fm2->mileageInfo(true) == mileageInfoOutbound2);

    sharedData2->updateFareMarketData(fm1, bit1);

    CPPUNIT_ASSERT_MESSAGE("FM mileage info inbound not set correctly after update",
                           fm1->mileageInfo(false) == mileageInfoInbound2);
    CPPUNIT_ASSERT_MESSAGE("FM mileage info outbound not set correctly after update",
                           fm1->mileageInfo(true) == mileageInfoOutbound2);

    sharedData1->updateFareMarketData(fm2, bit1);

    CPPUNIT_ASSERT_MESSAGE("FM mileage info inbound not set correctly after update",
                           fm2->mileageInfo(false) == mileageInfoInbound1);
    CPPUNIT_ASSERT_MESSAGE("FM mileage info outbound not set correctly after update",
                           fm2->mileageInfo(true) == mileageInfoOutbound1);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ShpBitValidationCollectorTest);
} // namespace tse
