//-------------------------------------------------------------------
//
//  File:	CategoryRuleInfo.h
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

#include "Flattenizable.h"

#include "TseTypes.h"
#include "LocKey.h"

#include "CategoryRuleItemInfoSet.h"

namespace tse
{
class CategoryRuleInfo
{
public:

    CategoryRuleInfo ()
    : _tariffNumber(0)
    , _categoryNumber(0)
    , _sequenceNumber(0)
    , _applInd(' ')
    , _hasCatStopovers(true)
    , _hasCatTransfers(true)
    { }

    virtual ~CategoryRuleInfo()
    {
        std::vector<CategoryRuleItemInfoSet*>::iterator cit(_categoryRuleItemInfoSet.begin()),
                                                        citend(_categoryRuleItemInfoSet.end());
        for ( ; cit != citend; ++cit)
        {
          delete *cit;
        }
    }

    void init ();

    uint16_t& categoryNumber() {return _categoryNumber;}
    const uint16_t& categoryNumber() const {return _categoryNumber;}

    uint32_t& sequenceNumber() {return _sequenceNumber;}
    const uint32_t& sequenceNumber() const {return _sequenceNumber;}

    VendorCode& vendorCode() { return _vendorCode; };
    const VendorCode& vendorCode() const { return _vendorCode; };

    TariffNumber& tariffNumber() { return _tariffNumber; };
    const TariffNumber& tariffNumber() const { return _tariffNumber; };

    CarrierCode& carrierCode() { return _carrierCode; };
    const CarrierCode& carrierCode() const { return _carrierCode; };

    DateTime &createDate() { return _createDate; };
    const DateTime &createDate() const { return _createDate; };


    LocKey& loc1() { return _location1; };
    const LocKey& loc1() const { return _location1; };

    LocKey& loc2() { return _location2; };
    const LocKey& loc2() const { return _location2; };

    RuleNumber& ruleNumber() { return _ruleNumber; };
    const RuleNumber& ruleNumber() const { return _ruleNumber; };

    std::vector<CategoryRuleItemInfoSet*> &categoryRuleItemInfoSet() { return _categoryRuleItemInfoSet; }
    const std::vector<CategoryRuleItemInfoSet*> &categoryRuleItemInfoSet() const { return _categoryRuleItemInfoSet; }

    Indicator &applInd() { return _applInd;}
    const Indicator &applInd() const { return _applInd; }

    virtual bool operator==( const CategoryRuleInfo & rhs ) const
    {
      bool eq = (    ( _createDate                     == rhs._createDate                     )
                  && ( _vendorCode                     == rhs._vendorCode                     )
                  && ( _tariffNumber                   == rhs._tariffNumber                   )
                  && ( _carrierCode                    == rhs._carrierCode                    )
                  && ( _ruleNumber                     == rhs._ruleNumber                     )
                  && ( _categoryNumber                 == rhs._categoryNumber                 )
                  && ( _sequenceNumber                 == rhs._sequenceNumber                 )
                  && ( _location1                      == rhs._location1                      )
                  && ( _location2                      == rhs._location2                      )
                  && ( _applInd                        == rhs._applInd                        )
		              && ( _hasCatStopovers                == rhs._hasCatStopovers                )
		              && ( _hasCatTransfers                == rhs._hasCatTransfers                )
                  && ( _categoryRuleItemInfoSet.size() == rhs._categoryRuleItemInfoSet.size() )
                ) ;

      for( size_t i = 0 ; ( eq && ( i < _categoryRuleItemInfoSet.size() ) ) ; ++i )
      {
        eq = ( *(_categoryRuleItemInfoSet[i]) == *(rhs._categoryRuleItemInfoSet[i]) ) ;
      }

      return eq ;
    }

    bool hasCat (unsigned catNumber) const
    {
      bool result(true);
      switch (catNumber)
      {
      case 8:
        result = _hasCatStopovers;
        break;
      case 9:
        result = _hasCatTransfers;
        break;
      default:
        break;
      }
      return result;
    }

    bool hasCatStopovers() const { return _hasCatStopovers; }

    bool hasCatTransfers() const { return _hasCatTransfers; }

