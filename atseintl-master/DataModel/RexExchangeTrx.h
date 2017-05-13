//-------------------------------------------------------------------
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

#pragma once

#include "Common/FallbackUtil.h"
#include "DataModel/RexPricingTrx.h"

#include <boost/container/flat_map.hpp>

#include <vector>

namespace tse {

class RexExchangeTrx: public RexPricingTrx
{
  friend class RexExchangeTrxTest;

public:
  enum ItinStatus
  {
    REISSUE_RULES_PASS = 0,
    NUMBER_OF_REISSUES_RESTRICTED,
    REISSUE_RULES_FAIL
  };

  enum KeepOriginalStrategy
  {
    KEEP_STRATEGY_DISABLED,
    KEEP_BRAND,
    KEEP_FARE
  };

  class MultiItinData
  {
  public:
    using NewItinKeepFareMap = std::map<const PaxTypeFare*, FareMarket*>;
    using NewToExcItinFareMarketMapForKeep = std::map<FareMarket*, const FareMarket*>;

  private:
    ItinStatus _itinStatus = ItinStatus::REISSUE_RULES_PASS;
    ReissueOptions _reissueOptions;
    std::vector<ProcessTagPermutation*> _processTagPermutations;
    std::vector<ProcessTagPermutation*> _separatedPermutations;
    FareRetrievalState _fareRetrievalFlags;
    bool _repriceWithDiffDates = false;
    NewItinKeepFareMap _newItinKeepFares;
    NewToExcItinFareMarketMapForKeep _newToExcItinFareMarketMapForKeep;
    DateTime _newItinROEConversionDate; // QREX/BSP project
    DateTime _newItinSecondROEConversionDate;
    bool _useSecondROEConversionDate = false;
    FarePath* _lowestRebookedFarePath = nullptr;
    FarePath* _lowestBookedFarePath = nullptr;
    Tag1Status _tag1PricingSvcCallStatus = Tag1Status::NONE;
    ExpndKeepMap _expndKeepMap;
    bool _allPermutationsRequireCurrentForFlown = true;
    bool _allPermutationsRequireNotCurrentForFlown = true;
    bool _allPermutationsRequireCurrentForUnflown = true;
    bool _allPermutationsRequireNotCurrentForUnflown = true;
    DepartureDateValidator _departureDateValidator;
    OptimizationMapper _optimizationMapper;

  public:
    ItinStatus& itinStatus() { return _itinStatus; }
    const ItinStatus& itinStatus() const { return _itinStatus; }
    
    ReissueOptions& reissueOptions() { return _reissueOptions; }
    const ReissueOptions& reissueOptions() const { return _reissueOptions; }

    std::vector<ProcessTagPermutation*>& processTagPermutations()
    {
      return _tag1PricingSvcCallStatus ? _separatedPermutations : _processTagPermutations;
    }

    const std::vector<ProcessTagPermutation*>& processTagPermutations() const
    {
      return _tag1PricingSvcCallStatus ? _separatedPermutations : _processTagPermutations;
    }

    DateTime& newItinROEConversionDate() { return _newItinROEConversionDate; }
    const DateTime& newItinROEConversionDate() const { return _newItinROEConversionDate; }

    DateTime& newItinSecondROEConversionDate() { return _newItinSecondROEConversionDate; }
    const DateTime& newItinSecondROEConversionDate() const
    {
      return _newItinSecondROEConversionDate;
    }

    bool& useSecondROEConversionDate() { return _useSecondROEConversionDate; }
    const bool& useSecondROEConversionDate() const { return _useSecondROEConversionDate; }

    FareRetrievalState& fareRetrievalFlags() { return _fareRetrievalFlags; }
    const FareRetrievalState& fareRetrievalFlags() const { return _fareRetrievalFlags; }

    bool& repriceWithDiffDates() { return _repriceWithDiffDates; }
    bool repriceWithDiffDates() const { return _repriceWithDiffDates; }

    NewItinKeepFareMap& newItinKeepFares() { return _newItinKeepFares; }
    const NewItinKeepFareMap& newItinKeepFares() const { return _newItinKeepFares; }

    NewToExcItinFareMarketMapForKeep& newToExcItinFareMarketMapForKeep()
    {
      return _newToExcItinFareMarketMapForKeep;
    }

    const NewToExcItinFareMarketMapForKeep& newToExcItinFareMarketMapForKeep() const
    {
      return _newToExcItinFareMarketMapForKeep;
    }

    FarePath*& lowestRebookedFarePath() { return _lowestRebookedFarePath; }
    const FarePath* lowestRebookedFarePath() const { return _lowestRebookedFarePath; }

    FarePath*& lowestBookedFarePath() { return _lowestBookedFarePath; }
    const FarePath* lowestBookedFarePath() const { return _lowestBookedFarePath; }

