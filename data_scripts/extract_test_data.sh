GOLDENDATA="/Users/sis/Desktop/GOLDENDATA"

fichier=$GOLDENDATA"/rel/nt/fasta/nt"

awk -f extract_bank_ac.awk $fichier