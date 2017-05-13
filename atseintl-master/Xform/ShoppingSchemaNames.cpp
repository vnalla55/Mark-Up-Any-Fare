namespace tse
{
namespace shopping
{
const char* shoppingElementNames[] = {
  "ShoppingRequest", "FGG", "FGI", "BH0", "MTY", "XPN", "TXS", "HSP", "BRD", "OFF", "FBK", "BKC",
  "SID",             "BKK", "BRI", "PRG", "FID", "AID", "SOP", "DRG", "PSG", "RQF", "DTL", "IBL",
  "ARG",             "AGI", "DUR", "PDT", "BIL", "ODD", "PXI", "APX", "SPV", "PRO", "BRN", "AAF",
  "AVL",             "LEG", "OND", "FFG", "ITN", "DIA", "CMD", "BFF", "EXC", "PUP", "PCL", "FCL",
  "FCI",             "VCT", "R3I", "SEQ", "MIL", "FFY", "RFG", "BRS", "DIV", "CXP", "PEN", "SUR",
  "SFD",             "HPS", "OPT", "XRA",
  "DynamicConfig"
};

const char* shoppingAttributeNames[] = {
  "B70", "Q0U", "BH0", "Q0T", "A01", "A30", "P33", "AC0", "AC1", "AC2", "AC3", "AC4", "ACX", "Q18",
  "Q0C", "Q0S", "Q0F", "P1W", "P2D", "D96", "P41", "PAE", "Q19", "N1T", "Q1S", "P80", "Q0E", "Q60",
  "Q61", "Q62", "Q63", "Q64", "Q65", "Q66", "Q67", "Q68", "Q69", "Q6A", "Q6Q", "Q6R", "Q6S", "Q5T",
  "Q5U", "Q5V", "Q5W", "Q5X", "Q6U", "S5A", "MET", "PXO", "P1F", "P1G", "PCG", "P1V", "Q4U", "P2F",
  "P23", "P49", "P0J", "P0L", "P0F", "P1Y", "P1Z", "P20", "P21", "P53", "P54", "PBG", "C48", "P69",
  "C47", "AF0", "AG0", "S11", "SM1", "SM2", "SM3", "SM4", "SMX", "P43", "B05", "PXD", "PYA", "D12",
  "N0M", "A40", "N0L", "C6C", "AH0", "A50", "S94", "C6Y", "C6P", "D07", "D54", "D92", "D94", "PAF",
  "PBM", "CS0", "PXC", "PXE", "SA0", "C6B", "P47", "Q1K", "B00", "B01", "Q0B", "S05", "P0Z", "PCR",
  "BB0", "BB2", "BB3", "D00", "D30", "Q17", "B30", "B31", "A03", "Q0R", "D01", "D31", "A02", "D02",
  "D32", "Q5Q", "PCA", "C65", "Q14", "Q15", "Q13", "PBX", "A11", "A12", "N23", "Q6T", "B50", "C75",
  "C76", "Q4V", "D17", "D18", "Q0A", "NAM", "VAL", "N06", "D70", "AS0", "AS1", "AS2", "AS3", "A10",
  "A20", "A21", "AB0", "AB1", "AE1", "AE2", "AE3", "AE4", "A90", "A80", "N0G", "C40", "Q01", "AE0",
  "MIN", "MAX", "A22", "A70", "Q03", "Q02", "AD0", "AA0", "C00", "C01", "C20", "C21", "D37", "N24",
  "N22", "PXY", "Q16", "C6L", "A13", "A14", "A18", "S68", "A19", "P72", "P73", "C50", "Q6D", "C6I",
  "S37", "B09", "S89", "S90", "PCI", "SSH", "SEZ", "PVI", "Q80", "Q81", "Q82", "Q83", "Q84", "Q85",
  "Q86", "Q87", "Q88", "Q89", "Q90", "Q91", "Q92", "Q93", "Q94", "Q95", "Q96", "Q97", "Q98", "Q99",
  "QA0", "QA1", "QA2", "QA3", "QA4", "QA5", "QA6", "QA7", "QA8", "QA9", "QB0", "QB1", "QB2", "QB3",
  "QB4", "QB5", "QB6", "QB7", "QB8", "QB9", "QC0", "QC1", "QC2", "QC3", "SG1", "SG2", "SG3", "EXL",
  "N0W", "S07", "S08", "Q6E", "AJ0", "RTD", "Q5Y", "Q5Z", "NAA", "Q79", "AP2", "AP3", "Q48", "Q6W",
  "Q6C", "N25", "D95", "C5A", "N27", "STK", "SEY", "SET", "AB2", "PDG", "Q6B", "Q7D", "SEU", "VTI",
  "MWI", "SEV", "N03", "S01", "PAS", "C57", "C58", "P44", "PBD", "SB2", "SBL", "SC0", "PEF", "PHR",
  "NUM", "DNS", "BSL", "FLM", "PXS", "PXU", "PTF", "TST", "SLC", "B11", "QD1", "QD2", "TOD", "OPC",
  "FAC", "NSO", "NSV", "IOP", "FLN", "FAS", "TTS", "TTD", "PCP", "BKD", "PFC", "FRL", "HDM", "MDL",
  "LPP", "Q6F", "UGL", "PA0", "PA1", "PA2", "PA3", "TFO", "NTO", "SPT", "T92", "T94", "T95", "DFL",
  "DFN", "PFF", "FLP", "QCU", "PCU", "BFR", "SRL", "CAB", "BPL", "CBS", "UAF", "IMT", "PBO", "AFD",
  "PZF", "PTC", "FFR", "VCX", "SM0", "DVL", "FBS", "URC", "NRA", "XFF", "S15", "BFA", "BFS", "SE2",
  "SE3", "TEP", "QFL", "MXP", "KOB", "PY6", "MPO", "ABD", "MPT", "MPI", "MPA", "MPC", "FIX", "DCL",
  "OCO", "KOF", "NSD", "PDO", "PDR", "XRS", "N0F", "C69", "C46", "N28", "CES", "RAF", "NBP", "TYP",
  "T52", "N52", "C52", "DDC", "DDA", "DDP", "DMA", "DMP", "B12", "B13", "BI0", "Q12", "D41", "D42",
  "S79", "SMV", "IEV", "CRC", "CTC", "RCQ", "PRM", "FPL", "SFM", "MID", "CID", "DRT",
  "Name", "Value", "Substitute", "Optional"
};
}
}
