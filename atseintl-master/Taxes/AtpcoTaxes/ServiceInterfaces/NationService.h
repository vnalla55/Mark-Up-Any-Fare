// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include <string>

namespace tax
{
namespace type
{
class Timestamp;
} // namespace type

class NationService
{
public:
  virtual ~NationService() = default;

  virtual std::string getMessage(const tax::type::Nation& nationCode,
                                 const tax::type::Timestamp& ticketingDate) const = 0;

protected:
  NationService() = default;
};

} // namespace tax
