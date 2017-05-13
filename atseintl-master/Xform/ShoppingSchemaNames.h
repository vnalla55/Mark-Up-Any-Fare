#pragma once
namespace tse
{
namespace shopping
{
enum _ElementNameIdx_
{
  _ShoppingRequest,
  _FGG,
  _FGI,
  _BH0,
  _MTY,
  _XPN,
  _TXS,
  _HSP,
  _BRD,
  _OFF,
  _FBK,
  _BKC,
  _SID,
  _BKK,
  _BRI,
  _PRG,
  _FID,
  _AID,
  _SOP,
  _DRG,
  _PSG,
  _RQF,
  _DTL,
  _IBL,
  _ARG,
  _AGI,
  _DUR,
  _PDT,
  _BIL,
  _ODD,
  _PXI,
  _APX,
  _SPV,
  _PRO,
  _BRN,
  _AAF,
  _AVL,
  _LEG,
  _OND,
  _FFG,
  _ITN,
  _DIA,
  _CMD,
  _BFF,
  _EXC,
  _PUP,
  _PCL,
  _FCL,
  _FCI,
  _VCT,
  _R3I,
  _SEQ,
  _MIL,
  _FFY,
  _RFG,
  _BRS,
  _DIV,
  _CXP,
  _PEN,
  _SUR,
  _SFD,
  _HPS,
  _OPT,
  _XRA,
  _DynamicConfig,
  _NumberElementNames_
};

enum _AttributeNameIdx_
{ _B70,
  _Q0U,
  _BH0A,
  _Q0T,
  _A01,
  _A30,
  _P33,
  _AC0,
  _AC1,
  _AC2,
  _AC3,
  _AC4,
  _ACX,
  _Q18,
  _Q0C,
  _Q0S,
  _Q0F,
  _P1W,
  _P2D,
  _D96,
  _P41,
  _PAE,
  _Q19,
  _N1T,
  _Q1S,
  _P80,
  _Q0E,
  _Q60,
  _Q61,
  _Q62,
  _Q63,
  _Q64,
  _Q65,
  _Q66,
  _Q67,
  _Q68,
  _Q69,
  _Q6A,
  _Q6Q,
  _Q6R,
  _Q6S,
  _Q5T,
  _Q5U,
  _Q5V,
  _Q5W,
  _Q5X,
  _Q6U,
  _S5A,
  _MET,
  _PXO,
  _P1F,
  _P1G,
  _PCG,
  _P1V,
  _Q4U,
  _P2F,
  _P23,
  _P49,
  _P0J,
  _P0L,
  _P0F,
  _P1Y,
  _P1Z,
  _P20,
  _P21,
  _P53,
  _P54,
  _PBG,
  _C48,
  _P69,
  _C47,
  _AF0,
  _AG0,
  _S11,
  _SM1,
  _SM2,
  _SM3,
  _SM4,
  _SMX,
  _P43,
  _B05,
  _PXD,
  _PYA,
  _D12,
  _N0M,
  _A40,
  _N0L,
  _C6C,
  _AH0,
  _A50,
  _S94,
  _C6Y,
  _C6P,
  _D07,
  _D54,
  _D92,
  _D94,
  _PAF,
  _PBM,
  _CS0,
  _PXC,
  _PXE,
  _SA0,
  _C6B,
  _P47,
  _Q1K,
  _B00,
  _B01,
  _Q0B,
  _S05,
  _P0Z,
  _PCR,
  _BB0,
  _BB2,
  _BB3,
  _D00,
  _D30,
  _Q17,
  _B30,
  _B31,
  _A03,
  _Q0R,
  _D01,
  _D31,
  _A02,
  _D02,
  _D32,
  _Q5Q,
  _PCA,
  _C65,
  _Q14,
  _Q15,
  _Q13,
  _PBX,
  _A11,
  _A12,
  _N23,
  _Q6T,
  _B50,
  _C75,
  _C76,
  _Q4V,
  _D17,
  _D18,
  _Q0A,
  _NAM,
  _VAL,
  _N06,
  _D70,
  _AS0,
  _AS1,
  _AS2,
  _AS3,
  _A10,
  _A20,
  _A21,
  _AB0,
  _AB1,
  _AE1,
  _AE2,
  _AE3,
  _AE4,
  _A90,
  _A80,
  _N0G,
  _C40,
  _Q01,
  _AE0,
  _MIN,
  _MAX,
  _A22,
  _A70,
  _Q03,
  _Q02,
  _AD0,
  _AA0,
  _C00,
  _C01,
  _C20,
  _C21,
  _D37,
  _N24,
  _N22,
  _PXY,
  _Q16,
  _C6L,
  _A13,
  _A14,
  _A18,
  _S68,
  _A19,
  _P72,
  _P73,
  _C50,
  _Q6D,
  _C6I,
  _S37,
  _B09,
  _S89,
  _S90,
  _PCI,
  _SSH,
  _SEZ,
  _PVI,
  _Q80,
  _Q81,
  _Q82,
  _Q83,
  _Q84,
  _Q85,
  _Q86,
  _Q87,
  _Q88,
  _Q89,
  _Q90,
  _Q91,
  _Q92,
  _Q93,
  _Q94,
  _Q95,
  _Q96,
  _Q97,
  _Q98,
  _Q99,
  _QA0,
  _QA1,
  _QA2,
  _QA3,
  _QA4,
  _QA5,
  _QA6,
  _QA7,
  _QA8,
  _QA9,
  _QB0,
  _QB1,
  _QB2,
  _QB3,
  _QB4,
  _QB5,
  _QB6,
  _QB7,
  _QB8,
  _QB9,
  _QC0,
  _QC1,
  _QC2,
  _QC3,
  _SG1,
  _SG2,
  _SG3,
  _EXL,
  _N0W,
  _S07,
  _S08,
  _Q6E,
  _AJ0,
  _RTD,
  _Q5Y,
  _Q5Z,
  _NAA,
  _Q79,
  _AP2,
  _AP3,
  _Q48,
  _Q6W,
  _Q6C,
  _N25,
  _D95,
  _C5A,
  _N27,
  _STK,
  _SEY,
  _SET,
  _AB2,
  _PDG,
  _Q6B,
  _Q7D,
  _SEU,
  _VTI,
  _MWI,
  _SEV,
  _N03,
  _S01,
  _PAS,
  _C57,
  _C58,
  _P44,
  _PBD,
  _SB2,
  _SBL,
  _SC0,
  _PEF,
  _PHR,
  _NUM,
  _DNS,
  _BSL,
  _FLM,
  _PXS,
  _PXU,
  _PTF,
  _TST,
  _SLC,
  _B11,
  _QD1,
  _QD2,
  _TOD,
  _OPC,
  _FAC,
  _NSO,
  _NSV,
  _IOP,
  _FLN,
  _FAS,
  _TTS,
  _TTD,
  _PCP,
  _BKD,
  _PFC,
  _FRL,
  _HDM,
  _MDL,
  _LPP,
  _Q6F,
  _UGL,
  _PA0,
  _PA1,
  _PA2,
  _PA3,
  _TFO,
  _NTO,
  _SPT,
  _T92,
  _T94,
  _T95,
  _DFL,
  _DFN,
  _PFF,
  _FLP,
  _QCU,
  _PCU,
  _BFR,
  _SRL,
  _CAB,
  _BPL,
  _CBS,
  _UAF,
  _IMT,
  _PBO,
  _AFD,
  _PZF,
  _PTC,
  _FFR,
  _VCX,
  _SM0,
  _DVL,
  _FBS,
  _URC,
  _NRA,
  _XFF,
  _S15,
  _BFA,
  _BFS,
  _SE2,
  _SE3,
  _TEP,
  _QFL, // Number of fares for CAT12 estimation
  _MXP,
  _KOB,
  _PY6,
  _MPO,
  _ABD,
  _MPT,
  _MPI,
  _MPA,
  _MPC,
  _FIX,
  _DCL,
  _OCO,
  _KOF,
  _NSD,
  _PDO,
  _PDR,
  _XRS,
  _N0F,
  _C69,
  _C46,
  _N28,
  _CES,
  _RAF,
  _NBP, // Number of Bag Pieces
  _TYP,
  _T52,
  _N52,
  _C52,
  _DDC,
  _DDA,
  _DDP,
  _DMA,
  _DMP,
  _B12,
  _B13,
  _BI0,
  _Q12,
  _D41,
  _D42,
  _S79,
  _SMV,
  _IEV,
  _CRC,
  _CTC,
  _RCQ,
  _PRM,
  _FPL,
  _SFM,
  _MID,
  _CID,
  _DRT,
  _Name,
  _Value,
  _Substitute,
  _Optional,
  _NumberAttributeNames_ };

extern const char* shoppingElementNames[];
extern const char* shoppingAttributeNames[];
}
}