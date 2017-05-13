//----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include <boost/filesystem.hpp>

inline
std::string
getSuiteName()
{
  static const std::string sep = "/";
  std::string path  = boost::filesystem::current_path().string();
  std::string suiteName = "";
  while (!path.empty())
  {
    std::string::size_type found = path.find_last_of("/\\");
    suiteName = path.substr(found+1) + sep + suiteName;
    path = path.substr(0,found);
    std::ifstream fGit((path +"/.git").c_str());
    if (fGit)
      return suiteName.substr(0,suiteName.length()-(4+2*sep.length()));
  }
  return "All Tests";
}

