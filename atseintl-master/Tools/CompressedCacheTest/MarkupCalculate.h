//----------------------------------------------------------------------------
//	   2012, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  ----------------------------------------------------------------------------

#pragma once

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include "TseTypes.h"
#include "LocKey.h"

#include "Flattenizable.h"
#include "CompressedDataUtils.h"

#define MARKUP_CALCULATE_MEMBERS \
  (_orderNo) \
  (_tvlEffDate) \
  (_tvlDiscDate) \
  (_negFareCalcSeq) \
  (_directionality) \
  (_loc1) \
  (_loc2) \
  (_bundledInd) \
  (_sellingFareInd) \
  (_sellingPercentNoDec) \
  (_sellingPercent) \
  (_netSellingInd) \
  (_sellingNoDec1) \
  (_sellingFareAmt1) \
  (_sellingCur1) \
  (_sellingNoDec2) \
  (_sellingFareAmt2) \
  (_sellingCur2) \
  (_percentMinNoDec) \
  (_percentMin) \
  (_percentMaxNoDec) \
  (_percentMax) \
  (_markupFareInd) \
  (_markupNoDec1) \
  (_markupMinAmt1) \
  (_markupMaxAmt1) \
  (_markupCur1) \
  (_markupNoDec2) \
  (_markupMinAmt2) \
  (_markupMaxAmt2) \
  (_markupCur2) \
  (_fareClass) \
  (_fareType) \
  (_seasonType) \
  (_dowType) \
  (_wholesalerNoDec1) \
  (_wholesalerFareAmt1) \
  (_wholesalerCur1) \
  (_wholesalerNoDec2) \
  (_wholesalerFareAmt2) \
  (_wholesalerCur2) \
  (_wholesalerFareInd) \
  (_wholesalerPercentNoDec) \
  (_wholesalerPercent) \
  (_psgType) \
  (_noSellInd)

namespace tse {

class MarkupCalculate
{
public:

      MarkupCalculate ()
      : _orderNo(0)
      , _negFareCalcSeq(0)
      , _directionality(' ')
      , _bundledInd(' ')
      , _sellingFareInd(' ')
      , _sellingPercentNoDec(0)
      , _sellingPercent(0)
      , _netSellingInd(' ')
      , _sellingNoDec1(0)
      , _sellingFareAmt1(0)
      , _sellingNoDec2(0)
      , _sellingFareAmt2(0)
      , _percentMinNoDec(0)
      , _percentMin(0)
      , _percentMaxNoDec(0)
      , _percentMax(0)
      , _markupFareInd(' ')
      , _markupNoDec1(0)
      , _markupMinAmt1(0)
      , _markupMaxAmt1(0)
      , _markupNoDec2(0)
      , _markupMinAmt2(0)
      , _markupMaxAmt2(0)
      , _seasonType(' ')
      , _dowType(' ')
      , _wholesalerNoDec1(0)
      , _wholesalerFareAmt1(0)
      , _wholesalerNoDec2(0)
      , _wholesalerFareAmt2(0)
      , _wholesalerFareInd(' ')
      , _wholesalerPercentNoDec(0)
      , _wholesalerPercent(0)
      , _noSellInd(' ')
      { }

    int &orderNo(){ return _orderNo;}
    const int &orderNo() const { return _orderNo; }

    DateTime &tvlEffDate(){ return _tvlEffDate;}
    const DateTime &tvlEffDate() const { return _tvlEffDate; }

    DateTime &tvlDiscDate(){ return _tvlDiscDate;}
    const DateTime &tvlDiscDate() const { return _tvlDiscDate; }

    int &negFareCalcSeq(){ return _negFareCalcSeq;}
    const int &negFareCalcSeq() const { return _negFareCalcSeq; }

    Indicator &directionality() { return _directionality;}
    const Indicator &directionality() const { return _directionality; }

    LocKey &loc1() { return _loc1;}
    const LocKey &loc1() const { return _loc1; }

