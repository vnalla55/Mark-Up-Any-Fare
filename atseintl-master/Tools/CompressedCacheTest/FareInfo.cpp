//-------------------------------------------------------------------
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

#include "FareInfo.h"

using namespace tse;
//DateTime::emptyDate()
FareInfo::FareInfo()
  : _originalFareAmount( 0 ),
    _fareAmount( 0 ),
    _fareTariff( 0 ),
    _linkNumber( 0 ),
    _sequenceNumber( 0 ),
    _noDec( 0 ),
    _owrt( ' ' ),
    _directionality( TO ),
    _globalDirection( ZZ ),
    _constructionInd( ' ' ),
    _inhibit( ' ' ),
    _increasedFareAmtTag(' '),
    _reducedFareAmtTag(' '),
    _footnoteTag(' '),
    _routingTag(' '),
    _mpmTag(' '),
    _effectiveDateTag(' '),
    _currencyCodeTag(' '),
    _ruleTag(' '),
    _pAdditionalInfoContainer(0),
    _vendorFWS(false)
{
}

FareInfo::~FareInfo ()
{
  delete _pAdditionalInfoContainer;
}

FareInfo* FareInfo::clone( DataHandle& dataHandle ) const
{
  FareInfo *cloneObj = 0;
  return cloneObj;
}

void FareInfo::clone( FareInfo& cloneObj ) const
{
  _clone(cloneObj);
}

void FareInfo::_clone (FareInfo &cloneObj) const
{
  _effInterval.cloneDateInterval( cloneObj.effInterval() );

  cloneObj._vendor             = _vendor;
  cloneObj._carrier            = _carrier;
  cloneObj._market1            = _market1;
  cloneObj._market2            = _market2;
  cloneObj._lastModDate        = _lastModDate;
  cloneObj._originalFareAmount = _originalFareAmount;
  cloneObj._fareAmount         = _fareAmount;
  cloneObj._fareClass          = _fareClass;
  cloneObj._fareTariff         = _fareTariff;
  cloneObj._linkNumber         = _linkNumber;
  cloneObj._sequenceNumber     = _sequenceNumber;
  cloneObj._noDec              = _noDec;
  cloneObj._currency           = _currency;
  cloneObj._footnote1          = _footnote1;
  cloneObj._footnote2          = _footnote2;
  cloneObj._owrt               = _owrt;
  cloneObj._directionality     = _directionality;
  cloneObj._ruleNumber         = _ruleNumber;
  cloneObj._routingNumber      = _routingNumber;
  cloneObj._globalDirection    = _globalDirection;
  cloneObj._constructionInd    = _constructionInd;
  cloneObj._inhibit            = _inhibit;
  cloneObj._increasedFareAmtTag = _increasedFareAmtTag;
  cloneObj._reducedFareAmtTag   = _reducedFareAmtTag;
  cloneObj._footnoteTag         = _footnoteTag;
  cloneObj._routingTag          = _routingTag;
  cloneObj._mpmTag              = _mpmTag;
  cloneObj._effectiveDateTag    = _effectiveDateTag;
  cloneObj._currencyCodeTag     = _currencyCodeTag;
  cloneObj._ruleTag             = _ruleTag;
  if (_pAdditionalInfoContainer != 0)
  {
    cloneObj._pAdditionalInfoContainer = 0;
  }
  cloneObj._vendorFWS           = _vendorFWS;
}

void FareInfo::dummyData( )
{
  static DateTime currentTime(time( NULL ));
  TSEDateInterval::dummyData( _effInterval ) ;

  _vendor                   = "ABCD"       ;
  _carrier                  = "EFG"        ;
  _market1                  = "aaaaaaaa"   ;
  _market2                  = "bbbbbbbb"   ;
  _lastModDate              =currentTime;
  _originalFareAmount       = 1.11         ;
  _fareAmount               = 2.22         ;
  _fareClass                = "HIJKLMNO"   ;
  _fareTariff               = 3            ;
  _linkNumber               = 4            ;
  _sequenceNumber           = 5            ;
  _noDec                    = 6            ;
  _currency                 = "PQR"        ;
  _footnote1                = "ST"         ;
  _footnote2                = "UV"         ;
  _owrt                     = 'W'          ;
  _directionality           = TO           ;
  _ruleNumber               = "XYZa"       ;
  _routingNumber            = "bcde"       ;
  _globalDirection          = US           ;
  _constructionInd          = 'f'          ;
  _inhibit                  = 'g'          ;
  _increasedFareAmtTag      = 'h'          ;
  _reducedFareAmtTag        = 'i'          ;
  _footnoteTag              = 'j'          ;
  _routingTag               = 'k'          ;
  _mpmTag                   = 'l'          ;
  _effectiveDateTag         = 'm'          ;
  _currencyCodeTag          = 'n'          ;
  _ruleTag                  = 'o'          ;

  _vendorFWS                = true;        ;
}
