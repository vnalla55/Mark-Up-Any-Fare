//-------------------------------------------------------------------
//
//  File:        ServiceFeesGroup.h
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
//-------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseUtil.h"
#include "DataModel/FarePath.h"
#include "DBAccess/MerchCarrierPreferenceInfo.h"

#include <map>
#include <tuple>
#include <vector>

namespace tse
{
class Diag880Collector;
class FarePath;
class MerchCarrierStrategy;
class Money;
class OCFees;
class OCFeesUsage;
class SubCodeInfo;

struct FPOCFees
{
  FPOCFees(const FarePath* fp, OCFees* ocFees) : _fp(fp), _ocFees(ocFees) {}
  const FarePath* fp(void) const { return _fp; }
  const OCFees* fees(void) const { return _ocFees; }

private:
  const FarePath* _fp;
  OCFees* _ocFees;
};

struct PaxOCFees
{
  PaxOCFees(const FPOCFees& fpOCFees)
    : _paxType(fpOCFees.fp()->paxType()->paxType()), _ocFees(fpOCFees.fees())
  {
  }

  const PaxTypeCode paxType(void) const { return _paxType; }
  void setPaxType(PaxTypeCode paxType) { _paxType = paxType; }
  const OCFees* fees(void) const { return _ocFees; }

protected:
  PaxTypeCode _paxType;
  const OCFees* _ocFees;
};

struct PaxR7OCFees : public PaxOCFees
{
  PaxR7OCFees(const FPOCFees& fpOCFees)
    : PaxOCFees(fpOCFees), _realPaxType(fpOCFees.fp()->paxType()), _itin(fpOCFees.fp()->itin())
  {
  }

  const PaxType* realPaxType() const { return _paxType.equalToConst("ALL") ? nullptr : _realPaxType; }
  const Itin* itin() const { return _itin; }

private:
  const PaxType* _realPaxType;
  const Itin* _itin;
};
// Three structs below are for the OCFeesUsage development at Response build time
struct FPOCFeesUsages
{
  FPOCFeesUsages(const FarePath* fp, OCFeesUsage* ocFeeUsage) : _fp(fp), _ocFeesU(ocFeeUsage) {}
  const FarePath* fp(void) const { return _fp; }
  const OCFeesUsage* fees(void) const { return _ocFeesU; }

private:
  const FarePath* _fp;
  OCFeesUsage* _ocFeesU;
};

struct PaxOCFeesUsages
{
  PaxOCFeesUsages(const FPOCFeesUsages& fpOCFees, int id=0)
    : _paxType(fpOCFees.fp()->paxType()->paxType()), _ocFeesU(fpOCFees.fees()), _id(id)
  {
  }

  const PaxTypeCode paxType(void) const { return _paxType; }
  void setPaxType(PaxTypeCode paxType) { _paxType = paxType; }
  const OCFeesUsage* fees(void) const { return _ocFeesU; }
  int getId() const { return _id; }

protected:
  PaxTypeCode _paxType;
  const OCFeesUsage* _ocFeesU;
  int _id;
};

struct PaxR7OCFeesUsages : public PaxOCFeesUsages
{
  PaxR7OCFeesUsages(const FPOCFeesUsages& fpOCFees)
    : PaxOCFeesUsages(fpOCFees),
      _realPaxType(fpOCFees.fp()->paxType()),
      _itin(fpOCFees.fp()->itin())
  {
  }

  const PaxType* realPaxType() const { return _paxType.equalToConst("ALL") ? nullptr : _realPaxType; }
  const Itin* itin() const { return _itin; }

private:
  const PaxType* _realPaxType;
  const Itin* _itin;
};

class ServiceFeesGroup
{
  friend class ServiceFeesGroupTest;

public:
  enum StateCode
  {
    VALID = 0,
    EMPTY,
    INVALID,
    NOT_AVAILABLE
  };

  static const char* const _stateCodeStrings[];

  typedef std::tuple<FarePath*, ServiceSubTypeCode, Indicator> SubCodeMapKey;
  typedef std::map<std::pair<TravelSeg*, TravelSeg*>, std::pair<OCFees*, bool> > KeyItemMap;
  typedef std::map<SubCodeMapKey, KeyItemMap> SubCodeMap;
  typedef std::pair<std::vector<TravelSeg*>::const_iterator,
                    std::vector<TravelSeg*>::const_iterator> TvlSegPair;
  typedef std::map<CarrierCode, MerchCarrierPreferenceInfo*> MerchCxrPrefMap;
  typedef std::vector<tse::TravelSeg*>::const_iterator TSIt;

  // ctors
  ServiceFeesGroup();
  virtual ~ServiceFeesGroup();
  void initialize(PricingTrx* trx) { _trx = trx; }

