#! /usr/local/bin/python

# Corinne Maufrais
# Institut Pasteur, Projets et developpements bioinformatiques
# maufrais@pasteur.fr, vlegrand@pasteur.fr
#

# version 0.1


import os
import sys
import getopt

import Golden

try:
    GOLDENDATA =  os.environ['GOLDENDATA']
except:
    GOLDENDATA = "/local/gensoft2/exe/golden/1.1a/share/golden/db/"
    #GOLDENDATA = "/local/gensoft/share/golden/db"
    os.environ['GOLDENDATA'] = GOLDENDATA



####################    

class ParserError:
    def __init__(self,err):
        self.err = err

    def __repr__(self):
        return "[ParserError] " + self.err

class ParserWarning:
    def __init__(self,err):
        self.err = err

    def __repr__(self):
        return "[ParserWarning] " + self.err


def parseUniprot( flatFile , DE ):
    """
    parse uniprot or embl like flat file
    """
    description = ''
    taxId= ''
    taxoLight = ''
    orgName = ''
    lineFld= flatFile.split('\n')
    vuOS = False
    vuOC = False
    vuOCXX = False
    for line in lineFld:
        if line =='\n':
            continue
        tag = line[0:2].strip()
        value = line[5:]
        value = value.replace('\n','')
        if DE and tag == 'DE':
            description += value
        elif tag == 'OS' and not vuOS:
            fldOS=value.split('(')
            orgName += fldOS[0].strip()
            vuOS = True
        elif tag == 'OC' and not vuOCXX:
            taxoLight += value
            vuOC = True
        elif tag == 'XX' and vuOC:
            vuOCXX = True
        elif tag == 'OX':
            taxId += value
        elif tag in ['RN', 'DR','CC', 'FH', 'SQ']:
            return orgName, taxId, taxoLight, description
    return orgName, taxId, taxoLight, description



def parseGenbank( flatFile , DE  ):
    """
    parse genbank like flat file
    """
    description = ''
    taxId= ''
    taxoLight = ''
    orgName = ''
    taxoL = False
    lineFld= flatFile.split('\n')
    for line in lineFld:
        if line =='\n':
            continue
        tag = line[0:12].strip()
        value = line[12:]
        value = value.replace('\n','')
        if DE and tag == 'DEFINITION':
            description += value
        elif tag == 'ORGANISM': # one line ##### pb qqfois plusieurs ligne
            orgName += value.strip()
            taxoL=True
        elif taxoL and not tag: # multiple line without tag
            taxoLight += value
        elif tag == 'TaxID': # gi number
            taxId += value
        elif tag in ['REFERENCE','COMMENT', 'FEATURES', 'ORIGIN']:
            return orgName, taxId, taxoLight, description
        elif not tag: # fin taxonomy
            taxoL = False
    return orgName, taxId, taxoLight, description
        

def parse( flatFile , DE ):
    """
    parse db flat file ( uniprot, embl, genbank) into a DBRecord object
    """
    if flatFile[:2] == 'ID':
        return parseUniprot( flatFile, DE )
    elif flatFile[:5] == 'LOCUS':
        return parseGenbank( flatFile, DE )
    return '','','',''


class TaxOptimizerError:
    def __init__( self, err ):
        self.err = err

    def __repr__( self ):
        return "[taxoptimizer] " + self.err

##############################################################################
#
#            Golden. Keep this for compatibility and performance testing.
#
##############################################################################

# def doGolden( f_cards, db,ac, DE ):
def doGolden( db,ac, DE ):
    ########################### db ref
    if db in[ 'sp', 'sw','swissprot','tr', 'trembl']:
        db = 'uniprot'
    elif db in ['emb', 'dbj']:
        db = 'embl'
    elif db in ['gb']:
        db = 'genbank'
    elif db in ['gp']:
        db = 'genpept'
    elif db in ['ref']:
        db = 'refseq'
    elif db in ['rdpii']:
        db = 'rdpii'
    elif db in ['pir','pdb','tpg', 'tpe', 'tpd', 'prf']:
        return '','','',''
    try:
        #print "Calling Golden on : ",db,ac
        #print "GOLDENDATA=",GOLDENDATA
        #print >>f_cards, db+":"+acc
        flatFile = Golden.access(db,ac)
    except IOError, err:
        print >>sys.stderr, err, db, ac
        sys.exit()
    if flatFile:
        orgName, taxId, taxoLight, description =  parse(flatFile , DE ) #orgName, taxId, taxoLight, description
        flatFile = '' # buffer free
        return orgName, taxId, taxoLight, description
    else:
        return '','','','' 


