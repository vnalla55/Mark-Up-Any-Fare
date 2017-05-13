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

#ifndef TSE_FAREINFO_H
#define TSE_FAREINFO_H

#include <vector>
#include "CompressedDataUtils.h"

namespace tse
{
  class AdditionalInfoContainer
  {};
  class FareInfo ;
}

#include "TseTypes.h"
#include "TSEDateInterval.h"

//-------------------------------------------------------------------
// ** NOTICE **
//
// This class has a clone method, if you add member variables
// please make sure you update the clone method as well!
//-------------------------------------------------------------------

namespace tse {

class DataHandle;

class FareInfo
{
public:

  FareInfo();
  virtual ~FareInfo ();

  /**
   * This methods obtains a new FareInfo pointer from
   * the data handle and populates it to be 'equal'
   * to the current object
   *
   * @param dataHandle
   *
   * @return new object
   */
  virtual FareInfo* clone( DataHandle& dataHandle ) const;

  /**
   * This methods populates a given FareInfo to be
   * 'equal' to the current object
   *
   * @param FareInfo - object to populate
   */
  virtual void clone( FareInfo& cloneObj ) const;


  virtual const bool isSITA() const { return false; };

  // accessors
  // =========

  TSEDateInterval& effInterval(){ return _effInterval;}
  const TSEDateInterval& effInterval() const { return _effInterval; }

  DateTime& createDate(){ return _effInterval.createDate();}
  const DateTime& createDate() const { return _effInterval.createDate(); }

  DateTime& effDate(){ return _effInterval.effDate();}
  const DateTime& effDate() const { return _effInterval.effDate(); }

  DateTime& expireDate(){ return _effInterval.expireDate(); }
  const DateTime& expireDate() const { return _effInterval.expireDate(); }

  DateTime& discDate(){ return _effInterval.discDate();}
  const DateTime& discDate() const { return _effInterval.discDate(); }

  const VendorCode& vendor() const { return _vendor; };
  VendorCode& vendor() { return _vendor; };

  const CarrierCode& carrier() const { return _carrier; };
  CarrierCode& carrier() { return _carrier; };

  const LocCode& market1() const { return _market1; };
  LocCode& market1() { return _market1; };

  const LocCode& market2() const { return _market2; };
  LocCode& market2() { return _market2; };

  const DateTime& lastModDate() const { return _lastModDate; }
  DateTime& lastModDate() { return _lastModDate; }

  const  MoneyAmount originalFareAmount() const { return _originalFareAmount; }
  MoneyAmount& originalFareAmount() { return _originalFareAmount; }

  const MoneyAmount fareAmount() const { return _fareAmount; }
  MoneyAmount& fareAmount() { return _fareAmount; }

  const FareClassCode& fareClass() const { return _fareClass; }
  FareClassCode& fareClass() { return _fareClass; }

  const TariffNumber fareTariff() const { return _fareTariff; }
  TariffNumber& fareTariff() { return _fareTariff; }

  const LinkNumber linkNumber() const { return _linkNumber; }
  LinkNumber& linkNumber() { return _linkNumber; }

  const SequenceNumber sequenceNumber() const { return _sequenceNumber; }
  SequenceNumber& sequenceNumber() { return _sequenceNumber; }

  const int noDec() const { return _noDec; }
  int& noDec() { return _noDec; };

  const CurrencyCode& currency() const { return _currency; };
  CurrencyCode& currency() { return _currency; };

  const Footnote& footNote1() const { return _footnote1; };
  Footnote& footNote1() { return _footnote1; };

  const Footnote& footNote2() const { return _footnote2; };
  Footnote& footNote2() { return _footnote2; };

  const Indicator owrt() const { return _owrt; };
  Indicator& owrt() { return _owrt; };

  const Directionality& directionality() const { return _directionality; };
  Directionality& directionality() { return _directionality; };

  const RuleNumber& ruleNumber() const { return _ruleNumber; };
  RuleNumber& ruleNumber() { return _ruleNumber; };

  const RoutingNumber& routingNumber() const { return _routingNumber; };
  RoutingNumber& routingNumber() { return _routingNumber; };

  const GlobalDirection globalDirection() const { return _globalDirection; };
  GlobalDirection& globalDirection() { return _globalDirection; };

