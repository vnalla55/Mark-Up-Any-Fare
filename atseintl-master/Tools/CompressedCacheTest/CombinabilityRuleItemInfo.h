//-----------------------------------------------------------------------------------------------
//	   Copyright 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  --------------------------------------------------------------------------------------------

#pragma once

#include "CategoryRuleItemInfo.h"
#include "TseTypes.h"

namespace tse {

class CombinabilityRuleItemInfo : public CategoryRuleItemInfo
{
public:

    CombinabilityRuleItemInfo ()
    : _textonlyInd(' ')
    , _eoervalueInd(' ')
    , _eoeallsegInd(' ')
    , _sameCarrierInd(' ')
    , _sameRuleTariffInd(' ')
    , _sameFareInd(' ')
    { }
    virtual ~CombinabilityRuleItemInfo(){}

    Indicator &textonlyInd() { return _textonlyInd;}
    const Indicator &textonlyInd() const { return _textonlyInd; }


    Indicator &eoervalueInd() { return _eoervalueInd;}
    const Indicator &eoervalueInd() const { return _eoervalueInd; }


    Indicator &eoeallsegInd() { return _eoeallsegInd;}
    const Indicator &eoeallsegInd() const { return _eoeallsegInd; }


    Indicator &sameCarrierInd() { return _sameCarrierInd;}
    const Indicator &sameCarrierInd() const { return _sameCarrierInd; }


    Indicator &sameRuleTariffInd() { return _sameRuleTariffInd;}
    const Indicator &sameRuleTariffInd() const { return _sameRuleTariffInd; }


    Indicator &sameFareInd() { return _sameFareInd;}
    const Indicator &sameFareInd() const { return _sameFareInd; }


    virtual bool operator==( const CombinabilityRuleItemInfo & rhs ) const
    {
      return(    (  CategoryRuleItemInfo::operator==( rhs )                             )
              && ( _textonlyInd                   == rhs._textonlyInd                   )
              && ( _eoervalueInd                  == rhs._eoervalueInd                  )
              && ( _eoeallsegInd                  == rhs._eoeallsegInd                  )
              && ( _sameCarrierInd                == rhs._sameCarrierInd                )
              && ( _sameRuleTariffInd             == rhs._sameRuleTariffInd             )
              && ( _sameFareInd                   == rhs._sameFareInd                   )
            ) ;
    }

    static void dummyData( CombinabilityRuleItemInfo & obj )
    {
      CategoryRuleItemInfo::dummyData( obj ) ;

      obj._textonlyInd       = 'Z' ;
      obj._eoervalueInd      = 'Y' ;
      obj._eoeallsegInd      = 'X' ;
      obj._sameCarrierInd    = 'W' ;
      obj._sameRuleTariffInd = 'V' ;
      obj._sameFareInd       = 'U' ;
    }

    virtual WBuffer& write(WBuffer& os,
                           size_t* memSize) const
    {
      os.write('C');
      if (memSize)
      {
        *memSize += sizeof(CombinabilityRuleItemInfo);
      }
      return convert(os, this);
    }

    virtual RBuffer& read(RBuffer& is)
    {
      return convert(is, this);
    }
protected:

    Indicator _textonlyInd       ;
    Indicator _eoervalueInd      ;
    Indicator _eoeallsegInd      ;
    Indicator _sameCarrierInd    ;
    Indicator _sameRuleTariffInd ;
    Indicator _sameFareInd       ;

public:

    virtual void flattenize( Flattenizable::Archive & archive )
    {
      FLATTENIZE_BASE_OBJECT( archive, CategoryRuleItemInfo  ) ;
      FLATTENIZE( archive, _textonlyInd        ) ;
      FLATTENIZE( archive, _eoervalueInd       ) ;
      FLATTENIZE( archive, _eoeallsegInd       ) ;
      FLATTENIZE( archive, _sameCarrierInd     ) ;
      FLATTENIZE( archive, _sameRuleTariffInd  ) ;
      FLATTENIZE( archive, _sameFareInd        ) ;
    }

 private:

    template <typename B, typename T> static B& convert(B& buffer,
                                                        T ptr)
    {
      return CategoryRuleItemInfo::convert(buffer, ptr)
             & ptr->_textonlyInd
             & ptr->_eoervalueInd
             & ptr->_eoeallsegInd
             & ptr->_sameCarrierInd
             & ptr->_sameRuleTariffInd
             & ptr->_sameFareInd;
    }
};

}
