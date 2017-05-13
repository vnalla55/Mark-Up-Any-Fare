// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include <string>

namespace tax
{
class InputRequest;
class InputApplyOn;
class InputRequestWithCache;
class InputProcessingOptions;
class InputExemptedRule;
class InputCalculationRestriction;
class InputChangeFee;
class InputTicketingFee;
class InputTicketingFeePath;
class InputTicketingFeeUsage;
class InputTicketingOptions;
class InputPointOfSale;
class InputGeoPath;
class InputPrevTicketGeoPath;
class InputGeo;
class InputFare;
class InputFarePath;
class InputFareUsage;
class InputOptionalServicePath;
class InputOptionalServiceUsage;
class InputYqYr;
class InputYqYrUsage;
class InputGeoPathMapping;
class InputMapping;
class InputCalculationRestrictionTax;
class InputMap;
class InputFlight;
struct InputFlightPath;
class InputItin;
class InputDiagnosticCommand;
class InputParameter;
class InputPassenger;
class InputTaxDetailsLevel;
class XmlCache;
class InputOptionalService;
class RulesRecord;
class ReportingRecord;
class RepricingEntry;
class Nation;
class InputYqYrPath;
class Currency;
class CurrencyConversion;
class IsInLoc;
class LocUsage;
class MileageGetterServer;
class Distance;
class InputFlightUsage;
class CarrierFlight;
class CarrierFlightSegment;
class AKHIFactor;
class CarrierApplication;
class CarrierApplicationEntry;
class ServiceBaggage;
class ServiceBaggageEntry;
class ReportingRecordEntry;
class PassengerTypeCodeItem;
template <class T> class CacheItem;
typedef CacheItem<PassengerTypeCodeItem> PassengerTypeCode;
class PaxLocUsage;
class PaxTypeMapping;
class PaxTypeMappingItem;
class SectorDetail;
class SectorDetailEntry;
class ServiceFeeSecurityItem;
typedef CacheItem<ServiceFeeSecurityItem> ServiceFeeSecurity;
class TaxRounding;
struct InputRequestWithStringCache;

class OutputBaggageDetails;
class OutputCalcDetails;
class DiagnosticResponse;
class OutputDiagnostic;
class ErrorMessage;
class OutputError;
class OutputFaresDetails;
class OutputGeoDetails;
class ItinPayments;
class OutputItin;
class ItinsPayments;
class OutputItins;
class Message;
class OutputMessage;
class OutputOBDetails;
class OutputOCDetails;
class OutputOptionalServiceDetails;
class OutputTax;
class Payment;
class PaymentDetail;
class OutputTaxDetails;
class OutputTaxDetailsRef;
class OutputTaxGroup;
class OutputTaxOnTaxDetails;
class OutputResponse;
class Response;
class OutputYQYRDetails;
class Customer;

class BCHOutputResponse;
class BCHOutputItin;
class BCHOutputItinPaxDetail;
class BCHOutputTaxDetail;

class XmlTagsList
{
public:
  template <typename T> std::string getTagName() const { return getTagName((T*)nullptr); }
  virtual ~XmlTagsList(){}

