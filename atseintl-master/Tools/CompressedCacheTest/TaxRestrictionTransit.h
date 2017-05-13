//----------------------------------------------------------------------------
//
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

class TaxRestrictionTransit
{
public:

      TaxRestrictionTransit ()
      : _orderNo(0)
      , _transitHours(0)
      , _transitMinutes(0)
      , _sameDayInd(' ')
      , _nextDayInd(' ')
      , _sameFlight(' ')
      , _transitTaxonly(' ')
      , _transitDomDom(' ')
      , _transitDomIntl(' ')
      , _transitIntlDom(' ')
      , _transitIntlIntl(' ')
      , _transitSurfDom(' ')
      , _transitSurfIntl(' ')
      , _transitOfflineCxr(' ')
      , _flightArrivalHours(0)
      , _flightArrivalMinutes(0)
      , _flightDepartHours(0)
      , _flightDepartMinutes(0)
      { }

    int& orderNo() { return _orderNo; }
    const int orderNo() const { return _orderNo; }

    int& transitHours() { return _transitHours; }
    const int transitHours() const { return _transitHours; }

    int& transitMinutes() { return _transitMinutes; }
    const int transitMinutes() const { return _transitMinutes; }

    Indicator& sameDayInd() { return _sameDayInd; }
    const Indicator& sameDayInd() const { return _sameDayInd; }

    Indicator& nextDayInd() { return _nextDayInd; }
    const Indicator& nextDayInd() const { return _nextDayInd; }

    Indicator& sameFlight() { return _sameFlight; }
    const Indicator& sameFlight() const { return _sameFlight; }

    Indicator& transitTaxonly() { return _transitTaxonly; }
    const Indicator& transitTaxonly() const { return _transitTaxonly; }

    Indicator& transitDomDom() { return _transitDomDom; }
    const Indicator& transitDomDom() const { return _transitDomDom; }

    Indicator& transitDomIntl() { return _transitDomIntl; }
    const Indicator& transitDomIntl() const { return _transitDomIntl; }

    Indicator& transitIntlDom() { return _transitIntlDom; }
    const Indicator& transitIntlDom() const { return _transitIntlDom; }

    Indicator& transitIntlIntl() { return _transitIntlIntl; }
    const Indicator& transitIntlIntl() const { return _transitIntlIntl; }

    Indicator& transitSurfDom() { return _transitSurfDom; }
    const Indicator& transitSurfDom() const { return _transitSurfDom; }

    Indicator& transitSurfIntl() { return _transitSurfIntl; }
    const Indicator& transitSurfIntl() const { return _transitSurfIntl; }

    Indicator& transitOfflineCxr() { return _transitOfflineCxr; }
    const Indicator& transitOfflineCxr() const { return _transitOfflineCxr; }

    int& flightArrivalHours() { return _flightArrivalHours; }
    const int flightArrivalHours() const { return _flightArrivalHours; }

    int& flightArrivalMinutes() { return _flightArrivalMinutes; }
    const int flightArrivalMinutes() const { return _flightArrivalMinutes; }

    int& flightDepartHours() { return _flightDepartHours; }
    const int flightDepartHours() const { return _flightDepartHours; }

    int& flightDepartMinutes() { return _flightDepartMinutes; }
    const int flightDepartMinutes() const { return _flightDepartMinutes; }

    LocType& viaLocType() { return _viaLocType; }
    const LocType& viaLocType() const { return _viaLocType; }

    LocCode& viaLoc() { return _viaLoc; }
    const LocCode& viaLoc() const { return _viaLoc; }

