//-------------------------------------------------------------------
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/QualifierPaxTypeValidator.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareDispCldInfPsgType.h"
#include "DBAccess/FareDisplayInclCd.h"
#include "DBAccess/PaxTypeInfo.h"

namespace tse
{

static Logger
logger("atseintl.FareDisplay.QualifierPaxTypeValidator");

using namespace std;

bool
QualifierPaxTypeValidator::validate(const PaxTypeFare& fare)
{
  LOG4CXX_DEBUG(logger, "Entered QualifierPaxTypeValidator::validate()");
  if (isMatchQualifier((fare.actualPaxType()->paxTypeInfo())))
  {
    if (fare.actualPaxType()->paxTypeInfo().isAdult())
    {

      return validateAdultFare(fare);
    }
    else
    {

      return validateChildInfantFare(fare);
    }
  }
  else
  {
    LOG4CXX_DEBUG(logger,
                  "Fare Failed to Validate  QualifierPaxTypeValidator::isMatchQualifier()");
    return false;
  }
}

bool
QualifierPaxTypeValidator::initialize(const FareDisplayTrx& trx)
{
  PaxTypeValidator::initialize(trx);

  if (_fareDisplayInclCd != nullptr)
  {
    if (trx.getOptions()->isChildFares())
    {

      const std::vector<FareDispCldInfPsgType*>& cldPsgTypes =
          trx.dataHandle().getFareDispCldInfPsgType(_fareDisplayInclCd->userApplType(),
                                                    _fareDisplayInclCd->userAppl(),
                                                    _fareDisplayInclCd->pseudoCityType(),
                                                    _fareDisplayInclCd->pseudoCity(),
                                                    _fareDisplayInclCd->inclusionCode(),
                                                    Validator::CHILD_IND);
      LOG4CXX_DEBUG(logger, "Size of CHILD psgTypes: " << cldPsgTypes.size());
      if (cldPsgTypes.size() != 0)
      {
        std::vector<FareDispCldInfPsgType*>::const_iterator iter = cldPsgTypes.begin();
        std::vector<FareDispCldInfPsgType*>::const_iterator iterEnd = cldPsgTypes.end();
        for (; iter != iterEnd; iter++)
        {
          _publishedChildInfant.insert((*iter)->psgType());
        }
      }
    }

    if (trx.getOptions()->isInfantFares())
    {

      const std::vector<FareDispCldInfPsgType*>& infPsgTypes =
          trx.dataHandle().getFareDispCldInfPsgType(_fareDisplayInclCd->userApplType(),
                                                    _fareDisplayInclCd->userAppl(),
                                                    _fareDisplayInclCd->pseudoCityType(),
                                                    _fareDisplayInclCd->pseudoCity(),
                                                    _fareDisplayInclCd->inclusionCode(),
                                                    Validator::INFANT_IND);
      LOG4CXX_DEBUG(logger, "Size of INFANT psgTypes: " << infPsgTypes.size());
      if (infPsgTypes.size() != 0)
      {
        std::vector<FareDispCldInfPsgType*>::const_iterator iter = infPsgTypes.begin();
        std::vector<FareDispCldInfPsgType*>::const_iterator iterEnd = infPsgTypes.end();
        for (; iter != iterEnd; iter++)
        {
          _publishedChildInfant.insert((*iter)->psgType());
        }
      }
    }
  }

  return true;
}

bool
QualifierPaxTypeValidator::isMatchQualifier(const PaxTypeInfo& fare) const
{
  LOG4CXX_DEBUG(logger, "VALIDATING PAX TYPE CODE");
  if (fare.isInfant())
    return _trx->getOptions()->isInfantFares();
  else if (fare.isChild())
    return _trx->getOptions()->isChildFares();
  else if (fare.isAdult())
    return _trx->getOptions()->isAdultFares();
  else
  {
    LOG4CXX_DEBUG(logger, "INVALID PAX TYPE IN C/I QUALIFIER REQUEST");
    return false;
  }
}

Validator::Restriction
QualifierPaxTypeValidator::restriction(const PaxTypeFare& fare) const
{
  if (!isMatchQualifier((fare.actualPaxType()->paxTypeInfo())))
  {
    return Validator::AND;
  }
  return Validator::restriction(fare);
}
}