    LocKey &loc2() { return _loc2;}
    const LocKey &loc2() const { return _loc2; }

    Indicator &bundledInd() { return _bundledInd;}
    const Indicator &bundledInd() const { return _bundledInd; }

    Indicator &sellingFareInd() { return _sellingFareInd;}
    const Indicator &sellingFareInd() const { return _sellingFareInd; }

    int &sellingPercentNoDec(){ return _sellingPercentNoDec;}
    const int &sellingPercentNoDec() const { return _sellingPercentNoDec; }

    Percent &sellingPercent(){ return _sellingPercent;}
    const Percent &sellingPercent() const { return _sellingPercent; }

    Indicator &netSellingInd() { return _netSellingInd;}
    const Indicator &netSellingInd() const { return _netSellingInd; }

    int &sellingNoDec1(){ return _sellingNoDec1;}
    const int &sellingNoDec1() const { return _sellingNoDec1; }

    MoneyAmount &sellingFareAmt1(){ return _sellingFareAmt1;}
    const MoneyAmount &sellingFareAmt1() const { return _sellingFareAmt1; }

    CurrencyCode &sellingCur1() { return _sellingCur1;}
    const CurrencyCode &sellingCur1() const { return _sellingCur1; }

    int &sellingNoDec2(){ return _sellingNoDec2;}
    const int &sellingNoDec2() const { return _sellingNoDec2; }

    MoneyAmount &sellingFareAmt2(){ return _sellingFareAmt2;}
    const MoneyAmount &sellingFareAmt2() const { return _sellingFareAmt2; }

    CurrencyCode &sellingCur2() { return _sellingCur2;}
    const CurrencyCode &sellingCur2() const { return _sellingCur2; }

    int &percentMinNoDec(){ return _percentMinNoDec;}
    const int &percentMinNoDec() const { return _percentMinNoDec; }

    Percent &percentMin(){ return _percentMin;}
    const Percent &percentMin() const { return _percentMin; }

    int &percentMaxNoDec(){ return _percentMaxNoDec;}
    const int &percentMaxNoDec() const { return _percentMaxNoDec; }

    Percent &percentMax(){ return _percentMax;}
    const Percent &percentMax() const { return _percentMax; }

    Indicator &markupFareInd() { return _markupFareInd;}
    const Indicator &markupFareInd() const { return _markupFareInd; }

    int &markupNoDec1(){ return _markupNoDec1;}
    const int &markupNoDec1() const { return _markupNoDec1; }

    MoneyAmount &markupMinAmt1(){ return _markupMinAmt1;}
    const MoneyAmount &markupMinAmt1() const { return _markupMinAmt1; }

    MoneyAmount &markupMaxAmt1(){ return _markupMaxAmt1;}
    const MoneyAmount &markupMaxAmt1() const { return _markupMaxAmt1; }

    CurrencyCode &markupCur1() { return _markupCur1;}
    const CurrencyCode &markupCur1() const { return _markupCur1; }

    int &markupNoDec2(){ return _markupNoDec2;}
    const int &markupNoDec2() const { return _markupNoDec2; }

    MoneyAmount &markupMinAmt2(){ return _markupMinAmt2;}
    const MoneyAmount &markupMinAmt2() const { return _markupMinAmt2; }

    MoneyAmount &markupMaxAmt2(){ return _markupMaxAmt2;}
    const MoneyAmount &markupMaxAmt2() const { return _markupMaxAmt2; }

    CurrencyCode &markupCur2() { return _markupCur2;}
    const CurrencyCode &markupCur2() const { return _markupCur2; }

    void setFareClass (const FareClassCode &fareClass) { _fareClass = fareClass; }
    const FareClassCode &fareClass() const { return _fareClass; }

    void setFareType (const FareTypeAbbrev &fareType) { _fareType = fareType;}
    const FareTypeAbbrev &fareType() const { return _fareType; }

    Indicator &seasonType() { return _seasonType;}
    const Indicator &seasonType() const { return _seasonType; }

