#! /bin/sh

## Debug mode
test "x$VERBOSE" = "xx" && set -x

## Set databanks
test $srcdir != . && ln -s $srcdir/all .
dbs=`ls all/*dat`

## Check indexes generation
rm -f *.acx *.idx *.dbx
../src/goldin all $dbs || exit 1

## Check indexes updates
ext='acx idx dbx'
for e in $ext; do
  mv all.$e old.$e || exit 1
done
for d in $dbs; do
  ../src/goldin all $d || exit 1
done
for e in $ext; do
  cmp old.$e all.$e || exit 1
done

## Check source file list
fnb=`ls all/*.dat | wc -l`
inb=`cat all.dbx | wc -l`
test $fnb = $inb || exit 1

## Cleanup
## test $srcdir != . && rm -f all
## rm -f *.acx *.idx *.dbx

## Normal end
exit 0
