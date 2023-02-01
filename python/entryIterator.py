/*
Copyright (C) 2001-2023  Institut Pasteur

  This program is part of the golden software.

  This program  is free software:  you can  redistribute it  and/or modify it  under the terms  of the GNU
  General Public License as published by the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,  but WITHOUT ANY WARRANTY;  without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the  GNU General Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.

  Contact:

   Veronique Legrand                                                           veronique.legrand@pasteur.fr

 */
__author__ = 'vlegrand'
import Golden

class entryIterator:
    #str_list_ids is a char string containing a list of dbank:AC to search for in the databanks. dbank:AC elements are separated by a whitespace.
    def __init__(self,str_list_ids):
        str_list_ids+="\n"
        self.str_list_ids=str_list_ids.replace(" ","\n")
        self.flat = Golden.access_new(self.str_list_ids)

    def __iter__(self):
        return self

    def next(self):
        if self.flat!=None:
            entry=self.flat
            self.flat = Golden.access_new(self.str_list_ids)
            return entry
        else:
            raise StopIteration()

    __next__=next # python3.x compatibility