    Indicator &dowType() { return _dowType;}
    const Indicator &dowType() const { return _dowType; }

    int &wholesalerNoDec1(){ return _wholesalerNoDec1;}
    const int &wholesalerNoDec1() const { return _wholesalerNoDec1; }

    MoneyAmount &wholesalerFareAmt1(){ return _wholesalerFareAmt1;}
    const MoneyAmount &wholesalerFareAmt1() const { return _wholesalerFareAmt1; }

    CurrencyCode &wholesalerCur1() { return _wholesalerCur1;}
    const CurrencyCode &wholesalerCur1() const { return _wholesalerCur1; }

    int &wholesalerNoDec2(){ return _wholesalerNoDec2;}
    const int &wholesalerNoDec2() const { return _wholesalerNoDec2; }

    MoneyAmount &wholesalerFareAmt2(){ return _wholesalerFareAmt2;}
    const MoneyAmount &wholesalerFareAmt2() const { return _wholesalerFareAmt2; }

    CurrencyCode &wholesalerCur2() { return _wholesalerCur2;}
    const CurrencyCode &wholesalerCur2() const { return _wholesalerCur2; }

    Indicator &wholesalerFareInd() { return _wholesalerFareInd;}
    const Indicator &wholesalerFareInd() const { return _wholesalerFareInd; }

    int &wholesalerPercentNoDec(){ return _wholesalerPercentNoDec;}
    const int &wholesalerPercentNoDec() const { return _wholesalerPercentNoDec; }

    Percent &wholesalerPercent(){ return _wholesalerPercent;}
    const Percent &wholesalerPercent() const { return _wholesalerPercent; }

    PaxTypeCode &psgType() { return _psgType;}
    const PaxTypeCode &psgType() const { return _psgType; }

    Indicator &noSellInd() { return _noSellInd;}
    const Indicator &noSellInd() const { return _noSellInd; }

    bool operator==( const MarkupCalculate & rhs ) const
    {
      return(    ( _orderNo                == rhs._orderNo                )
              && ( _tvlEffDate             == rhs._tvlEffDate             )
              && ( _tvlDiscDate            == rhs._tvlDiscDate            )
              && ( _negFareCalcSeq         == rhs._negFareCalcSeq         )
              && ( _directionality         == rhs._directionality         )
              && ( _loc1                   == rhs._loc1                   )
              && ( _loc2                   == rhs._loc2                   )
              && ( _bundledInd             == rhs._bundledInd             )
              && ( _sellingFareInd         == rhs._sellingFareInd         )
              && ( _sellingPercentNoDec    == rhs._sellingPercentNoDec    )
              && ( _sellingPercent         == rhs._sellingPercent         )
              && ( _netSellingInd          == rhs._netSellingInd          )
              && ( _sellingNoDec1          == rhs._sellingNoDec1          )
              && ( _sellingFareAmt1        == rhs._sellingFareAmt1        )
              && ( _sellingCur1            == rhs._sellingCur1            )
              && ( _sellingNoDec2          == rhs._sellingNoDec2          )
              && ( _sellingFareAmt2        == rhs._sellingFareAmt2        )
              && ( _sellingCur2            == rhs._sellingCur2            )
              && ( _percentMinNoDec        == rhs._percentMinNoDec        )
              && ( _percentMin             == rhs._percentMin             )
              && ( _percentMaxNoDec        == rhs._percentMaxNoDec        )
              && ( _percentMax             == rhs._percentMax             )
              && ( _markupFareInd          == rhs._markupFareInd          )
              && ( _markupNoDec1           == rhs._markupNoDec1           )
              && ( _markupMinAmt1          == rhs._markupMinAmt1          )
              && ( _markupMaxAmt1          == rhs._markupMaxAmt1          )
              && ( _markupCur1             == rhs._markupCur1             )
              && ( _markupNoDec2           == rhs._markupNoDec2           )
              && ( _markupMinAmt2          == rhs._markupMinAmt2          )
              && ( _markupMaxAmt2          == rhs._markupMaxAmt2          )
              && ( _markupCur2             == rhs._markupCur2             )
              && ( _fareClass              == rhs._fareClass              )
              && ( _fareType               == rhs._fareType               )
              && ( _seasonType             == rhs._seasonType             )
              && ( _dowType                == rhs._dowType                )
              && ( _wholesalerNoDec1       == rhs._wholesalerNoDec1       )
              && ( _wholesalerFareAmt1     == rhs._wholesalerFareAmt1     )
              && ( _wholesalerCur1         == rhs._wholesalerCur1         )
              && ( _wholesalerNoDec2       == rhs._wholesalerNoDec2       )
              && ( _wholesalerFareAmt2     == rhs._wholesalerFareAmt2     )
              && ( _wholesalerCur2         == rhs._wholesalerCur2         )
              && ( _wholesalerFareInd      == rhs._wholesalerFareInd      )
              && ( _wholesalerPercentNoDec == rhs._wholesalerPercentNoDec )
              && ( _wholesalerPercent      == rhs._wholesalerPercent      )
              && ( _psgType                == rhs._psgType                )
              && ( _noSellInd              == rhs._noSellInd              )
            ) ;
    }

