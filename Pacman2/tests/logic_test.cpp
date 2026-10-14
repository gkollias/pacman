/*
 * Game-logic tests: no window, no sound, no sockets. Multiplayer games
 * are wired together through an in-memory fake network.
 * Run from the Pacman2 directory (assets use relative paths): make test
 */
#include <stdio.h>
#include <deque>
#include <string>
#include <vector>
#include "../game.h"
#include "../net.h"

static int failures = 0, checks = 0;
#define CHECK(cond) do { checks++; if (!(cond)) { failures++; \
	printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); } } while (0)

/* ---- fake network: server game + client games, delivered in order -- */

struct Hub;
struct FakeNet : Net {
	Hub* hub;
	int id;
	FakeNet(Hub* h, int i) : hub(h), id(i) {}
	bool IsServer() const { return id == 1; }
	void Send(const std::string& line);
};

struct Hub {
	struct Packet { int from; std::string text; };
	std::deque<Packet> queue;
	std::vector<Game*> games;   /* index = player id - 1 */
	void Pump()
	{
		while (!queue.empty()) {
			Packet p = queue.front();
			queue.pop_front();
			if (p.from == 1) {
				/* server -> every client */
				for (size_t i = 1; i < games.size(); i++) games[i]->HandleMessage(p.text);
			} else {
				/* client -> server, which relays to the other clients */
				games[0]->HandleMessage(p.text);
				for (size_t i = 1; i < games.size(); i++)
					if ((int)i + 1 != p.from) games[i]->HandleMessage(p.text);
			}
		}
	}
};

void FakeNet::Send(const std::string& line)
{
	Hub::Packet p = { id, line };
	hub->queue.push_back(p);
}

/* ---- access to Game internals -------------------------------------- */

struct GameTest {
	static Level& level(Game& g) { return g.level; }
	static Actor& pac(Game& g) { return g.pac; }
	static std::vector<Ghost>& ghosts(Game& g) { return g.ghosts; }
	static PlayerInfo& me(Game& g) { return g.players[g.my_id]; }
	static PlayerInfo& player(Game& g, int id) { return g.players[id]; }
	static Phase& phase(Game& g) { return g.phase; }
	static Dir& wanted(Game& g) { return g.wanted; }
	static bool PacCanEnter(Game& g, int c, int r) { return g.PacCanEnter(c, r); }
	static void StartFright(Game& g) { g.StartFright(); }
	static void CheckCollisions(Game& g) { g.CheckCollisions(); }
	static void EatAt(Game& g, int c, int r) { g.EatAt(c, r); }
	static void SendGhostTo(Game& g, int to) { g.SendGhostTo(to); }
	static void RequestMark(Game& g, const char* w, int from, int to) { g.RequestMark(w, from, to); }
	static bool AnyonePaused(Game& g) { return g.AnyonePaused(); }

	static void StartPlaying(Game& g)
	{
		g.phase = PHASE_PLAYING;
		for (int i = 1; i <= MAX_PLAYERS; i++)
			if (g.players[i].present) g.players[i].state = PS_PLAYING;
	}
};
typedef GameTest T;

static void Run(Game& g, int ticks)
{
	Input none;
	for (int i = 0; i < ticks; i++) g.Tick(none);
}

static int CountOwned(Game& g, int owner)
{
	return T::level(g).RemainingMarks(owner);
}

/* ---- tests ---------------------------------------------------------- */

static void TestLevel()
{
	printf("level loading\n");
	Level l;
	CHECK(l.Load("./init/Grid/Grid_1.txt"));
	CHECK(l.Width() == 19 && l.Height() == 21);
	CHECK(l.RemainingMarks(0) == 153);           /* 149 dots + 4 power marks */
	CHECK(l.RemainingMarks(1) == 153);
	CHECK(l.IsWall(0, 0) && !l.IsWall(1, 1));
	CHECK(!l.IsWall(-1, 9) && !l.IsWall(19, 9));  /* tunnel wraps */
	Level l4;
	CHECK(l4.Load("./init/Grid/Grid_4.txt"));
	int total = 0;
	for (int p = 1; p <= 4; p++) { CHECK(l4.RemainingMarks(p) > 0); total += l4.RemainingMarks(p); }
	CHECK(total == 153);
}

static void TestDoorAndTunnel()
{
	printf("ghost house door and tunnel\n");
	Game g;
	CHECK(g.Init(NORMAL, MEDIUM, 1, 1, NULL));
	CHECK(!T::PacCanEnter(g, 9, 8));   /* door */
	CHECK(T::PacCanEnter(g, 9, 7));
	T::ghosts(g).clear();
	T::StartPlaying(g);
	T::pac(g).Place(2, 9);
	T::pac(g).dir = DIR_LEFT;
	T::wanted(g) = DIR_LEFT;
	Run(g, 60);
	CHECK(T::pac(g).Col() >= 14);       /* came out on the right */
	CHECK(T::pac(g).Row() == 9);
}

