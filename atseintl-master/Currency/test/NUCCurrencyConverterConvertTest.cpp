#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/ErrorResponseException.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/NUCCollectionResults.h"
#include "Common/NUCCurrencyConverter.h"
#include "Common/StopWatch.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Currency/test/MockDataHandle.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/StaticObjectPool.h"
#include "Pricing/test/MockFarePath.h"
#include "Pricing/test/MockLoc.h"
#include "Server/TseServer.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

#include <algorithm>
#include <iostream>
#include <string>

using namespace std;

namespace tse
{

class NUCCurrencyConverterConvertTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NUCCurrencyConverterConvertTest);
  CPPUNIT_TEST(convert);
  CPPUNIT_TEST(convertForeignDomesticBaseFare);
  CPPUNIT_TEST(convertInternationalBaseFare);
  CPPUNIT_TEST(testDomesticRounding);
  CPPUNIT_TEST(testInternationalRounding);
  CPPUNIT_TEST(testInvalidInput);
  CPPUNIT_TEST(testHasArunkSegments);
  CPPUNIT_TEST(testConversionFacade);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _mockDataHandle = _memHandle.create<CurrencyDataHandle>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void convert()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();

      NUCCurrencyConverter nucConverter;
      NUCCollectionResults results;

      std::string nucStr("NUC");
      std::string jpyStr("JPY");

      tse::Money nuc(0.0, nucStr);
      tse::Money jpy(100, "JPY");
      bool convertRC = false;

      LOG4CXX_INFO(_logger, "Number of nuc decimals: " << nuc.noDec());

      DateTime travelDate = DateTime::localTime();
      PricingTrx trx;
      PricingRequest request;
      trx.setRequest(&request);

      CurrencyConversionRequest request1(
          nuc, jpy, travelDate, *(trx.getRequest()), trx.dataHandle());

      // convertRC = nucConverter.convert(nuc,jpy,travelDate);
      convertRC = nucConverter.convert(request1, &results);

      LOG4CXX_INFO(_logger, "Converted JPY Amount in NUCs: " << nuc.value());

      CPPUNIT_ASSERT(convertRC != false);

      jpy.value() = 0.0;

      CurrencyConversionRequest request2(
          jpy, nuc, travelDate, *(trx.getRequest()), trx.dataHandle());
      // convertRC = nucConverter.convert(jpy,nuc,travelDate);

      convertRC = nucConverter.convert(request2, &results);

      LOG4CXX_INFO(_logger, "Converted NUC Amount in rounded  JPY: " << jpy.value());

      CPPUNIT_ASSERT(convertRC != false);

      nuc.value() = 0.0;

      LOG4CXX_INFO(_logger, "Invoking NUCCurrencyConverter to convert 10 INR  to NUCs");

      std::string inrStr("INR");

      Money inr(10.0, inrStr);

      CurrencyConversionRequest request3(
          nuc, inr, travelDate, *(trx.getRequest()), trx.dataHandle());

      // convertRC = nucConverter.convert(nuc,inr,travelDate);
      convertRC = nucConverter.convert(request3, &results);

      LOG4CXX_INFO(_logger, "Converted INR Amount in NUCs: " << nuc.value());

      LOG4CXX_INFO(_logger, "Invoking NUCCurrencyConverter to convert NUCs to INR");

      CurrencyConversionRequest request4(
          inr, nuc, travelDate, *(trx.getRequest()), trx.dataHandle());

      // convertRC = nucConverter.convert(inr,nuc,travelDate);
      convertRC = nucConverter.convert(request4, &results);

      LOG4CXX_INFO(_logger, "Converted NUC Amount in rounded  INR's: " << inr.value());

      CPPUNIT_ASSERT(convertRC != false);

      LOG4CXX_INFO(_logger, "Invoking NUCCurrencyConverter to convert 100 BRL's  to NUCs");

      std::string brlStr("BRL");

      Money brl(100.0, brlStr);

      nuc.value() = 0.0;

      CurrencyConversionRequest request5(
          nuc, brl, travelDate, *(trx.getRequest()), trx.dataHandle());

      // convertRC = nucConverter.convert(nuc,brl,travelDate);
      convertRC = nucConverter.convert(request5, &results);

      LOG4CXX_INFO(_logger, "Converted BRL Amount in NUCs: " << nuc.value());

      LOG4CXX_INFO(_logger, "Invoking NUCCurrencyConverter to convert NUCs to BRL");

      CurrencyConversionRequest request6(
          brl, nuc, travelDate, *(trx.getRequest()), trx.dataHandle());

      // convertRC = nucConverter.convert(brl,nuc,travelDate);
      convertRC = nucConverter.convert(request6, &results);

      LOG4CXX_INFO(_logger, "Converted NUC Amount in rounded  BRL's:  " << brl.value());

      CPPUNIT_ASSERT(convertRC != false);

      LOG4CXX_INFO(_logger, "Invoking NUCCurrencyConverter to convert 95.67 GBP's  to NUCs");

      std::string gbpStr("GBP");

      Money gbp(95.67, gbpStr);

      nuc.value() = 0.0;

      CurrencyConversionRequest request7(
          nuc, gbp, travelDate, *(trx.getRequest()), trx.dataHandle());

      // convertRC = nucConverter.convert(nuc,gbp,travelDate);

      convertRC = nucConverter.convert(request7, &results);

      LOG4CXX_INFO(_logger, "Converted GBP Amount in NUCs: " << nuc.value());

      LOG4CXX_INFO(_logger, "Invoking NUCCurrencyConverter to convert NUCs to rounded  GBP");

      CurrencyConversionRequest request8(
          gbp, nuc, travelDate, *(trx.getRequest()), trx.dataHandle());

      // convertRC = nucConverter.convert(gbp,nuc,travelDate);

      convertRC = nucConverter.convert(request8, &results);

      LOG4CXX_INFO(_logger, "Converted NUC Amount in GBP's: " << gbp.value());

      CPPUNIT_ASSERT(convertRC != false);

      /*
       LOG4CXX_INFO(_logger, "Testing NUCCurrencyConverter valid input");

       Money gbpMoney("GBP");

       //NUC Currency but no value
       Money nucMoney("NUC");

       convertRC = nucConverter.convert(gbpMoney,nucMoney,travelDate);

       CPPUNIT_ASSERT(convertRC != false);

       LOG4CXX_INFO(_logger, "Convert returned : "<<convertRC);
       */

      stopWatch.stop();

      LOG4CXX_INFO(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_INFO(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_INFO(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_INFO(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_INFO(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_INFO(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void convertForeignDomesticBaseFare()
  {
    try
    {
      LOG4CXX_INFO(_logger, "Entered convertForeignDomesticBaseFare");

      tse::StopWatch stopWatch;
      stopWatch.start();

      // DateTime travelDate(theYear,theMonth,theDay);
      DateTime travelDate = DateTime::localTime();

      NUCCurrencyConverter nucConverter;

      // Create FarePath and Itin
      FarePath farePath;
      Itin itin;
      itin.geoTravelType() = GeoTravelType::ForeignDomestic;

      farePath.itin() = &itin;

      PricingTrx trx;
      trx.itin().push_back(&itin);

      // Create PU,
      farePath.setTotalNUCAmount(236.64);
      farePath.baseFareCurrency() = "INR";
      PricingUnit pricingUnit;
      FareUsage fareUsage;
      PaxTypeFare paxTypeFare1;

      // Create FareInfo
      //
      FareInfo fareInfo;
      fareInfo._market1 = "BOM";
      fareInfo._market2 = "DEL";
      fareInfo._currency = "INR";
      fareInfo._carrier = "9W";
      fareInfo._directionality = FROM;
      FareMarket fm1;

      // Create Fare
      //
      Fare fare1;
      fare1.initialize(Fare::FS_ForeignDomestic, &fareInfo, fm1);

      MockLoc bom;
      bom.nation() = "IN";
      LocCode bomLoc("BOM");
      bom.loc() = bomLoc;

      MockLoc del;
      del.nation() = "IN";
      LocCode delLoc("DEL");
      del.loc() = delLoc;

      // Create TravelSeg
      //
      AirSeg travelSeg1;
      travelSeg1.segmentOrder() = 0;
      travelSeg1.geoTravelType() = GeoTravelType::ForeignDomestic;
      travelSeg1.origin() = &bom;
      travelSeg1.destination() = &del;
      travelSeg1.departureDT() = travelDate;
      itin.travelSeg().push_back(&travelSeg1);

      // Create Associations
      //
      paxTypeFare1.setFare(&fare1);
      fareUsage.paxTypeFare() = &paxTypeFare1;
      pricingUnit.fareUsage().push_back(&fareUsage);
      farePath.pricingUnit().push_back(&pricingUnit);

      MoneyAmount convertedAmount;
      CurrencyNoDec convertedAmtNoDec;
      CurrencyCode convertedCurrencyCode;
      ExchRate roeRate;
      CurrencyNoDec roeRateNoDec;
      DateTime nucEffectiveDate;
      DateTime nucDiscontinueDate;
      PricingRequest request;
      request.ticketingDT() = travelDate;
      trx.setRequest(&request);
      DataHandle dataHandle;

      const tse::Loc* loc = dataHandle.getLoc(bomLoc, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (!loc)
      {
        CPPUNIT_FAIL("Allocation of Loc Object Failed");
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();
      agent->agentLocation() = loc;

      request.ticketingAgent() = agent;
      bool convertRC = false;

      convertRC = nucConverter.convertBaseFare(trx,
                                               farePath,
                                               farePath.getTotalNUCAmount(),
                                               convertedAmount,
                                               convertedAmtNoDec,
                                               convertedCurrencyCode,
                                               roeRate,
                                               roeRateNoDec,
                                               nucEffectiveDate,
                                               nucDiscontinueDate);

      LOG4CXX_DEBUG(_logger, "Converted base fare amount: " << convertedAmount);
      LOG4CXX_DEBUG(_logger, "Converted base fare amount no dec: " << convertedAmtNoDec);
      LOG4CXX_DEBUG(_logger, "Converted base fare currency code: " << convertedCurrencyCode);
      LOG4CXX_DEBUG(_logger, "Nuc roe: " << roeRate);
      LOG4CXX_DEBUG(_logger, "Nuc roe nodec: " << roeRateNoDec);
      LOG4CXX_DEBUG(_logger, "Nuc effective date: " << nucEffectiveDate.toSimpleString());
      LOG4CXX_DEBUG(_logger, "Nuc discontinue date: " << nucDiscontinueDate.toSimpleString());

      CPPUNIT_ASSERT(convertRC != false);

      stopWatch.stop();

      LOG4CXX_INFO(_logger, "Leaving convertForeignDomesticBaseFare");

      LOG4CXX_INFO(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_INFO(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_INFO(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_INFO(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_INFO(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_INFO(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void convertInternationalBaseFare()
  {
    try
    {
      LOG4CXX_INFO(_logger, "Entered convertInternationalBaseFare");

      tse::StopWatch stopWatch;
      stopWatch.start();
      DataHandle dataHandle;

      Year theYear(2004);
      Month theMonth(8);
      Day theDay(20);

      DateTime travelDate(theYear, theMonth, theDay);

      NUCCurrencyConverter nucConverter;

      // Create FarePath and Itin
      FarePath farePath;
      Itin itin;
      itin.geoTravelType() = GeoTravelType::International;
      farePath.itin() = &itin;
      farePath.baseFareCurrency() = "INR";

      itin.farePath().push_back(&farePath);

      PricingTrx trx;
      PricingRequest request;
      trx.itin().push_back(&itin);

      MockLoc del;
      del.nation() = "IN";
      LocCode delLoc("DEL");
      del.loc() = delLoc;

      const tse::Loc* loc = dataHandle.getLoc(delLoc, travelDate);

      LOG4CXX_DEBUG(_logger, "Retrieved location ");

      if (!loc)
      {
        CPPUNIT_FAIL("Allocation of Loc Object Failed");
      }

      tse::Agent* agent = tse::StaticObjectPool<Agent>::unsafe_get();

      agent->agentLocation() = loc;

      request.ticketingAgent() = agent;
      trx.setRequest(&request);
      request.ticketingDT() = travelDate;

      // Create PU,
      farePath.setTotalNUCAmount(312.32); // DEL - LON - DFW totals
      PricingUnit pricingUnit;
      FareUsage fareUsage;
      PaxTypeFare paxTypeFare1;

      // Create FareInfo
      //
      FareInfo fareInfo;
      fareInfo._market1 = "DEL";
      fareInfo._market2 = "LON";
      fareInfo._currency = "INR";
      fareInfo._carrier = "9W";
      fareInfo._directionality = FROM;

      // Create Fare
      //
      Fare fare1;
      FareMarket fm1;
      fare1.initialize(Fare::FS_ForeignDomestic, &fareInfo, fm1);

      MockLoc lon;
      lon.nation() = "GB";
      LocCode lonLoc("LON");
      lon.loc() = lonLoc;

      // Create TravelSeg
      //
      AirSeg travelSeg1;
      travelSeg1.segmentOrder() = 1;
      travelSeg1.geoTravelType() = GeoTravelType::International;
      travelSeg1.origin() = &del;
      travelSeg1.destination() = &lon;
      travelSeg1.departureDT() = travelDate;
      itin.travelSeg().push_back(&travelSeg1);

      // Create Associations
      //
      paxTypeFare1.setFare(&fare1);
      fareUsage.paxTypeFare() = &paxTypeFare1;
      pricingUnit.fareUsage().push_back(&fareUsage);
      //   farePath.pricingUnit().push_back(&pricingUnit);

      // Create second PricingUnit
      //
      //  PricingUnit pricingUnit2;
      FareUsage fareUsage2;
      PaxTypeFare paxTypeFare2;

      // Create FareInfo
      //
      FareInfo fareInfo2;
      fareInfo2._market1 = "LON";
      fareInfo2._market2 = "DFW";
      fareInfo2._currency = "GBP";
      fareInfo2._carrier = "BA";
      fareInfo2._directionality = FROM;

      // Create Fare
      //
      Fare fare2;
      FareMarket fm2;
      fare2.initialize(Fare::FS_International, &fareInfo2, fm2);

      MockLoc dfw;
      dfw.nation() = "US";
      LocCode dfwLoc("DFW");
      dfw.loc() = dfwLoc;

      // Create TravelSeg
      //
      AirSeg travelSeg2;
      travelSeg2.segmentOrder() = 2;
      travelSeg2.geoTravelType() = GeoTravelType::International;
      travelSeg2.origin() = &lon;
      travelSeg2.destination() = &dfw;
      travelSeg2.departureDT() = travelDate;
      itin.travelSeg().push_back(&travelSeg2);

      // Create Associations
      //
      paxTypeFare2.setFare(&fare2);
      fareUsage2.paxTypeFare() = &paxTypeFare2;
      pricingUnit.fareUsage().push_back(&fareUsage2);
      farePath.pricingUnit().push_back(&pricingUnit);

      MoneyAmount convertedAmount;
      CurrencyNoDec convertedAmtNoDec;
      CurrencyCode convertedCurrencyCode;
      ExchRate roeRate;
      CurrencyNoDec roeRateNoDec;
      DateTime nucEffectiveDate;
      DateTime nucDiscontinueDate;

      std::vector<FareMarket*>& fareMarkets = itin.fareMarket();

      // Create fare market
      //
      FareMarket fareMarket;
      fareMarket.origin() = (&del);
      fareMarket.destination() = (&lon);
      fareMarket.governingCarrier() = ("BA");
      fareMarket.geoTravelType() = GeoTravelType::International;
      std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();

      // 5. Create instance of PaxTypeBucket
      //
      PaxTypeBucket paxTypeCortege;
      paxTypeCortege.outboundCurrency() = "INR";

      paxTypeCortegeVec.push_back(paxTypeCortege);

      fareMarket.travelSeg().push_back(&travelSeg1);
      fareMarkets.push_back(&fareMarket);

      bool convertRC = false;

      convertRC = nucConverter.convertBaseFare(trx,
                                               farePath,
                                               farePath.getTotalNUCAmount(),
                                               convertedAmount,
                                               convertedAmtNoDec,
                                               convertedCurrencyCode,
                                               roeRate,
                                               roeRateNoDec,
                                               nucEffectiveDate,
                                               nucDiscontinueDate);

      LOG4CXX_DEBUG(_logger, "Converted base fare amount: " << convertedAmount);
      LOG4CXX_DEBUG(_logger, "Converted base fare amount no dec: " << convertedAmtNoDec);
      LOG4CXX_DEBUG(_logger, "Converted base fare currency code: " << convertedCurrencyCode);
      LOG4CXX_DEBUG(_logger, "Nuc roe: " << roeRate);
      LOG4CXX_DEBUG(_logger, "Nuc roe nodec: " << roeRateNoDec);
      LOG4CXX_DEBUG(_logger, "Nuc effective date: " << nucEffectiveDate.toSimpleString());
      LOG4CXX_DEBUG(_logger, "Nuc discontinue date: " << nucDiscontinueDate.toSimpleString());

      CPPUNIT_ASSERT(convertRC != false);

      stopWatch.stop();

      LOG4CXX_INFO(_logger, "Leaving convertInternationalBaseFare");

      LOG4CXX_INFO(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_INFO(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_INFO(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_INFO(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_INFO(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_INFO(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testDomesticRounding()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing NUC Domestic Rounding");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");
      NUCCurrencyConverter nucConverter;
      NUCCollectionResults results;
      results.collect() = true;

      std::string nucStr("NUC");
      std::string jpyStr("JPY");

      tse::Money nuc(100.65, nucStr);
      tse::Money jpy("JPY");
      bool convertRC = false;

      DateTime travelDate = DateTime::localTime();
      PricingTrx trx;
      PricingRequest request;
      trx.setRequest(&request);

      CurrencyConversionRequest request1(
          jpy, nuc, travelDate, *(trx.getRequest()), trx.dataHandle(), false);
      request1.isInternational() = false;

      convertRC = nucConverter.convert(request1, &results);

      LOG4CXX_INFO(_logger, "Converted JPY Amount : " << jpy.value());

      CPPUNIT_ASSERT(convertRC != false);

      nuc.value() = 50.0;

      LOG4CXX_INFO(_logger, "Invoking NUCCurrencyConverter to convert 50 NUCs to GTQ");

      std::string gtqStr("GTQ");

      Money gtq(gtqStr);

      CurrencyConversionRequest request3(
          gtq, nuc, travelDate, *(trx.getRequest()), trx.dataHandle(), false);

      convertRC = nucConverter.convert(request3, &results);

      LOG4CXX_INFO(_logger, "Converted GTQ Amount : " << nuc.value());

      Money nuc4(194.88, NUC);
      Money brl("BRL");

      CurrencyConversionRequest request4(
          brl, nuc4, travelDate, *(trx.getRequest()), trx.dataHandle(), false);
      request4.isInternational() = false;

      convertRC = nucConverter.convert(request4, &results);

      LOG4CXX_INFO(_logger, "Converted BRL Amount : " << brl.value());

      Money mgf("MGF");

      CurrencyConversionRequest request5(
          mgf, nuc, travelDate, *(trx.getRequest()), trx.dataHandle(), false);

      convertRC = nucConverter.convert(request5, &results);

      LOG4CXX_INFO(_logger, "Converted MGF Amount : " << nuc.value());

      Money usd2("USD");

      nuc.value() = 193.349;

      CurrencyConversionRequest request6(
          usd2, nuc, travelDate, *(trx.getRequest()), trx.dataHandle(), false);

      convertRC = nucConverter.convert(request6, &results);

      LOG4CXX_INFO(_logger, "Converted USD Amount : " << usd2.value());

      stopWatch.stop();

      LOG4CXX_INFO(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_INFO(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_INFO(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_INFO(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_INFO(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_INFO(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testInternationalRounding()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();

      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "Testing NUC International Rounding");
      LOG4CXX_DEBUG(_logger, "==============================\n");
      LOG4CXX_DEBUG(_logger, "\n");
      NUCCurrencyConverter nucConverter;
      NUCCollectionResults results;
      results.collect() = true;

      std::string nucStr("NUC");
      std::string jpyStr("JPY");

      tse::Money nuc(100.65, nucStr);
      tse::Money jpy("JPY");
      bool convertRC = false;

      DateTime travelDate = DateTime::localTime();
      PricingTrx trx;
      PricingRequest request;
      trx.setRequest(&request);

      CurrencyConversionRequest request1(
          jpy, nuc, travelDate, *(trx.getRequest()), trx.dataHandle());

      convertRC = nucConverter.convert(request1, &results);

      LOG4CXX_INFO(_logger, "Converted JPY Amount : " << jpy.value());

      CPPUNIT_ASSERT(convertRC != false);

      stopWatch.stop();

      LOG4CXX_INFO(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_INFO(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_INFO(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_INFO(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_INFO(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_INFO(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }
  void testInvalidInput()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();

      NUCCurrencyConverter nucConverter;
      NUCCollectionResults results;
      results.collect() = true;

      std::string nucStr("NUC");
      std::string jpyStr("JPY");

      tse::Money nuc(nucStr);
      tse::Money jpy("JPY");
      bool convertRC = false;

      DateTime travelDate = DateTime::localTime();
      PricingTrx trx;
      PricingRequest request;
      trx.setRequest(&request);

      CurrencyConversionRequest request1(
          jpy, nuc, travelDate, *(trx.getRequest()), trx.dataHandle(), false);

      convertRC = nucConverter.convert(request1, &results);

      LOG4CXX_INFO(_logger, "Converted JPY Amount : " << nuc.value());

      CPPUNIT_ASSERT(convertRC != true);

      stopWatch.stop();

      LOG4CXX_INFO(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_INFO(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_INFO(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_INFO(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_INFO(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_INFO(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testHasArunkSegments()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();
      DateTime travelDate = DateTime::localTime();

      NUCCurrencyConverter nucConverter;
      FareUsage fareUsage;

      MockLoc perth;
      perth.nation() = "AU";
      LocCode perthLoc("PER");
      perth.loc() = perthLoc;

      MockLoc syd;
      syd.nation() = "AU";
      LocCode sydLoc("SYD");
      syd.loc() = sydLoc;

      MockLoc auckland;
      auckland.nation() = "NZ";
      LocCode auckLandLoc("AKL");
      auckland.loc() = auckLandLoc;

      MockLoc lax;
      lax.nation() = "US";
      LocCode laxLoc("LAX");
      lax.loc() = laxLoc;

      // Create TravelSeg
      //
      AirSeg travelSeg1;

      travelSeg1.segmentOrder() = 0;
      travelSeg1.geoTravelType() = GeoTravelType::ForeignDomestic;
      travelSeg1.origin() = &perth;
      travelSeg1.destination() = &syd;
      travelSeg1.departureDT() = travelDate;

      fareUsage.travelSeg().push_back(&travelSeg1);

      ArunkSeg arunkSeg;

      arunkSeg.segmentOrder() = 1;
      arunkSeg.geoTravelType() = GeoTravelType::ForeignDomestic;
      arunkSeg.origin() = &syd;
      arunkSeg.destination() = &auckland;
      arunkSeg.departureDT() = travelDate;

      fareUsage.travelSeg().push_back(&arunkSeg);

      AirSeg travelSeg2;

      travelSeg2.segmentOrder() = 2;
      travelSeg2.geoTravelType() = GeoTravelType::ForeignDomestic;
      travelSeg2.origin() = &auckland;
      travelSeg2.destination() = &lax;
      travelSeg2.departureDT() = travelDate;

      fareUsage.travelSeg().push_back(&travelSeg2);

      LocCode originLoc;

      bool arunkRC = nucConverter.hasArunkSegments(&fareUsage, originLoc);

      if (arunkRC)
      {
        LOG4CXX_INFO(_logger,
                     "Itin has arunk segments: Origin of International travel: " << originLoc);
      }
      else
        LOG4CXX_INFO(_logger, "Itin has no arunk segments");

      stopWatch.stop();

      LOG4CXX_INFO(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_INFO(_logger, "        cpuTime " << stopWatch.cpuTime());

      LOG4CXX_INFO(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_INFO(_logger, "     systemTime " << stopWatch.systemTime());

      LOG4CXX_INFO(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_INFO(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

  void testConversionFacade()
  {
    try
    {
      tse::StopWatch stopWatch;
      stopWatch.start();

      CurrencyConversionFacade ccFacade;
      LOG4CXX_INFO(_logger, "---------------------------------------");
      LOG4CXX_INFO(_logger, "TESTING CURRENCY CONVERSION FACADE FOR NUCS");
      LOG4CXX_INFO(_logger, "---------------------------------------");

      std::string nucStr("NUC");
      std::string jpyStr("JPY");

      tse::Money nuc(0.0, nucStr);
      tse::Money jpy(100, "JPY");
      bool convertRC = false;

      LOG4CXX_INFO(_logger, "Number of nuc decimals: " << nuc.noDec());

      DateTime ticketingDate = DateTime::localTime();
      PricingTrx trx;
      PricingRequest request;
      trx.setRequest(&request);
      request.ticketingDT() = ticketingDate;

      // convert() with NUCs

      convertRC = ccFacade.convert(nuc, jpy, trx, true);
      LOG4CXX_INFO(_logger, "Converted 100 JPY Amount in NUCs: " << nuc.value());
      CPPUNIT_ASSERT(convertRC != false);

      jpy.value() = 0.0;

      convertRC = ccFacade.convert(jpy, nuc, trx);
      LOG4CXX_INFO(_logger, "Converted NUC Amount back to rounded JPY: " << jpy.value());
      CPPUNIT_ASSERT(convertRC != false);

      nuc.value() = 0.0;
      std::string inrStr("INR");
      Money inr(10.0, inrStr);

      convertRC = ccFacade.convert(nuc, inr, trx);
      LOG4CXX_INFO(_logger, "Converted 10 INR Amount in NUCs: " << nuc.value());
      CPPUNIT_ASSERT(convertRC != false);

      Money brl(500, "BRL");
      nuc.value() = 0.0;

      _logger->setLevel(log4cxx::Level::getInfo());

      convertRC = ccFacade.convert(nuc, brl, trx);
      LOG4CXX_INFO(_logger, "Converted 500 BRL Amount in NUCs: " << nuc.value());
      CPPUNIT_ASSERT(convertRC != false);

      // convertCalc with NUCs and non-NUC itin.calcCurrency

      jpy.value() = 0.0;

      convertRC = ccFacade.convertCalc(jpy, nuc, trx);
      LOG4CXX_INFO(_logger, "converCalc() NUCs (from 500 BRL) back to JPY: " << jpy.value());
      CPPUNIT_ASSERT(convertRC != false);

      jpy.value() = 0.0;

      convertRC = ccFacade.convertCalc(jpy, brl, trx);
      LOG4CXX_INFO(_logger, "converCalc() 500 BRL in JPY: " << jpy.value());
      CPPUNIT_ASSERT(convertRC != false);

      stopWatch.stop();
      LOG4CXX_DEBUG(_logger, "    elapsedTime " << stopWatch.elapsedTime());
      LOG4CXX_DEBUG(_logger, "        cpuTime " << stopWatch.cpuTime());
      LOG4CXX_DEBUG(_logger, "       userTime " << stopWatch.userTime());
      LOG4CXX_DEBUG(_logger, "     systemTime " << stopWatch.systemTime());
      LOG4CXX_DEBUG(_logger, "  childUserTime " << stopWatch.childUserTime());
      LOG4CXX_DEBUG(_logger, "childSystemTime " << stopWatch.childSystemTime());
    }
    catch (exception& ex) { LOG4CXX_DEBUG(_logger, "Caught exception " << ex.what()); }
  }

private:
  CurrencyDataHandle* _mockDataHandle;
  TestMemHandle _memHandle;
  static Logger _logger;
};

Logger NUCCurrencyConverterConvertTest::_logger("atseintl.Currency.test.NUCCurrencyConverterConvertTest");

CPPUNIT_TEST_SUITE_REGISTRATION(NUCCurrencyConverterConvertTest);
}
