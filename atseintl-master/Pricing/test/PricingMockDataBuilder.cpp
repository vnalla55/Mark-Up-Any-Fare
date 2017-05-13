#include <iostream>
#include <string>

#include "DBAccess/DataHandle.h"
#include "Pricing/test/PricingMockDataBuilder.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/Loc.h"
#include "DataModel/Itin.h"
#include "DataModel/Trx.h"
#include "DataModel/PricingTrx.h"
#include "Common/TseCodeTypes.h"
#include "Server/TseServer.h"
#include "Common/TseEnums.h"
#include "DataModel/PaxType.h"
#include "Common/Vendor.h"
#include "Common/DateTime.h"
#include "Common/TseUtil.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/FareInfo.h"
#include "Pricing/MergedFareMarket.h"
#include "DataModel/AirSeg.h"

#include "test/testdata/TestLocFactory.h"

using namespace tse;
using namespace std;

PricingTrx*
PricingMockDataBuilder::getPricingTrx()
{
  PricingTrx* trx = new PricingTrx;
  DataHandle& dataHandle = trx->dataHandle();
  PricingOptions* po = 0;
  dataHandle.get(po);
  trx->setOptions(po);
  PricingRequest* pr = 0;
  dataHandle.get(pr);
  trx->setRequest(pr);
  Agent* agent = 0;
  dataHandle.get(agent);
  trx->getRequest()->ticketingAgent() = agent;

  trx->getRequest()->ticketingAgent()->agentLocation() =
      TestLocFactory::create("/vobs/atseintl/test/sampledata/DFW_Loc.xml");
  addPaxType(*trx, "ADT");
  return trx;
}

tse::PaxType*
PricingMockDataBuilder::addPaxType(tse::PricingTrx& trx, const std::string& paxTypeCode)
{
  PaxType* paxType = 0;
  trx.dataHandle().get(paxType);
  paxType->paxType() = paxTypeCode;
  paxType->number() = 1;
  trx.paxType().push_back(paxType);
  return paxType;
}

Itin*
PricingMockDataBuilder::addItin(PricingTrx& trx)
{
  Itin* itin = 0;
  trx.dataHandle().get(itin);
  trx.itin().push_back(itin);
  return itin;
}

Loc*
PricingMockDataBuilder::getLoc(PricingTrx& trx, const std::string& code, const std::string& nation)
{
  Loc* loc = 0;
  trx.dataHandle().get(loc);
  LocCode locCode = code;
  loc->loc() = locCode;
  loc->nation() = nation;
  return loc;
}

TravelSeg*
PricingMockDataBuilder::addTravelSeg(PricingTrx& trx,
                                     Itin& itin,
                                     const std::string& carrier,
                                     Loc* orig,
                                     Loc* dest,
                                     uint32_t segmentOrder)
{
  AirSeg* tvlseg = 0;
  trx.dataHandle().get(tvlseg);
  tvlseg->origin() = orig;
  tvlseg->destination() = dest;
  tvlseg->origAirport() = orig->loc();
  tvlseg->destAirport() = dest->loc();
  tvlseg->boardMultiCity() = orig->loc();
  tvlseg->offMultiCity() = dest->loc();
  tvlseg->segmentOrder() = segmentOrder;
  tvlseg->departureDT() = DateTime::localTime();
  tvlseg->carrier() = carrier;

  itin.travelSeg().push_back(tvlseg);
  trx.travelSeg().push_back(tvlseg);
  return tvlseg;
}

void
PricingMockDataBuilder::addTraveSegToFareMarket(TravelSeg* tseg, FareMarket& fm)
{
  fm.travelSeg().push_back(tseg);
}

FareMarket*
PricingMockDataBuilder::addFareMarket(
    PricingTrx& trx, Itin& itin, const std::string& carrier, Loc* orig, Loc* dest)
{
  FareMarket* fareMarket = 0;
  trx.dataHandle().get(fareMarket);

  fareMarket->origin() = orig;
  fareMarket->destination() = dest;

  fareMarket->boardMultiCity() = orig->loc();
  fareMarket->offMultiCity() = dest->loc();

  fareMarket->setGlobalDirection(GlobalDirection::WH);
  fareMarket->governingCarrier() = carrier;

  addFareToFareMarket(trx, itin, carrier, *fareMarket);

  trx.fareMarket().push_back(fareMarket);
  itin.fareMarket().push_back(fareMarket);

  return fareMarket;
}

