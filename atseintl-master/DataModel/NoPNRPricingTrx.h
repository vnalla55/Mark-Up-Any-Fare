//-------------------------------------------------------------------
//  Copyright Sabre 2007
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

#include "Common/CabinType.h"
#include "Common/LocUtil.h"
#include "Common/NoPNRTravelSegmentTimeUpdater.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/NoPNRPricingOptions.h"
#include "DBAccess/NoPNROptions.h"
#include "Service/Service.h"

#include <tr1/unordered_map>

namespace tse
{
class Agent;

class NoPNRPricingTrx : public AltPricingTrx
{
public:
  class FareTypes
  {
  private:
    struct equalFT
    {
      bool operator()(const FareType ftp1, const FareType ftp2) const
      {
        return ftp1.compare(ftp2) == 0;
      }
    };

    struct hashFT
    {
      size_t operator()(const FareType& ftp) const;
    };

    using FareTypeGroup = int;
    using HashMap = std::tr1::unordered_map<FareType, FareTypeGroup, hashFT, equalFT>;

  protected:
    HashMap _fareTypes;
    const NoPNRPricingTrx& _trx;

  public:
    enum FTGroup
    {
      FTG_NONE = 0,
      FTG_BUSINESS,
      FTG_ECONOMY,
      FTG_ECONOMY_EXCURSION,
      FTG_ECONOMY_ADVANCE_PURCHASE,
      FTG_ECONOMY_INSTANT_PURCHASE,
      FTG_FIRST,
      FTG_PREMIUM_ECONOMY,
      FTG_PREMIUM_FIRST,
      FTG_PROMOTIONAL,
      FTG_SPECIAL,
      FTG_MAX
    };

    static const char* FTGC_NONE;
    static const char* FTGC_BUSINESS;
    static const char* FTGC_ECONOMY;
    static const char* FTGC_ECONOMY_EXCURSION;
    static const char* FTGC_ECONOMY_ADVANCE_PURCHASE;
    static const char* FTGC_ECONOMY_INSTANT_PURCHASE;
    static const char* FTGC_FIRST;
    static const char* FTGC_PREMIUM_ECONOMY;
    static const char* FTGC_PREMIUM_FIRST;
    static const char* FTGC_PROMOTIONAL;
    static const char* FTGC_SPECIAL;

    static const char* FTGC[];

    FareTypes(const NoPNRPricingTrx& trx) : _trx(trx) {}

    void loadFareTypes();
    FTGroup getFareTypeGroup(const FareType& ftp);

    friend class NoPNRPricingTrxTest;
    friend class FarePathFactoryTest;
    friend class FareValidatorOrchestratorTest;
  };

  class Solutions
  {
  public:
    Solutions(const NoPNRPricingTrx& trx) : _trx(trx) {}

    void initialize();
    void clear();

    bool none() const { return count() == 0; }
    bool all() const { return count() == _maxAll; }

    int limit(PaxType* paxType) { return element(paxType)->_limit; }
    void limit(int solutions);

    int found(PaxType* paxType) { return element(paxType)->_found; }
    void found(PaxType* paxType, int solutions, bool add = false);

    bool& process(PaxType* paxType) { return element(paxType)->_process; }

    int max() { return _max; }

  private:
    struct PaxTypeInfo
    {
      int _found = 0;
      int _limit = 0;
      bool _process = false;
    };

    PaxTypeInfo* element(PaxType* paxType);
    int count() const;

    const NoPNRPricingTrx& _trx;
    std::map<PaxType*, PaxTypeInfo*> _info;
    int _max = 0;
    int _maxAll = 0;
  };

  bool process(Service& svc) override { return svc.process(*this); }

  void convert(tse::ErrorResponseException& ere, std::string& response) override;

  bool convert(std::string& response) override;

  NoPNRPricingOptions* getOptions() { return static_cast<NoPNRPricingOptions*>(_options); }
  const NoPNRPricingOptions* getOptions() const
  {
    return static_cast<NoPNRPricingOptions*>(_options);
  }

  void setOptions(NoPNRPricingOptions* options) { _options = options; }

  const NoPNROptions* noPNROptions() const { return _noPNROptions; }

  const FareTypes& fareTypes() const { return _fareTypes; }
  FareTypes& fareTypes() { return _fareTypes; }

  void loadNoPNROptions();
  static NoPNROptions* getNoPNROptions(const Agent* agent, DataHandle& dataHandle);

  bool noRBDItin() const { return _noRBDItin; }
  bool& noRBDItin() { return _noRBDItin; }

  bool reprocess() const { return _reprocess; }
  bool& reprocess() { return _reprocess; }

  const FareTypes::FTGroup& processedFTGroup() const { return _processedFTGroup; }
  FareTypes::FTGroup& processedFTGroup() { return _processedFTGroup; }

  bool isNoMatch() const
  {
    return (_noRBDItin || _reprocess || getOptions()->isNoMatch()) && !_fullFBCItin;
  }

  void initializeOpenSegmentDates() { _travelSegmentTimeUpdater.initialize(*this); }
  void updateOpenDateIfNeccesary(const TravelSeg* theSegment, DateTime& toUpdate)
  {
    _travelSegmentTimeUpdater.updateOpenDateIfNeccesary(theSegment, toUpdate);
  }

  const Solutions& solutions() const { return _solutions; }
  Solutions& solutions() { return _solutions; }

  bool diagDisplay() const { return _diagDisplay; }
  bool& diagDisplay() { return _diagDisplay; }

  bool isXM() { return getOptions()->isNoMatch(); }

  bool integrated() const { return _integrated; }
  bool& integrated() { return _integrated; }

  NoPNRPricingTrx::FareTypes::FTGroup mapFTtoFTG(const std::string& diagQlf) const;

  void changeOptions(NoPNROptions* newOptions) { _noPNROptions = newOptions; } // for diag only

  bool lowestFare() const { return _lowestFare; }
  bool& lowestFare() { return _lowestFare; }

  void prepareNoMatchItin();
  void restoreOldItin();

  const std::map<const TravelSeg*, const std::string>& GIWarningMap() const
  {
    return _GIWarningMap;
  }

  std::map<const TravelSeg*, const std::string>& GIWarningMap() { return _GIWarningMap; }

  std::map<const TravelSeg*, std::string>& globalDirectionOverride()
  {
    return _globalDirectionOverride;
  }

  const std::map<const TravelSeg*, std::string>& globalDirectionOverride() const
  {
    return _globalDirectionOverride;
  }

  void setFullFBCItin();
  bool isFullFBCItin() const { return _fullFBCItin; }

protected:
  bool _noRBDItin = false;
  bool _reprocess = false;
  NoPNROptions* _noPNROptions = nullptr;
  FareTypes::FTGroup _processedFTGroup = NoPNRPricingTrx::FareTypes::FTG_NONE;
  bool _diagDisplay = true;
  FareTypes _fareTypes{*this};
  Solutions _solutions{*this};
  bool _integrated = false;
  bool _lowestFare = false;
  std::map<const TravelSeg*, const std::string> _GIWarningMap;
  std::map<const TravelSeg*, std::string> _globalDirectionOverride;
  bool _fullFBCItin = false;

private:
  struct SegmentInfo
  {
    BookingCode _bookingCode;
    CabinType _bookedCabin;
  };

  NoPNRTravelSegmentTimeUpdater _travelSegmentTimeUpdater;

  std::vector<SegmentInfo*> _entryInfos;

  friend class NoPNRPricingTrxTest;
};
} // tse namespace
