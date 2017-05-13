//----------------------------------------------------------------------------
//	   Copyright 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  ----------------------------------------------------------------------------

#pragma once

#include "Flattenizable.h"
#include "CompressedDataUtils.h"

namespace tse {

class TaxRestrictionPsg
{
public:

      TaxRestrictionPsg ()
      : _showPsg(' ')
      , _fareZeroOnly(' ')
      , _minAge(0)
      , _maxAge(0)
      { }

    PaxTypeCode& psgType() { return _psgType; }
    const PaxTypeCode& psgType() const { return _psgType; }

    Indicator& showPsg() { return _showPsg; }
    const Indicator& showPsg() const { return _showPsg; }

    Indicator& fareZeroOnly() { return _fareZeroOnly; }
    const Indicator& fareZeroOnly() const { return _fareZeroOnly; }

    int& minAge() { return _minAge; }
    const int minAge() const { return _minAge; }

    int& maxAge() { return _maxAge; }
    const int maxAge() const { return _maxAge; }

    bool operator==( const TaxRestrictionPsg & rhs ) const
    {
      return(    ( _psgType      == rhs._psgType      )
              && ( _showPsg      == rhs._showPsg      )
              && ( _fareZeroOnly == rhs._fareZeroOnly )
              && ( _minAge       == rhs._minAge       )
              && ( _maxAge       == rhs._maxAge       )
            ) ;
    }

    WBuffer& write(WBuffer& os) const
    {
      return convert(os, this);
    }

    RBuffer& read(RBuffer& is)
    {
      return convert(is, this);
    }

    static void dummyData( TaxRestrictionPsg & obj )
    {
      obj._psgType      = "ABC" ;
      obj._showPsg      = 'D'   ;
      obj._fareZeroOnly = 'E'   ;
      obj._minAge       = 1     ;
      obj._maxAge       = 2     ;
    }

private:

    PaxTypeCode _psgType      ;
    Indicator   _showPsg      ;
    Indicator   _fareZeroOnly ;
    int         _minAge       ;
    int         _maxAge       ;


    template <typename B, typename T> static B& convert (B& buffer,
                                                         T ptr)
    {
      return buffer
             & ptr->_psgType
             & ptr->_showPsg
             & ptr->_fareZeroOnly
             & ptr->_minAge
             & ptr->_maxAge;
    }

public:

    void flattenize( Flattenizable::Archive & archive )
    {
      FLATTENIZE( archive, _psgType      ) ;
      FLATTENIZE( archive, _showPsg      ) ;
      FLATTENIZE( archive, _fareZeroOnly ) ;
      FLATTENIZE( archive, _minAge       ) ;
      FLATTENIZE( archive, _maxAge       ) ;
    }
};

}
