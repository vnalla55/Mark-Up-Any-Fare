//-------------------------------------------------------------------
//
//  File:        TestCommon.h
//  Created:     January 7, 2009
//  Authors:     Miroslaw Bartyna
//
//  Updates:
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
#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include "Rules/Tab988Merger.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DataModel/FarePath.h"
#include "DBAccess/ReissueSequence.h"

namespace tse
{

class TestCommon
{
public:
  static const int SEG_1 = 0x01;
  static const int SEG_2 = 0x02;
  static const int SEG_3 = 0x04;
  static const int SEG_4 = 0x08;
  static const int SEG_5 = 0x10;
  static const int SEG_6 = 0x20;
  static const int FARE_COMPONENT = 0x100;
  static const int SUB_JOURNEY = 0x200;
  static const int JOURNEY = 0x400;
  static const int TSI = 0x800;
  static const int CHECK_ORIG = 0x1000;
  static const int CHECK_DEST = 0x2000;
  static const int CHECK_ORIG_DEST = 0x4000;

  template <class V, class T>
  static void addToVector(V& v, T* p1)
  {
    v.push_back(p1);
  }

  template <class V, class T>
  static void addToVector(V& v, T* p1, T* p2)
  {
    v.push_back(p1);
    v.push_back(p2);
  }

  template <class V, class T>
  static void addToVector(V& v, T* p1, T* p2, T* p3)
  {
    v.push_back(p1);
    v.push_back(p2);
    v.push_back(p3);
  }

  template <class V>
  static void freeVect(V& v)
  {
    for (typename V::iterator iter = v.begin(); iter != v.end(); ++iter)
      delete *iter;
    v.clear();
  }

  static void freeTvlVect(std::vector<TravelSeg*>* v)
  {
    for (std::vector<TravelSeg*>::iterator iter = v->begin(); iter != v->end(); ++iter)
      delete (dynamic_cast<AirSeg*>(*iter));
    v->clear();
  }

  static FareMarket* FM(TravelSeg* p1, TravelSeg* p2 = NULL)
  {
    FareMarket* fm = new FareMarket();
    p2 ? addToVector(fm->travelSeg(), p1, p2) : addToVector(fm->travelSeg(), p1);

    return fm;
  }

  static FareUsage* FU(TravelSeg* p1, TravelSeg* p2 = NULL, TravelSeg* p3 = NULL)
  {
    FareUsage* fu = new FareUsage();
    if (p3)
    {
      addToVector(fu->travelSeg(), p1, p2, p3);
    }
    else
    {
      p2 ? addToVector(fu->travelSeg(), p1, p2) : addToVector(fu->travelSeg(), p1);
    }

    return fu;
  }

  static PricingUnit* PU(FareUsage* p1, FareUsage* p2 = NULL)
  {
    PricingUnit* pu = new PricingUnit();
    p2 ? addToVector(pu->fareUsage(), p1, p2) : addToVector(pu->fareUsage(), p1);

    return pu;
  }

  static FarePath* FP(PricingUnit* p1, PricingUnit* p2 = NULL)
  {
    FarePath* fp = new FarePath();
    p2 ? addToVector(fp->pricingUnit(), p1, p2) : addToVector(fp->pricingUnit(), p1);

    return fp;
  }

  static void addSegment(std::vector<TravelSeg*>* tvlSeg,
                         const LocCode& board,
                         const LocCode& off,
                         bool stopOver = false,
                         int flightNo = 123,
                         const CarrierCode& cxr = "AA",
                         const LocCode& boardMCity = "",
                         const LocCode& offMCity = "",
                         const TravelSeg::ChangeStatus changeStatus = TravelSeg::CHANGED,
                         bool unflown = true,
                         const std::string& depDateTime = "2007-07-07 07:07",
                         const TravelSegType segType = Air)

