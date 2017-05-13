//-------------------------------------------------------------------
//
//  File:        Field.cpp
//  Created:     July 14, 2005
//  Authors:     Mike Carroll
//  Description: Basic object for display of name, name value, or value
//
//  Updates:
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

#include "FareDisplay/Templates/Field.h"

namespace tse
{
template <class T>
void
Field::getData(T& anInstance, PFI func)
{
  intValue = (anInstance.*func)();
}

template <class T>
void
Field::getData(T& anInstance, PFS func)
{
  strValue = (anInstance.*func)();
}

template <class T>
void
Field::getData(T& anInstance, PFMoneyAmount func)
{
  moneyValue = (anInstance.*func)();
}

void
Field::render(std::ostringstream* oss, const FieldValueType& valueType)
{
  if (oss == nullptr)
    return;

  // Zero based
  oss->seekp(_valuePosition - 1, std::ios_base::beg);

  if (_justify == JustificationType(LEFT))
    oss->setf(std::ios::left, std::ios::adjustfield);
  else
    oss->setf(std::ios::right, std::ios::adjustfield);

  *oss << std::setfill(' ') << std::setw(_valueFieldSize);

  switch (valueType)
  {
  case BOOL_VALUE:
    *oss << _boolValue;
    break;
  case INT_VALUE:
    *oss << _intValue;
    break;
  case STRING_VALUE:
    *oss << _strValue.substr(0, _valueFieldSize);
    break;
  case MONEY_AMOUNT_VALUE:
    *oss << _moneyValue;
    break;
  default:
    break;
  }
}

void
Field::renderLabel(std::ostringstream* oss)
{
  if (oss == nullptr)
    return;

  // Zero based
  oss->seekp(_valuePosition - 1, std::ios_base::beg);
  oss->setf(std::ios::left, std::ios::adjustfield);
  *oss << _strValue;
}
}
