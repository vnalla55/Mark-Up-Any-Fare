//----------------------------------------------------------------------------
//	   Copyright 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  ----------------------------------------------------------------------------

#pragma once

#include "TseTypes.h"
#include "CategoryRuleInfo.h"

namespace tse {

class FootNoteCtrlInfo: public CategoryRuleInfo
{

public:

      FootNoteCtrlInfo ()
      : _fareTariff(0)
      , _category(0)
      , _seqNo(0)
      , _segcount(0)
      , _newseqNo(0)
      , _jointCarrierTblItemNo(0)
      , _owrt(' ')
      , _routingAppl(' ')
      , _inhibit(' ')
      { }

	  virtual bool process() { return false;};

    const Indicator     & inhibit              () const { return _inhibit               ; }
    const TariffNumber  & fareTariff           () const { return _fareTariff            ; }
    const Footnote      & footNote             () const { return _footNote              ; }
    const int           & category             () const { return _category              ; }
    const int           & seqNo                () const { return _seqNo                 ; }
    const DateTime      & expireDate           () const { return _expireDate            ; }
    const DateTime      & effDate              () const { return _effDate               ; }
    const DateTime      & discDate             () const { return _discDate              ; }
    const int           & segcount             () const { return _segcount              ; }
    const int           & newseqNo             () const { return _newseqNo              ; }
    const int           & jointCarrierTblItemNo() const { return _jointCarrierTblItemNo ; }
    const FareClassCode & fareClass            () const { return _fareClass             ; }
    const LocKey        & loc1                 () const { return _loc1                  ; }
    const LocKey        & loc2                 () const { return _loc2                  ; }
    const Indicator     & owrt                 () const { return _owrt                  ; }
    const Indicator     & routingAppl          () const { return _routingAppl           ; }
    const RoutingNumber & routing              () const { return _routing               ; }

    Indicator     & inhibit              () { return _inhibit               ; }
    TariffNumber  & fareTariff           () { return _fareTariff            ; }
    Footnote      & footNote             () { return _footNote              ; }
    int           & category             () { return _category              ; }
    int           & seqNo                () { return _seqNo                 ; }
    DateTime      & expireDate           () { return _expireDate            ; }
    DateTime      & effDate              () { return _effDate               ; }
    DateTime      & discDate             () { return _discDate              ; }
    int           & segcount             () { return _segcount              ; }
    int           & newseqNo             () { return _newseqNo              ; }
    int           & jointCarrierTblItemNo() { return _jointCarrierTblItemNo ; }
    FareClassCode & fareClass            () { return _fareClass             ; }
    LocKey        & loc1                 () { return _loc1                  ; }
    LocKey        & loc2                 () { return _loc2                  ; }
    Indicator     & owrt                 () { return _owrt                  ; }
    Indicator     & routingAppl          () { return _routingAppl           ; }
    RoutingNumber & routing              () { return _routing               ; }

    virtual bool operator==( const FootNoteCtrlInfo & rhs ) const
    {
      bool eq = (    ( CategoryRuleInfo::operator==( rhs )                     )
                  && ( _fareTariff               == rhs._fareTariff            )
                  && ( _footNote                 == rhs._footNote              )
                  && ( _category                 == rhs._category              )
                  && ( _seqNo                    == rhs._seqNo                 )
                  && ( _expireDate               == rhs._expireDate            )
                  && ( _effDate                  == rhs._effDate               )
                  && ( _discDate                 == rhs._discDate              )
                  && ( _segcount                 == rhs._segcount              )
                  && ( _newseqNo                 == rhs._newseqNo              )
                  && ( _jointCarrierTblItemNo    == rhs._jointCarrierTblItemNo )
                  && ( _fareClass                == rhs._fareClass             )
                  && ( _loc1                     == rhs._loc1                  )
                  && ( _loc2                     == rhs._loc2                  )
                  && ( _owrt                     == rhs._owrt                  )
                  && ( _routingAppl              == rhs._routingAppl           )
                  && ( _routing                  == rhs._routing               )
                  && ( _inhibit                  == rhs._inhibit               )
                ) ;

      return eq ;
    }

