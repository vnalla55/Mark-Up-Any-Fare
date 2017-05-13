//-------------------------------------------------------------------
//
//  File:         TktFeesRequest.h
//  Created:
//  Authors:
//
//  Description:
//
//  Updates:
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

#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PricingRequest.h"

#include <string>
#include <vector>

namespace tse
{
class PaxType;
class Itin;

class TktFeesRequest : public PricingRequest
{
public:
  class FormOfPayment
  {
  public:
    FormOfPayment() 
      : _chargeAmount(0.0),
        _chargeAmountInRequest(true)
    {}
    FopBinNumber& fopBinNumber() { return _fopBinNumber; }
    const FopBinNumber& fopBinNumber() const { return _fopBinNumber; }

    std::string& type() { return _type; }
    const std::string& type() const { return _type; }

    MoneyAmount& chargeAmount() { return _chargeAmount; }
    const MoneyAmount& chargeAmount() const { return _chargeAmount; }

    bool& chargeAmountInRequest() { return _chargeAmountInRequest; }
    const bool& chargeAmountInRequest() const { return _chargeAmountInRequest; }


  private:
    FopBinNumber _fopBinNumber;
    std::string _type;
    MoneyAmount _chargeAmount;
    bool        _chargeAmountInRequest;
  };

  class PassengerIdentity
  {
  public:
    PassengerIdentity() : _firstNameNumber(0), _surNameNumber(0), _pnrNameNumber(0), _objectId(0) {}

    SequenceNumber& firstNameNumber() { return _firstNameNumber; }
    const SequenceNumber& firstNameNumber() const { return _firstNameNumber; }

    SequenceNumber& surNameNumber() { return _surNameNumber; }
    const SequenceNumber& surNameNumber() const { return _surNameNumber; }

    SequenceNumber& pnrNameNumber() { return _pnrNameNumber; }
    const SequenceNumber& pnrNameNumber() const { return _pnrNameNumber; }

    SequenceNumber& objectId() { return _objectId; }
    const SequenceNumber& objectId() const { return _objectId; }

  private:
    SequenceNumber _firstNameNumber;
    SequenceNumber _surNameNumber;
    SequenceNumber _pnrNameNumber;
    SequenceNumber _objectId;
  };

  class PassengerPaymentInfo
  {
  public:
    PassengerPaymentInfo() : _paxRefObjectID(0) {}

    std::vector<FormOfPayment*>& fopVector() { return _fopVector; }
    const std::vector<FormOfPayment*>& fopVector() const { return _fopVector; }

    SequenceNumber& paxRefObjectID() { return _paxRefObjectID; }
    const SequenceNumber& paxRefObjectID() const { return _paxRefObjectID; }

  private:
    std::vector<FormOfPayment*> _fopVector;
    SequenceNumber _paxRefObjectID;
  };

  class PaxTypePayment
  {
  public:
    PaxTypePayment() : _paxType(nullptr), _amount(0), _currency(""), _noDec(0), _tktOverridePoint("") {}
    PaxType*& paxType() { return _paxType; }
    const PaxType* paxType() const { return _paxType; }

    MoneyAmount& amount() { return _amount; }
    const MoneyAmount& amount() const { return _amount; }

    CurrencyCode& currency() { return _currency; }
    const CurrencyCode& currency() const { return _currency; }

    CurrencyNoDec& noDec() { return _noDec; }
    const CurrencyNoDec& noDec() const { return _noDec; }

    LocCode& tktOverridePoint() { return _tktOverridePoint; }
    const LocCode& tktOverridePoint() const { return _tktOverridePoint; }

    std::vector<PassengerPaymentInfo*>& ppiV() { return _ppiV; }
    const std::vector<PassengerPaymentInfo*>& ppiV() const { return _ppiV; }

  private:
    PaxType* _paxType;
    MoneyAmount _amount;
    CurrencyCode _currency;
    CurrencyNoDec _noDec;
    LocCode _tktOverridePoint;
    std::vector<PassengerPaymentInfo*> _ppiV;
  };

