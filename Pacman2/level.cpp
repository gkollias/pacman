#include "level.h"
#include <stdio.h>
#include <string.h>

Level::Level() : width(0), height(0) {}

bool Level::Load(const std::string& path)
{
	FILE* fp = fopen(path.c_str(), "r");
	if (!fp) {
		allegro_message("Cannot open %s", path.c_str());
		return false;
	}
	if (fscanf(fp, " Height:%d Width:%d", &height, &width) != 2 || width <= 0 || height <= 0) {
		fclose(fp);
		allegro_message("Bad grid header in %s", path.c_str());
		return false;
	}
	cells.assign(width * height, Cell());
	for (int i = 0; i < width * height; i++) {
		char type[8];
		int value = 0;
		if (fscanf(fp, "%7s %d", type, &value) != 2) {
			fclose(fp);
			allegro_message("Grid %s ends early at cell %d", path.c_str(), i);
			return false;
		}
		Cell& c = cells[i];
		c.type = CELL_EMPTY;
		c.wall_tile = 0;
		c.owner = 0;
		c.eaten = false;
		if (!strcmp(type, "OB")) { c.type = CELL_WALL; c.wall_tile = value; }
		else if (!strcmp(type, "SM")) { c.type = CELL_SIMPLE_MARK; c.owner = value; }
		else if (!strcmp(type, "PM")) { c.type = CELL_POWER_MARK; c.owner = value; }
	}
	fclose(fp);
	return true;
}

int Level::WrapCol(int col) const
{
	col %= width;
	return col < 0 ? col + width : col;
}

Cell& Level::At(int col, int row) { return cells[Index(col, row)]; }
const Cell& Level::At(int col, int row) const { return cells[Index(col, row)]; }

bool Level::IsWall(int col, int row) const
{
	if (row < 0 || row >= height) return true;
	return At(col, row).type == CELL_WALL;
}

int Level::RemainingMarks(int owner) const
{
	int n = 0;
	for (size_t i = 0; i < cells.size(); i++)
		if (IsMark(cells[i]) && !cells[i].eaten && (owner == 0 || cells[i].owner == owner))
			n++;
	return n;
}
