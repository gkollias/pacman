/*
 * Game-logic tests: no window, no sound, no sockets. Multiplayer games
 * are wired together through an in-memory fake network.
 * Run from the Pacman2 directory (assets use relative paths): make test
 */
#include <stdio.h>
#include <stdlib.h>
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
	static std::vector<MovingWall>& walls(Game& g) { return g.walls; }
	static std::vector<Ghost>& remote_ghosts(Game& g) { return g.remote_ghosts; }
	static std::map<int, RemotePac>& remote_pacs(Game& g) { return g.remote_pacs; }
	static PlayerInfo& me(Game& g) { return g.players[g.my_id]; }
	static PlayerInfo& player(Game& g, int id) { return g.players[id]; }
	static Phase& phase(Game& g) { return g.phase; }
	static Dir& wanted(Game& g) { return g.wanted; }
	static int terrain(Game& g) { return g.pac_terrain; }
	static int fright(Game& g) { return g.fright_ticks; }
	static bool PacCanEnter(Game& g, int c, int r) { return g.PacCanEnter(c, r); }
	static void StartFright(Game& g) { g.StartFright(); }
	static void CheckCollisions(Game& g) { g.CheckCollisions(); }
	static void CheckCornered(Game& g) { g.CheckCornered(); }
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

/* The first uneaten mark of a type and owner, or -1. */
static int FindMark(Game& g, CellType type, int owner)
{
	Level& l = T::level(g);
	for (int i = 0; i < l.MarkCount(); i++) {
		const Mark& m = l.GetMark(i);
		if (!m.eaten && m.type == type && m.owner == owner) return i;
	}
	return -1;
}

static bool BoxOverlapsCell(int x, int y, int col, int row, int width)
{
	if (y >= (row + 1) * TILE || y + TILE <= row * TILE) return false;
	for (int s = -1; s <= 1; s++) {
		int cx = (col + s * width) * TILE;
		if (x < cx + TILE && x + TILE > cx) return true;
	}
	return false;
}

static bool WallBlocks(Game& g, int c, int r)
{
	std::vector<MovingWall>& ws = T::walls(g);
	for (size_t i = 0; i < ws.size(); i++) if (ws[i].Blocks(c, r)) return true;
	return false;
}

/* First step of a shortest path from Pac-Man to (tc, tr). */
static Dir ChaseStep(Game& g, int tc, int tr)
{
	Level& l = T::level(g);
	int w = l.Width(), h = l.Height();
	std::vector<int> first(w * h, -1);
	std::deque<int> q;
	int sc = l.WrapCol(T::pac(g).Col()), sr = T::pac(g).Row();
	first[sr * w + sc] = 4;
	q.push_back(sr * w + sc);
	while (!q.empty()) {
		int cur = q.front(); q.pop_front();
		int c = cur % w, r = cur / w;
		if (c == tc && r == tr) return first[cur] == 4 ? DIR_NONE : (Dir)first[cur];
		for (int d = 0; d < 4; d++) {
			int nc = l.WrapCol(c + DIR_DX[d]), nr = r + DIR_DY[d];
			if (!T::PacCanEnter(g, nc, nr) || first[nr * w + nc] != -1) continue;
			first[nr * w + nc] = first[cur] == 4 ? d : first[cur];
			q.push_back(nr * w + nc);
		}
	}
	return DIR_NONE;
}

/* ---- single player ------------------------------------------------- */

static void TestLevel()
{
	printf("level loading\n");
	Level l;
	CHECK(l.Load("./init/Grid/Grid_1.txt"));
	CHECK(l.Width() == 19 && l.Height() == 21);
	CHECK(l.MarkCount() == 153);                 /* 149 dots + 4 power marks */
	CHECK(l.RemainingMarks(1) == 153);
	CHECK(l.IsWall(0, 0) && !l.IsWall(1, 1));
	CHECK(!l.IsWall(-1, 9) && !l.IsWall(19, 9));  /* tunnel wraps */
	CHECK(l.MarkAt(1, 1) >= 0 && l.MarkAt(0, 9) < 0);
	CHECK(l.IsTrack(1, 1) && !l.IsTrack(0, 9));
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
	CHECK(g.Init(NORMAL, MEDIUM, 1, 1, NULL, 0));
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
	CHECK(g.Init(NORMAL, MEDIUM, 1, 1, NULL, 0));
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
	}
	CHECK(T::phase(g) == PHASE_GAMEOVER);
	CHECK(T::me(g).lives == 0);
}

