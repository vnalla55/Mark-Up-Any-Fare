//-------------------------------------------------------------------
//
//  File:        S8BrandedFaresSelectorTest.cpp
//  Created:     2014
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#ifndef S8BRANDEDFARESSELECTORINITIALIZER_H_
#define S8BRANDEDFARESSELECTORINITIALIZER_H_


#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/MarketCriteria.h"
#include "BrandedFares/MarketResponse.h"
#include "BrandedFares/S8BrandedFaresSelector.h"
#include "BrandedFares/BrandedFaresSelector.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/FareClassAppSegInfo.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/SvcFeesFareIdInfo.h"
#include "Diagnostic/Diag889Collector.h"
#include "test/include/MockDataManager.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"



using namespace std;
namespace tse
{

template <class T>
class S8BrandedFaresSelectorInitializer
{
public:
  S8BrandedFaresSelectorInitializer(){}
  virtual ~S8BrandedFaresSelectorInitializer(){_memHandle.clear();}

protected:

  class SpecificTestConfigInitializer : public TestConfigInitializer
   {
   public:
     SpecificTestConfigInitializer()
     {
       DiskCache::initialize(_config);
       _memHandle.create<MockDataManager>();
     }

     ~SpecificTestConfigInitializer() { _memHandle.clear(); }

   private:
     TestMemHandle _memHandle;
   };


  void init();
  const Loc* getLoc(const LocCode& locCode);
  void buildMarketResponse();
  void buildSvcFeesFareIdInfo();
  void buildSecondarySvcFeesFareIdInfo();
  FareMarket* createFareMarket(const Loc* origin, const Loc* destination, CarrierCode goveringCarrier);
  Fare* createFare(FareMarket* fm1, Fare::FareState state, GlobalDirection gd, Indicator owrt,
                   CurrencyCode currency, FareClassCode& fareClass);
  PaxTypeFare* createPaxTypeFare(Fare* f1, FareMarket& fm1, PaxTypeCode paxTypeCode,
                                 VendorCode vendorCode, Indicator adultInd);