  Indicator& constructionInd() { return _constructionInd; }
  const Indicator constructionInd() const { return _constructionInd; }

  const Indicator inhibit() const {return _inhibit;};
  Indicator& inhibit() {return _inhibit;};

  const Indicator increasedFareAmtTag() const {return _increasedFareAmtTag;};
  Indicator& increasedFareAmtTag() {return _increasedFareAmtTag;};

  const Indicator reducedFareAmtTag() const {return _reducedFareAmtTag;};
  Indicator& reducedFareAmtTag() {return _reducedFareAmtTag;};

  const Indicator footnoteTag() const {return _footnoteTag;};
  Indicator& footnoteTag() {return _footnoteTag;};

  const Indicator routingTag() const {return _routingTag;};
  Indicator& routingTag() {return _routingTag;};

  const Indicator mpmTag() const {return _mpmTag;};
  Indicator& mpmTag() {return _mpmTag;};

  const Indicator effectiveDateTag() const {return _effectiveDateTag;};

  Indicator& effectiveDateTag() {return _effectiveDateTag;};

  const Indicator currencyCodeTag() const {return _currencyCodeTag;};
  Indicator& currencyCodeTag() {return _currencyCodeTag;};

  const Indicator ruleTag() const {return _ruleTag;};
  Indicator& ruleTag() {return _ruleTag;};

  const bool vendorFWS() const {return _vendorFWS;}
  bool& vendorFWS() {return _vendorFWS;}

  bool checkBookingCodes () const;
  bool isWebFare (bool bTravelocity) const;
  bool isExpediaWebFare() const;
  TariffNumber getRuleTariff () const;
  TariffNumber getRoutingTariff () const;
  const PaxTypeCode &getPaxType () const;
  const FareType &getFareType () const;
  Indicator getTariffType () const;
  Indicator getDomInternInd() const;
  Indicator negViaAppl () const;
  Indicator nonstopDirectInd () const;
  const bool sameCarrier102() const;
  const bool sameCarrier103() const;
  const bool sameCarrier104() const;

public:
//protected:

  TSEDateInterval           _effInterval              ;
  VendorCode                _vendor                   ; // fare/rules vendor
  CarrierCode               _carrier                  ; // carrier code
  LocCode                   _market1                  ; // origin or destination market
  LocCode                   _market2                  ; // origin or destination market
  DateTime                  _lastModDate              ; // last date modified
  MoneyAmount               _originalFareAmount       ; // published amount of the fare
  MoneyAmount               _fareAmount               ; // one way amount of the fare
  FareClassCode             _fareClass                ; // fare class code
  TariffNumber              _fareTariff               ; // fare tariff
  LinkNumber                _linkNumber               ; // link number
  SequenceNumber            _sequenceNumber           ; // sequence number
  CurrencyNoDec             _noDec                    ; // number of decimal places to be
  CurrencyCode              _currency                 ; // currency code
  Footnote                  _footnote1                ; // footnote #1
  Footnote                  _footnote2                ; // footnote #2
  Indicator                 _owrt                     ; // one-way/round-trip indicator
  Directionality            _directionality           ; // fare directionality (from/to _market1)
  RuleNumber                _ruleNumber               ; // rule number
  RoutingNumber             _routingNumber            ; // routing number
  GlobalDirection           _globalDirection          ;// applicable global direction
  Indicator                 _constructionInd          ;// SMF & SITA only
  Indicator                 _inhibit                  ; // Inhibit now checked at App Level

  // Change tags                                           intl         dom
  Indicator                 _increasedFareAmtTag      ; //   2           1
  Indicator                 _reducedFareAmtTag        ; //   3           2
  Indicator                 _footnoteTag              ; //   5           3
  Indicator                 _routingTag               ; //   6           4
  Indicator                 _mpmTag                   ; //   8           5
  Indicator                 _effectiveDateTag         ; //  10          N/A
  Indicator                 _currencyCodeTag          ; //  17          N/A
  Indicator                 _ruleTag                  ; //  N/A         11

  AdditionalInfoContainer * _pAdditionalInfoContainer ;
  bool                      _vendorFWS;