    void dummyData()
    {
      static DateTime currentTime(time( NULL ));
      _orderNo                = 1            ;
      _tvlEffDate             = currentTime ;
      _tvlDiscDate            = currentTime ;
      _negFareCalcSeq         = 2            ;
      _directionality         = 'A'          ;

      _loc1.dummyData()             ;
      _loc2.dummyData()             ;

      _bundledInd             = 'B'          ;
      _sellingFareInd         = 'C'          ;
      _sellingPercentNoDec    = 3            ;
      _sellingPercent         = 4.444        ;
      _netSellingInd          = 'D'          ;
      _sellingNoDec1          = 5            ;
      _sellingFareAmt1        = 6.66         ;
      _sellingCur1            = "EFG"        ;
      _sellingNoDec2          = 7            ;
      _sellingFareAmt2        = 8.88         ;
      _sellingCur2            = "HIJ"        ;
      _percentMinNoDec        = 9            ;
      _percentMin             = 10.101       ;
      _percentMaxNoDec        = 11           ;
      _percentMax             = 12.121       ;
      _markupFareInd          = 'K'          ;
      _markupNoDec1           = 13           ;
      _markupMinAmt1          = 14.14        ;
      _markupMaxAmt1          = 15.15        ;
      _markupCur1             = "LMN"        ;
      _markupNoDec2           = 16           ;
      _markupMinAmt2          = 17.17        ;
      _markupMaxAmt2          = 18.18        ;
      _markupCur2             = "OPQ"        ;
      _fareClass              = "RSTUVWXY"   ;
      _fareType               = "aaa"   ;
      _seasonType             = 'Z'          ;
      _dowType                = 'a'          ;
      _wholesalerNoDec1       = 19           ;
      _wholesalerFareAmt1     = 20.20        ;
      _wholesalerCur1         = "bcd"        ;
      _wholesalerNoDec2       = 21           ;
      _wholesalerFareAmt2     = 22.22        ;
      _wholesalerCur2         = "efg"        ;
      _wholesalerFareInd      = 'h'          ;
      _wholesalerPercentNoDec = 23           ;
      _wholesalerPercent      = 24.242       ;
      _psgType                = "ijk"        ;
      _noSellInd              = 'l'          ;
    }

    WBuffer& write(WBuffer& os) const
    {
      return convert(os, this);
    }

    RBuffer& read(RBuffer& is)
    {
      return convert(is, this);
    }

protected:

