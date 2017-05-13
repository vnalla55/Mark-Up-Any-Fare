#include "Rules/FDPenalties.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"

namespace tse
{
static Logger
logger("atseintl.Rules.FDPenalties");

FDPenalties::FDPenalties() {}

FDPenalties::~FDPenalties() {}

//-------------------------------------------------------------------
//   @method validate
//
//   Description: Performs rule validations on a FareMarket.
//
//   @param PricingTrx           - Pricing transaction
//   @param Itin                 - itinerary
//   @param PaxTypeFare          - reference to Pax Type Fare
//   @param RuleItemInfo         - Record 2 Rule Item Segment Info
//   @param FareMarket           - Fare Market
//
//   @return Record3ReturnTypes - possible values are:
//                                NOTPROCESSED  = 1
//                                FAIL          = 2
//                                PASS          = 3
//                                SKIP          = 4
//                                STOP          = 5
//
//-------------------------------------------------------------------
Record3ReturnTypes
FDPenalties::validate(PricingTrx& trx,
                      Itin& itin,
                      const PaxTypeFare& paxTypeFare,
                      const RuleItemInfo* rule,
                      const FareMarket& fareMarket)
{
  LOG4CXX_INFO(logger, " Entered FDPenalties::validate()");

  //------------------------------------------------------------
  // Call base class function to validate
  //------------------------------------------------------------

  LOG4CXX_INFO(logger, " Calling Penalties::validate()");

  Record3ReturnTypes retVal = Penalties::validate(trx, itin, paxTypeFare, rule, fareMarket);

  if (retVal == SOFTPASS || retVal == PASS)
  {
    //---------------------------------------------------
    // Get a PenaltyInfo from the rule
    //---------------------------------------------------
    const PenaltyInfo* penaltiesRule = dynamic_cast<const PenaltyInfo*>(rule);

    if (!penaltiesRule)
    {
      LOG4CXX_DEBUG(logger, "Unable to get PenaltyInfo from the rule  - SKIP");
      LOG4CXX_INFO(logger, " Leaving FDPenalties::validate() - SKIP");

      return SKIP;
    }

    //--------------------------------------------------------------
    // Get a Fare Display Transaction from the Pricing Transaction
    //--------------------------------------------------------------
    FareDisplayUtil fdUtil;
    FareDisplayTrx* fdTrx;

    if (!fdUtil.getFareDisplayTrx(&trx, fdTrx))
    {
      LOG4CXX_DEBUG(logger, "Unable to get FareDisplayTrx - FAIL");
      LOG4CXX_INFO(logger, " Leaving FDPenalties::validate() - FAIL");

      return FAIL;
    }

    //----------------------------------------------------------------
    // Check if it is a request to exclude fares with Penalty or to
    // exclude fares with any restrictions
    //----------------------------------------------------------------
    FareDisplayOptions* fdOptions = fdTrx->getOptions();

    if (fdOptions && (fdOptions->isExcludePenaltyFares() || fdOptions->isExcludeRestrictedFares()))
    {
      // Check if a percentage was specified
      if (fdOptions->excludePercentagePenaltyFares() != 0)
      {
        // Fail fares with a penalty at or above a specified percentage
        if (penaltiesRule->penaltyPercent() >=
            static_cast<Percent>(fdOptions->excludePercentagePenaltyFares()))
        {
          LOG4CXX_DEBUG(logger, "Fare with Penalty at or above a specified percentage - FAIL");
          LOG4CXX_INFO(logger, " Leaving FDPenalties::validate() - FAIL");

          return FAIL;
        }
        else
        {
          LOG4CXX_DEBUG(logger, "Fare with Penalty bellow a specified percentage - SKIP");
          LOG4CXX_INFO(logger, " Leaving FDPenalties::validate() - SKIP");

          return SKIP;
        }
      }

      // Percentage not specified
      else
      {
        // Check if there's a "real" Penalty
        if (penaltiesRule->noRefundInd() != BLANK ||
            ((penaltiesRule->penaltyCancel() == APPLIES ||
              penaltiesRule->penaltyFail() == APPLIES ||
              penaltiesRule->penaltyReissue() == APPLIES ||
              penaltiesRule->penaltyNoReissue() == APPLIES ||
              penaltiesRule->penaltyRefund() == APPLIES || penaltiesRule->penaltyPta() == APPLIES ||
              (penaltiesRule->cancelRefundAppl() == APPLIES &&
               penaltiesRule->penaltyExchange() == BLANK)) &&
             (penaltiesRule->penaltyAmt1() > 0 || penaltiesRule->penaltyAmt2() > 0 ||
              penaltiesRule->penaltyPercent() > 0)))
        {
          LOG4CXX_DEBUG(logger, "Fare with Penalty restriction - FAIL");
          LOG4CXX_INFO(logger, " Leaving FDPenalties::validate() - FAIL");

          return FAIL;
        }

        // No "real" penalty
        else
        {
          LOG4CXX_DEBUG(logger, "No Penalty restriction - SKIP");
          LOG4CXX_INFO(logger, " Leaving FDPenalties::validate() - SKIP");

          return SKIP;
        }
      }
    }
  }

  // At this point return SKIP because we should have not validated Penalties
  LOG4CXX_INFO(logger, " Leaving FDPenalties::validate() - SKIP");

  return SKIP;
}
}
