#pragma once

#include "Common/Gauss.h"

#include <boost/thread/mutex.hpp>

#include <vector>

namespace tse
{

class MetricsMan
{
  typedef Gauss<double> GaussType;

public:
  explicit MetricsMan(int size = 0);
  void initialize(int size);
  void update(int factor, double response);
  void select(int factor, GaussType& response);
  void clear();
  void clear(int factor);

private:
  boost::mutex _mutex;
  std::vector<GaussType> _data;
};

inline MetricsMan::MetricsMan(int size) : _data(size) {}

inline void
MetricsMan::initialize(int size)
{
  boost::mutex::scoped_lock lock(_mutex);
  _data.resize(size);
}

inline void
MetricsMan::update(int factor, double response)
{
  boost::mutex::scoped_lock lock(_mutex);
  _data.at(factor).include(response);
}

inline void
MetricsMan::select(int factor, GaussType& response)
{
  boost::mutex::scoped_lock lock(_mutex);
  response = _data.at(factor);
}

inline void
MetricsMan::clear()
{
  boost::mutex::scoped_lock lock(_mutex);
  for (auto& gauss : _data)
  {
    gauss.clear();
  }
}

inline void
MetricsMan::clear(int factor)
{
  boost::mutex::scoped_lock lock(_mutex);
  _data.at(factor).clear();
}

} // namespace tse

