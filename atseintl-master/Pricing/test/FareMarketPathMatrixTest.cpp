#include "Pricing/test/FareMarketPathMatrixTest.h"

#include "Common/Config/ConfigMan.h"
#include "Common/TseEnums.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DCFactory.h"
#include "Pricing/FareMarketMerger.h"
#include "Pricing/FareMarketPathMatrix.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"

#include <iostream>

using namespace tse;
using namespace std;

void
FareMarketPathMatrixTest::testBuildAllFareMarketPath()
{
  TestMemHandle memHandle;
  memHandle.create<TestConfigInitializer>();

  PricingTrx trx;

  trx.diagnostic().diagnosticType() = Diagnostic600;
  trx.diagnostic().activate();

  PricingOptions options;
  trx.setOptions(&options);

  PricingRequest request;
  trx.setRequest(&request);

  //---- Create Loc Objects  -------------
  Loc* locLAX = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLAX.xml");
  Loc* locDFW = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
  Loc* locTUL = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocTUL.xml");
  Loc* locLON = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
  Loc* locNYC = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
  Loc* locORD = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocORD.xml");
  Loc* locAUS = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocAUS.xml");
  Loc* locHOU = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHOU.xml");

  //---- Create Travel Seg  -------------
  AirSeg* tvlsegLAXDFW = getTravelSeg(locLAX, locDFW, trx);

  AirSeg* tvlsegDFWTUL = getTravelSeg(locDFW, locTUL, trx);
  AirSeg* tvlsegTULDFW = getTravelSeg(locTUL, locDFW, trx);

  AirSeg* tvlsegDFWORD = getTravelSeg(locDFW, locORD, trx);

  AirSeg* tvlsegORDNYC = getTravelSeg(locORD, locNYC, trx);
  AirSeg* tvlsegNYCLON = getTravelSeg(locNYC, locLON, trx);
  AirSeg* tvlsegLONNYC = getTravelSeg(locLON, locNYC, trx);
  AirSeg* tvlsegNYCORD = getTravelSeg(locNYC, locORD, trx);

  AirSeg* tvlsegORDAUS = getTravelSeg(locORD, locAUS, trx);
  AirSeg* tvlsegAUSHOU = getTravelSeg(locAUS, locHOU, trx);
  AirSeg* tvlsegHOULAX = getTravelSeg(locHOU, locLAX, trx);

  vector<TravelSeg*> sideTrip;
  sideTrip.push_back(tvlsegDFWTUL);
  sideTrip.push_back(tvlsegTULDFW);

  //---  Create Fare Markets ------
  FareMarket* mktLAXDFW = getFareMarket(locLAX, locDFW, trx);
  mktLAXDFW->travelSeg().push_back(tvlsegLAXDFW);

  FareMarket* mktLAXTUL = getFareMarket(locLAX, locDFW, trx);
  mktLAXTUL->travelSeg().push_back(tvlsegLAXDFW);
  mktLAXTUL->travelSeg().push_back(tvlsegDFWTUL);

  FareMarket* mktLAXORD = getFareMarket(locLAX, locORD, trx);
  mktLAXORD->travelSeg().push_back(tvlsegLAXDFW);
  mktLAXORD->travelSeg().push_back(tvlsegDFWTUL);
  mktLAXORD->travelSeg().push_back(tvlsegTULDFW);
  mktLAXORD->travelSeg().push_back(tvlsegDFWORD);

  FareMarket* mktLAXORD_ST = getFareMarket(locLAX, locORD, trx);
  mktLAXORD_ST->travelSeg().push_back(tvlsegLAXDFW);
  mktLAXORD_ST->travelSeg().push_back(tvlsegDFWORD);
  mktLAXORD_ST->sideTripTravelSeg().push_back(sideTrip);

  FareMarket* mktDFWTUL = getFareMarket(locDFW, locTUL, trx);
  mktDFWTUL->travelSeg().push_back(tvlsegDFWTUL);

  FareMarket* mktDFWORD = getFareMarket(locDFW, locORD, trx);
  mktDFWORD->travelSeg().push_back(tvlsegDFWORD);

  FareMarket* mktDFWNYC = getFareMarket(locDFW, locNYC, trx);
  mktDFWNYC->travelSeg().push_back(tvlsegDFWORD);
  mktDFWNYC->travelSeg().push_back(tvlsegORDNYC);

  FareMarket* mktDFWLON = getFareMarket(locDFW, locLON, trx);
  mktDFWLON->travelSeg().push_back(tvlsegDFWORD);
  mktDFWLON->travelSeg().push_back(tvlsegORDNYC);
  mktDFWLON->travelSeg().push_back(tvlsegNYCLON);

  FareMarket* mktTULDFW = getFareMarket(locTUL, locDFW, trx);
  mktTULDFW->travelSeg().push_back(tvlsegTULDFW);

  FareMarket* mktTULNYC = getFareMarket(locTUL, locNYC, trx);
  mktTULNYC->travelSeg().push_back(tvlsegTULDFW);
  mktTULNYC->travelSeg().push_back(tvlsegDFWORD);
  mktTULNYC->travelSeg().push_back(tvlsegORDNYC);

  FareMarket* mktNYCLON = getFareMarket(locNYC, locLON, trx);
  mktNYCLON->travelSeg().push_back(tvlsegNYCLON);

  FareMarket* mktLONNYC = getFareMarket(locLON, locNYC, trx);
  mktLONNYC->travelSeg().push_back(tvlsegLONNYC);

  FareMarket* mktNYCORD = getFareMarket(locNYC, locORD, trx);
  mktNYCORD->travelSeg().push_back(tvlsegNYCORD);

  FareMarket* mktORDNYC = getFareMarket(locORD, locNYC, trx);
  mktORDNYC->travelSeg().push_back(tvlsegORDNYC);

  FareMarket* mktORDAUS = getFareMarket(locORD, locAUS, trx);
  mktORDAUS->travelSeg().push_back(tvlsegORDAUS);

  FareMarket* mktAUSHOU = getFareMarket(locAUS, locHOU, trx);
  mktAUSHOU->travelSeg().push_back(tvlsegAUSHOU);

  FareMarket* mktHOULAX = getFareMarket(locHOU, locLAX, trx);
  mktHOULAX->travelSeg().push_back(tvlsegHOULAX);

  //---- Create Itin ------
  Itin itin;
  tvlsegLAXDFW->segmentOrder() = 1;
  itin.travelSeg().push_back(tvlsegLAXDFW);
  tvlsegDFWTUL->segmentOrder() = 2;
  itin.travelSeg().push_back(tvlsegDFWTUL);
  tvlsegTULDFW->segmentOrder() = 3;
  itin.travelSeg().push_back(tvlsegTULDFW);
  tvlsegDFWORD->segmentOrder() = 4;
  itin.travelSeg().push_back(tvlsegDFWORD);
  tvlsegORDNYC->segmentOrder() = 5;
  itin.travelSeg().push_back(tvlsegORDNYC);
  tvlsegNYCLON->segmentOrder() = 6;
  itin.travelSeg().push_back(tvlsegNYCLON);
  tvlsegLONNYC->segmentOrder() = 7;
  itin.travelSeg().push_back(tvlsegLONNYC);
  tvlsegNYCORD->segmentOrder() = 8;
  itin.travelSeg().push_back(tvlsegNYCORD);
  tvlsegORDAUS->segmentOrder() = 9;
  itin.travelSeg().push_back(tvlsegORDAUS);
  tvlsegAUSHOU->segmentOrder() = 10;
  itin.travelSeg().push_back(tvlsegAUSHOU);
  tvlsegHOULAX->segmentOrder() = 11;
  itin.travelSeg().push_back(tvlsegHOULAX);

  itin.fareMarket().push_back(mktLAXDFW);
  itin.fareMarket().push_back(mktLAXTUL);
  itin.fareMarket().push_back(mktLAXORD);
  itin.fareMarket().push_back(mktLAXORD_ST);
  itin.fareMarket().push_back(mktDFWTUL);
  itin.fareMarket().push_back(mktDFWORD);
  itin.fareMarket().push_back(mktDFWNYC);
  itin.fareMarket().push_back(mktDFWLON);
  itin.fareMarket().push_back(mktTULDFW);
  itin.fareMarket().push_back(mktTULNYC);
  itin.fareMarket().push_back(mktNYCLON);
  itin.fareMarket().push_back(mktLONNYC);
  itin.fareMarket().push_back(mktNYCORD);
  itin.fareMarket().push_back(mktORDNYC);
  itin.fareMarket().push_back(mktORDAUS);
  itin.fareMarket().push_back(mktAUSHOU);
  itin.fareMarket().push_back(mktHOULAX);

  trx.itin().push_back(&itin);

  //------- Build FareMarket Path Matrix ----------
  vector<MergedFareMarket*> mergedFareMarketVect;
  CarrierCode cxrCode = "BA";
  FareMarketMerger fmMerger(trx, itin, true, &cxrCode);
  fmMerger.buildAllMergedFareMarket(mergedFareMarketVect);

  FareMarketPathMatrix fmpMatrix(trx, itin, mergedFareMarketVect);
  bool ret = fmpMatrix.buildAllFareMarketPath();

  //------- Print Diagnostic Messages -----------
  // cout << endl << trx.diagnostic().toString() << endl;

  CPPUNIT_ASSERT(ret);
}

