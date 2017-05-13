//-------------------------------------------------------------------
//
//  File:        DiagElementField.h
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
static const int16_t HEADER_NOT_USED = -1;

class DiagElementField : public Field
{
public:
  void initialize(const DiagFieldElement& element,
                  const int16_t& position,
                  const int16_t& length,
                  const JustificationType& justify,
                  const int16_t& hdrPosition = HEADER_NOT_USED,
                  const std::string& hdrText = "");

  void formatData() override {}

  // Acessors
  int16_t& headerPosition() { return _headerPosition; }
  const int16_t& headerPosition() const { return _headerPosition; }

  std::string& headerText() { return _headerText; }
  const std::string& headerText() const { return _headerText; }

  DiagFieldElement& columnElement() { return _columnElement; }
  const DiagFieldElement& columnElement() const { return _columnElement; }

private:
  int16_t _headerPosition = 0;
  std::string _headerText;

  DiagFieldElement _columnElement = DiagFieldElement::UNKNOWN_DIAG_ELEMENT;
};

inline void
DiagElementField::initialize(const DiagFieldElement& element,
                             const int16_t& position,
                             const int16_t& length,
                             const JustificationType& justification,
                             const int16_t& hdrPosition,
                             const std::string& hdrText)
{
  _columnElement = element;
  _valuePosition = position;
  _valueFieldSize = length;
  _justify = justification;
  _headerPosition = hdrPosition;
  _headerText = hdrText;
}
} // namespace tse