  class TktFeesDifferentialData
  {
  public:
    CarrierCode& diffCarrierCode() { return _diffCarrierCode; }
    const CarrierCode& diffCarrierCode() const { return _diffCarrierCode; }

    FareClassCode& fareBasis() { return _fareBasis; }
    const FareClassCode& fareBasis() const { return _fareBasis; }

    VendorCode& vendorCode() { return _vendorCode; }
    const VendorCode& vendorCode() const { return _vendorCode; }

    TktFeesDifferentialData() : _vendorCode("ATP") {}

  private:
    CarrierCode _diffCarrierCode;
    FareClassCode _fareBasis;
    VendorCode _vendorCode;
  };

  class TktFeesFareBreakInfo
  {
  public:
    SequenceNumber& fareComponentID() { return _fareComponentID; }
    const SequenceNumber& fareComponentID() const { return _fareComponentID; }

    CarrierCode& governingCarrier() { return _airlineCode; }
    const CarrierCode& governingCarrier() const { return _airlineCode; }

    FareClassCode& fareBasis() { return _fareBasis; }
    const FareClassCode& fareBasis() const { return _fareBasis; }

    VendorCode& vendorCode() { return _vendorCode; }
    const VendorCode& vendorCode() const { return _vendorCode; }

    void setForbidCreditCard(bool cc) { _forbidCreditCard = cc; }
    bool forbidCreditCard() const { return _forbidCreditCard; }

    std::vector<TktFeesDifferentialData*>& tktFeesDiffData() { return _tktFeesDiffData; }
    const std::vector<TktFeesDifferentialData*>& tktFeesDiffData() const
    {
      return _tktFeesDiffData;
    }

    TktFeesFareBreakInfo() : _fareComponentID(-1), _vendorCode("ATP") {}

  private:
    SequenceNumber _fareComponentID;
    CarrierCode _airlineCode;
    FareClassCode _fareBasis;
    VendorCode _vendorCode;
    bool _forbidCreditCard = false;
    std::vector<TktFeesDifferentialData*> _tktFeesDiffData;
  };
  class TktFeesFareBreakAssociation
  {
  public:
    SequenceNumber& segmentID() { return _segmentID; }
    const SequenceNumber& segmentID() const { return _segmentID; }

    SequenceNumber& fareComponentID() { return _fareComponentID; }
    const SequenceNumber& fareComponentID() const { return _fareComponentID; }

    SequenceNumber& sideTripID() { return _sideTripID; }
    const SequenceNumber& sideTripID() const { return _sideTripID; }

    bool& sideTripStart() { return _sideTripStart; }
    const bool& sideTripStart() const { return _sideTripStart; }

    bool& sideTripEnd() { return _sideTripEnd; }
    const bool& sideTripEnd() const { return _sideTripEnd; }

    TktDesignator& tktDesignator() { return _tktDesignator; }
    const TktDesignator& tktDesignator() const { return _tktDesignator; }

    TktFeesFareBreakAssociation()
      : _segmentID(0),
        _fareComponentID(0),
        _sideTripID(0),
        _sideTripStart(false),
        _sideTripEnd(false)
    {
    }

  private:
    SequenceNumber _segmentID;
    SequenceNumber _fareComponentID;
    SequenceNumber _sideTripID;
    bool _sideTripStart;
    bool _sideTripEnd;
    TktDesignator _tktDesignator;
  };

  TktFeesRequest() : _majorSchemaVersion(0) {}

  std::map<const Itin*, std::vector<PaxType*> >& paxTypesPerItin() { return _paxTypesPerItin; }
  const std::map<const Itin*, std::vector<PaxType*> >& paxTypesPerItin() const
  {
    return _paxTypesPerItin;
  }

  std::map<const Itin*, PaxTypePayment*>& paxTypePaymentPerItin() { return _paxTypePaymentPerItin; }
  const std::map<const Itin*, PaxTypePayment*>& paxTypePaymentPerItin() const
  {
    return _paxTypePaymentPerItin;
  }

