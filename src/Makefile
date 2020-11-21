objects = main.o interp.o scan.o utils.o
CPPFLAGS += -ferror-limit=2 -std=c++17 -frelaxed-template-template-args
# --no-warnings  https://clang.llvm.org/docs/ClangCommandLineReference.html

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