static void TestOneLifePerDeath()
{
	printf("one death costs one life\n");
	Game g;
	CHECK(g.Init(NORMAL, MEDIUM, 1, 1, NULL));
	for (int death = 1; death <= 4; death++) {
		T::StartPlaying(g);
		Ghost& gh = T::ghosts(g)[0];
		gh.x = T::pac(g).x;
		gh.y = T::pac(g).y;
		gh.mode = GHOST_NORMAL;
		T::CheckCollisions(g);
		CHECK(T::phase(g) == PHASE_DYING);
		Run(g, 5 * TICKS_PER_SEC);       /* long enough for the animation */
		CHECK(T::me(g).lives == 4 - death);
		if (death < 4) CHECK(T::phase(g) == PHASE_PLAYING || T::phase(g) == PHASE_READY || T::phase(g) == PHASE_DYING);
	}
	CHECK(T::phase(g) == PHASE_GAMEOVER);
	CHECK(T::me(g).lives == 0);
}

static void TestEatGhost()
{
	printf("frightened ghost is eaten, returns home, revives\n");
	Game g;
	CHECK(g.Init(NORMAL, MEDIUM, 1, 1, NULL));
	T::StartPlaying(g);
	T::StartFright(g);
	Ghost& gh = T::ghosts(g)[0];
	CHECK(gh.mode == GHOST_FRIGHTENED);
	gh.x = T::pac(g).x;
	gh.y = T::pac(g).y;
	int before = T::me(g).score;
	T::CheckCollisions(g);
	CHECK(T::ghosts(g)[0].mode == GHOST_EYES);
	CHECK(T::me(g).score == before + 200);
	CHECK(T::phase(g) == PHASE_PLAYING);
	/* Keep Pac-Man out of the way and wait for the eyes to get home. */
	bool reached_home = false, revived = false;
	for (int i = 0; i < 30 * TICKS_PER_SEC && !revived; i++) {
		T::pac(g).Place(9, 15);
		for (size_t k = 1; k < T::ghosts(g).size(); k++) T::ghosts(g)[k].Place(1, 1);
		Run(g, 1);
		const Ghost& e = T::ghosts(g)[0];
		if (e.mode == GHOST_LEAVING) reached_home = true;
		if (reached_home && e.mode == GHOST_NORMAL) revived = true;
	}
	CHECK(reached_home);
	CHECK(revived);
}

static void TestCompleteSinglePlayer()
{
	printf("clearing the maze completes the game\n");
	Game g;
	CHECK(g.Init(NORMAL, MEDIUM, 1, 1, NULL));
	T::StartPlaying(g);
	Level& l = T::level(g);
	int last = -1;
	for (int i = 0; i < l.CellCount(); i++)
		if (l.IsMark(l.AtIndex(i))) { if (last >= 0) l.AtIndex(last).eaten = true; last = i; }
	CHECK(T::phase(g) == PHASE_PLAYING);
	T::EatAt(g, last % l.Width(), last / l.Width());
	CHECK(T::phase(g) == PHASE_COMPLETED);
}

/* Two or three games connected through the fake hub. */
struct Match {
	Hub hub;
	std::vector<Game*> games;
	std::vector<FakeNet*> nets;
	Match(int players)
	{
		for (int id = 1; id <= players; id++) {
			nets.push_back(new FakeNet(&hub, id));
			games.push_back(new Game());
		}
		hub.games = games;
		/* Like the real game: every client has loaded the maze (on [START])
		   before the server's first messages arrive. */
		for (int id = players; id >= 1; id--)
			CHECK(games[id - 1]->Init(NORMAL, MEDIUM, players, id, nets[id - 1]));
		hub.Pump();
		for (int id = 1; id <= players; id++) T::StartPlaying(*games[id - 1]);
	}
	~Match()
	{
		for (size_t i = 0; i < games.size(); i++) { delete games[i]; delete nets[i]; }
	}
	Game& operator[](int id) { return *games[id - 1]; }
	/* Every instance agrees on every mark. */
	bool Consistent()
	{
		Level& a = T::level(*games[0]);
		for (size_t g = 1; g < games.size(); g++) {
			Level& b = T::level(*games[g]);
			for (int i = 0; i < a.CellCount(); i++)
				if (a.AtIndex(i).owner != b.AtIndex(i).owner || a.AtIndex(i).eaten != b.AtIndex(i).eaten)
					return false;
		}
		return true;
	}
};

static void TestMarkTransfers()
{
	printf("multiplayer: send/get marks stay consistent\n");
	Match m(2);
	int own1 = CountOwned(m[1], 1), own2 = CountOwned(m[1], 2);
	T::RequestMark(m[2], "SEND_SM", 2, 1);  /* client gives a dot away */
	m.hub.Pump();
	CHECK(CountOwned(m[1], 1) == own1 + 1 && CountOwned(m[1], 2) == own2 - 1);
	T::RequestMark(m[1], "GET_PM", 1, 2);   /* server takes a power mark */
	m.hub.Pump();
	CHECK(CountOwned(m[2], 1) == own1 + 2 && CountOwned(m[2], 2) == own2 - 2);
	CHECK(m.Consistent());

	/* Eating is seen by everyone. */
	Level& l2 = T::level(m[2]);
	for (int i = 0; i < l2.CellCount(); i++) {
		Cell& c = l2.AtIndex(i);
		if (c.type == CELL_SIMPLE_MARK && c.owner == 2 && !c.eaten) {
			T::EatAt(m[2], i % l2.Width(), i / l2.Width());
			break;
		}
	}
	m.hub.Pump();
	CHECK(CountOwned(m[1], 2) == own2 - 3);
	CHECK(T::player(m[1], 2).score == 10);
	CHECK(m.Consistent());
}

