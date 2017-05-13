// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "DomainDataObjects/Message.h"

namespace tax
{

struct ErrorMessage : public Message
{
  std::string _errorDetail;

  ErrorMessage() = default;

  explicit ErrorMessage(std::string content, std::string detail)
  : _errorDetail{std::move(detail)}
  {
    Message::_content = std::move(content);
  }
};

}

