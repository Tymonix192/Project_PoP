# Definitions de macros
CXX     = g++
CXXFLAGS = -g -Wall -std=c++17 `pkg-config --cflags gtkmm-3.0`
LDFLAGS = `pkg-config --libs gtkmm-3.0`
CXXFILES = tools.cpp message.cc chaine.cpp mobile.cpp jeu.cpp project.cpp gui.cc graphic.cc
OFILES = project.o jeu.o mobile.o chaine.o message.o tools.o gui.o graphic.o

# Definition de la premiere regle
prog: $(OFILES)
	$(CXX) $(OFILES) $(LDFLAGS) -o projet

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
	@/bin/rm -f *.o *.x *.cc~ *.h~ prog projet

#
# -- Regles de dependances generees automatiquement
#
# DO NOT DELETE THIS LINE
project.o: project.cpp jeu.h gui.h
gui.o: gui.cc gui.h jeu.h graphic.h
jeu.o: jeu.cpp tools.h message.h chaine.h mobile.h
mobile.o: mobile.cpp message.h tools.h
chaine.o: chaine.cpp tools.h message.h
message.o: message.cc message.h
tools.o: tools.cpp tools.h graphic.h
graphic.o: graphic.cc graphic.h graphic_gui.h