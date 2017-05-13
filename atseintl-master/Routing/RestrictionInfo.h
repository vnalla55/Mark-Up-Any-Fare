#pragma once
#include "Routing/ValidationInfo.h"

namespace tse
{

class RestrictionInfo : public ValidationInfo
{
public:
  RestrictionInfo() : _base(false), _origAddOn(false), _destAddOn(false) {}
  RestrictionInfo(bool processed, bool valid)
    : ValidationInfo(processed, valid), _base(false), _origAddOn(false), _destAddOn(false)
  {
  }

  bool base() const { return _base; }
  bool& base() { return _base; }

  bool origAddOn() const { return _origAddOn; }
  bool& origAddOn() { return _origAddOn; }

  bool destAddOn() const { return _destAddOn; }
  bool& destAddOn() { return _destAddOn; }

private:
  bool _base;
  bool _origAddOn;
  bool _destAddOn;
};
}
