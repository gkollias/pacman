# macOS / SDL3 build of the Pacman game.
#   brew install sdl3
#   make && make run

SDL_PREFIX ?= $(shell brew --prefix sdl3 2>/dev/null || echo /opt/homebrew)

CXX      ?= clang++
CXXFLAGS ?= -O2 -g -w
CXXFLAGS += -std=c++11 -Imacos -I$(SDL_PREFIX)/include
LDFLAGS  += -L$(SDL_PREFIX)/lib -Wl,-rpath,$(SDL_PREFIX)/lib -lSDL3

SRCS = ai.cpp animation.cpp animationFilm.cpp animationFilmHolder.cpp \
       animator.cpp animatorHolder.cpp bitmapLoader.cpp collisionchecker.cpp \
       frameRangeAnimation.cpp frameRangeAnimator.cpp game.cpp \
       gameInitialization.cpp ghost.cpp latelyDestroyable.cpp main.cpp \
       movingAnimation.cpp movingAnimator.cpp obstacle.cpp obstacleHolder.cpp \
       ObstaclePainter.cpp pacman.cpp Player.cpp PlayerHolder.cpp powerMark.cpp \
       powerMarkHolder.cpp rendering.cpp simpleMark.cpp simpleMarkHolder.cpp \
       sprite.cpp macos/allegro_compat.cpp

OBJDIR = build
OBJS   = $(addprefix $(OBJDIR)/,$(SRCS:.cpp=.o))

pacman: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

$(OBJDIR)/%.o: %.cpp $(wildcard *.h) $(wildcard macos/*.h)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: pacman
	./pacman

clean:
	rm -rf $(OBJDIR) pacman

.PHONY: run clean
