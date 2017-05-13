//-------------------------------------------------------------------
//
//  File:        BaseTrx.h
//  Created:     October 2, 2009
//
//  Copyright Sabre 2009
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

#include "Common/Memory/TrxManager.h"

#include <memory>

#include <cstdint>
#include <string>

namespace tse
{
class BaseTrx
{
public:
  void setMemoryManager(Memory::TrxManager* manager) { _memoryManager.reset(manager); }
  Memory::TrxManager* getMemoryManager() const { return _memoryManager.get(); }

  int64_t getBaseIntId() const { return _baseIntId; }

  void setRawRequest(std::string rawRequest) { _rawRequest = rawRequest; }
  const std::string& rawRequest() const { return _rawRequest; }

protected:
  BaseTrx(Memory::TrxManager* manager = nullptr);
  virtual ~BaseTrx() = 0;

private:
  std::unique_ptr<Memory::TrxManager> _memoryManager;
  const int64_t _baseIntId;
  std::string _rawRequest;

  BaseTrx(const BaseTrx&) = delete;
  BaseTrx& operator=(const BaseTrx&) = delete;
};
} // namespace tse
