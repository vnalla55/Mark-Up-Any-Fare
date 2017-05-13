//----------------------------------------------------------------------------
//  Copyright Sabre 2004, 2009
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

#include "Common/DateTime.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/LimitationCmn.h"
#include "DBAccess/LimitationFare.h"
#include "DBAccess/LimitationFare.h"
#include "DBAccess/LimitationJrny.h"
#include "DBAccess/LimitationJrny.h"
#include "DBAccess/LimitFareCxrLoc.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/MultiTransport.h"
#include "DBAccess/SurfaceSectorExempt.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Limitations/LimitationOnIndirectTravel.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TimeBomb.h"
#include "test/include/TestConfigInitializer.h"
#include "test/testdata/TestLocFactory.h"

using namespace std;

namespace tse
{

namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  LimitationFare* getLimF(int seq,
                          Indicator whollyWithinAppl,
                          LocTypeCode lwt,
                          LocCode lwc,
                          Indicator originTvlAppl,
                          LocTypeCode lot,
                          LocCode loc,
                          FareType fareType,
                          int intlDepartMaxNo,
                          int intlArrivalMaxNo,
                          int maxRetransitAllowed,
                          Indicator retransitPointAppl,
                          int maxStopsAtRetransit,
                          Indicator viaPointStopoverAppl,
                          Indicator govCarrierAppl,
                          Indicator directionality,
                          Indicator fareComponentAppl,
                          LocTypeCode l1t,
                          LocCode l1c,
                          LocTypeCode l2t,
                          LocCode l2c,
                          GlobalDirection globalDir,
                          Indicator mustNotViaHip,
                          Indicator confirmedAppl,
                          Indicator maxDomSegments,
                          Indicator exceptViaCxrLocInd,
                          Indicator viaGovCarrierInd,
                          Indicator retransitGovCxrAppl)
  {
    LimitationFare* ret = _memHandle.create<LimitationFare>();
    ret->setSeqNo(seq);
    ret->limitationAppl() = 'F';
    ret->whollyWithinAppl() = whollyWithinAppl;
    ret->whollyWithinLoc().locType() = lwt;
    ret->whollyWithinLoc().loc() = lwc;
    ret->originTvlAppl() = originTvlAppl;
    ret->originLoc().locType() = lot;
    ret->originLoc().loc() = loc;
    ret->fareType() = fareType;
    ret->intlDepartMaxNo() = intlDepartMaxNo;
    ret->intlArrivalMaxNo() = intlArrivalMaxNo;
    ret->maxRetransitAllowed() = maxRetransitAllowed;
    ret->retransitPointAppl() = retransitPointAppl;
    ret->maxStopsAtRetransit() = maxStopsAtRetransit;
    ret->viaPointStopoverAppl() = viaPointStopoverAppl;
    ret->govCarrierAppl() = govCarrierAppl;
    ret->directionality() = directionality;
    ret->fareComponentAppl() = fareComponentAppl;
    ret->loc1().locType() = l1t;
    ret->loc1().loc() = l1c;
    ret->loc2().locType() = l2t;
    ret->loc2().loc() = l2c;
    ret->globalDir() = globalDir;
    ret->mustNotViaHip() = mustNotViaHip;
    ret->confirmedAppl() = confirmedAppl;
    ret->maxDomSegments() = maxDomSegments;
    ret->exceptViaCxrLocInd() = exceptViaCxrLocInd;
    ret->viaGovCarrierInd() = viaGovCarrierInd;
    ret->retransitGovCxrAppl() = retransitGovCxrAppl;
    return ret;
  }
  LimitFareCxrLoc*
  getLimCxr(CarrierCode cxr, LocTypeCode lt1, LocCode lc1, LocTypeCode lt2, LocCode lc2)
  {
    LimitFareCxrLoc* ret = new LimitFareCxrLoc; // will be desttroyed by LimitFare destructor
    ret->carrier() = cxr;
    ret->loc1().locType() = lt1;
    ret->loc1().loc() = lc1;
    ret->loc2().locType() = lt2;
    ret->loc2().loc() = lc2;
    return ret;
  }

public:
  const std::vector<LimitationJrny*>& getJLimitation(const DateTime& date)
  {
    std::vector<LimitationJrny*>* ret = _memHandle.create<std::vector<LimitationJrny*> >();
    LimitationJrny* lim = _memHandle.create<LimitationJrny>();
    lim->separateTktInd() = 'Y';
    lim->intlDepartMaxNo() = 4;
    lim->intlArrivalMaxNo() = 4;
    lim->retransitLoc().locType() = 'Z';
    lim->retransitLoc().loc() = "1954";
    ret->push_back(lim);
    return *ret;
  }
  const std::vector<LimitationFare*>& getFCLimitation(const DateTime& date)
  {
    std::vector<LimitationFare*>* ret = _memHandle.create<std::vector<LimitationFare*> >();
    ret->push_back(getLimF(6,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           -1,
                           ' ',
                           -1,
                           ' ',
                           ' ',
                           'B',
                           ' ',
                           'N',
                           "US",
                           'Z',
                           "7798",
                           GlobalDirection::ZZ,
                           ' ',
                           ' ',
                           '9',
                           'N',
                           ' ',
                           ' '));
    ret->back()->exceptViaCxrs().push_back(getLimCxr("", 'N', "JP", ' ', ""));
    ret->push_back(getLimF(12,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           -1,
                           ' ',
                           -1,
                           ' ',
                           ' ',
                           'B',
                           ' ',
                           'N',
                           "US",
                           'Z',
                           "07800",
                           GlobalDirection::ZZ,
                           ' ',
                           ' ',
                           '3',
                           'N',
                           ' ',
                           ' '));
    ret->back()->exceptViaCxrs().push_back(getLimCxr("", 'N', "JP", ' ', ""));
    ret->push_back(getLimF(25,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           -1,
                           ' ',
                           -1,
                           ' ',
                           ' ',
                           'B',
                           ' ',
                           'Z',
                           "06361",
                           'Z',
                           "07798",
                           GlobalDirection::ZZ,
                           ' ',
                           ' ',
                           '3',
                           'N',
                           ' ',
                           ' '));
    ret->back()->exceptViaCxrs().push_back(getLimCxr("", 'N', "JP", ' ', ""));
    ret->push_back(getLimF(50,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           -1,
                           ' ',
                           -1,
                           ' ',
                           ' ',
                           'B',
                           ' ',
                           'Z',
                           "7797",
                           'Z',
                           "7797",
                           GlobalDirection::ZZ,
                           ' ',
                           ' ',
                           '3',
                           'N',
                           ' ',
                           ' '));
    ret->back()->exceptViaCxrs().push_back(getLimCxr("", 'N', "JP", ' ', ""));
    ret->push_back(getLimF(250,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           -1,
                           ' ',
                           1,
                           '5',
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           " ",
                           ' ',
                           "",
                           GlobalDirection::ZZ,
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           ' '));
    ret->push_back(getLimF(400,
                           '2',
                           'Z',
                           "4419",
                           'P',
                           'N',
                           "BR",
                           "",
                           -1,
                           -1,
                           -1,
                           ' ',
                           -1,
                           ' ',
                           ' ',
                           'F',
                           'O',
                           'N',
                           "BR",
                           ' ',
                           "",
                           GlobalDirection::ZZ,
                           ' ',
                           ' ',
                           '2',
                           'N',
                           ' ',
                           ' '));
    ret->back()->tktgCarriers().push_back("JJ");
    ret->back()->exceptTktgCxrInd() = 'Y';
    ret->back()->exceptViaCxrs().push_back(getLimCxr("", 'N', "BR", ' ', ""));
    ret->push_back(getLimF(500,
                           ' ',
                           ' ',
                           "",
                           'P',
                           'N',
                           "DE",
                           "",
                           -1,
                           -1,
                           -1,
                           ' ',
                           -1,
                           ' ',
                           ' ',
                           'B',
                           ' ',
                           'N',
                           "DE",
                           ' ',
                           "",
                           GlobalDirection::ZZ,
                           ' ',
                           ' ',
                           '2',
                           'N',
                           ' ',
                           ' '));
    ret->back()->exceptViaCxrs().push_back(getLimCxr("", 'N', "DE", ' ', ""));
    ret->push_back(getLimF(600,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           -1,
                           '5',
                           1,
                           '5',
                           'Y',
                           'B',
                           ' ',
                           'A',
                           "1",
                           'A',
                           "3",
                           GlobalDirection::PA,
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           'Y',
                           'Y'));
    ret->back()->govCarriers().push_back("AA");
    ret->push_back(getLimF(650,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           0,
                           '5',
                           -1,
                           ' ',
                           'Y',
                           'B',
                           ' ',
                           'A',
                           "1",
                           'A',
                           "3",
                           GlobalDirection::PA,
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           'N'));
    ret->back()->govCarriers().push_back("AA");
    ret->push_back(getLimF(700,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           1,
                           '5',
                           1,
                           '5',
                           'Y',
                           'B',
                           ' ',
                           'A',
                           "1",
                           'A',
                           "3",
                           GlobalDirection::PA,
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           'Y',
                           ' '));
    ret->back()->govCarriers().push_back("UA");
    ret->push_back(getLimF(750,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           0,
                           '5',
                           -1,
                           ' ',
                           'Y',
                           'B',
                           ' ',
                           'A',
                           "1",
                           'A',
                           "3",
                           GlobalDirection::PA,
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           'N',
                           ' '));
    ret->back()->govCarriers().push_back("UA");
    ret->push_back(getLimF(800,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           -1,
                           ' ',
                           -1,
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           "",
                           ' ',
                           "",
                           GlobalDirection::ZZ,
                           ' ',
                           ' ',
                           '0',
                           'N',
                           ' ',
                           ' '));
    ret->back()->exceptViaCxrs().push_back(getLimCxr("CI", 'N', "VN", ' ', ""));
    ret->back()->exceptViaCxrs().push_back(getLimCxr("NH", 'N', "VN", ' ', ""));
    ret->back()->exceptViaCxrs().push_back(getLimCxr("QF", 'N', "VN", ' ', ""));
    ret->back()->exceptViaCxrs().push_back(getLimCxr("S7", 'N', "VN", ' ', ""));
    ret->back()->exceptViaCxrs().push_back(getLimCxr("UN", 'N', "VN", ' ', ""));
    ret->back()->exceptViaCxrs().push_back(getLimCxr("VN", 'N', "VN", ' ', ""));
    ret->push_back(getLimF(900,
                           ' ',
                           ' ',
                           "",
                           'J',
                           'A',
                           "1",
                           "",
                           1,
                           1,
                           -1,
                           '1',
                           -1,
                           ' ',
                           ' ',
                           'W',
                           'B',
                           'A',
                           "1",
                           ' ',
                           "",
                           GlobalDirection::WH,
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           ' '));
    ret->push_back(getLimF(1050,
                           '1',
                           'Z',
                           "1792",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           -1,
                           ' ',
                           -1,
                           ' ',
                           ' ',
                           'B',
                           ' ',
                           'C',
                           "SIN",
                           'C',
                           "BWN",
                           GlobalDirection::EH,
                           'N',
                           'N',
                           ' ',
                           ' ',
                           ' ',
                           ' '));
    ret->push_back(getLimF(1100,
                           '1',
                           'Z',
                           "1792",
                           ' ',
                           ' ',
                           "",
                           "FU",
                           -1,
                           -1,
                           -1,
                           ' ',
                           0,
                           ' ',
                           ' ',
                           'B',
                           ' ',
                           'Z',
                           "1792",
                           'Z',
                           "1792",
                           GlobalDirection::EH,
                           'Y',
                           'Y',
                           ' ',
                           ' ',
                           ' ',
                           ' '));
    ret->push_back(getLimF(1200,
                           '1',
                           'Z',
                           "1792",
                           ' ',
                           ' ',
                           "",
                           "FR",
                           -1,
                           -1,
                           -1,
                           ' ',
                           0,
                           ' ',
                           ' ',
                           'B',
                           ' ',
                           'Z',
                           "1792",
                           'Z',
                           "1792",
                           GlobalDirection::EH,
                           'Y',
                           'Y',
                           ' ',
                           ' ',
                           ' ',
                           ' '));
    ret->push_back(getLimF(1300,
                           '1',
                           'Z',
                           "1792",
                           ' ',
                           ' ',
                           "",
                           "BU",
                           -1,
                           -1,
                           -1,
                           ' ',
                           0,
                           ' ',
                           ' ',
                           'B',
                           ' ',
                           'Z',
                           "1792",
                           'Z',
                           "1792",
                           GlobalDirection::EH,
                           'Y',
                           'Y',
                           ' ',
                           ' ',
                           ' ',
                           ' '));
    ret->push_back(getLimF(1400,
                           '1',
                           'Z',
                           "1792",
                           ' ',
                           ' ',
                           "",
                           "BR",
                           -1,
                           -1,
                           -1,
                           ' ',
                           0,
                           ' ',
                           ' ',
                           'B',
                           ' ',
                           'Z',
                           "1792",
                           'Z',
                           "1792",
                           GlobalDirection::EH,
                           'Y',
                           'Y',
                           ' ',
                           ' ',
                           ' ',
                           ' '));
    ret->push_back(getLimF(1500,
                           '1',
                           'Z',
                           "1792",
                           ' ',
                           ' ',
                           "",
                           "EU",
                           -1,
                           -1,
                           -1,
                           ' ',
                           0,
                           ' ',
                           ' ',
                           'B',
                           ' ',
                           'Z',
                           "1792",
                           'Z',
                           "1792",
                           GlobalDirection::EH,
                           'Y',
                           'Y',
                           ' ',
                           ' ',
                           ' ',
                           ' '));
    ret->push_back(getLimF(1600,
                           '1',
                           'Z',
                           "1792",
                           ' ',
                           ' ',
                           "",
                           "ER",
                           -1,
                           -1,
                           -1,
                           ' ',
                           0,
                           ' ',
                           ' ',
                           'B',
                           ' ',
                           'Z',
                           "1792",
                           'Z',
                           "1792",
                           GlobalDirection::EH,
                           'Y',
                           'Y',
                           ' ',
                           ' ',
                           ' ',
                           ' '));
    ret->push_back(getLimF(1700,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           -1,
                           ' ',
                           -1,
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           "",
                           ' ',
                           "",
                           GlobalDirection::ZZ,
                           ' ',
                           ' ',
                           '0',
                           'N',
                           ' ',
                           ' '));
    ret->back()->exceptViaCxrs().push_back(getLimCxr("", 'N', "NM", ' ', ""));
    ret->push_back(getLimF(1900,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           -1,
                           '5',
                           1,
                           '5',
                           'Y',
                           'W',
                           ' ',
                           'A',
                           "1",
                           ' ',
                           "",
                           GlobalDirection::WH,
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           'Y',
                           ' '));
    ret->back()->govCarriers().push_back("JM");
    ret->push_back(getLimF(2000,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           -1,
                           '5',
                           1,
                           '5',
                           'Y',
                           'B',
                           ' ',
                           'A',
                           "1",
                           'A',
                           "3",
                           GlobalDirection::PA,
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           'Y',
                           ' '));
    ret->back()->govCarriers().push_back("NW");
    ret->push_back(getLimF(2050,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           0,
                           '5',
                           -1,
                           ' ',
                           'Y',
                           'B',
                           ' ',
                           'A',
                           "1",
                           'A',
                           "3",
                           GlobalDirection::PA,
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           'N',
                           ' '));
    ret->back()->govCarriers().push_back("NW");
    ret->push_back(getLimF(2300,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           0,
                           '5',
                           -1,
                           ' ',
                           'N',
                           'W',
                           ' ',
                           'A',
                           "1",
                           ' ',
                           "",
                           GlobalDirection::WH,
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           ' '));
    ret->back()->govCarriers().push_back("JM");
    ret->push_back(getLimF(2400,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           0,
                           '5',
                           -1,
                           ' ',
                           'N',
                           'B',
                           ' ',
                           'A',
                           "1",
                           'A',
                           "3",
                           GlobalDirection::PA,
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           ' '));
    ret->back()->govCarriers().push_back("AA");
    ret->back()->govCarriers().push_back("NW");
    ret->back()->govCarriers().push_back("UA");
    ret->push_back(getLimF(2700,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           -1,
                           '5',
                           1,
                           '5',
                           'Y',
                           'W',
                           ' ',
                           'A',
                           "1",
                           ' ',
                           "",
                           GlobalDirection::WH,
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           'Y',
                           ' '));
    ret->back()->govCarriers().push_back("JM");
    ret->push_back(getLimF(2800,
                           ' ',
                           ' ',
                           "",
                           ' ',
                           ' ',
                           "",
                           "",
                           -1,
                           -1,
                           0,
                           '5',
                           -1,
                           ' ',
                           'Y',
                           'W',
                           ' ',
                           'A',
                           "1",
                           ' ',
                           "",
                           GlobalDirection::WH,
                           ' ',
                           ' ',
                           ' ',
                           ' ',
                           'N',
                           ' '));
    ret->back()->govCarriers().push_back("JM");

    return *ret;
  }
  const std::vector<MultiTransport*>& getMultiTransportCity(const LocCode& locCode,
                                                            const CarrierCode& carrierCode,
                                                            GeoTravelType tvlType,
                                                            const DateTime& tvlDate)
  {
    if (locCode == "EWR" && (carrierCode == "JL" || carrierCode == "") && tvlType == GeoTravelType::International)
    {
      std::vector<MultiTransport*>* ret = _memHandle.create<std::vector<MultiTransport*> >();
      MultiTransport* mt = _memHandle.create<MultiTransport>();
      mt->multitranscity() = "NYC";
      mt->carrier() = "JL";
      mt->multitransLoc() = "EWR";
      ret->push_back(mt);
      return *ret;
    }
    return DataHandleMock::getMultiTransportCity(locCode, carrierCode, tvlType, tvlDate);
  }
  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "YVR")
      return "YVR";

    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }
  const SurfaceSectorExempt*
  getSurfaceSectorExempt(const LocCode& origLoc, const LocCode& destLoc, const DateTime& date)
  {
    if (origLoc == "YVR" && destLoc == "NRT")
      return 0;
    else if (origLoc == "CGN" && destLoc == "DUS")
      return _memHandle.create<SurfaceSectorExempt>();
    return DataHandleMock::getSurfaceSectorExempt(origLoc, destLoc, date);
  }
  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    Mileage* ret = _memHandle.create<Mileage>();
    ret->orig() = origin;
    ret->dest() = dest;
    ret->mileageType() = mileageType;
    if (origin == "YVR" && dest == "NRT")
    {
      ret->mileage() = 4681;
      return ret;
    }
    else if (origin == "SFO" && dest == "YVR")
    {
      ret->mileage() = 800;
      return ret;
    }

    return DataHandleMock::getMileage(origin, dest, mileageType, globalDir, date);
  }
  const std::vector<SurfaceSectorExemptionInfo*>&
  getSurfaceSectorExemptionInfo(const CarrierCode& carrierCode, const DateTime& ticketDate)
  {
    if (carrierCode == "")
      return *_memHandle.create<std::vector<SurfaceSectorExemptionInfo*> >();
    return DataHandleMock::getSurfaceSectorExemptionInfo(carrierCode, ticketDate);
  }
};
}
class LimitationOnIndirectTravelTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(LimitationOnIndirectTravelTest);
  CPPUNIT_TEST(testValidateJourney);
  CPPUNIT_TEST(testPuBrazilExcept);
  CPPUNIT_TEST(testValidateFareComponent);
  CPPUNIT_TEST(testRetransitFcBoardOffPoint);
  CPPUNIT_TEST(testFcInterlineRestr);
  CPPUNIT_TEST(testFcAAExcept);
  CPPUNIT_TEST(testFcUAExcept);
  CPPUNIT_TEST(testFcVNExcept);
  CPPUNIT_TEST(testFcJourneyWithinArea1);
  CPPUNIT_TEST(testFcJourneyOrigInEurope);
  CPPUNIT_TEST(testFcThruFare);
  CPPUNIT_TEST(testArea1RetransitExcept);
  CPPUNIT_TEST(testArea1RetransitExceptAA);
  CPPUNIT_TEST(testAllCxrStopOverExcept);
  CPPUNIT_TEST(testSurfTrvl);
  CPPUNIT_TEST(testStopOverAtEurope);
  CPPUNIT_SKIP_TEST(testGetAllLitItems);
  CPPUNIT_TEST(testMultiCity);

  CPPUNIT_TEST(testGetCrsFromVendoCrxCode_1S);
  CPPUNIT_TEST(testGetCrsFromCrxCode_AA);
  CPPUNIT_TEST(testGetCrs_FromHostCrx_Return_Blank);

  CPPUNIT_TEST(testDefineDiagCxrRequested_DiagNotSet_Return_True);
  CPPUNIT_TEST(testDefineDiagCxrRequested_DiagSet_CxrNotSet_Return_True);
  CPPUNIT_TEST(testDefineDiagCxrRequested_DiagSet_CxrSet_Match_Return_True);
  CPPUNIT_TEST(testDefineDiagCxrRequested_DiagSet_CxrSet_NotMatch_Return_False);
  CPPUNIT_TEST(testCheckDiagReqCxr_CxrNotRequested_Return_True);
  CPPUNIT_TEST(testCheckDiagReqCxr_CxrIsRequested_Return_False);
  CPPUNIT_TEST(testAnyCxrPass_All_CxrFail_Return_False);
  CPPUNIT_TEST(testAnyCxrPass_DiagRequested_CxrFail_Return_False);
  CPPUNIT_TEST(testAnyCxrPass_DiagRequested_CxrNotFound_Return_True);
  CPPUNIT_TEST(testDefineDiagSQinRequest_NoDiag_Return_True);
  CPPUNIT_TEST(testDefineDiagSQinRequest_OverSize_Return_False);
  CPPUNIT_TEST(testDefineDiagSQinRequest_SeqIsFound_Return_True);
  CPPUNIT_TEST(testDefineDiagSQinRequest_SeqIsNotFound_Return_True);
  CPPUNIT_TEST(testDisplayValitionResult_MATCH);
  CPPUNIT_TEST(testDisplayValitionResult_FAIL);

  CPPUNIT_TEST(testPrepareDiag372OutPut_No_FP);
  CPPUNIT_TEST(testPrepareDiag372OutPut_Include_FP);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
    trx.diagnostic().diagnosticType() = Diagnostic370;
    trx.diagnostic().activate();

    request.ticketingDT() = DateTime::localTime();
    trx.setRequest(&request);
    trx.setOptions(_memHandle.create<PricingOptions>());
    _farePath = _memHandle.create<FarePath>();
  }

  void tearDown() { _memHandle.clear(); }

  void testValidateJourney()
  {
    Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");
    Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocZRH.xml");
    Loc* loc3 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocFRA.xml");
    // Loc* loc4 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocGVA.xml");
    Loc* loc5 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocVIE.xml");
    // Loc* loc5 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNRT.xml");
    Loc* loc7 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocPAR.xml");

    Loc loc4;
    loc4.loc() = "GVA";
    loc4.nation() = "CH";
    loc4.area() = "2";
    loc4.subarea() = EUROPE;

    Loc loc6;
    loc6.loc() = "NRT";
    loc6.nation() = "JP";
    loc6.area() = "3";
    loc6.subarea() = "33"; // For Japan/Korea

    AirSeg trvlSeg1;
    trvlSeg1.origin() = loc1;
    trvlSeg1.destination() = loc2;
    trvlSeg1.carrier() = "SR";
    trvlSeg1.boardMultiCity() = trvlSeg1.origin()->loc();
    trvlSeg1.offMultiCity() = trvlSeg1.destination()->loc();

    AirSeg trvlSeg2;
    trvlSeg2.origin() = loc2;
    trvlSeg2.destination() = loc3;
    trvlSeg2.carrier() = "SR";
    trvlSeg2.boardMultiCity() = trvlSeg2.origin()->loc();
    trvlSeg2.offMultiCity() = trvlSeg2.destination()->loc();

    AirSeg trvlSeg3;
    trvlSeg3.origin() = loc3;
    trvlSeg3.destination() = &loc4;
    trvlSeg3.carrier() = "OS";
    trvlSeg3.boardMultiCity() = trvlSeg3.origin()->loc();
    trvlSeg3.offMultiCity() = trvlSeg3.destination()->loc();

    AirSeg trvlSeg4;
    trvlSeg4.origin() = &loc4;
    trvlSeg4.destination() = loc5;
    trvlSeg4.carrier() = "OS";
    trvlSeg4.boardMultiCity() = trvlSeg4.origin()->loc();
    trvlSeg4.offMultiCity() = trvlSeg4.destination()->loc();

    AirSeg trvlSeg5;
    trvlSeg5.origin() = loc5;
    trvlSeg5.destination() = &loc4;
    trvlSeg5.carrier() = "OS";
    trvlSeg5.boardMultiCity() = trvlSeg5.origin()->loc();
    trvlSeg5.offMultiCity() = trvlSeg5.destination()->loc();

    AirSeg trvlSeg6;
    trvlSeg6.origin() = &loc4;
    trvlSeg6.destination() = loc7;
    trvlSeg6.carrier() = "AF";
    trvlSeg6.boardMultiCity() = trvlSeg6.origin()->loc();
    trvlSeg6.offMultiCity() = trvlSeg6.destination()->loc();

    AirSeg trvlSeg7;
    trvlSeg7.origin() = loc7;
    trvlSeg7.destination() = loc2;
    trvlSeg7.carrier() = "SR";
    trvlSeg7.boardMultiCity() = trvlSeg7.origin()->loc();
    trvlSeg7.offMultiCity() = trvlSeg7.destination()->loc();

    AirSeg trvlSeg8;
    trvlSeg8.origin() = loc2;
    trvlSeg8.destination() = &loc6;
    trvlSeg8.carrier() = "SR";
    trvlSeg8.boardMultiCity() = trvlSeg8.origin()->loc();
    trvlSeg8.offMultiCity() = trvlSeg8.destination()->loc();

    AirSeg trvlSeg9;
    trvlSeg9.origin() = &loc6;
    trvlSeg9.destination() = loc2;
    trvlSeg9.carrier() = "SR";
    trvlSeg9.boardMultiCity() = trvlSeg9.origin()->loc();
    trvlSeg9.offMultiCity() = trvlSeg9.destination()->loc();

    Itin itin;
    itin.geoTravelType() = GeoTravelType::International;

    itin.setTravelDate(DateTime::localTime());

    itin.travelSeg().push_back(&trvlSeg1);
    itin.travelSeg().push_back(&trvlSeg2);
    itin.travelSeg().push_back(&trvlSeg3);
    itin.travelSeg().push_back(&trvlSeg4);
    itin.travelSeg().push_back(&trvlSeg5);
    itin.travelSeg().push_back(&trvlSeg6);
    itin.travelSeg().push_back(&trvlSeg7);
    itin.travelSeg().push_back(&trvlSeg8);
    itin.travelSeg().push_back(&trvlSeg9);

    LimitationOnIndirectTravel limitation(trx, itin);
    try
    {
      limitation.validateJourney();
      CPPUNIT_ASSERT_MESSAGE("Expected ErrorResponseException", false);
    }
    catch (ErrorResponseException& ex)
    {
      CPPUNIT_ASSERT_MESSAGE("\nCaught ErrorResponseException:: as expected \n" + ex.message(),
                             true);
    }
  }

  void testPuBrazilExcept()
  {
    PaxType paxType;
    paxType.paxType() = ADULT;

    Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocFLN.xml");
    Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocCWB.xml");
    Loc* loc3 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocIGU.xml");
    Loc* loc4 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSAO.xml");
    Loc* loc5 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLHR.xml");

    loc4->nation() = "";

    AirSeg trvlSeg1;
    trvlSeg1.origin() = loc1;
    trvlSeg1.destination() = loc2;
    trvlSeg1.carrier() = "RG";
    trvlSeg1.boardMultiCity() = trvlSeg1.origin()->loc();
    trvlSeg1.offMultiCity() = trvlSeg1.destination()->loc();

    AirSeg trvlSeg2;
    trvlSeg2.origin() = loc2;
    trvlSeg2.destination() = loc3;
    trvlSeg2.carrier() = "VP";
    trvlSeg2.boardMultiCity() = trvlSeg2.origin()->loc();
    trvlSeg2.offMultiCity() = trvlSeg2.destination()->loc();

    AirSeg trvlSeg3;
    trvlSeg3.origin() = loc3;
    trvlSeg3.destination() = loc4;
    trvlSeg3.carrier() = "VP";
    trvlSeg3.boardMultiCity() = trvlSeg3.origin()->loc();
    trvlSeg3.offMultiCity() = trvlSeg3.destination()->loc();

    AirSeg trvlSeg4;
    trvlSeg4.origin() = loc4;
    trvlSeg4.destination() = loc5;
    trvlSeg4.carrier() = "RG";
    trvlSeg4.boardMultiCity() = trvlSeg4.origin()->loc();
    trvlSeg4.offMultiCity() = trvlSeg4.destination()->loc();

    // Test Outbound
    FareMarket fareMarket;
    fareMarket.governingCarrier() = "VP";
    fareMarket.geoTravelType() = GeoTravelType::International;
    fareMarket.travelSeg().push_back(&trvlSeg1);
    fareMarket.travelSeg().push_back(&trvlSeg2);
    fareMarket.travelSeg().push_back(&trvlSeg3);
    fareMarket.travelSeg().push_back(&trvlSeg4);
    fareMarket.direction() = FMDirection::OUTBOUND;

    Fare fare;

    PaxTypeFare paxTypeFare;
    paxTypeFare.initialize(&fare, &paxType, &fareMarket);

    FareUsage fareUsage;
    fareUsage.paxTypeFare() = &paxTypeFare;

    PricingUnit pu;
    pu.fareUsage().push_back(&fareUsage);
    pu.travelSeg().push_back(&trvlSeg1);
    pu.travelSeg().push_back(&trvlSeg2);
    pu.travelSeg().push_back(&trvlSeg3);
    pu.travelSeg().push_back(&trvlSeg4);
    pu.geoTravelType() = GeoTravelType::International;

    Itin itin;
    itin.geoTravelType() = GeoTravelType::International;

    itin.setTravelDate(DateTime::localTime());

    itin.travelSeg().push_back(&trvlSeg1);
    itin.travelSeg().push_back(&trvlSeg2);
    itin.travelSeg().push_back(&trvlSeg3);
    itin.travelSeg().push_back(&trvlSeg4);

    LimitationOnIndirectTravel limitation(trx, itin);
    DCFactory* factory = DCFactory::instance();
    DiagCollector* diag682 = factory->create(trx);

    CPPUNIT_ASSERT(limitation.validatePricingUnit(pu, *diag682));

    // Test Inbound
    FareMarket fareMarket1;
    fareMarket1.governingCarrier() = "VP";
    fareMarket1.geoTravelType() = GeoTravelType::International;
    fareMarket1.travelSeg().push_back(&trvlSeg1);
    fareMarket1.travelSeg().push_back(&trvlSeg2);
    fareMarket1.travelSeg().push_back(&trvlSeg3);
    fareMarket1.travelSeg().push_back(&trvlSeg4);
    fareMarket1.direction() = FMDirection::INBOUND;

    Fare fare1;

    PaxTypeFare paxTypeFare1;
    paxTypeFare1.initialize(&fare1, &paxType, &fareMarket1);

    FareUsage fareUsage1;
    fareUsage1.paxTypeFare() = &paxTypeFare1;

    PricingUnit pu1;
    pu1.fareUsage().push_back(&fareUsage1);
    pu1.travelSeg().push_back(&trvlSeg1);
    pu1.travelSeg().push_back(&trvlSeg2);
    pu1.travelSeg().push_back(&trvlSeg3);
    pu1.travelSeg().push_back(&trvlSeg4);
    pu1.geoTravelType() = GeoTravelType::International;

    CPPUNIT_ASSERT(limitation.validatePricingUnit(pu1, *diag682));

    string str = trx.diagnostic().toString();
    // cout << str << endl;
  }

  void testValidateFareComponent()
  {
    Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocZRH.xml");
    Loc* loc3 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocFRA.xml");

    AirSeg trvlSeg1;
    trvlSeg1.origin() = loc1;
    trvlSeg1.destination() = loc2;
    trvlSeg1.carrier() = "AA";
    trvlSeg1.boardMultiCity() = trvlSeg1.origin()->loc();
    trvlSeg1.offMultiCity() = trvlSeg1.destination()->loc();

    AirSeg trvlSeg2;
    trvlSeg2.origin() = loc2;
    trvlSeg2.destination() = loc3;
    trvlSeg2.carrier() = "SR";
    trvlSeg2.boardMultiCity() = trvlSeg2.origin()->loc();
    trvlSeg2.offMultiCity() = trvlSeg2.destination()->loc();

    FareMarket fareMarket;
    fareMarket.governingCarrier() = "AA";
    fareMarket.geoTravelType() = GeoTravelType::International;
    fareMarket.travelSeg().push_back(&trvlSeg1);
    fareMarket.travelSeg().push_back(&trvlSeg2);

    Itin itin;
    itin.geoTravelType() = GeoTravelType::International;

    itin.setTravelDate(DateTime::localTime());

    itin.travelSeg().push_back(&trvlSeg1);
    itin.travelSeg().push_back(&trvlSeg2);

    LimitationOnIndirectTravel limitation(trx, itin);
    CPPUNIT_ASSERT(limitation.validateFareComponent(fareMarket));

    string str = trx.diagnostic().toString();
    // cout << str << endl;
  }

  void testRetransitFcBoardOffPoint()
  {
    Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");
    Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocZRH.xml");
    Loc* loc4 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocFRA.xml");

    Loc loc3;
    loc3.loc() = "LGA";
    loc3.nation() = "US";
    loc3.area() = "1";
    loc3.subarea() = NORTH_AMERICA;

    AirSeg trvlSeg1;
    trvlSeg1.origin() = loc1;
    trvlSeg1.destination() = loc2;
    trvlSeg1.carrier() = "AA";
    trvlSeg1.origAirport() = "NYC";
    trvlSeg1.destAirport() = trvlSeg1.destination()->loc();
    trvlSeg1.geoTravelType() = GeoTravelType::International;

    AirSeg trvlSeg2;
    trvlSeg2.origin() = loc2;
    trvlSeg2.destination() = &loc3;
    trvlSeg2.carrier() = "SR";
    trvlSeg2.origAirport() = trvlSeg2.origin()->loc();
    trvlSeg2.destAirport() = "NYC";
    trvlSeg2.geoTravelType() = GeoTravelType::International;

    AirSeg trvlSeg3;
    trvlSeg3.origin() = &loc3;
    trvlSeg3.destination() = loc4;
    trvlSeg3.carrier() = "SR";
    trvlSeg3.origAirport() = "NYC";
    trvlSeg3.destAirport() = trvlSeg3.destination()->loc();
    trvlSeg3.geoTravelType() = GeoTravelType::International;

    FareMarket fareMarket;
    fareMarket.governingCarrier() = "AA";
    fareMarket.geoTravelType() = GeoTravelType::International;
    fareMarket.travelSeg().push_back(&trvlSeg1);
    fareMarket.travelSeg().push_back(&trvlSeg2);
    fareMarket.travelSeg().push_back(&trvlSeg3);

    Itin itin;
    LimitationOnIndirectTravel limitation(trx, itin);
    CPPUNIT_ASSERT(limitation.retransitFareComponentBoardOffPoint(fareMarket));

    string str = trx.diagnostic().toString();
    // cout << str << endl;
  }

  void testFcInterlineRestr()
  {
    Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLHR.xml");

    Loc loc2;
    loc2.loc() = "ADD";
    loc2.nation() = "ET";
    loc2.area() = "2";
    loc2.subarea() = "23";

    Loc loc3;
    loc3.loc() = "NBO";
    loc3.nation() = "CA";
    loc3.area() = "1";
    loc3.subarea() = NORTH_AMERICA;

    AirSeg trvlSeg1;
    trvlSeg1.origin() = loc1;
    trvlSeg1.destination() = &loc2;
    trvlSeg1.carrier() = "BA";
    trvlSeg1.boardMultiCity() = trvlSeg1.origin()->loc();
    trvlSeg1.offMultiCity() = trvlSeg1.destination()->loc();

    AirSeg trvlSeg2;
    trvlSeg2.origin() = &loc2;
    trvlSeg2.destination() = &loc3;
    trvlSeg2.carrier() = "ET";
    trvlSeg2.boardMultiCity() = trvlSeg2.origin()->loc();
    trvlSeg2.offMultiCity() = trvlSeg2.destination()->loc();

    FareMarket fareMarket;
    fareMarket.governingCarrier() = "BA";
    fareMarket.geoTravelType() = GeoTravelType::International;
    fareMarket.travelSeg().push_back(&trvlSeg1);
    fareMarket.travelSeg().push_back(&trvlSeg2);

    Itin itin;
    itin.geoTravelType() = GeoTravelType::International;

    itin.setTravelDate(DateTime::localTime());

    itin.travelSeg().push_back(&trvlSeg1);
    itin.travelSeg().push_back(&trvlSeg2);

    // ET item is removed
    LimitationOnIndirectTravel limitation(trx, itin);
    CPPUNIT_ASSERT(limitation.validateFareComponent(fareMarket));

    string str = trx.diagnostic().toString();
    // cout << str << endl;
  }

  void testFcAAExcept()
  {
    Loc loc1;
    loc1.loc() = "BOS";
    loc1.nation() = "US";
    loc1.area() = "1";
    loc1.subarea() = NORTH_AMERICA;

    Loc loc2;
    loc2.loc() = "DFW";
    loc2.nation() = "US";
    loc2.area() = "1";
    loc2.subarea() = NORTH_AMERICA;

    Loc loc3;
    loc3.loc() = "AUS";
    loc3.nation() = "US";
    loc3.area() = "1";
    loc3.subarea() = NORTH_AMERICA;

    Loc loc4;
    loc4.loc() = "NRT";
    loc4.nation() = "JP";
    loc4.area() = "3";
    loc4.subarea() = "33";

    AirSeg trvlSeg1;
    trvlSeg1.origin() = &loc1;
    trvlSeg1.destination() = &loc2;
    trvlSeg1.carrier() = "AA";
    trvlSeg1.boardMultiCity() = trvlSeg1.origin()->loc();
    trvlSeg1.offMultiCity() = trvlSeg1.destination()->loc();

    AirSeg trvlSeg2;
    trvlSeg2.origin() = &loc2;
    trvlSeg2.destination() = &loc3;
    trvlSeg2.carrier() = "DL";
    trvlSeg2.boardMultiCity() = trvlSeg2.origin()->loc();
    trvlSeg2.offMultiCity() = trvlSeg2.destination()->loc();

    AirSeg trvlSeg3;
    trvlSeg3.origin() = &loc3;
    trvlSeg3.destination() = &loc2;
    trvlSeg3.carrier() = "AA";
    trvlSeg3.boardMultiCity() = trvlSeg3.origin()->loc();
    trvlSeg3.offMultiCity() = trvlSeg3.destination()->loc();

    AirSeg trvlSeg4;
    trvlSeg4.origin() = &loc2;
    trvlSeg4.destination() = &loc4;
    trvlSeg4.carrier() = "AA";
    trvlSeg4.boardMultiCity() = trvlSeg4.origin()->loc();
    trvlSeg4.offMultiCity() = trvlSeg4.destination()->loc();

    FareMarket fareMarket;
    fareMarket.governingCarrier() = "AA";
    fareMarket.setGlobalDirection(GlobalDirection::PA);
    fareMarket.geoTravelType() = GeoTravelType::International;
    fareMarket.travelSeg().push_back(&trvlSeg1);
    fareMarket.travelSeg().push_back(&trvlSeg2);
    fareMarket.travelSeg().push_back(&trvlSeg3);
    fareMarket.travelSeg().push_back(&trvlSeg4);

    Itin itin;
    itin.geoTravelType() = GeoTravelType::International;
    itin.setTravelDate(DateTime::localTime());

    itin.travelSeg().push_back(&trvlSeg1);
    itin.travelSeg().push_back(&trvlSeg2);
    itin.travelSeg().push_back(&trvlSeg3);
    itin.travelSeg().push_back(&trvlSeg4);

    LimitationOnIndirectTravel limitation(trx, itin);
    CPPUNIT_ASSERT(!limitation.validateFareComponent(fareMarket));

    trvlSeg2.carrier() = "AA";
    CPPUNIT_ASSERT(limitation.validateFareComponent(fareMarket));

    // string str = trx.diagnostic().toString();
    // cout << str << endl;
  }

  void testFcUAExcept()
  {
    Loc loc1;
    loc1.loc() = "BOS";
    loc1.nation() = "US";
    loc1.area() = "1";
    loc1.subarea() = NORTH_AMERICA;

    Loc loc2;
    loc2.loc() = "ORD";
    loc2.nation() = "US";
    loc2.area() = "1";
    loc2.subarea() = NORTH_AMERICA;

    Loc loc3;
    loc3.loc() = "DEN";
    loc3.nation() = "US";
    loc3.area() = "1";
    loc3.subarea() = NORTH_AMERICA;

    Loc loc4;
    loc4.loc() = "NRT";
    loc4.nation() = "JP";
    loc4.area() = "3";
    loc4.subarea() = "33";

    AirSeg trvlSeg1;
    trvlSeg1.origin() = &loc1;
    trvlSeg1.destination() = &loc2;
    trvlSeg1.carrier() = "UA";
    trvlSeg1.boardMultiCity() = trvlSeg1.origin()->loc();
    trvlSeg1.offMultiCity() = trvlSeg1.destination()->loc();

    AirSeg trvlSeg2;
    trvlSeg2.origin() = &loc2;
    trvlSeg2.destination() = &loc3;
    trvlSeg2.carrier() = "UA";
    trvlSeg2.boardMultiCity() = trvlSeg2.origin()->loc();
    trvlSeg2.offMultiCity() = trvlSeg2.destination()->loc();

    AirSeg trvlSeg3;
    trvlSeg3.origin() = &loc3;
    trvlSeg3.destination() = &loc2;
    trvlSeg3.carrier() = "UA";
    trvlSeg3.boardMultiCity() = trvlSeg3.origin()->loc();
    trvlSeg3.offMultiCity() = trvlSeg3.destination()->loc();

    AirSeg trvlSeg4;
    trvlSeg4.origin() = &loc2;
    trvlSeg4.destination() = &loc4;
    trvlSeg4.carrier() = "JL";
    trvlSeg4.boardMultiCity() = trvlSeg4.origin()->loc();
    trvlSeg4.offMultiCity() = trvlSeg4.destination()->loc();

    FareMarket fareMarket;
    fareMarket.governingCarrier() = "UA";
    fareMarket.setGlobalDirection(GlobalDirection::PA);
    fareMarket.geoTravelType() = GeoTravelType::International;
    fareMarket.travelSeg().push_back(&trvlSeg1);
    fareMarket.travelSeg().push_back(&trvlSeg2);
    fareMarket.travelSeg().push_back(&trvlSeg3);
    fareMarket.travelSeg().push_back(&trvlSeg4);

    Itin itin;
    itin.geoTravelType() = GeoTravelType::International;
    itin.setTravelDate(DateTime::localTime());
    itin.travelSeg().push_back(&trvlSeg1);
    itin.travelSeg().push_back(&trvlSeg2);
    itin.travelSeg().push_back(&trvlSeg3);
    itin.travelSeg().push_back(&trvlSeg4);

    LimitationOnIndirectTravel limitation(trx, itin);
    CPPUNIT_ASSERT(!limitation.validateFareComponent(fareMarket));

    trvlSeg4.carrier() = "UA";
    CPPUNIT_ASSERT(limitation.validateFareComponent(fareMarket));

    string str = trx.diagnostic().toString();
    // cout << str << endl;
  }

  void testFcVNExcept()
  {
    Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLAX.xml");
    Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHKG.xml");
    Loc* loc3 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocHAN.xml");
    Loc* loc4 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSGN.xml");
    Loc* loc5 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocBKK.xml");

    AirSeg trvlSeg1;
    trvlSeg1.origin() = loc1;
    trvlSeg1.destination() = loc2;
    trvlSeg1.carrier() = "AA";
    trvlSeg1.stopOver() = true;
    trvlSeg1.boardMultiCity() = trvlSeg1.origin()->loc();
    trvlSeg1.offMultiCity() = trvlSeg1.destination()->loc();

    AirSeg trvlSeg2;
    trvlSeg2.origin() = loc2;
    trvlSeg2.destination() = loc3;
    trvlSeg2.carrier() = "CX";
    trvlSeg2.stopOver() = true;
    trvlSeg2.boardMultiCity() = trvlSeg2.origin()->loc();
    trvlSeg2.offMultiCity() = trvlSeg2.destination()->loc();

    AirSeg trvlSeg3;
    trvlSeg3.origin() = loc3;
    trvlSeg3.destination() = loc4;
    trvlSeg3.carrier() = "VN";
    trvlSeg3.stopOver() = true;
    trvlSeg3.boardMultiCity() = trvlSeg3.origin()->loc();
    trvlSeg3.offMultiCity() = trvlSeg3.destination()->loc();

    AirSeg trvlSeg4;
    trvlSeg4.origin() = loc4;
    trvlSeg4.destination() = loc5;
    trvlSeg4.carrier() = "TG";
    trvlSeg4.stopOver() = true;
    trvlSeg4.boardMultiCity() = trvlSeg4.origin()->loc();
    trvlSeg4.offMultiCity() = trvlSeg4.destination()->loc();

    FareMarket fareMarket;
    fareMarket.governingCarrier() = "CX";
    fareMarket.setGlobalDirection(GlobalDirection::PA);
    fareMarket.travelSeg().push_back(&trvlSeg1);
    fareMarket.travelSeg().push_back(&trvlSeg2);
    fareMarket.travelSeg().push_back(&trvlSeg3);
    fareMarket.travelSeg().push_back(&trvlSeg4);

    Itin itin;
    itin.geoTravelType() = GeoTravelType::International;
    itin.setTravelDate(DateTime::localTime());

    itin.travelSeg().push_back(&trvlSeg1);
    itin.travelSeg().push_back(&trvlSeg2);
    itin.travelSeg().push_back(&trvlSeg3);
    itin.travelSeg().push_back(&trvlSeg4);

    LimitationOnIndirectTravel limitation(trx, itin);

    CPPUNIT_ASSERT(limitation.validateFareComponent(fareMarket));

    string str = trx.diagnostic().toString();
    // cout << str << endl;
  }

  void testFcJourneyWithinArea1()
  {
    // Set up ticket loc
    Loc* loc = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSEA.xml");
    Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSEA.xml");
    Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMEX.xml");
    Loc* loc3 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMIA.xml");
    Loc* loc4 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocGIG.xml");

    Agent agent;
    agent.agentLocation() = loc;

    PricingRequest request;
    request.ticketingAgent() = &agent;

    trx.setRequest(&request);

    AirSeg trvlSeg1;
    trvlSeg1.origin() = loc1;
    trvlSeg1.destination() = loc2;
    trvlSeg1.carrier() = "UA";
    trvlSeg1.stopOver() = true;
    trvlSeg1.boardMultiCity() = trvlSeg1.origin()->loc();
    trvlSeg1.offMultiCity() = trvlSeg1.destination()->loc();

    AirSeg trvlSeg2;
    trvlSeg2.origin() = loc2;
    trvlSeg2.destination() = loc3;
    trvlSeg2.carrier() = "AA";
    trvlSeg2.stopOver() = true;
    trvlSeg2.boardMultiCity() = trvlSeg2.origin()->loc();
    trvlSeg2.offMultiCity() = trvlSeg2.destination()->loc();

    AirSeg trvlSeg3;
    trvlSeg3.origin() = loc3;
    trvlSeg3.destination() = loc4;
    trvlSeg3.carrier() = "AA";
    trvlSeg3.stopOver() = true;
    trvlSeg3.boardMultiCity() = trvlSeg3.origin()->loc();
    trvlSeg3.offMultiCity() = trvlSeg3.destination()->loc();

    FareMarket fareMarket;
    fareMarket.governingCarrier() = "UA";
    fareMarket.setGlobalDirection(GlobalDirection::WH);
    fareMarket.geoTravelType() = GeoTravelType::International;
    fareMarket.travelSeg().push_back(&trvlSeg1);
    fareMarket.travelSeg().push_back(&trvlSeg2);
    fareMarket.travelSeg().push_back(&trvlSeg3);
    fareMarket.direction() = FMDirection::OUTBOUND;

    Itin itin;
    itin.geoTravelType() = GeoTravelType::International;
    itin.setTravelDate(DateTime::localTime());

    itin.travelSeg().push_back(&trvlSeg1);
    itin.travelSeg().push_back(&trvlSeg2);
    itin.travelSeg().push_back(&trvlSeg3);

    LimitationOnIndirectTravel limitation(trx, itin);
    CPPUNIT_ASSERT(!limitation.validateFareComponent(fareMarket));

    string str = trx.diagnostic().toString();
    // cout << str << endl;
  }

  void testFcJourneyOrigInEurope()
  {
    // Create locs
    Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocFRA.xml");
    Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocZRH.xml");
    Loc* loc3 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMUC.xml");
    Loc* loc4 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocJFK.xml");

    AirSeg trvlSeg1;
    trvlSeg1.origin() = loc1;
    trvlSeg1.destination() = loc2;
    trvlSeg1.carrier() = "LH";
    trvlSeg1.stopOver() = true;
    trvlSeg1.boardMultiCity() = trvlSeg1.origin()->loc();
    trvlSeg1.offMultiCity() = trvlSeg1.destination()->loc();

    AirSeg trvlSeg2;
    trvlSeg2.origin() = loc2;
    trvlSeg2.destination() = loc3;
    trvlSeg2.carrier() = "LH";
    trvlSeg2.stopOver() = true;
    trvlSeg2.boardMultiCity() = trvlSeg2.origin()->loc();
    trvlSeg2.offMultiCity() = trvlSeg2.destination()->loc();

    AirSeg trvlSeg3;
    trvlSeg3.origin() = loc3;
    trvlSeg3.destination() = loc4;
    trvlSeg3.carrier() = "LH";
    trvlSeg3.stopOver() = true;
    trvlSeg3.boardMultiCity() = trvlSeg3.origin()->loc();
    trvlSeg3.offMultiCity() = trvlSeg3.destination()->loc();

    FareMarket fareMarket;
    fareMarket.governingCarrier() = "LH";
    fareMarket.setGlobalDirection(GlobalDirection::ZZ);
    fareMarket.travelSeg().push_back(&trvlSeg1);
    fareMarket.travelSeg().push_back(&trvlSeg2);
    fareMarket.travelSeg().push_back(&trvlSeg3);

    Itin itin;
    itin.geoTravelType() = GeoTravelType::International;
    itin.setTravelDate(DateTime::localTime());

    itin.travelSeg().push_back(&trvlSeg1);
    itin.travelSeg().push_back(&trvlSeg2);
    itin.travelSeg().push_back(&trvlSeg3);

    LimitationOnIndirectTravel limitation(trx, itin);
    CPPUNIT_ASSERT(limitation.validateFareComponent(fareMarket));

    // string str = trx.diagnostic().toString();
    // cout << str << endl;
  }

  void testFcThruFare()
  {
    // Create locs
    Loc loc1;
    loc1.loc() = "SIN";
    loc1.nation() = "MY";
    loc1.area() = "3";
    loc1.subarea() = "32";

    Loc loc2;
    loc2.loc() = "KCH";
    loc2.nation() = "MY";
    loc2.area() = "3";
    loc2.subarea() = "32";

    Loc loc3;
    loc3.loc() = "BKI";
    loc3.nation() = "MY";
    loc3.area() = "3";
    loc3.subarea() = "32";

    AirSeg trvlSeg1;
    trvlSeg1.origin() = &loc1;
    trvlSeg1.destination() = &loc2;
    trvlSeg1.carrier() = "MH";
    trvlSeg1.boardMultiCity() = trvlSeg1.origin()->loc();
    trvlSeg1.offMultiCity() = trvlSeg1.destination()->loc();

    AirSeg trvlSeg2;
    trvlSeg2.origin() = &loc2;
    trvlSeg2.destination() = &loc3;
    trvlSeg2.carrier() = "MH";
    trvlSeg2.resStatus() = "OK";
    trvlSeg2.boardMultiCity() = trvlSeg2.origin()->loc();
    trvlSeg2.offMultiCity() = trvlSeg2.destination()->loc();

    FareMarket fareMarket;
    fareMarket.governingCarrier() = "MH";
    fareMarket.setGlobalDirection(GlobalDirection::EH);
    fareMarket.geoTravelType() = GeoTravelType::International;
    fareMarket.travelSeg().push_back(&trvlSeg1);
    fareMarket.travelSeg().push_back(&trvlSeg2);

    Itin itin;
    itin.geoTravelType() = GeoTravelType::International;
    itin.setTravelDate(DateTime::localTime());
    itin.travelSeg().push_back(&trvlSeg1);
    itin.travelSeg().push_back(&trvlSeg2);

    // Create first PaxTypeFare
    Fare fare1;
    fare1.nucFareAmount() = 1845.26;

    FareInfo fareInfo1;
    fareInfo1._carrier = "MH";
    fareInfo1._market1 = "SIN";
    fareInfo1._market2 = "BKI";
    fareInfo1._fareClass = "WMLUQOW";
    fareInfo1._fareAmount = 999.00;
    fareInfo1._currency = "GBP";
    fareInfo1._owrt = ONE_WAY_MAYNOT_BE_DOUBLED;
    fareInfo1._ruleNumber = "5135";
    fareInfo1._routingNumber = "XXXX";
    fareInfo1._directionality = FROM;
    fareInfo1._globalDirection = GlobalDirection::ZZ;
    fareInfo1._vendor = Vendor::ATPCO;

    TariffCrossRefInfo tariffRefInfo1;
    tariffRefInfo1._fareTariffCode = "TAFPBA";

    fare1.initialize(Fare::FS_International, &fareInfo1, fareMarket, &tariffRefInfo1);

    PaxType paxType;
    paxType.paxType() = "ADT";

    PaxTypeFare paxTypeFare1;
    paxTypeFare1.initialize(&fare1, &paxType, &fareMarket);

    FareClassAppInfo appInfo1;
    appInfo1._fareType = "EU";
    appInfo1._pricingCatType = 'N';
    paxTypeFare1.fareClassAppInfo() = &appInfo1;

    FareClassAppSegInfo fareClassAppSegInfo1;
    fareClassAppSegInfo1._paxType = "ADT";
    paxTypeFare1.fareClassAppSegInfo() = &fareClassAppSegInfo1;
    paxTypeFare1.cabin().setEconomyClass();

    trvlSeg1.departureDT() = DateTime::localTime();
    trx.travelSeg().push_back(&trvlSeg1);

    FareUsage fu;
    fu.paxTypeFare() = &paxTypeFare1;

    LimitationOnIndirectTravel limitation(trx, itin);

    trvlSeg1.stopOver() = false;
    trvlSeg1.resStatus() = "OK";
    CPPUNIT_ASSERT(limitation.validateFareComponentAfterHip(fu));

    trvlSeg1.stopOver() = true;
    CPPUNIT_ASSERT(!limitation.validateFareComponentAfterHip(fu));

    trvlSeg1.stopOver() = false;
    CPPUNIT_ASSERT(limitation.validateFareComponentAfterHip(fu));

    trvlSeg1.resStatus() = "";
    CPPUNIT_ASSERT(!limitation.validateFareComponentAfterHip(fu));

    string str = trx.diagnostic().toString();
    // cout << str << endl;
  }

  void testArea1RetransitExcept()
  {
    Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMEX.xml");
    Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLAX.xml");
    Loc* loc4 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLAX.xml");

    Loc loc3;
    loc3.loc() = "YVR";
    loc3.cityInd() = true;
    loc3.nation() = "CA";
    loc3.area() = "1";
    loc3.subarea() = NORTH_AMERICA;

    Loc loc5;
    loc5.loc() = "YYZ";
    loc5.cityInd() = true;
    loc5.nation() = "CA";
    loc5.area() = "1";
    loc5.subarea() = NORTH_AMERICA;

    AirSeg trvlSeg1;
    trvlSeg1.origin() = loc1;
    trvlSeg1.destination() = loc2;
    trvlSeg1.carrier() = "MX";
    trvlSeg1.stopOver() = false;
    trvlSeg1.boardMultiCity() = trvlSeg1.origin()->loc();
    trvlSeg1.offMultiCity() = trvlSeg1.destination()->loc();

    AirSeg trvlSeg2;
    trvlSeg2.origin() = loc2;
    trvlSeg2.destination() = &loc3;
    trvlSeg2.carrier() = "CA";
    trvlSeg2.stopOver() = false;
    trvlSeg2.boardMultiCity() = trvlSeg2.origin()->loc();
    trvlSeg2.offMultiCity() = trvlSeg2.destination()->loc();

    AirSeg trvlSeg3;
    trvlSeg3.origin() = &loc3;
    trvlSeg3.destination() = loc4;
    trvlSeg3.carrier() = "CA";
    trvlSeg3.stopOver() = false;
    trvlSeg3.boardMultiCity() = trvlSeg3.origin()->loc();
    trvlSeg3.offMultiCity() = trvlSeg3.destination()->loc();

    AirSeg trvlSeg4;
    trvlSeg4.origin() = loc4;
    trvlSeg4.destination() = &loc5;
    trvlSeg4.carrier() = "CA";
    trvlSeg4.stopOver() = false;
    trvlSeg4.boardMultiCity() = trvlSeg4.origin()->loc();
    trvlSeg4.offMultiCity() = trvlSeg4.destination()->loc();

    FareMarket fareMarket;
    fareMarket.governingCarrier() = "CA";
    fareMarket.setGlobalDirection(GlobalDirection::WH);
    fareMarket.travelSeg().push_back(&trvlSeg1);
    fareMarket.travelSeg().push_back(&trvlSeg2);
    fareMarket.travelSeg().push_back(&trvlSeg3);
    fareMarket.travelSeg().push_back(&trvlSeg4);

    Itin itin;
    itin.geoTravelType() = GeoTravelType::International;
    itin.setTravelDate(DateTime::localTime());

    itin.travelSeg().push_back(&trvlSeg1);
    itin.travelSeg().push_back(&trvlSeg2);
    itin.travelSeg().push_back(&trvlSeg3);
    itin.travelSeg().push_back(&trvlSeg4);

    LimitationOnIndirectTravel limitation(trx, itin);

    CPPUNIT_ASSERT(!limitation.validateFareComponent(fareMarket));

    string str = trx.diagnostic().toString();
    // cout << str << endl;
  }

  void testArea1RetransitExceptAA()
  {
    Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMEX.xml");
    Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLAX.xml");
    Loc* loc4 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLAX.xml");

    Loc loc3;
    loc3.loc() = "YVR";
    loc3.cityInd() = true;
    loc3.nation() = "CA";
    loc3.area() = "1";
    loc3.subarea() = NORTH_AMERICA;

    Loc loc5;
    loc5.loc() = "YYZ";
    loc5.cityInd() = true;
    loc5.nation() = "CA";
    loc5.area() = "1";
    loc5.subarea() = NORTH_AMERICA;

    AirSeg trvlSeg1;
    trvlSeg1.origin() = loc1;
    trvlSeg1.destination() = loc2;
    trvlSeg1.carrier() = "AA";
    trvlSeg1.stopOver() = false;
    trvlSeg1.boardMultiCity() = trvlSeg1.origin()->loc();
    trvlSeg1.offMultiCity() = trvlSeg1.destination()->loc();

    AirSeg trvlSeg2;
    trvlSeg2.origin() = loc2;
    trvlSeg2.destination() = &loc3;
    trvlSeg2.carrier() = "CA";
    trvlSeg2.stopOver() = false;
    trvlSeg2.boardMultiCity() = trvlSeg2.origin()->loc();
    trvlSeg2.offMultiCity() = trvlSeg2.destination()->loc();

    AirSeg trvlSeg3;
    trvlSeg3.origin() = &loc3;
    trvlSeg3.destination() = loc4;
    trvlSeg3.carrier() = "CA";
    trvlSeg3.stopOver() = false;
    trvlSeg3.boardMultiCity() = trvlSeg3.origin()->loc();
    trvlSeg3.offMultiCity() = trvlSeg3.destination()->loc();

    AirSeg trvlSeg4;
    trvlSeg4.origin() = loc4;
    trvlSeg4.destination() = &loc5;
    trvlSeg4.carrier() = "CA";
    trvlSeg4.stopOver() = false;
    trvlSeg4.boardMultiCity() = trvlSeg4.origin()->loc();
    trvlSeg4.offMultiCity() = trvlSeg4.destination()->loc();

    FareMarket fareMarket;
    fareMarket.governingCarrier() = "AA";
    fareMarket.setGlobalDirection(GlobalDirection::WH);
    fareMarket.travelSeg().push_back(&trvlSeg1);
    fareMarket.travelSeg().push_back(&trvlSeg2);
    fareMarket.travelSeg().push_back(&trvlSeg3);
    fareMarket.travelSeg().push_back(&trvlSeg4);

    Itin itin;
    itin.geoTravelType() = GeoTravelType::International;
    itin.setTravelDate(DateTime::localTime());

    itin.travelSeg().push_back(&trvlSeg1);
    itin.travelSeg().push_back(&trvlSeg2);
    itin.travelSeg().push_back(&trvlSeg3);
    itin.travelSeg().push_back(&trvlSeg4);

    LimitationOnIndirectTravel limitation(trx, itin);

    CPPUNIT_ASSERT(!limitation.validateFareComponent(fareMarket));

    string str = trx.diagnostic().toString();
    // cout << str << endl;
  }

  void testAllCxrStopOverExcept()
  {
    Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocMEX.xml");
    Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLAX.xml");
    Loc* loc4 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLAX.xml");

    Loc loc3;
    loc3.loc() = "NRT";
    loc3.cityInd() = true;
    loc3.nation() = "JP";
    loc3.area() = "3";
    loc3.subarea() = 33;

    Loc loc5;
    loc5.loc() = "YYZ";
    loc5.cityInd() = true;
    loc5.nation() = "CA";
    loc5.area() = "1";
    loc5.subarea() = NORTH_AMERICA;

    AirSeg trvlSeg1;
    trvlSeg1.origin() = loc1;
    trvlSeg1.destination() = loc2;
    trvlSeg1.carrier() = "MX";
    trvlSeg1.stopOver() = true;
    trvlSeg1.boardMultiCity() = trvlSeg1.origin()->loc();
    trvlSeg1.offMultiCity() = trvlSeg1.destination()->loc();

    AirSeg trvlSeg2;
    trvlSeg2.origin() = loc2;
    trvlSeg2.destination() = &loc3;
    trvlSeg2.carrier() = "AA";
    trvlSeg2.stopOver() = true;
    trvlSeg2.boardMultiCity() = trvlSeg2.origin()->loc();
    trvlSeg2.offMultiCity() = trvlSeg2.destination()->loc();

    AirSeg trvlSeg3;
    trvlSeg3.origin() = &loc3;
    trvlSeg3.destination() = loc4;
    trvlSeg3.carrier() = "CA";
    trvlSeg3.stopOver() = true;
    trvlSeg3.boardMultiCity() = trvlSeg3.origin()->loc();
    trvlSeg3.offMultiCity() = trvlSeg3.destination()->loc();

    AirSeg trvlSeg4;
    trvlSeg4.origin() = loc4;
    trvlSeg4.destination() = &loc5;
    trvlSeg4.carrier() = "CA";
    trvlSeg4.stopOver() = true;
    trvlSeg4.boardMultiCity() = trvlSeg4.origin()->loc();
    trvlSeg4.offMultiCity() = trvlSeg4.destination()->loc();

    FareMarket fareMarket;
    fareMarket.governingCarrier() = "CA";
    fareMarket.setGlobalDirection(GlobalDirection::WH);
    fareMarket.travelSeg().push_back(&trvlSeg1);
    fareMarket.travelSeg().push_back(&trvlSeg2);
    fareMarket.travelSeg().push_back(&trvlSeg3);
    fareMarket.travelSeg().push_back(&trvlSeg4);

    Itin itin;
    itin.geoTravelType() = GeoTravelType::International;
    itin.setTravelDate(DateTime::localTime());

    itin.travelSeg().push_back(&trvlSeg1);
    itin.travelSeg().push_back(&trvlSeg2);
    itin.travelSeg().push_back(&trvlSeg3);
    itin.travelSeg().push_back(&trvlSeg4);

    LimitationOnIndirectTravel limitation(trx, itin);

    CPPUNIT_ASSERT(!limitation.validateFareComponent(fareMarket));

    string str = trx.diagnostic().toString();
    // cout << str << endl;
  }

  void testSurfTrvl()
  {
    const Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    const Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocYVR.xml");
    const Loc* loc3 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNRT.xml");
    const Loc* loc4 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");

    Agent agent;
    agent.agentLocation() = loc1;
    trx.getRequest()->ticketingAgent() = &agent;

    AirSeg trvlSeg1;
    trvlSeg1.origin() = loc1;
    trvlSeg1.destination() = loc2;
    trvlSeg1.carrier() = "AA";
    trvlSeg1.boardMultiCity() = trvlSeg1.origin()->loc();
    trvlSeg1.offMultiCity() = trvlSeg1.destination()->loc();

    ArunkSeg arunk;
    arunk.origin() = loc2;
    arunk.destination() = loc3;
    arunk.boardMultiCity() = arunk.origin()->loc();
    arunk.offMultiCity() = arunk.destination()->loc();

    AirSeg trvlSeg2;
    trvlSeg2.origin() = loc3;
    trvlSeg2.destination() = loc4;
    trvlSeg2.carrier() = "BA";
    trvlSeg2.boardMultiCity() = trvlSeg2.origin()->loc();
    trvlSeg2.offMultiCity() = trvlSeg2.destination()->loc();

    PaxType paxType;
    paxType.paxType() = "ADT";

    FareMarket fareMarket1;
    fareMarket1.travelSeg().push_back(&trvlSeg1);
    fareMarket1.origin() = loc1;
    fareMarket1.destination() = loc2;
    Fare fare1;
    PaxTypeFare paxTypeFare1;
    paxTypeFare1.initialize(&fare1, &paxType, &fareMarket1);

    FareMarket fareMarket2;
    fareMarket2.travelSeg().push_back(&trvlSeg2);
    fareMarket2.origin() = loc3;
    fareMarket2.destination() = loc4;
    Fare fare2;
    PaxTypeFare paxTypeFare2;
    paxTypeFare2.initialize(&fare2, &paxType, &fareMarket2);

    FareUsage fareUsage1;
    fareUsage1.paxTypeFare() = &paxTypeFare1;

    FareUsage fareUsage2;
    fareUsage2.paxTypeFare() = &paxTypeFare2;

    PricingUnit pu1;
    pu1.fareUsage().push_back(&fareUsage1);
    pu1.puFareType() = PricingUnit::NL;

    PricingUnit pu2;
    pu2.fareUsage().push_back(&fareUsage2);
    pu2.puFareType() = PricingUnit::NL;

    Itin itin;
    itin.geoTravelType() = GeoTravelType::International;
    itin.setTravelDate(DateTime::localTime());

    itin.travelSeg().push_back(&trvlSeg1);
    itin.travelSeg().push_back(&arunk);
    itin.travelSeg().push_back(&trvlSeg2);

    FarePath farePath;
    farePath.itin() = &itin;
    farePath.pricingUnit().push_back(&pu1);
    farePath.pricingUnit().push_back(&pu2);

    LimitationOnIndirectTravel limitation(trx, itin);

    CPPUNIT_ASSERT(limitation.validateIntlSurfaceTravel(farePath));

    // Test table exempt
    CPPUNIT_ASSERT(limitation.hasTpmSurfExempt("CGN", "DUS"));

    string str = trx.diagnostic().toString();
    // cout << str << endl;

    // cout << "YVR-NRT GCM " << TseUtil::greatCircleMiles(*loc2, *loc3) << endl;
  }

  void testStopOverAtEurope()
  {
    const Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocCCS.xml");
    const Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocCDG.xml");
    const Loc* loc3 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocATH.xml");

    AirSeg trvlSeg0;
    trvlSeg0.origin() = loc2;
    trvlSeg0.destination() = loc1;
    trvlSeg0.carrier() = "AF";
    trvlSeg0.stopOver() = true;
    trvlSeg0.boardMultiCity() = trvlSeg0.origin()->loc();
    trvlSeg0.offMultiCity() = trvlSeg0.destination()->loc();

    AirSeg trvlSeg1;
    trvlSeg1.origin() = loc1;
    trvlSeg1.destination() = loc2;
    trvlSeg1.carrier() = "AF";
    trvlSeg1.stopOver() = true;
    trvlSeg1.boardMultiCity() = trvlSeg1.origin()->loc();
    trvlSeg1.offMultiCity() = trvlSeg1.destination()->loc();

    AirSeg trvlSeg2;
    trvlSeg2.origin() = loc2;
    trvlSeg2.destination() = loc3;
    trvlSeg2.carrier() = "AF";
    trvlSeg2.boardMultiCity() = trvlSeg2.origin()->loc();
    trvlSeg2.offMultiCity() = trvlSeg2.destination()->loc();

    FareMarket fareMarket;
    fareMarket.governingCarrier() = "AF";
    fareMarket.setGlobalDirection(GlobalDirection::WH);
    fareMarket.geoTravelType() = GeoTravelType::International;
    fareMarket.travelSeg().push_back(&trvlSeg1);
    fareMarket.travelSeg().push_back(&trvlSeg2);

    Itin itin;
    itin.geoTravelType() = GeoTravelType::International;
    itin.setTravelDate(DateTime::localTime());
    itin.travelSeg().push_back(&trvlSeg0);
    itin.travelSeg().push_back(&trvlSeg1);
    itin.travelSeg().push_back(&trvlSeg2);

    LimitationOnIndirectTravel limitation(trx, itin);
    LimitationFare lim;
    lim.setSeqNo(50);
    lim.limitationAppl() = 'F';
    lim.govCarrierAppl() = ' ';
    lim.exceptTktgCxrInd() = ' ';
    lim.globalDir() = GlobalDirection::ZZ;
    lim.directionality() = ' ';
    lim.whollyWithinAppl() = '2';
    lim.whollyWithinLoc().locType() = 'Z';
    lim.whollyWithinLoc().loc() = "875";
    lim.originTvlAppl() = 'J';
    lim.originLoc().locType() = 'Z';
    lim.originLoc().loc() = "875";
    lim.posLoc().locType() = ' ';
    lim.potLoc().locType() = ' ';
    lim.retransitLoc().locType() = ' ';
    lim.directionality() = ' ';
    lim.notViaLoc().locType() = ' ';
    lim.maxDomSegments() = ' ';
    lim.exceptViaCxrLocInd() = ' ';
    lim.stopoverLoc().locType() = ' ';
    lim.maxStopsAtRetransit() = 0;
    lim.viaPointStopoverAppl() = '1';
    lim.notViaToFromLoc().locType() = 'Z';
    lim.notViaToFromLoc().loc() = "875";

    DiagCollector* diag = DCFactory::instance()->create(trx);
    diag->enable(Diagnostic370);

    CPPUNIT_ASSERT(!limitation.validateFareComponentItem(fareMarket, lim, diag));
    // limitation.validateFareComponentItem(fareMarket, lim, diag);

    diag->flushMsg();

    CPPUNIT_ASSERT(limitation.validateFareComponent(fareMarket));
  }

  void testGetAllLitItems()
  {
    DiagCollector* diag = DCFactory::instance()->create(trx);

    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);

    const vector<LimitationJrny*>& limJs = trx.dataHandle().getJLimitation(DateTime::localTime());
    vector<LimitationJrny*>::const_iterator iterJ = limJs.begin();
    for (; iterJ != limJs.end(); iterJ++)
    {
      (*diag) << "ITEM NO " << (*iterJ)->seqNo() << ":\n";
      lit.displayLimitation(*iterJ, *diag);
    }
    CPPUNIT_ASSERT(limJs.size() > 0);

    const vector<LimitationFare*>& limFCs = trx.dataHandle().getFCLimitation(DateTime::localTime());
    vector<LimitationFare*>::const_iterator iterFC = limFCs.begin();
    for (; iterFC != limFCs.end(); iterFC++)
    {
      (*diag) << "ITEM NO " << (*iterFC)->seqNo() << ":\n";
      lit.displayLimitation(*iterFC, *diag);
    }
    CPPUNIT_ASSERT(limFCs.size() > 0);

    diag->flushMsg();
    string str = trx.diagnostic().toString();
  }

  void testMultiCity()
  {
    DataHandle dataHandle;
    const Loc* loc1 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNRT.xml");
    const Loc* loc2 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocEWR.xml");
    const Loc* loc3 = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocBOS.xml");

    AirSeg trvlSeg;
    trvlSeg.origin() = loc1;
    trvlSeg.destination() = loc2;
    trvlSeg.origAirport() = trvlSeg.origin()->loc();
    trvlSeg.destAirport() = trvlSeg.destination()->loc();
    trvlSeg.carrier() = "JL";
    trvlSeg.boardMultiCity() = trvlSeg.origin()->loc();
    trvlSeg.offMultiCity() = "";
    trvlSeg.geoTravelType() = GeoTravelType::Domestic; // For test
    trvlSeg.departureDT() = DateTime::localTime();

    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);
    CPPUNIT_ASSERT(lit.offMultiCity(dataHandle, trvlSeg, GeoTravelType::International, "") == "NYC");

    AirSeg trvlSeg1;
    trvlSeg1.origin() = loc2;
    trvlSeg1.destination() = loc3;
    trvlSeg1.origAirport() = trvlSeg1.origin()->loc();
    trvlSeg1.destAirport() = trvlSeg1.destination()->loc();
    trvlSeg1.carrier() = ""; // Test empty carrier
    trvlSeg1.boardMultiCity() = "";
    trvlSeg1.offMultiCity() = trvlSeg1.destination()->loc();
    trvlSeg1.geoTravelType() = GeoTravelType::Domestic; // For test
    trvlSeg1.departureDT() = DateTime::localTime();

    CPPUNIT_ASSERT(lit.boardMultiCity(dataHandle, trvlSeg1, GeoTravelType::International, "") == "NYC");
  }

  void testGetCrsFromVendoCrxCode_1S()
  {
    Agent agent;
    agent.vendorCrsCode() = "1S";

    PricingRequest request;
    request.ticketingAgent() = &agent;

    trx.setRequest(&request);
    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);
    CPPUNIT_ASSERT_EQUAL(std::string("1S"), lit.getCrs());
  }

  void testGetCrsFromCrxCode_AA()
  {
    Agent agent;
    agent.cxrCode() = "AA";

    PricingRequest request;
    request.ticketingAgent() = &agent;

    trx.setRequest(&request);
    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);
    CPPUNIT_ASSERT_EQUAL(std::string("AA"), lit.getCrs());
  }

  void testGetCrs_FromHostCrx_Return_Blank()
  {
    Agent agent;
    agent.hostCarrier() = "AA";

    PricingRequest request;
    request.ticketingAgent() = &agent;

    trx.setRequest(&request);
    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);
    CPPUNIT_ASSERT_EQUAL(std::string(""), lit.getCrs());
  }

  void testDefineDiagCxrRequested_DiagNotSet_Return_True()
  {
    bool diagCxrFound = false;
    CarrierCode cxr = "AA";
    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);
    CPPUNIT_ASSERT(lit.defineDiagCxrRequested(cxr, diagCxrFound));
  }

  void testDefineDiagCxrRequested_DiagSet_CxrNotSet_Return_True()
  {
    bool diagCxrFound = false;
    CarrierCode cxr = "AA";
    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);
    lit.setDiag372InRequest();
    CPPUNIT_ASSERT(lit.defineDiagCxrRequested(cxr, diagCxrFound));
  }

  void testDefineDiagCxrRequested_DiagSet_CxrSet_Match_Return_True()
  {
    trx.diagnostic().diagnosticType() = Diagnostic372;
    trx.diagnostic().diagParamMap().insert(std::make_pair(Diagnostic::DIAG_CARRIER, "AA"));

    bool diagCxrFound = false;
    CarrierCode cxr = "AA";
    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);
    lit.setDiag372InRequest();
    CPPUNIT_ASSERT(lit.defineDiagCxrRequested(cxr, diagCxrFound));
    CPPUNIT_ASSERT(diagCxrFound);
  }

  void testDefineDiagCxrRequested_DiagSet_CxrSet_NotMatch_Return_False()
  {
    trx.diagnostic().diagnosticType() = Diagnostic372;
    trx.diagnostic().diagParamMap().insert(std::make_pair(Diagnostic::DIAG_CARRIER, "BA"));

    bool diagCxrFound = false;
    CarrierCode cxr = "AA";
    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);
    lit.setDiag372InRequest();
    CPPUNIT_ASSERT(!lit.defineDiagCxrRequested(cxr, diagCxrFound));
    CPPUNIT_ASSERT(!diagCxrFound);
  }

  void testCheckDiagReqCxr_CxrNotRequested_Return_True()
  {
    trx.diagnostic().diagnosticType() = Diagnostic372;
    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);
    CPPUNIT_ASSERT(lit.checkDiagReqCxr(false));
  }

  void testCheckDiagReqCxr_CxrIsRequested_Return_False()
  {
    trx.diagnostic().diagnosticType() = Diagnostic372;
    trx.diagnostic().diagParamMap().insert(std::make_pair(Diagnostic::DIAG_CARRIER, "AA"));

    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);
    CPPUNIT_ASSERT(!lit.checkDiagReqCxr(false));
  }

  void testAnyCxrPass_All_CxrFail_Return_False()
  {
    std::vector<CarrierCode> cxrToRemove;
    _farePath->validatingCarriers().push_back("AA");
    cxrToRemove.push_back("AA");

    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);
    CPPUNIT_ASSERT(!lit.anyCxrPass(*_farePath, cxrToRemove));
  }

  void testAnyCxrPass_DiagRequested_CxrFail_Return_False()
  {
    trx.diagnostic().diagnosticType() = Diagnostic372;
    trx.diagnostic().diagParamMap().insert(std::make_pair(Diagnostic::DIAG_CARRIER, "AA"));

    std::vector<CarrierCode> cxrToRemove;
    _farePath->validatingCarriers().push_back("AA");
    _farePath->validatingCarriers().push_back("BA");
    cxrToRemove.push_back("AA");

    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);
    bool status = lit.anyCxrPass(*_farePath, cxrToRemove);
    CPPUNIT_ASSERT(!status);
    CPPUNIT_ASSERT(!_farePath->validatingCarriers().empty());
  }

  void testAnyCxrPass_DiagRequested_CxrNotFound_Return_True()
  {
    trx.diagnostic().diagnosticType() = Diagnostic372;
    trx.diagnostic().diagParamMap().insert(std::make_pair(Diagnostic::DIAG_CARRIER, "BB"));

    std::vector<CarrierCode> cxrToRemove;
    _farePath->validatingCarriers().push_back("AA");
    _farePath->validatingCarriers().push_back("BA");
    _farePath->validatingCarriers().push_back("CA");
    cxrToRemove.push_back("AA");

    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);
    bool status = lit.anyCxrPass(*_farePath, cxrToRemove);
    CPPUNIT_ASSERT(status);
    CPPUNIT_ASSERT(!_farePath->validatingCarriers().empty());
    CPPUNIT_ASSERT_EQUAL(size_t(2), _farePath->validatingCarriers().size());
  }

  void testDefineDiagSQinRequest_NoDiag_Return_True()
  {
    CarrierCode cxr = "AA";
    Diag372Collector::DiagStream diagStr;
    int exemptionsSize = 1;
    bool seqFound = false;

    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);
    trx.diagnostic().diagnosticType() = Diagnostic370;
    trx.diagnostic().diagParamMap().erase(Diagnostic::SEQ_NUMBER);
    lit.setDiag372InRequest();
    CPPUNIT_ASSERT(lit.defineDiagSQinRequest(cxr, diagStr, exemptionsSize, seqFound));
  }

  void testDefineDiagSQinRequest_OverSize_Return_False()
  {
    CarrierCode cxr = "AA";

    int exemptionsSize = 1;
    bool seqFound = false;

    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);

    trx.diagnostic().diagnosticType() = Diagnostic372;
    trx.diagnostic().diagParamMap().insert(std::make_pair(Diagnostic::SEQ_NUMBER, "100"));
    lit.setDiag372InRequest();
    Diag372Collector::DiagStream diagStr;

    CPPUNIT_ASSERT(!lit.defineDiagSQinRequest(cxr, diagStr, exemptionsSize, seqFound));
  }

  void testDefineDiagSQinRequest_SeqIsNotFound_Return_True()
  {
    CarrierCode cxr = "AA";
    Diag372Collector::DiagStream diagStr;
    int exemptionsSize = 10;
    bool seqFound = false;
    trx.diagnostic().diagnosticType() = Diagnostic372;
    trx.diagnostic().diagParamMap().insert(std::make_pair(Diagnostic::SEQ_NUMBER, "9"));

    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);
    lit.setDiag372InRequest();

    CPPUNIT_ASSERT(lit.defineDiagSQinRequest(cxr, diagStr, exemptionsSize, seqFound));
    CPPUNIT_ASSERT(diagStr.str().find("NO SEQUENCE DETAIL FOUND") != string::npos);
  }

  void testDefineDiagSQinRequest_SeqIsFound_Return_True()
  {
    CarrierCode cxr = "AA";
    Diag372Collector::DiagStream diagStr;
    int exemptionsSize = 10;
    bool seqFound = true;
    trx.diagnostic().diagnosticType() = Diagnostic372;
    trx.diagnostic().diagParamMap().insert(std::make_pair(Diagnostic::SEQ_NUMBER, "9"));

    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);
    lit.setDiag372InRequest();

    CPPUNIT_ASSERT(lit.defineDiagSQinRequest(cxr, diagStr, exemptionsSize, seqFound));
    CPPUNIT_ASSERT(diagStr.str().find("* SEQUENCE DETAIL *") != string::npos);
  }

  void testDisplayValitionResult_MATCH()
  {
    Diag372Collector::DiagStream diagStr;
    bool result = true;
    trx.diagnostic().diagnosticType() = Diagnostic372;
    std::string failOn = "";
    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);

    lit.displayValitionResult(diagStr, result, failOn);
    CPPUNIT_ASSERT(diagStr.str().find("MATCH") != string::npos);
  }
  void testDisplayValitionResult_FAIL()
  {
    Diag372Collector::DiagStream diagStr;
    bool result = false;
    trx.diagnostic().diagnosticType() = Diagnostic372;
    std::string failOn = "";
    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);

    lit.displayValitionResult(diagStr, result, failOn);
    CPPUNIT_ASSERT(diagStr.str().find("FAIL -") != string::npos);
  }

  void testPrepareDiag372OutPut_No_FP()
  {
    FarePath fp;
    Diag372Collector::DiagStream diagStr;
    std::string crs = "1S";
    Itin itin;
    LimitationOnIndirectTravel lit(trx, itin);
    lit.displayFarePath(true);
    trx.diagnostic().diagnosticType() = Diagnostic372;
    lit.setDiag372InRequest();
    lit.prepareDiag372OutPut(*_farePath, crs, diagStr);
   CPPUNIT_ASSERT(trx.diagnostic().toString().find("* TPM SURFACE RESTRICTION EXEMPTION CHECK *") != string::npos);
  }

  void testPrepareDiag372OutPut_Include_FP()
  {
    Agent agent;
    agent.vendorCrsCode() = "1S";

    PricingRequest request;
    request.ticketingAgent() = &agent;

    trx.setRequest(&request);
    Diag372Collector::DiagStream diagStr;
    std::string crs = "1S";
    Itin itin;
    trx.itin().push_back(&itin);
    LimitationOnIndirectTravel lit(trx, itin);
    trx.diagnostic().diagnosticType() = Diagnostic372;
    lit.setDiag372InRequest();
    lit.prepareDiag372OutPut(*_farePath, crs, diagStr);

    CPPUNIT_ASSERT(trx.diagnostic().toString().find("TPM SURFACE RESTRICTION EXEMPTION TABLE") != string::npos);
  }


  PricingTrx trx;
  PricingRequest request;
  FarePath* _farePath;

  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(LimitationOnIndirectTravelTest);
}