  PricingTrx* _trx;
  FareDisplayTrx* _fdTrx;
  std::vector<PaxTypeFare*> _paxTypeFare;
  MarketResponse* _mResponse;
  MarketResponse* _mResponse2;
  MarketResponse* _mResponse3;
  MarketCriteria* _mCriteria;
  MarketCriteria* _mCriteria2;
  MarketCriteria* _mCriteria3;
  BrandProgram* _bProgram1;
  BrandProgram* _bProgram2;
  BrandInfo* _brand1;
  BrandInfo* _brand2;
  BrandInfo* _brand3;
  PaxTypeFare* _ptf1;
  FareClassAppInfo* _fareClassAppInfo;
  FareClassAppSegInfo* _fareClassAppSegInfo;
  std::vector<MarketResponse*>* _mR;
  std::vector<MarketResponse*>* _mR2;
  std::vector<SvcFeesFareIdInfo*>* _svcFeesFareIdInfoVector;
  std::vector<SvcFeesFareIdInfo*>* _secSvcFeesFareIdInfoVector;
  SvcFeesFareIdInfo* _svcFeesFareIdInfo;
  SvcFeesFareIdInfo* _secSvcFeesFareIdInfo;
  T* _s8BrandedFaresSelector;
  T* _fdS8BrandedFaresSelector;
  TestMemHandle _memHandle;
  FareMarket* _fm1;
  FareInfo* _fi1;
  FBRPaxTypeFareRuleData* _fbrPaxTypeFareRuleData;
  FareByRuleItemInfo* _fbrItemInfo;
  FareByRuleApp* _fbrApp;
  PaxTypeFare::PaxTypeFareAllRuleData* _ptfARD;
  PaxTypeFareRuleData* _ptfRD;
  PricingRequest* _request;
  const tse::Loc* sfo;
  const tse::Loc* dfw;
  AirSeg* _seg1;
  std::vector<SecSvcFeesFareIdInfo*>* _secSecSvcFeesFareIdInfoVector;
};



template <class T>
void S8BrandedFaresSelectorInitializer<T>::init()
{
 _memHandle.create<SpecificTestConfigInitializer>();
 _trx = _memHandle.create<PricingTrx>();
 _fdTrx = _memHandle.create<FareDisplayTrx>();
 _fdTrx->setTrxType(PricingTrx::FAREDISPLAY_TRX);
 sfo = getLoc("SFO");
 dfw = getLoc("DFW");
 _mResponse = _memHandle.create<MarketResponse>();
 _mResponse2 = _memHandle.create<MarketResponse>();
 _mResponse3 = _memHandle.create<MarketResponse>();
 _mCriteria = _memHandle.create<MarketCriteria>();
 _mCriteria2 = _memHandle.create<MarketCriteria>();
 _mCriteria3 = _memHandle.create<MarketCriteria>();
 _bProgram1 = _memHandle.create<BrandProgram>();
 _bProgram2 = _memHandle.create<BrandProgram>();
 _svcFeesFareIdInfoVector = _memHandle.create<vector<SvcFeesFareIdInfo*> >();
 _secSvcFeesFareIdInfoVector = _memHandle.create<vector<SvcFeesFareIdInfo*> >();
 _mR = _memHandle.create<vector<MarketResponse*> >();
 _mR2 = _memHandle.create<vector<MarketResponse*> >();
 buildSvcFeesFareIdInfo();
 buildSecondarySvcFeesFareIdInfo();
 buildMarketResponse();
 _mR->push_back(_mResponse);
 _mR->push_back(_mResponse2);
 _mR2->push_back(_mResponse3);
 _trx->brandedMarketMap().insert(make_pair(1, *_mR));
 _trx->brandedMarketMap().insert(make_pair(2, *_mR2));
 _trx->brandedMarketMap().insert(make_pair(3, *_mR2));
 _fdTrx->brandedMarketMap().insert(make_pair(1, *_mR));

 _fm1 = createFareMarket(sfo, dfw, "AA");
 _fm1->addOriginRequestedForOriginalDirection(_mCriteria->departureAirportCode());
 _fm1->addOriginRequestedForOriginalDirection(_mCriteria2->departureAirportCode());
 _fm1->addOriginRequestedForOriginalDirection(_mCriteria3->departureAirportCode());
 _fm1->addOriginRequestedForOriginalDirection(_mCriteria->arrivalAirportCode());
 _fm1->addOriginRequestedForOriginalDirection(_mCriteria2->arrivalAirportCode());
 _fm1->addOriginRequestedForOriginalDirection(_mCriteria3->arrivalAirportCode());
 _fm1->addDestinationRequestedForReversedDirection(_mCriteria->departureAirportCode());
 _fm1->addDestinationRequestedForReversedDirection(_mCriteria2->departureAirportCode());
 _fm1->addDestinationRequestedForReversedDirection(_mCriteria3->departureAirportCode());
 _fm1->addDestinationRequestedForReversedDirection(_mCriteria->arrivalAirportCode());
 _fm1->addDestinationRequestedForReversedDirection(_mCriteria2->arrivalAirportCode());
 _fm1->addDestinationRequestedForReversedDirection(_mCriteria3->arrivalAirportCode());

 FareClassCode fareClass = "GOGO";
 Fare* f1 = createFare(
     _fm1, Fare::FS_Domestic, GlobalDirection::US, ONE_WAY_MAY_BE_DOUBLED, "", fareClass);
 f1->nucFareAmount() = 2582.45;
 _ptf1 = createPaxTypeFare(f1, *_fm1, PaxTypeCode("ADT"), VendorCode("ATP"), 'Y');
 FareClassAppInfo* appInfo1 = _memHandle.create<FareClassAppInfo>();
 appInfo1->_fareType = "BUR";
 _ptf1->fareClassAppInfo() = appInfo1;
 _ptfRD = _memHandle.create<PaxTypeFareRuleData>();
 _ptfRD->baseFare() = _ptf1;
 _ptfARD = _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();
 _ptfARD->fareRuleData = _ptfRD;

 _request = _memHandle.create<PricingRequest>();
 _trx->setRequest(_request);
 _trx->setPbbRequest(PBB_RQ_PROCESS_BRANDS);

 _fbrApp = _memHandle.create<FareByRuleApp>();
 _fbrApp->vendor() = "ATP";
 _fbrApp->carrier() = "AA";
 _fbrApp->segCnt() = 1;
 _fbrApp->primePaxType() = "ADT";
 _fbrItemInfo = _memHandle.create<FareByRuleItemInfo>();
 _fbrItemInfo->percent() = 100;
 _fbrItemInfo->specifiedFareAmt1() = 23.00;
 _fbrPaxTypeFareRuleData = _memHandle.create<FBRPaxTypeFareRuleData>();
 _fbrPaxTypeFareRuleData->ruleItemInfo() = _fbrItemInfo;
 _fbrPaxTypeFareRuleData->fbrApp() = _fbrApp;
 std::vector<BookingCode> bCodeVec;
 bCodeVec.push_back("AB");
 bCodeVec.push_back("RK");
 bCodeVec.push_back("YN");
 bCodeVec.push_back("CJ");
 _fbrPaxTypeFareRuleData->setBaseFarePrimeBookingCode(bCodeVec);
 PaxTypeFare basePaxTypeFare;
 _fbrPaxTypeFareRuleData->baseFare() = &basePaxTypeFare;

 _fareClassAppInfo = _memHandle.create<FareClassAppInfo>();
 _fareClassAppSegInfo = _memHandle.create<FareClassAppSegInfo>();

 _paxTypeFare.push_back(_ptf1);
 _fm1->allPaxTypeFare().push_back(_ptf1);
 _trx->fareMarket().push_back(_fm1);
 _trx->brandProgramVec().push_back(std::make_pair(_bProgram1, _brand1));
 _trx->brandProgramVec().push_back(std::make_pair(_bProgram1, _brand2));
 _trx->brandProgramVec().push_back(std::make_pair(_bProgram1, _brand3));
 _trx->brandProgramVec().push_back(std::make_pair(_bProgram2, _brand1));
 _trx->brandProgramVec().push_back(std::make_pair(_bProgram2, _brand2));
 _trx->brandProgramVec().push_back(std::make_pair(_bProgram2, _brand3));
 _fm1->brandProgramIndexVec().push_back(0);
 _fm1->brandProgramIndexVec().push_back(1);
 _fm1->brandProgramIndexVec().push_back(2);
 _fm1->brandProgramIndexVec().push_back(3);
 _fm1->brandProgramIndexVec().push_back(4);
 _fm1->brandProgramIndexVec().push_back(5);
 _fm1->marketIDVec().push_back(1);
 _fm1->marketIDVec().push_back(2);
 _fm1->marketIDVec().push_back(3);
 _fdTrx->brandProgramVec().push_back(std::make_pair(_bProgram1, _brand1));
 _secSecSvcFeesFareIdInfoVector = _memHandle.create<vector<SecSvcFeesFareIdInfo*> >();
}

template <class T>
const Loc* S8BrandedFaresSelectorInitializer<T>::getLoc(const LocCode& locCode)
{
 return TestLocFactory::create("/vobs/atseintl/test/testdata/data/Loc" + locCode + ".xml");
}

template <class T>
void S8BrandedFaresSelectorInitializer<T>::buildMarketResponse()
{

 _brand1 = _memHandle.create<BrandInfo>();
 _brand2 = _memHandle.create<BrandInfo>();
 _brand3 = _memHandle.create<BrandInfo>();

 _mResponse->carrier() = "AA";
 _mResponse->brandPrograms().push_back(_bProgram1);
 _mResponse->brandPrograms().push_back(_bProgram2);
 _bProgram1->brandsData().push_back(_brand1);
 _bProgram1->brandsData().push_back(_brand2);
 _bProgram2->brandsData().push_back(_brand3);
 // Program1
 _bProgram1->programCode() = "us";
 _bProgram1->programName() = "domestic us";
 _bProgram1->passengerType().push_back("ADT");
 _bProgram1->programID() = "areaone";
 // Brand1
 _brand1->brandCode() = "app";
 _brand1->brandName() = "apple";
 _brand1->tier() = 99;
 _brand1->fareIDdataPrimaryT189() = *_svcFeesFareIdInfoVector;

 // Brand2
 _brand2->brandCode() = "abb";
 _brand2->brandName() = "abbreviate";
 _brand2->tier() = 10;
 _brand2->fareIDdataPrimaryT189() = *_svcFeesFareIdInfoVector;

 // Program2 - Brand3
 _bProgram2->programCode() = "afa";
 _bProgram2->programName() = "flight anywhere";
 _bProgram2->passengerType().push_back("ADT");
 _bProgram2->programID() = "program2";
 _brand3->brandCode() = "BC";
 _brand3->brandName() = "forever";
 _brand3->tier() = 55;
 _brand3->fareIDdataPrimaryT189() = *_svcFeesFareIdInfoVector;

 _mResponse->programIDList().push_back(_bProgram1->programID());
 _mResponse->programIDList().push_back(_bProgram2->programID());
 _mResponse2->programIDList().push_back(_bProgram1->programID());
 _mResponse2->programIDList().push_back(_bProgram2->programID());
 _mResponse3->programIDList().push_back(_bProgram1->programID());
 _mResponse3->programIDList().push_back(_bProgram2->programID());
 _mResponse->setMarketID() = 1;
 _mResponse2->setMarketID() = 2;
 _mResponse3->setMarketID() = 3;

 _mCriteria->departureAirportCode() = "SYD";
 _mCriteria->arrivalAirportCode() = "MEL";

 _mResponse->marketCriteria() = _mCriteria;
 _bProgram1->originsRequested().insert(_mCriteria->departureAirportCode());
 _bProgram1->destinationsRequested().insert(_mCriteria->arrivalAirportCode());
 _bProgram2->originsRequested().insert(_mCriteria->departureAirportCode());
 _bProgram2->destinationsRequested().insert(_mCriteria->arrivalAirportCode());

 _mCriteria2->departureAirportCode() = "MEL";
 _mCriteria2->arrivalAirportCode() = "AKL";

 _mResponse2->marketCriteria() = _mCriteria2;
 _bProgram1->originsRequested().insert(_mCriteria2->departureAirportCode());
 _bProgram1->destinationsRequested().insert(_mCriteria2->arrivalAirportCode());
 _bProgram2->originsRequested().insert(_mCriteria2->departureAirportCode());
 _bProgram2->destinationsRequested().insert(_mCriteria2->arrivalAirportCode());

 _mCriteria3->departureAirportCode() = "ZQN";
 _mCriteria3->arrivalAirportCode() = "ROT";

 _mResponse3->marketCriteria() = _mCriteria3;
 _bProgram1->originsRequested().insert(_mCriteria3->departureAirportCode());
 _bProgram1->destinationsRequested().insert(_mCriteria3->arrivalAirportCode());
 _bProgram2->originsRequested().insert(_mCriteria3->departureAirportCode());
 _bProgram2->destinationsRequested().insert(_mCriteria3->arrivalAirportCode());
}


template <class T>
void S8BrandedFaresSelectorInitializer<T>::buildSvcFeesFareIdInfo()
{
 _svcFeesFareIdInfo = _memHandle.create<SvcFeesFareIdInfo>();
 _svcFeesFareIdInfo->vendor() = Vendor::ATPCO;
 _svcFeesFareIdInfo->itemNo() = 8888;
 _svcFeesFareIdInfo->seqNo() = 199;
 _svcFeesFareIdInfo->fareApplInd() = ' ';
 _svcFeesFareIdInfo->owrt() = '1';
 _svcFeesFareIdInfo->ruleTariff() = -1;
 _svcFeesFareIdInfo->ruleTariffInd() = "PUB";
 //_svcFeesFareIdInfo->rule() = "345";
 _svcFeesFareIdInfo->rule() = "";
 _svcFeesFareIdInfo->fareClass() = "GOGO";
 _svcFeesFareIdInfo->fareType() = "BUR";
 //_svcFeesFareIdInfo->paxType() = "ABC";
 _svcFeesFareIdInfo->paxType() = "";
 _svcFeesFareIdInfo->routing() = 99999;
 //_svcFeesFareIdInfo->routing () = 1991;
 _svcFeesFareIdInfo->bookingCode1() = "";
 //_svcFeesFareIdInfo->bookingCode1() = "FN";
 _svcFeesFareIdInfo->bookingCode2() = "A";
 _svcFeesFareIdInfo->source() = 'A';
 _svcFeesFareIdInfo->minFareAmt1() = 50;
 _svcFeesFareIdInfo->maxFareAmt1() = 255;
 _svcFeesFareIdInfo->cur1() = "";
 //_svcFeesFareIdInfo->cur1() = "USD";
 _svcFeesFareIdInfo->noDec1() = 2;
 _svcFeesFareIdInfo->minFareAmt2() = 20;
 _svcFeesFareIdInfo->maxFareAmt2() = 777;
 _svcFeesFareIdInfo->cur2() = "AUD";
 _svcFeesFareIdInfo->noDec2() = 3;
 _svcFeesFareIdInfoVector->push_back(_svcFeesFareIdInfo);
}


template <class T>
void S8BrandedFaresSelectorInitializer<T>::buildSecondarySvcFeesFareIdInfo()
{
 _secSvcFeesFareIdInfo = _memHandle.create<SvcFeesFareIdInfo>();
 _secSvcFeesFareIdInfo->vendor() = Vendor::ATPCO;
 _secSvcFeesFareIdInfo->itemNo() = 8888;
 _secSvcFeesFareIdInfo->seqNo() = 199;
 _secSvcFeesFareIdInfo->fareApplInd() = ' ';
 _secSvcFeesFareIdInfo->owrt() = '1';
 _secSvcFeesFareIdInfo->ruleTariff() = -1;
 _secSvcFeesFareIdInfo->ruleTariffInd() = "PUB";
 //_secSvcFeesFareIdInfo->rule() = "345";
 _secSvcFeesFareIdInfo->rule() = "";
 _secSvcFeesFareIdInfo->fareClass() = "GOGO";
 _secSvcFeesFareIdInfo->fareType() = "BUR";
 //_secSvcFeesFareIdInfo->paxType() = "ABC";
 _secSvcFeesFareIdInfo->paxType() = "";
 _secSvcFeesFareIdInfo->routing() = 99999;
 //_secSvcFeesFareIdInfo->routing () = 1991;
 _secSvcFeesFareIdInfo->bookingCode1() = "";
 //_secSvcFeesFareIdInfo->bookingCode1() = "FN";
 _secSvcFeesFareIdInfo->bookingCode2() = "A";
 _secSvcFeesFareIdInfo->source() = 'A';
 _secSvcFeesFareIdInfo->minFareAmt1() = 50;
 _secSvcFeesFareIdInfo->maxFareAmt1() = 255;
 _secSvcFeesFareIdInfo->cur1() = "";
 //_secSvcFeesFareIdInfo->cur1() = "USD";
 _secSvcFeesFareIdInfo->noDec1() = 2;
 _secSvcFeesFareIdInfo->minFareAmt2() = 20;
 _secSvcFeesFareIdInfo->maxFareAmt2() = 777;
 _secSvcFeesFareIdInfo->cur2() = "AUD";
 _secSvcFeesFareIdInfo->noDec2() = 3;
 _secSvcFeesFareIdInfoVector->push_back(_secSvcFeesFareIdInfo);
}

template <class T>
FareMarket* S8BrandedFaresSelectorInitializer<T>::createFareMarket(const Loc* origin, const Loc* destination,
                                                                   CarrierCode goveringCarrier)
{
 FareMarket* fm1 = _memHandle.create<FareMarket>();
 fm1->origin() = origin;
 fm1->destination() = destination;
 fm1->governingCarrier() = goveringCarrier;
 _seg1 = _memHandle.create<AirSeg>();
 _seg1->origAirport() = fm1->origin()->loc();
 _seg1->destAirport() = fm1->destination()->loc();
 fm1->travelSeg().push_back(_seg1);
 return fm1;
}

template <class T>
Fare* S8BrandedFaresSelectorInitializer<T>::createFare(FareMarket* fm1,
                                                      Fare::FareState state,
                                                      GlobalDirection gd,
                                                      Indicator owrt,
                                                      CurrencyCode currency,
                                                      FareClassCode& fareClass)
{
 Fare* f1 = _memHandle.create<Fare>();
 _fi1 = _memHandle.create<FareInfo>();
 TariffCrossRefInfo* tcri1 = _memHandle.create<TariffCrossRefInfo>();

 _fi1->_globalDirection = gd;
 _fi1->_owrt = owrt;
 _fi1->_currency = currency;
 _fi1->_fareClass = fareClass;
 _fi1->_directionality = BOTH;

 f1->initialize(state, _fi1, *fm1, tcri1);
 return f1;
}

template <class T>
PaxTypeFare* S8BrandedFaresSelectorInitializer<T>::createPaxTypeFare(
   Fare* f1, FareMarket& fm1, PaxTypeCode paxTypeCode, VendorCode vendorCode, Indicator adultInd)
{
 PaxTypeFare* ptf1 = _memHandle.create<PaxTypeFare>();
 PaxType* pt1 = _memHandle.create<PaxType>();
 PaxTypeInfo* pti1 = _memHandle.create<PaxTypeInfo>();

 pt1->paxType() = paxTypeCode;
 pt1->vendorCode() = vendorCode;
 pti1->adultInd() = adultInd;
 pt1->paxTypeInfo() = pti1;
 ptf1->initialize(f1, pt1, &fm1);
 return ptf1;
}

} /* namespace tse */
#endif /* S8BRANDEDFARESSELECTORINITIALIZER_H_ */
