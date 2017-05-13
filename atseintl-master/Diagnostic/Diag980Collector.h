//-----------------------------------------------------------------------------
//
//  File:     Diag980Collector.h
//
//  Author :  Kul Shekhar
//
//  Copyright Sabre 2005
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Diagnostic/DiagCollector.h"

#include <map>

namespace tse
{
class Diag980Collector : public DiagCollector
{
public:
  explicit Diag980Collector(Diagnostic& root) : DiagCollector(root), _showFareDetails(false) {}
  Diag980Collector() : _showFareDetails(false) {}

  Diag980Collector& operator<<(const PricingTrx& trx) override;

protected:
  void printItin(const PricingTrx& trx, const FarePath* fp);

private:
  void activation(const PricingTrx& trx);
  void showOptions(const PricingTrx& trx);
  void displayOptions(const PricingTrx& trx);

  FareUsage* getFareUsage(const FarePath* fpath, TravelSeg* tvlSeg, uint16_t& tvlSegIndex);
  bool itinValidOption(const Itin& itin);
  void amounts(const PricingTrx& trx, const Itin* itin);
  void aseAmounts(const PricingTrx& trx, const FarePath* fpath);
  MoneyAmount amount(const PricingTrx& trx, const Itin* itin);
  bool journeys(const PricingTrx& trx, const Itin& itin);
  void availability(const PricingTrx& trx, const FarePath* fpath);
  void thruAvailability(const PricingTrx& trx, const FarePath* fpath);
  bool partOfJourney(const TravelSeg* tvlSeg,
                     const Itin& itin,
                     const PricingTrx& trx,
                     std::vector<FareMarket*>& journeysForShoppingItin);
  bool isValidJourney(const PricingTrx& trx,
                      const FareMarket* fm,
                      std::vector<FareMarket*>& journeysForShoppingItin);
  void marriage(const PricingTrx& trx, const AirSeg* airSeg, const BookingCode& bc);
  DifferentialData* differentialData(const FareUsage* fu, const TravelSeg* tvlSeg);
  const PaxTypeFare::SegmentStatus&
  diffSegStatus(const DifferentialData* diff, const TravelSeg* tvlSeg);
  int getMileage(const FarePath& farePath) const;
  void displayAltDateOptions(const PricingTrx& trx,
                             std::multimap<MoneyAmount, std::vector<FarePath*> >& solutionMap);
  void printItinNum(const Itin&);
  void printFareInfo(const PaxTypeFare* const fare, const bool hasNoDifferentials);
  void printFarePath(const FarePath* const fpath);

  bool _showFareDetails; // show fare path and a few fare details when DDFARES argument is given to the diagnostics
};

} // namespace tse

