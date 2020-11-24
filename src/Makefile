objects = main.o interp.o scan.o utils.o
CPPFLAGS += -ferror-limit=15 -std=c++17 -g -O0
# --no-warnings  https://clang.llvm.org/docs/ClangCommandLineReference.html
# -frelaxed-template-template-args

joy : $(objects)
	$(LINK.cc) -o $@ $(objects)

all : joy tests

ttest : ttest.o
	$(LINK.cc) -o $@ $^

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