static void TestEatGhost()
{
	printf("frightened ghost is eaten, returns home, revives\n");
	Game g;
	CHECK(g.Init(NORMAL, MEDIUM, 1, 1, NULL, 0));
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
	CHECK(g.Init(NORMAL, MEDIUM, 1, 1, NULL, 0));
	T::StartPlaying(g);
	Level& l = T::level(g);
	for (int i = 0; i + 1 < l.MarkCount(); i++) l.GetMark(i).eaten = true;
	CHECK(T::phase(g) == PHASE_PLAYING);
	const Mark& last = l.GetMark(l.MarkCount() - 1);
	T::EatAt(g, last.col, last.row);
	CHECK(T::phase(g) == PHASE_COMPLETED);
}

static void TestGates()
{
	printf("gates open and close passages, never on anyone\n");
	srand(1);
	Game g;
	CHECK(g.Init(NORMAL, MEDIUM, 1, 1, NULL, OPT_MOVING_WALLS));
	std::vector<MovingWall>& ws = T::walls(g);
	CHECK(ws.size() == 5);
	T::StartPlaying(g);
	T::me(g).lives = 1000;   /* keep playing through deaths */
	static const Dir DIRS[4] = { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };
	int w = T::level(g).Width();
	std::vector<int> open_ticks(ws.size(), 0), closed_ticks(ws.size(), 0), passed(ws.size(), 0);
	bool crushed = false;
	const int TICKS = 180 * TICKS_PER_SEC;
	for (int t = 0; t < TICKS; t++) {
		if (t % 30 == 0) T::wanted(g) = DIRS[rand() % 4];  /* wander about */
		Run(g, 1);
		if (T::phase(g) != PHASE_PLAYING) continue;
		for (size_t i = 0; i < ws.size(); i++) {
			bool blocks = ws[i].Blocks(ws[i].col, ws[i].row);
			(blocks ? closed_ticks : open_ticks)[i]++;
			bool someone = BoxOverlapsCell(T::pac(g).x, T::pac(g).y, ws[i].col, ws[i].row, w);
			for (size_t j = 0; j < T::ghosts(g).size(); j++)
				if (BoxOverlapsCell(T::ghosts(g)[j].x, T::ghosts(g)[j].y, ws[i].col, ws[i].row, w)) someone = true;
			if (someone && blocks && ws[i].slide == 0 && ws[i].closed) crushed = true;
			if (someone && !blocks) passed[i]++;
		}
	}
	int total_passed = 0;
	for (size_t i = 0; i < ws.size(); i++) {
		int total = open_ticks[i] + closed_ticks[i];
		/* Each gate spends a fair share of the time open and closed. */
		CHECK(open_ticks[i] > total / 5 && closed_ticks[i] > total / 5);
		total_passed += passed[i];
	}
	CHECK(total_passed > 0);                  /* someone walked through an open gate */
	CHECK(!crushed);

	/* A gate never closes on someone standing in it. */
	Game h;
	CHECK(h.Init(NORMAL, MEDIUM, 1, 1, NULL, OPT_MOVING_WALLS));
	T::StartPlaying(h);
	T::ghosts(h).clear();
	T::pac(h).Place(3, 9);                    /* in the left tunnel gate's cell */
	bool closed_on_pac = false;
	for (int t = 0; t < 20 * TICKS_PER_SEC; t++) {
		Run(h, 1);
		T::pac(h).Place(3, 9);
		if (WallBlocks(h, 3, 9)) closed_on_pac = true;
	}
	CHECK(!closed_on_pac);
}