  {
    AirSeg* seg = new AirSeg;

    seg->origAirport() = board;
    seg->destAirport() = off;
    seg->segmentType() = segType;
    seg->stopOver() = stopOver;
    seg->carrier() = cxr;
    if (segType == Air)
      seg->flightNumber() = flightNo;
    seg->unflown() = unflown;
    if (!depDateTime.empty())
      seg->departureDT() = DateTime(const_cast<std::string&>(depDateTime));

    if (boardMCity == "")
    {
      seg->boardMultiCity() = board;
      seg->offMultiCity() = off;
    }
    else
    {
      seg->boardMultiCity() = boardMCity;
      seg->offMultiCity() = offMCity;
    }
    seg->changeStatus() = changeStatus;
    seg->pnrSegment() = tvlSeg->size() + 1;
    tvlSeg->push_back(seg);
  }

  static ReissueSequence*
  SEQ(int r3ItemNo, int stsFrom, int stsTo, Indicator portionInd = ReissueTable::NOT_APPLY)
  {
    ReissueSequence* r3 = new ReissueSequence;
    r3->itemNo() = r3ItemNo;
    r3->seqNo() = 1234;
    r3->portionInd() = portionInd;
    r3->tvlGeoTblItemNoFrom() = stsFrom;
    r3->tvlGeoTblItemNoTo() = stsTo;
    return r3;
  }
};

class ReissueTableWithoutDB
{
protected:
  virtual bool validateGeoRuleItem(const int& geoTblItemNo,
                                   const VendorCode& vendor,
                                   const RuleConst::TSIScopeParamType defaultScope,
                                   const FarePath* fp,
                                   const PricingUnit* pu,
                                   const FareMarket* fm,
                                   RuleUtil::TravelSegWrapperVector& applTravelSegment,
                                   const bool checkOrig,
                                   const bool checkDest) = 0;
  virtual bool getGeoRuleItem(const uint32_t geoTblItemNo,
                              const VendorCode& vendor,
                              RexPricingTrx& trx,
                              bool& checkOrig,
                              bool& checkDest,
                              TSICode& tsi,
                              LocKey& locKey1,
                              LocKey& locKey2) = 0;
  virtual const TSIInfo* getTSI(const TSICode& tsi) = 0;
  virtual const std::vector<CarrierApplicationInfo*>&
  getCarrierApplication(const VendorCode& vendor, int itemNo) = 0;
  virtual void diagGeoTblItem(const uint32_t geoTblItemNo,
                              const tse::VendorCode& vendor,
                              RexPricingTrx& trx) = 0;
  virtual bool getUtcOffsetDifference(const Loc& loc1, const Loc& loc2, short& utcoffset) = 0;
  virtual bool getUtcOffsetDifference(const Loc& loc1,
                                      const Loc& loc2,
                                      short& utcoffset,
                                      const DateTime& dt1,
                                      const DateTime& dt2) = 0;

public:
  virtual ~ReissueTableWithoutDB() {}
};

class ReissueTableOverride : public Tab988Merger, public ReissueTableWithoutDB
{
  friend class VoluntaryChangesTest;
  friend class ReissueTableTest;

public:
  typedef std::vector<std::pair<unsigned char, std::vector<CarrierCode> > > Tabl990Vect;
  ReissueTableOverride(RexPricingTrx& trx, const Itin* itin, const PricingUnit* pu)
    : Tab988Merger(trx, itin, pu), _itin(itin), _returnNotEmptyRSVec(true)
  {
  }

protected:
  bool getUtcOffsetDifference(const Loc& loc1, const Loc& loc2, short& utcoffset) { return true; }

  virtual bool getUtcOffsetDifference(
      const Loc& loc1, const Loc& loc2, short& utcoffset, const DateTime& dt1, const DateTime& dt2)
  {
    return true;
  }

