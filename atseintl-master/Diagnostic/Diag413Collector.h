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

#include "DataModel/Fare.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class FareMarket;
class FareUsage;
class DifferentialData;

class Diag413Collector : public DiagCollector
{
public:
  explicit Diag413Collector(Diagnostic& root)
    : DiagCollector(root), _header(0), _paxTfare(nullptr), _reqPaxType(nullptr), _fareUsage(nullptr)
  {
  }
  Diag413Collector() : _header(0), _paxTfare(nullptr), _reqPaxType(nullptr), _fareUsage(nullptr) {}

  Diag413Collector& operator<<(const FareMarket& mkt) override;
  Diag413Collector& operator<<(const FareUsage& output) override;
  virtual void lineSkip(int) override;
  const std::vector<DifferentialData*>& differential(void) const throw(std::string&);

private:
  void findThruNumber(char buff[5], DifferentialData& df);

  int _header; // indicator to call header
  const PaxTypeFare* _paxTfare;
  const PaxType* _reqPaxType;
  const FareUsage* _fareUsage;

  enum DiffDiagHeader
  {
    HEADER = 1 // Common Header
  };

  void bkg413Header(DiffDiagHeader);
};

} // namespace tse

