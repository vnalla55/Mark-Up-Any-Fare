#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

#include <stdint.h>

namespace tse
{

/**
 * @class RuleItemInfo
 *
 * @brief Defines a wrapper for a Record 3.
 */

class RuleItemInfo
{
public:
  virtual ~RuleItemInfo() = default;

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  uint32_t& itemNo() { return _itemNo; }
  const uint32_t& itemNo() const { return _itemNo; }

  uint32_t& textTblItemNo() { return _textTblItemNo; }
  const uint32_t& textTblItemNo() const { return _textTblItemNo; }

  uint32_t& overrideDateTblItemNo() { return _overrideDateTblItemNo; }
  const uint32_t& overrideDateTblItemNo() const { return _overrideDateTblItemNo; }

  virtual bool operator==(const RuleItemInfo& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_itemNo == rhs._itemNo) &&
            (_textTblItemNo == rhs._textTblItemNo) &&
            (_overrideDateTblItemNo == rhs._overrideDateTblItemNo));
  }

  friend inline std::ostream& dumpObject(std::ostream& os, const RuleItemInfo& obj)
  {
    return os << "[" << obj._vendor << "|" << obj._itemNo << "|" << obj._textTblItemNo << "|"
              << obj._overrideDateTblItemNo << "]";
  }

  static void dummyData(RuleItemInfo& obj)
  {
    obj._vendor = "ABCD";
    obj._itemNo = 1;
    obj._textTblItemNo = 2;
    obj._overrideDateTblItemNo = 3;
  }

protected:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_vendor & ptr->_itemNo & ptr->_textTblItemNo & ptr->_overrideDateTblItemNo;
  }

  VendorCode _vendor;
  uint32_t _itemNo = 0;
  uint32_t _textTblItemNo = 0;
  uint32_t _overrideDateTblItemNo = 0;

public:
  virtual void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _itemNo);
    FLATTENIZE(archive, _textTblItemNo);
    FLATTENIZE(archive, _overrideDateTblItemNo);
  }
};
}

