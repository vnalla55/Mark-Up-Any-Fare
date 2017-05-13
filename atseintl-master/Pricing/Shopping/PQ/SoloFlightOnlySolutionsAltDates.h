// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
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

#include "Common/ValidatingCarrierUpdater.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/PQ/SoloFlightOnlySolutionsDataTypes.h"

namespace tse
{

class Diag910Collector;
}

namespace tse
{
namespace fos
{

class SoloFlightOnlySolutionsSopCollector;
class Validator;

class SoloFlightOnlySolutionsAltDates
{
  enum SolutionType
  {
    ONLINE,
    INTERLINE,
    ANYSOLUTION,
    OWSOLUTION
  };

public:
  SoloFlightOnlySolutionsAltDates(ShoppingTrx& trx);
  ~SoloFlightOnlySolutionsAltDates();

  void process();

private:
  bool isSolutionExists(SopsCombination& sops);
  SopsCombination getActualSops(Sops& candidate);
  bool addFOSCandidate(GroupedSolutions& solutions, SopsCombination& sops, bool online);
  void addFOS(SolutionsContainer::const_iterator itBegin, SolutionsContainer::const_iterator itEnd);
  void removeEmptyCarriers(SopsByLegByCxrByDate& cxrSopsCollection);

  void generateSingleLegCandidate(SOPCollections& allSopsCollection,
                                  SolutionsByCarrierByDatePair& interlineSolutions);

  void generateCandidates(SoloFlightOnlySolutionsSopCollector& collector,
                          SolutionsByCarrierByDatePair& onlineSolutions,
                          SolutionsByCarrierByDatePair& interlineSolutions);

  uint32_t generateOnlineSolutions(const CarrierCode& carrierCode,
                                   GroupedSolutions& solutions,
                                   const SopsByLeg& sopsByLeg,
                                   SOPCollections& sopsCollection,
                                   const uint32_t numOfSolutionsToGenerate);

  uint32_t generateInterlineSolutions(const CarrierCode& carrierCode,
                                      GroupedSolutions& solutions,
                                      const SopsByLeg& sopsByLeg,
                                      SOPCollections& sopsCollection,
                                      const uint32_t numOfSolutionsToGenerate);

  void collectSolutions(SolutionsByCarrierByDatePair& onlineSolutions,
                        SolutionsByCarrierByDatePair& interlineSolutions);

  // Diagnostic
  void printAddFOS(const DatePair& datePair,
                   size_t solutionsNeeded,
                   SolutionType type,
                   size_t found,
                   SolutionsContainer::const_iterator itBegin,
                   SolutionsContainer::const_iterator itEnd,
                   const CarrierCode& govCxr = "");

private:
  ShoppingTrx& _trx;

  Diag910Collector* _dc;

  Validator* _validator;
  ValidatingCarrierUpdater _validatingCarrierUpdater;
  std::set<DatePair> _datePairSet;
  uint16_t _numOfSolutionsNeeded;
  ShoppingTrx::FlightMatrix _totalMatrix;
};
}
} // end namespace tse::fos

