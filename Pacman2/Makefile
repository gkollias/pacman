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

# Double-clickable app: assets in Resources, SDL3 bundled in Frameworks,
# ad-hoc signed. (Apple Silicon, like the Homebrew SDL3 it bundles.)
APP = Pacman.app
SDL_DYLIB = $(SDL_PREFIX)/lib/libSDL3.0.dylib

app: pacman macos/Info.plist macos/Pacman.icns
	rm -rf $(APP)
	mkdir -p $(APP)/Contents/MacOS $(APP)/Contents/Resources $(APP)/Contents/Frameworks
	cp macos/Info.plist $(APP)/Contents/
	cp macos/Pacman.icns $(APP)/Contents/Resources/
	cp -R images init sounds $(APP)/Contents/Resources/
	cp pacman $(APP)/Contents/MacOS/Pacman
	cp $(SDL_DYLIB) $(APP)/Contents/Frameworks/
	chmod u+w $(APP)/Contents/Frameworks/libSDL3.0.dylib
	install_name_tool -id @rpath/libSDL3.0.dylib $(APP)/Contents/Frameworks/libSDL3.0.dylib
	install_name_tool -change $(SDL_DYLIB) @rpath/libSDL3.0.dylib \
		-delete_rpath $(SDL_PREFIX)/lib -add_rpath @executable_path/../Frameworks \
		$(APP)/Contents/MacOS/Pacman
	codesign --force --sign - $(APP)/Contents/Frameworks/libSDL3.0.dylib
	codesign --force --sign - $(APP)

# Browser build (single player): WebAssembly via Emscripten's SDL3 port.
#   brew install emscripten && make web && make serve
# -sASYNCIFY lets the game's waits (rest) yield to the browser.
EMXX     ?= em++
WEB_OUT   = web
WEB_SRCS  = $(filter-out net.cpp,$(SRCS)) net_web.cpp   # WebSocket to the relay instead of TCP
WEB_FLAGS = -std=c++11 -O2 -I$(COMPAT) --use-port=sdl3 -sASYNCIFY -sALLOW_MEMORY_GROWTH -lwebsocket.js \
            --preload-file images@/images --preload-file init@/init --preload-file sounds@/sounds \
            --shell-file web_shell.html

web: $(WEB_SRCS) $(COMPAT)/allegro_compat.cpp $(wildcard *.h) $(COMPAT)/allegro.h web_shell.html
	@mkdir -p $(WEB_OUT)
	$(EMXX) $(WEB_FLAGS) $(WEB_SRCS) $(COMPAT)/allegro_compat.cpp -o $(WEB_OUT)/index.html
	@# Version the game files so a new deploy never mixes with cached old ones.
	@id=$$(git rev-parse --short HEAD 2>/dev/null || echo dev)-$$(date +%s); \
	sed -i.bak -E -e "s/__BUILD_ID__/$$id/" -e "s#src=\"?index\.js\"?#src=\"index.js?v=$$id\"#" $(WEB_OUT)/index.html && \
	rm -f $(WEB_OUT)/index.html.bak && echo "web build $$id"

serve: web
	python3 -m http.server 8080 --directory $(WEB_OUT)

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
	rm -rf $(OBJDIR) pacman logic_test $(APP) $(WEB_OUT)

.PHONY: run test app web serve clean