# Builds query input string
def buildQueryStr(db, acc,l_cards,cnt_cards,allTaxo):
    if db in[ 'sp', 'sw','swissprot','tr', 'trembl']:
        db = 'uniprot'
    elif db in ['emb', 'dbj']:
        db = 'embl'
    elif db in ['gb']:
        db = 'genbank'
    elif db in ['gp']:
        db = 'genpept'
    elif db in ['ref']:
        db = 'refseq'
    elif db in ['rdpii']:
        db = 'rdpii'
    elif db in ['pir','pdb','tpg', 'tpe', 'tpd', 'prf']:
        #return '','','',''
        pass

    l_cards+=db
    l_cards+=":"
    l_cards+=acc
    l_cards+="\n"
    cnt_cards+=1
    allTaxo[acc]={'db':db }
    return l_cards,cnt_cards,allTaxo

##############################################################################
#
#            Golden Multi. Using new version of golden.
#
# l_input : Depending on input_flag, a list of bank:AC bank:locus separated by '\n'
#  so, bank:AC\nbank:locus...
##############################################################################
def doGoldenMulti(allTaxo,l_input,DE):
    idx_res=0
    lst_input=l_input.split("\n")
    try:
        flatFile=Golden.access_new(l_input)
        while (flatFile!=None):
            db_acc=lst_input[idx_res]
            l_db_acc=db_acc.split(":")
            acc=l_db_acc[1]
            allTaxo[acc]['orgName'], allTaxo[acc]['taxId'], allTaxo[acc]['taxoLight'], allTaxo[acc]['DE'] =  parse(flatFile , DE ) #orgName, taxId, taxoLight, description
            flatFile=Golden.access_new(l_input)
            return allTaxo
    except IOError, err:
        print >>sys.stderr, err, l_input
        sys.exit()


##############################################################################
#
#            Taxonomy
#
##############################################################################
def extractTaxoFrom_osVSocBDB( acc, allTaxo, allTaxId, BDB ):
    taxonomy = allTaxo[acc]['taxoLight']
    orgName = allTaxo[acc]['orgName']
    if orgName and not allTaxId.has_key(orgName):
        taxoFull = BDB.get( str(orgName) )
        if  taxoFull:
            allTaxo[acc]['taxoFull'] = taxoFull
            allTaxId[orgName]= taxoFull
            allTaxo[acc]['taxoLight'] = ''
            taxonomy = taxoFull
    elif orgName:
        allTaxo[acc]['taxoFull'] = allTaxId[orgName]
        allTaxo[acc]['taxoLight'] = ''
        taxonomy = allTaxId[orgName]
    return taxonomy,  allTaxo, allTaxId    
    

##############################################################################
#
#            MAIN
#
##############################################################################


def usage():
    print """
usage: taxoptimizer [options] -i <infile> -o <outfile>

options:
   
   -i <file> ... Tabulated file. 
   -o <file> ... Output file

   -h        ... Print this message and exit.
   -c <int>  ... Column number to parse (default second column: 2)
   -s <car>  ... Separator character (default '|')
   -d <str>  ... Specified Database name for finding taxonomy in only once database.
   -e        ... Add description (DE) in output
   -f <file> ... Extract line without taxonomy from input file
   -x        ... Only write line with taxonomy in output file
"""


