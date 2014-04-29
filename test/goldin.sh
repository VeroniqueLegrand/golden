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

## tests for new version
dbs=`ls all/*.g*`
mkdir tmp_new

## generates indexes in the classic way (merged and sorted)
../src/goldin --index_dir tmp_new -d all -i wgs_classic $dbs || exit 1
nb_flat_classic=`cat tmp_new/wgs_classic.dbx | wc -l`

## test new options : generated concatenated indexes (not merged).
../src/goldin --index_dir tmp_new -d all -a wgs_ac1 all/wgs_extract.1.gbff all/wgs_extract.1.gnp || exit 1
nb_flat1=`cat tmp_new/wgs_ac1.dbx | wc -l`
nb_idx1=`od tmp_new/wgs_ac1.acx|awk 'FNR==1 {print $2}'`
test $nb_idx1 = "000005" || exit 1

../src/goldin --index_dir tmp_new -d all -a wgs_ac2 all/wgs_extract.2.gbff all/wgs_extract.2.gnp || exit 1
nb_flat2=`cat tmp_new/wgs_ac2.dbx | wc -l`
nb_idx2=`od tmp_new/wgs_ac2.acx|awk 'FNR==1 {print $2}'`
test $nb_idx2 = "000007" || exit 1

../src/goldin --index_dir tmp_new -d all -a wgs_ac3 all/wgs_extract.3.gnp || exit 1
nb_flat3=`cat tmp_new/wgs_ac3.dbx | wc -l`
nb_idx3=`od tmp_new/wgs_ac3.acx|awk 'FNR==1 {print $2}'`
test $nb_idx3 = "000003" || exit 1

../src/goldin --index_dir tmp_new --idx_input --concat_only -a wgs_ac_c1 tmp_new/wgs_ac1 tmp_new/wgs_ac2 tmp_new/wgs_ac3 || exit 1

nb_flat_new=`cat tmp_new/wgs_ac_c1.dbx | wc -l`
echo "nb_flat_classic" $nb_flat_classic
echo "nb_flat_new" $nb_flat_new
test $nb_flat_classic = $nb_flat_new || exit 1

# check that $n1+$n2+$n3==$nb_flat_new
interm=`expr $nb_flat1 + $nb_flat2`
interm=`expr $interm + $nb_flat3`

echo "interm" $interm

test $interm = $nb_flat_new || exit 1

cp tmp_new/wgs_ac_c1.acx tmp_new/wgs_ac_c1_ns.acx # ns stands for "not sorted"
cp tmp_new/wgs_ac_c1.dbx tmp_new/wgs_ac_c1_ns.dbx

## now sort and compare results
../src/goldin --index_dir tmp_new --idx_input --concat_sort -a wgs_ac_c1_sorted tmp_new/wgs_ac_c1 || exit 1

cmp tmp_new/wgs_ac_c1_sorted.acx tmp_new/wgs_ac_c1_ns.acx|grep "differ" || exit 1

../src/goldin --index_dir tmp_new --idx_input --concat_sort -a wgs_ac_c1_sorted_2 tmp_new/wgs_ac1 tmp_new/wgs_ac2 tmp_new/wgs_ac3 || exit 1

cmp tmp_new/wgs_ac_c1_sorted.acx tmp_new/wgs_ac_c1_sorted_2.acx || exit 1

## this should throw an error since locus were not indexed previously.
my_output=`../src/goldin --index_dir tmp_new --idx_input --concat_only -i wgs_ac_c1 tmp_new/wgs_ac1 tmp_new/wgs_ac2 tmp_new/wgs_ac3`
test $? = 2 || exit 1
#echo "my_output : "
#echo $my_output
#echo "trying grep"
#echo $my_output|grep "No such file or directory"
#echo $?

## this should cause a print usage
my_output=`../src/goldin --index_dir tmp_new --idx_input --concat_sort --concat_only wgs_ac_c1_SNE tmp_new/wgs_ac1 tmp_new/wgs_ac2 tmp_new/wgs_ac3`
echo $my_output|grep "usage" || exit 1

## clean files
rm -f tmp_new/*.acx tmp_new/*.idx tmp_new/*.dbx

rmdir tmp_new

## Cleanup
test $srcdir != . && rm -f all
rm -f *.acx *.idx *.dbx

## Normal end
exit 0
