//-------------------------------------------------------------------
//
//  File:        MinMaxFilter.h
//  Authors:     LeAnn Perez
//  Created:     February 14, 2005
//  Description: A class to handle the formatting
//               associated with the MIN-MAX column
//               on the Fare Quote display.
//
//  Updates:     Doug Batchelor
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

#include "FareDisplay/Templates/MinMaxFilter.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayInfo.h"
#include "FareDisplay/Templates/Field.h"

#include <sstream>
#include <string>

namespace tse
{
namespace
{
// this value defines the value of max stay units = Month
const std::string MAX_STAY_UNITS_MONTH = "M";
const std::string MIN_MAX_STAY_NP = "NP/NP";

Logger
logger("atseintl.FareDisplay.Templates.MinMaxFilter");
}

namespace MinMaxFilter
{
// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  MinMaxField::formatData()
//
// The main processing method.  Called to determine and
// format the Travel & Ticket info.
//
// @param   oss		- the output stream into which properly formatted
//                  data is injected.
// @param   fareDisplayInfo	- the object that contains all of the
//                            applicable data to be formatted.
//
// @param   numlines	- output, tells caller how many lines
//                      were formatted.
//
// @return  none
//
// </PRE>
// -------------------------------------------------------------------------
void
formatData(FareDisplayInfo& fareDisplayInfo, Field& field)
{
  std::ostringstream valueOne;

  LOG4CXX_DEBUG(logger, "In MinMaxField: InfoId = " << fareDisplayInfo.myObjId());

  // Check for NP condition (Not auto-Priceable)
  if (!fareDisplayInfo.isAutoPriceable())
  {
    field.strValue() = MIN_MAX_STAY_NP;
    return;
  }

  // ==============================
  //  Now apply formatting logic for the Minimum Stay part of the field.
  if (fareDisplayInfo.minStay() == "///") // Check for no minimum stay validation performed
  {
    valueOne << " -/";
    fareDisplayInfo.minStay() = "   ";
  }
  else if (fareDisplayInfo.minStay() == "   ") // Check for no minimum stay requirement
  {
    valueOne << " -/";
  }
  else if ((isalpha(fareDisplayInfo.minStay()[0])) || // check for day of week data
           (fareDisplayInfo.minStay()[0] == '$')) // check for cross-lorraine data
  {
    valueOne << fareDisplayInfo.minStay()[0] << fareDisplayInfo.minStay()[1] << '/';
  }
  else
  {
    if (fareDisplayInfo.minStay()[1] == '0')
    {
      valueOne << " " << fareDisplayInfo.minStay()[2] << '/';
    }
    else
    {
      //            valueOne << fareDisplayInfo.minStay()[1] << fareDisplayInfo.minStay()[2] << '/';
      if (fareDisplayInfo.minStay().size() == 1)
      {
        valueOne << " " << fareDisplayInfo.minStay() << '/';
      }
      else
      {
        valueOne << fareDisplayInfo.minStay()[1] << fareDisplayInfo.minStay()[2] << '/';
      }
    }
  }

  // ==============================
  //  Now apply formatting logic for the Maximum Stay part of the field.
  if (fareDisplayInfo.maxStay() == "///") // Check for no maximum stay validation performed
  {
    valueOne << "  -";
    fareDisplayInfo.maxStay() = "   ";
  }
  else if (fareDisplayInfo.maxStay()[0] == '$') // check for cross-lorraine data
  {
    valueOne << fareDisplayInfo.maxStay()[0] << fareDisplayInfo.maxStay()[1] << ' ';
  }
  else if (fareDisplayInfo.maxStayUnit() == "  ") // if maxStayUnit == 2 blank space
  {
    valueOne << "  -";
  }
  else if (fareDisplayInfo.maxStay() == "   ") // Check for no maximum stay requirement
  {
    valueOne << "  -";
  }
  /*
      else if( fareDisplayInfo.maxStay()[0] == '$' )        // check for cross-lorraine data
      {
          valueOne << fareDisplayInfo.maxStay()[0] << fareDisplayInfo.maxStay()[1] << ' ';
      }
  */
  else if (fareDisplayInfo.maxStayUnit() == MAX_STAY_UNITS_MONTH)
  {
    valueOne << ((fareDisplayInfo.maxStay()[1] != '0') ? fareDisplayInfo.maxStay()[1] : ' ');
    valueOne << fareDisplayInfo.maxStay()[2] << MAX_STAY_UNITS_MONTH[0];
  }
  else
  {
    if (fareDisplayInfo.maxStay()[0] == '0')
    {
      valueOne << ' ';
      if (fareDisplayInfo.maxStay()[1] == '0')
      {
        valueOne << ' ';
        //                if ( fareDisplayInfo.maxStay()[2] )
        if (fareDisplayInfo.maxStay()[2] == '0')
          valueOne << ' ';
        else
          valueOne << fareDisplayInfo.maxStay()[2];
      }
      else
        valueOne << fareDisplayInfo.maxStay()[1] << fareDisplayInfo.maxStay()[2];
    }
    else
      valueOne << fareDisplayInfo.maxStay();
  }

  if (!valueOne.str().empty())
    field.strValue() = valueOne.str();
}
}
} // tse namespace
