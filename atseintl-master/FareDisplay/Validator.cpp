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

#include "FareDisplay/Validator.h"

#include "DBAccess/FareDisplayInclCd.h"

namespace tse
{
const Indicator Validator::INCL_AND = 'A';
const Indicator Validator::INCL_OR = 'O';
const Indicator Validator::INCL_YES = 'Y';
const Indicator Validator::INCL_NO = 'N';
const Indicator Validator::FARE_NOT_TO_BE_DISPLAYED = 'Z';
const Indicator Validator::CHILD_IND = 'C';
const Indicator Validator::INFANT_IND = 'I';
const Indicator Validator::TRUE_IND = 'T';

Validator::Restriction
Validator::restriction(const PaxTypeFare& fare) const
{
  if (_fareDisplayInclCd)
  {
    Indicator fTpT = _fareDisplayInclCd->fareTypeAndOrPsgType();
    Indicator dTpT = _fareDisplayInclCd->displTypeAndOrPsgType();
    Indicator dTfT = _fareDisplayInclCd->displTypeAndOrFareType();

    if ((fTpT != Validator::INCL_AND && dTpT != Validator::INCL_AND &&
         dTfT != Validator::INCL_AND) &&
        (fTpT == Validator::INCL_OR || dTpT == Validator::INCL_OR || dTfT == Validator::INCL_OR))
      return Validator::OR;

    else if (fTpT != Validator::INCL_OR && dTpT != Validator::INCL_OR && dTfT != Validator::INCL_OR)
      return Validator::AND;
    else
      return Validator::ALL;
  }
  else
  {
    return NONE;
  }
}

bool
Validator::validateMixedRestriction(bool& invalidDT, bool& invalidFT, bool& invalidPT)
{
  if (invalidDT && invalidFT && invalidPT)
    return false;

  if (_fareDisplayInclCd)
  {
    Indicator fTpT = _fareDisplayInclCd->fareTypeAndOrPsgType();
    Indicator dTpT = _fareDisplayInclCd->displTypeAndOrPsgType();
    Indicator dTfT = _fareDisplayInclCd->displTypeAndOrFareType();

    if (fTpT == dTpT)
    {
      if (dTfT == Validator::INCL_AND)
      {
        // (dt AND ft) OR pt
        if ((invalidDT || invalidFT) && invalidPT)
          return false;
      }
      else
      {
        // (dt OR ft) AND pt
        if ((invalidDT && invalidFT) || invalidPT)
          return false;
      }
    }

    else if (fTpT == dTfT)
    {
      if (dTpT == Validator::INCL_AND)
      {
        // (dt AND pt) OR ft
        if ((invalidDT || invalidPT) && invalidFT)
          return false;
      }
      else
      {
        // (dt OR pt) AND ft
        if ((invalidDT && invalidPT) || invalidFT)
          return false;
      }
    }

    else if (dTfT == dTpT)
    {
      if (fTpT == Validator::INCL_AND)
      {
        // dt OR (ft AND pt)
        if ((invalidFT || invalidPT) && invalidDT)
          return false;
      }
      else
      {
        // dt AND (ft OR pt)
        if ((invalidFT && invalidPT) || invalidDT)
          return false;
      }
    }
  }
  return true;
}
}
