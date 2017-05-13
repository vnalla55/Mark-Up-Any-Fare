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

#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/FPCpuTimeMeter.h"
#include "Pricing/Shopping/PQ/AltDatesTaxes.h"

#include <cmath>
#include <set>
#include <sstream>

namespace tse
{
class FarePath;
class GroupFarePath;
class InterlineTicketCarrier;
class Itin;
class ItinStatistic;
class PaxTypeFare;
struct SOPInfo;

class Diag941Collector : public DiagCollector
{
  friend class Diag941CollectorTest;

public:
  /**
   * Combination validation result codes are sorted by importance (using descending order)
   * to handle a case, when the same combination is reported twice using
   * different code.
   *
   * When that happens (e.g. DELAYED_FLIGHT_BIT vs DIVERSITY),
   * the code, which is more important to know (and is less by value)
   * will be displayed in validation matrix.
   */
  enum CombinationResult
  { EXISTS,
    DELAYED_FLIGHT_BIT,
    FBR_SAME_CARRIER,
    FBR_USE_CARRIER_IN_TABLE,
    CAT35_VALIDATING_CARRIER,
    RULE_CONTROLLER,
    VALIDATE_TICKETING_INTERLINE,
    PASSED,
    MCT,
    INTERLINE_ONLY,
    ALT_DATES,
    CUSTOM,
    LONG_CONNECTION,
    SRL,
    DIVERSITY,
    SMP,
    NOT_CONSIDERED,
    COMBINATION_RESULT_LAST };

  enum NonStopAction
  {
    ADD_NS,
    ADD_ANS,
    SWAPPER_ADD_ANS,
    SWAPPER_REM_NS,
    SWAPPER_REM_ANS
  };

  typedef ShoppingTrx::FlightMatrix::value_type Solution;

  Diag941Collector()
    : _fpKey(0),
      _fpTotalCombNum(0),
      _dumpVITA(false),
      _diagFlights(false),
      _diagNonStops(false),
      _adOptIdxNum(0),
      _diagDirectFlightsOnly(false),
      _altDatesDetails(false),
      _fpCtx(this)
  {
  }

  void printHeader() override;
  virtual void initParam(Diagnostic& root) override;

  /**
   * Print fare path and validation details
   */
  void printFarePath(const FarePath* fp, size_t fpKey);
  void printCarrierLine(size_t puIdx, size_t fuIdx, const PaxTypeFare* ptf);
  void printCarrierTotals(const std::vector<uint32_t>& carriers);
  void printSOPLine(size_t puIdx, size_t fuIdx, const PaxTypeFare* ptf);
  void printSOPTotalsPerLeg(size_t legIndex, const std::vector<SOPInfo>& sopInfoVec);
  void setFPTotalCombNum(std::size_t num) { _fpTotalCombNum = num; }
  void addCombinationResult(const SopIdVec& oSopVec, CombinationResult result, bool isLongConnectFlag = false);
  void addMaximumPenaltyFailedFarePath(const FarePath& farePath);
  void addSopResult(std::size_t legId, int sopId, CombinationResult result, bool isLongConnectFlag = false);
  void printDelayedFlightBitValidationResult(FarePath* fp, const std::vector<uint32_t>& carrierKey);

  void printCombinationMatrix();
  void printVITA(const SopIdVec& sopVec,
                 InterlineTicketCarrier& interlineTicketCarrierData,
                 const Itin& itin,
                 bool validationResult,
                 const std::string& validationMessage = std::string());
  void printVITAPriceInterlineNotActivated();
  void addOptPriceAdjustedByRC(const SopIdVec& oSopVec,
                               const FarePath& newFP,
                               const FarePath& origFP);
  // print & flush remembered options list
  void printOptsPriceAdjustedByRC();
  void printFPFailedMessage(const char* msg);
  void printFPTooManyFailedCombMessage(const char* action);
  void printFPSolutionsFound(std::size_t numSolutionsFound, std::size_t numSolutionsInFlightMatrix);
  void logFPDiversityCpuMetrics(double wallTime, double userTime, double sysTime);
  void printFPCustomSolutionsFound(std::size_t numCustomSolutionsFound);
  void flushFarePath();
  /*****************************************/

  void flushFPDiversityCpuMetrics();

  /**
   * Print AltDatesTaxes details
   */
  void printAltDatesDetails();
  void printAltDatesTaxHeader(size_t numSolutionPerDatePair);
  void printADSolution(const Solution& solution,
                       MoneyAmount totalPrice,
                       AltDatesTaxes::TaxComponentVec taxCompVec,
                       bool isKept);
  void printDurationFltBitmap(uint32_t carrierKey, PaxTypeFare* ptf);
  /*****************************************/

  /**
   * Print non-stop action
   */
  void printNonStopAction(NonStopAction action,
                          const SopIdVec& comb,
                          const ItinStatistic& stats);
  void flushNonStopActions();
  /*****************************************/
private:
  /**
   * Print fare path and validation details
   */
  void printPUFUInfo(size_t puIdx, size_t fuIdx, const PaxTypeFare* ptf);
  void printCombResult(const SopIdVec& oSopVec);

  bool isCarrierAgreementsPrinted(const CarrierCode& validatingCarrier) const;
  void printCarrierAgreements(InterlineTicketCarrier& interlineTicketCarrierData,
                              const CarrierCode& validatingCarrier);
  bool isDirectFlightsSolution(const SopIdVec& oSopVec) const;

  /**
   * Print AltDatesTaxes details
   */
  void printSolutionFlights(const Solution& solution, AltDatesTaxes::TaxComponentVec taxCompVec);
  void printFarePathData(const SopIdVec* fmv, const FarePath& path);
  void printGroupFarePath(const GroupFarePath& groupFarePath, const SopIdVec& sops);
  void printFarePath(const FarePath& farePath, const SopIdVec& sops);

  /**
   * Print non-stop action
   */
  void printNonStopsHeader();
  const char* getNSActionString(NonStopAction action);

private:
  struct MoneyAmountCompare
  {
    bool operator()(MoneyAmount lhs, MoneyAmount rhs) const
    {
      bool areEqual = std::fabs(lhs - rhs) < EPSILON;
      return !areEqual && (lhs < rhs);
    }
  };

  typedef std::vector<SopIdVec> SopIdVecs;
  using MoneyAmountToSopIdxVec = std::map<MoneyAmount, SopIdVecs, MoneyAmountCompare>;

  std::vector<std::set<int> > _sopsPerLeg;
  std::map<SopIdVec, CombinationResult> _combResults;
  std::vector<std::map<SopId, CombinationResult> > _sopResults;
  std::size_t _fpKey;
  std::size_t _fpTotalCombNum;
  FPCpuTimeMeter _fpDiversityCpuTimeMeter;
  std::set<CarrierCode> _printedCarrierAgreements;
  bool _dumpVITA;
  MoneyAmountToSopIdxVec _optsPriceAdjustedByRC;
  bool _diagFlights;
  bool _diagNonStops;
  int _adOptIdxNum;

  bool _diagDirectFlightsOnly;
  bool _altDatesDetails;

  // Print current fare path will be delayed to ostringstream, to filter by FP,
  // which have validation results for direct flights
  std::ostream* _fpCtx;
  std::ostringstream _fpCtxBuf;
  std::ostringstream _fpNSActionsBuf;
};

} /* namespace tse */
