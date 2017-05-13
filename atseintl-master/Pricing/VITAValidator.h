#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/ValidatingCarrierUpdater.h"

#include <map>
#include <string>
#include <vector>

namespace tse
{
class ShoppingTrx;
class InterlineTicketCarrier;

class VITAValidator
{
public:
  VITAValidator(ShoppingTrx& _trx);
  VITAValidator(ShoppingTrx& _trx, const CarrierCode* carrier);

  bool operator()(const std::vector<int>& sops, const CarrierCode* carrier = nullptr);

private:
  void createInterlineTicketCarrier();
  bool showDiag910Vita() const;

private:
  struct VITAInfo
  {
    VITAInfo(bool result,
             const CarrierCode& validatingCarrier,
             const std::string& validationMessage)
      : _result(result),
        _validatingCarrier(validatingCarrier),
        _validationMessage(validationMessage)
    {
    }

    bool result() const { return _result; }
    const CarrierCode& validatingCarrier() const { return _validatingCarrier; }
    const std::string& validationMessage() const { return _validationMessage; }

  private:
    bool _result;
    CarrierCode _validatingCarrier;
    std::string _validationMessage;
  };

  typedef std::map<std::vector<int>, VITAInfo> SopVITAMap;
  typedef std::map<int, CarrierCode> FirstSopValidatingCarrierMap;

  ShoppingTrx& _trx;
  const CarrierCode* _carrier;
  ValidatingCarrierUpdater _validatingCarrier;
  InterlineTicketCarrier* _interlineTicketCarrierData;

  SopVITAMap _sopVITAResultMap;
  FirstSopValidatingCarrierMap _firstSopValidatingCarrierMap;
};
}

