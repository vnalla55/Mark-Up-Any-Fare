import re

from BrandServiceProxy import add_xmlns
from BrandServiceProxy import anywhere
from BrandServiceProxy import ET
from BrandServiceProxy import find_or_create
from BrandServiceProxy import XMLTransformerPlugin


def find_brands(root, brand_code=None):
    """ Returns all Brand nodes. If brand_code is set
         - returns only the brands with given Brand Code """
    if brand_code is None:
        return root.findall(anywhere('Brand'))
    else:
        # [tag=smth] is an undocumented (or rater not clearly documented) feature
        # of ET but it works as expected
        return root.findall(anywhere('Brand[{:s}="{:s}"]'.format(add_xmlns('Code'), brand_code)))


def add_fake_MarketDataSource(root, con_log, data_source, market_id=None):
    """ Add a fake dataSource attribute to all ProgramIdList elements"""
    updated_element_count = 0
    if market_id:
        search_pattern = 'MarketResponse[@marketID="{:s}"]/'.format(market_id)
        search_pattern += add_xmlns('ProgramIdList')
        search_pattern = anywhere(search_pattern)
    else:
        search_pattern = anywhere('ProgramIdList')
    for program_list in root.findall(search_pattern):
        program_list.set('dataSource', data_source)
        updated_element_count += 1
    output = 'Set dataSource for market {:s} to ' \
             '"{:s}" in {:d} elements'.format(market_id or '*',
                                              data_source,
                                              updated_element_count)
    con_log(log_category='plg_out', text=output)
    return root


def add_fake_BrandProgramDataSource(root, con_log, data_source, program_id=None):
    """ Add a fake dataSource attribute to all BrandProgram elements"""
    updated_element_count = 0
    if program_id:
        search_pattern = 'BrandProgram[@programID="{:s}"]'.format(program_id)
        search_pattern = anywhere(search_pattern)
    else:
        search_pattern = anywhere('BrandProgram')
    for program_list in root.findall(search_pattern):
        program_list.set('dataSource', data_source)
        updated_element_count += 1
    output = 'Set dataSource for BrandProgram {:s} to ' \
             '"{:s}" in {:d} elements'.format(program_id or '*',
                                              data_source,
                                              updated_element_count)
    con_log(log_category='plg_out', text=output)
    return root


def add_fake_brand_text(root, con_log, text, brand_code=None):
    """ Add given text as Text elements under each Brand element """
    updated_element_count = 0
    for brand in find_brands(root, brand_code):
        text_element = find_or_create(brand, 'Text')
        text_element.text = text
        updated_element_count += 1
    output = 'Set {:s} text to "{:s}": {:d} elements'.format(brand_code or 'gobal',
                                                             text,
                                                             updated_element_count)
    con_log(log_category='plg_out', text=output)
    return root


def add_fake_brand_bclist(root, con_log, collection_name, bclist, brand_code=None):
    """ Add given list of strings as collection_name/Code elements under each Brand element """
    updated_element_count = 0
    for brand in find_brands(root, brand_code):
        collection = find_or_create(brand, collection_name, clear=True)
        for code in bclist:
            code_element = ET.SubElement(collection, 'Code')
            code_element.text = code.strip()
        updated_element_count += 1
    output = 'Set {:s} {:s} to {:s} in {:d} elements'.format(brand_code or 'global',
                                                             collection_name,
                                                             ','.join(bclist),
                                                             updated_element_count)
    con_log(log_category='plg_out', text=output)
    return root


def add_fake_brand_fbclist(root, con_log, fbclist, brand_code=None):
    """ Add given list of strings as FareBasisCodes/FareBasisCode  elements under each Brand element """
    updated_element_count = 0
    for brand in find_brands(root, brand_code):
        collection = find_or_create(brand, 'FareBasisCodes', clear=True)
        for code in fbclist:
            code_element = ET.SubElement(collection, 'FareBasisCode')
            if code[1]:
                code_element.set('includeInd', 'true')
            else:
                code_element.set('includeInd', 'false')
            code_element.text = code[0]
        updated_element_count += 1
    output = 'Set {:s} FareBasisCodes to {:s} in {:d} elements'.format(brand_code or 'global',
                                                                       repr(fbclist),
                                                                       updated_element_count)
    con_log(log_category='plg_out', text=output)
    return root


class cbas(XMLTransformerPlugin):
    def plugin_init(self, config):
        for key, value in config:
            key = key.split('_')
            cmd = key[0]
            args = key[1:]
            if cmd == 'text':
                self.add_transformation(add_fake_brand_text, text=value,
                                        brand_code=(args[0] if args else None))
            elif cmd == 'marketDataSource':
                self.add_transformation(add_fake_MarketDataSource, data_source=value,
                                        market_id=(args[0] if args else None))
            elif cmd == 'brandProgramDataSource':
                self.add_transformation(add_fake_BrandProgramDataSource, data_source=value,
                                        program_id=(args[0] if args else None))
            elif cmd == 'bclist':
                self.add_transformation(add_fake_brand_bclist,
                                        brand_code=(args[0] if args else None),
                                        collection_name='BookingCodeList',
                                        bclist=value.split(','))
            elif cmd == 'sbclist':
                self.add_transformation(add_fake_brand_bclist,
                                        brand_code=(args[0] if args else None),
                                        collection_name='SecondaryBookingCodeList',
                                        bclist=value.split(','))
            elif cmd == 'fbclist':
                fbc_list = []
                for fbc in value.split(','):
                    match = re.match(r'^(?P<includeInd>\<i\>)?(?P<fbc>.*)$', fbc.strip())
                    if match:
                        fbc_list.append((match.group('fbc'), (match.group('includeInd') is not None)))
                self.add_transformation(add_fake_brand_fbclist,
                                        brand_code=(args[0] if args else None),
                                        fbclist=fbc_list)
            else:
                raise ValueError('Unknown configuration option {:s}={:s}'.format(cmd, value))

    def default_config(self):
        return """[cbas]
# dataSource attribute in ProgramIdList elements
marketDataSource: Will be inserted into all MarketResponse/ProgramIdList element as dataSource attribute
marketDataSource_2: Will be inserted into MarketResponse/ProgramIdList element of MarketResponse with marketID=2

brandProgramDataSource: Will be inserted into all CarrierBrandsData/BrandProgram element as dataSource attribute
brandProgramDataSource_2: Will be inserted into CarrierBrandsData/BrandProgram element with programID=2

# Text elements in Brand elements
text: Will be inserted into all Brand elements as Text element
text_FL: Text for FL brand
text_SV: Text for SV brand

# BookingCodeList elements in Brand elements
bclist: A,B,C,D ; Will be added to all Brand elements as BookingCodeList/Code elements
bclist_FL: A,B,C,D ; Will be added to all Brand elements with Code=FL as BookingCodeList/Code elements

# SecondaryBookingCodeList elements in Brand elements
sbclist: A,B,C,D ; Will be added to all Brand elements as SecondaryBookingCodeList/Code elements
sbclist_FL: A,B,C,D ; Will be added to all Brand elements with Code=FL as SecondaryBookingCodeList/Code elements

# FareBasisCodes elements in Brand elements
fbclist: fbc0,fbc1,fbc3 ; Will be added to all Brand elements as FareBasisCodes/FareBasisCode elements
fbclist_SV: fbc4,fbc5,fbc6 ; Will be added to all Brand elements with Code=SV as FareBasisCodes/FareBasisCode elements
fbclist_SV: <i>fbc7,fbc8,<i>fbc9 ; IncludeInd attribute for fbc7 and fbc9 will be set to True"""
