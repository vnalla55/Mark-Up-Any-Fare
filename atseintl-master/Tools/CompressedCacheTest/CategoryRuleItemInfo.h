//-------------------------------------------------------------------
//
//  File:	CategoryRuleItemInfo.h
//  Authors:	Devapriya SenGupta
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "TseTypes.h"
#include "Flattenizable.h"
#include "CompressedDataUtils.h"

namespace tse
{
  class CategoryRuleItemInfo ;

  namespace flattenizer
  {
    template <class CategoryRuleItemInfo>
    inline void flatten( Flattenizable::Archive & archive, const std::vector<CategoryRuleItemInfo*> & v ) ;
    template <class CategoryRuleItemInfo>
    inline void flatten( Flattenizable::Archive & archive, const std::vector<const CategoryRuleItemInfo*> & v );
    template <class CategoryRuleItemInfo>
    inline void unflatten( Flattenizable::Archive & archive, std::vector<CategoryRuleItemInfo*> & v );
    template <class CategoryRuleItemInfo>
    inline void unflatten( Flattenizable::Archive & archive, std::vector<const CategoryRuleItemInfo*> & v );
    template <class CategoryRuleItemInfo>
    inline void calcmem( Flattenizable::Archive & archive, const std::vector<CategoryRuleItemInfo*> & v );
    template <class CategoryRuleItemInfo>
    inline void calcmem( Flattenizable::Archive & archive, const std::vector<const CategoryRuleItemInfo*> & v );
  }
}

//#include "DBAccess/CategoryRuleItemInfoFactory.h"
#include <vector>

namespace tse
{

class RuleItemInfo;

/**
 * @class CategoryRuleItemInfo
 *
 * @brief Defines a wrapper for a Record 2 segment.
 */
class CategoryRuleItemInfo
{
public:

        CategoryRuleItemInfo ()
        : LogicalOperators(THEN)
        , _itemcat(0)
        , _orderNo(0)
        , _relationalInd(0)
        , _inOutInd(' ')
        , _directionality(' ')
        , _itemNo(0)
        { }
	virtual ~CategoryRuleItemInfo(){}



	enum {
          IF,
          THEN,
          OR,
          ELSE,
          AND
	} LogicalOperators;

	uint32_t& itemcat() { return _itemcat;}
	const uint32_t& itemcat() const { return _itemcat;}

	uint32_t& orderNo() { return _orderNo;}
  const uint32_t &orderNo() const { return _orderNo; }

  uint32_t& itemNo() { return _itemNo;}
  const uint32_t  &itemNo() const { return _itemNo; }

  uint32_t& relationalInd() { return _relationalInd;}
  const uint32_t &relationalInd() const { return _relationalInd; }

  Indicator& inOutInd() { return _inOutInd;}
  const Indicator& inOutInd() const { return _inOutInd; }

  Indicator &directionality() { return _directionality;}
  const Indicator &directionality() const { return _directionality; }

  virtual bool operator==( const CategoryRuleItemInfo & rhs ) const
  {
    bool eq = (    ( _itemcat        == rhs._itemcat        )
                && ( _orderNo        == rhs._orderNo        )
                && ( _relationalInd  == rhs._relationalInd  )
                && ( _inOutInd       == rhs._inOutInd       )
                && ( _directionality == rhs._directionality )
                && ( _itemNo         == rhs._itemNo         )
              ) ;

    return eq ;
  }

  static void dummyData( CategoryRuleItemInfo & obj )
  {
    obj._itemcat        = 101 ;
    obj._orderNo        = 102 ;
    obj._relationalInd  = 103 ;
    obj._inOutInd       = 'A' ;
    obj._directionality = 'B' ;
    obj._itemNo         = 104 ;
  }

  virtual void flattenize( Flattenizable::Archive & archive )
  {
    FLATTENIZE( archive, _itemcat         ) ;
    FLATTENIZE( archive, _orderNo         ) ;
    FLATTENIZE( archive, _relationalInd   ) ;
    FLATTENIZE( archive, _inOutInd        ) ;
    FLATTENIZE( archive, _directionality  ) ;
    FLATTENIZE( archive, _itemNo          ) ;
  }

  virtual WBuffer& write(WBuffer& os,
                         size_t* memSize = 0) const
  {
    os.write('B');
    if (memSize)
    {
      *memSize += sizeof(CategoryRuleItemInfo);
    }
    return convert(os, this);
  }

  virtual RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }
protected:

	  uint32_t  _itemcat        ;
    uint32_t  _orderNo        ;
    uint32_t  _relationalInd  ;
    Indicator _inOutInd       ;
    Indicator _directionality ;
  	uint32_t  _itemNo         ;

    template <typename B, typename T> static B& convert (B& buffer,
                                                         T ptr)
    {
      return buffer
             //& ptr->LogicalOperators
             & ptr->_itemcat
             & ptr->_orderNo
             & ptr->_relationalInd
             & ptr->_inOutInd
             & ptr->_directionality
             & ptr->_itemNo;
    }
private:
    CategoryRuleItemInfo(const CategoryRuleItemInfo&);
    CategoryRuleItemInfo& operator= (const CategoryRuleItemInfo&);

};


  namespace flattenizer
  {
    template <>
    inline void flatten( Flattenizable::Archive & archive, const std::vector<const CategoryRuleItemInfo*> & v )
    {
    }

    template <>
    inline void unflatten( Flattenizable::Archive & archive, std::vector<const CategoryRuleItemInfo*> & v )
    {
     }

    template <>
    inline void calcmem( Flattenizable::Archive & archive, const std::vector<const CategoryRuleItemInfo*> & v )
    {
    }

    template <>
    inline void flatten( Flattenizable::Archive & archive, const std::vector<CategoryRuleItemInfo*> & v )
    {
    }

    template <>
    inline void unflatten( Flattenizable::Archive & archive, std::vector<CategoryRuleItemInfo*> & v )
    {
    }

    template <>
    inline void calcmem( Flattenizable::Archive & archive, const std::vector<CategoryRuleItemInfo*> & v )
    {
    }
  }
} // tse namespace
