.PHONY: all clean

name = tilecount

FLTKCONFIG = $(shell which fltk-config13 || which fltk-config)

CXXFLAGS += -Wall -Wextra
LDFLAGS += -Wl,-gc-sections -lpng

CXXFLAGS += $(shell $(FLTKCONFIG) --cxxflags)
LDFLAGS += $(shell $(FLTKCONFIG) --ldflags)

src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)

all: $(name)

$(name): $(obj)
	$(CXX) -o $(name) $(obj) $(CXXFLAGS) $(LDFLAGS)

$(obj): $(wildcard *.h)