  virtual std::string getAttributeName_AgentAirlineDept()                       const = 0;
  virtual std::string getAttributeName_AgentCity()                              const = 0;
  virtual std::string getAttributeName_AgentOfficeDesignator()                  const = 0;
  virtual std::string getAttributeName_AgentPCC()                               const = 0;
  virtual std::string getAttributeName_AllDetails()                             const = 0;
  virtual std::string getAttributeName_Amount()                                 const = 0;
  virtual std::string getAttributeName_ArrivalDateShift()                       const = 0;
  virtual std::string getAttributeName_ArrivalTime()                            const = 0;
  virtual std::string getAttributeName_BasisCode()                              const = 0;
  virtual std::string getAttributeName_BookingCodeType()                        const = 0;
  virtual std::string getAttributeName_BufferZoneInd()                          const = 0;
  virtual std::string getAttributeName_CabinCode()                              const = 0;
  virtual std::string getAttributeName_CalcDetails()                            const = 0;
  virtual std::string getAttributeName_CarrierCode()                            const = 0;
  virtual std::string getAttributeName_Code()                                   const = 0;
  virtual std::string getAttributeName_ConnectionDateShift()                    const = 0;
  virtual std::string getAttributeName_DateOfBirth()                            const = 0;
  virtual std::string getAttributeName_DepartureTime()                          const = 0;
  virtual std::string getAttributeName_Directionality()                         const = 0;
  virtual std::string getAttributeName_DutyCode()                               const = 0;
  virtual std::string getAttributeName_EchoToken()                              const = 0;
  virtual std::string getAttributeName_Employment()                             const = 0;
  virtual std::string getAttributeName_Equipment()                              const = 0;
  virtual std::string getAttributeName_ExchangeReissueDetails()                 const = 0;
  virtual std::string getAttributeName_FarePathGeoPathMappingRefId()            const = 0;
  virtual std::string getAttributeName_FarePathRefId()                          const = 0;
  virtual std::string getAttributeName_FareRefId()                              const = 0;
  virtual std::string getAttributeName_FlightRefId()                            const = 0;
  virtual std::string getAttributeName_FlightPathRefId()                        const = 0;
  virtual std::string getAttributeName_FormOfPayment()                          const = 0;
  virtual std::string getAttributeName_FunctionCode()                           const = 0;
  virtual std::string getAttributeName_GeoDetails()                             const = 0;
  virtual std::string getAttributeName_GeoPathRefId()                           const = 0;
  virtual std::string getAttributeName_GeoRefId()                               const = 0;
  virtual std::string getAttributeName_IATANumber()                             const = 0;
  virtual std::string getAttributeName_Id()                                     const = 0;
  virtual std::string getAttributeName_IsNetRemitAvailable()                    const = 0;
  virtual std::string getAttributeName_Loc()                                    const = 0;
  virtual std::string getAttributeName_MarketingCarrier()                       const = 0;
  virtual std::string getAttributeName_MarketingCarrierFlightNumber()           const = 0;
  virtual std::string getAttributeName_MarkupAmount()                                 const = 0;
  virtual std::string getAttributeName_Name()                                   const = 0;
  virtual std::string getAttributeName_Nation()                                 const = 0;
  virtual std::string getAttributeName_Nationality()                            const = 0;
  virtual std::string getAttributeName_OneWayRoundTrip()                        const = 0;
  virtual std::string getAttributeName_OperatingCarrier()                       const = 0;
  virtual std::string getAttributeName_OptionalServicePathGeoPathMappingRefId() const = 0;
  virtual std::string getAttributeName_OptionalServicePathRefId()               const = 0;
  virtual std::string getAttributeName_OptionalServiceRefId()                   const = 0;
  virtual std::string getAttributeName_OutputPassengerCode()                    const = 0;
  virtual std::string getAttributeName_OwnerCarrier()                           const = 0;
  virtual std::string getAttributeName_PartitionId()                            const = 0;
  virtual std::string getAttributeName_PassengerCode()                          const = 0;
  virtual std::string getAttributeName_PassengerRefId()                         const = 0;
  virtual std::string getAttributeName_PaymentCurrency()                        const = 0;
  virtual std::string getAttributeName_PointOfDeliveryLoc()                     const = 0;
  virtual std::string getAttributeName_PointOfSaleLoc()                         const = 0;
  virtual std::string getAttributeName_PointOfSaleRefId()                       const = 0;
  virtual std::string getAttributeName_ProcessingGroup()                        const = 0;
  virtual std::string getAttributeName_ReservationDesignator()                  const = 0;
  virtual std::string getAttributeName_Residency()                              const = 0;
  virtual std::string getAttributeName_Rule()                                   const = 0;
  virtual std::string getAttributeName_RuleRefId()                              const = 0;
  virtual std::string getAttributeName_SellAmount()                             const = 0;
  virtual std::string getAttributeName_ServiceSubTypeCode()                     const = 0;
  virtual std::string getAttributeName_SkipDateTime()                           const = 0;
  virtual std::string getAttributeName_StateCode()                              const = 0;
  virtual std::string getAttributeName_SvcGroup()                               const = 0;
  virtual std::string getAttributeName_SvcSubGroup()                            const = 0;
  virtual std::string getAttributeName_Tariff()                                 const = 0;
  virtual std::string getAttributeName_TariffInd()                              const = 0;
  virtual std::string getAttributeName_TaxAmount()                              const = 0;
  virtual std::string getAttributeName_TaxIncluded()                            const = 0;
  virtual std::string getAttributeName_TaxOnExchangeReissueDetails()            const = 0;
  virtual std::string getAttributeName_TaxOnFaresDetails()                      const = 0;
  virtual std::string getAttributeName_TaxOnOptionalServicesDetails()           const = 0;
  virtual std::string getAttributeName_TaxOnTaxDetails()                        const = 0;
  virtual std::string getAttributeName_TaxOnYQYRDetails()                       const = 0;
  virtual std::string getAttributeName_TicketingDate()                          const = 0;
  virtual std::string getAttributeName_TicketingPoint()                         const = 0;
  virtual std::string getAttributeName_TicketingTime()                          const = 0;
  virtual std::string getAttributeName_TotalAmount()                            const = 0;
  virtual std::string getAttributeName_TotalAmountBeforeDiscount()              const = 0;
  virtual std::string getAttributeName_TravelOriginDate()                       const = 0;
  virtual std::string getAttributeName_Type()                                   const = 0;
  virtual std::string getAttributeName_TypeCode()                               const = 0;
  virtual std::string getAttributeName_UnticketedTransfer()                     const = 0;
  virtual std::string getAttributeName_ValidatingCarrier()                      const = 0;
  virtual std::string getAttributeName_Value()                                  const = 0;
  virtual std::string getAttributeName_VendorCrsCode()                          const = 0;
  virtual std::string getAttributeName_YqYrPathGeoPathMappingRefId()            const = 0;
  virtual std::string getAttributeName_YqYrPathRefId()                          const = 0;
  virtual std::string getAttributeName_YqYrRefId()                              const = 0;
  virtual std::string getAttributeName_ForcedConnection()                       const = 0;
  virtual std::string getAttributeName_ChangeFeeRefId()                         const = 0;
  virtual std::string getAttributeName_TicketingFeeRefId()                      const = 0;
  virtual std::string getAttributeName_GeoPathsSize()                           const = 0;
  virtual std::string getAttributeName_GeoPathMappingsSize()                    const = 0;
  virtual std::string getAttributeName_FaresSize()                              const = 0;
  virtual std::string getAttributeName_FarePathsSize()                          const = 0;
  virtual std::string getAttributeName_FlightsSize()                            const = 0;
  virtual std::string getAttributeName_FlightPathsSize()                        const = 0;
  virtual std::string getAttributeName_YqYrsSize()                              const = 0;
  virtual std::string getAttributeName_YqYrPathsSize()                          const = 0;
  virtual std::string getAttributeName_ItinsSize()                              const = 0;
  virtual std::string getAttributeName_ApplyUSCAGrouping()                      const = 0;

