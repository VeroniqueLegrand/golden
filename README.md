# GOLDEN

1. Intro

Golden is a small databanks entries retriever. It searches and displays
entries on indexed databanks `flat' files.

This package currently contains the following programs :
  golden   ... Entries retriever.
  goldin   ... Indexes generator.

Queries/Indexes can be made using entry name, primary accession
numbers or even both (by default).

Currently supported formats : Genbank, Embl, Pir codata, and
assimiled.

2. Installation

To install golden and goldin just do :

```
./configure
make
[sudo] make install
```

golden functionalities can also be exported to python (in order to be called from your python script).
In order to do that, you must of course have python installed on your machine.

These python "bindings" or "exports" were tested with python 2.7 up to python 3.11.

They are available on PyPi (for python3 only).
To install them, just do (after installing python):

```
pip3 install --user golden-seq-retriever
```

You must then export the GOLDENDATA environment variable to point to where the indexes are.

3. Use of golden bindings from a python script

There are 2 functions that allow to perform queries:
- access
to query a single entry.

- access_multi
to query multiple entries.
Am "EntryIterator" is provided in order to iterate over all retrieved entries.

For a matter of performance, access_multi must be preferred if you have more than 1 entry to retrive.
Below is a small example

```
import Golden
Golden.access("uniprot:A0A6C0J8N5")
```
for a single query
Any remark/suggestion/problem should be reported to  <vlegrand@pasteur.fr>.