    Tag1Status tag1PricingSvcCallStatus() const { return _tag1PricingSvcCallStatus; }
    Tag1Status& tag1PricingSvcCallStatus() { return _tag1PricingSvcCallStatus; }

    ExpndKeepMap& expndKeepMap() { return _expndKeepMap; }
    const ExpndKeepMap& expndKeepMap() const { return _expndKeepMap; }

    bool& allPermutationsRequireCurrentForFlown() { return _allPermutationsRequireCurrentForFlown; }
    const bool& allPermutationsRequireCurrentForFlown() const
    {
      return _allPermutationsRequireCurrentForFlown;
    }

    bool& allPermutationsRequireNotCurrentForFlown()
    {
      return _allPermutationsRequireNotCurrentForFlown;
    }

    const bool& allPermutationsRequireNotCurrentForFlown() const
    {
      return _allPermutationsRequireNotCurrentForFlown;
    }

    bool& allPermutationsRequireCurrentForUnflown()
    {
      return _allPermutationsRequireCurrentForUnflown;
    }

    const bool& allPermutationsRequireCurrentForUnflown() const
    {
      return _allPermutationsRequireCurrentForUnflown;
    }

    bool& allPermutationsRequireNotCurrentForUnflown()
    {
      return _allPermutationsRequireNotCurrentForUnflown;
    }

    const bool& allPermutationsRequireNotCurrentForUnflown() const
    {
      return _allPermutationsRequireNotCurrentForUnflown;
    }

    DepartureDateValidator& departureDateValidator() { return _departureDateValidator; }
    const DepartureDateValidator& departureDateValidator() const { return _departureDateValidator; }

    const OptimizationMapper& getOptimizationMapper() const
    {
      return _optimizationMapper;
    }

    OptimizationMapper& optimizationMapper()
    {
      return _optimizationMapper;
    }
  };

  void ititializeMultiItinData();
  
  ReissueOptions& reissueOptions() override
  {
    return getMultiItinData().reissueOptions();
  }
  
  const ReissueOptions& reissueOptions() const override
  {
    return getMultiItinData().reissueOptions();
  }

  virtual std::vector<ProcessTagPermutation*>& processTagPermutations() override
  {
    return getMultiItinData().processTagPermutations();
  }

  virtual const std::vector<ProcessTagPermutation*>& processTagPermutations() const override
  {
    return getMultiItinData().processTagPermutations();
  }

  virtual std::vector<ProcessTagPermutation*>& processTagPermutations(uint16_t itinIndex) override
  {
    return getMultiItinData(itinIndex).processTagPermutations();
  }

  virtual const std::vector<ProcessTagPermutation*>&
  processTagPermutations(uint16_t itinIndex) const override
  {
    return getMultiItinData(itinIndex).processTagPermutations();
  }

  virtual Tag1Status tag1PricingSvcCallStatus() const override
  {
    return getMultiItinData().tag1PricingSvcCallStatus();
  }

  virtual DateTime& newItinROEConversionDate() override
  {
    return getMultiItinData().newItinROEConversionDate();
  }

  virtual const DateTime& newItinROEConversionDate() const override
  {
    return getMultiItinData().newItinROEConversionDate();
  }

  virtual DateTime& newItinSecondROEConversionDate() override
  {
    return getMultiItinData().newItinSecondROEConversionDate();
  }

  virtual const DateTime& newItinSecondROEConversionDate() const override
  {
    return getMultiItinData().newItinSecondROEConversionDate();
  }

  virtual bool& useSecondROEConversionDate() override
  {
    return getMultiItinData().useSecondROEConversionDate();
  }
  virtual const bool& useSecondROEConversionDate() const override
  {
    return getMultiItinData().useSecondROEConversionDate();
  }

  virtual DateTime& newItinROEConversionDate(uint16_t itinIndex) override
  {
    return getMultiItinData(itinIndex).newItinROEConversionDate();
  }

  virtual const DateTime& newItinROEConversionDate(uint16_t itinIndex) const override
  {
    return getMultiItinData(itinIndex).newItinROEConversionDate();
  }

  virtual FareRetrievalState& fareRetrievalFlags() override
  {
    return getMultiItinData().fareRetrievalFlags();
  }

  virtual const FareRetrievalState& fareRetrievalFlags() const override
  {
    return getMultiItinData().fareRetrievalFlags();
  }

  virtual void markFareRetrievalMethodHistorical(const bool setBits = true) override
  {
    getMultiItinData().fareRetrievalFlags().set(FareMarket::RetrievHistorical, setBits);
  }

  virtual void markFareRetrievalMethodKeep(const bool setBits = true) override
  {
    getMultiItinData().fareRetrievalFlags().set(FareMarket::RetrievKeep, setBits);
  }

