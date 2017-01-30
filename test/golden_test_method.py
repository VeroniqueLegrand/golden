from __future__ import print_function

import sys
print(sys.path)
sys.path.append("users/vlegrand/tmp/lib/python3.5/site-packages/")
import Golden
from entryIterator import entryIterator

ids = ["uniprot:D0PRM9", "uniprot:D0PRM7", "uniprot:B8PWH5", "uniprot:test", "uniprot:Q66431", "uniprot:Q66431"]

# Using 'access' method 
for i in ids:
    print("Searching %s ... " % str(i), end="")
    print(i)
    flat = Golden.access(i.split(":")[0], i.split(":")[1])
    if flat is None:
        print("Not FOUND!")
    else:
        print("FOUND!")
        
# Using 'access_new' method
str_list_ids='\n'.join([str(x) for x in ids])
str_list_ids+="\n"
flat = Golden.access_new(str_list_ids)
while (flat!=None):
    print(flat)
    flat = Golden.access_new(str_list_ids)
      
# using entry iterator
str_list_ids=' '.join([str(x) for x in ids])
for my_entry in entryIterator(str_list_ids):
    print(my_entry)
    print(" ")


sys.exit(0)