    void dummyData()
    {
      static DateTime currentTime(time(NULL));
      CategoryRuleInfo::dummyData() ;

      inhibit              () = 'A'           ;
      fareTariff           () =  1            ;
      footNote             () = "BC"          ;
      category             () =  2            ;
      seqNo                () =  3            ;
      expireDate           () = currentTime ;
      effDate              () = currentTime ;
      discDate             () = currentTime ;
      segcount             () =  4            ;
      newseqNo             () =  5            ;
      jointCarrierTblItemNo() =  6            ;
      fareClass            () = "DEFGHIJK"    ;
      loc1().loc           () = "LMNOPQRS"    ;
      loc1().locType       () = 'T'           ;
      loc2().loc           () = "UVWXYZab"    ;
      loc2().locType       () = 'c'           ;
      owrt                 () = 'd'           ;
      routingAppl          () = 'e'           ;
      routing              () = "FGHI"        ;
    }

    virtual void flattenize( Flattenizable::Archive & archive )
    {
      FLATTENIZE_BASE_OBJECT( archive, CategoryRuleInfo ) ;

      FLATTENIZE( archive, _fareTariff            ) ;
      FLATTENIZE( archive, _footNote              ) ;
      FLATTENIZE( archive, _category              ) ;
      FLATTENIZE( archive, _seqNo                 ) ;
      FLATTENIZE( archive, _expireDate            ) ;
      FLATTENIZE( archive, _effDate               ) ;
      FLATTENIZE( archive, _discDate              ) ;
      FLATTENIZE( archive, _segcount              ) ;
      FLATTENIZE( archive, _newseqNo              ) ;
      FLATTENIZE( archive, _jointCarrierTblItemNo ) ;
      FLATTENIZE( archive, _fareClass             ) ;
      FLATTENIZE( archive, _loc1                  ) ;
      FLATTENIZE( archive, _loc2                  ) ;
      FLATTENIZE( archive, _owrt                  ) ;
      FLATTENIZE( archive, _routingAppl           ) ;
      FLATTENIZE( archive, _routing               ) ;
      FLATTENIZE( archive, _inhibit               ) ;
    }

    WBuffer &write (WBuffer &os) const
    {
      return convert(os, this);
    }
    RBuffer &read (RBuffer &is)
    {
      return convert(is, this);
    }
protected:

    TariffNumber  _fareTariff            ;
    Footnote      _footNote              ;
    int           _category              ;
    int           _seqNo                 ;
    DateTime      _expireDate            ;
    DateTime      _effDate               ;
    DateTime      _discDate              ;
    int           _segcount              ;
    int           _newseqNo              ;
    int           _jointCarrierTblItemNo ;
    FareClassCode _fareClass             ;
    LocKey        _loc1                  ;
    LocKey        _loc2                  ;
    Indicator     _owrt                  ;
    Indicator     _routingAppl           ;
    RoutingNumber _routing               ;
    Indicator     _inhibit               ; // Inhibit now checked at App Level
private:
  template <typename B, typename T> static B& convert (B& buffer,
                                                       T ptr)
  {
    CategoryRuleInfo::convert(buffer, ptr);
    return buffer
           & ptr->_fareTariff
           & ptr->_footNote
           & ptr->_category
           & ptr->_seqNo
           & ptr->_expireDate
           & ptr->_effDate
           & ptr->_discDate
           & ptr->_segcount
           & ptr->_newseqNo
           & ptr->_jointCarrierTblItemNo
           & ptr->_fareClass
           & ptr->_loc1
           & ptr->_loc2
           & ptr->_owrt
           & ptr->_routingAppl
           & ptr->_routing
           & ptr->_inhibit;
  }
};

typedef std::pair<const FootNoteCtrlInfo*, bool> FootNoteCtrlInfoPair;
typedef std::vector<FootNoteCtrlInfoPair> FootNoteCtrlInfoVec;

// FB Display
typedef FootNoteCtrlInfo		FootNoteRecord2Info ;

}
