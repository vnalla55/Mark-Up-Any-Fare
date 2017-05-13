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
#pragma once

namespace tax
{
class OptionalServicePath;
class InputOptionalServicePath;

class OptionalServicePathFactory
{
public:
  static
  OptionalServicePath createFromInput(const InputOptionalServicePath& inputOptionalServicePath);
};

} // namespace tax

