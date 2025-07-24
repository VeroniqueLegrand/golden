##
# Copyright (C) 2001-2023  Institut Pasteur
#
#  This program is part of the golden software.
#
#  This program  is free software:  you can  redistribute it  and/or modify it  under the terms  of the GNU
#  General Public License as published by the Free Software Foundation, either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;  without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
#  License for more details.
#
#  You should have received a copy of the  GNU General Public License along with this program.  If not, see
#  <http://www.gnu.org/licenses/>.
#
#  Contact:
#
#   Veronique Legrand                                                           veronique.legrand@pasteur.fr
##

import os, sys
from setuptools.command.build import build as _build
from setuptools.command.sdist import sdist as _sdist
from setuptools import Extension, setup

print("finished import")

SETUP_DIR = os.path.dirname(os.path.abspath(__file__))

print("SETUP_DIR=",SETUP_DIR) 
Goldenmod = Extension( "Golden",
            sources = [ "Golden.c", 
                        "../src/access.c",
                        "../src/locus.c", 
                        "../src/index.c",
                        "../src/index_hl.c",
                        "../src/list.c", 
                        "../src/error.c", 
                        "../src/query.c", 
                        "../src/entry.c" 
                        ],
            include_dirs=["../src"],
            define_macros = [ ("HAVE_CONFIG_H", "1") ])

print("defined Goldenmod")

# Ensure that package is configured
class build(_build):
  def run(self):
    print("running custom build")
    chk = os.access("../src/config.h", os.F_OK)
    if not chk:
      sys.exit("ERROR: Please run golden package configure")
    _build.run(self)

# No stand-alone distribution
class no_standalone_sdist(_sdist):
      def run(self): pass

# Nothing particular to test but need that to be able to run make check
class check(_build):
  def run(self):
    print("running custom check")
    chk = os.access("../src/config.h", os.F_OK)
    if not chk:
      sys.exit("ERROR: Please run golden package configure")
    _build.run(self)

custom_cmdclass = { 'build':build, 'sdist':no_standalone_sdist, 'check':check }

import os; print(os.getcwd())
with os.scandir() as entries:
    for entry in entries:
        print(entry.name, "- Directory" if entry.is_dir() else "- File")

print("calling setup")

setup( name = "golden_seq_retriever", 
       version = "3.4.4.post1",
       cmdclass=custom_cmdclass,
       description = "Python bindings for the golden tool",
       url = "https://github.com/VeroniqueLegrand/golden",
       author = "Nicolas Joly, Veronique Legrand", 
       author_email = "vlegrand@pasteur.fr",
       ext_modules = [ Goldenmod ], 
       py_modules= [ "entryIterator" ] )
