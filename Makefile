# -*- mode: Makefile; -*-

CPPFLAGS = -I.

CXXFLAGS = -ggdb3 -O0 -std=c++1z -W -Wall
LIBS = -lboost_unit_test_framework -lboost_iostreams -lstdc++fs

DEPENDDIR = ./.deps
DEPENDFLAGS = -M

SRCS := $(wildcard *.cc)
OBJS := $(patsubst %.cc,%.o,$(SRCS))

TARGETS = fofi test

all: $(TARGETS)

DEPS = $(patsubst %.o,$(DEPENDDIR)/%.d,$(OBJS))
-include $(DEPS)

$(DEPENDDIR)/%.d: %.cc $(DEPENDDIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPENDFLAGS) $< >$@

$(DEPENDDIR):
	@[ ! -d $(DEPENDDIR) ] && mkdir -p $(DEPENDDIR)

%: %.cc

fofi: fofi.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

test: test.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJS) $(TARGETS)
