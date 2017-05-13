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
#include <string>
#include <boost/optional.hpp>

#include "Common/Thread/TseCallableTrxTask.h"

namespace tse
{
  class PricingTrx;
}

namespace tax
{

class PfcTask : public tse::TseCallableTrxTask
{
  boost::optional<std::string> _failureMessage;

public:
  explicit PfcTask(tse::PricingTrx& trx);
  virtual void performTask() override;
  const boost::optional<std::string>& failureMessage() const { return _failureMessage; }
};

}

