#pragma once

namespace tse
{

template <class T>
class Singleton
{
public:
  static T& instance();

private:
  ~Singleton();
  Singleton();
  Singleton(const Singleton&);
  void operator=(const Singleton&);
};

template <class T>
T&
Singleton<T>::instance()
{
  // NOTE this code works only for GCC and clang ( and for VC++ if it will have full
  // implementation of C++11 which specifies static locals behaviour ).
  // GCC/clang guarantees thread safety for static locals. This may be turned off
  // by -fno-threadsafe-statics option. From man page:
  // -fno-threadsafe-statics option
  //     Do not emit the extra code to use the routines specified in the C++ ABI
  //        for thread-safe initialization of local statics.
  //     You can use this option to reduce code size slightly in code that doesn't
  //        need to be thread-safe.
  static T instanceObject;
  return instanceObject;
}

} // namespace tse

