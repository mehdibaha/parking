COMP = g++
EDL = g++
RM = rm
EXE = parking
CLEAN = clean
CPPFLAGS = -std=c++11 -c -Wall -Wextra
LIBSPATH = -Ltp-multitache
INTPATH = -Itp-multitache
RMFLAGS = -f
EDLFLAGS = -std=c++11
LIBS = -ltp -lncurses -ltcl
INTERFACE = Mere.h Clavier.h Entree.h Sortie.h Config.h
REAL = $(INTERFACE:.h=.cpp)
OBJ = $(INTERFACE:.h=.o)

.PHONY : $(CLEAN)

$(EXE) : $(OBJ)
	$(EDL) -o $(EXE) $(OBJ) $(LIBSPATH) $(LIBS)

%.o : %.cpp
	$(COMP) $(CPPFLAGS) $(INTPATH) $<

$(CLEAN) :
	$(RM) $(RMFLAGS) *.o $(EXE) core

#TODO : rajouter les dépendances supplémentaires
