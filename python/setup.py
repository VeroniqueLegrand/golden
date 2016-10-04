
import os, sys

from distutils.command.build import build as _build
from distutils.command.sdist import sdist as _sdist
from distutils.core import setup, Extension
from distutils.util import change_root

Goldenmod = Extension( "Golden",
  sources = [ "Golden.c", "../src/access.c", "../src/locus.c", "../src/index.c","../src/index_hl.c",
              "../src/list.c", "../src/error.c", "../src/query.c", "../src/entry.c" ],
  include_dirs = [ '../src' ],
  define_macros = [ ("HAVE_CONFIG_H", "1") ] )

# Ensure that package is configured
class build(_build):
  def run(self):
    chk = os.access("../src/config.h", os.F_OK)
    if not chk:
      sys.exit("ERROR: Please run golden package configure")
    _build.run(self)

# No stand-alone distribution
class sdist(_sdist):
  def run(self): pass

# Nothing particular to test but need that to be able to run make check
class check(_build):
  def run(self):
    chk = os.access("../src/config.h", os.F_OK)
    if not chk:
      sys.exit("ERROR: Please run golden package configure")

cmdclass = { 'build':build, 'sdist':sdist, 'check':check }

setup( name = "Golden", version = "3.3.1", cmdclass=cmdclass,
       description = "Python bindings for the golden tool",
       url = " ", author = " ", author_email = " ",
       ext_modules = [ Goldenmod ] )