  std::string getAttributeName_ALTERNATERULEREFTAG()          const { return "ALTERNATERULEREFTAG";          }
  std::string getAttributeName_APPLIND()                      const { return "APPLIND";                      }
  std::string getAttributeName_APPLTAG()                      const { return "APPLTAG";                      }
  std::string getAttributeName_ATTRGROUP()                    const { return "ATTRGROUP";                    }
  std::string getAttributeName_ATTRSUBGROUP()                 const { return "ATTRSUBGROUP";                 }
  std::string getAttributeName_CABINCODE()                    const { return "CABINCODE";                    }
  std::string getAttributeName_CALCORDER()                    const { return "CALCORDER";                    }
  std::string getAttributeName_CARRIER()                      const { return "CARRIER";                      }
  std::string getAttributeName_CARRIERAPPLITEMNO1()           const { return "CARRIERAPPLITEMNO1";           }
  std::string getAttributeName_CARRIERFLTITEMNO1()            const { return "CARRIERFLTITEMNO1";            }
  std::string getAttributeName_CARRIERFLTITEMNO2()            const { return "CARRIERFLTITEMNO2";            }
  std::string getAttributeName_CODE()                         const { return "CODE";                         }
  std::string getAttributeName_CODETYPE()                     const { return "CODETYPE";                     }
  std::string getAttributeName_CONNECTIONSTAG1()              const { return "CONNECTIONSTAG1";              }
  std::string getAttributeName_CONNECTIONSTAG2()              const { return "CONNECTIONSTAG2";              }
  std::string getAttributeName_CONNECTIONSTAG3()              const { return "CONNECTIONSTAG3";              }
  std::string getAttributeName_CONNECTIONSTAG4()              const { return "CONNECTIONSTAG4";              }
  std::string getAttributeName_CONNECTIONSTAG5()              const { return "CONNECTIONSTAG5";              }
  std::string getAttributeName_CONNECTIONSTAG6()              const { return "CONNECTIONSTAG6";              }
  std::string getAttributeName_CONNECTIONSTAG7()              const { return "CONNECTIONSTAG7";              }
  std::string getAttributeName_CURRENCYOFSALE()               const { return "CURRENCYOFSALE";               }
  std::string getAttributeName_CXRGDSCODE()                   const { return "CXRGDSCODE";                   }
  std::string getAttributeName_DISCDATE()                     const { return "DISCDATE";                     }
  std::string getAttributeName_DUTYFUNCTIONCODE()             const { return "DUTYFUNCTIONCODE";             }
  std::string getAttributeName_EFFDATE()                      const { return "EFFDATE";                      }
  std::string getAttributeName_EQUIPMENTCODE()                const { return "EQUIPMENTCODE";                }
  std::string getAttributeName_ERSP()                         const { return "ERSP";                         }
  std::string getAttributeName_EXEMPTTAG()                    const { return "EXEMPTTAG";                    }
  std::string getAttributeName_EXEMPTDUFORJJ()                const { return "EXEMPTDUFORJJ";                }
  std::string getAttributeName_EXEMPTDUFORT4()                const { return "EXEMPTDUFORT4";                }
  std::string getAttributeName_EXEMPTDUFORG3()                const { return "EXEMPTDUFORG3";                }
  std::string getAttributeName_EXPIREDATE()                   const { return "EXPIREDATE";                   }
  std::string getAttributeName_FAREBASISTKTDESIGNATOR()       const { return "FAREBASISTKTDESIGNATOR";       }
  std::string getAttributeName_FAREOWNINGCXR()                const { return "FAREOWNINGCXR";                }
  std::string getAttributeName_FARETARIFF()                   const { return "FARETARIFF";                   }
  std::string getAttributeName_FARETARIFFIND()                const { return "FARETARIFFIND";                }
  std::string getAttributeName_FARETYPE()                     const { return "FARETYPE";                     }
  std::string getAttributeName_FEEOWNERCXR()                  const { return "FEEOWNERCXR";                  }
  std::string getAttributeName_FLT1()                         const { return "FLT1";                         }
  std::string getAttributeName_FLT2()                         const { return "FLT2";                         }
  std::string getAttributeName_HISTORICSALEDISCDATE()         const { return "HISTORICSALEDISCDATE";         }
  std::string getAttributeName_HISTORICSALEEFFDATE()          const { return "HISTORICSALEEFFDATE";          }
  std::string getAttributeName_HISTORICTRVLDISCDATE()         const { return "HISTORICTRVLDISCDATE";         }
  std::string getAttributeName_HISTORICTRVLEFFDATE()          const { return "HISTORICTRVLEFFDATE";          }
  std::string getAttributeName_ITEMNO()                       const { return "ITEMNO";                       }
  std::string getAttributeName_JRNYINCLUDESLOC()              const { return "JRNYINCLUDESLOC";              }
  std::string getAttributeName_JRNYINCLUDESLOCTYPE()          const { return "JRNYINCLUDESLOCTYPE";          }
  std::string getAttributeName_JRNYINCLUDESLOCZONETBLNO()     const { return "JRNYINCLUDESLOCZONETBLNO";     }
  std::string getAttributeName_JRNYIND()                      const { return "JRNYIND";                      }
  std::string getAttributeName_JRNYLOC1()                     const { return "JRNYLOC1";                     }
  std::string getAttributeName_JRNYLOC1TYPE()                 const { return "JRNYLOC1TYPE";                 }
  std::string getAttributeName_JRNYLOC1ZONETBLNO()            const { return "JRNYLOC1ZONETBLNO";            }
  std::string getAttributeName_JRNYLOC2()                     const { return "JRNYLOC2";                     }
  std::string getAttributeName_JRNYLOC2TYPE()                 const { return "JRNYLOC2TYPE";                 }
  std::string getAttributeName_JRNYLOC2ZONETBLNO()            const { return "JRNYLOC2ZONETBLNO";            }
  std::string getAttributeName_LOC()                          const { return "LOC";                          }
  std::string getAttributeName_LOCTYPE()                      const { return "LOCTYPE";                      }
  std::string getAttributeName_MARKETINGCARRIER()             const { return "MARKETINGCARRIER";             }
  std::string getAttributeName_MAXTAX()                       const { return "MAXTAX";                       }
  std::string getAttributeName_MINMAXCURRENCY()               const { return "MINMAXCURRENCY";               }
  std::string getAttributeName_MINMAXDECIMALS()               const { return "MINMAXDECIMALS";               }
  std::string getAttributeName_MINTAX()                       const { return "MINTAX";                       }
  std::string getAttributeName_NATION()                       const { return "NATION";                       }
  std::string getAttributeName_NETREMITAPPLTAG()              const { return "NETREMITAPPLTAG";              }
  std::string getAttributeName_OPERATINGCARRIER()             const { return "OPERATINGCARRIER";             }
  std::string getAttributeName_PAIDBY3RDPARTYTAG()            const { return "PAIDBY3RDPARTYTAG";            }
  std::string getAttributeName_PAXFROM()                      const { return "PAXFROM";                      }
  std::string getAttributeName_PAXTO()                        const { return "PAXTO";                        }
  std::string getAttributeName_PERCENTFLATTAG()               const { return "PERCENTFLATTAG";               }
  std::string getAttributeName_PODELIVERYLOC()                const { return "PODELIVERYLOC";                }
  std::string getAttributeName_PODELIVERYLOCTYPE()            const { return "PODELIVERYLOCTYPE";            }
  std::string getAttributeName_PODELIVERYLOCZONETBLNO()       const { return "PODELIVERYLOCZONETBLNO";       }
  std::string getAttributeName_POSLOC()                       const { return "POSLOC";                       }
  std::string getAttributeName_POSLOCTYPE()                   const { return "POSLOCTYPE";                   }
  std::string getAttributeName_POSLOCZONETBLNO()              const { return "POSLOCZONETBLNO";              }
  std::string getAttributeName_POTKTLOC()                     const { return "POTKTLOC";                     }
  std::string getAttributeName_POTKTLOCTYPE()                 const { return "POTKTLOCTYPE";                 }
  std::string getAttributeName_POTKTLOCZONETBLNO()            const { return "POTKTLOCZONETBLNO";            }
  std::string getAttributeName_PSGRMAXAGE()                   const { return "PSGRMAXAGE";                   }
  std::string getAttributeName_PSGRMINAGE()                   const { return "PSGRMINAGE";                   }
  std::string getAttributeName_PSGRSTATUS()                   const { return "PSGRSTATUS";                   }
  std::string getAttributeName_PSGRTYPE()                     const { return "PSGRTYPE";                     }
  std::string getAttributeName_PSGRTYPECODEITEMNO()           const { return "PSGRTYPECODEITEMNO";           }
  std::string getAttributeName_PTCMATCHIND()                  const { return "PTCMATCHIND";                  }
  std::string getAttributeName_RBD1()                         const { return "RBD1";                         }
  std::string getAttributeName_RBD2()                         const { return "RBD2";                         }
  std::string getAttributeName_RBD3()                         const { return "RBD3";                         }
  std::string getAttributeName_ROUNDINGRULE()                 const { return "ROUNDINGRULE";                 }
  std::string getAttributeName_RTNTOORIG()                    const { return "RTNTOORIG";                    }
  std::string getAttributeName_RULE()                         const { return "RULE";                         }
  std::string getAttributeName_SECTORDETAILAPPLTAG()          const { return "SECTORDETAILAPPLTAG";          }
  std::string getAttributeName_SECTORDETAILITEMNO()           const { return "SECTORDETAILITEMNO";           }
  std::string getAttributeName_SEQNO()                        const { return "SEQNO";                        }
  std::string getAttributeName_SERVICEBAGGAGEAPPLTAG()        const { return "SERVICEBAGGAGEAPPLTAG";        }
  std::string getAttributeName_SERVICEBAGGAGEITEMNO()         const { return "SERVICEBAGGAGEITEMNO";         }
  std::string getAttributeName_STOPOVERTIMETAG()              const { return "STOPOVERTIMETAG";              }
  std::string getAttributeName_STOPOVERTIMEUNIT()             const { return "STOPOVERTIMEUNIT";             }
  std::string getAttributeName_SVCTYPE()                      const { return "SVCTYPE";                      }
  std::string getAttributeName_TAXABLEUNITTAG1()              const { return "TAXABLEUNITTAG1";              }
  std::string getAttributeName_TAXABLEUNITTAG10()             const { return "TAXABLEUNITTAG10";             }
  std::string getAttributeName_TAXABLEUNITTAG2()              const { return "TAXABLEUNITTAG2";              }
  std::string getAttributeName_TAXABLEUNITTAG3()              const { return "TAXABLEUNITTAG3";              }
  std::string getAttributeName_TAXABLEUNITTAG4()              const { return "TAXABLEUNITTAG4";              }
  std::string getAttributeName_TAXABLEUNITTAG5()              const { return "TAXABLEUNITTAG5";              }
  std::string getAttributeName_TAXABLEUNITTAG6()              const { return "TAXABLEUNITTAG6";              }
  std::string getAttributeName_TAXABLEUNITTAG7()              const { return "TAXABLEUNITTAG7";              }
  std::string getAttributeName_TAXABLEUNITTAG8()              const { return "TAXABLEUNITTAG8";              }
  std::string getAttributeName_TAXABLEUNITTAG9()              const { return "TAXABLEUNITTAG9";              }
  std::string getAttributeName_TAXAMT()                       const { return "TAXAMT";                       }
  std::string getAttributeName_TAXAPPLIESTOTAGIND()           const { return "TAXAPPLIESTOTAGIND";           }
  std::string getAttributeName_TAXAPPLIMIT()                  const { return "TAXAPPLIMIT";                  }
  std::string getAttributeName_TAXCARRIER()                   const { return "TAXCARRIER";                   }
  std::string getAttributeName_TAXCODE()                      const { return "TAXCODE";                      }
  std::string getAttributeName_TAXCURDECIMALS()               const { return "TAXCURDECIMALS";               }
  std::string getAttributeName_TAXCURRENCY()                  const { return "TAXCURRENCY";                  }
  std::string getAttributeName_TAXMATCHINGAPPLTAG()           const { return "TAXMATCHINGAPPLTAG";           }
  std::string getAttributeName_TAXNAME()                      const { return "TAXNAME";                      }
  std::string getAttributeName_TAXNATION()                    const { return "TAXNATION";                    }
  std::string getAttributeName_TAXPERCENT()                   const { return "TAXPERCENT";                   }
  std::string getAttributeName_TAXPOINTLOC1()                 const { return "TAXPOINTLOC1";                 }
  std::string getAttributeName_TAXPOINTLOC1INTDOMIND()        const { return "TAXPOINTLOC1INTDOMIND";        }
  std::string getAttributeName_TAXPOINTLOC1STOPOVERTAG()      const { return "TAXPOINTLOC1STOPOVERTAG";      }
  std::string getAttributeName_TAXPOINTLOC1TRNSFRTYPE()       const { return "TAXPOINTLOC1TRNSFRTYPE";       }
  std::string getAttributeName_TAXPOINTLOC1TYPE()             const { return "TAXPOINTLOC1TYPE";             }
  std::string getAttributeName_TAXPOINTLOC1ZONETBLNO()        const { return "TAXPOINTLOC1ZONETBLNO";        }
  std::string getAttributeName_TAXPOINTLOC2()                 const { return "TAXPOINTLOC2";                 }
  std::string getAttributeName_TAXPOINTLOC2COMPARE()          const { return "TAXPOINTLOC2COMPARE";          }
  std::string getAttributeName_TAXPOINTLOC2INTLDOMIND()       const { return "TAXPOINTLOC2INTLDOMIND";       }
  std::string getAttributeName_TAXPOINTLOC2STOPOVERTAG()      const { return "TAXPOINTLOC2STOPOVERTAG";      }
  std::string getAttributeName_TAXPOINTLOC2TYPE()             const { return "TAXPOINTLOC2TYPE";             }
  std::string getAttributeName_TAXPOINTLOC2ZONETBLNO()        const { return "TAXPOINTLOC2ZONETBLNO";        }
  std::string getAttributeName_TAXPOINTLOC3()                 const { return "TAXPOINTLOC3";                 }
  std::string getAttributeName_TAXPOINTLOC3GEOTYPE()          const { return "TAXPOINTLOC3GEOTYPE";          }
  std::string getAttributeName_TAXPOINTLOC3TYPE()             const { return "TAXPOINTLOC3TYPE";             }
  std::string getAttributeName_TAXPOINTLOC3ZONETBLNO()        const { return "TAXPOINTLOC3ZONETBLNO";        }
  std::string getAttributeName_TAXPOINTTAG()                  const { return "TAXPOINTTAG";                  }
  std::string getAttributeName_TAXREMITTANCEID()              const { return "TAXREMITTANCEID";              }
  std::string getAttributeName_TAXROUNDDIR()                  const { return "TAXROUNDDIR";                  }
  std::string getAttributeName_TAXROUNDUNIT()                 const { return "TAXROUNDUNIT";                 }
  std::string getAttributeName_TAXROUNDUNITNODEC()            const { return "TAXROUNDUNITNODEC";            }
  std::string getAttributeName_TAXTYPE()                      const { return "TAXTYPE";                      }
  std::string getAttributeName_TAXTYPESUBCODE()               const { return "TAXTYPESUBCODE";               }
  std::string getAttributeName_TCH()                          const { return "TCH";                          }
  std::string getAttributeName_TICKETEDPOINTTAG()             const { return "TICKETEDPOINTTAG";             }
  std::string getAttributeName_TKTCODE()                      const { return "TKTCODE";                      }
  std::string getAttributeName_TKTVALAPPLQUALIFIER()          const { return "TKTVALAPPLQUALIFIER";          }
  std::string getAttributeName_TKTVALCURDECIMALS()            const { return "TKTVALCURDECIMALS";            }
  std::string getAttributeName_TKTVALCURRENCY()               const { return "TKTVALCURRENCY";               }
  std::string getAttributeName_TKTVALMAX()                    const { return "TKTVALMAX";                    }
  std::string getAttributeName_TKTVALMIN()                    const { return "TKTVALMIN";                    }
  std::string getAttributeName_TRAVELAGENCYIND()              const { return "TRAVELAGENCYIND";              }
  std::string getAttributeName_TRAVELDATEAPPTAG()             const { return "TRAVELDATEAPPTAG";             }
  std::string getAttributeName_TRVLWHOLLYWITHINLOC()          const { return "TRVLWHOLLYWITHINLOC";          }
  std::string getAttributeName_TRVLWHOLLYWITHINLOCTYPE()      const { return "TRVLWHOLLYWITHINLOCTYPE";      }
  std::string getAttributeName_TAXPROCESSINGAPPLTAG()         const { return "TAXPROCESSINGAPPLTAG";         }
  std::string getAttributeName_TRVLWHOLLYWITHINLOCZONETBLNO() const { return "TRVLWHOLLYWITHINLOCZONETBLNO"; }
  std::string getAttributeName_TVLFIRSTDAY()                  const { return "TVLFIRSTDAY";                  }
  std::string getAttributeName_TVLFIRSTMONTH()                const { return "TVLFIRSTMONTH";                }
  std::string getAttributeName_TVLFIRSTYEAR()                 const { return "TVLFIRSTYEAR";                 }
  std::string getAttributeName_TVLLASTDAY()                   const { return "TVLLASTDAY";                   }
  std::string getAttributeName_TVLLASTMONTH()                 const { return "TVLLASTMONTH";                 }
  std::string getAttributeName_TVLLASTYEAR()                  const { return "TVLLASTYEAR";                  }
  std::string getAttributeName_VALIDATINGCARRIER()            const { return "VALIDATINGCARRIER";            }
  std::string getAttributeName_VENDOR()                       const { return "VENDOR";                       }
  std::string getAttributeName_VIEWBOOKTKTIND()               const { return "VIEWBOOKTKTIND";               }
  std::string getAttributeName_SVCFEESSECURITYITEMNO()        const { return "SVCFEESSECURITYITEMNO";        }
  std::string getAttributeName_AlaskaZone()                   const { return "AlaskaZone";                   }
  std::string getAttributeName_BSR()                          const { return "BSR";                          }
  std::string getAttributeName_CityCode()                     const { return "CityCode";                     }
  std::string getAttributeName_Currency()                     const { return "Currency";                     }
  std::string getAttributeName_CurrencyDecimals()             const { return "CurrencyDecimals";             }
  std::string getAttributeName_FromCurrency()                 const { return "FromCurrency";                 }
  std::string getAttributeName_FromGeoRefId()                 const { return "FromGeoRefId";                 }
  std::string getAttributeName_GlobalDirection()              const { return "GlobalDirection";              }
  std::string getAttributeName_HawaiiPercent()                const { return "HawaiiPercent";                }
  std::string getAttributeName_Included()                     const { return "Included";                     }
  std::string getAttributeName_IsPartial()                    const { return "IsPartial";                    }
  std::string getAttributeName_ItinId()                       const { return "ItinId";                       }
  std::string getAttributeName_LocCode()                      const { return "LocCode";                      }
  std::string getAttributeName_LocRefId()                     const { return "LocRefId";                     }
  std::string getAttributeName_LocType()                      const { return "LocType";                      }
  std::string getAttributeName_MileageGeoPathRefId()          const { return "GeoPathRefId";                 }
  std::string getAttributeName_Miles()                        const { return "Miles";                        }
  std::string getAttributeName_NationCode()                   const { return "NationCode";                   }
  std::string getAttributeName_NationId()                     const { return "Id";                           }
  std::string getAttributeName_RepricedAmount()               const { return "RepricedAmount";               }
  std::string getAttributeName_State()                        const { return "State";                        }
  std::string getAttributeName_TaxPointBegin()                const { return "TaxPointBegin";                }
  std::string getAttributeName_TaxPointEnd()                  const { return "TaxPointEnd";                  }
  std::string getAttributeName_ToCurrency()                   const { return "ToCurrency";                   }
  std::string getAttributeName_ToGeoRefId()                   const { return "ToGeoRefId";                   }
  std::string getAttributeName_UseRepricing()                 const { return "UseRepricing";                 }
  std::string getAttributeName_Vendor()                       const { return "Vendor";                       }
  std::string getAttributeName_ZoneAPercent()                 const { return "ZoneAPercent";                 }
  std::string getAttributeName_ZoneBPercent()                 const { return "ZoneBPercent";                 }
  std::string getAttributeName_ZoneCPercent()                 const { return "ZoneCPercent";                 }
  std::string getAttributeName_ZoneDPercent()                 const { return "ZoneDPercent";                 }
  std::string getAttributeName_RoundTheWorld()                const { return "RoundTheWorld";                }

