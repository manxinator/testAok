
all : ekLex.cpp ekPars.cpp

ekPars.cpp ekPars.hpp: ekPars.y
	yacc -p ek_yy -o ekPars.cpp -d ekPars.y #--debug --verbose

ekLex.cpp: ekLex.l ekPars.hpp
	flex -Pek_yy -o ekLex.cpp ekLex.l

clean:
	rm -f ekLex.cpp ekPars.cpp ekPars.hpp


