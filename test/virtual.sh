#! /bin/sh

## Debug mode
test "x$VERBOSE" = "xx" && set -x

## Set databanks
test $srcdir != . && ln -s $srcdir/all .
GOLDENDATA=.; export GOLDENDATA

## Make indexes
rm -f *.acx *.idx *.dbx *.vix
lst=`ls all/*.dat`
for f in $lst; do
  b=`basename $f .dat`
  test -r $b || ln -sf $srcdir/all $b
  ../src/goldin $b $b/$b.dat
  echo "$b" >> check.vix
done

## Check databanks list
../src/golden -l >/dev/null || exit 1
(../src/golden -l | grep check) >/dev/null || exit 1

## Check entries
lst="AC007218 HSA395L14 1PYMA ASX_HYDROXYL"
for e in $lst; do
  ../src/golden -c check:$e || exit 1
done

## Cleanup
lst=`ls all/*.dat`
for f in $lst; do
  b=`basename $f .dat`
  rm $b
done
test $srcdir != . && rm -f all
rm -f *.acx *.idx *.dbx *.vix

## Normal end
exit 0