  virtual void markFareRetrievalMethodTvlCommence(const bool setBits = true) override
  {
    getMultiItinData().fareRetrievalFlags().set(FareMarket::RetrievTvlCommence, setBits);
  }

  virtual void markFareRetrievalMethodCurrent(const bool setBits = true) override
  {
    getMultiItinData().fareRetrievalFlags().set(FareMarket::RetrievCurrent, setBits);
  }

  virtual void markFareRetrievalMethodLastReissue(const bool setBits = true) override
  {
    getMultiItinData().fareRetrievalFlags().set(FareMarket::RetrievLastReissue, setBits);
  }

  virtual void markAllFareRetrievalMethod(const bool setBits = true) override
  {
    getMultiItinData().fareRetrievalFlags().set(
        (FareMarket::FareRetrievalFlags)(FareMarket::RetrievHistorical | FareMarket::RetrievKeep |
                                         FareMarket::RetrievTvlCommence | FareMarket::RetrievCurrent
                                         //| FareMarket::RetrievLastReissue
                                         ),
        setBits);
  }

  virtual bool needRetrieveHistoricalFare() const override
  {
    return getMultiItinData().fareRetrievalFlags().isSet(FareMarket::RetrievHistorical);
  }

  virtual bool needRetrieveKeepFare() const override
  {
    return getMultiItinData().fareRetrievalFlags().isSet(FareMarket::RetrievKeep);
  }

  virtual bool needRetrieveKeepFareAnyItin() const override;

  virtual bool needRetrieveKeepFare(uint16_t itinIndex) const override
  {
    return getMultiItinData(itinIndex).fareRetrievalFlags().isSet(FareMarket::RetrievKeep);
  }

  virtual bool needRetrieveTvlCommenceFare() const override
  {
    return getMultiItinData().fareRetrievalFlags().isSet(FareMarket::RetrievTvlCommence);
  }

  virtual bool needRetrieveCurrentFare() const override
  {
    return getMultiItinData().fareRetrievalFlags().isSet(FareMarket::RetrievCurrent);
  }

  virtual bool needRetrieveLastReissueFare() const override
  {
    return getMultiItinData().fareRetrievalFlags().isSet(FareMarket::RetrievLastReissue);
  }

  virtual bool needRetrieveAllFare() const override
  {
    return getMultiItinData().fareRetrievalFlags().isAllSet(
        (FareMarket::FareRetrievalFlags)(FareMarket::RetrievHistorical | FareMarket::RetrievKeep |
                                         FareMarket::RetrievTvlCommence | FareMarket::RetrievCurrent
                                         // | FareMarket::RetrievLastReissue
                                         ));
  }

  virtual bool needRetrieveFare() const override
  {
    return !getMultiItinData().fareRetrievalFlags().isNull();
  }

  virtual NewItinKeepFareMap& newItinKeepFares() override
  {
    return getMultiItinData().newItinKeepFares();
  }

  virtual const NewItinKeepFareMap& newItinKeepFares() const override
  {
    return getMultiItinData().newItinKeepFares();
  }

  virtual NewItinKeepFareMap& newItinKeepFares(uint16_t itinIndex) override
  {
    return getMultiItinData(itinIndex).newItinKeepFares();
  }

  virtual const NewItinKeepFareMap& newItinKeepFares(uint16_t itinIndex) const override
  {
    return getMultiItinData(itinIndex).newItinKeepFares();
  }

  virtual NewToExcItinFareMarketMapForKeep& newToExcItinFareMarketMapForKeep() override
  {
    return getMultiItinData().newToExcItinFareMarketMapForKeep();
  }

  virtual const NewToExcItinFareMarketMapForKeep& newToExcItinFareMarketMapForKeep() const override
  {
    return getMultiItinData().newToExcItinFareMarketMapForKeep();
  }

  virtual bool& repriceWithDiffDates() override
  {
    return getMultiItinData().repriceWithDiffDates();
  }
  virtual bool repriceWithDiffDates() const override
  {
    return getMultiItinData().repriceWithDiffDates();
  }

  virtual bool repriceWithSameFareDate() override;

  ItinStatus& itinStatus() { return getMultiItinData().itinStatus(); }
  const ItinStatus& itinStatus() const { return getMultiItinData().itinStatus(); }

  virtual bool process(Service& srv) override { return srv.process(*this); }

  void addMotherItinIndex(const Itin* itin, uint16_t itinIndex)
  {
    _motherItinIndex[itin] = itinIndex;
  }

  FarePath*& lowestRebookedFarePath() override
  {
    return getMultiItinData().lowestRebookedFarePath();
  }
  const FarePath* lowestRebookedFarePath() const override
  {
    return getMultiItinData().lowestRebookedFarePath();
  }

