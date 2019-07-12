#!/usr/bin/python
# -*- Mode: Python -*-
# vi:si:et:sw=4:sts=4:ts=4

"""
parse, merge and write gstdoc-scanobj files
"""

from __future__ import print_function, unicode_literals

import sys
import os

def debug(*args):
    pass

# OrderedDict class based on
# http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/107747
# Licensed under the Python License
class OrderedDict(dict):
    def __init__(self):
        self._keys = []
        dict.__init__(self)

    def __delitem__(self, key):
        dict.__delitem__(self, key)
        self._keys.remove(key)

    def __setitem__(self, key, item):
        dict.__setitem__(self, key, item)
        if key not in self._keys: self._keys.append(key)

    def clear(self):
        dict.clear(self)
        self._keys = []

    def copy(self):
        dict = dict.copy(self)
        dict._keys = self._keys[:]
        return dict

    def items(self):
        return zip(self._keys, self.values())

    def keys(self):
        return self._keys

    def popitem(self):
        try:
            key = self._keys[-1]
        except IndexError:
            raise KeyError('dictionary is empty')

        val = self[key]
        del self[key]

        return (key, val)

    def setdefault(self, key, failobj = None):
        dict.setdefault(self, key, failobj)
        if key not in self._keys: self._keys.append(key)

    def update(self, dict):
        dict.update(self, dict)
        for key in dict.keys():
            if key not in self._keys: self._keys.append(key)

    def values(self):
        return map(self.get, self._keys)

class Object:
    def __init__(self, name):
        self._signals = OrderedDict()
        self._args = OrderedDict()
        self.name = name

    def __repr__(self):
        return "<Object %s>" % self.name

    def add_signal(self, signal, overwrite=True):
        if not overwrite and signal.name in self._signals:
            raise IndexError("signal %s already in %r" % (signal.name, self))
        self._signals[signal.name] = signal

    def add_arg(self, arg, overwrite=True):
        if not overwrite and arg.name in self._args:
            raise IndexError("arg %s already in %r" % (arg.name, self))
        self._args[arg.name] = arg

class Docable:
    def __init__(self, **kwargs):
        for key in self.attrs:
            setattr(self, key, kwargs[key])
        self.dict = kwargs

    def __repr__(self):
        return "<%r %s>" % (str(self.__class__), self.name)

class Signal(Docable):
    attrs = ['name', 'returns', 'args']

class Arg(Docable):
    attrs = ['name', 'type', 'range', 'flags', 'nick', 'blurb', 'default']

class GDoc:
    def load_file(self, filename):
        try:
            lines = open(filename).readlines()
            self.load_data("".join(lines))
        except IOError:
            print ("WARNING - could not read from %s" % filename)

    def save_file(self, filename, backup=False):
        """
        Save the information to the given file if the file content changed.
        """
        olddata = None
        try:
            lines = open(filename).readlines()
            olddata = "".join(lines)
        except IOError:
            print ("WARNING - could not read from %s" % filename)
        newdata = self.get_data()
        if olddata and olddata == newdata:
            return

        if olddata:
            if backup:
                os.rename(filename, filename + '.bak')

        handle = open(filename, "w")
        handle.write(newdata)
        handle.close()

class Signals(GDoc):
    def __init__(self):
        self._objects = OrderedDict()

    def load_data(self, data):
        """
        Load the .signals lines, creating our list of objects and signals.
        """
        import re
        smatcher = re.compile(
            '(?s)'                                      # make . match \n
            '<SIGNAL>\n(.*?)</SIGNAL>\n'
            )
        nmatcher = re.compile(
            '<NAME>'
            '(?P<object>\S*)'                           # store object
            '::'
            '(?P<signal>\S*)'                           # store signal
            '</NAME>'
        )
        rmatcher = re.compile(
            '(?s)'                                      # make . match \n
            '<RETURNS>(?P<returns>\S*)</RETURNS>\n'     # store returns
            '(?P<args>.*)'                              # store args
        )
        for block in smatcher.findall(data):
            nmatch = nmatcher.search(block)
            if nmatch:
                o = nmatch.group('object')
                debug("Found object", o)
                debug("Found signal", nmatch.group('signal'))
                if o not in self._objects:
                    object = Object(o)
                    self._objects[o] = object

                rmatch = rmatcher.search(block)
                if rmatch:
                    dict = rmatch.groupdict().copy()
                    dict['name'] = nmatch.group('signal')
                    signal = Signal(**dict)
                    self._objects[o].add_signal(signal)

    def get_data(self):
        lines = []
        for o in self._objects.values():
            for s in o._signals.values():
                block = """<SIGNAL>
<NAME>%(object)s::%(name)s</NAME>
<RETURNS>%(returns)s</RETURNS>
%(args)s</SIGNAL>
"""
                d = s.dict.copy()
                d['object'] = o.name
                lines.append(block % d)

        return "\n".join(lines) + '\n'

