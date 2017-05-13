#pragma once

namespace ZThread
{

class Runnable
{
public:
  virtual ~Runnable() {}
  virtual void run() = 0;

};

} // namespace ZThread

