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
