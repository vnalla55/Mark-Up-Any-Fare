#pragma once

#include "FareCalc/FcMessage.h"

#include <set>

namespace tse
{

class FarePath;

class FcMultiMessage : public FcMessage
{
public:
  FcMultiMessage(MessageType type, ErrorCode code, const std::string& msg, bool prefix = true)
    : FcMessage(type, code, msg, prefix)
  {
  }

  FcMultiMessage(MessageType type,
                 ErrorCode code,
                 const std::string& alCode,
                 const std::string& msg,
                 bool prefix = true)
    : FcMessage(type, code, alCode, msg, prefix)
  {
  }

  FcMultiMessage(const FcMultiMessage& rhs) : FcMessage(dynamic_cast<const FcMessage&>(rhs))
  {
    _farePaths.insert(rhs._farePaths.begin(), rhs._farePaths.end());
    _indexList.insert(_indexList.begin(), rhs._indexList.begin(), rhs._indexList.end());
  }

  const FcMultiMessage& operator=(const FcMultiMessage& rhs)
  {
    if (this == &rhs)
      return *this;

    FcMessage::operator=(dynamic_cast<const FcMessage&>(rhs));

    _farePaths.clear();
    _farePaths.insert(rhs._farePaths.begin(), rhs._farePaths.end());

    _indexList.clear();
    _indexList.insert(_indexList.begin(), rhs.indexList().begin(), rhs.indexList().end());

    return *this;
  }

  const std::set<const FarePath*>& farePaths() const { return _farePaths; }
  void addFarePath(const FarePath* fp) { _farePaths.insert(fp); }
  bool chkFarePath(const FarePath* fp) const { return (_farePaths.find(fp) != _farePaths.end()); }

  const std::vector<int>& indexList() const { return _indexList; }

private:
  std::set<const FarePath*> _farePaths;
  std::vector<int> _indexList;
};
}

