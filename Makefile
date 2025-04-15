# Definitions de macros

CXX     = g++
CXXFLAGS = -g -Wall -std=c++17
CXXFILES = tools.cpp message.cc chaine.cpp mobile.cpp jeu.cpp project.cpp
OFILES = project.o jeu.o mobile.o chaine.o message.o tools.o

# Definition de la premiere regle

prog: $(OFILES)
	$(CXX) $(OFILES) -o projet

# Definitions de cibles particulieres

depend:
	@echo " *** MISE A JOUR DES DEPENDANCES ***"
	@(sed '/^# DO NOT DELETE THIS LINE/q' Makefile && \
	  $(CXX) -MM $(CXXFLAGS) $(CXXFILES) | \
	  egrep -v "/usr/include" \
	 ) >Makefile.new
	@mv Makefile.new Makefile

clean:
	@echo " *** EFFACE MODULES OBJET ET EXECUTABLE ***"
	@/bin/rm -f *.o *.x *.cc~ *.h~ prog

#
# -- Regles de dependances generees automatiquement
#
# DO NOT DELETE THIS LINE
project.o: project.cpp jeu.h
jeu.o: jeu.cpp tools.h message.h chaine.h mobile.h
mobile.o: mobile.cpp message.h tools.h
chaine.o: chaine.cpp tools.h message.h
message.o: message.cc message.h
tools.o: tools.cpp tools.h
