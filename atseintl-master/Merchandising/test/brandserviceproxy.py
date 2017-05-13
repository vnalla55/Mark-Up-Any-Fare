#!/usr/bin/env python


import argparse
from contextlib import closing
import functools
from SimpleHTTPServer import SimpleHTTPRequestHandler
import SocketServer
import sys
import urllib2
import xml.etree.cElementTree as ET


INT_URL = "http://int.brandedfaresSWS.sabre.com:8280/BrandedFaresService/getBrands"
XMLNS = "http://stl.sabre.com/Merchandising/v1"
XMLNS_ANCS = "http://stl.sabre.com/Merchandising/v0"


def here(tag):
    """Return XPath to a direct child in Branding Service namespace."""
    return "{%s}%s" % (XMLNS, tag)


def anywhere(tag):
    """Return XPath to a descendant in Branding Service namespace."""
    return ".//%s" % here(tag)


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
    bc_elem = root.find(anywhere("BrandCode"))
    if bc_elem is not None:
        parent_map = get_parent_map(root)
        parent_map[bc_elem].remove(bc_elem)
        return bc_elem.text


def filter_brands_by_code(root, code):
    """Remove brands with brand code different than code.

    Removes BrandPrograms and adds an error message if there are
    no brands left.
    """
    parent_map = get_parent_map(root)
    brands = []
    for brand in root.findall(anywhere("Brand")):
        brand_code = brand.findtext(here("Code"))
        if brand_code != code:
            parent_map[brand].remove(brand)
        else:
            brands.append(brand)
    if not brands:
        for program in root.findall(anywhere("BrandProgram")):
            parent_map[program].remove(program)
        mkt_response = root.find(anywhere("MarketResponse"))
        # TODO Update when BS team specify error message for no matching brands
        message = ET.SubElement(mkt_response, "Message",
                                messageCode="Warning", failCode="3001")
        message.text = "No programs found"


def add_fake_t186_itemno(root, offset):
    """Add an itemno to each brand.

    The value of itemno = Brand/Tier + offset.
    """
    for brand in root.findall(anywhere("Brand")):
        tier = brand.findtext(here("Tier"))
        if tier:
            itemno = int(tier) + offset
            itemno_elem = ET.SubElement(brand, "CarrierFlightItemNum")
            itemno_elem.text = str(itemno)


class SequentialTransform:

    def __init__(self):
        self.pipe = []

    def __call__(self, root):
        return [f(root, *a, **k) for f, a, k in self.pipe]

    def register(self, func, *args, **kwargs):
        self.pipe.append((func, args, kwargs))

    def __nonzero__(self):
        return bool(self.pipe)


def transform_xml(func, xml, *args, **kwargs):
    root = ET.XML(xml)
    result = func(root, *args, **kwargs)
    out_xml = ET.tostring(root)
    return out_xml, result


def query_branding_service(request, url):
    with closing(urllib2.urlopen(url, request)) as f:
        return f.read()


class BrandingServiceProxy(SimpleHTTPRequestHandler):

    def __init__(self, request, client_address, server, service_url=None,
                 fake_t186=False, t186_offset=0):
        self._url = service_url
        self._fake_t186 = fake_t186
        self._t186_offset = t186_offset
        # We need to call base ctor last because it never returns.
        SimpleHTTPRequestHandler.__init__(
                self, request, client_address, server)

    def do_HEAD(self):
        self.send_error(405)

    def do_GET(self):
        self.send_error(405)

    def do_POST(self):
        self.log_message("HTTP request headers:\n%s", self.headers)
        contentlen = int(self.headers.get("content-length"))
        request = self.rfile.read(contentlen)
        self.send_response(200)
        self.end_headers()
        try:
            request, brand_code = transform_xml(extract_brand_code, request)
            response = query_branding_service(request, self._url)
            t = SequentialTransform()
            if brand_code:
                self.log_message("Filtering by brand code %s", brand_code)
                t.register(filter_brands_by_code, brand_code)
            if self._fake_t186:
                self.log_message("Faking seqnos from %d", self._t186_offset)
                t.register(add_fake_t186_itemno, self._t186_offset)
            response, _ = transform_xml(t, response)
            self.wfile.write(response)
        except ET.ParseError as e:
            self.wfile.write(str(e))
        except urllib2.URLError as e:
            self.wfile.write(str(e))


def serve_proxy(port, url, fake_t186, t186_offset):
    make_proxy = functools.partial(
            BrandingServiceProxy, service_url=url, fake_t186=fake_t186,
            t186_offset=t186_offset)
    httpd = SocketServer.ThreadingTCPServer(("", port), make_proxy)
    httpd.serve_forever()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
            description="Mock new functionality of Branding Service",
            formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument("-p", "--port", type=int, default=8000,
                        help="HTTP port for this server")
    parser.add_argument("-u", "--url", default=INT_URL,
                        help="URL of the real Branding Service")

    t186group = parser.add_argument_group("T186 fake itemno generation")
    t186group.add_argument("--fake-t186", action="store_true",
                           help="Return a fake T186 itemno for each brand")
    t186group.add_argument("--t186-offset", type=int, default=384000,
                           help="Fake itemnos will start at OFFSET",
                           metavar="OFFSET")

    args = parser.parse_args()

    ET.register_namespace("", XMLNS)
    ET.register_namespace("ANCS", XMLNS_ANCS)

    serve_proxy(args.port, args.url, args.fake_t186, args.t186_offset)

# vim:ai:et:sw=4
