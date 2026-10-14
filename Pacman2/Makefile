# macOS / SDL3 build of Pacman (LAN edition).
#   brew install sdl3
#   make && make run

SDL_PREFIX ?= $(shell brew --prefix sdl3 2>/dev/null || echo /opt/homebrew)
COMPAT   = ../Pacman/macos

CXX      ?= clang++
CXXFLAGS ?= -O2 -g -Wall
CXXFLAGS += -std=c++11 -I$(COMPAT) -I$(SDL_PREFIX)/include
LDFLAGS  += -L$(SDL_PREFIX)/lib -Wl,-rpath,$(SDL_PREFIX)/lib -lSDL3

SRCS = main.cpp game.cpp level.cpp net.cpp input.cpp
OBJDIR = build
OBJS = $(addprefix $(OBJDIR)/,$(SRCS:.cpp=.o)) $(OBJDIR)/allegro_compat.o

pacman: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

$(OBJDIR)/%.o: %.cpp $(wildcard *.h) $(COMPAT)/allegro.h
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/allegro_compat.o: $(COMPAT)/allegro_compat.cpp $(COMPAT)/allegro.h $(COMPAT)/font8x8.h
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -w -c $< -o $@

run: pacman
	./pacman

# Game-logic tests (no window or sound needed).
TEST_OBJS = $(OBJDIR)/game.o $(OBJDIR)/level.o $(OBJDIR)/net.o $(OBJDIR)/input.o \
            $(OBJDIR)/allegro_compat.o $(OBJDIR)/logic_test.o

$(OBJDIR)/logic_test.o: tests/logic_test.cpp $(wildcard *.h)
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -I. -c $< -o $@

logic_test: $(TEST_OBJS)
	$(CXX) $(TEST_OBJS) $(LDFLAGS) -o $@

test: logic_test
	./logic_test

clean:
	rm -rf $(OBJDIR) pacman logic_test

.PHONY: run test clean
