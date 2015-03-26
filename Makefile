ifeq ($(origin CXX),default)
CXX:=clang++
endif

# because make doesn't have a string equality function
eq = $(and $(findstring $(1),$(2)),$(findstring $(2),$(1)))

ifneq ($(or $(call eq,$(CXX),clang++),$(call eq,$(CXX),codelite-cc clang++)),)
SHARED_C_LD_FLAGS:=-fsanitize=undefined -fno-sanitize-recover -g -O3 -march=native -flto
#SHARED_C_LD_FLAGS:=-fsanitize=undefined -fno-sanitize-recover -g -stdlib=libc++ -O3 -march=native -flto
# sadly -fsanitize=object-size and -stdlib=libc++ don't play nicely at the moment

WARNFLAGS:=-Weverything -Wno-padded -Wno-c++98-compat -Wno-c++98-compat-pedantic
SPECIAL_LD_FLAGS:=-fuse-ld=gold
endif


ifeq ($(CXX),g++-5)
SHARED_C_LD_FLAGS:=-fsanitize=undefined -O3 -march=native -fuse-linker-plugin -flto
SPECIAL_LD_FLAGS:=
WARNFLAGS:=-Wall -Wextra
endif

CFLAGS:=$(WARNFLAGS) $(SHARED_C_LD_FLAGS) -I.
CXXFLAGS:=$(CFLAGS) -std=c++14
LDFLAGS:=$(SHARED_C_LD_FLAGS) $(SPECIAL_LD_FLAGS) -lSDL2 -lSDL2_image
OBJDIR:=.obj/

SOURCE:=$(shell find -name '*.cpp' -printf '%P\n')
OBJ:=$(addprefix $(OBJDIR),$(SOURCE:%.cpp=%.o))
DEPS:=$(addprefix $(OBJDIR),$(SOURCE:%.cpp=%.d))

EXEC:=eurydice
TARED:=$(EXEC).tbz

ALLFILES:=Makefile $(SOURCE) $(shell find -name '*.h' -or -name '*.hpp' -printf '%P\n')

all: $(EXEC)

$(EXEC): $(OBJ) Makefile
	$(CXX) $(OBJ) -o $(EXEC) $(LDFLAGS)

tar:
	tar -cjf $(TARED) $(ALLFILES)

$(OBJDIR)%.o $(OBJDIR)%.P: %.cpp Makefile
	@mkdir -p $(dir $(OBJDIR)$*)
	$(CXX) -c -MD $(CXXFLAGS) $*.cpp -o $(OBJDIR)$*.o

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),tar)
ifneq ($(MAKECMDGOALS),analyze)
-include $(DEPS)
endif
endif
endif
	
clean:
	rm -rf $(OBJDIR)
	rm -f $(EXEC)
	rm -f $(TARED)

analyze:
	make clean
	scan-build -V --use-c++ clang++ make

todo:
	-@for file in $(ALLFILES); do fgrep -Hn --color=auto -e 'TO'DO -e 'FIX'ME $$file; done; true
	
.PHONY: all tar clean analyze todo
.SUFFIXES:
