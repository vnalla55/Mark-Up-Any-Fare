// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "DataModel/SoloFmPath.h"
#include "Pricing/Shopping/PQ/ItinStatistic.h"
#include "Pricing/Shopping/PQ/SoloSurcharges.h"

namespace tse
{

class DiversityModel;
class ItinStatistic;
class Logger;
class PricingOrchestrator;
class ShoppingTrx;
class SoloSurcharges;
class FareUsage;
}

namespace tse
{
namespace shpq
{

class SoloPQ;
class SoloTrxData;

class SoloOrchestrator
{
public:
  SoloOrchestrator(ShoppingTrx& trx, PricingOrchestrator& po);
  virtual ~SoloOrchestrator();

  void process();

private:
  SoloOrchestrator(const SoloOrchestrator& orig);

  void initializePQ(SoloPQ& pq,
                    DiversityModel* model,
                    const SoloFmPathPtr& soloFmPath,
                    SoloTrxData& soloTrxData) const;

  void expandPQDiag929(SoloPQ& pq, SoloTrxData& trxData, const size_t noOfExpansions) const;
  void generateDiagnostic923(const SoloSurcharges& soloSurcharges) const;
  void generateDiagnostic924(const SoloFmPathPtr& soloFmPath) const;
  void generateFlightOnlySolutions(ItinStatistic& stats, const bool pqConditionOverride = false);
  void generateBrandedFaresFlightOnlySolutions(ItinStatistic& stats);
  void generateRegularFlightOnlySolutions(ItinStatistic& stats, const bool pqConditionOverride);
  void sortCurrentOptionsAccordingtoIbf(ItinStatistic::CombinationWithStatusVec& combs);
  void updateBucketMatching(ItinStatistic& stats);
  void
  removeSuperfluousOptionsIfNeeded(ShoppingTrx::FlightMatrix& flightMatrix, ItinStatistic& stats);
  bool isAltDateTaxProcessingRequired();
  void removeUnwantedSolutions(DiversityModel* model, SoloSurcharges& soloSurcharges);
  void sortSolutions(SortedFlightsMap& flightsMap) const;
  void determineBaseFare(SortedFlightsMap& flightMap);
  void setSegmentStatuses(SortedFlightsMap& flightMap, ShoppingTrx& trx);
  void addMissingSegmentStatuses(FarePath* farePath);
  void moveShortcutPricingThreshold(SortedFlightsMap& flightMap);
  void setBreakAvailbility(FareUsage& fu);
  void subscribeTo(ItinStatistic& stats);

  ShoppingTrx& _trx;
  PricingOrchestrator& _po;
  static Logger _logger;
};
}
}

