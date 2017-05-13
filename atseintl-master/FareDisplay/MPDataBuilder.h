#pragma once

// FIXME <memory> should be included by Singleton.h
#include "Common/Singleton.h"

#include <memory>

namespace tse
{

class MPData;
class FareDisplayTrx;
class MPChooser;

class MPDataBuilder
{
public:
  MPData* buildMPData(FareDisplayTrx&, MPChooser&) const;

private:
  MPDataBuilder() {}
  MPDataBuilder(const MPDataBuilder&);
  MPDataBuilder& operator=(const MPDataBuilder&);
  friend class tse::Singleton<MPDataBuilder>;
};

} // namespace tse