  void createApplTravelSegment(const int& itemNo,
                               RuleUtil::TravelSegWrapperVector& applTravelSegment,
                               const bool checkOrig,
                               const bool checkDest,
                               const std::vector<TravelSeg*>& segs)
  {
    int segBytes = (itemNo & 0xFF);
    int segIndex = 0;
    while (segBytes && (segBytes & 0x1) == 0)
    {
      segBytes = segBytes >> 1;
      ++segIndex;
    }
    while (segBytes)
    {
      if (std::find(segs.begin(), segs.end(), _itin->travelSeg()[segIndex]) != segs.end())
      {
        RuleUtil::TravelSegWrapper* tsw = new RuleUtil::TravelSegWrapper;

        tsw->travelSeg() = _itin->travelSeg()[segIndex];
        tsw->origMatch() = checkOrig;
        tsw->destMatch() = checkDest;
        applTravelSegment.push_back(tsw);
      }

      segBytes = segBytes >> 1;
      ++segIndex;
      while (segBytes && (segBytes & 0x1) == 0)
      {
        segBytes = segBytes >> 1;
        ++segIndex;
      }
    }
  }

  virtual bool validateGeoRuleItem(const int& itemNo,
                                   const VendorCode& vendor,
                                   const RuleConst::TSIScopeParamType defaultScope,
                                   const FarePath* fp,
                                   const PricingUnit* pu,
                                   const FareMarket* fm,
                                   RuleUtil::TravelSegWrapperVector& applTravelSegment,
                                   const bool checkOrig,
                                   const bool checkDest)
  {
    if (itemNo & 0xFF)
    {
      std::vector<TravelSeg*> segs;
      std::vector<FareUsage*>::const_iterator fu;
      std::vector<FareUsage*>::const_iterator fue;

      switch (defaultScope)
      {
      case RuleConst::TSI_SCOPE_JOURNEY:
      {
        std::vector<PricingUnit*>::const_iterator pui = fp->pricingUnit().begin();
        std::vector<PricingUnit*>::const_iterator puie = fp->pricingUnit().end();
        for (; pui != puie; ++pui)
        {
          fu = (*pui)->fareUsage().begin();
          fue = (*pui)->fareUsage().end();
          for (; fu != fue; ++fu)
            copy((*fu)->travelSeg().begin(), (*fu)->travelSeg().end(), back_inserter(segs));
        }
      }
      break;
      case RuleConst::TSI_SCOPE_SUB_JOURNEY:
      {
        fu = pu->fareUsage().begin();
        fue = pu->fareUsage().end();
        for (; fu != fue; ++fu)
          copy((*fu)->travelSeg().begin(), (*fu)->travelSeg().end(), back_inserter(segs));
      }
      break;
      case RuleConst::TSI_SCOPE_FARE_COMPONENT:
      {
        copy(fm->travelSeg().begin(), fm->travelSeg().end(), back_inserter(segs));
      }
      break;
      default:
        return false;
      }
      if (segs.empty())
        return false;
      createApplTravelSegment(itemNo, applTravelSegment, checkOrig, checkDest, segs);

      return applTravelSegment.size();
    }

    return false;
  }
  bool getGeoRuleItem(const uint32_t geoTblItemNo,
                      const VendorCode& vendor,
                      RexPricingTrx& trx,
                      bool& checkOrig,
                      bool& checkDest,
                      TSICode& tsi,
                      LocKey& locKey1,
                      LocKey& locKey2)
  {
    checkOrig =
        (geoTblItemNo & TestCommon::CHECK_ORIG) || (geoTblItemNo & TestCommon::CHECK_ORIG_DEST);
    checkDest =
        (geoTblItemNo & TestCommon::CHECK_DEST) || (geoTblItemNo & TestCommon::CHECK_ORIG_DEST);
    switch (geoTblItemNo &
            (TestCommon::FARE_COMPONENT | TestCommon::SUB_JOURNEY | TestCommon::JOURNEY))
    {
    case TestCommon::JOURNEY:
      tsi = RuleConst::TSI_SCOPE_JOURNEY;
      break;
    case TestCommon::SUB_JOURNEY:
      tsi = RuleConst::TSI_SCOPE_SUB_JOURNEY;
      break;
    case TestCommon::FARE_COMPONENT:
      tsi = RuleConst::TSI_SCOPE_FARE_COMPONENT;
      break;
    default:
      tsi = 0;
      break;
    }
    tsi |= (geoTblItemNo & TestCommon::TSI);

    return true;
  }
  const TSIInfo* getTSI(const TSICode& tsi)
  {
    TSIInfo* tsiInfo = new TSIInfo();
    tsiInfo->scope() = (tsi & 0xFF);
    TSICode& ntsi = const_cast<TSICode&>(tsi);
    ntsi = tsi & TestCommon::TSI;

    return tsiInfo;
  }