static void TestFleeingDots()
{
	printf("dots run away from Pac-Man\n");
	srand(2);
	{
		/* A power dot pushes past an ordinary dot. */
		Game g;
		CHECK(g.Init(NORMAL, MEDIUM, 1, 1, NULL, OPT_MOVING_DOTS));
		T::StartPlaying(g);
		T::ghosts(g).clear();
		Level& l = T::level(g);
		int pm = l.MarkAt(1, 2), dot = l.MarkAt(1, 1);
		CHECK(pm >= 0 && l.GetMark(pm).type == CELL_POWER_MARK && dot >= 0);
		T::pac(g).Place(1, 4);                /* two cells below it */
		Run(g, 3);
		CHECK(l.GetMark(pm).col == 1 && l.GetMark(pm).row == 1);    /* fled up */
		CHECK(l.GetMark(dot).col == 1 && l.GetMark(dot).row == 2);  /* swapped down */
		CHECK(!l.GetMark(pm).eaten && !l.GetMark(dot).eaten);
	}
	{
		/* Ordinary dots stay put while packed in, but run once there is room. */
		Game g;
		CHECK(g.Init(NORMAL, MEDIUM, 1, 1, NULL, OPT_MOVING_DOTS));
		T::StartPlaying(g);
		T::ghosts(g).clear();
		Level& l = T::level(g);
		int d = l.MarkAt(6, 1);
		T::pac(g).Place(4, 1);
		Run(g, 30);
		CHECK(l.GetMark(d).col == 6 && l.GetMark(d).row == 1);       /* boxed in by dots */
		l.GetMark(l.MarkAt(7, 1)).eaten = true;                       /* clear a cell behind it */
		Run(g, 30);
		CHECK(l.GetMark(d).col == 7 && l.GetMark(d).row == 1);
		/* No two live dots ever share a cell. */
		bool stacked = false;
		for (int i = 0; i < l.MarkCount(); i++)
			for (int j = i + 1; j < l.MarkCount(); j++)
				if (!l.GetMark(i).eaten && !l.GetMark(j).eaten &&
				    l.GetMark(i).col == l.GetMark(j).col && l.GetMark(i).row == l.GetMark(j).row) stacked = true;
		CHECK(!stacked);
	}
	{
		/* Chasing a power dot takes longer when it runs. */
		int took[2];
		for (int flee = 0; flee <= 1; flee++) {
			srand(5);
			Game g;
			CHECK(g.Init(NORMAL, MEDIUM, 1, 1, NULL, flee ? OPT_MOVING_DOTS : 0));
			T::StartPlaying(g);
			T::ghosts(g).clear();
			Level& l = T::level(g);
			int pm = l.MarkAt(1, 14);
			T::pac(g).Place(9, 15);
			took[flee] = 60 * TICKS_PER_SEC;
			for (int t = 0; t < 60 * TICKS_PER_SEC; t++) {
				if (T::pac(g).AtCenter()) T::wanted(g) = ChaseStep(g, l.GetMark(pm).col, l.GetMark(pm).row);
				Run(g, 1);
				if (l.GetMark(pm).eaten) { took[flee] = t; break; }
			}
		}
		printf("    catching a power dot: %.1f s standing still, %.1f s running away\n",
		       took[0] / (float)TICKS_PER_SEC, took[1] / (float)TICKS_PER_SEC);
		CHECK(took[0] < 60 * TICKS_PER_SEC);
		CHECK(took[1] > took[0] + TICKS_PER_SEC);
	}
}

/* ---- multiplayer ---------------------------------------------------- */

