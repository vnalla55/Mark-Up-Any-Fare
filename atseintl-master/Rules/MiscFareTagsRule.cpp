//-------------------------------------------------------------------
//
//  File:        MiscFareTagsRule.cpp
//  Created:     April 22, 2005
//  Authors:     Vladimir Koliasnikov
//
//  Description:
//
//
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
//------------------------------------------------------------------

#include "Rules/MiscFareTagsRule.h"

#include "Common/LocUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseStringTypes.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MiscFareTag.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag323Collector.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

#include <string>

using namespace std;

namespace tse
{
MiscFareTagsRule::MiscFareTagsRule() {}

MiscFareTagsRule::~MiscFareTagsRule() {}

//----------------------------------------------------------------------------
// validatePublished()     Fare preconstruction process
//----------------------------------------------------------------------------
Record3ReturnTypes
MiscFareTagsRule::validatePublished(PricingTrx& trx,
                                    PaxTypeFare& paxTypeFare,
                                    MiscFareTag* miscFareTag)
{
  Diag323Collector* diag = nullptr;
  bool dg = false;

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic323))
  {
    const string& diagFareClass = trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);
    if (diagFareClass.empty() ||
        RuleUtil::matchFareClass(diagFareClass.c_str(), paxTypeFare.fareClass().c_str()))
    {
      // Display a surcharge rule data
      DCFactory* factory = DCFactory::instance();
      diag = dynamic_cast<Diag323Collector*>(factory->create(trx));
      diag->enable(Diagnostic323);
      dg = true;
      diag->diag323Collector(paxTypeFare, miscFareTag);
    }
  }

  if (miscFareTag->unavailtag() == RuleConst::DATA_UNAVAILABLE)
  {
    if (dg && diag->isActive())
    {
      *diag << "    MISCELLANEOUS FARE TAGS - FAIL:" << endl;
      *diag << "                              DATA IS UNAVAILABLE" << endl;
      diag->flushMsg();
    }
    return FAIL;
  }
  else if (miscFareTag->unavailtag() == RuleConst::TEXT_ONLY)
  {
    if (dg && diag->isActive())
    {
      *diag << "    MISCELLANEOUS FARE TAGS - SKIP:" << endl;
      *diag << "                              TEXT ONLY" << endl;
      diag->flushMsg();
    }
    return SKIP;
  }

  // Note: the current fare should be always pulished at this point!
  //
  if (!paxTypeFare.isConstructed() && (miscFareTag->constInd() == RuleConst::MUST_BE_USED))
  {
    if (dg && diag->isActive())
    {
      *diag << "    MISCELLANEOUS FARE TAGS - FAIL:" << endl;
      *diag << "                              PUBLISHED FARE" << endl;
      diag->flushMsg();
    }
    paxTypeFare.fare()->setCat23PublishedFail();
    return FAIL;
  }

  // Do not use fare with a "prorate" indicator for the pricing/fare display
  if (miscFareTag->prorateInd() == RuleConst::MUST_BE_USED)
  {
    if (dg && diag->isActive())
    {
      *diag << "    MISCELLANEOUS FARE TAGS - FAIL:" << endl;
      *diag << "                              PRORATE " << endl;
      diag->flushMsg();
    }
    return FAIL;
  }
  /* do not check 'proportional' indicator in this category
     this condition will be checked in the combinability.
    if (miscFareTag->proportionalInd() == RuleConst::PROPORTIONAL)
    {
        if (dg && diag->isActive())
        {
          *diag << "    MISCELLANEOUS FARE TAGS - FAIL:" << endl;
          *diag << "                              PROPORTIONAL " << endl;
          diag -> flushMsg();
        }
        return FAIL;
    }
  */

  // save MiscFareTag pointer for the differential process
  paxTypeFare.miscFareTag() = miscFareTag;

  if (dg && diag->isActive())
  {
    *diag << "    MISCELLANEOUS FARE TAGS - PASS" << endl;
    diag->flushMsg();
  }
  return PASS;
}

//----------------------------------------------------------------------------
// validate()     Fare component/ Pricing Unit scope
//----------------------------------------------------------------------------
Record3ReturnTypes
MiscFareTagsRule::validate(PricingTrx& trx, PaxTypeFare& paxTypeFare, MiscFareTag* miscFareTag)
{
  Diag323Collector* diag = nullptr;
  bool dg = false;

  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic323))
  {
    const string& diagFareClass = trx.diagnostic().diagParamMapItem(Diagnostic::FARE_CLASS_CODE);
    if (diagFareClass.empty() ||
        RuleUtil::matchFareClass(diagFareClass.c_str(), paxTypeFare.fareClass().c_str()))
    {
      // Display a surcharge rule data
      DCFactory* factory = DCFactory::instance();
      diag = dynamic_cast<Diag323Collector*>(factory->create(trx));
      diag->enable(Diagnostic323);
      dg = true;
      diag->diag323Collector(paxTypeFare, miscFareTag);
    }
  }

  if (UNLIKELY(miscFareTag->unavailtag() == RuleConst::DATA_UNAVAILABLE))
  {
    if (dg && diag->isActive())
    {
      *diag << "    MISCELLANEOUS FARE TAGS - FAIL:" << endl;
      *diag << "                              DATA IS UNAVAILABLE" << endl;
      diag->flushMsg();
    }
    return FAIL;
  }
  else if (miscFareTag->unavailtag() == RuleConst::TEXT_ONLY)
  {
    if (UNLIKELY(dg && diag->isActive()))
    {
      *diag << "    MISCELLANEOUS FARE TAGS - SKIP:" << endl;
      *diag << "                              TEXT ONLY" << endl;
      diag->flushMsg();
    }
    return SKIP;
  }

  // cat23 does not have any qualified categories.
  // set of rec2's contains only "THEN"
  // (with a directionality = ' ','3','4')
  //

  if ((paxTypeFare.isConstructed() && (miscFareTag->constInd() == RuleConst::MAY_NOT_BE_USED)) ||
      (!paxTypeFare.isConstructed() && (miscFareTag->constInd() == RuleConst::MUST_BE_USED)))
  {
    if (UNLIKELY(dg && diag->isActive()))
    {
      *diag << "    MISCELLANEOUS FARE TAGS - FAIL:" << endl;
      *diag << "                              CONSTRUCTED FARE" << endl;
      diag->flushMsg();
    }
    return FAIL;
  }
  // Do not use fare with a "prorate" indicator for the pricing/fare display
  if (UNLIKELY(miscFareTag->prorateInd() == RuleConst::MUST_BE_USED))
  {
    if (dg && diag->isActive())
    {
      *diag << "    MISCELLANEOUS FARE TAGS - FAIL:" << endl;
      *diag << "                              PRORATE " << endl;
      diag->flushMsg();
    }
    return FAIL;
  }
  /* do not check 'proportional' indicator in this category
     this condition will be checked in the combinability.
      if (miscFareTag->proportionalInd() == RuleConst::PROPORTIONAL)
      {
        if (dg && diag->isActive())
        {
          *diag << "    MISCELLANEOUS FARE TAGS - FAIL:" << endl;
          *diag << "                              PROPORTIONAL " << endl;
          diag -> flushMsg();
        }
        return FAIL;
      }  */

  // save MiscFareTag pointer for the differential process
  paxTypeFare.miscFareTag() = miscFareTag;

  if (UNLIKELY(dg && diag->isActive()))
  {
    *diag << "    MISCELLANEOUS FARE TAGS - PASS" << endl;
    diag->flushMsg();
  }
  return PASS;
}
}