  virtual const std::vector<CarrierApplicationInfo*>&
  getCarrierApplication(const VendorCode& vendor, int itemNo)
  {
    std::vector<CarrierApplicationInfo*>* CAIvecp = new std::vector<CarrierApplicationInfo*>;

    const RexShoppingTrx* rsTrx = dynamic_cast<RexShoppingTrx*>(&_trx);
    if (rsTrx)
    {
      std::vector<CarrierCode>::const_iterator cxrIter = _tabl990[itemNo - 1].second.begin();
      for (; cxrIter != _tabl990[itemNo - 1].second.end(); ++cxrIter)
      {
        CarrierApplicationInfo* CAIp = new CarrierApplicationInfo();
        CAIp->applInd() = _tabl990[itemNo - 1].first;
        CAIp->carrier() = *cxrIter;
        CAIvecp->push_back(CAIp);
      }
      if (_tabl990[itemNo - 1].first == 'X')
      {
        CarrierApplicationInfo* CAIp = new CarrierApplicationInfo();
        CAIp->applInd() = ' ';
        CAIp->carrier() = "$$";
        CAIvecp->push_back(CAIp);
      }
    }
    else
    {
      CarrierApplicationInfo* CAIp = new CarrierApplicationInfo();
      CAIp->applInd() = 'X';
      CAIp->carrier() = "LH";

      CAIvecp->push_back(CAIp);
    }

    return *CAIvecp;
  }

  virtual void
  diagGeoTblItem(const uint32_t geoTblItemNo, const tse::VendorCode& vendor, RexPricingTrx& trx)
  {
  }
  virtual const CarrierCode& findPublishingCarrier(const FareMarket& fareMarket)
  {
    return fareMarket.governingCarrier();
  }

  virtual const std::vector<ReissueSequence*>&
  getReissue(const VendorCode& vendor, int itemNo, const DateTime& applDate)
  {
    _rsv = new std::vector<ReissueSequence*>;

    if (_returnNotEmptyRSVec)
    {
      ReissueSequence* rsp;
      _rsv->push_back(rsp);
    }
    return *_rsv;
  }

  virtual const std::vector<ReissueSequence*>&
  getMatchedT988Seqs(const FareMarket& fareMarket,
                     const VoluntaryChangesInfo& vcRec3,
                     bool overridenApplied,
                     FareCompInfo::SkippedValidationsSet* skippedValidations,
                     const DateTime& applDate,
                     const std::set<int>* prevalidatedSeqTab988 = 0)
  {
    _rsv = new std::vector<ReissueSequence*>;

    if (_returnNotEmptyRSVec)
    {
      ReissueSequence* rsp;
      _rsv->push_back(rsp);
    }
    return *_rsv;
  }

public:
  Tabl990Vect& tabl990() { return _tabl990; }

private:
  const Itin* _itin;

public:
  std::vector<ReissueSequence*>* _rsv;
  bool _returnNotEmptyRSVec;
  Tabl990Vect _tabl990;
};
}

#endif // TEST_COMMON_H
