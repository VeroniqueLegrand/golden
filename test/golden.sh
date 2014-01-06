#! /bin/sh

## Debug mode
test "x$VERBOSE" = "xx" && set -x

## Set databanks
test $srcdir != . && ln -s $srcdir/all .
GOLDENDATA=.; export GOLDENDATA

## Make indexes
rm -f *.acx *.idx *.dbx
../src/goldin all all/*.dat || exit 1

## Check databanks list
../src/golden -l >/dev/null || exit 1
(../src/golden -l | grep all) >/dev/null || exit 1

## Check existing entries
lst="AC007218 HSA395L14 1PYMA ASX_HYDROXYL"
for e in $lst; do
  ../src/golden all:$e >/dev/null || exit 1
done

## Check non existing entries
lst="foo bar nul"
for e in $lst; do
  ../src/golden all:$e 2>/dev/null || exit 1
  (../src/golden all:$e 2>&1 | grep 'no such entry') && exit 1
done

## Check compatibility (without dir prefix)
mv all.dbx all.dbx.new
sed 's/^[^\/]*\///g' all.dbx.new >all.dbx
lst="AC007218 HSA395L14 1PYMA ASX_HYDROXYL"
for e in $lst; do
  ../src/golden all:$e >/dev/null || exit 1
done
mv all.dbx.new all.dbx

## Check flags
lst='-a -i -c'
for f in $lst; do
  ../src/golden $f all:AC007218 >/dev/null || exit 1
done

## Cleanup
test $srcdir != . && rm -f all
rm -f *.acx *.idx *.dbx

## Normal end
exit 0
