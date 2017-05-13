#pragma once

namespace tse
{

class MileageRoute;

/* Classes that represent exclusions/reductions applied to the whole mileage route */
class MileageExclusion
{
public:
  virtual bool apply(MileageRoute&) const = 0;
  virtual ~MileageExclusion() {}
};

} // namespace tse

