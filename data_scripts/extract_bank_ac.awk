BEGIN  { FS="|" } ; 
$0 ~ ">" { if ($3 ~ "em") $3="embl"
		   else if ($3 == "gb") $3="genbank"
		   else if ($3 == "ref") $3="refseq"
           # if ($4 ~ /\./) {
           if ($4 ~ /\.+[0-9]*/) {
               # print ".digit found"
               gsub(/\.+[0-9]*/,"",$4)
               }
print $3":"$4 >> "nt_ac_extracts2.txt" }