void
PricingMockDataBuilder::addFareToFareMarket(PricingTrx& trx,
                                            Itin& itin,
                                            const std::string carrier,
                                            FareMarket& fareMarket)
{
  // LON-BA-NYC    /CXR-BA/ #GI-XX#  .OUT.
  // BA  AT A 5135   WMLUQOW  TAFPBA O I    999.00 GBP EU   ADT   1845.26

  std::vector<PaxType*>::iterator it = trx.paxType().begin();
  std::vector<PaxType*>::iterator itEnd = trx.paxType().end();
  for (; it != itEnd; ++it)
  {
    PaxTypeBucket* paxTypeCortege = trx.dataHandle().create<PaxTypeBucket>();
    paxTypeCortege->requestedPaxType() = *it;

    for (uint16_t i = 0; i < 10; ++i)
    {
      Fare* fare = 0;
      trx.dataHandle().get(fare);
      fare->nucFareAmount() = 1845.26 + i;

      FareInfo* fareInfo = 0;
      trx.dataHandle().get(fareInfo);
      fareInfo->carrier() = carrier;
      fareInfo->market1() = fareMarket.origin()->loc();
      fareInfo->market2() = fareMarket.destination()->loc();
      fareInfo->fareClass() = "WMLUQOW";
      fareInfo->fareAmount() = 999.00 + i;
      fareInfo->currency() = "USD";

      if (i % 3 == 0)
        fareInfo->_owrt = ONE_WAY_MAY_BE_DOUBLED;
      else if (i % 3 == 1)
        fareInfo->_owrt = ROUND_TRIP_MAYNOT_BE_HALVED;
      else
        fareInfo->_owrt = ONE_WAY_MAYNOT_BE_DOUBLED;

      fareInfo->_ruleNumber = "5135";
      fareInfo->_routingNumber = "XXXX";

      if (i % 3 == 0)
        fareInfo->_directionality = FROM;
      else if (i % 3 == 1)
        fareInfo->_directionality = TO;
      else
        fareInfo->_directionality = BOTH;

      if (i % 2 == 0)
        fareInfo->_globalDirection = GlobalDirection::WH;
      else
        fareInfo->_globalDirection = GlobalDirection::AT;

      fareInfo->_vendor = Vendor::ATPCO;

      TariffCrossRefInfo* tariffRefInfo = trx.dataHandle().create<TariffCrossRefInfo>();
      tariffRefInfo->_fareTariffCode = "TAFPBA";

      fare->initialize(Fare::FS_International, fareInfo, fareMarket, tariffRefInfo);

      PaxTypeFare* paxTypeFare = trx.dataHandle().create<PaxTypeFare>();
      paxTypeFare->initialize(fare, *it, &fareMarket);

      FareClassAppInfo* appInfo = 0;
      trx.dataHandle().get(appInfo);
      appInfo->_fareType = "EU";
      paxTypeFare->fareClassAppInfo() = appInfo;

      FareClassAppSegInfo* fareClassAppSegInfo = trx.dataHandle().create<FareClassAppSegInfo>();
      fareClassAppSegInfo->_paxType = (*it)->paxType();
      paxTypeFare->fareClassAppSegInfo() = fareClassAppSegInfo;

      paxTypeCortege->paxTypeFare().push_back(paxTypeFare);
    }
    fareMarket.paxTypeCortege().push_back(*paxTypeCortege);
  }
}

PUPath*
PricingMockDataBuilder::getPUPath(PricingTrx& trx)
{
  PUPath* puPath = 0;
  trx.dataHandle().get(puPath);
  return puPath;
}

