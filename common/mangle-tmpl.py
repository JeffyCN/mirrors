# -*- Mode: Python -*-
# vi:si:et:sw=4:sts=4:ts=4

"""
use the output from gst-xmlinspect.py to mangle tmpl/*.sgml and
insert/overwrite Short Description and Long Description
"""

# FIXME: right now it uses pygst and scans on its own;
# we really should use inspect/*.xml instead since the result of
# gst-xmlinspect.py is committed by the docs maintainer, who can be
# expected to have pygst, but this step should be done for every docs build,
# so no pygst allowed

# read in inspect/*.xml
# for every tmpl/element-(name).xml: mangle with details from element

from __future__ import print_function, unicode_literals

import glob
import re
import sys
import os

class Tmpl:
    def __init__(self, filename):
        self.filename = filename
        self._sectionids = []
        self._sections = {}

    def read(self):
        """
        Read and parse the sections from the given file.
        """
        lines = open(self.filename).readlines()
        matcher = re.compile("<!-- ##### SECTION (\S+) ##### -->\n")
        id = None

        for line in lines:
            match = matcher.search(line)
            if match:
                id = match.expand("\\1")
                self._sectionids.append(id)
                self._sections[id] = []
            else:
                if not id:
                    sys.stderr.write(
                        "WARNING: line before a SECTION header: %s" % line)
                else:
                    self._sections[id].append(line)

    def get_section(self, id):
        """
        Get the content from the given section.
        """
        return self._sections[id]

    def set_section(self, id, content):
        """
        Replace the given section id with the given content.
        """
        self._sections[id] = content

    def output(self):
        """
        Return the output of the current template in the tmpl/*.sgml format.
        """
        lines = []
        for id in self._sectionids:
            lines.append("<!-- ##### SECTION %s ##### -->\n" % id)
            for line in self._sections[id]:
                lines.append(line)

        return "".join(lines)

    def write(self, backup=False):
        """
        Write out the template file again, backing up the previous one.
        """
        if backup:
            target = self.filename + ".mangle.bak"
            os.rename(self.filename, target)

        handle = open(self.filename, "w")
        handle.write(self.output())
        handle.close()

import xml.dom.minidom

def get_elements(file):
    elements = {}
    doc = xml.dom.minidom.parse(file)

    elem = None
    for e in doc.childNodes:
        if e.nodeType == e.ELEMENT_NODE and e.localName == 'plugin':
            elem = e
            break
    if elem == None:
        return None

    elem2 = None
    for e in elem.childNodes:
        if e.nodeType == e.ELEMENT_NODE and e.localName == 'elements':
            elem2 = e
            break
    if elem2 == None:
        return None

    elem = elem2

    for e in elem.childNodes:
        if e.nodeType == e.ELEMENT_NODE and e.localName == 'element':
            name = None
            description = None

            for e2 in e.childNodes:
                if e2.nodeType == e2.ELEMENT_NODE and e2.localName == 'name':
                    name = e2.childNodes[0].nodeValue.encode("UTF-8")
                elif e2.nodeType == e2.ELEMENT_NODE and e2.localName == 'description':
                    if e2.childNodes:
                      description = e2.childNodes[0].nodeValue.encode("UTF-8")
                    else:
                      description = 'No description'

            if name != None and description != None:
                elements[name] = {'description': description}

    return elements

def main():
    if not len(sys.argv) == 3:
        sys.stderr.write('Please specify the inspect/ dir and the tmpl/ dir')
        sys.exit(1)

    inspectdir = sys.argv[1]
    tmpldir = sys.argv[2]

    # parse all .xml files; build map of element name -> short desc
    #for file in glob.glob("inspect/plugin-*.xml"):
    elements = {}
    for file in glob.glob("%s/plugin-*.xml" % inspectdir):
        elements.update(get_elements(file))

    for file in glob.glob("%s/element-*.sgml" % tmpldir):
        base = os.path.basename(file)
        element = base[len("element-"):-len(".sgml")]
        tmpl = Tmpl(file)
        tmpl.read()
        if element in elements.keys():
            description = elements[element]['description']
            tmpl.set_section("Short_Description", "%s\n\n" % description)

        # put in an include if not yet there
        line = '<include xmlns="http://www.w3.org/2003/XInclude" href="' + \
            'element-' + element + '-details.xml">' + \
            '<fallback xmlns="http://www.w3.org/2003/XInclude" />' + \
            '</include>\n'
        section = tmpl.get_section("Long_Description")
        if not section[0]  == line:
            section.insert(0, line)
        tmpl.set_section("Long_Description", section)
        tmpl.write()

main()
