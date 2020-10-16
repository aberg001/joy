objects = main.o interp.o scan.o utils.o

joy : $(objects)
	$(LINK.cc) -o $@ $(objects)

all : joy tests

tests :
	echo "ran tests"

#.cc.o
#	$(COMPILE.cc) -o $@ $^

main.o : globals.h
interp.o : globals.h
scan.o : globals.h
utils.o : globals.h

.PHONY : clean
clean :
	rm -f joy $(objects)
