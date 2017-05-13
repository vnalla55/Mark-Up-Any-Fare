//-------------------------------------------------------------------
//
//  File:        RepriceSolutionValidator.h
//  Created:     September 10, 2007
//  Authors:     Grzegorz Cholewiak
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

#pragma once

#include "DataModel/TravelSeg.h"
#include "DBAccess/ReissueSequence.h"
#include "Diagnostic/Diag689Collector.h"
#include "RexPricing/GenericRexMapper.h"
#include "RexPricing/RepriceSolutionValidator.h"
#include "Rules/AdvanceResTkt.h"
#include "Rules/AdvResOverride.h"


namespace tse
{

struct OnlyChangedSegments : public std::vector<TravelSeg*>
{
  void operator()(TravelSeg* seg)
  {
    if ((seg->changeStatus() == TravelSeg::CHANGED ||
         seg->changeStatus() == TravelSeg::INVENTORYCHANGED))
      push_back(seg);
  }
};
struct CompareBookingDate
{
  bool operator()(TravelSeg* elem1, TravelSeg* elem2) const
  {
    return elem1->bookingDT() < elem2->bookingDT();
  }
};
struct CompareDepartureDate
{
  bool operator()(TravelSeg* elem1, TravelSeg* elem2) const
  {
    return elem1->departureDT() < elem2->departureDT();
  }
};

class RexPricingTrx;
class ProcessTagPermutation;
class Diag689Collector;
class FarePath;
class ProcessTagInfo;
class GenericRexMapper;

class RexAdvResTktValidator
{
  friend class RexAdvResTktValidatorTest;
  friend class RexAdvResTktValidatorAdvPurTest;

  // member functions
public:
  RexAdvResTktValidator(RexPricingTrx& trx,
                        Itin& newItin,
                        FarePath& newFarePath,
                        FarePath& excFarePath,
                        RepriceSolutionValidator::AdvResOverrideCache& _advResOverrideCache,
                        Diag689Collector*& dc,
                        const GenericRexMapper& grm);
  virtual ~RexAdvResTktValidator();
  void permutationIndependentSetUp();
  bool validate(const ProcessTagPermutation& permutation);

private:
  void initialize();
  bool validate(PricingUnit& pu);
  bool validate(FareUsage& fu, PricingUnit& pu);
  bool checkAfterResAndBeforeDept(const ReissueSequenceW& rs,
                                  AdvResOverride& advResOverride,
                                  FareUsage& fu,
                                  PricingUnit& pu,
                                  bool& isProcessed);
  bool isPUWhollyUnchanged(const PricingUnit& pu);
  virtual bool validateAdvRes(AdvResOverride& advResOverride, FareUsage& fu, PricingUnit& pu);
  const std::vector<const ProcessTagInfo*>&
  findFCsWithNoChangesToFBAndCarrier(const FareMarket* fm);
  bool areAllFCsWithNoChangesToFBAndCarrier();
  void setTicketResvIndTheSameOnJourney();
  void setTicketResvInd(const Indicator ticketResvInd, bool& ticketResvIndNeeded);
  bool isJourneyScope() const;
  bool checkSimultaneous();
  bool checkPriorOfDeparture();
  void diag305Header();
  void diag305CacheResult(const FareUsage& fu,
                          const PricingUnit& pu,
                          const AdvResOverride& advResOverride,
                          const bool result);
  void prepareCat31PCGOptionN(AdvResOverride& advResOverride, FareUsage& fu, PricingUnit& pu);
  bool isOutboundChanged(const PricingUnit& pu) const;
  bool isMostRestrictiveFromDate(const ProcessTagInfo& pti,
                                 const PricingUnit& pu,
                                 const FareUsage& fu,
                                 const FareUsage* newFu = nullptr) const;
  bool findMostRestrictiveFromDateForJourney() const;
  bool findMostRestrictiveFromDateForPU(const PricingUnit& pu);
  const DateTime& findMostRestrictiveToDateForJourney(const PricingUnit& pu, const FareUsage& fu);
  const DateTime& findMostRestrictiveToDateForPU(const PricingUnit& pu, const FareUsage& fu);
  void updateOldFM2OldFUMapping();

  bool ignoreCat5(const FareUsage& fu);
  bool isUnflown(const Indicator portion, const PricingUnit& pti, const FareUsage& fu) const;
  bool emptyBytes93to106(const ReissueSequenceW& rsw);

  // *** 2012 new advance purchase application ***

  template <typename Criterium>
  bool findFromMappedFCs(Criterium criterium);