    bool operator==( const TaxRestrictionTransit & rhs ) const
    {
      return(    ( _orderNo              == rhs._orderNo              )
              && ( _transitHours         == rhs._transitHours         )
              && ( _transitMinutes       == rhs._transitMinutes       )
              && ( _sameDayInd           == rhs._sameDayInd           )
              && ( _nextDayInd           == rhs._nextDayInd           )
              && ( _sameFlight           == rhs._sameFlight           )
              && ( _transitTaxonly       == rhs._transitTaxonly       )
              && ( _transitDomDom        == rhs._transitDomDom        )
              && ( _transitDomIntl       == rhs._transitDomIntl       )
              && ( _transitIntlDom       == rhs._transitIntlDom       )
              && ( _transitIntlIntl      == rhs._transitIntlIntl      )
              && ( _transitSurfDom       == rhs._transitSurfDom       )
              && ( _transitSurfIntl      == rhs._transitSurfIntl      )
              && ( _transitOfflineCxr    == rhs._transitOfflineCxr    )
              && ( _flightArrivalHours   == rhs._flightArrivalHours   )
              && ( _flightArrivalMinutes == rhs._flightArrivalMinutes )
              && ( _flightDepartHours    == rhs._flightDepartHours    )
              && ( _flightDepartMinutes  == rhs._flightDepartMinutes  )
              && ( _viaLocType           == rhs._viaLocType           )
              && ( _viaLoc               == rhs._viaLoc               )
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

    static void dummyData( TaxRestrictionTransit & obj )
    {
      obj._orderNo              = 1          ;
      obj._transitHours         = 2          ;
      obj._transitMinutes       = 3          ;
      obj._sameDayInd           = 'A'        ;
      obj._nextDayInd           = 'B'        ;
      obj._sameFlight           = 'C'        ;
      obj._transitTaxonly       = 'D'        ;
      obj._transitDomDom        = 'E'        ;
      obj._transitDomIntl       = 'F'        ;
      obj._transitIntlDom       = 'G'        ;
      obj._transitIntlIntl      = 'H'        ;
      obj._transitSurfDom       = 'U'        ;
      obj._transitSurfIntl      = 'J'        ;
      obj._transitOfflineCxr    = 'K'        ;
      obj._flightArrivalHours   = 4          ;
      obj._flightArrivalMinutes = 5          ;
      obj._flightDepartHours    = 6          ;
      obj._flightDepartMinutes  = 7          ;
      obj._viaLocType           = NATION     ;
      obj._viaLoc               = "aaaaaaaa" ;
    }

private:

    int       _orderNo              ;
    int       _transitHours         ;
    int       _transitMinutes       ;
    Indicator _sameDayInd           ;
    Indicator _nextDayInd           ;
    Indicator _sameFlight           ;
    Indicator _transitTaxonly       ;
    Indicator _transitDomDom        ;
    Indicator _transitDomIntl       ;
    Indicator _transitIntlDom       ;
    Indicator _transitIntlIntl      ;
    Indicator _transitSurfDom       ;
    Indicator _transitSurfIntl      ;
    Indicator _transitOfflineCxr    ;
    int       _flightArrivalHours   ;
    int       _flightArrivalMinutes ;
    int       _flightDepartHours    ;
    int       _flightDepartMinutes  ;
    LocType   _viaLocType           ;
    LocCode   _viaLoc               ;

    template <typename B, typename T> static B& convert (B& buffer,
                                                         T ptr)
    {
      return buffer
             & ptr->_orderNo
             & ptr->_transitHours
             & ptr->_transitMinutes
             & ptr->_sameDayInd
             & ptr->_nextDayInd
             & ptr->_sameFlight
             & ptr->_transitTaxonly
             & ptr->_transitDomDom
             & ptr->_transitDomIntl
             & ptr->_transitIntlDom
             & ptr->_transitIntlIntl
             & ptr->_transitSurfDom
             & ptr->_transitSurfIntl
             & ptr->_transitOfflineCxr
             & ptr->_flightArrivalHours
             & ptr->_flightArrivalMinutes
             & ptr->_flightDepartHours
             & ptr->_flightDepartMinutes
             & ptr->_viaLocType
             & ptr->_viaLoc;
    }

public:

    void flattenize( Flattenizable::Archive & archive )
    {
      FLATTENIZE( archive, _orderNo              ) ;
      FLATTENIZE( archive, _transitHours         ) ;
      FLATTENIZE( archive, _transitMinutes       ) ;
      FLATTENIZE( archive, _sameDayInd           ) ;
      FLATTENIZE( archive, _nextDayInd           ) ;
      FLATTENIZE( archive, _sameFlight           ) ;
      FLATTENIZE( archive, _transitTaxonly       ) ;
      FLATTENIZE( archive, _transitDomDom        ) ;
      FLATTENIZE( archive, _transitDomIntl       ) ;
      FLATTENIZE( archive, _transitIntlDom       ) ;
      FLATTENIZE( archive, _transitIntlIntl      ) ;
      FLATTENIZE( archive, _transitSurfDom       ) ;
      FLATTENIZE( archive, _transitSurfIntl      ) ;
      FLATTENIZE( archive, _transitOfflineCxr    ) ;
      FLATTENIZE( archive, _flightArrivalHours   ) ;
      FLATTENIZE( archive, _flightArrivalMinutes ) ;
      FLATTENIZE( archive, _flightDepartHours    ) ;
      FLATTENIZE( archive, _flightDepartMinutes  ) ;
      FLATTENIZE( archive, _viaLocType           ) ;
      FLATTENIZE( archive, _viaLoc               ) ;
    }
};

}
