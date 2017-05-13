//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#pragma once

#include <stdint.h>
#include "TseTypes.h"
#include "CompressedDataUtils.h"

#include "Flattenizable.h"

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

        RuleItemInfo ()
        : _itemNo(0)
        , _textTblItemNo(0)
        , _overrideDateTblItemNo(0)
        { }
	virtual ~RuleItemInfo(){}

	VendorCode& vendor() { return _vendor;}
  const VendorCode& vendor() const { return _vendor; }

  uint32_t& itemNo(){ return _itemNo;}
  const uint32_t& itemNo() const { return _itemNo; }

  uint32_t& textTblItemNo(){ return _textTblItemNo;}
  const uint32_t& textTblItemNo() const { return _textTblItemNo; }

  uint32_t& overrideDateTblItemNo(){ return _overrideDateTblItemNo;}
  const uint32_t& overrideDateTblItemNo() const { return _overrideDateTblItemNo; }

 	virtual bool operator==( const RuleItemInfo & rhs ) const
 	{
 	  return(    ( _vendor                == rhs._vendor                )
 	          && ( _itemNo                == rhs._itemNo                )
 	          && ( _textTblItemNo         == rhs._textTblItemNo         )
 	          && ( _overrideDateTblItemNo == rhs._overrideDateTblItemNo )
 	        ) ;
 	}

  friend inline std::ostream & dumpObject( std::ostream & os, const RuleItemInfo & obj )
  {
    return os << "[" << obj._vendor
              << "|" << obj._itemNo
              << "|" << obj._textTblItemNo
              << "|" << obj._overrideDateTblItemNo
              << "]"
              ;
  }

  static void dummyData( RuleItemInfo & obj )
  {
    obj._vendor                = "ABCD" ;
    obj._itemNo                = 1      ;
    obj._textTblItemNo         = 2      ;
    obj._overrideDateTblItemNo = 3      ;
  }

private:

  VendorCode   _vendor;
  uint32_t     _itemNo;
  uint32_t     _textTblItemNo;
  uint32_t     _overrideDateTblItemNo;

  template <typename B, typename T> static B& convert (B& buffer,
                                                       T ptr)
  {
    return buffer
           & ptr->_vendor
           & ptr->_itemNo
           & ptr->_textTblItemNo
           & ptr->_overrideDateTblItemNo;
  }
public:

  virtual void flattenize( Flattenizable::Archive & archive )
  {
    FLATTENIZE( archive, _vendor                ) ;
    FLATTENIZE( archive, _itemNo                ) ;
    FLATTENIZE( archive, _textTblItemNo         ) ;
    FLATTENIZE( archive, _overrideDateTblItemNo ) ;
  }

};

}