  virtual std::string getOutputAttributeName_Carrier()                        const = 0;
  virtual std::string getOutputAttributeName_Code()                           const = 0;
  virtual std::string getOutputAttributeName_Content()                        const = 0;
  virtual std::string getOutputAttributeName_EchoToken()                      const = 0;
  virtual std::string getOutputAttributeName_ElementRefId()                   const = 0;
  virtual std::string getOutputAttributeName_GST()                            const = 0;
  virtual std::string getOutputAttributeName_Id()                             const = 0;
  virtual std::string getOutputAttributeName_JourneyLoc1()                    const = 0;
  virtual std::string getOutputAttributeName_JourneyLoc2()                    const = 0;
  virtual std::string getOutputAttributeName_Name()                           const = 0;
  virtual std::string getOutputAttributeName_Nation()                         const = 0;
  virtual std::string getOutputAttributeName_OptionalServiceType()            const = 0;
  virtual std::string getOutputAttributeName_PaymentAmt()                     const = 0;
  virtual std::string getOutputAttributeName_PaymentCur()                     const = 0;
  virtual std::string getOutputAttributeName_PercentFlatTag()                 const = 0;
  virtual std::string getOutputAttributeName_PercentageFlatTag()              const = 0;
  virtual std::string getOutputAttributeName_PointOfDeliveryLoc()             const = 0;
  virtual std::string getOutputAttributeName_PointOfSaleLoc()                 const = 0;
  virtual std::string getOutputAttributeName_PointOfTicketingLoc()            const = 0;
  virtual std::string getOutputAttributeName_PublishedAmt()                   const = 0;
  virtual std::string getOutputAttributeName_PublishedCur()                   const = 0;
  virtual std::string getOutputAttributeName_SabreCode()                      const = 0;
  virtual std::string getOutputAttributeName_SeqNo()                          const = 0;
  virtual std::string getOutputAttributeName_ServiceSubTypeCode()             const = 0;
  virtual std::string getOutputAttributeName_SvcGroup()                       const = 0;
  virtual std::string getOutputAttributeName_SvcSubGroup()                    const = 0;
  virtual std::string getOutputAttributeName_TaxAmt()                         const = 0;
  virtual std::string getOutputAttributeName_TaxCode()                        const = 0;
  virtual std::string getOutputAttributeName_TaxCurToPaymentCurBSR()          const = 0;
  virtual std::string getOutputAttributeName_TaxRoundingUnit()                const = 0;
  virtual std::string getOutputAttributeName_TaxRoundingDir()                 const = 0;
  virtual std::string getOutputAttributeName_TaxEquivAmt()                    const = 0;
  virtual std::string getOutputAttributeName_TaxEquivCurr()                   const = 0;
  virtual std::string getOutputAttributeName_TaxGroupId()                     const = 0;
  virtual std::string getOutputAttributeName_TaxGroupType()                   const = 0;
  virtual std::string getOutputAttributeName_TaxLabel()                       const = 0;
  virtual std::string getOutputAttributeName_TaxOnOptionalServiceGroupRefId() const = 0;
  virtual std::string getOutputAttributeName_TaxOnBaggageGroupRefId()         const = 0;
  virtual std::string getOutputAttributeName_TaxOnChangeFeeGroupRefId()       const = 0;
  virtual std::string getOutputAttributeName_TaxPointIndexBegin()             const = 0;
  virtual std::string getOutputAttributeName_TaxPointIndexEnd()               const = 0;
  virtual std::string getOutputAttributeName_TaxPointLoc1()                   const = 0;
  virtual std::string getOutputAttributeName_TaxPointLoc1Index()              const = 0;
  virtual std::string getOutputAttributeName_TaxPointLoc2()                   const = 0;
  virtual std::string getOutputAttributeName_TaxPointLoc2Index()              const = 0;
  virtual std::string getOutputAttributeName_TaxPointLoc3()                   const = 0;
  virtual std::string getOutputAttributeName_TaxPointLocBegin()               const = 0;
  virtual std::string getOutputAttributeName_TaxPointLocEnd()                 const = 0;
  virtual std::string getOutputAttributeName_TaxPointTag()                    const = 0;
  virtual std::string getOutputAttributeName_TaxType()                        const = 0;
  virtual std::string getOutputAttributeName_TotalAmt()                       const = 0;
  virtual std::string getOutputAttributeName_Type()                           const = 0;
  virtual std::string getOutputAttributeName_TaxableUnitTags()                const = 0;
  virtual std::string getOutputAttributeName_UnticketedPoint()                const = 0;

