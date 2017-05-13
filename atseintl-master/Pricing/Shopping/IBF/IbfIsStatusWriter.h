//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "Diagnostic/Diagnostic.h"

#include <boost/utility.hpp>

#include <iostream>
#include <string>

namespace tse
{

class V2IbfManager;
class ShoppingTrx;

class IbfIsStatusWriter : boost::noncopyable
{
public:
  IbfIsStatusWriter(ShoppingTrx& trx, const V2IbfManager& mgr);
  void writeIsStatus(DiagnosticTypes diagType, const std::string& header);

private:
  void formatAndWriteMsg(std::ostream& out, const std::string& header);
  bool areDetailsEnabled();

  ShoppingTrx& _trx;
  const V2IbfManager& _mgr;
};

} // namespace tse

