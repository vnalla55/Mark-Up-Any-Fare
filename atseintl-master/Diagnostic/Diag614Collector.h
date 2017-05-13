//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
#pragma once

#include "Diagnostic/DiagCollector.h"

namespace tse
{

class Diag614Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag614Collector(Diagnostic& root);
  Diag614Collector()
    : _farePathNumber(0),
      _puNumber(0),
      _fcNumber(0),
      _currentFarePathNumber(0),
      _currentPricingUnitNumber(0),
      _currentFareUsageNumber(0)
  {
  }

  virtual void initParam(Diagnostic& root) override;
  virtual void printHeader() override;
  virtual void printLine() override;
  virtual bool enableFilter(DiagnosticTypes diagType, int dtCount, size_t currentFarePathNumber) override;
  virtual bool enableFilter(DiagnosticTypes diagType,
                            uint32_t farePathNumber,
                            uint32_t pricingUnitNumber,
                            uint32_t fareUsageNumber);

  virtual Diag614Collector& operator<<(char x) override;
  virtual Diag614Collector& operator<<(int x) override;
  virtual Diag614Collector& operator<<(long x) override;
  virtual Diag614Collector& operator<<(float x) override;
  virtual Diag614Collector& operator<<(double x) override;
  virtual Diag614Collector& operator<<(uint16_t x) override;
  virtual Diag614Collector& operator<<(uint32_t x) override;
  virtual Diag614Collector& operator<<(uint64_t x) override;
  virtual Diag614Collector& operator<<(const char* x) override;
  virtual Diag614Collector& operator<<(const std::string& x) override;
  virtual Diag614Collector& operator<<(std::ostream& (*pf)(std::ostream&)) override;
  Diag614Collector& operator<<(const FarePath& farePath) override;

private:
  size_t _farePathNumber;
  uint16_t _puNumber;
  uint16_t _fcNumber;

  size_t _currentFarePathNumber;
  uint16_t _currentPricingUnitNumber;
  uint16_t _currentFareUsageNumber;
};

} /* end tse namespace */

