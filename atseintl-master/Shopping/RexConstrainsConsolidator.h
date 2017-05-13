//-------------------------------------------------------------------
//
//  File:        RexConstrainsConsolidator.h
//  Created:     April 17, 2009
//  Authors:     Miroslaw Bartyna
//
//  Description: Define OAD and consolidate cat31/R3/T988 constrains
//
//  Copyright Sabre 2009
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

#pragma once

#include "Common/DateTime.h"
#include "DataModel/RexShoppingTrx.h"

namespace tse
{
class DiagCollector;

class RexConstrainsConsolidator
{
public:
  RexConstrainsConsolidator(RexShoppingTrx& trx);
  virtual ~RexConstrainsConsolidator();

  void process();

protected:
  friend class RexConstrainsConsolidatorTest;

  struct ONDInf
  {
    ONDInf()
      : travelDate(DateTime::emptyDate()),
        unflown(false),
        firstONDInfoNewSegPos(-1),
        lastONDInfoNewSegPos(-1),
        splittedFM(false)
    {
    }

    ONDInf(LocCode origAirport, LocCode destAirport, const std::string& departureDT)
      : origAirport(origAirport),
        destAirport(destAirport),
        travelDate(DateTime(const_cast<std::string&>(departureDT))),
        unflown(true),
        firstONDInfoNewSegPos(-1),
        lastONDInfoNewSegPos(-1),
        splittedFM(false)
    {
    }

    bool operator==(ONDInf& ondInf)
    {
      return (origAirport == ondInf.origAirport && destAirport == ondInf.destAirport &&
              travelDate.get64BitRepDateOnly() == ondInf.travelDate.get64BitRepDateOnly());
    }
    LocCode origAirport;
    LocCode destAirport;
    DateTime travelDate;
    std::vector<int> unshoppedFlights;
    bool unflown;
    int firstONDInfoNewSegPos;
    int lastONDInfoNewSegPos;
    bool splittedFM;
  };
  typedef std::map<int, const FareMarket*> ExcSegPosToFMMap;
  typedef std::map<const ONDInf*, std::vector<int> > ONDInfoToExcSegPosSetMap;
  typedef std::map<const ONDInf*, std::vector<const FareMarket*> > ONDInfoToFM;
  typedef std::map<ONDInf*, std::vector<int> > ONDInfoToExcSegPosVectMap;
  typedef std::map<const FareMarket*, std::vector<int> > FMToExcSegPosVectMap;
  typedef std::map<int, LocCode> ForcedCnnxToExcSegPosMap;
  typedef std::vector<const FareMarket*> FMToSkipConsRest;
  typedef std::map<const ONDInf*, std::map<int, int> > ExcSegPosToNewSegPos;
  typedef std::map<const PricingTrx::OriginDestination*, boost::gregorian::date_duration>
  PlusMinusDateShift;
  typedef std::map<const RexShoppingTrx::OADConsolidatedConstrains*, const ONDInf*>
  ConstToONDInfoMap;

