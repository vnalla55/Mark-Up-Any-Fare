#pragma once

namespace ZThread
{

class Waitable
{
protected:
  virtual ~Waitable() {}

public:
  virtual void wait() = 0;
  virtual bool wait(unsigned long timeout) = 0;
};

} // namespace ZThread

