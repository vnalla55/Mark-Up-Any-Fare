//-------------------------------------------------------------------
//  Copyright Sabre 2015
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

#include "Common/OcTypes.h"
#include "FreeBagService/BaggageS7SubvalidatorInterfaces.h"
#include "FreeBagService/BTAValidationProcessObserver.h"

#include <bitset>
#include <map>
#include <vector>

namespace tse
{
class Diag852Collector;
class BaggageTravel;
class IS7RecordFieldsValidator;
class OptionalServicesInfo;
class SvcFeesResBkgDesigInfo;
class SvcFeesCxrResultingFCLInfo;
class TravelSeg;

class RegularBtaSubvalidator : public IBtaSubvalidator
{
public:
  enum BtaCheck
  { CABIN,
    RBD,
    T186,
    FARE_CLASS,
    TKT_DESIG,
    TARIFF,
    RULE,
    FARE_IND,
    NUM_CHECKS };

  using SegAndFare = std::pair<TravelSeg*, const PaxTypeFare*>;
  using CheckMask = std::bitset<NUM_CHECKS>;

  RegularBtaSubvalidator(const BaggageTravel& bt,
                         const IS7RecordFieldsValidator& s7Validator,
                         const Ts2ss& ts2ss,
                         Diag852Collector* dc)
    : _bt(bt), _s7Validator(s7Validator), _ts2ss(ts2ss), _validationObserver(dc)
  {
  }

  void onFaresUpdated();
  StatusS7Validation validate(const OptionalServicesInfo& s7, OCFees& ocFees) override;
  StatusS7Validation revalidate(const OptionalServicesInfo& s7, OCFees& ocFees, const bool fcLevel);

protected:
  class Context
  {
  public:
    Context(const OptionalServicesInfo& s7, OCFees& ocFees, DataHandle& dh)
      : _s7(s7), _ocFees(ocFees), _dh(dh) {}

    const std::vector<SvcFeesResBkgDesigInfo*>& getRbdInfos();

    const std::vector<SvcFeesCxrResultingFCLInfo*>& getFareClassInfos();

    OCFees& ocFees() { return _ocFees; }

    const OptionalServicesInfo& s7() const { return _s7; }

  private:
    const OptionalServicesInfo& _s7;
    OCFees& _ocFees;
    DataHandle& _dh;
    const std::vector<SvcFeesResBkgDesigInfo*>* _rbdInfos = nullptr;
    const std::vector<SvcFeesCxrResultingFCLInfo*>* _fareClassInfos = nullptr;
  };

  void prepareFareInfo();
  void prepareFareInfoForRevalidation();
  void prepareBagTravelFares();

  CheckMask adjustMask(CheckMask mask, const Context& ctx) const;

  StatusS7Validation validateImpl(CheckMask mask, Context& ctx);
  StatusS7Validation validateA(CheckMask mask, Context& ctx) const;
  StatusS7Validation validateS(CheckMask mask, Context& ctx) const;
  StatusS7Validation validateJ(CheckMask mask, Context& ctx) const;
  StatusS7Validation validateM(CheckMask mask, Context& ctx) const;
  StatusS7Validation validateEmpty(CheckMask mask, Context& ctx) const;
  StatusS7Validation
  validateSegment(const SegAndFare& seg, const CheckMask mask, Context& ctx) const;
  StatusS7Validation
  validateSegmentImpl(const SegAndFare& seg, const CheckMask mask, Context& ctx) const;

  bool checkCabin(const SegAndFare& seg, const Indicator s7Cabin) const;
  bool checkRbd(const SegAndFare& seg, Context& ctx) const;
  bool checkT186(const SegAndFare& seg, Context& ctx) const;
  bool checkFareClass(const SegAndFare& seg, Context& ctx) const;
  bool checkTktDesig(const SegAndFare& seg, Context& ctx) const;
  bool checkRuleTariff(const SegAndFare& seg, Context& ctx) const;
  bool checkRule(const SegAndFare& seg, Context& ctx) const;
  bool checkFareInd(const SegAndFare& seg, const Indicator fareInd) const;

  static const CheckMask ALL_CHECKS;
  static const CheckMask NON_FARE_CHECKS;
  static const CheckMask FARE_CHECKS;
  static const CheckMask REVALIDATION_CHECKS;
  static const CheckMask FC_SKIPPED_CHECKS;

  const BaggageTravel& _bt;
  const IS7RecordFieldsValidator& _s7Validator;
  const Ts2ss& _ts2ss;
  BTAValidationProcessObserver _validationObserver;

  SegAndFare _mostSignificantSeg;
  SegAndFare _specialSegForT186;
  std::vector<SegAndFare> _bagTravelSegs;
  std::vector<SegAndFare> _bagTravelFares;
  std::vector<SegAndFare> _journeySegs;
  bool _isFareInfoInitialized = false;
  bool _allFaresOnBtKnown = true;
  bool _allFaresOnJnyKnown = true;
};
}