  inline bool diagCat5Info() { return _dc && _dc->filterPassed() && _dc->printCat5Info(); }

  bool advPurchaseValidation();
  bool validateAdvPurchase(FareUsage& fu, PricingUnit& pu, std::vector<uint16_t>::size_type ptfIdx);
  virtual bool performRuleControllerValidation(FareUsage& fu, PricingUnit& pu);

  void determineFromDate(AdvResOverride& advResOverride,
                         const ProcessTagInfo& pti,
                         const PricingUnit& pu,
                         const FareUsage& fu) const;
  void determineToDate(AdvResOverride& advResOverride,
                       const ProcessTagInfo& pti,
                       const PricingUnit& pu,
                       const FareUsage& fu) const;
  void determineUnmappedOverrideData(AdvResOverride& advResOverride,
                                     const PricingUnit& pu,
                                     const FareUsage& fu) const;

  void setIgnorePermutationBased();
  void determineIgnoreTktAfterRes(AdvResOverride& advResOverride,
                                  const ProcessTagInfo& pti,
                                  const PricingUnit& pu,
                                  const FareUsage& fu);
  void determineIgnoreTktBeforeDeparture(AdvResOverride& advResOverride,
                                         const ProcessTagInfo& pti,
                                         const PricingUnit& pu,
                                         const FareUsage& fu);
  bool checkAdvancedTicketing();
  void determinePermIndepFromDate();
  void permIndepFromDateForNotMapped();

  // types
public:
  typedef std::map<const FareMarket*, std::vector<const ProcessTagInfo*> >
  FareMarket2ProcessTagInfos;
  typedef std::map<const FareMarket*, FareUsage*> FM2FU;
  typedef std::map<const FareMarket*, std::pair<PricingUnit*, FareUsage*> > FM2PUFU;

private:
  struct SameFareBreaksAndCarrier;
  enum VALIDATION_STATUS_ENUM
  {
    NOT_PROCESSED = 0,
    VALID,
    NOT_VALID
  };
  struct CompareTicketResvInd;

  // statics
private:
  static constexpr int VAL_NOT_APPLY = 0;
  static constexpr Indicator IND_NOT_APPLY = ' ';
  static const std::string NOT_APPLY;

  static constexpr Indicator FROM_ADVRES_ORIGIN = 'O';
  static constexpr Indicator FROM_ADVRES_OUTBOUND = 'P';
  static constexpr Indicator FROM_ADVRES_TYPEOFFARE = 'C';
  static constexpr Indicator FROM_ADVRES_NEWTKTDATE = ' ';
  static constexpr Indicator TO_ADVRES_JOURNEY = 'J';
  static constexpr Indicator TO_ADVRES_FARECOMPONENT = 'F';
  static constexpr Indicator TO_ADVRES_PRICINGUNIT = ' ';

  static constexpr Indicator IGNORE = ' ';
  static constexpr Indicator SIMULTANEOUS = 'X';
  static constexpr Indicator PRIOR_TO_DEPARTURE = 'Y';

  static const std::vector<uint16_t> PU_CATEGORIES;

  // members
private:
  RexPricingTrx& _trx;
  Diag689Collector*& _dc;
  DiagCollector* _dc305;
  Itin& _newItin;
  FarePath& _newFarePath;
  FarePath& _excFarePath;
  const ProcessTagPermutation* _permutation;
  std::vector<const ProcessTagInfo*> _allPTIs;
  FM2PUFU _excFM2ExcPuAndFuMapping;
  FareMarket2ProcessTagInfos _unchangedFareBreaksAndCarrierCache;
  std::vector<uint16_t> _puCategories;
  bool _allFCsFBAndCarrierUnchanged;
  bool _isTicketResvIndTheSameOnJourney;
  bool _isSimultaneousCheckNeeded;
  bool _isPriorOfDepartureCheckNeeded;
  VALIDATION_STATUS_ENUM _simultaneousCheckValid;
  VALIDATION_STATUS_ENUM _priorOfDepartureCheckValid;
  DateTime _fromDate;
  RepriceSolutionValidator::AdvResOverrideCache& _advResOverrideCache;
  bool _emptyBytes93to106;

  // *** 2012 new advance purchase application ***
  const GenericRexMapper& _mapper;
  bool _ignoreTktAfterResRestBasedOnPermData;
  bool _ignoreTktBeforeDeptRestBasedOnPermData;
  int _filterPermutationIndex;

  // Permutation independent data
  std::pair<const DateTime*, const Loc*> _permIndepFromDateAndLoc;
  std::map<const PricingUnit*, const DateTime*> _fromDateForNotMapped;
};
}

