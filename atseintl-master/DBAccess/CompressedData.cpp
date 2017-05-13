#include "DBAccess/CompressedData.h"

namespace sfc
{

CompressedData::CompressedData() : _inflatedSz(0) {}

CompressedData::CompressedData(const std::vector<char>& deflated,
                               size_t inflatedSz,
                               size_t deflatedSz)
  : _deflated(deflated.begin(), deflated.begin() + deflatedSz), _inflatedSz(inflatedSz)
{
}

CompressedData::CompressedData(std::vector<char>& deflated, size_t inflatedSz)
  : _inflatedSz(inflatedSz)
{
  _deflated.swap(deflated);
}

CompressedData::~CompressedData() {}

void
CompressedData::swap(CompressedData& other)
{
  size_t inflatedSz(_inflatedSz);
  _inflatedSz = other._inflatedSz;
  other._inflatedSz = inflatedSz;
  _deflated.swap(other._deflated);
}

bool
CompressedData::empty() const
{
  return 0 == _inflatedSz && _deflated.empty();
}

} // sfc
