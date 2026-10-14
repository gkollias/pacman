#ifndef LEVEL_H_
#define LEVEL_H_

#include <string>
#include <vector>
#include "common.h"

/* One cell of an init/Grid/Grid_N.txt file. Each cell is a "TYPE value"
   pair: OB <wall tile id>, SM <owner player>, PM <owner player>, EM 0. */
enum CellType { CELL_EMPTY, CELL_WALL, CELL_SIMPLE_MARK, CELL_POWER_MARK };

struct Cell {
	CellType type;
	int wall_tile;  /* obstacle bitmap index for CELL_WALL */
	int owner;      /* player id that owns a mark (1-based) */
	bool eaten;
};

class Level {
	int width, height;
	std::vector<Cell> cells;
public:
	Level();
	bool Load(const std::string& path);

	int Width() const { return width; }
	int Height() const { return height; }

	/* Columns wrap around (the tunnel row); rows outside the grid are walls. */
	int WrapCol(int col) const;
	Cell& At(int col, int row);
	const Cell& At(int col, int row) const;
	int Index(int col, int row) const { return row * width + WrapCol(col); }
	Cell& AtIndex(int idx) { return cells[idx]; }
	int CellCount() const { return (int)cells.size(); }

	bool IsWall(int col, int row) const;
	/* The ghost house door; only ghosts going home or leaving may pass it. */
	bool IsDoor(int col, int row) const { return WrapCol(col) == DoorCol() && row == DoorRow(); }
	bool IsMark(const Cell& c) const { return c.type == CELL_SIMPLE_MARK || c.type == CELL_POWER_MARK; }

	int DoorCol() const { return width / 2; }
	int DoorRow() const { return 8; }
	int HomeRow() const { return 9; }     /* inside the ghost house */
	int ExitRow() const { return 7; }     /* just above the door */
	bool InTunnel(int col, int row) const { return row == 9 && (WrapCol(col) < 4 || WrapCol(col) >= width - 4); }

	int RemainingMarks(int owner) const;  /* owner 0 = everyone */
};

#endif
