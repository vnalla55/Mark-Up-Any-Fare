#include "Common/ErrorResponseException.h"
#include "Common/Logger.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CurrencySelection.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DBAccess/StaticObjectPool.h"
#include "Fares/CurrencySelectionValidator.h"
#include "Fares/FareCurrencySelection.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/testdata/TestPricingTrxFactory.h"

#include <iostream>
#include <vector>

#include <time.h>

using namespace std;

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const LocCode getMultiTransportCity(const LocCode& locCode)
  {
    if (locCode == "SAO" || locCode == "SJP" || locCode == "LON" || locCode == "DFW" ||
        locCode == "ZRH" || locCode == "SSA")
      return "";
    return DataHandleMock::getMultiTransportCity(locCode);
  }
  const std::vector<const FareInfo*>& getFaresByMarketCxr(const LocCode& market1,
                                                          const LocCode& market2,
                                                          const CarrierCode& cxr,
                                                          const DateTime& date)
  {
    if ((market1 == "SJP" && market2 == "SSA" && cxr == "JJ") ||
        (market1 == "SAO" && market2 == "SJP" && cxr == "JJ"))
    {
      std::vector<const FareInfo*>* ret = _memHandle.create<std::vector<const FareInfo*> >();
      FareInfo* fi = _memHandle.create<FareInfo>();
      fi->currency() = "BRL";
      ret->push_back(fi);
      fi = _memHandle.create<FareInfo>();
      fi->currency() = "USD";
      ret->push_back(fi);
      return *ret;
    }
    else if (market1 == "LON" && market2 == "DFW" && cxr == "AA")
    {
      std::vector<const FareInfo*>* ret = _memHandle.create<std::vector<const FareInfo*> >();
      FareInfo* fi = _memHandle.create<FareInfo>();
      fi->currency() = "GBP";
      ret->push_back(fi);
      fi = _memHandle.create<FareInfo>();
      fi->currency() = "USD";
      ret->push_back(fi);
      return *ret;
    }
    else if (market1 == "DFW" && market2 == "ZRH" && cxr == "AA")
    {
      std::vector<const FareInfo*>* ret = _memHandle.create<std::vector<const FareInfo*> >();
      FareInfo* fi = _memHandle.create<FareInfo>();
      fi->currency() = "CHF";
      ret->push_back(fi);
      fi = _memHandle.create<FareInfo>();
      fi->currency() = "USD";
      ret->push_back(fi);
      fi = _memHandle.create<FareInfo>();
      fi->currency() = "EUR";
      ret->push_back(fi);

      return *ret;
    }
    return DataHandleMock::getFaresByMarketCxr(market1, market2, cxr, date);
  }
  const std::vector<CurrencySelection*>&
  getCurrencySelection(const NationCode& nation, const DateTime& date)
  {
    if (nation == "GB" || nation == "US")
      return *_memHandle.create<std::vector<CurrencySelection*> >();
    return DataHandleMock::getCurrencySelection(nation, date);
  }
  const Loc* getLoc(const LocCode& locCode, const DateTime& date)
  {
    if (locCode == "")
      return 0;
    return DataHandleMock::getLoc(locCode, date);
  }
};
}
class FareCurrencySelectionTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareCurrencySelectionTest);
  CPPUNIT_TEST(testSelectPrimeForeignDomestic);
  CPPUNIT_TEST(testSelectAlternateForeignDomestic);
  CPPUNIT_TEST(testSelectPrimeInternational);
  CPPUNIT_TEST(testSelectAlternateInternational);
  CPPUNIT_TEST(testSelectUSToCA);
  CPPUNIT_TEST(testSelectCAToUS);
  CPPUNIT_TEST(testSelectCAToUSWithDomesticCA);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _globalMemHandle;