class Args(GDoc):
    def __init__(self):
        self._objects = OrderedDict()

    def load_data(self, data):
        """
        Load the .args lines, creating our list of objects and args.
        """
        import re
        amatcher = re.compile(
            '(?s)'                                      # make . match \n
            '<ARG>\n(.*?)</ARG>\n'
            )
        nmatcher = re.compile(
            '<NAME>'
            '(?P<object>\S*)'                           # store object
            '::'
            '(?P<arg>\S*)'                              # store arg
            '</NAME>'
        )
        rmatcher = re.compile(
            '(?s)'                                      # make . match \n
            '<TYPE>(?P<type>\S*)</TYPE>\n'              # store type
            '<RANGE>(?P<range>.*?)</RANGE>\n'           # store range
            '<FLAGS>(?P<flags>\S*)</FLAGS>\n'           # store flags
            '<NICK>(?P<nick>.*?)</NICK>\n'              # store nick
            '<BLURB>(?P<blurb>.*?)</BLURB>\n'           # store blurb
            '<DEFAULT>(?P<default>.*?)</DEFAULT>\n'     # store default
        )
        for block in amatcher.findall(data):
            nmatch = nmatcher.search(block)
            if nmatch:
                o = nmatch.group('object')
                debug("Found object", o)
                debug("Found arg", nmatch.group('arg'))
                if o not in self._objects:
                    object = Object(o)
                    self._objects[o] = object

                rmatch = rmatcher.search(block)
                if rmatch:
                    dict = rmatch.groupdict().copy()
                    dict['name'] = nmatch.group('arg')
                    arg = Arg(**dict)
                    self._objects[o].add_arg(arg)
                else:
                    print ("ERROR: could not match arg from block %s" % block)

    def get_data(self):
        lines = []
        for o in self._objects.values():
            for a in o._args.values():
                block = """<ARG>
<NAME>%(object)s::%(name)s</NAME>
<TYPE>%(type)s</TYPE>
<RANGE>%(range)s</RANGE>
<FLAGS>%(flags)s</FLAGS>
<NICK>%(nick)s</NICK>
<BLURB>%(blurb)s</BLURB>
<DEFAULT>%(default)s</DEFAULT>
</ARG>
"""
                d = a.dict.copy()
                d['object'] = o.name
                lines.append(block % d)

        return "\n".join(lines) + '\n'

class SingleLine(GDoc):
    def __init__(self):
        self._objects = []

    def load_data(self, data):
        """
        Load the .interfaces/.prerequisites lines, merge duplicates
        """
        # split data on '\n'
        lines = data.splitlines();
        # merge them into self._objects
        for line in lines:
            if line not in self._objects:
                self._objects.append(line)

    def get_data(self):
        lines = sorted(self._objects)
        return "\n".join(lines) + '\n'

def main(argv):
    modulename = None
    try:
        modulename = argv[1]
    except IndexError:
        sys.stderr.write('Please provide a documentation module name\n')
        sys.exit(1)

    signals = Signals()
    signals.load_file(modulename + '.signals')
    signals.load_file(modulename + '.signals.new')
    signals.save_file(modulename + '.signals', backup=True)
    os.unlink(modulename + '.signals.new')

    args = Args()
    args.load_file(modulename + '.args')
    args.load_file(modulename + '.args.new')
    args.save_file(modulename + '.args', backup=True)
    os.unlink(modulename + '.args.new')

    ifaces = SingleLine()
    ifaces.load_file(modulename + '.interfaces')
    ifaces.load_file(modulename + '.interfaces.new')
    ifaces.save_file(modulename + '.interfaces', backup=True)
    os.unlink(modulename + '.interfaces.new')

    prereq = SingleLine()
    prereq.load_file(modulename + '.prerequisites')
    prereq.load_file(modulename + '.prerequisites.new')
    prereq.save_file(modulename + '.prerequisites', backup=True)
    os.unlink(modulename + '.prerequisites.new')

main(sys.argv)
