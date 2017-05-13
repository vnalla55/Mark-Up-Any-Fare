//----------------------------------------------------------------------------
//  Copyright Sabre 2004
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

#include "MinFares/BHCMinimumFare.h"
#include "MinFares/CTMMinimumFare.h"
#include "MinFares/DMCMinimumFare.h"
#include "MinFares/EPQMinimumFare.h"
#include "DataModel/ExchangePricingTrx.h"

namespace tse
{
class PricingTrx;
class FarePath;
class DiagCollector;
class PricingUnit;
class MinFareRuleLevelExcl;
class MinFareAppl;
class MinFareDefaultLogic;

typedef std::multimap<uint16_t, const MinFareRuleLevelExcl*> RuleLevelMap;
typedef std::multimap<uint16_t, const MinFareAppl*> ApplMap;
typedef std::map<uint16_t, const MinFareDefaultLogic*> DefaultLogicMap;

struct MinFareChecks
{
  MinFareChecks()
    : processOSC(false), processRSC(false), processCOM(false), processCOP(false), processDMC(false)
  {
  }

  bool processOSC; // require to do One Way Sub Journey Minimum Check
  bool processRSC; // require to do Round Trip Sub Journey Minimum Check
  bool processCOM; // require to do Country of Origin Minimum Check
  bool processCOP; // require to do Country of Payment Minimum Check
  bool processDMC; // require to do Directional Minimum Check

  RuleLevelMap ruleLevelMap;
  ApplMap applMap;
  DefaultLogicMap defaultLogicMap;
};

class MinFareChecker final
{
public:
  static constexpr int PRIORITY_LEVEL1 = 1;
  static constexpr int PRIORITY_LEVEL2 = 2;
  static constexpr int PRIORITY_LEVEL3 = 3;

  friend class MinFareCheckerTest;

  /**
   * This function is called to do Minimum Fare check on a fare path.
   *
   * @param trx An reference to the Trx.
   * @param farePath An reference to the Fare Path.
   * @param logger A reference to the Logger.
   *
   * @return bool true if the fare path passed the check, false if the fare
   * path failed the check.
   *
   * @throw tse::Exception exception thrown when there is critical error
   * preventing the Minimum Fare check to continue such as "MISSING DATA". The
   * caller should stop processing other fare paths within the itinerary.
   *
   \todo Flow of events :
   -  The Combinations calls MinFareChecker::process(...) for Minimum Fare check
   -  This function will selectively create the Minimum Fare Module objects based on PU Assignment
   logic.
   -  Perform HIP checks (Check with Pricing Unit Process if already called).
   -  Loop all PUs in Fare Path
   -  Determine Normal Fare and Oneway PU type
   -	Determine Domestic or International PU
   -	Perform BHC check for International PU
   -	Perform OSC check for both International and Domestic PU
   -	End of minimum fare check for Domestic PU
   -  Continue process for International PU
   -	Perform COM check
   -	Determine EPQ condition.
   -  If EPQ is not set then perform DMC check
   -  End of minimum fare check
   */
  bool process(PricingTrx& trx, FarePath& farePath);

private:
  void processMinFareCheckOnPU(PricingTrx& trx,
                               FarePath& farePath,
                               MinFareChecks& mfChecks,
                               MoneyAmount& totalPlusUp,
                               DiagCollector& diag,
                               int priority);

  void processMinFareCheckOnSideTripPU(PricingTrx& trx,
                                       FarePath& farePath,
                                       PricingUnit& pu,
                                       MinFareChecks& mfChecks,
                                       MoneyAmount& totalPlusUp,
                                       DiagCollector& diag,
                                       int priority);

  void processMinFareCheckOnPUCommon(PricingTrx& trx,
                                     FarePath& farePath,
                                     const std::vector<PricingUnit*>& puVec,
                                     MinFareChecks& mfChecks,
                                     MoneyAmount& totalPlusUp,
                                     DiagCollector& diag,
                                     int priority,
                                     bool checkOnSideTripPU);

  bool processHIPMinimumFare(PricingTrx& trx,
                             FarePath& farePath,
                             MinFareChecks& mfChecks,
                             MoneyAmount& totalPlusUp,
                             DiagCollector& diag);

  void processCTMMinimumFare(PricingTrx& trx,
                             FarePath& farePath,
                             PricingUnit& pu,
                             MinFareChecks& mfChecks,
                             MoneyAmount& totalPlusUp,
                             DiagCollector& diag);

  void processOJMMinimumFare(PricingTrx& trx,
                             FarePath& farePath,
                             PricingUnit& pu,
                             MoneyAmount& totalPlusUp,
                             DiagCollector& diag);

  void processRTMinimumFare(PricingTrx& trx,
                            FarePath& farePath,
                            PricingUnit& pu,
                            MoneyAmount& totalPlusUp,
                            DiagCollector& diag);

  void processCOPMinimumFare(PricingTrx& trx,
                             FarePath& farePath,
                             MoneyAmount& totalPlusUp,
                             DiagCollector& diag);

  void processCOMMinimumFare(PricingTrx& trx,
                             FarePath& farePath,
                             MinFareChecks& mfChecks,
                             MoneyAmount& totalPlusUp,
                             DiagCollector& diag);

  void processDMCMinimumFare(PricingTrx& trx,
                             FarePath& farePath,
                             MoneyAmount& totalPlusUp,
                             DiagCollector& diag);

  void setTotalPlusUp(FarePath& farePath, MoneyAmount totalPlusUp);

  /**
   * This function is called to process normal fare open jaw PU type
   *
   * @param trx a reference to Trx
   * @param farePath a reference to Fare Path
   * @param pu a reference to pricing unit
   * @param priority for each minimum fare module per Pricing Unit Type.
   * @param diag a reference to diagnostic.
   * @return plus up amount
   */
  MoneyAmount processNormalOpenJaw(
      PricingTrx& trx, FarePath& farePath, PricingUnit& pu, int priority, DiagCollector& diag);

  void checkExemption(PricingTrx& trx, FarePath& farePath);
  void checkExemption(PricingTrx& trx, PricingUnit& pu);
};
}