PU*
PricingMockDataBuilder::addOWPUToMainTrip(PricingTrx& trx,
                                          PUPath& puPath,
                                          FareMarket* fm1,
                                          Directionality dir)
{
  DataHandle& dataHandle = trx.dataHandle();
  PU* pu = 0;
  dataHandle.get(pu);
  pu->puType() = PricingUnit::Type::ONEWAY;
  MergedFareMarket* mfm1 = getMergedFareMarket(trx, fm1);
  pu->fareMarket().push_back(mfm1);
  pu->fareDirectionality().push_back(dir);
  puPath.puPath().push_back(pu);
  puPath.allPU().push_back(pu);
  return pu;
}

PU*
PricingMockDataBuilder::addOJPUToMainTrip(PricingTrx& trx,
                                          PUPath& puPath,
                                          FareMarket* fm1,
                                          FareMarket* fm2,
                                          Directionality dir1,
                                          Directionality dir2)
{
  DataHandle& dataHandle = trx.dataHandle();
  PU* pu = 0;
  dataHandle.get(pu);
  pu->puType() = PricingUnit::Type::ROUNDTRIP;
  MergedFareMarket* mfm1 = getMergedFareMarket(trx, fm1);
  MergedFareMarket* mfm2 = getMergedFareMarket(trx, fm2);
  pu->fareMarket().push_back(mfm1);
  pu->fareMarket().push_back(mfm2);
  pu->fareDirectionality().push_back(dir1);
  pu->fareDirectionality().push_back(dir2);
  puPath.puPath().push_back(pu);
  puPath.allPU().push_back(pu);
  return pu;
}

PU*
PricingMockDataBuilder::addRTPUToMainTrip(PricingTrx& trx,
                                          PUPath& puPath,
                                          FareMarket* fm1,
                                          FareMarket* fm2)
{
  DataHandle& dataHandle = trx.dataHandle();
  PU* pu = 0;
  dataHandle.get(pu);
  pu->puType() = PricingUnit::Type::OPENJAW;
  MergedFareMarket* mfm1 = getMergedFareMarket(trx, fm1);
  MergedFareMarket* mfm2 = getMergedFareMarket(trx, fm2);
  pu->fareMarket().push_back(mfm1);
  pu->fareMarket().push_back(mfm2);
  pu->fareDirectionality().push_back(FROM);
  pu->fareDirectionality().push_back(TO);
  puPath.puPath().push_back(pu);
  puPath.allPU().push_back(pu);
  return pu;
}
PU*
PricingMockDataBuilder::addCTPUToMainTrip(
    PricingTrx& trx, PUPath& puPath, FareMarket* fm1, FareMarket* fm2, FareMarket* fm3)
{
  DataHandle& dataHandle = trx.dataHandle();
  PU* pu = 0;
  dataHandle.get(pu);
  pu->puType() = PricingUnit::Type::ROUNDTRIP;
  MergedFareMarket* mfm1 = getMergedFareMarket(trx, fm1);
  MergedFareMarket* mfm2 = getMergedFareMarket(trx, fm2);
  MergedFareMarket* mfm3 = getMergedFareMarket(trx, fm3);
  pu->fareMarket().push_back(mfm1);
  pu->fareMarket().push_back(mfm2);
  pu->fareMarket().push_back(mfm3);
  pu->fareDirectionality().push_back(FROM);
  pu->fareDirectionality().push_back(FROM);
  pu->fareDirectionality().push_back(TO);
  puPath.puPath().push_back(pu);
  puPath.allPU().push_back(pu);
  return pu;
}

MergedFareMarket*
PricingMockDataBuilder::getMergedFareMarket(PricingTrx& trx, FareMarket* fm)
{
  MergedFareMarket* mfm = 0;
  trx.dataHandle().get(mfm);
  mfm->origin() = fm->origin();
  mfm->origin() = fm->origin();
  mfm->destination() = fm->destination();

  mfm->boardMultiCity() = fm->boardMultiCity();
  mfm->offMultiCity() = fm->offMultiCity();

  mfm->globalDirection() = fm->getGlobalDirection();
  mfm->geoTravelType() = fm->geoTravelType();

  mfm->travelSeg() = fm->travelSeg();
  mfm->mergedFareMarket().push_back(fm);

  return mfm;
}
