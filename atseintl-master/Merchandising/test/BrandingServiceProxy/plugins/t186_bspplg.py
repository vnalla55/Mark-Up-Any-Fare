from BrandServiceProxy import XMLTransformerPlugin
from BrandServiceProxy import anywhere
from BrandServiceProxy import add_xmlns
from BrandServiceProxy import ET


def get_parent_map(root):
    """Return a dict of element parents."""
    return {c:p for p in root.iter() for c in p}


def extract_brand_code(root):
    """Get brand code and remove it from root.

    Modifies root by removing the BrandCode element.
    Returns brand code or None if BrandCode element wasn't found.

    TODO: This is only valid for a single market. Pricing can send
    multiple markets in a single request.
    """
    bc_elem = root.find(anywhere('BrandCode'))
    if bc_elem is not None:
        parent_map = get_parent_map(root)
        parent_map[bc_elem].remove(bc_elem)
        return bc_elem.text


def filter_brands_by_code(root, con_log, code):
    """Remove brands with brand code different than code.

    Removes BrandPrograms and adds an error message if there are
    no brands left.
    """
    parent_map = get_parent_map(root)
    brands = []
    brands_removed = 0
    for brand in root.findall(anywhere('Brand')):
        brand_code = brand.findtext(add_xmlns('Code'))
        if brand_code != code:
            parent_map[brand].remove(brand)
            brands_removed += 1
        else:
            brands.append(brand)
    if not brands:
        for program in root.findall(anywhere('BrandProgram')):
            parent_map[program].remove(program)
        mkt_response = root.find(anywhere('MarketResponse'))
        # TODO Update when BS team specify error message for no matching brands
        message = ET.SubElement(mkt_response, 'Message',
                                messageCode='Warning', failCode='3001')
        message.text = 'No programs found'
    output = 'Filtered out {:d} non-{:s} brands, {:d} left'.format(brands_removed,
                                                                   code,
                                                                   len(brands))
    con_log(log_category='plg_out', text=output)
    return root


def add_fake_t186_itemno(root, con_log, offset):
    """Add an itemno to each brand.

    The value of itemno = Brand/Tier + offset.
    """
    updated_element_count = 0
    for brand in root.findall(anywhere('Brand')):
        tier = brand.findtext(add_xmlns('Tier'))
        if tier:
            itemno = int(tier) + offset
            itemno_elem = ET.SubElement(brand, 'CarrierFlightItemNum')
            itemno_elem.text = str(itemno)
        updated_element_count += 1
    output = 'Added itemno to {:d} elements'.format(updated_element_count)
    con_log(log_category='plg_out', text=output)
    return root


class t186(XMLTransformerPlugin):
    def plugin_init(self, config):
        for key, value in config:
            if key == 'filter_brands_by_code':
                self.add_transformation(filter_brands_by_code, code=value)
            elif key == 'add_itemno':
                self.add_transformation(add_fake_t186_itemno, offset=int(value))
            else:
                raise ValueError('Unknown configuration option {:s}={:s}'.format(key, value))

    def default_config(self):
        return """[t186]
filter_brands_by_code: <brand code> ; Removes brands with brand code different from the given code.
add_itemno: <offset> ; Adds an itemno to each brand. The value of itemno = Brand/Tier + offset"""