/* Two to four games connected through the fake hub. */
struct Match {
	Hub hub;
	std::vector<Game*> games;
	std::vector<FakeNet*> nets;
	Match(int players, int options = 0)
	{
		for (int id = 1; id <= players; id++) {
			nets.push_back(new FakeNet(&hub, id));
			games.push_back(new Game());
		}
		hub.games = games;
		/* Like the real game: every client has loaded the maze (on [START])
		   before the server's first messages arrive. */
		for (int id = players; id >= 1; id--)
			CHECK(games[id - 1]->Init(NORMAL, MEDIUM, players, id, nets[id - 1], options));
		hub.Pump();
		for (int id = 1; id <= players; id++) T::StartPlaying(*games[id - 1]);
	}
	~Match()
	{
		for (size_t i = 0; i < games.size(); i++) { delete games[i]; delete nets[i]; }
	}
	Game& operator[](int id) { return *games[id - 1]; }
	/* Runs every game for some ticks, delivering messages after each. */
	void Run(int ticks)
	{
		Input none;
		for (int t = 0; t < ticks; t++) {
			for (size_t i = 0; i < games.size(); i++) games[i]->Tick(none);
			hub.Pump();
		}
	}
	/* Every instance agrees on every mark. */
	bool Consistent()
	{
		Level& a = T::level(*games[0]);
		for (size_t g = 1; g < games.size(); g++) {
			Level& b = T::level(*games[g]);
			for (int i = 0; i < a.MarkCount(); i++) {
				const Mark& x = a.GetMark(i);
				const Mark& y = b.GetMark(i);
				if (x.owner != y.owner || x.eaten != y.eaten || x.col != y.col || x.row != y.row) return false;
			}
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

	int id = FindMark(m[2], CELL_SIMPLE_MARK, 2);
	const Mark& mk = T::level(m[2]).GetMark(id);
	T::EatAt(m[2], mk.col, mk.row);         /* eating is seen by everyone */
	m.hub.Pump();
	CHECK(CountOwned(m[1], 2) == own2 - 3);
	CHECK(T::player(m[1], 2).score == 10);
	CHECK(m.Consistent());
}

static void TestDotRace()
{
	printf("multiplayer: a dot moved and eaten at once stays consistent\n");
	Match m(2, OPT_MOVING_DOTS);
	int id = FindMark(m[2], CELL_POWER_MARK, 2);
	Mark& mk = T::level(m[2]).GetMark(id);
	T::EatAt(m[2], mk.col, mk.row);         /* client eats it ... */
	m[1].HandleMessage("[GHOST 9 0 0 0 0 0]");  /* (no-op; keeps the queue busy) */
	char move[64];                            /* ... while the server moves it */
	snprintf(move, sizeof(move), "[MOVE_DOT %d %d %d]", id, 1, 1);
	T::level(m[1]).GetMark(id).col = 1;
	T::level(m[1]).GetMark(id).row = 1;
	m.nets[0]->Send(move);
	m.hub.Pump();
	CHECK(T::level(m[1]).GetMark(id).eaten);
	CHECK(m.Consistent());
	/* And a real run with the server moving dots keeps everyone in step. */
	for (int i = 0; i < T::level(m[1]).MarkCount(); i++)
		if (T::level(m[1]).GetMark(i).type == CELL_SIMPLE_MARK) {
			T::level(m[1]).GetMark(i).eaten = true;
			T::level(m[2]).GetMark(i).eaten = true;
		}
	m.Run(20 * TICKS_PER_SEC);
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
	m[2].HandleMessage("[GET_GHOST 1 2]");    /* G on the server, as seen by the client */
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
		CHECK(CountOwned(m[2], 4) == 0);       /* the fourth territory was shared at the start */
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
	for (int id; (id = FindMark(m[1], CELL_SIMPLE_MARK, 1)) >= 0 || (id = FindMark(m[1], CELL_POWER_MARK, 1)) >= 0; )
		T::EatAt(m[1], T::level(m[1]).GetMark(id).col, T::level(m[1]).GetMark(id).row);
	m.hub.Pump();
	CHECK(T::phase(m[1]) == PHASE_PLAYING);   /* player 2 still has marks */
	for (int id; (id = FindMark(m[2], CELL_SIMPLE_MARK, 2)) >= 0 || (id = FindMark(m[2], CELL_POWER_MARK, 2)) >= 0; )
		T::EatAt(m[2], T::level(m[2]).GetMark(id).col, T::level(m[2]).GetMark(id).row);
	m.hub.Pump();
	CHECK(T::phase(m[1]) == PHASE_COMPLETED);
	CHECK(T::phase(m[2]) == PHASE_COMPLETED);
}

static void TestCornered()
{
	printf("multiplayer: a cornered Pac-Man sends the nearest ghost away\n");
	Match m(2, OPT_AUTO_SEND);
	Game& s = m[1];
	/* (3,13) is a corridor open only to the left and right. */
	T::pac(s).Place(3, 13);
	std::vector<Ghost>& gs = T::ghosts(s);
	for (size_t i = 0; i < gs.size(); i++) { gs[i].Place(17, 19); gs[i].mode = GHOST_NORMAL; }
	gs[0].Place(5, 13);                       /* one ghost on the right only */
	T::CheckCornered(s);
	m.hub.Pump();
	CHECK(gs.size() == 4);                    /* not cornered yet */
	gs[1].Place(1, 13);                       /* and now one on the left */
	T::CheckCornered(s);
	m.hub.Pump();
	CHECK(T::ghosts(s).size() == 3);
	CHECK(T::ghosts(m[2]).size() == 5);
	T::CheckCornered(s);                      /* cooldown: no second send at once */
	m.hub.Pump();
	CHECK(T::ghosts(s).size() == 3);
}

static void TestTerrains()
{
	printf("multiplayer: the tunnel leads into the neighbour's maze\n");
	Match m(2, OPT_TERRAINS);
	Game& s = m[1];
	Game& c = m[2];
	/* Park every ghost far from the tunnel so nothing interferes. */
	for (int g = 1; g <= 2; g++)
		for (size_t i = 0; i < T::ghosts(m[g]).size(); i++) T::ghosts(m[g])[i].Place(9, 3);
	T::pac(s).Place(17, 9);
	T::pac(s).dir = DIR_RIGHT;
	T::wanted(s) = DIR_RIGHT;
	for (int t = 0; t < 2 * TICKS_PER_SEC && T::terrain(s) == 1; t++) m.Run(1);
	CHECK(T::terrain(s) == 2);               /* player 1 is now in player 2's maze */
	CHECK(T::pac(s).Col() <= 1);             /* entering from the left */
	m.Run(5);
	CHECK(T::remote_pacs(c)[1].terrain == 2);
	CHECK(T::remote_ghosts(s).size() == T::ghosts(c).size());  /* sees the host's ghosts */

	/* A power dot eaten while visiting frightens the host's ghosts, not mine. */
	int pm = FindMark(s, CELL_POWER_MARK, 1);
	T::EatAt(s, T::level(s).GetMark(pm).col, T::level(s).GetMark(pm).row);
	m.hub.Pump();
	CHECK(T::fright(c) > 0);
	CHECK(T::fright(s) == 0);
	/* Let the fright wear off, with the host's ghosts kept out of the way. */
	for (int t = 0; t < 10 * TICKS_PER_SEC; t++) {
		for (size_t i = 0; i < T::ghosts(c).size(); i++) T::ghosts(c)[i].Place(9, 3);
		m.Run(1);
	}
	CHECK(T::terrain(s) == 2);

	/* The host's ghost catches the visitor: it dies and goes home. */
	T::wanted(s) = DIR_NONE;
	T::pac(s).dir = DIR_NONE;
	m.Run(2);
	/* (The host's maze is frozen while its own Pac-Man respawns.) */
	T::StartPlaying(c);
	T::pac(c).Place(9, 15);
	Ghost& gh = T::ghosts(c)[0];
	gh.x = T::pac(s).x;
	gh.y = T::pac(s).y;
	gh.mode = GHOST_NORMAL;
	gh.dir = DIR_NONE;
	int lives = T::me(s).lives;
	m.Run(2);
	CHECK(T::phase(s) == PHASE_DYING);
	m.Run(5 * TICKS_PER_SEC);
	CHECK(T::me(s).lives == lives - 1);
	CHECK(T::terrain(s) == 1);
	CHECK(T::remote_pacs(c)[1].terrain == 1);

	/* A ghost leaving through the tunnel moves to the next maze. */
	size_t before_s = T::ghosts(s).size(), before_c = T::ghosts(c).size();
	Ghost& wanderer = T::ghosts(s)[0];
	wanderer.Place(18, 9);
	wanderer.dir = DIR_RIGHT;
	wanderer.mode = GHOST_NORMAL;
	T::StartPlaying(s);
	T::StartPlaying(c);
	m.Run(TICKS_PER_SEC);
	CHECK(T::ghosts(s).size() == before_s - 1);
	CHECK(T::ghosts(c).size() == before_c + 1);
}

int main()
{
	if (getenv("SEED")) srand(atoi(getenv("SEED")));  /* SEED=n ./logic_test */
	TestLevel();
	TestDoorAndTunnel();
	TestOneLifePerDeath();
	TestEatGhost();
	TestCompleteSinglePlayer();
	TestGates();
	TestFleeingDots();
	TestMarkTransfers();
	TestDotRace();
	TestGhostTransfers();
	TestPause();
	TestLeaveAndGameOver();
	TestTeamCompletion();
	TestCornered();
	TestTerrains();
	printf("%d checks, %d failed\n", checks, failures);
	return failures ? 1 : 0;
}