  FarePath*& lowestBookedFarePath() override { return getMultiItinData().lowestBookedFarePath(); }
  const FarePath* lowestBookedFarePath() const override
  {
    return getMultiItinData().lowestBookedFarePath();
  }

  virtual ExpndKeepMap& expndKeepMap() override { return getMultiItinData().expndKeepMap(); }
  virtual const ExpndKeepMap& expndKeepMap() const override
  {
    return getMultiItinData().expndKeepMap();
  }

  virtual bool& allPermutationsRequireCurrentForFlown() override
  {
    return getMultiItinData().allPermutationsRequireCurrentForFlown();
  }

  virtual bool& allPermutationsRequireNotCurrentForFlown() override
  {
    return getMultiItinData().allPermutationsRequireNotCurrentForFlown();
  }

  virtual bool& allPermutationsRequireCurrentForUnflown() override
  {
    return getMultiItinData().allPermutationsRequireCurrentForUnflown();
  }

  virtual bool& allPermutationsRequireNotCurrentForUnflown() override
  {
    return getMultiItinData().allPermutationsRequireNotCurrentForUnflown();
  }

  virtual const OptimizationMapper& getOptimizationMapper() const
  {
    return getMultiItinData().getOptimizationMapper();
  }

  virtual OptimizationMapper& optimizationMapper()
  {
    return getMultiItinData().optimizationMapper();
  }

  int32_t getMaxPriceJump() const
  {
    return _maxPriceJump;
  }

  void setMaxPriceJump(int32_t maxPriceJump)
  {
    _maxPriceJump = maxPriceJump;
  }

  void addNewFMtoMaxPrice(const FareMarket* newFM, MoneyAmount maxPrice)
  {
    _newFMtoMaxPrice[newFM->getHashKey()] = maxPrice;
  }

  MoneyAmount getMaxPriceForFM(const FareMarket* newFM) const
  {
    FmToMaxPriceMap::const_iterator element(_newFMtoMaxPrice.find(newFM->getHashKey()));
    return element != _newFMtoMaxPrice.end() ? element->second :
                                               std::numeric_limits<MoneyAmount>::max();
  }

  void addNewFMtoBrandCodeSet(const FareMarket* newFM, const BrandCode brandCode)
  {
    _newFMtoKeepBrand[newFM->getHashKey()] = brandCode;
  }

  BrandCode getBrandsUsedInOriginalTicket(const FareMarket* fm) const
  {
    FmToKeepBrandMap::const_iterator fmIter = _newFMtoKeepBrand.find(fm->getHashKey());

    if(fmIter == _newFMtoKeepBrand.end())
    {
      //not flown or shopped faremarkets always pass
      return BrandCode();
    }

    return fmIter->second;
  }

  void setKeepOriginal(bool value)
  {
    _keepOriginal = value;
  }

  bool getKeepOriginal() const
  {
    return _keepOriginal;
  }

  void setKeepOriginalStrategy(KeepOriginalStrategy keepOriginalStrategy)
  {
    _keepOriginalStrategy = keepOriginalStrategy;
  }

  KeepOriginalStrategy getKeepBrandStrategy() const
  {
    return _keepOriginalStrategy;
  }

protected:
  virtual Tag1Status& setTag1PricingSvcCallStatus() override
  {
    return getMultiItinData().tag1PricingSvcCallStatus();
  }

  virtual DepartureDateValidator& departureDateValidator() override
  {
    return getMultiItinData().departureDateValidator();
  }

  virtual const DepartureDateValidator& departureDateValidator() const override
  {
    return getMultiItinData().departureDateValidator();
  }

  MultiItinData& getMultiItinData(uint16_t itinIndex) { return *_multiItinData[itinIndex]; }
  const MultiItinData& getMultiItinData(uint16_t itinIndex) const
  {
    return *_multiItinData[itinIndex];
  }

  MultiItinData& getMultiItinData() { return *_multiItinData[_itinIndex]; }
  const MultiItinData& getMultiItinData() const { return *_multiItinData[_itinIndex]; }

  using FmToMaxPriceMap = boost::container::flat_map<std::string, MoneyAmount>;
  using FmToKeepBrandMap = boost::container::flat_map<std::string, BrandCode>;

  std::vector<MultiItinData*> _multiItinData;

  //max price jump for flown and not shopped segments
  //in exchange shopping with brands
  int32_t _maxPriceJump = -1;

  //keep original brand or fare for flown or not shopped portion
  bool _keepOriginal = false;

  FmToMaxPriceMap _newFMtoMaxPrice;
  FmToKeepBrandMap _newFMtoKeepBrand;
  KeepOriginalStrategy _keepOriginalStrategy = KeepOriginalStrategy::KEEP_STRATEGY_DISABLED;
};
} // tse namespace