public:
  void setUp() { _globalMemHandle.create<TestConfigInitializer>(); }

  void tearDown() { _globalMemHandle.clear(); }

  void testSelectPrimeForeignDomestic()
  {
    try
    {
      MyDataHandle mdh;
      FareCurrencySelection fcs;
      PricingTrx trx;
      Itin itin;
      DataHandle dataHandle;

      PricingOptions options;
      PricingRequest request;
      trx.setOptions(&options);
      trx.setRequest(&request);
      trx.diagnostic().activate();
      trx.diagnostic().diagnosticType() = Diagnostic212;

      LocCode rio("RIO");
      DateTime travelDate = DateTime::localTime();

      // LOG4CXX_DEBUG(_logger, "Location Code: "<<rio);
      // LOG4CXX_DEBUG(_logger, "Travel Date: "<<travelDate);

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {
        NationCode nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      if (!agent)
      {
        LOG4CXX_DEBUG(_logger, "Failed to retrieve agent ");
      }

      LOG4CXX_DEBUG(_logger, "Setting location");

      agent->agentLocation() = loc;

      trx.getRequest()->ticketingAgent() = agent;

      LOG4CXX_DEBUG(_logger, "Setting Agent location");

      // 1. Set Itin travel type
      //
      itin.geoTravelType() = GeoTravelType::ForeignDomestic;

      // 2. Retrieve FareMarkete vector from Itin
      std::vector<FareMarket*>& fareMarkets = itin.fareMarket();

      const Loc* sao = dataHandle.getLoc("SAO", DateTime::localTime());
      const Loc* sjp = dataHandle.getLoc("SJP", DateTime::localTime());

      AirSeg travelSeg;
      travelSeg.segmentOrder() = 0;
      travelSeg.geoTravelType() = GeoTravelType::ForeignDomestic;
      travelSeg.origin() = sao;
      travelSeg.destination() = sjp;
      travelSeg.departureDT() = travelDate;
      trx.travelSeg().push_back(&travelSeg);

      // 3. Create fare market
      //
      FareMarket fareMarket;

      fareMarket.origin() = sao;
      fareMarket.boardMultiCity() = sao->loc();
      fareMarket.destination() = sjp;
      fareMarket.offMultiCity() = sjp->loc();
      fareMarket.governingCarrier() = "JJ";
      fareMarket.geoTravelType() = GeoTravelType::ForeignDomestic;

      // 4. Retrieve instance of PaxTypeBucket vector
      //
      std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();

      // 5. Create instance of PaxTypeBucket
      //
      PaxTypeBucket paxTypeCortege;

      // 6. Create PaxTypeFare
      //
      PaxTypeFare paxFare1;

      // 7. Create Fare
      //
      FareInfo fareInfo;
      fareInfo._market1 = "SAO";
      fareInfo._market2 = "SJP";
      fareInfo._currency = "BRL";
      fareInfo._carrier = "JJ";
      fareInfo._directionality = FROM;

      Fare mockFare1;
      mockFare1.initialize(Fare::FS_ForeignDomestic, &fareInfo, fareMarket);

      // 7a. Create PaxType
      //
      PaxType paxType1;
      paxType1.paxType() = "ADT";

      // 8. Associate Fare to PaxTypeFare
      //
      paxFare1.setFare(&mockFare1);
      paxFare1.paxType() = &paxType1;
      paxFare1.fareMarket() = &fareMarket;

      paxTypeCortege.requestedPaxType() = &paxType1;

      // 8. Put PaxTypeFare in PaxTypeBucket PaxTypeFare vector
      //
      LOG4CXX_DEBUG(_logger, "Inserting pax type fare into vector");
      paxTypeCortege.paxTypeFare().push_back(&paxFare1);

      // 9. Put PaxTypeBucket into FareMarket PaxTypeBucket vector
      //
      LOG4CXX_DEBUG(_logger, "Inserting pax type fare vector into faremarket vector");
      LOG4CXX_DEBUG(_logger, "Size of paxTypeFareVec " << paxTypeCortege.paxTypeFare().size());
      // LOG4CXX_DEBUG(_logger, "Size of paxFareVec "<<fareMarket._paxTypeFare.size());

      paxTypeCortegeVec.push_back(paxTypeCortege);

      LOG4CXX_DEBUG(_logger, "Inserting fare market vector");

      // 10. Put FareMarket into Itin FareMarket vector
      fareMarkets.push_back(&fareMarket);

      //-------------------------
      //  ADD ANOTHER FARE MARKET
      //-------------------------

      // 1. Create another fare market
      //
      FareMarket fareMarket2;

      const Loc* ssa = dataHandle.getLoc("SSA", DateTime::localTime());

      fareMarket2.origin() = sjp;
      fareMarket2.boardMultiCity() = sjp->loc();
      fareMarket2.destination() = ssa;
      fareMarket2.offMultiCity() = ssa->loc();
      fareMarket2.governingCarrier() = "JJ";

      AirSeg travelSeg2;
      travelSeg2.segmentOrder() = 1;
      travelSeg2.geoTravelType() = GeoTravelType::ForeignDomestic;
      travelSeg2.origin() = sjp;
      travelSeg2.destination() = ssa;
      travelSeg2.departureDT() = travelDate;
      trx.travelSeg().push_back(&travelSeg2);

      fareMarket2.geoTravelType() = GeoTravelType::ForeignDomestic;

      // 2. Retrieve PaxTypeBucket for this FareMarket
      //
      std::vector<PaxTypeBucket>& paxTypeCortegeVec2 = fareMarket2.paxTypeCortege();

      // 3. Create instance of PaxTypeBucket
      //
      PaxTypeBucket paxTypeCortege2;

      // 4. Create PaxTypeFare
      //
      PaxTypeFare paxFare2;

      // 5. Create FareInfo
      //
      FareInfo fareInfo2;
      fareInfo2._market1 = "SJP";
      fareInfo2._market2 = "SSA";
      fareInfo2._currency = "USD";
      fareInfo2._carrier = "JJ";
      fareInfo2._directionality = TO;
      LOG4CXX_DEBUG(_logger, "Value of TO = " << TO);

      // 6. Create MockFare2
      //
      Fare mockFare2;
      mockFare2.initialize(Fare::FS_ForeignDomestic, &fareInfo2, fareMarket2);

      // 7. Create PaxType
      //
      PaxType paxType2;
      paxType2.paxType() = "ADT";

      // 8. Associate Fare to PaxTypeFare
      //
      paxFare2.setFare(&mockFare2);
      paxFare2.paxType() = &paxType2;
      paxFare2.fareMarket() = &fareMarket2;

      paxTypeCortege2.requestedPaxType() = &paxType2;
      // 9. Put PaxTypeFare instance in
      //   PaxTypeFare vector.
      //
      paxTypeCortege2.paxTypeFare().push_back(&paxFare2);

      // 9. Put PaxTypeBucket instance in
      //   PaxTypeBucket  vector for 2nd FareMarket.
      //
      paxTypeCortegeVec2.push_back(paxTypeCortege2);

      fareMarkets.push_back(&fareMarket2);

      trx.itin().push_back(&itin);

      fcs.selectPrimeCurrency(trx, fareMarket, itin, true);
      fcs.selectPrimeCurrency(trx, fareMarket2, itin, true);

      // Test PFA Cruise Agent with cruisePfaCur
      // Test PFA Cruise Agent with cruisePfaCur
      Customer agentTJR;
      agentTJR.cruisePfaCur() = "EUR";

      agent->agentTJR() = &agentTJR;
      request.ticketingAgent() = agent;

      PaxType paxTypePFA;
      paxTypePFA.paxType() = "PFA";
      PaxTypeBucket paxTypeCortegePFA;
      paxTypeCortegePFA.requestedPaxType() = &paxTypePFA;
      paxTypeCortegePFA.paxTypeFare().push_back(&paxFare1);
      paxTypeCortegeVec.push_back(paxTypeCortegePFA);

      fareMarket.currencies().clear();
      fcs.selectPrimeCurrency(trx, fareMarket, itin, true);

      // std::string str = trx.diagnostic().toString();
      // std::cout << std::endl << str.c_str() << std::endl;

      LOG4CXX_DEBUG(_logger, "Completed test");
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception: " << ex.what()); }
  }

  void testSelectPrimeInternational()
  {
    try
    {
      MyDataHandle mdh;
      FareCurrencySelection fcs;
      PricingTrx trx;
      PricingRequest request;
      trx.setRequest(&request);
      trx.diagnostic().activate();
      trx.diagnostic().diagnosticType() = Diagnostic212;
      Itin itin;

      DateTime travelDate;

      PricingOptions options;
      trx.setOptions(&options);

      // 1. Set Itin travel type
      //
      itin.geoTravelType() = GeoTravelType::International;

      // 2. Build Fare Markets
      std::vector<FareMarket*>& fareMarkets = itin.fareMarket();

      Loc lon;
      Loc dfw;
      LocCode lonLoc("LON");
      LocCode dfwLoc("DFW");
      lon.loc() = lonLoc; // LON
      lon.nation() = "GB";
      dfw.loc() = dfwLoc; // Manchester Paulo
      dfw.nation() = "US";

      PricingRequest req;
      Agent agent;
      agent.agentLocation() = &dfw;
      req.ticketingAgent() = &agent;
      trx.setRequest(&req);

      AirSeg travelSeg;
      travelSeg.segmentOrder() = 0;
      travelSeg.geoTravelType() = GeoTravelType::International;
      travelSeg.origin() = &lon;
      travelSeg.destination() = &dfw;
      travelSeg.departureDT() = travelDate;
      trx.travelSeg().push_back(&travelSeg);

      // 3. Create fare market
      //
      FareMarket fareMarket;

      fareMarket.origin() = (&lon);
      fareMarket.destination() = (&dfw);
      fareMarket.governingCarrier() = ("AA");
      fareMarket.geoTravelType() = GeoTravelType::International;

      // 4. Retrieve instance of PaxTypeBucket Vector
      //
      std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();

      // 5. Create Instance of PaxTypeBucket
      //
      PaxTypeBucket paxTypeCortege;

      // 6. Create PaxTypeFare
      //
      PaxTypeFare paxFare1;

      // 6. Create Fare
      //
      FareInfo fareInfo;
      fareInfo._market1 = "DFW";
      fareInfo._market2 = "LON";
      fareInfo._currency = "USD";
      fareInfo._carrier = "AA";
      fareInfo._directionality = FROM;

      Fare mockFare1;
      mockFare1.initialize(Fare::FS_International, &fareInfo, fareMarket);

      // 6a. Create PaxType
      //
      PaxType paxType1;
      paxType1.paxType() = "ADT";

      // 7. Associate Fare to PaxTypeFare
      //
      paxFare1.setFare(&mockFare1);
      paxFare1.paxType() = &paxType1;
      paxFare1.fareMarket() = &fareMarket;

      paxTypeCortege.requestedPaxType() = &paxType1;
      // 8. Put PaxTypeFare instance in PaxTypeFare vector
      //
      LOG4CXX_DEBUG(_logger, "Inserting pax type fare into vector");
      paxTypeCortege.paxTypeFare().push_back(&paxFare1);

      // 9. Associate PaxTypeFare vector to FareMarket
      //
      LOG4CXX_DEBUG(_logger, "Inserting pax type fare vector into faremarket vector");
      LOG4CXX_DEBUG(_logger, "Size of paxTypeFareVec " << paxTypeCortege.paxTypeFare().size());

      paxTypeCortegeVec.push_back(paxTypeCortege);

      LOG4CXX_DEBUG(_logger, "Inserting fare market vector");
      fareMarkets.push_back(&fareMarket);

      //-------------------------
      //  ADD ANOTHER FARE MARKET
      //-------------------------

      // 1. Create another fare market
      //
      FareMarket fareMarket2;

      Loc zurich;
      zurich.nation() = "CH";
      LocCode zrhLoc("ZRH");
      zurich.loc() = zrhLoc;
      fareMarket2.origin() = (&dfw);
      fareMarket2.destination() = (&zurich);
      fareMarket2.governingCarrier() = ("AA");

      fareMarket2.geoTravelType() = GeoTravelType::International;

      AirSeg travelSeg1;
      travelSeg1.segmentOrder() = 1;
      travelSeg1.geoTravelType() = GeoTravelType::International;
      travelSeg1.origin() = &dfw;
      travelSeg1.destination() = &zurich;
      travelSeg1.departureDT() = travelDate;
      trx.travelSeg().push_back(&travelSeg1);

      // 3. Retrieve instance of PaxTypeBucket Vector
      //
      std::vector<PaxTypeBucket>& paxTypeCortegeVec2 = fareMarket2.paxTypeCortege();

      // 4. Create Instance of PaxTypeBucket
      //
      PaxTypeBucket paxTypeCortege2;

      // 5. Create PaxTypeFare
      //
      PaxTypeFare paxFare2;

      // 6. Create MockFare
      //
      FareInfo fareInfo2;
      fareInfo2._market1 = "DFW";
      fareInfo2._market2 = "ZRH";
      fareInfo2._currency = "USD";
      fareInfo2._carrier = "AA";
      fareInfo2._directionality = TO;

      Fare mockFare2;
      mockFare2.initialize(Fare::FS_International, &fareInfo2, fareMarket2);

      // 6a. Create PaxType
      //
      PaxType paxType2;
      paxType2.paxType() = "ADT";

      // 7. Associate Fare to PaxTypeFare
      //
      paxFare2.setFare(&mockFare2);
      paxFare2.paxType() = &paxType2;
      paxFare2.fareMarket() = &fareMarket2;

      paxTypeCortege2.requestedPaxType() = &paxType2;
      // 8. Put another PaxTypeFare instance in
      //   PaxTypeFare vector.
      //
      paxTypeCortege2.paxTypeFare().push_back(&paxFare2);

      paxTypeCortegeVec2.push_back(paxTypeCortege2);

      fareMarkets.push_back(&fareMarket2);

      trx.itin().push_back(&itin);

      bool selectRC = fcs.selectPrimeCurrency(trx, fareMarket, itin, true);

      LOG4CXX_DEBUG(
          _logger,
          "First FareMarket for International Select Prime Currency Returned: " << selectRC);

      selectRC = fcs.selectPrimeCurrency(trx, fareMarket2, itin, true);

      LOG4CXX_DEBUG(
          _logger,
          "Second FareMarket for International Select Prime Currency Returned: " << selectRC);

      for (unsigned int i = 0; i < fareMarkets.size(); i++)
      {
        std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarkets[i]->paxTypeCortege();

        for (unsigned int j = 0; j < paxTypeCortegeVec.size(); j++)
        {
          PaxTypeBucket& paxTypeCortege = paxTypeCortegeVec[j];
          LOG4CXX_DEBUG(
              _logger,
              "InBound currency for this FareMarket: " << paxTypeCortege.inboundCurrency());
          LOG4CXX_DEBUG(
              _logger,
              "OutBound currency for this FareMarket: " << paxTypeCortege.outboundCurrency());
        }
      }

      LOG4CXX_DEBUG(_logger, "Completed test");
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception: " << ex.what()); }
  }

  void testSelectAlternateForeignDomestic()
  {
    try
    {
      MyDataHandle mdh;
      FareCurrencySelection fcs;
      PricingTrx trx;
      Itin itin;
      DataHandle dataHandle;

      PricingOptions options;
      options.alternateCurrency() = "BRL";
      PricingRequest request;
      trx.setOptions(&options);
      trx.setRequest(&request);
      trx.diagnostic().activate();
      trx.diagnostic().diagnosticType() = Diagnostic212;

      LocCode rio("RIO");
      DateTime travelDate = DateTime::localTime();

      // LOG4CXX_DEBUG(_logger, "Location Code: "<<rio);
      // LOG4CXX_DEBUG(_logger, "Travel Date: "<<travelDate);

      const tse::Loc* loc = dataHandle.getLoc(rio, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {
        NationCode nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for RIO is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      if (!agent)
      {
        LOG4CXX_DEBUG(_logger, "Failed to retrieve agent ");
      }

      LOG4CXX_DEBUG(_logger, "Setting location");

      agent->agentLocation() = loc;

      trx.getRequest()->ticketingAgent() = agent;

      LOG4CXX_DEBUG(_logger, "Setting Agent location");

      // 1. Set Itin travel type
      //
      itin.geoTravelType() = GeoTravelType::ForeignDomestic;

      // 2. Retrieve FareMarkete vector from Itin
      std::vector<FareMarket*>& fareMarkets = itin.fareMarket();

      Loc sao;
      Loc sjp;
      LocCode saoLoc("SAO");
      LocCode sjpLoc("SJP");
      sao.loc() = saoLoc; //
      sao.nation() = "BR";
      sjp.loc() = sjpLoc; //
      sjp.nation() = "BR";

      AirSeg travelSeg;
      travelSeg.segmentOrder() = 0;
      travelSeg.geoTravelType() = GeoTravelType::ForeignDomestic;
      travelSeg.origin() = &sao;
      travelSeg.destination() = &sjp;
      travelSeg.departureDT() = travelDate;
      trx.travelSeg().push_back(&travelSeg);
      trx.getRequest()->ticketingDT() = travelDate;

      // 3. Create fare market
      //
      FareMarket fareMarket;

      fareMarket.origin() = (&sao);
      fareMarket.destination() = (&sjp);
      fareMarket.governingCarrier() = ("JJ");
      fareMarket.geoTravelType() = GeoTravelType::ForeignDomestic;

      // 4. Retrieve instance of PaxTypeBucket vector
      //
      std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();

      // 5. Create instance of PaxTypeBucket
      //
      PaxTypeBucket paxTypeCortege;

      // 6. Create PaxTypeFare
      //
      PaxTypeFare paxFare1;

      // 7. Create Fare
      //
      FareInfo fareInfo;
      fareInfo._market1 = "SAO";
      fareInfo._market2 = "SJP";
      fareInfo._currency = "BRL";
      fareInfo._carrier = "JJ";
      fareInfo._directionality = FROM;

      Fare mockFare1;
      mockFare1.initialize(Fare::FS_ForeignDomestic, &fareInfo, fareMarket);

      // 7a. Create PaxType
      //
      PaxType paxType1;
      paxType1.paxType() = "ADT";

      // 8. Associate Fare to PaxTypeFare
      //
      paxFare1.setFare(&mockFare1);
      paxFare1.paxType() = &paxType1;
      paxFare1.fareMarket() = &fareMarket;

      paxTypeCortege.requestedPaxType() = &paxType1;
      // 8. Put PaxTypeFare in PaxTypeBucket PaxTypeFare vector
      //
      LOG4CXX_DEBUG(_logger, "Inserting pax type fare into vector");
      paxTypeCortege.paxTypeFare().push_back(&paxFare1);

      // 9. Put PaxTypeBucket into FareMarket PaxTypeBucket vector
      //
      LOG4CXX_DEBUG(_logger, "Inserting pax type fare vector into faremarket vector");
      LOG4CXX_DEBUG(_logger, "Size of paxTypeFareVec " << paxTypeCortege.paxTypeFare().size());
      // LOG4CXX_DEBUG(_logger, "Size of paxFareVec "<<fareMarket._paxTypeFare.size());

      paxTypeCortegeVec.push_back(paxTypeCortege);

      LOG4CXX_DEBUG(_logger, "Inserting fare market vector");

      // 10. Put FareMarket into Itin FareMarket vector
      fareMarkets.push_back(&fareMarket);

      //-------------------------
      //  ADD ANOTHER FARE MARKET
      //-------------------------

      // 1. Create another fare market
      //
      FareMarket fareMarket2;

      Loc ssa;
      ssa.nation() = "BR";
      LocCode ssaLoc("SSA");
      ssa.loc() = ssaLoc;
      fareMarket2.origin() = (&sjp);
      fareMarket2.destination() = (&ssa);
      fareMarket2.governingCarrier() = ("JJ");

      AirSeg travelSeg1;
      travelSeg1.segmentOrder() = 1;
      travelSeg1.geoTravelType() = GeoTravelType::ForeignDomestic;
      travelSeg1.origin() = &sjp;
      travelSeg1.destination() = &ssa;
      travelSeg1.departureDT() = travelDate;
      trx.travelSeg().push_back(&travelSeg1);

      fareMarket2.geoTravelType() = GeoTravelType::ForeignDomestic;

      // 2. Retrieve PaxTypeBucket for this FareMarket
      //
      std::vector<PaxTypeBucket>& paxTypeCortegeVec2 = fareMarket2.paxTypeCortege();

      // 3. Create instance of PaxTypeBucket
      //
      PaxTypeBucket paxTypeCortege2;

      // 4. Create PaxTypeFare
      //
      PaxTypeFare paxFare2;

      // 5. Create FareInfo
      //
      FareInfo fareInfo2;
      fareInfo2._market1 = "SJP";
      fareInfo2._market2 = "SSA";
      fareInfo2._currency = "USD";
      fareInfo2._carrier = "JJ";
      fareInfo2._directionality = TO;
      LOG4CXX_DEBUG(_logger, "Value of TO = " << TO);

      // 6. Create MockFare2
      //
      Fare mockFare2;
      mockFare2.initialize(Fare::FS_ForeignDomestic, &fareInfo2, fareMarket2);

      // 7. Create PaxType
      //
      PaxType paxType2;
      paxType2.paxType() = "ADT";

      // 8. Associate Fare to PaxTypeFare
      //
      paxFare2.setFare(&mockFare2);
      paxFare2.paxType() = &paxType2;
      paxFare2.fareMarket() = &fareMarket2;

      paxTypeCortege2.requestedPaxType() = &paxType2;
      // 9. Put PaxTypeFare instance in
      //   PaxTypeFare vector.
      //
      paxTypeCortege2.paxTypeFare().push_back(&paxFare2);

      // 9. Put PaxTypeBucket instance in
      //   PaxTypeBucket  vector for 2nd FareMarket.
      //
      paxTypeCortegeVec2.push_back(paxTypeCortege2);

      fareMarkets.push_back(&fareMarket2);

      trx.itin().push_back(&itin);

      GeoTravelType itinTravelType = itin.geoTravelType();

      CurrencyCode alternateCurrency("USD");

      bool selectRC = false;

      try
      {
        selectRC = fcs.selectAlternateCurrency(trx, fareMarket, alternateCurrency, itinTravelType);

        LOG4CXX_DEBUG(_logger,
                      "First FareMarket for Foreign Domestic Select Alternate Currency Returned: "
                          << selectRC);
      }
      catch (tse::ErrorResponseException& ex)
      {
        LOG4CXX_DEBUG(
            _logger,
            "Alternate Foreign Domestic Currency Selection caught Error Response exception: "
                << ex.what());
      }

      try
      {
        selectRC = fcs.selectAlternateCurrency(trx, fareMarket2, alternateCurrency, itinTravelType);
      }
      catch (tse::ErrorResponseException& ex)
      {
        LOG4CXX_DEBUG(
            _logger,
            "Alternate Foreign Domestic Currency Selection caught Error Response exception: "
                << ex.what());
      }

      LOG4CXX_DEBUG(_logger,
                    "Second FareMarket For Foreign Domestic Select Alternate Currency Returned: "
                        << selectRC);

      for (unsigned int i = 0; i < fareMarkets.size(); i++)
      {
        std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarkets[i]->paxTypeCortege();

        for (unsigned int j = 0; j < paxTypeCortegeVec.size(); j++)
        {
          PaxTypeBucket& paxTypeCortege = paxTypeCortegeVec[j];
          LOG4CXX_DEBUG(
              _logger,
              "InBound currency for this FareMarket: " << paxTypeCortege.inboundCurrency());
          LOG4CXX_DEBUG(
              _logger,
              "OutBound currency for this FareMarket: " << paxTypeCortege.outboundCurrency());
        }
      }

      LOG4CXX_DEBUG(_logger, "Completed test");
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception: " << ex.what()); }
  }

  void testSelectAlternateInternational()
  {
    try
    {
      MyDataHandle mdh;
      FareCurrencySelection fcs;
      PricingTrx trx;
      Itin itin;

      DateTime travelDate;

      PricingOptions options;
      // options.alternateCurrency() = "FRF";
      options.alternateCurrency() = "BRL";
      trx.setOptions(&options);

      PricingRequest request;
      trx.setRequest(&request);
      trx.diagnostic().activate();
      trx.diagnostic().diagnosticType() = Diagnostic212;

      // 1. Set Itin travel type
      //
      itin.geoTravelType() = GeoTravelType::International;

      // 2. Build Fare Markets
      std::vector<FareMarket*>& fareMarkets = itin.fareMarket();

      Loc lon;
      Loc dfw;
      LocCode lonLoc("LON");
      LocCode dfwLoc("DFW");
      lon.loc() = lonLoc; // LON
      lon.nation() = "GB";
      dfw.loc() = dfwLoc; // Manchester Paulo
      dfw.nation() = "US";

      AirSeg travelSeg;
      travelSeg.segmentOrder() = 0;
      travelSeg.geoTravelType() = GeoTravelType::International;
      travelSeg.origin() = &lon;
      travelSeg.destination() = &dfw;
      travelSeg.departureDT() = travelDate;
      trx.travelSeg().push_back(&travelSeg);

      // 3. Create fare market
      //
      FareMarket fareMarket;

      fareMarket.origin() = (&lon);
      fareMarket.destination() = (&dfw);
      fareMarket.governingCarrier() = ("AA");
      fareMarket.geoTravelType() = GeoTravelType::International;

      // 4. Retrieve instance of PaxTypeBucket Vector
      //
      std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();

      // 5. Create Instance of PaxTypeBucket
      //
      PaxTypeBucket paxTypeCortege;

      // 6. Create PaxTypeFare
      //
      PaxTypeFare paxFare1;

      // 6. Create Fare
      //
      FareInfo fareInfo;
      fareInfo._market1 = "DFW";
      fareInfo._market2 = "LON";
      fareInfo._currency = "USD";
      fareInfo._carrier = "AA";
      fareInfo._directionality = FROM;

      Fare mockFare1;
      mockFare1.initialize(Fare::FS_International, &fareInfo, fareMarket);

      // 6a. Create PaxType
      //
      PaxType paxType1;
      paxType1.paxType() = "ADT";

      // 7. Associate Fare to PaxTypeFare
      //
      paxFare1.setFare(&mockFare1);
      paxFare1.paxType() = &paxType1;
      paxFare1.fareMarket() = &fareMarket;

      paxTypeCortege.requestedPaxType() = &paxType1;
      // 8. Put PaxTypeFare instance in PaxTypeFare vector
      //
      LOG4CXX_DEBUG(_logger, "Inserting pax type fare into vector");
      paxTypeCortege.paxTypeFare().push_back(&paxFare1);

      // 9. Associate PaxTypeFare vector to FareMarket
      //
      LOG4CXX_DEBUG(_logger, "Inserting pax type fare vector into faremarket vector");
      LOG4CXX_DEBUG(_logger, "Size of paxTypeFareVec " << paxTypeCortege.paxTypeFare().size());

      paxTypeCortegeVec.push_back(paxTypeCortege);

      LOG4CXX_DEBUG(_logger, "Inserting fare market vector");
      fareMarkets.push_back(&fareMarket);

      //-------------------------
      //  ADD ANOTHER FARE MARKET
      //-------------------------

      // 1. Create another fare market
      //
      FareMarket fareMarket2;

      Loc zurich;
      zurich.nation() = "CH";
      LocCode zrhLoc("ZRH");
      zurich.loc() = zrhLoc;
      fareMarket2.origin() = (&dfw);
      fareMarket2.destination() = (&zurich);
      fareMarket2.governingCarrier() = ("AA");

      fareMarket2.geoTravelType() = GeoTravelType::International;

      AirSeg travelSeg1;
      travelSeg1.segmentOrder() = 0;
      travelSeg1.geoTravelType() = GeoTravelType::International;
      travelSeg1.origin() = &dfw;
      travelSeg1.destination() = &zurich;
      travelSeg1.departureDT() = travelDate;
      trx.travelSeg().push_back(&travelSeg1);

      // 3. Retrieve instance of PaxTypeBucket Vector
      //
      std::vector<PaxTypeBucket>& paxTypeCortegeVec2 = fareMarket2.paxTypeCortege();

      // 4. Create Instance of PaxTypeBucket
      //
      PaxTypeBucket paxTypeCortege2;

      // 5. Create PaxTypeFare
      //
      PaxTypeFare paxFare2;

      // 6. Create MockFare
      //
      FareInfo fareInfo2;
      fareInfo2._market1 = "DFW";
      fareInfo2._market2 = "ZRH";
      fareInfo2._currency = "USD";
      fareInfo2._carrier = "AA";
      fareInfo2._directionality = TO;

      Fare mockFare2;
      mockFare2.initialize(Fare::FS_International, &fareInfo2, fareMarket2);

      // 6a. Create PaxType
      //
      PaxType paxType2;
      paxType2.paxType() = "ADT";

      // 7. Associate Fare to PaxTypeFare
      //
      paxFare2.setFare(&mockFare2);
      paxFare2.paxType() = &paxType2;

      paxTypeCortege2.requestedPaxType() = &paxType2;
      // 8. Put another PaxTypeFare instance in
      //   PaxTypeFare vector.
      //
      paxTypeCortege2.paxTypeFare().push_back(&paxFare2);

      paxTypeCortegeVec2.push_back(paxTypeCortege2);

      fareMarkets.push_back(&fareMarket2);

      trx.itin().push_back(&itin);
      trx.getRequest()->ticketingDT() = DateTime::localTime();

      GeoTravelType itinTravelType = itin.geoTravelType();

      CurrencyCode alternateCurrency("BRL");

      bool selectRC = false;

      try
      {
        selectRC = fcs.selectAlternateCurrency(trx, fareMarket, alternateCurrency, itinTravelType);
      }
      catch (tse::ErrorResponseException& ex)
      {
        LOG4CXX_DEBUG(_logger,
                      "Alternate International Currency Selection caught Error Response exception: "
                          << ex.what());
      }

      LOG4CXX_DEBUG(
          _logger,
          "First FareMarket for International Select Alternate Currency Returned: " << selectRC);

      try
      {
        selectRC = fcs.selectAlternateCurrency(trx, fareMarket2, alternateCurrency, itinTravelType);
      }
      catch (tse::ErrorResponseException& ex)
      {
        LOG4CXX_DEBUG(_logger,
                      "Alternate International Currency Selection caught Error Response exception: "
                          << ex.what());
      }

      LOG4CXX_DEBUG(
          _logger,
          "Second FareMarket for International Select Alternate Currency Returned: " << selectRC);

      for (unsigned int i = 0; i < fareMarkets.size(); i++)
      {
        std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarkets[i]->paxTypeCortege();

        for (unsigned int j = 0; j < paxTypeCortegeVec.size(); j++)
        {
          PaxTypeBucket& paxTypeCortege = paxTypeCortegeVec[j];
          LOG4CXX_DEBUG(
              _logger,
              "InBound currency for this FareMarket: " << paxTypeCortege.inboundCurrency());
          LOG4CXX_DEBUG(
              _logger,
              "OutBound currency for this FareMarket: " << paxTypeCortege.outboundCurrency());
        }
      }

      LOG4CXX_DEBUG(_logger, "Completed test");
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception: " << ex.what()); }
  }

  void testSelectUSToCA()
  {
    try
    {
      MyDataHandle mdh;
      FareCurrencySelection fcs;
      PricingTrx trx;
      Itin itin;
      DataHandle dataHandle;

      PricingOptions options;
      PricingRequest request;
      trx.setOptions(&options);
      trx.setRequest(&request);
      trx.diagnostic().activate();
      trx.diagnostic().diagnosticType() = Diagnostic212;

      LocCode dfw("DFW");
      DateTime travelDate = DateTime::localTime();

      // LOG4CXX_DEBUG(_logger, "Location Code: "<<dfw);
      // LOG4CXX_DEBUG(_logger, "Travel Date: "<<travelDate);

      const tse::Loc* loc = dataHandle.getLoc(dfw, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {
        NationCode nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for DFW is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      if (!agent)
      {
        LOG4CXX_DEBUG(_logger, "Failed to retrieve agent ");
      }

      LOG4CXX_DEBUG(_logger, "Setting location");

      agent->agentLocation() = loc;

      trx.getRequest()->ticketingAgent() = agent;

      LOG4CXX_DEBUG(_logger, "Setting Agent location");

      // 1. Set Itin travel type
      //
      itin.geoTravelType() = GeoTravelType::Transborder;

      // 2. Retrieve FareMarkete vector from Itin
      std::vector<FareMarket*>& fareMarkets = itin.fareMarket();

      const Loc* nyc = dataHandle.getLoc("NYC", DateTime::localTime());
      const Loc* yvr = dataHandle.getLoc("YVR", DateTime::localTime());

      AirSeg travelSeg;
      travelSeg.segmentOrder() = 0;
      travelSeg.geoTravelType() = GeoTravelType::Transborder;
      travelSeg.origin() = nyc;
      travelSeg.destination() = yvr;
      travelSeg.departureDT() = travelDate;
      trx.travelSeg().push_back(&travelSeg);
      trx.getRequest()->ticketingDT() = travelDate;

      // 3. Create fare market
      //
      FareMarket fareMarket;

      fareMarket.origin() = nyc;
      fareMarket.boardMultiCity() = nyc->loc();
      fareMarket.destination() = yvr;
      fareMarket.offMultiCity() = yvr->loc();
      fareMarket.governingCarrier() = ("AA");
      fareMarket.geoTravelType() = GeoTravelType::Transborder;
      GlobalDirection gd = GlobalDirection::WH;
      fareMarket.setGlobalDirection(gd);

      // 4. Retrieve instance of PaxTypeBucket vector
      //
      std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();

      // 5. Create instance of PaxTypeBucket
      //
      PaxTypeBucket paxTypeCortege;

      // 6. Create PaxTypeFare
      //
      PaxTypeFare paxFare1;

      // 7. Create Fare
      //
      FareInfo fareInfo;
      fareInfo._carrier = "AA";
      fareInfo._market1 = "NYC";
      fareInfo._market2 = "YVR";
      fareInfo._fareClass = "YO1";
      fareInfo._fareAmount = 600.00;
      fareInfo._currency = "USD";
      fareInfo._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
      fareInfo._directionality = FROM;
      fareInfo._globalDirection = GlobalDirection::WH;

      TariffCrossRefInfo tariffRefInfo;

      Fare mockFare1;
      mockFare1.initialize(Fare::FS_Transborder, &fareInfo, fareMarket, &tariffRefInfo);

      // 7a. Create PaxType
      //
      PaxType paxType1;
      paxType1.paxType() = "ADT";

      // 8. Associate Fare to PaxTypeFare
      //
      paxFare1.setFare(&mockFare1);
      paxFare1.paxType() = &paxType1;
      paxFare1.fareMarket() = &fareMarket;

      paxFare1.setIsShoppingFare(); // To make it pass PaxTypeFare::isValid()

      paxTypeCortege.requestedPaxType() = &paxType1;
      // 8. Put PaxTypeFare in PaxTypeBucket PaxTypeFare vector
      //
      LOG4CXX_DEBUG(_logger, "Inserting pax type fare into vector");
      paxTypeCortege.paxTypeFare().push_back(&paxFare1);

      // 9. Put PaxTypeBucket into FareMarket PaxTypeBucket vector
      //
      LOG4CXX_DEBUG(_logger, "Inserting pax type fare vector into faremarket vector");
      LOG4CXX_DEBUG(_logger, "Size of paxTypeFareVec " << paxTypeCortege.paxTypeFare().size());
      // LOG4CXX_DEBUG(_logger, "Size of paxFareVec "<<fareMarket._paxTypeFare.size());

      paxTypeCortegeVec.push_back(paxTypeCortege);

      LOG4CXX_DEBUG(_logger, "Inserting fare market vector");

      // 10. Put FareMarket into Itin FareMarket vector
      fareMarkets.push_back(&fareMarket);

      //-------------------------
      //  ADD ANOTHER FARE MARKET
      //-------------------------

      // 1. Create another fare market
      //
      FareMarket fareMarket2;

      const Loc* nyc2 = dataHandle.getLoc("NYC", DateTime::localTime());

      fareMarket2.origin() = yvr;
      fareMarket2.boardMultiCity() = yvr->loc();
      fareMarket2.destination() = nyc2;
      fareMarket2.offMultiCity() = nyc2->loc();
      fareMarket2.governingCarrier() = "AA";
      fareMarket2.setGlobalDirection(gd);

      AirSeg travelSeg1;
      travelSeg1.segmentOrder() = 1;
      travelSeg1.geoTravelType() = GeoTravelType::Transborder;
      travelSeg1.origin() = yvr;
      travelSeg1.destination() = nyc2;
      travelSeg1.departureDT() = travelDate;
      trx.travelSeg().push_back(&travelSeg1);

      fareMarket2.geoTravelType() = GeoTravelType::Transborder;

      // 2. Retrieve PaxTypeBucket for this FareMarket
      //
      std::vector<PaxTypeBucket>& paxTypeCortegeVec2 = fareMarket2.paxTypeCortege();

      // 3. Create instance of PaxTypeBucket
      //
      PaxTypeBucket paxTypeCortege2;

      // 4. Create PaxTypeFare
      //
      PaxTypeFare paxFare2;

      // 5. Create FareInfo
      //
      FareInfo fareInfo2;
      fareInfo2._market1 = "YVR";
      fareInfo2._market2 = "NYC";
      fareInfo2._currency = "CAD";
      fareInfo2._carrier = "AA";
      fareInfo2._directionality = TO;
      LOG4CXX_DEBUG(_logger, "Value of TO = " << TO);

      // 6. Create MockFare2
      //
      Fare mockFare2;
      mockFare2.initialize(Fare::FS_Transborder, &fareInfo2, fareMarket2);

      // 7. Create PaxType
      //
      PaxType paxType2;
      paxType2.paxType() = "ADT";

      // 8. Associate Fare to PaxTypeFare
      //
      paxFare2.setFare(&mockFare2);
      paxFare2.paxType() = &paxType2;
      paxFare2.fareMarket() = &fareMarket2;

      paxFare2.setIsShoppingFare(); // To make it pass PaxTypeFare::isValid()

      paxTypeCortege2.requestedPaxType() = &paxType2;
      // 9. Put PaxTypeFare instance in
      //   PaxTypeFare vector.
      //
      paxTypeCortege2.paxTypeFare().push_back(&paxFare2);

      // 9. Put PaxTypeBucket instance in
      //   PaxTypeBucket  vector for 2nd FareMarket.
      //
      paxTypeCortegeVec2.push_back(paxTypeCortege2);

      fareMarkets.push_back(&fareMarket2);

      itin.originationCurrency() = "USD";

      trx.itin().push_back(&itin);

      NationCode originNation("US");

      fcs.selectUSCACurrency(trx, originNation, fareMarket);

      fcs.selectUSCACurrency(trx, originNation, fareMarket2);

      // Test PFA Cruise Agent with cruisePfaCur
      Customer agentTJR;
      agentTJR.cruisePfaCur() = "EUR";

      agent->agentTJR() = &agentTJR;
      request.ticketingAgent() = agent;

      PaxType paxTypePFA;
      paxTypePFA.paxType() = "PFA";
      PaxTypeBucket paxTypeCortegePFA;
      paxTypeCortegePFA.requestedPaxType() = &paxTypePFA;
      paxTypeCortegePFA.paxTypeFare().push_back(&paxFare1);
      paxTypeCortegeVec.push_back(paxTypeCortegePFA);

      fareMarket.currencies().clear();
      fcs.selectUSCACurrency(trx, originNation, fareMarket);

      // std::string str = trx.diagnostic().toString();
      // std::cout << std::endl << str.c_str() << std::endl;

      LOG4CXX_DEBUG(_logger, "Completed test");
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception: " << ex.what()); }
  }

  void testSelectCAToUS()
  {
    try
    {
      FareCurrencySelection fcs;
      PricingTrx trx;
      Itin itin;
      DataHandle dataHandle;

      PricingOptions options;
      PricingRequest request;
      trx.setOptions(&options);
      trx.setRequest(&request);
      trx.diagnostic().activate();
      trx.diagnostic().diagnosticType() = Diagnostic212;

      LocCode dfw("DFW");
      DateTime travelDate;

      // LOG4CXX_DEBUG(_logger, "Location Code: "<<dfw);
      // LOG4CXX_DEBUG(_logger, "Travel Date: "<<travelDate);

      const tse::Loc* loc = dataHandle.getLoc(dfw, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {
        NationCode nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for DFW is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      if (!agent)
      {
        LOG4CXX_DEBUG(_logger, "Failed to retrieve agent ");
      }

      LOG4CXX_DEBUG(_logger, "Setting location");

      agent->agentLocation() = loc;

      trx.getRequest()->ticketingAgent() = agent;

      LOG4CXX_DEBUG(_logger, "Setting Agent location");

      // 1. Set Itin travel type
      //
      itin.geoTravelType() = GeoTravelType::Transborder;

      // 2. Retrieve FareMarkete vector from Itin
      std::vector<FareMarket*>& fareMarkets = itin.fareMarket();

      Loc nyc;
      Loc yvr;
      LocCode nycLoc("NYC");
      LocCode yvrLoc("YVR");
      nyc.loc() = nycLoc; //
      nyc.nation() = "US";
      yvr.loc() = yvrLoc; //
      yvr.nation() = "CA";

      AirSeg travelSeg;
      travelSeg.segmentOrder() = 0;
      travelSeg.geoTravelType() = GeoTravelType::Transborder;
      travelSeg.origin() = &yvr;
      travelSeg.destination() = &nyc;
      travelSeg.departureDT() = travelDate;
      trx.travelSeg().push_back(&travelSeg);

      // 3. Create fare market
      //
      FareMarket fareMarket;

      fareMarket.origin() = (&yvr);
      fareMarket.destination() = (&nyc);
      fareMarket.governingCarrier() = ("AA");
      fareMarket.geoTravelType() = GeoTravelType::Transborder;

      // 4. Retrieve instance of PaxTypeBucket vector
      //
      std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();

      // 5. Create instance of PaxTypeBucket
      //
      PaxTypeBucket paxTypeCortege;

      // 6. Create PaxTypeFare
      //
      PaxTypeFare paxFare1;

      // 7. Create Fare
      //
      FareInfo fareInfo;
      fareInfo._market1 = "YVR";
      fareInfo._market2 = "NYC";
      fareInfo._currency = "CAD";
      fareInfo._carrier = "AA";
      fareInfo._directionality = FROM;

      Fare mockFare1;
      mockFare1.initialize(Fare::FS_Transborder, &fareInfo, fareMarket);

      // 7a. Create PaxType
      //
      PaxType paxType1;
      paxType1.paxType() = "ADT";

      // 8. Associate Fare to PaxTypeFare
      //
      paxFare1.setFare(&mockFare1);
      paxFare1.paxType() = &paxType1;
      paxFare1.fareMarket() = &fareMarket;

      paxTypeCortege.requestedPaxType() = &paxType1;
      // 8. Put PaxTypeFare in PaxTypeBucket PaxTypeFare vector
      //
      LOG4CXX_DEBUG(_logger, "Inserting pax type fare into vector");
      paxTypeCortege.paxTypeFare().push_back(&paxFare1);

      // 9. Put PaxTypeBucket into FareMarket PaxTypeBucket vector
      //
      LOG4CXX_DEBUG(_logger, "Inserting pax type fare vector into faremarket vector");
      LOG4CXX_DEBUG(_logger, "Size of paxTypeFareVec " << paxTypeCortege.paxTypeFare().size());
      // LOG4CXX_DEBUG(_logger, "Size of paxFareVec "<<fareMarket._paxTypeFare.size());

      paxTypeCortegeVec.push_back(paxTypeCortege);

      LOG4CXX_DEBUG(_logger, "Inserting fare market vector");

      // 10. Put FareMarket into Itin FareMarket vector
      fareMarkets.push_back(&fareMarket);

      //-------------------------
      //  ADD ANOTHER FARE MARKET
      //-------------------------

      // 1. Create another fare market
      //
      FareMarket fareMarket2;

      Loc nyc2;
      nyc2.nation() = "US";
      LocCode nyc2Loc("NYC");
      nyc2.loc() = nyc2Loc;
      fareMarket2.origin() = (&nyc2);
      fareMarket2.destination() = (&yvr);
      fareMarket2.governingCarrier() = ("AA");

      AirSeg travelSeg1;
      travelSeg1.segmentOrder() = 1;
      travelSeg1.geoTravelType() = GeoTravelType::Transborder;
      travelSeg1.origin() = &nyc2;
      travelSeg1.destination() = &yvr;
      travelSeg1.departureDT() = travelDate;
      trx.travelSeg().push_back(&travelSeg1);

      fareMarket2.geoTravelType() = GeoTravelType::ForeignDomestic;

      // 2. Retrieve PaxTypeBucket for this FareMarket
      //
      std::vector<PaxTypeBucket>& paxTypeCortegeVec2 = fareMarket2.paxTypeCortege();

      // 3. Create instance of PaxTypeBucket
      //
      PaxTypeBucket paxTypeCortege2;

      // 4. Create PaxTypeFare
      //
      PaxTypeFare paxFare2;

      // 5. Create FareInfo
      //
      FareInfo fareInfo2;
      fareInfo2._market1 = "NYC";
      fareInfo2._market2 = "YVR";
      fareInfo2._currency = "USD";
      fareInfo2._carrier = "AA";
      fareInfo2._directionality = TO;
      LOG4CXX_DEBUG(_logger, "Value of TO = " << TO);

      // 6. Create MockFare2
      //
      Fare mockFare2;
      mockFare2.initialize(Fare::FS_Transborder, &fareInfo2, fareMarket2);

      // 7. Create PaxType
      //
      PaxType paxType2;
      paxType2.paxType() = "ADT";

      // 8. Associate Fare to PaxTypeFare
      //
      paxFare2.setFare(&mockFare2);
      paxFare2.paxType() = &paxType2;
      paxFare2.fareMarket() = &fareMarket2;

      paxTypeCortege2.requestedPaxType() = &paxType2;
      // 9. Put PaxTypeFare instance in
      //   PaxTypeFare vector.
      //
      paxTypeCortege2.paxTypeFare().push_back(&paxFare2);

      // 9. Put PaxTypeBucket instance in
      //   PaxTypeBucket  vector for 2nd FareMarket.
      //
      paxTypeCortegeVec2.push_back(paxTypeCortege2);

      fareMarkets.push_back(&fareMarket2);

      trx.itin().push_back(&itin);

      NationCode originNation("CA");

      fcs.selectUSCACurrency(trx, originNation, fareMarket);

      // LOG4CXX_DEBUG(_logger, "First FareMarket for CA-US Select Currency Returned: "<<selectRC);

      fcs.selectUSCACurrency(trx, originNation, fareMarket2);

      // LOG4CXX_DEBUG(_logger, "Second FareMarket for CA-US Select Currency Returned: "<<selectRC);

      for (unsigned int i = 0; i < fareMarkets.size(); i++)
      {
        std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarkets[i]->paxTypeCortege();

        for (unsigned int j = 0; j < paxTypeCortegeVec.size(); j++)
        {
          PaxTypeBucket& paxTypeCortege = paxTypeCortegeVec[j];
          LOG4CXX_DEBUG(
              _logger,
              "InBound currency for this FareMarket: " << paxTypeCortege.inboundCurrency());
          LOG4CXX_DEBUG(
              _logger,
              "OutBound currency for this FareMarket: " << paxTypeCortege.outboundCurrency());
        }
      }

      LOG4CXX_DEBUG(_logger, "Completed test");
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception: " << ex.what()); }
  }

  void testSelectCAToUSWithDomesticCA()
  {
    try
    {
      FareCurrencySelection fcs;
      PricingTrx trx;
      Itin itin;
      DataHandle dataHandle;

      PricingOptions options;
      PricingRequest request;
      trx.setOptions(&options);
      trx.setRequest(&request);
      trx.diagnostic().activate();
      trx.diagnostic().diagnosticType() = Diagnostic212;

      LocCode yvr("YVR");
      DateTime travelDate;

      // LOG4CXX_DEBUG(_logger, "Location Code: "<<dfw);
      // LOG4CXX_DEBUG(_logger, "Travel Date: "<<travelDate);

      const tse::Loc* loc = dataHandle.getLoc(yvr, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (loc)
      {
        NationCode nationCode = loc->nation();
        LOG4CXX_DEBUG(_logger, "Nation code for YVR is: " << nationCode);
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      if (!agent)
      {
        LOG4CXX_DEBUG(_logger, "Failed to retrieve agent ");
      }

      LOG4CXX_DEBUG(_logger, "Setting location");

      agent->agentLocation() = loc;

      trx.getRequest()->ticketingAgent() = agent;

      LOG4CXX_DEBUG(_logger, "Setting Agent location");

      // 1. Set Itin travel type
      //
      itin.geoTravelType() = GeoTravelType::Transborder;

      // 2. Retrieve FareMarkete vector from Itin
      std::vector<FareMarket*>& fareMarkets = itin.fareMarket();

      Loc nyc;
      Loc yvrLoc;
      LocCode nycLoc("NYC");
      nyc.loc() = nycLoc; //
      nyc.nation() = "US";
      yvrLoc.loc() = yvr; //
      yvrLoc.nation() = "CA";

      AirSeg travelSeg;
      travelSeg.segmentOrder() = 0;
      travelSeg.geoTravelType() = GeoTravelType::Transborder;
      travelSeg.origin() = &yvrLoc;
      travelSeg.destination() = &nyc;
      travelSeg.departureDT() = travelDate;
      trx.travelSeg().push_back(&travelSeg);

      // 3. Create fare market
      //
      FareMarket fareMarket;

      fareMarket.origin() = (&yvrLoc);
      fareMarket.destination() = (&nyc);
      fareMarket.governingCarrier() = ("AA");
      fareMarket.geoTravelType() = GeoTravelType::Transborder;

      // 4. Retrieve instance of PaxTypeBucket vector
      //
      std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();

      // 5. Create instance of PaxTypeBucket
      //
      PaxTypeBucket paxTypeCortege;

      // 6. Create PaxTypeFare
      //
      PaxTypeFare paxFare1;

      // 7. Create Fare
      //
      FareInfo fareInfo;
      fareInfo._market1 = "YVR";
      fareInfo._market2 = "NYC";
      fareInfo._currency = "CAD";
      fareInfo._carrier = "AA";
      fareInfo._directionality = FROM;

      Fare mockFare1;
      mockFare1.initialize(Fare::FS_Transborder, &fareInfo, fareMarket);

      // 7a. Create PaxType
      //
      PaxType paxType1;
      paxType1.paxType() = "ADT";

      // 8. Associate Fare to PaxTypeFare
      //
      paxFare1.setFare(&mockFare1);
      paxFare1.paxType() = &paxType1;
      paxFare1.fareMarket() = &fareMarket;

      paxTypeCortege.requestedPaxType() = &paxType1;
      // 8. Put PaxTypeFare in PaxTypeBucket PaxTypeFare vector
      //
      LOG4CXX_DEBUG(_logger, "Inserting pax type fare into vector");
      paxTypeCortege.paxTypeFare().push_back(&paxFare1);

      // 9. Put PaxTypeBucket into FareMarket PaxTypeBucket vector
      //
      LOG4CXX_DEBUG(_logger, "Inserting pax type fare vector into faremarket vector");
      LOG4CXX_DEBUG(_logger, "Size of paxTypeFareVec " << paxTypeCortege.paxTypeFare().size());
      // LOG4CXX_DEBUG(_logger, "Size of paxFareVec "<<fareMarket._paxTypeFare.size());

      paxTypeCortegeVec.push_back(paxTypeCortege);

      LOG4CXX_DEBUG(_logger, "Inserting fare market vector");

      // 10. Put FareMarket into Itin FareMarket vector
      fareMarkets.push_back(&fareMarket);

      //-------------------------
      //  ADD ANOTHER FARE MARKET
      //-------------------------

      // 1. Create another fare market
      //
      FareMarket fareMarket2;

      Loc laxLoc;
      laxLoc.nation() = "US";
      LocCode lax("LAX");
      laxLoc.loc() = lax;
      fareMarket2.origin() = (&nyc);
      fareMarket2.destination() = (&laxLoc);
      fareMarket2.governingCarrier() = ("AA");

      AirSeg travelSeg1;
      travelSeg1.segmentOrder() = 1;
      travelSeg1.geoTravelType() = GeoTravelType::Domestic;
      travelSeg1.origin() = &nyc;
      travelSeg1.destination() = &laxLoc;
      travelSeg1.departureDT() = travelDate;
      trx.travelSeg().push_back(&travelSeg1);

      fareMarket2.geoTravelType() = GeoTravelType::Domestic;

      // 2. Retrieve PaxTypeBucket for this FareMarket
      //
      std::vector<PaxTypeBucket>& paxTypeCortegeVec2 = fareMarket2.paxTypeCortege();

      // 3. Create instance of PaxTypeBucket
      //
      PaxTypeBucket paxTypeCortege2;

      // 4. Create PaxTypeFare
      //
      PaxTypeFare paxFare2;

      // 5. Create FareInfo
      //
      FareInfo fareInfo2;
      fareInfo2._market1 = "NYC";
      fareInfo2._market2 = "LAX";
      fareInfo2._currency = "USD";
      fareInfo2._carrier = "AA";
      fareInfo2._directionality = TO;
      LOG4CXX_DEBUG(_logger, "Value of TO = " << TO);

      // 6. Create MockFare2
      //
      Fare mockFare2;
      mockFare2.initialize(Fare::FS_Domestic, &fareInfo2, fareMarket2);

      // 7. Create PaxType
      //
      PaxType paxType2;
      paxType2.paxType() = "ADT";

      // 8. Associate Fare to PaxTypeFare
      //
      paxFare2.setFare(&mockFare2);
      paxFare2.paxType() = &paxType2;
      paxFare2.fareMarket() = &fareMarket2;

      paxTypeCortege2.requestedPaxType() = &paxType2;
      // 9. Put PaxTypeFare instance in
      //   PaxTypeFare vector.
      //
      paxTypeCortege2.paxTypeFare().push_back(&paxFare2);

      // 9. Put PaxTypeBucket instance in
      //   PaxTypeBucket  vector for 2nd FareMarket.
      //
      paxTypeCortegeVec2.push_back(paxTypeCortege2);

      fareMarkets.push_back(&fareMarket2);

      trx.itin().push_back(&itin);

      NationCode originNation("CA");

      fcs.selectUSCACurrency(trx, originNation, fareMarket);

      // LOG4CXX_DEBUG(_logger, "First FareMarket for CA-US Select Currency Returned: "<<selectRC);

      fcs.selectUSCACurrency(trx, originNation, fareMarket2);

      // LOG4CXX_DEBUG(_logger, "Second FareMarket for CA-US Select Currency Returned: "<<selectRC);

      for (unsigned int i = 0; i < fareMarkets.size(); i++)
      {
        std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarkets[i]->paxTypeCortege();

        for (unsigned int j = 0; j < paxTypeCortegeVec.size(); j++)
        {
          PaxTypeBucket& paxTypeCortege = paxTypeCortegeVec[j];
          LOG4CXX_DEBUG(
              _logger,
              "InBound currency for this FareMarket: " << paxTypeCortege.inboundCurrency());
          LOG4CXX_DEBUG(
              _logger,
              "OutBound currency for this FareMarket: " << paxTypeCortege.outboundCurrency());
        }
      }

      LOG4CXX_DEBUG(_logger, "Completed test");
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception: " << ex.what()); }
  }

private:
  static log4cxx::LoggerPtr _logger;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareCurrencySelectionTest);

log4cxx::LoggerPtr
FareCurrencySelectionTest::_logger(
    log4cxx::Logger::getLogger("atseintl.Fares.test.FareCurrencySelectionTest"));
}
