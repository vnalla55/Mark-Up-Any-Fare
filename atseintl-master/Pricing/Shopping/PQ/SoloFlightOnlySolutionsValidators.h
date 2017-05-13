// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Pricing/Shopping/PQ/SoloFlightOnlySolutionsDataTypes.h"


#include <map>
#include <string>
#include <vector>

namespace tse
{
class Diag910Collector;
class InterlineTicketCarrier;
class Itin;
class ShoppingTrx;
}

namespace tse
{
namespace fos
{

class VITAValidatorFOS
{

  struct ValidationResultInfo
  {
    ValidationResultInfo(SopsCombination sopVec,
                         Itin* itin,
                         bool validationResult,
                         const std::string& validationMessage)
      : _sopVec(sopVec),
        _itin(itin),
        _validationResult(validationResult),
        _validationMessage(validationMessage)
    {
    }

    SopsCombination _sopVec;
    Itin* _itin;
    bool _validationResult;
    std::string _validationMessage;
  };

  typedef std::vector<ValidationResultInfo> ValidationResultVec;
  typedef std::map<CarrierCode, ValidationResultVec> ResultPerCarrierMap;

public:
  VITAValidatorFOS(ShoppingTrx& trx);
  bool operator()(SopsCombination& sopVec);

  void printDiagnostics();

private:
  void addValidationResult(SopsCombination& sopVec,
                           Itin* itin,
                           bool validationResult,
                           const std::string& validationMessage);

private:
  ShoppingTrx& _trx;
  Diag910Collector* _diag910;
  InterlineTicketCarrier* _interlineTicketCxrData;
  ResultPerCarrierMap _resultsPerCxrMap;
};

class Validator
{
public:
  Validator(ShoppingTrx& trx)
    : _trx(trx),
      _vitaValidator(trx)
  {
  }

  virtual ~Validator() {};
  virtual bool isValidSolution(SopsCombination& sopVec);
  virtual bool checkMinConnectionTime(SopsCombination& sopVec);
  virtual ShapeSolution::Type
  validateSolutionShape(SOPCollections& sopsCollection, SopsCombination& sopVec)
  {
    return ShapeSolution::VALID;
  }
  virtual bool checkOnlineFlightsCombination(const SopsCombination& sopVec);

  virtual bool checkDirectFlight(SopsCombination& sopVec) { return true; }

  void printDiagnostics();

protected:
  bool isValidSolutionCommon(SopsCombination& sopVec);

  virtual bool checkConnectingFlights(SopsCombination& sopVec) { return true; }
  virtual bool checkConnectingFlights(SopsCombination& sopVec, size_t noOfSegments);
  virtual bool checkConnectingCities(SopsCombination& sopVec) { return true; }

protected:
  ShoppingTrx& _trx;
  VITAValidatorFOS _vitaValidator;
};

class AltDatesValidator : public Validator
{
public:
  AltDatesValidator(ShoppingTrx& trx) : Validator(trx) {}
  virtual ~AltDatesValidator() {}

  virtual bool isValidSolution(SopsCombination& sopVec) override { return isValidSolutionCommon(sopVec); }

  virtual ShapeSolution::Type
  validateSolutionShape(SOPCollections& sopsCollection, SopsCombination& sopVec) override;

  virtual bool checkDirectFlight(SopsCombination& sopVec) override;

protected:
  bool isSnowman(const SOPDetailsVec& outbDetails, const SOPDetailsVec& inbDetails);
};


class SoloFlightOnlySolutionsValidators
{
public:
  enum ValidatorType
  {
    COMMON,
    ALTDATES
  };

  static Validator*
  getValidator(ShoppingTrx& trx, ValidatorType type);
};
} // namespace fos
} // namespace tse

