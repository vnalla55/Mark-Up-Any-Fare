//-------------------------------------------------------------------
//
//  File:        InOutIndFilter.cpp
//  Created:     September 28, 2006
//  Authors:     Svetlana Tsarkova
//
//  Description: This class will display Inbound/Outbound Indicator
//               infront of Seasons information. The indicator resides in
//               SeasonsInfo object.
//
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

// InOutIndFilter.cpp: implementation of the InOutIndFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "FareDisplay/Templates/InOutIndFilter.h"

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/SeasonsInfo.h"
#include "FareDisplay/Templates/ElementField.h"
#include "FareDisplay/Templates/Field.h"

#include <string>
#include <sstream>
#include <vector>

namespace tse
{
namespace
{
const std::string INDICATOR_SPACE = " ";
}

namespace InOutIndFilter
{
void
formatData(FareDisplayInfo& fareDisplayInfo,
           Field& field1,
           std::vector<ElementField>& fields,
           const Indicator dateFormat)

{
  ElementField tempField;

  if (fareDisplayInfo.seasons().empty())
  {
    field1.strValue() = INDICATOR_SPACE;
    tempField.strValue() = INDICATOR_SPACE;
    fields.push_back(tempField);
    return;
  }

  std::vector<SeasonsInfo*>::const_reverse_iterator seasonsInfoI =
      fareDisplayInfo.seasons().rbegin();
  std::vector<SeasonsInfo*>::const_reverse_iterator seasonsInfoEnd =
      fareDisplayInfo.seasons().rend();
  for (int fieldCnt = 0; seasonsInfoI != seasonsInfoEnd; seasonsInfoI++, fieldCnt++)
  {
    std::ostringstream oss;
    oss << (*seasonsInfoI)->direction();

    if (!oss.str().empty())
    {
      if (fieldCnt == 0)
      {
        field1.strValue() = oss.str();
      }

      tempField.strValue() = oss.str();
      fields.push_back(tempField);
    }
  }
  if (fields.empty())
  {
    field1.strValue() = INDICATOR_SPACE;
    tempField.strValue() = INDICATOR_SPACE;
    fields.push_back(tempField);
    return;
  }
}
}
} // tse namespace
