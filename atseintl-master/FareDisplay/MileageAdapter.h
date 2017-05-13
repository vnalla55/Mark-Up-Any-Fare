#pragma once

// FIXME <memory> should be included by Singleton.h
#include "Common/Singleton.h"

#include <memory>

namespace tse
{

class MPData;
class MPChooser;

class MileageAdapter
{
public:
  bool getMPM(MPData&, MPChooser&) const;

private:
  MileageAdapter() {}
  MileageAdapter(const MileageAdapter&);
  MileageAdapter& operator=(const MileageAdapter&);
  friend class tse::Singleton<MileageAdapter>;
};

} // namespace tse

