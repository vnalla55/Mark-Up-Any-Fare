#pragma once

namespace tse
{

class ValidationInfo
{
public:
  ValidationInfo() = default;
  ValidationInfo(bool isProcessed, bool isValid) : _processed(isProcessed), _valid(isValid) {}
  bool processed() const { return _processed; }
  bool& processed() { return _processed; }
  bool valid() const { return _valid; }
  bool& valid() { return _valid; }
  virtual ~ValidationInfo() = default;

private:
  bool _processed = false;
  bool _valid = true;
};
}