  typedef const std::vector<TvlSegPair*>* (ServiceFeesGroup::*FindSolution)(
      std::pair<const SubCodeMapKey, KeyItemMap>& subCodeItem,
      TseUtil::SolutionSet& solutions,
      std::vector<TravelSeg*>::const_iterator firstSegI,
      std::vector<TravelSeg*>::const_iterator endSegI,
      Diag880Collector* diag880,
      const DateTime& tktDate,
      uint16_t multiTicketNbr);
  FindSolution getFindSolutionForSubCode() { return &ServiceFeesGroup::findSolutionForSubCode; }
  FindSolution getFindSolutionForSubCodeForAncillaryPricing()
  {
    return &ServiceFeesGroup::findSolutionForSubCodeForAncillaryPricing;
  }

  void findSolution(int unitNo,
                    TseUtil::SolutionSet& solutions,
                    std::vector<TravelSeg*>::const_iterator firstSegI,
                    std::vector<TravelSeg*>::const_iterator endSegI,
                    Diag880Collector* diag880,
                    const DateTime& tktDate,
                    FindSolution findSolutionForSub,
                    uint16_t multiTicketNbr);

  bool isSubCodePassed(int unitNo,
                       FarePath* farePath,
                       TravelSeg* firstTvl,
                       TravelSeg* lastTvl,
                       const ServiceSubTypeCode& subCode,
                       const Indicator& serviceType) const;

  bool findTravelWhenNoArkunksOnBegOrEnd(
      std::pair<ServiceFeesGroup::TSIt, ServiceFeesGroup::TSIt>& noArunkTvl,
      const TSIt& begIt,
      const TSIt& endIt) const;

  virtual void collectUnitsOfTravel(std::vector<std::tuple<TSIt, TSIt, bool> >& unitsOfTravel,
                                    TSIt begIt,
                                    const TSIt endIt);

  virtual void
  collectUnitsOfTravelForSA(std::vector<std::tuple<TSIt, TSIt, bool> >& unitsOfTravel,
                            TSIt begIt,
                            const TSIt endIt);

  // accessors
  ServiceGroup& groupCode() { return _groupCode; }
  const ServiceGroup& groupCode() const { return _groupCode; }

  std::string& groupDescription() { return _description; }
  const std::string& groupDescription() const { return _description; }

  StateCode& state() { return _state; }
  const StateCode& state() const { return _state; }
  const char* stateStr() const { return (_state > NOT_AVAILABLE) ? "INVALID_ENUM" : _stateCodeStrings[_state]; }

  std::map<const FarePath*, std::vector<OCFees*> >& ocFeesMap() { return _ocFeesMap; }
  const std::map<const FarePath*, std::vector<OCFees*> >& ocFeesMap() const { return _ocFeesMap; }

  SubCodeMap& subCodeMap(int unitNo) { return _subCodeMap[unitNo]; }
  const SubCodeMap& subCodeMap(int unitNo) const { return _subCodeMap[unitNo]; }

  bool& sliceDicePassed() { return _sliceDicePassed; }
  const bool& sliceDicePassed() const { return _sliceDicePassed; }

  MerchCxrPrefMap& merchCxrPref() { return _merchCxrPref; }
  const MerchCxrPrefMap& merchCxrPref() const { return _merchCxrPref; }

  void setSliceAndDiceProcessed(bool set = true) { _sliceAndDiceProcessed = set; }
  bool isSliceAndDiceProcessed() const { return _sliceAndDiceProcessed; }

  boost::mutex& mutex() const { return _mutex; }

protected:
  bool foundSolution(uint16_t& skippedSegs,
                     const uint16_t currSkippedSegs,
                     const MoneyAmount minFeeAmountSum) const;
  bool getAmountSum(Money& sum, const std::vector<TvlSegPair*>& routes, KeyItemMap& item) const;
  bool getConvertedAmountSum_old(std::vector<OCFees*>& fees, Money& sum) const; // To remove with ocFeesAmountRoundingRefactoring
  bool getConvertedAmountSum(std::vector<OCFees*>& fees, Money& sum) const;
  CurrencyCode determineOutputCurrency(const TseUtil::SolutionSet::nth_index<0>::type& index,
                                       const KeyItemMap& item) const;
  void getMktCxr(std::multiset<CarrierCode>& marketingCxrs,
                 const std::vector<TvlSegPair*>& routes,
                 const KeyItemMap& item) const;
  void countCarrierOccurrences(std::multiset<uint16_t, std::greater<uint16_t> >& cxrCounter,
                               const std::multiset<CarrierCode>& marketingCxrs);
  void chooseSolution(MoneyAmount& minFeeAmountSum,
                      const std::vector<TvlSegPair*>*& winningSolution,
                      std::multiset<uint16_t, std::greater<uint16_t> >& winningCxrCounter,
                      std::multiset<uint16_t, std::greater<uint16_t> >& cxrCounter,
                      const MoneyAmount feeAmountSum,
                      const std::vector<TvlSegPair*>& routes) const;
  void printSolutions(Diag880Collector* diag880,
                      uint16_t& solNo,
                      const std::vector<TvlSegPair*>& routes,
                      const KeyItemMap& item,
                      const std::vector<TvlSegPair*>* winningSolution);
  void printSolutions(Diag880Collector* diag880,
                      const DateTime& tktDate,
                      uint16_t& solNo,
                      const std::vector<TvlSegPair*>& routes,
                      const KeyItemMap& item,
                      const std::vector<TvlSegPair*>* winningSolution);

