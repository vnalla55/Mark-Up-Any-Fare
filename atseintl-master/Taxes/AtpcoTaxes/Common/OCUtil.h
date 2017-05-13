// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "DataModel/Common/Types.h"

namespace tax
{

class OCUtil
{
public:
  OCUtil() = delete;

  static bool
  isOCSegmentRelated(const type::OptionalServiceTag& type);

  static bool
  isOCValidForGroup(
      const type::OptionalServiceTag& type,
      const type::ProcessingGroup& processingGroup);

  static std::string
  getOCTypeString(const type::OptionalServiceTag& type);
};

} // namespace tax
