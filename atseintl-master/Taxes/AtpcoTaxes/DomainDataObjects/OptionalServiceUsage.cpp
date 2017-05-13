// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include "DomainDataObjects/OptionalServiceUsage.h"
#include "DomainDataObjects/OptionalService.h"

namespace tax
{

OptionalServiceUsage::OptionalServiceUsage(): _index(0), _optionalService(nullptr) {}

OptionalServiceUsage::~OptionalServiceUsage() {}

std::ostream&
OptionalServiceUsage::print(std::ostream& out,
                            int indentLevel /* = 0 */,
                            char indentChar /* = ' ' */) const
{
  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "INDEX: " << _index << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "OPTIONALSERVICE\n";
  if (_optionalService != nullptr)
    _optionalService->print(out, indentLevel + 1);
  else
    out << " NULL\n";

  return out;
}

} // namespace tax