  std::map<const Itin*, std::vector<std::string> >& corpIdPerItin() { return _corpIdPerItin; }
  const std::map<const Itin*, std::vector<std::string> >& corpIdPerItin() const
  {
    return _corpIdPerItin;
  }

  std::map<const Itin*, std::vector<std::string> >& invalidCorpIdPerItin()
  {
    return _invaliCorpIdPerItin;
  }
  const std::map<const Itin*, std::vector<std::string> >& invalidCorpIdPerItin() const
  {
    return _invaliCorpIdPerItin;
  }

  std::map<const Itin*, std::vector<std::string> >& accountCodeIdPerItin()
  {
    return _accountCodeIdPerItin;
  }
  const std::map<const Itin*, std::vector<std::string> >& accountCodeIdPerItin() const
  {
    return _accountCodeIdPerItin;
  }

  std::map<const Itin*, std::vector<TktFeesFareBreakInfo*> >& tktFeesFareBreakPerItin()
  {
    return _tktFeesFareBreakPerItin;
  }
  const std::map<const Itin*, std::vector<TktFeesFareBreakInfo*> >& tktFeesFareBreakPerItin() const
  {
    return _tktFeesFareBreakPerItin;
  }

  std::map<const Itin*, std::vector<TktFeesFareBreakAssociation*> >&
  tktFeesFareBreakAssociationPerItin()
  {
    return _tktFeesFareBreakAssociationPerItin;
  }
  const std::map<const Itin*, std::vector<TktFeesFareBreakAssociation*> >&
  tktFeesFareBreakAssociationPerItin() const
  {
    return _tktFeesFareBreakAssociationPerItin;
  }

  std::map<const Itin*, std::map<int16_t, TktDesignator> >& tktDesignatorPerItin()
  {
    return _tktDesignatorPerItin;
  }
  const std::map<const Itin*, std::map<int16_t, TktDesignator> >& tktDesignatorPerItin() const
  {
    return _tktDesignatorPerItin;
  }

  std::vector<PaxType*> paxType(const Itin* it) const;

  uint16_t& majorSchemaVersion() { return _majorSchemaVersion; }
  uint16_t majorSchemaVersion() const { return _majorSchemaVersion; }

  std::map<const Itin*, DateTime>& ticketingDatesPerItin() { return _ticketingDatesPerItin; }
  const std::map<const Itin*, DateTime>& ticketingDatesPerItin() const
  {
    return _ticketingDatesPerItin;
  }

  std::vector<PassengerIdentity*>& paxId() { return _paxId; }
  const std::vector<PassengerIdentity*>& paxId() const { return _paxId; }

  std::string& fullVersionRQ() { return _fullVersionRQ; }
  const std::string& fullVersionRQ() const { return _fullVersionRQ; }

protected:
  TktFeesRequest(const TktFeesRequest&);
  TktFeesRequest& operator=(const TktFeesRequest&);

private:
  std::map<const Itin*, std::vector<PaxType*> > _paxTypesPerItin;
  std::map<const Itin*, std::vector<std::string> > _corpIdPerItin;
  std::map<const Itin*, std::vector<std::string> > _invaliCorpIdPerItin;
  std::map<const Itin*, std::vector<std::string> > _accountCodeIdPerItin;
  std::map<const Itin*, std::vector<TktFeesFareBreakInfo*> > _tktFeesFareBreakPerItin;
  std::map<const Itin*, std::vector<TktFeesFareBreakAssociation*> >
  _tktFeesFareBreakAssociationPerItin;
  std::map<const Itin*, std::map<int16_t, TktDesignator> > _tktDesignatorPerItin;
  uint16_t _majorSchemaVersion;
  std::map<const Itin*, DateTime> _ticketingDatesPerItin;
  std::map<const Itin*, PaxTypePayment*> _paxTypePaymentPerItin;
  std::vector<PassengerIdentity*> _paxId;
  std::string _fullVersionRQ;
};

} // tse namespace