  void detachFlownToNewLeg();
  void addONDInfo(const std::vector<TravelSeg*>::iterator& frontSeg,
                  const std::vector<TravelSeg*>::iterator& backSeg,
                  const PricingTrx::OriginDestination& odThruFM,
                  bool splittedFM);
  void getSegForOND(const PricingTrx::OriginDestination& odThruFM,
                    std::vector<TravelSeg*>& newTravelSeg,
                    std::vector<TravelSeg*>::iterator& frontSeg,
                    std::vector<TravelSeg*>::iterator& backSeg);
  void markUnshoppedFlights(const std::vector<TravelSeg*>::iterator& frontSeg,
                            const std::vector<TravelSeg*>::iterator& backSeg,
                            ONDInf* ondInf);
  void matchONDInfotoExcOND();
  void createExcSegPosToFMMap(ExcSegPosToFMMap& excSegPosToFM);
  void matchONDInfoToExcSegPos();
  void matchFMsToONDInfo(ExcSegPosToFMMap& excSegPosToFM);
  void printONDInfoToFM(const ExcSegPosToFMMap& excSegPosToFM);
  void consolideConstrainsForONDInfo();
  void
  initOADConsRest(RexShoppingTrx::OADConsolidatedConstrains& oadConsRest, const ONDInf* ondInfo);
  void matchFMToExcSegPos();
  void printFMToExcSegPos();
  void consolidateRestFlightsWithBkg(RexShoppingTrx::OADConsolidatedConstrains& oadConsRest,
                                     const ONDInf* ondInfo);
  void addRestFlightsWithBkgToOADConsRest(RexShoppingTrx::SODConstraint& sodConstraint,
                                          const RexShoppingTrx::PortionMergeTvlVectType& portion,
                                          const int firstSegPos,
                                          const int lastSegPos,
                                          const ONDInf* ondInfo);
  void addUnshoppedFlightsToOADConsRest(RexShoppingTrx::OADConsolidatedConstrains& oadConsRest,
                                        const ONDInf* ondInfo);
  void consolidateRestFlights(RexShoppingTrx::OADConsolidatedConstrains& oadConsRest,
                              const ONDInf* ondInfo);
  void addRestFlightsToOADConsRest(RexShoppingTrx::SODConstraint& sodConstraint,
                                   const FareMarket* fareMarket,
                                   const int firstSegPos,
                                   const int lastSegPos,
                                   const ONDInf* ondInfo);
  void consolidateRestCxrs(RexShoppingTrx::OADConsolidatedConstrains& oadConsRest,
                           const ONDInf* ondInfo);
  void checkRestrictionNeeded(RexShoppingTrx::OADConsolidatedConstrains& oadConsRest,
                              const ONDInf* ondInfo);
  void addCarrierRestToOADConsRest(RexShoppingTrx::SODConstraint& sodConstraint,
                                   const FareMarket* fareMarket,
                                   const RexShoppingTrx::OADResponseData& oadResponseData,
                                   const ONDInf* ondInfo);
  bool isPrefGovCxrForcedByFirstBreakRest(const FareMarket* fareMarket,
                                          const RexShoppingTrx::OADResponseData& oadResponseData);
  void consolidateCalendarRanges(RexShoppingTrx::OADConsolidatedConstrains& oadConsRest,
                                 const ONDInf* ondInfo);
  void printFMToFirstFMinPU();
  void printConsolidatedConstrains();
  void createFMToSkipConsRest();
  void printFMToSkipConsRest();
  void createSODForONDInfo();
  void addSOD(const int firstSODnewSegPos,
              const int lastSODnewSegPos,
              RexShoppingTrx::OADConsolidatedConstrains& oadConsolConstr,
              const RexShoppingTrx::SODConstraint& sodConstraint,
              const int restricted,
              const bool addCxrRest,
              const std::size_t ondIndex);
  void printConsolidatedConstrainsWithSOD();
  void addUnshoppedFlightsToOADConsRest(RexShoppingTrx::OADConsolidatedConstrains& oadConsRest);
  void setExcSegPosToNewSegPos(const std::vector<TravelSeg*>::iterator& frontSeg,
                               const std::vector<TravelSeg*>::iterator& backSeg,
                               ONDInf* ondInf);
  void mergeCxrRest(RexShoppingTrx::FareByteCxrApplData& fareByteCxrApplDataRest,
                    const RexShoppingTrx::OADResponseData& oadResponseData,
                    const FareMarket* fareMarket);
  void printItin(const Itin& itin);
  void mergeOADCrxListWithPSSCrxList(RexShoppingTrx::SODConstraint& sodConstraint);
  void printPSSCxrList();
  void addGovCxr(RexShoppingTrx::FareByteCxrApplData& fareByteCxrApplDataRest,
                 const CarrierCode& governingCarrier);
  void calculatePlusMinusDateShift();
  void getNextSegForOND(const PricingTrx::OriginDestination& odThruFM,
                        std::vector<TravelSeg*>::iterator& lastSeg,
                        std::vector<TravelSeg*>& newTravelSeg,
                        std::vector<TravelSeg*>::iterator& frontSeg,
                        std::vector<TravelSeg*>::iterator& backSeg);
  int segmentOrder(const std::vector<TravelSeg*>& tvlSegVect, const TravelSeg* segment) const;
  bool isTravelDateTheSame(const TravelSeg* tvlSeg, const PricingTrx::OriginDestination& odThruFM);
  bool isMutliAirportCityTheSame(const LocCode& multiAirportCity,
                                 const LocCode& ondMultiAirportCity) const;
  void copySegWithoutARUNKs();
  void createFmWithFlownGovSet(std::vector<const FareMarket*>& fmWithFlownGov);
  bool getFlownExcONDTvlSegPos(const int firstONDNewTvlSegPos,
                               const int lastONDNewTvlSegPos,
                               int& firstONDExcTvlSegPos,
                               int& lastONDExcTvlSegPos);
  bool ondHasPrimarySectorInExcFlownPart(const FareMarket* fareMarket,
                                         const int firstONDExcTvlSegPos,
                                         const int lastONDExcTvlSegPos);
  const FareMarket* findFMforOND(const int firstONDExcTvlSegPos, const int lastONDExcTvlSegPos);
  const FareMarket* findFMforSegment(const TravelSeg* tvlSeg);
  bool isONDinExcFMWithFlownGov(std::vector<const FareMarket*>& fmWithFlownGov,
                                const RexShoppingTrx::OADConsolidatedConstrains* oadConsRest);
  void printFmWithFlownGovSet(std::vector<const FareMarket*>& fmWithFlownGov);
  void setGovCxr(RexShoppingTrx::FareByteCxrData*& sodCarriers,
                 const RexShoppingTrx::FareByteCxrData* ondCarriers);
  void setUsrCxr(RexShoppingTrx::FareByteCxrData*& sodCarriers,
                 const RexShoppingTrx::FareByteCxrData* ondCarriers);

