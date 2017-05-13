#pragma once
//----------------------------------------------------------------------------
// LimitationJrny.h
//
// Copyright Sabre 2004
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LimitationCmn.h"

#include <vector>

namespace tse
{

class LimitationJrny : public LimitationCmn
{
public:
  LimitationJrny() : _separateTktInd(' ') {}

  virtual ~LimitationJrny() {}

  Indicator& separateTktInd() { return _separateTktInd; }
  const Indicator& separateTktInd() const { return _separateTktInd; }

  std::vector<std::string>& textMsg() { return _textMsg; }
  const std::vector<std::string>& textMsg() const { return _textMsg; }

  virtual bool operator==(const LimitationJrny& rhs) const
  {
    return ((LimitationCmn::operator==(rhs)) && (_separateTktInd == rhs._separateTktInd) &&
            (_textMsg == rhs._textMsg));
  }

  static void dummyData(LimitationJrny& obj)
  {
    obj._separateTktInd = 'A';
    obj._textMsg.push_back("aaaaaaaa");
    obj._textMsg.push_back("bbbbbbbb");
  }

private:
  Indicator _separateTktInd;
  std::vector<std::string> _textMsg;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, LimitationCmn);
    FLATTENIZE(archive, _separateTktInd);
    FLATTENIZE(archive, _textMsg);
  }
};
} // namespace tse