    int                _orderNo                ;
    DateTime           _tvlEffDate             ;
    DateTime           _tvlDiscDate            ;
    int                _negFareCalcSeq         ;
    Indicator          _directionality         ;
    LocKey             _loc1                   ;
    LocKey             _loc2                   ;
    Indicator          _bundledInd             ;
    Indicator          _sellingFareInd         ;
    int                _sellingPercentNoDec    ;
    Percent            _sellingPercent         ;
    Indicator          _netSellingInd          ;
    int                _sellingNoDec1          ;
    MoneyAmount        _sellingFareAmt1        ;
    CurrencyCode       _sellingCur1            ;
    int                _sellingNoDec2          ;
    MoneyAmount        _sellingFareAmt2        ;
    CurrencyCode       _sellingCur2            ;
    int                _percentMinNoDec        ;
    Percent            _percentMin             ;
    int                _percentMaxNoDec        ;
    Percent            _percentMax             ;
    Indicator          _markupFareInd          ;
    int                _markupNoDec1           ;
    MoneyAmount        _markupMinAmt1          ;
    MoneyAmount        _markupMaxAmt1          ;
    CurrencyCode       _markupCur1             ;
    int                _markupNoDec2           ;
    MoneyAmount        _markupMinAmt2          ;
    MoneyAmount        _markupMaxAmt2          ;
    CurrencyCode       _markupCur2             ;
    FareClassCode     _fareClass              ;
    FareTypeAbbrev    _fareType               ;
    Indicator          _seasonType             ;
    Indicator          _dowType                ;
    int                _wholesalerNoDec1       ;
    MoneyAmount        _wholesalerFareAmt1     ;
    CurrencyCode       _wholesalerCur1         ;
    int                _wholesalerNoDec2       ;
    MoneyAmount        _wholesalerFareAmt2     ;
    CurrencyCode       _wholesalerCur2         ;
    Indicator          _wholesalerFareInd      ;
    int                _wholesalerPercentNoDec ;
    Percent            _wholesalerPercent      ;
    PaxTypeCode        _psgType                ;
    Indicator          _noSellInd              ;

public:

    void flattenize( Flattenizable::Archive & archive )
    {
      #define READ_WRITE(r, d, member) FLATTENIZE( archive, member );
      BOOST_PP_SEQ_FOR_EACH(READ_WRITE, BOOST_PP_EMPTY(), MARKUP_CALCULATE_MEMBERS)
      #undef READ_WRITE
    }

protected:
private:
  template <typename B, typename T> static B& convert(B& buffer,
                                                      T ptr)
  {
  return buffer
         & ptr->_orderNo
         & ptr->_tvlEffDate
         & ptr->_tvlDiscDate
         & ptr->_negFareCalcSeq
         & ptr->_directionality
         & ptr->_loc1
         & ptr->_loc2
         & ptr->_bundledInd
         & ptr->_sellingFareInd
         & ptr->_sellingPercentNoDec
         & ptr->_sellingPercent
         & ptr->_netSellingInd
         & ptr->_sellingNoDec1
         & ptr->_sellingFareAmt1
         & ptr->_sellingCur1
         & ptr->_sellingNoDec2
         & ptr->_sellingFareAmt2
         & ptr->_sellingCur2
         & ptr->_percentMinNoDec
         & ptr->_percentMin
         & ptr->_percentMaxNoDec
         & ptr->_percentMax
         & ptr->_markupFareInd
         & ptr->_markupNoDec1
         & ptr->_markupMinAmt1
         & ptr->_markupMaxAmt1
         & ptr->_markupCur1
         & ptr->_markupNoDec2
         & ptr->_markupMinAmt2
         & ptr->_markupMaxAmt2
         & ptr->_markupCur2
         & ptr->_fareClass
         & ptr->_fareType
         & ptr->_seasonType
         & ptr->_dowType
         & ptr->_wholesalerNoDec1
         & ptr->_wholesalerFareAmt1
         & ptr->_wholesalerCur1
         & ptr->_wholesalerNoDec2
         & ptr->_wholesalerFareAmt2
         & ptr->_wholesalerCur2
         & ptr->_wholesalerFareInd
         & ptr->_wholesalerPercentNoDec
         & ptr->_wholesalerPercent
         & ptr->_psgType
         & ptr->_noSellInd;
  }
};
}