  virtual bool operator==( const FareInfo & rhs ) const
  {
    bool eq = (    ( _effInterval                     == rhs._effInterval                     )
                && ( _vendor                          == rhs._vendor                          )
                && ( _carrier                         == rhs._carrier                         )
                && ( _market1                         == rhs._market1                         )
                && ( _market2                         == rhs._market2                         )
                && ( _lastModDate                     == rhs._lastModDate                     )
                && ( _originalFareAmount              == rhs._originalFareAmount              )
                && ( _fareAmount                      == rhs._fareAmount                      )
                && ( _fareClass                       == rhs._fareClass                       )
                && ( _fareTariff                      == rhs._fareTariff                      )
                && ( _linkNumber                      == rhs._linkNumber                      )
                && ( _sequenceNumber                  == rhs._sequenceNumber                  )
                && ( _noDec                           == rhs._noDec                           )
                && ( _currency                        == rhs._currency                        )
                && ( _footnote1                       == rhs._footnote1                       )
                && ( _footnote2                       == rhs._footnote2                       )
                && ( _owrt                            == rhs._owrt                            )
                && ( _directionality                  == rhs._directionality                  )
                && ( _ruleNumber                      == rhs._ruleNumber                      )
                && ( _routingNumber                   == rhs._routingNumber                   )
                && ( _globalDirection                 == rhs._globalDirection                 )
                && ( _constructionInd                 == rhs._constructionInd                 )
                && ( _inhibit                         == rhs._inhibit                         )
                && ( _increasedFareAmtTag             == rhs._increasedFareAmtTag             )
                && ( _reducedFareAmtTag               == rhs._reducedFareAmtTag               )
                && ( _footnoteTag                     == rhs._footnoteTag                     )
                && ( _routingTag                      == rhs._routingTag                      )
                && ( _mpmTag                          == rhs._mpmTag                          )
                && ( _effectiveDateTag                == rhs._effectiveDateTag                )
                && ( _currencyCodeTag                 == rhs._currencyCodeTag                 )
                && ( _ruleTag                         == rhs._ruleTag                         )
                && ( _vendorFWS                       == rhs._vendorFWS                       )
          ) ;

    return eq ;
  }
  virtual void dummyData( );

  virtual WBuffer& write(WBuffer& os,
                         size_t* memSize = 0) const
  {
    os.write('A');
    if (memSize)
    {
      *memSize += sizeof(FareInfo);
    }
    return convert(os, this);
  }

  virtual RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

protected:
  void dumpFareInfo( std::ostream & os ) const;

  template <typename B, typename T> static B& convert(B& buffer,
                                                      T ptr)
  {
    return buffer
           & ptr->_effInterval
           & ptr->_vendor
           & ptr->_carrier
           & ptr->_market1
           & ptr->_market2
           & ptr->_lastModDate
           & ptr->_originalFareAmount
           & ptr->_fareAmount
           & ptr->_fareClass
           & ptr->_fareTariff
           & ptr->_linkNumber
           & ptr->_sequenceNumber
           & ptr->_noDec
           & ptr->_currency
           & ptr->_footnote1
           & ptr->_footnote2
           & ptr->_owrt
           & ptr->_directionality
           & ptr->_ruleNumber
           & ptr->_routingNumber
           & ptr->_globalDirection
           & ptr->_constructionInd
           & ptr->_inhibit
           & ptr->_increasedFareAmtTag
           & ptr->_reducedFareAmtTag
           & ptr->_footnoteTag
           & ptr->_routingTag
           & ptr->_mpmTag
           & ptr->_effectiveDateTag
           & ptr->_currencyCodeTag
           & ptr->_ruleTag
           & ptr->_pAdditionalInfoContainer
           & ptr->_vendorFWS;
  }

private:
  void _clone (FareInfo &cloneObj) const;

  // Placed here so the clone methods must be used
  // ====== ==== == === ===== ======= ==== == ====

  FareInfo( const FareInfo& rhs );
  FareInfo& operator = ( const FareInfo& rhs );
};

typedef std::vector<const FareInfo*> FareInfoVec;
typedef FareInfoVec::iterator FareInfoVecI;

} // namespace tse

#endif // !defined( TSE_FAREINFO_H )