  virtual std::string getOutputAttributeName_BCHItinId()                      const = 0;
  virtual std::string getOutputAttributeName_BCHPaxType()                     const = 0;
  virtual std::string getOutputAttributeName_BCHPaxCount()                    const = 0;
  virtual std::string getOutputAttributeName_BCHPaxTotalAmount()              const = 0;
  virtual std::string getOutputAttributeName_BCHPaxTaxes()                    const = 0;
  virtual std::string getOutputAttributeName_BCHTaxId()                       const = 0;
  virtual std::string getOutputAttributeName_BCHTaxCode()                     const = 0;
  virtual std::string getOutputAttributeName_BCHTaxAmount()                   const = 0;
  virtual std::string getOutputAttributeName_BCHTaxAmountAdjusted()           const = 0;
  virtual std::string getOutputAttributeName_BCHTaxDescription()              const = 0;

private:
  virtual std::string getTagName(InputRequest*)                   const = 0;
  virtual std::string getTagName(InputApplyOn*)                   const = 0;
  virtual std::string getTagName(InputCalculationRestriction*)    const = 0;
  virtual std::string getTagName(InputChangeFee*)                 const = 0;
  virtual std::string getTagName(InputDiagnosticCommand*)         const = 0;
  virtual std::string getTagName(InputExemptedRule*)              const = 0;
  virtual std::string getTagName(InputFare*)                      const = 0;
  virtual std::string getTagName(InputFarePath*)                  const = 0;
  virtual std::string getTagName(InputFareUsage*)                 const = 0;
  virtual std::string getTagName(InputFlight*)                    const = 0;
  virtual std::string getTagName(InputFlightPath*)                const = 0;
  virtual std::string getTagName(InputFlightUsage*)               const = 0;
  virtual std::string getTagName(InputGeo*)                       const = 0;
  virtual std::string getTagName(InputGeoPath*)                   const = 0;
  virtual std::string getTagName(InputGeoPathMapping*)            const = 0;
  virtual std::string getTagName(InputPrevTicketGeoPath*)         const = 0;
  virtual std::string getTagName(InputItin*)                      const = 0;
  virtual std::string getTagName(InputMap*)                       const = 0;
  virtual std::string getTagName(InputMapping*)                   const = 0;
  virtual std::string getTagName(InputOptionalService*)           const = 0;
  virtual std::string getTagName(InputOptionalServicePath*)       const = 0;
  virtual std::string getTagName(InputOptionalServiceUsage*)      const = 0;
  virtual std::string getTagName(InputParameter*)                 const = 0;
  virtual std::string getTagName(InputPassenger*)                 const = 0;
  virtual std::string getTagName(InputPointOfSale*)               const = 0;
  virtual std::string getTagName(InputProcessingOptions*)         const = 0;
  virtual std::string getTagName(InputCalculationRestrictionTax*) const = 0;
  virtual std::string getTagName(InputRequestWithCache*)          const = 0;
  virtual std::string getTagName(InputTaxDetailsLevel*)           const = 0;
  virtual std::string getTagName(InputTicketingOptions*)          const = 0;
  virtual std::string getTagName(InputTicketingFee*)              const = 0;
  virtual std::string getTagName(InputTicketingFeePath*)          const = 0;
  virtual std::string getTagName(InputTicketingFeeUsage*)         const = 0;
  virtual std::string getTagName(InputYqYr*)                      const = 0;
  virtual std::string getTagName(InputYqYrPath*)                  const = 0;
  virtual std::string getTagName(InputYqYrUsage*)                 const = 0;

