#pragma once

namespace tse
{

class MPData;

class MileageAdapterImpl
{
public:
  virtual bool getMPM(MPData&) const = 0;

  virtual ~MileageAdapterImpl() {}

protected:
  bool getMPMforGD(MPData&) const;
};

} // namespace tse