  void calculateUnmappedFC(ExcSegPosToFMMap& excSegPosToFM);
  bool routingHasChanged();
  void matchFMsToONDInfoWithoutNoMatched(ExcSegPosToFMMap& excSegPosToFM);
  void createOndInfoNotMatched();
  void createFmNotMatched();
  bool doesAnyFlownFCHasSameCxrAsUnmappedFC();
  void printONDInfoNotMatched();
  void printFMNotMatched();
  void
  printOADConsRest(RexShoppingTrx::OADConsolidatedConstrains& oadConsolConstr, std::string reason);
  void removeDuplicates(std::map<const ONDInf*, std::vector<const FareMarket*> >& ondMap);

private:
  RexConstrainsConsolidator(const RexConstrainsConsolidator&);
  RexConstrainsConsolidator& operator=(const RexConstrainsConsolidator&);

  DiagCollector* _dc;
  RexShoppingTrx& _trx;
  std::vector<ONDInf*> _ondInfo;
  ONDInfoToFM _ondInfoToFM;
  ONDInfoToExcSegPosSetMap _ondInfoToExcSegPosSet;
  FMToExcSegPosVectMap _fmToExcSegPosSet;
  FMToSkipConsRest _fmToSkipConsRest;
  ExcSegPosToNewSegPos _excSegPosToNewSegPos;
  PlusMinusDateShift _plusMinusDateShift;
  std::vector<TravelSeg*> _excItinSeg;
  std::vector<TravelSeg*> _newItinSeg;
  ConstToONDInfoMap _constToONDInfo;
  ONDInf* _ondInfWithSplittedFMUnflown;
  ONDInf* _ondInfWithSplittedFMFlown;
  ONDInfoToExcSegPosSetMap _ondInfoToExcSegPosWithoutNoMatchedSet;
  std::set<const ONDInf*> _ondInfoNotMatched;
  std::vector<const FareMarket*> _fmNotMatched;
  bool _addCxrRestToUnmappedONDInfos;
  ONDInfoToFM _ondInfoToFMWithoutNoMatched;
};
}
