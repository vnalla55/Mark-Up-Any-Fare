//----------------------------------------------------------------------------
//  Copyright Sabre 2004
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

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class Diag606Collector : public DiagCollector
{
public:
  void initParam(Diagnostic& root) override;

  virtual void printHeader() override;
  Diag606Collector& operator<<(const PricingUnit& pu) override;

  static boost::mutex& fcSBdisplayMutex() { return _fcSBdisplayMutex; }
  static bool& fcSBdisplayDone() { return _fcSBdisplayDone; }

private:
  static boost::mutex _fcSBdisplayMutex;
  static bool _fcSBdisplayDone;
  std::string _fareClass;
  std::string _origin;
  std::string _destination;
  std::string _carrier;
};

} /* end tse namespace */


