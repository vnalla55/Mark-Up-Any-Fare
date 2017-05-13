//-------------------------------------------------------------------
//
//  File:	CategoryRuleItemInfoSet.h
//  Authors:	Devpariya SenGupta
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
#ifndef CATEGORY_RULE_ITEM_INFO_SET_H
#define CATEGORY_RULE_ITEM_INFO_SET_H

#include "Flattenizable.h"

#include <vector>

#include "CategoryRuleItemInfo.h"

namespace tse
{

class CategoryRuleItemInfoSet
{
public:
    CategoryRuleItemInfoSet()
    {
    }

    virtual ~CategoryRuleItemInfoSet()
    {
      std::vector<CategoryRuleItemInfo*>::const_iterator it(_categoryRuleItemInfo.begin()),
                                                         itend(_categoryRuleItemInfo.end());
      for ( ; it != itend; ++it)
      {
        delete *it;
      }
    }

    void callDestructors()
    {
      std::vector<CategoryRuleItemInfo*>::const_iterator it(_categoryRuleItemInfo.begin()),
                                                         itend(_categoryRuleItemInfo.end());
      for ( ; it != itend; ++it)
      {
        (*it)->~CategoryRuleItemInfo();
      }
      std::vector<CategoryRuleItemInfo*> empty;
      _categoryRuleItemInfo.swap(empty);
    }

    std::vector<CategoryRuleItemInfo*> &categoryRuleItemInfo() { return _categoryRuleItemInfo; }
    const std::vector<CategoryRuleItemInfo*> &categoryRuleItemInfo() const { return _categoryRuleItemInfo; }

    virtual bool operator==( const CategoryRuleItemInfoSet & rhs ) const
    {
      bool eq( _categoryRuleItemInfo.size() == rhs._categoryRuleItemInfo.size() ) ;
      for( size_t i = 0 ; ( eq && ( i < _categoryRuleItemInfo.size() ) ) ; ++i )
      {
        eq = ( *(_categoryRuleItemInfo[i]) == *(rhs._categoryRuleItemInfo[i]) ) ;
      }
      return eq ;
    }

    void flattenize( Flattenizable::Archive & archive )
    {
      FLATTENIZE( archive, _categoryRuleItemInfo ) ;
    }

    WBuffer& write(WBuffer& os) const
    {
      os.write(_categoryRuleItemInfo);
      return os;
    }

    RBuffer& read(RBuffer& is)
    {
      is.read(_categoryRuleItemInfo);
      return is;
    }

protected:

    std::vector<CategoryRuleItemInfo*>  _categoryRuleItemInfo;

private:

    CategoryRuleItemInfoSet(const CategoryRuleItemInfoSet&);
    CategoryRuleItemInfoSet& operator= (const CategoryRuleItemInfoSet&);

};

} // tse namespace

#endif // ifndef CATEGORY_RULE_ITEM_INFO_SET_H