  bool shouldProcessSliceAndDice(std::pair<const SubCodeMapKey, KeyItemMap>& subCodeItem,
                                 const std::vector<TravelSeg*>::const_iterator& firstSegI,
                                 const std::vector<TravelSeg*>::const_iterator& endSegI);
  const std::vector<TvlSegPair*>*
  findSolutionForSubCode(std::pair<const SubCodeMapKey, KeyItemMap>& subCodeItem,
                         TseUtil::SolutionSet& solutions,
                         std::vector<TravelSeg*>::const_iterator firstSegI,
                         std::vector<TravelSeg*>::const_iterator endSegI,
                         Diag880Collector* diag880,
                         const DateTime& tktDate,
                         uint16_t multiTicketNbr);

  const std::vector<ServiceFeesGroup::TvlSegPair*>*
  findSolutionForSubCodeForAncillaryPricing(std::pair<const SubCodeMapKey, KeyItemMap>& subCodeItem,
                                            TseUtil::SolutionSet& solutions,
                                            std::vector<TravelSeg*>::const_iterator firstSegI,
                                            std::vector<TravelSeg*>::const_iterator endSegI,
                                            Diag880Collector* diag880,
                                            const DateTime& tktDate,
                                            uint16_t multiTicketNbr);

  // overrides of external interfaces

  virtual bool getFeeRounding_old(const CurrencyCode& currencyCode,
                              RoundingFactor& roundingFactor,
                              CurrencyNoDec& roundingNoDec,
                              RoundingRule& roundingRule) const; // To remove with ocFeesAmountRoundingRefactoring
  virtual void convertCurrency(Money& target, const Money& source) const;
  void
  removeSoftMatches(std::vector<std::pair<const TravelSeg*, const TravelSeg*> >& winningSolutionSeg,
                    FarePath* farePath,
                    const ServiceSubTypeCode srvSubTypeCode,
                    const Indicator& serviceType,
                    TravelSeg* travelStart,
                    TravelSeg* travelEnd);
  void removeInvalidOCFees(const std::vector<ServiceFeesGroup::TvlSegPair*>* winningSolution,
                           FarePath* farePath,
                           const ServiceSubTypeCode srvSubTypeCode,
                           const Indicator& serviceType,
                           TravelSeg* travelStart,
                           TravelSeg* travelEnd,
                           bool keepOnlyHardMatch);

private:
  ServiceGroup _groupCode{""};
  std::string _description;
  StateCode _state{VALID};
  std::map<const FarePath*, std::vector<OCFees*> > _ocFeesMap;
  PricingTrx* _trx{nullptr};
  SubCodeMap _subCodeMap[2];
  uint32_t _maxDiagSize{0};
  bool _sliceDicePassed{true};
  MerchCxrPrefMap _merchCxrPref;
  bool _sliceAndDiceProcessed{false};
  mutable boost::mutex _mutex;

public:
  struct SubCodeInitializer
  {
    friend class ServiceFeesGroupTest;

    SubCodeInitializer(PricingTrx& trx,
                       FarePath* _farePath,
                       TravelSeg* first,
                       TravelSeg* last,
                       MerchCarrierStrategy& merchStrategy);
    virtual ~SubCodeInitializer();

    void operator()(ServiceFeesGroup* srvFeesGroup,
                    const CarrierCode& carrier,
                    const ServiceTypeCode& srvTypeCode,
                    const ServiceGroup& srvGroup,
                    const DateTime& travelDate) const;

  protected:
    virtual void getSubCode(std::vector<SubCodeInfo*>& subCodes,
                            const CarrierCode& carrier,
                            const ServiceTypeCode& srvTypeCode,
                            const ServiceGroup& groupCode,
                            const DateTime& travelDate) const;
    virtual OCFees* newOCFee() const;
    void addOCFees(const SubCodeInfo* subCodeInfo,
                   ServiceFeesGroup* srvFeesGroup,
                   const CarrierCode& carrier) const;

  private:
    PricingTrx& _trx;
    FarePath* _farePath;
    TravelSeg* _first;
    TravelSeg* _end;
    MerchCarrierStrategy& _merchStrategy;
  };
};

} // tse


