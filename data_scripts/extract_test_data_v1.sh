GOLDENDATA="/Users/sis/Desktop/GOLDENDATA"

fichier=$GOLDENDATA"/rel/nt/fasta/nt"
oldIFS=$IFS     # sauvegarde du séparateur de champ
IFS=$'\n'       # nouveau séparateur de champ, le caractère fin de ligne

# doesn't work
#for ligne in $(<$fichier)
#do
#   echo $ligne
#done

#!/bin/bash
while read line
do
if echo "$line" | grep ">" >/dev/null
then
echo $line | awk 'BEGIN { FS="|" } ; { print $3":"$4 >> "nt_ac_extracts.txt" }' 
#echo ${line}
fi
done < $fichier