  std::string getTagName(AKHIFactor*)              const { return "AKHIFactor";             }
  std::string getTagName(CarrierApplicationEntry*) const { return "CarrierApplEntry";       }
  std::string getTagName(CarrierApplication*)      const { return "CarrierApplication";     }
  std::string getTagName(CarrierFlight*)           const { return "CarrierFlt";             }
  std::string getTagName(CarrierFlightSegment*)    const { return "CarrierFltSeg";          }
  std::string getTagName(Currency*)                const { return "Currency";               }
  std::string getTagName(CurrencyConversion*)      const { return "CurrencyConversion";     }
  std::string getTagName(Customer*)                const { return "Customer";               }
  std::string getTagName(Distance*)                const { return "Distance";               }
  std::string getTagName(IsInLoc*)                 const { return "IsInLoc";                }
  std::string getTagName(Nation*)                  const { return "Loc";                    }
  std::string getTagName(LocUsage*)                const { return "LocUsage";               }
  std::string getTagName(PaxLocUsage*)             const { return "PaxLocUsage";            }
  std::string getTagName(MileageGetterServer*)     const { return "Mileage";                }
  std::string getTagName(PassengerTypeCode*)       const { return "PassengerTypeCode";      }
  std::string getTagName(PassengerTypeCodeItem*)   const { return "PassengerTypeCodeItem";  }
  std::string getTagName(PaxTypeMapping*)          const { return "PaxTypeMapping";         }
  std::string getTagName(PaxTypeMappingItem*)      const { return "PaxTypeMappingItem";     }
  std::string getTagName(ReportingRecord*)         const { return "ReportingRecord";        }
  std::string getTagName(ReportingRecordEntry*)    const { return "ReportingRecordEntry";   }
  std::string getTagName(RepricingEntry*)          const { return "RepricingEntry";         }
  std::string getTagName(RulesRecord*)             const { return "RulesRecord";            }
  std::string getTagName(SectorDetail*)            const { return "SectorDetail";           }
  std::string getTagName(SectorDetailEntry*)       const { return "SectorDetailItem";       }
  std::string getTagName(ServiceBaggage*)          const { return "ServiceBaggage";         }
  std::string getTagName(ServiceBaggageEntry*)     const { return "ServiceBaggageItem";     }
  std::string getTagName(ServiceFeeSecurity*)      const { return "ServiceFeeSecurity";     }
  std::string getTagName(ServiceFeeSecurityItem*)  const { return "ServiceFeeSecurityItem"; }
  std::string getTagName(TaxRounding*)             const { return "TaxRounding";            }
  std::string getTagName(XmlCache*)                const { return "TestData";               }
  std::string getTagName(InputRequestWithStringCache*)const { return "TaxRq";               }

