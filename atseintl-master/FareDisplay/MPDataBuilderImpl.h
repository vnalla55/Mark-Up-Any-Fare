#pragma once

namespace tse
{

class MPData;
class FareDisplayTrx;

class MPDataBuilderImpl
{
public:
  virtual MPData* buildMPData(FareDisplayTrx&) const = 0;

  virtual ~MPDataBuilderImpl() {}
};

} // namespace tse

