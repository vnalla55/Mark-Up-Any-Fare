#pragma once

#include <boost/thread/exceptions.hpp>

namespace ZThread
{

typedef boost::thread_interrupted Interrupted_Exception;
typedef boost::thread_exception Synchronization_Exception;

class Cancellation_Exception:
  public Synchronization_Exception
{
public:
  virtual const char* what() const throw() { return "Canceled"; }
};

class InvalidOp_Exception:
  public Synchronization_Exception
{
public:
  virtual const char* what() const throw() { return "Invalid operation"; }
};

class Timeout_Exception:
  public Synchronization_Exception
{
public:
  virtual const char* what() const throw() { return "Timeout"; }
};

} // namespace ZThread

