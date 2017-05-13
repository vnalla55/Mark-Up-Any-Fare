import xml.sax.handler
from copy import deepcopy
from content_handlers import WPResContentHandler
from content_handlers import AirTaxResContentHandler

def is_AirTaxRs(line):
    xmlStart, xmlEnd = line.find('<AirTaxRS'), line.rfind('</AirTaxRS>')
    return True if xmlStart >= 0 and xmlEnd > 0 else False

def merge_dicts(d1, d2):
    result = dict()
    for key in d1.keys() + d2.keys():
      result[key] = (d1.get(key), d2.get(key))
    return result

def compare_taxes(rsp1, rsp2):
    try:
        res1_parser = WPResContentHandler() if not is_AirTaxRs(rsp1) else AirTaxResContentHandler()
        res2_parser = WPResContentHandler() if not is_AirTaxRs(rsp2) else AirTaxResContentHandler()
        xml.sax.parseString(rsp1, res1_parser)
        xml.sax.parseString(rsp2, res2_parser)
    except:
        return 'XML RESPONSE PARSING ERROR'
    rsp = ''
    if not res1_parser.tax_by_group and res2_parser.tax_by_group:
        rsp = 'RESPONSE1 returned no data\n' + rsp
    if res1_parser.tax_by_group and not res2_parser.tax_by_group:
        rsp = 'RESPONSE2 returned no data\n' + rsp
    if res1_parser.tax_by_group and res2_parser.tax_by_group:
      taxes_by_group = merge_dicts(res1_parser.tax_by_group, res2_parser.tax_by_group)
      for group in taxes_by_group:
        rsp += '***{0}***\n'.format(group)
        taxes1 = taxes_by_group.get(group)[0]
        taxes2 = taxes_by_group.get(group)[1]
        if taxes1 is None: taxes1 = {}
        if taxes2 is None: taxes2 = {}
        taxes = merge_dicts(taxes1, taxes2)
        for tax_name in taxes:
          amounts = taxes.get(tax_name)
          ref_tax_amount = amounts[0]
          tax_amount = amounts[1]
          if tax_amount is None: tax_amount = 0.0
          if ref_tax_amount is None: ref_tax_amount = 0.0
          denominator = max(tax_amount, ref_tax_amount)
          error = abs((tax_amount - ref_tax_amount) / denominator) * 100 if denominator > 0 else 0.0
          rsp += ' {0},{1},{2},{3:.4g}\n'.format(tax_name, ref_tax_amount, tax_amount, error)
    return rsp

