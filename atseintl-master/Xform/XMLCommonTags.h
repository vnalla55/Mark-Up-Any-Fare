//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include <string>

namespace tse
{
  namespace xml2
  {
    static const std::string BrandCode = "SB2";
    static const std::string BrandDescription = "SB3";
    // sometimes contains Program Code, sometimes Program ID
    static const std::string ProgramCode = "SC0";
    static const std::string SystemCode = "SC1";
    static const std::string ProgramName = "SC2";
    // new tags for Program Id and Program Code
    static const std::string ProgramId = "SC3";
    static const std::string ProgramCodeNew = "SC4";
  }
} // namespace tse

