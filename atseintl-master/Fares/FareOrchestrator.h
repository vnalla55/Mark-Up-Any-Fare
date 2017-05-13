// ----------------------------------------------------------------
//
//   File:        FareOrchestrator.h
//   Created:     Dec 4, 2003
//   Authors:     Abu Islam, Mark Kasprowicz, Bruce Melberg,
//                Vadim Nikushin
//
//   Description: Base class for all Fare Orchestrators
//
//   Copyright Sabre 2003
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Common/ErrorResponseException.h"
#include "Common/Thread/TseThreadingConst.h"
#include "Service/Service.h"

#include <vector>

namespace tse
{
class ConfigMan;
class PricingTrx;
class TseServer;

class FareOrchestrator : public Service
{
private:
  FareOrchestrator(const FareOrchestrator& rhs);
  FareOrchestrator& operator=(const FareOrchestrator& rhs);

protected:
  FareOrchestrator(const std::string& name, TseServer& server, TseThreadingConst::TaskId taskId);

  virtual bool initialize(int argc, char** argv) override;

  bool checkIfFailCodeExist(const PricingTrx& trx,
                            const ErrorResponseException::ErrorResponseCode& failCode) const;

  ConfigMan& _config;
  const TseThreadingConst::TaskId _taskId;

public:
  virtual uint32_t getActiveThreads() override;
};

} // namespace tse

