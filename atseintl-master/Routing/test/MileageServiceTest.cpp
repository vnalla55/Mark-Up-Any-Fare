#include <iostream>
#include "test/include/CppUnitHelperMacros.h"
#include <vector>
#include "DBAccess/RoutingRestriction.h"
#include "Routing/RoutingConsts.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "Common/DateTime.h"
#include "DBAccess/Loc.h"
#include "Routing/RestrictionValidator.h"
#include "Routing/TravelRoute.h"
#include "test/include/MockTseServer.h"
#include "DataModel/MileageTrx.h"
#include "Routing/MileageService.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/MultiTransport.h"

using namespace std;

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  Mileage* getMil(int mileage)
  {
    Mileage* ret = _memHandle.create<Mileage>();
    ret->mileage() = mileage;
    ret->globaldir() = GlobalDirection::WH;
    return ret;
  }
  MultiTransport* getMC(LocCode city, LocCode loc)
  {
    MultiTransport* ret = _memHandle.create<MultiTransport>();
    ret->multitranscity() = city;
    ret->multitransLoc() = loc;
    return ret;
  }

public:
  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    if (origin == "DFW" && dest == "MIA" && mileageType == 'M')
      return 0;
    else if (origin == "DFW" && dest == "PHX" && mileageType == 'M')
      return 0;
    return DataHandleMock::getMileage(origin, dest, mileageType, globalDir, date);
  }
  const std::vector<Mileage*>& getMileage(const LocCode& origin,
                                          const LocCode& dest,
                                          const DateTime& date,
                                          Indicator mileageType = 'T')
  {
    if (origin == "DFW" && dest == "PHX")
    {
      std::vector<Mileage*>& ret = *_memHandle.create<std::vector<Mileage*> >();
      if (mileageType == 'T')
        ret.push_back(getMil(872));
      return ret;
    }
    else if (origin == "PHX" && dest == "MIA")
    {
      std::vector<Mileage*>& ret = *_memHandle.create<std::vector<Mileage*> >();
      if (mileageType == 'T')
        ret.push_back(getMil(1969));
      return ret;
    }

    return DataHandleMock::getMileage(origin, dest, date, mileageType);
  }
  const MileageSubstitution* getMileageSubstitution(const LocCode& key, const DateTime& date)
  {
    if (key == "DFW" || key == "MIA" || key == "PHX")
      return 0;
    return DataHandleMock::getMileageSubstitution(key, date);
  }
  const TariffMileageAddon* getTariffMileageAddon(const CarrierCode& carrier,
                                                  const LocCode& unpublishedAddonLoc,
                                                  const GlobalDirection& globalDir,
                                                  const DateTime& date)
  {
    if (unpublishedAddonLoc == "DFW" || unpublishedAddonLoc == "MIA" ||
        unpublishedAddonLoc == "PHX")
      return 0;
    return DataHandleMock::getTariffMileageAddon(carrier, unpublishedAddonLoc, globalDir, date);
  }
  const std::vector<MultiTransport*>& getMultiTransportCity(const LocCode& locCode,
                                                            const CarrierCode& carrierCode,
                                                            GeoTravelType tvlType,
                                                            const DateTime& tvlDate)
  {
    std::vector<MultiTransport*>& ret = *_memHandle.create<std::vector<MultiTransport*> >();
    if (locCode == "DFW")
    {
      ret.push_back(getMC("DFW", "DFW"));
      return ret;
    }
    else if (locCode == "MIA")
      return ret;
    else if (locCode == "PHX")
      return ret;
    return DataHandleMock::getMultiTransportCity(locCode, carrierCode, tvlType, tvlDate);
  }
  const std::vector<TPMExclusion*>& getTPMExclus(const CarrierCode& carrier)
  {
    if (carrier == "LH")
      return *_memHandle.create<std::vector<TPMExclusion*> >();
    return DataHandleMock::getTPMExclus(carrier);
  }
};
}

class MileageServiceTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MileageServiceTest);
  CPPUNIT_TEST(testValidateTrue);
  CPPUNIT_TEST(testProcess);
  CPPUNIT_TEST(testProcess_Diagnostic452);
  CPPUNIT_TEST_SUITE_END();

  // data
private:
  TestMemHandle _memHandle;
  MileageTrx* _trx;
  PricingRequest* _request;
  MileageService* _mileageService;
  MockTseServer* _svr;

  // helper methods
public:
  void setUp()
  {
    _memHandle.create<MyDataHandle>();
    _svr = _memHandle.insert(new MockTseServer);
    _mileageService = new MileageService("MILEAGE_SVC", *_svr);
    _mileageService->initialize(0, 0);
    _trx = _memHandle.insert(new MileageTrx);
    _request = _memHandle.insert(new PricingRequest);
  }

  void tearDown()
  {
    delete _mileageService; // ~MileageService() is private
    _memHandle.clear();
  }

  void setTrx()
  {
    _trx->setRequest(_request);
    _request->ticketingAgent() = _memHandle.insert(new Agent);
    _trx->inputDT() = DateTime(boost::gregorian::date(2009, 11, 11));

    // create some Loc objects
    const Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    const Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPHX.xml");
    const Loc* loc3 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMIA.xml");

    MileageTrx::MileageItem* item1 = _memHandle.insert(new MileageTrx::MileageItem);
    MileageTrx::MileageItem* item2 = _memHandle.insert(new MileageTrx::MileageItem);
    MileageTrx::MileageItem* item3 = _memHandle.insert(new MileageTrx::MileageItem);

    item1->cityLoc = loc1;
    item2->cityLoc = loc2;
    item3->cityLoc = loc3;
    item1->carrierCode = "LH";
    item2->carrierCode = "LH";
    item3->carrierCode = "LH";

    _trx->items().push_back(item1);
    _trx->items().push_back(item2);
    _trx->items().push_back(item3);
  }

  void setTrxWithDiag452()
  {
    setTrx();
    _trx->diagnostic().diagnosticType() = Diagnostic452;
    _trx->diagnostic().activate();
  }

  // tests
public:
  void testValidateTrue()
  {
    setTrx();
    CPPUNIT_ASSERT(_mileageService->process(*_trx));
  }

  void testProcess()
  {
    setTrx();
    _mileageService->process(*_trx);
    CPPUNIT_ASSERT_EQUAL(string(" \n"
                                " WN X/DFW LH X/PHX LH X/MIA /11NOV09\n"
                                "\n"
                                "*************************************************************\n"
                                " \n"
                                "    CTY   GI   TPM    CUM    MPM  EMS   DED  LAST  NEXT   25M\n"
                                " \n"
                                "    DFW 1\n"
                                " 1.XPHX 1 -    872    872         \n"
                                "\n"
                                " 2.XMIA 1 -   1969   2841         \n"
                                " \n"),
                         _trx->response().str());
  }

  void testProcess_Diagnostic452()
  {
    setTrxWithDiag452();
    _mileageService->process(*_trx);
    CPPUNIT_ASSERT_EQUAL(string("*******************  TPM EXCLUSION TABLE  *********************"),
                         _trx->diagnostic().toString().substr(0, 63));
  }

}; // class MileageServiceTest

CPPUNIT_TEST_SUITE_REGISTRATION(MileageServiceTest);
} // namespace tse
