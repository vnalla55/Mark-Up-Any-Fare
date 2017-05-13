//----------------------------------------------------------------------------
//  File:           RoundTripFareSelection.h
//  Created:        10/12/04
//  Authors:        Quan Ta
//
//  Description:    A class to capture the round trip fare selection process.
//
//
//  Updates:
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein is the
//          property of Sabre.
//
//          The program(s) may be used and/or copied only with the
//          written permission of Sabre or in accordance with the
//          terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "MinFares/MinFareFareSelection.h"

#include <vector>

namespace tse
{
class DiagCollector;
class FareMarket;
class FarePath;
class MinFareAppl;
class MinFareDefaultLogic;
class PaxType;
class PaxTypeFare;
class PricingTrx;
class PricingUnit;
class RepricingTrx;
class TravelSeg;

class RoundTripFareSelection
{
  friend class RoundTripFareSelectionDerived;

public:
  RoundTripFareSelection(MinimumFareModule module,
                         DiagCollector* diag,
                         PricingTrx& trx,
                         const FarePath& farePath,
                         const PricingUnit& pu,
                         const std::vector<TravelSeg*>& obTvlSegs,
                         const std::vector<TravelSeg*>& ibTvlSegs,
                         CabinType lowestCabin,
                         const PaxType* paxType,
                         const DateTime& travelDate,
                         const PaxTypeFare* obThruFare = nullptr,
                         const PaxTypeFare* ibThruFare = nullptr,
                         const MinFareAppl* minFareAppl = nullptr,
                         const MinFareDefaultLogic* minFareDefLogic = nullptr,
                         const RepricingTrx* obRepricingTrx = nullptr,
                         const RepricingTrx* ibRepricingTrx = nullptr,
                         FMDirection selectFareForDirection = FMDirection::UNKNOWN,
                         FMDirection selectfareUsageDirection = FMDirection::UNKNOWN);

  virtual ~RoundTripFareSelection();

  RoundTripFareSelection(const RoundTripFareSelection&) = delete;
  RoundTripFareSelection& operator=(const RoundTripFareSelection&) = delete;

  const std::vector<const PaxTypeFare*>& roundTripFare() const { return _roundTripFare; }

  /**
   * Whether the fare is constructed.
   **/
  bool constructedFare() const { return _constructedFare; }
  MoneyAmount constructedFareAmt() const { return _constFareAmt; }

private:
  MinimumFareModule _module;
  DiagCollector* _diag;
  PricingTrx& _trx;
  const FarePath& _farePath;
  const PricingUnit& _pu;
  const std::vector<TravelSeg*>& _obTvlSegs;
  const std::vector<TravelSeg*>& _ibTvlSegs;
  CabinType _lowestCabin;
  const PaxType* _paxType;
  DateTime _travelDate;
  const PaxTypeFare* _obThruFare;
  const PaxTypeFare* _ibThruFare;
  const MinFareAppl* _minFareAppl;
  const MinFareDefaultLogic* _minFareDefLogic;
  const RepricingTrx* _obRepricingTrx;
  const RepricingTrx* _ibRepricingTrx;
  FMDirection _selectFareForDirection;
  FMDirection _selectfareUsageDirection;

  std::vector<const PaxTypeFare*> _roundTripFare;

  bool _constructedFare = false;
  MoneyAmount _constFareAmt = 0.0;
  const LocCode* _constructionPoint = nullptr;

  const PaxTypeFare* _obFare = nullptr;
  const PaxTypeFare* _ibFare = nullptr;

  GlobalDirection _obGi;
  GlobalDirection _ibGi;

  bool _status = false;

  void reuseThruFare();

  virtual bool isThruFareValidForReUse(const PaxTypeFare& paxTypeFare);

  /**
   * The main driver for the fare selection process.
   **/
  void processRtFareSelection();

  /**
   * Typeinfo continuation passing for fare selectors
   **/
  template <class ObFareSelector>
  void doSelectFares(ObFareSelector& obFareSelector);

  template <class ObFareSelector, class IbFareSelector>
  void doSelectFares(ObFareSelector& obFareSelector, IbFareSelector& ibFareSelector);

  template <class ObFareSelector, class IbFareSelector>
  void selectFareSameGI(ObFareSelector& obFareSel, IbFareSelector& ibFareSel);

  template <class ObFareSelector, class IbFareSelector>
  void selectFareSameGI(ObFareSelector& obFareSel,
                        IbFareSelector& ibFareSel,
                        PaxTypeStatus paxTypeStatus);

  template <class ObFareSelector, class IbFareSelector>
  void selectFareDiffGI(ObFareSelector& obFareSel, IbFareSelector& ibFareSel);

  template <class ObFareSelector, class IbFareSelector>
  void selectFareDiffGI(ObFareSelector& obFareSel,
                        IbFareSelector& ibFareSel,
                        PaxTypeStatus paxTypeStatus);
};
} // end of tse namespace