    virtual void dummyData()
    {
      CategoryRuleItemInfoSet * criis1 = new CategoryRuleItemInfoSet() ;
      CategoryRuleItemInfoSet * criis2 = new CategoryRuleItemInfoSet() ;

      categoryRuleItemInfoSet().push_back( criis1 ) ;
      categoryRuleItemInfoSet().push_back( criis2 ) ;

      CategoryRuleItemInfo * crii1  = new CategoryRuleItemInfo()    ;
      CategoryRuleItemInfo * crii2  = new CategoryRuleItemInfo()    ;
      CategoryRuleItemInfo * crii3  = new CategoryRuleItemInfo()    ;
      CategoryRuleItemInfo * crii4  = new CategoryRuleItemInfo()    ;

      CategoryRuleItemInfo::dummyData( *crii1 ) ;
      CategoryRuleItemInfo::dummyData( *crii2 ) ;
      CategoryRuleItemInfo::dummyData( *crii3 ) ;
      CategoryRuleItemInfo::dummyData( *crii4 ) ;

      criis1->categoryRuleItemInfo().push_back( crii1 ) ;
      criis1->categoryRuleItemInfo().push_back( crii2 ) ;
      criis2->categoryRuleItemInfo().push_back( crii3 ) ;
      criis2->categoryRuleItemInfo().push_back( crii4 ) ;

      createDate           () = DateTime( 2010, 04, 01, 13, 45, 30, 10 ) ;
      vendorCode           () = "ABCD"       ;
      tariffNumber         () = 1            ;
      carrierCode          () = "EFG"        ;
      ruleNumber           () = "HIJK"       ;
      categoryNumber       () = 2            ;
      sequenceNumber       () = 3            ;
      loc1().loc           () = "LMNOPQRS"   ;
      loc1().locType       () = 'T'          ;
      loc2().loc           () = "UVWXYZab"   ;
      loc2().locType       () = 'c'          ;
      applInd              () = 'd'          ;
      _hasCatStopovers        = false        ;
      _hasCatTransfers        = true         ;
      init();
    }

    virtual void flattenize( Flattenizable::Archive & archive )
    {
      FLATTENIZE( archive, _categoryRuleItemInfoSet  ) ;
      FLATTENIZE( archive, _createDate               ) ;
      FLATTENIZE( archive, _vendorCode               ) ;
      FLATTENIZE( archive, _tariffNumber             ) ;
      FLATTENIZE( archive, _carrierCode              ) ;
      FLATTENIZE( archive, _ruleNumber               ) ;
      FLATTENIZE( archive, _categoryNumber           ) ;
      FLATTENIZE( archive, _sequenceNumber           ) ;
      FLATTENIZE( archive, _location1                ) ;
      FLATTENIZE( archive, _location2                ) ;
      FLATTENIZE( archive, _applInd                  ) ;
      FLATTENIZE( archive, _hasCatStopovers          ) ;
      FLATTENIZE( archive, _hasCatTransfers          ) ;
    }
private:

    std::vector<CategoryRuleItemInfoSet*> _categoryRuleItemInfoSet ;
    DateTime                              _createDate              ;
    VendorCode                            _vendorCode              ;
    TariffNumber                          _tariffNumber            ;
    CarrierCode	                          _carrierCode             ;
    RuleNumber                            _ruleNumber              ;
    uint16_t                              _categoryNumber          ;
    uint32_t                              _sequenceNumber          ;
    LocKey                                _location1               ;
    LocKey                                _location2               ;
    Indicator                             _applInd                 ;

    template <typename B, typename T> static B& convert (B& buffer,
                                                         T ptr)
    {
      return buffer
             & ptr->_categoryRuleItemInfoSet
             & ptr->_createDate
             & ptr->_vendorCode
             & ptr->_tariffNumber
             & ptr->_carrierCode
             & ptr->_ruleNumber
             & ptr->_categoryNumber
             & ptr->_sequenceNumber
             & ptr->_location1
             & ptr->_location2
             & ptr->_applInd
             & ptr->_hasCatStopovers
             & ptr->_hasCatTransfers;
    }

    bool init (unsigned int category);

    bool _hasCatStopovers,
         _hasCatTransfers;

    CategoryRuleInfo(const CategoryRuleInfo&);
    CategoryRuleInfo& operator= (const CategoryRuleInfo&);

};

} // tse namespace