  virtual std::string getTagName(DiagnosticResponse*)           const = 0;
  virtual std::string getTagName(ErrorMessage*)                 const = 0;
  virtual std::string getTagName(ItinPayments*)                 const = 0;
  virtual std::string getTagName(ItinsPayments*)                const = 0;
  virtual std::string getTagName(Message*)                      const = 0;
  virtual std::string getTagName(OutputBaggageDetails*)         const = 0;
  virtual std::string getTagName(OutputCalcDetails*)            const = 0;
  virtual std::string getTagName(OutputDiagnostic*)             const = 0;
  virtual std::string getTagName(OutputError*)                  const = 0;
  virtual std::string getTagName(OutputFaresDetails*)           const = 0;
  virtual std::string getTagName(OutputGeoDetails*)             const = 0;
  virtual std::string getTagName(OutputItin*)                   const = 0;
  virtual std::string getTagName(OutputItins*)                  const = 0;
  virtual std::string getTagName(OutputMessage*)                const = 0;
  virtual std::string getTagName(OutputOBDetails*)              const = 0;
  virtual std::string getTagName(OutputOCDetails*)              const = 0;
  virtual std::string getTagName(OutputOptionalServiceDetails*) const = 0;
  virtual std::string getTagName(OutputResponse*)               const = 0;
  virtual std::string getTagName(OutputTax*)                    const = 0;
  virtual std::string getTagName(OutputTaxDetails*)             const = 0;
  virtual std::string getTagName(OutputTaxDetailsRef*)          const = 0;
  virtual std::string getTagName(OutputTaxGroup*)               const = 0;
  virtual std::string getTagName(OutputTaxOnTaxDetails*)        const = 0;
  virtual std::string getTagName(OutputYQYRDetails*)            const = 0;
  virtual std::string getTagName(Payment*)                      const = 0;
  virtual std::string getTagName(PaymentDetail*)                const = 0;
  virtual std::string getTagName(Response*)                     const = 0;

  virtual std::string getTagName(BCHOutputResponse*)            const = 0;
  virtual std::string getTagName(BCHOutputItin*)                const = 0;
  virtual std::string getTagName(BCHOutputItinPaxDetail*)       const = 0;
  virtual std::string getTagName(BCHOutputTaxDetail*)           const = 0;
};

} // namespace tax
