#ifndef LEVEL_H_
#define LEVEL_H_

#include <string>
#include <vector>
#include "common.h"

/* One cell of an init/Grid/Grid_N.txt file. Each cell is a "TYPE value"
   pair: OB <wall tile id>, SM <owner player>, PM <owner player>, EM 0.
   Marks (dots) are kept in their own list so they can move. */
enum CellType { CELL_EMPTY, CELL_WALL, CELL_SIMPLE_MARK, CELL_POWER_MARK };

struct Cell {
	CellType type;   /* CELL_EMPTY or CELL_WALL */
	int wall_tile;   /* obstacle bitmap index for CELL_WALL */
};

/* A dot. Its id (index in the list) never changes, so players can refer
   to it while it moves, changes owner or is eaten. */
struct Mark {
	int col, row;
	CellType type;   /* CELL_SIMPLE_MARK or CELL_POWER_MARK */
	int owner;       /* player id (1-based) */
	bool eaten;
};

class Level {
	int width, height;
	std::vector<Cell> cells;
	std::vector<Mark> marks;
	std::vector<bool> track;  /* cells that started with a dot */
public:
	Level();
	bool Load(const std::string& path);

	int Width() const { return width; }
	int Height() const { return height; }

	/* Columns wrap around (the tunnel row); rows outside the grid are walls. */
	int WrapCol(int col) const;
	const Cell& At(int col, int row) const { return cells[row * width + WrapCol(col)]; }
	bool IsWall(int col, int row) const;

	/* The ghost house: only ghosts going home or leaving may pass the door. */
	bool IsDoor(int col, int row) const { return WrapCol(col) == DoorCol() && row == DoorRow(); }
	bool InHouse(int col, int row) const { return row >= 8 && row <= 10 && WrapCol(col) >= 8 && WrapCol(col) <= 10; }
	int DoorCol() const { return width / 2; }
	int DoorRow() const { return 8; }
	int HomeRow() const { return 9; }     /* inside the ghost house */
	int ExitRow() const { return 7; }     /* just above the door */
	int TunnelRow() const { return 9; }
	bool InTunnel(int col, int row) const { return row == TunnelRow() && (WrapCol(col) < 4 || WrapCol(col) >= width - 4); }

	/* Cells that held a dot at the start: the corridors dots may wander along. */
	bool IsTrack(int col, int row) const { return row >= 0 && row < height && track[row * width + WrapCol(col)]; }

	int MarkCount() const { return (int)marks.size(); }
	Mark& GetMark(int id) { return marks[id]; }
	const Mark& GetMark(int id) const { return marks[id]; }
	bool ValidMark(int id) const { return id >= 0 && id < (int)marks.size(); }
	/* The uneaten mark on a cell, or -1. */
	int MarkAt(int col, int row) const;
	int RemainingMarks(int owner) const;  /* owner 0 = everyone */
};

#endif