static void TestGhostTransfers()
{
	printf("multiplayer: H sends a ghost, G gets one\n");
	Match m(2);
	T::SendGhostTo(m[1], 2);                 /* H on the server */
	m.hub.Pump();
	CHECK(T::ghosts(m[1]).size() == 3);
	CHECK(T::ghosts(m[2]).size() == 5);
	CHECK(T::ghosts(m[2]).back().label == 1); /* still shows its owner */
	CHECK(T::ghosts(m[2]).back().mode == GHOST_LEAVING);
	m[1].HandleMessage("[GET_GHOST 1 2]");    /* G on the server, as seen by the client */
	m[2].HandleMessage("[GET_GHOST 1 2]");
	m.hub.Pump();
	CHECK(T::ghosts(m[1]).size() == 4);
	CHECK(T::ghosts(m[2]).size() == 4);
	for (int i = 0; i < 10; i++) T::SendGhostTo(m[1], 2);
	m.hub.Pump();
	CHECK(T::ghosts(m[1]).size() == 1);       /* the last ghost is kept */
}

static void TestPause()
{
	printf("multiplayer: pause waits for everyone to rejoin\n");
	Match m(3);
	m[2].HandleMessage("[PAUSE 2]");
	CHECK(T::AnyonePaused(m[2]));
	m[2].HandleMessage("[UNPAUSE 1]");
	m[2].HandleMessage("[UNPAUSE 2]");
	CHECK(T::AnyonePaused(m[2]));             /* player 3 still away */
	m[2].HandleMessage("[UNPAUSE 3]");
	CHECK(!T::AnyonePaused(m[2]));
}

static void TestLeaveAndGameOver()
{
	printf("multiplayer: marks are shared out when a player leaves or loses\n");
	{
		Match m(3);
		CHECK(CountOwned(m[1], 3) > 0);
		m[1].PlayerLeft(3);                    /* server notices the disconnect */
		m.hub.Pump();
		CHECK(CountOwned(m[1], 3) == 0);
		CHECK(m.Consistent());
		CHECK(T::player(m[2], 3).state == PS_LEFT);
		/* The fourth territory was shared out at the start too. */
		CHECK(CountOwned(m[2], 4) == 0);
	}
	{
		Match m(2);
		T::me(m[2]).lives = 1;
		Ghost& gh = T::ghosts(m[2])[0];
		gh.x = T::pac(m[2]).x;
		gh.y = T::pac(m[2]).y;
		T::CheckCollisions(m[2]);
		Run(m[2], 5 * TICKS_PER_SEC);
		m.hub.Pump();
		CHECK(T::phase(m[2]) == PHASE_GAMEOVER);
		CHECK(T::player(m[1], 2).state == PS_GAMEOVER);
		CHECK(CountOwned(m[1], 2) == 0);
		CHECK(m.Consistent());
		CHECK(T::phase(m[1]) == PHASE_PLAYING);  /* the server plays on */
	}
	{
		Match m(2);
		m[2].ServerLost();
		CHECK(CountOwned(m[2], 1) == 0);         /* client takes over everything */
		CHECK(T::player(m[2], 1).state == PS_LEFT);
	}
}

static void TestTeamCompletion()
{
	printf("multiplayer: the team finishes when every mark is eaten\n");
	Match m(2);
	Level& l1 = T::level(m[1]);
	for (int i = 0; i < l1.CellCount(); i++) {
		Cell& c = l1.AtIndex(i);
		if (l1.IsMark(c) && c.owner == 1) T::EatAt(m[1], i % l1.Width(), i / l1.Width());
	}
	m.hub.Pump();
	CHECK(T::phase(m[1]) == PHASE_PLAYING);   /* player 2 still has marks */
	Level& l2 = T::level(m[2]);
	for (int i = 0; i < l2.CellCount(); i++) {
		Cell& c = l2.AtIndex(i);
		if (l2.IsMark(c) && c.owner == 2 && !c.eaten) T::EatAt(m[2], i % l2.Width(), i / l2.Width());
	}
	m.hub.Pump();
	CHECK(T::phase(m[1]) == PHASE_COMPLETED);
	CHECK(T::phase(m[2]) == PHASE_COMPLETED);
}

int main()
{
	TestLevel();
	TestDoorAndTunnel();
	TestOneLifePerDeath();
	TestEatGhost();
	TestCompleteSinglePlayer();
	TestMarkTransfers();
	TestGhostTransfers();
	TestPause();
	TestLeaveAndGameOver();
	TestTeamCompletion();
	printf("%d checks, %d failed\n", checks, failures);
	return failures ? 1 : 0;
}
