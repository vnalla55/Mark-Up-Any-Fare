#pragma once

#include <boost/noncopyable.hpp>

#include <memory>
#include <vector>

namespace sfc
{

class CompressedData;
typedef std::shared_ptr<CompressedData> CompressedDataPtr;

class CompressedData : boost::noncopyable
{
public:
  CompressedData();

  CompressedData(const std::vector<char>& deflated, size_t inflatedSz, size_t deflatedSz);

  CompressedData(std::vector<char>& deflated, size_t inflatedSz);

  ~CompressedData();

  void swap(CompressedData& other);

  bool empty() const;

  std::vector<char> _deflated;
  size_t _inflatedSz;
};

} // sfc
