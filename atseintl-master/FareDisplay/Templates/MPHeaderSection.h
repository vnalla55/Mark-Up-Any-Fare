#pragma once

#include "FareDisplay/Templates/Section.h"

namespace tse
{

class FareDisplayTrx;
class DateTime;

class MPHeaderSection : public Section
{
public:
  MPHeaderSection(FareDisplayTrx& trx) : Section(trx) {}
  void buildDisplay() override;

private:
  static std::string formatDate(const DateTime&);
};

} // namespace tse
