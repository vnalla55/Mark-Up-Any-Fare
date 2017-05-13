// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#pragma once

#include <memory>
#include <stdint.h>
#include <string>

namespace tse
{
class ConfigMan;
template <typename T> class DynamicConfigurableValue;

template <typename T>
struct DynamicConfigurableValuesBundlePool
{
  static void registerInBundle(DynamicConfigurableValue<T>& cv);
  static void unregisterInBundle(DynamicConfigurableValue<T>& cv);

  static uint32_t size();
};

void allocateAllConfigBundles();

class ConfigBundle
{
public:
  ConfigBundle() = default;

  explicit operator bool() const { return bool(_memory); }
  bool operator!() const { return !_memory; }

  void allocate();

  void clear()
  {
    _memory.reset();
  }

  template <typename Type>
  Type& get(uint32_t i)
  {
    return *reinterpret_cast<Type*>(array() + i);
  }

  template <typename Type>
  const Type& get(uint32_t i) const
  {
    return *reinterpret_cast<Type*>(array() + i);
  }

  ConfigBundle clone() const;
  void makeUnique();
  void fill(const ConfigMan& config);
  void update(const ConfigMan& config, std::string section, std::string option);

private:
  std::shared_ptr<void> _memory;

  char* array() const { return reinterpret_cast<char*>(_memory.get()); }
};
}