AirSeg*
FareMarketPathMatrixTest::getTravelSeg(Loc* orig, Loc* dest, PricingTrx& trx)
{
  AirSeg* tvlseg = &trx.dataHandle().safe_create<AirSeg>();
  tvlseg->origin() = orig;
  tvlseg->destination() = dest;
  tvlseg->origAirport() = orig->loc();
  tvlseg->destAirport() = dest->loc();
  tvlseg->boardMultiCity() = orig->loc();
  tvlseg->offMultiCity() = dest->loc();
  tvlseg->departureDT() = DateTime::localTime();
  tvlseg->segmentOrder() = 0;
  return tvlseg;
}

FareMarket*
FareMarketPathMatrixTest::getFareMarket(Loc* orig, Loc* dest, PricingTrx& trx)
{
  FareMarket* fareMarket = &trx.dataHandle().safe_create<FareMarket>();

  fareMarket->origin() = (orig);
  fareMarket->destination() = (dest);

  fareMarket->boardMultiCity() = orig->loc();
  fareMarket->offMultiCity() = dest->loc();

  GlobalDirection globleDirection = GlobalDirection::AT;
  fareMarket->setGlobalDirection(globleDirection);
  CarrierCode cxrCode = "BA";
  fareMarket->governingCarrier() = cxrCode;

  // LON-BA-NYC    /CXR-BA/ #GI-XX#  .OUT.
  // BA  AT A 5135   WMLUQOW  TAFPBA O I    999.00 GBP EU   ADT   1845.26
  Fare* fare1 = &trx.dataHandle().safe_create<Fare>();
  fare1->nucFareAmount() = 1845.26;

  FareInfo* fareInfo1 = &trx.dataHandle().safe_create<FareInfo>();
  fareInfo1->_carrier = "BA";
  fareInfo1->_market1 = orig->loc();
  fareInfo1->_market2 = dest->loc();
  fareInfo1->_fareClass = "WMLUQOW";
  fareInfo1->_fareAmount = 999.00;
  fareInfo1->_currency = "GBP";
  fareInfo1->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
  fareInfo1->_ruleNumber = "5135";
  fareInfo1->_routingNumber = "XXXX";
  fareInfo1->_directionality = FROM;
  fareInfo1->_globalDirection = GlobalDirection::AT;
  fareInfo1->_vendor = Vendor::ATPCO;

  TariffCrossRefInfo* tariffRefInfo = &trx.dataHandle().safe_create<TariffCrossRefInfo>();
  tariffRefInfo->_fareTariffCode = "TAFPBA";

  fare1->initialize(Fare::FS_International, fareInfo1, *fareMarket, tariffRefInfo);

  PaxType* paxType = &trx.dataHandle().safe_create<PaxType>();
  PaxTypeCode paxTypeCode = "ADT";
  paxType->paxType() = paxTypeCode;
  return fareMarket;
}
