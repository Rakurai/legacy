INCDIR  = include
OBJDIR  = .obj
BINDIR  = ../bin
DEPDIR  = .dep
EXTDIR  = deps
EXEFILE = legacy

$(shell mkdir -p $(DEPDIR) >/dev/null)
$(shell mkdir -p $(OBJDIR) >/dev/null)

CXX      = g++
STD      = -std=c++0x
WARNING  = -Wall -Winline -Wno-write-strings -Wno-format-security -Wno-sign-compare
DEBUG    = -ggdb
DEFINE   = -Dunix -DSQL -DDEBUG -DIPV6
INCLUDE  = -I$(INCDIR) -I/usr/include -I/usr/local/include
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
CPPFLAGS = ${STD} ${WARNING} ${DEBUG} ${DEFINE} ${INCLUDE}
L_FLAGS  = -lsqlite3 -lm

COMPILE.c  = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c
COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c
POSTCOMPILE = @mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

DATE=`date +%Y-%m-%d`

EXT_SOURCES = cJSON/cJSON.c
EXT_OBJECTS = $(patsubst %,$(EXTDIR)/%.o,$(basename $(EXT_SOURCES)))

CPP_SOURCES := $(shell find . -maxdepth 2 -name '*.cc' -print | sed 's|./||' | sort)
CPP_OBJECTS = $(patsubst %,$(OBJDIR)/%.o,$(basename $(CPP_SOURCES)))

legacy: $(EXT_OBJECTS) $(CPP_OBJECTS)
	$(CXX) -o $@ $^ $(CPPFLAGS) $(L_FLAGS)

$(OBJDIR)/%.o: %.c $(DEPDIR)/%.d
	$(COMPILE.c) -o $@ $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cc $(DEPDIR)/%.d
	$(COMPILE.cc) -o $@ $<
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(CPP_SOURCES))))

clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCDIR)/*~

cleaner: clean
	rm -rf ${EXEFILE} $(DEPDIR)/*.d
	cd sand && make cleaner

install:
	mv ${BINDIR}/${EXEFILE} ${BINDIR}/${EXEFILE}.bak
	cp ${EXEFILE} ${BINDIR}