if __name__=='__main__':
    max_cards=1000 # for Golden
    cnt_cards=0
    # tmp path
    try:
        TMP_PATH = os.environ[ 'TMPPATH' ]
    except:
        TMP_PATH = "/tmp"


    # ===== Command line parser
    try:
        opts, args = getopt.gnu_getopt( sys.argv[1:], "hi:c:s:d:eo:f:x",["help",] )
    except getopt.GetoptError:
        usage()
        sys.exit( 0 )

    tabFile = None  
    outFile = None 
    noTaxoFile = None
    splitFile = False
    column = 1
    separator = '|'
    DE = False
    database = None
    for o, v in opts: #( opt, value )
        if o in ( "-h","--help" ):
            usage()
            sys.exit( 0 )
        elif o in ( "-i" , "--in" ):
            tabFile = v
        elif o in ( "-o" , "--out" ):
            outFile = v
        elif o in ( "-c" , "--col" ):
            try:
                column = int(v) -1
            except:
                print >>sys.stderr, TaxOptimizerError("Integer value is mandatory for column number")
                sys.exit( 0 )
        elif o in ( "-d" , "--db" ):
            database = v
        elif o in ( "-s" , "--sep" ):
            separator = v
        elif o in ( "-e" , "--desc" ):
            DE = True
        elif o in ( "-f" , "--notaxout" ):
            noTaxoFile = v
        elif o in ( "-x" , "--sep" ):
            splitFile = True
        else:
            usage()
            sys.exit( 0 )

    if not tabFile:
        usage()
        sys.exit( 0 )
        
    # more m8Blast_nrprot_light2.txt| ../../src/taxoptimizer.py -o toto -i /dev/stdin
        
    # ===== Tabulated file parsing
    try:
        tabfhin = open( tabFile )
    except:
        tabfhin = sys.stdin
        
    try:
        outfh = open( outFile, 'w' )
    except:
        outfh = sys.stdout
        
    if noTaxoFile:
        notaxfhout = open( noTaxoFile, 'w' )
    
    allTaxo = {} #{acc:{'db':db, 'orgName':'', 'taxId':'', 'taxoLight':'', 'DE':'', 'taxoFull':'' }
    allTaxId = {} #{ orgName: taxonomy }  ==> taxonomy == taxoFull if exist, else taxoLight. Rapid access.
    try:
        line = tabfhin.readline()
        lineNb =1
    except EOFError, err:
        print >>sys.stderr, err
        sys.exit()
    # VL : to get sample only
    # try :
    #     f_cards=open('/tmp/all_requested_cards.txt','w')
    # except:
    #     print "Cannot trace requests"
    #     sys.exit()

    l_cards=""
    while line:
        description = ''
        fld = line.split()
        if line =='\n':
            line = tabfhin.readline()
            continue
        try:
            fldInfo = fld[column].split(separator)
        except:
            
            print >>sys.stderr,  TaxOptimizerError("Parsing: column error: couldn't parse line: %s\n --> %s" % (lineNb))
            sys.exit()
            #continue
        acc = ''
        db = ''
        if len(fldInfo) == 5:
            # PF8/Pathoquest
            if database:
                db = database
            else:
                db = fldInfo[2]
            acc = fldInfo[3].split('.')[0]
        elif len(fldInfo) == 2 or len(fldInfo) == 3:
            # Lionel extraction
            if database:
                db = database
            else:
                db = fldInfo[0]
            if len(fldInfo) == 3 and fldInfo[1] == '':
                acc = fldInfo[2].split('.')[0]
            else:
                acc = fldInfo[1].split('.')[0]
        elif len(fldInfo) == 1 and database:
                db = database
                acc = fldInfo[0]
        
        if not acc or not db:
            if not splitFile:
                print >>outfh, line[:-1]
            if noTaxoFile:
                print >>notaxfhout,  line[:-1]
            print >>sys.stderr, TaxOptimizerError ( "Parsing: acc or db error: :%s in line %s" % (fld[column],lineNb) )
            
            try:
                line = tabfhin.readline()
                lineNb+=1
            except EOFError, err:
                print >>sys.stderr, err
                print >>sys.stderr, TaxOptimizerError ( "in line %s" % (lineNb) )
                sys.exit()
            continue 
        # taxonomy = ''
        # if allTaxo.has_key(acc):
        #     if allTaxo[acc].has_key('taxoFull'):
        #         taxonomy = allTaxo[acc]['taxoFull']
        #     else:
        #         taxonomy = allTaxo[acc]['taxoLight']
        #     description = allTaxo[acc]['DE']
        else:
            # l_cards+=db
            # l_cards+=":"
            # l_cards+=acc
            # l_cards+="\n"
            # cnt_cards+=1
            # allTaxo[acc]={'db':db }
            l_cards,cnt_cards,allTaxo=buildQueryStr(db, acc,l_cards,cnt_cards,allTaxo)
            if cnt_cards == max_cards:
                allTaxo=doGoldenMulti(allTaxo,l_cards,DE)
                l_cards=""
                cnt_cards=0
            # print >>f_cards, db,":",acc
            #allTaxo[acc]['orgName'], allTaxo[acc]['taxId'], allTaxo[acc]['taxoLight'], allTaxo[acc]['DE'] = doGolden( db, acc, DE ) # doGolden( f_cards, db, acc, DE )
            #doGoldenMulti(,acc,DE)
            #taxonomy = allTaxo[acc]['taxoLight']
            
        # if taxonomy:
        #     print >>outfh, line[:-1], "\t%s\t%s\t%s" % (allTaxo[acc]['orgName'], taxonomy, allTaxo[acc]['DE'] )
        # else:
        #     if noTaxoFile:
        #         print >>notaxfhout, line[:-1]
        #     if not splitFile:
        #         print >>outfh, line[:-1]
        try:
            line = tabfhin.readline()
            lineNb+=1
        except EOFError, err:
            print >>sys.stderr, err
            print >>sys.stderr, TaxOptimizerError ( "in line %s" % (lineNb) )

            sys.exit()
        lineNb+=1

    if cnt_cards !=0:
        allTaxo=doGoldenMulti(allTaxo,l_cards,acc,DE)
    tabfhin.close()
    # write results
    print allTaxo
    # f_cards.close()
        
