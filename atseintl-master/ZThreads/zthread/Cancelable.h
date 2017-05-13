#pragma once

namespace ZThread
{

class Cancelable
{
protected:
  virtual ~Cancelable() {}

public:
  virtual void cancel() = 0;
  virtual bool isCanceled() = 0;
};

} // namespace ZThread

