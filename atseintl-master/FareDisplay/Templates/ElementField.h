//-------------------------------------------------------------------
//
//  File:        ElementField.h
//  Created:     July 10, 2005
//  Authors:     Mike Carroll
//  Description: Field class used for DB resident elements.  See
//               TemplateEnums.h for more information
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

#pragma once

#include <stdarg.h>
#include <sstream>
#include "FareDisplay/Templates/Field.h"
#include "FareDisplay/Templates/TemplateEnums.h"

namespace tse
{

class ElementField : public Field
{
public:
  void initialize(const FieldColumnElement& element,
                  const int16_t& position,
                  const int16_t& length,
                  const JustificationType& justify);

  void formatData() override {}

  // Acessors
  FieldColumnElement& columnElement() { return _columnElement; }
  const FieldColumnElement& columnElement() const { return _columnElement; }

private:
  FieldColumnElement _columnElement = FieldColumnElement::UNKNOWN_ELEMENT;
};

inline void
ElementField::initialize(const FieldColumnElement& element,
                         const int16_t& position,
                         const int16_t& length,
                         const JustificationType& justification)
{
  _columnElement = element;
  _valuePosition = position;
  _valueFieldSize = length;
  _justify = justification;
}

} // namespace tse

