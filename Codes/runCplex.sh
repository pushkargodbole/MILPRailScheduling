SYSTEM=x86-64_debian4.0_4.1
LIBFORMAT=static_pic


if [ $# -lt 2 ]; then
    echo "Run the script with choice and file name"  
    exit
fi


#------------------------------------------------------------
#
# When you adapt this makefile to compile your CPLEX programs
# please copy this makefile and set CPLEXDIR and CONCERTDIR to
# the directories where CPLEX and CONCERT are installed.
#
#------------------------------------------------------------

CPLEXDIR=/pkgs/ilog/cplex121
CONCERTDIR=/pkgs/ilog/concert29


# ---------------------------------------------------------------------
# Compiler selection 
# ---------------------------------------------------------------------

CCC=g++
CC=gcc
JAVAC=javac
PY=/pkgs/Python-2.4/python

# ---------------------------------------------------------------------
# Compiler options 
# ---------------------------------------------------------------------

CCOPT="-m64 -O -fPIC -fexceptions -DNDEBUG -DIL_STD"
COPT="-m64 -fPIC"
JOPT="-classpath "$CPLEXDIR"/lib/cplex.jar -O"

# ---------------------------------------------------------------------
# Link options and libraries
# ---------------------------------------------------------------------

CPLEXBINDIR=$CPLEXDIR/bin/$BINDIST
CPLEXJARDIR=$CPLEXDIR/lib/cplex.jar
CPLEXLIBDIR=$CPLEXDIR/lib/$SYSTEM/$LIBFORMAT
CONCERTLIBDIR=$CONCERTDIR/lib/$SYSTEM/$LIBFORMAT

CCLNFLAGS="-L"$CPLEXLIBDIR" -lilocplex -lcplex -L"$CONCERTLIBDIR" -lconcert -lm -pthread"
CLNFLAGS="-L"$CPLEXLIBDIR" -lcplex -lm -pthread"
JAVA="java -d64 -Djava.library.path="$CPLEXDIR"/bin/"$SYSTEM" -classpath "$CPLEXJARDIR:

CONCERTINCDIR=$CONCERTDIR/include
CPLEXINCDIR=$CPLEXDIR/include

CFLAGS=$COPT"  -I"$CPLEXINCDIR
CCFLAGS=$CCOPT" -I"$CPLEXINCDIR" -I"$CONCERTINCDIR
JCFLAGS=$JOPT


#-----------------------------------------------------------------------
#-----------------------------------------------------------------------
#-----------------------------------------------------------------------

FILENAME=$2

if [ $1 = java ]  ; then

echo "executing java"
$JAVAC $JCFLAGS -d . *.java
#-----------------------------------------------------------------------

elif [ $1 = runjava ] ; then
echo "running Java class file"
echo $JAVA $FILENAME
$JAVA $FILENAME


#-----------------------------------------------------------------------

elif [ $1 = c++ ] ; then
echo "CPP compilation"
echo $CCC -c $CCFLAGS $FILENAME.cpp -o $FILENAME.o
$CCC -c $CCFLAGS $FILENAME.cpp -o $FILENAME.o
echo $CCC $CCFLAGS $FILENAME.o -o $FILENAME $CCLNFLAGS
$CCC $CCFLAGS $FILENAME.o -o $FILENAME $CCLNFLAGS

#-----------------------------------------------------------------------

elif [ $1 = python ] ; then
echo "running pthon on 2.4"
$PY $FILENAME.py
#-----------------------------------------------------------------------
elif [ $1 = clean ] ; then
rm *.o *.class *.pyc 
echo "Remove the cpp executable manually"

else
echo "invalid choice, use c++ | java | runJava | python | clean"
fi



