#include <Point.h>

#include <cassert>
#include <cstdio>
#include <limits>
#include <vector>

#include "PositionScaling.h"

using namespace Haichi;

int main() {
	BPoint displayed = DisplayPosition(BPoint(1050, 20));
	assert(displayed == BPoint(1050, 20));

	BPoint unchanged = ScalePosition(BPoint(120, 340), 1920, 1080, 1920, 1080);
	assert(unchanged == BPoint(120, 340));

	BPoint doubled = ScalePosition(BPoint(120, 340), 960, 540, 1920, 1080);
	assert(doubled == BPoint(240, 680));

	BPoint noSavedFrame = ScalePosition(BPoint(120, 340), 0, 0, 1920, 1080);
	assert(noSavedFrame == BPoint(120, 340));

	assert(IsSanePosition(BPoint(-20, -30)));
	assert(IsSanePosition(BPoint(-50000, 50000)));
	assert(!IsSanePosition(BPoint(-50001, 0)));
	assert(!IsSanePosition(BPoint(0, 50001)));
	assert(!IsSanePosition(BPoint(std::numeric_limits<float>::quiet_NaN(), 0)));

	std::vector<BPoint> positions;
	positions.push_back(BPoint(100, 300));
	positions.push_back(BPoint(20, 200));
	positions.push_back(BPoint(60, 100));
	std::vector<BPoint> row = AlignPositions(positions, true, 30);
	assert(row[0] == BPoint(80, 100));
	assert(row[1] == BPoint(20, 100));
	assert(row[2] == BPoint(50, 100));
	std::vector<BPoint> column = AlignPositions(positions, false, 30);
	assert(column[0] == BPoint(20, 160));
	assert(column[1] == BPoint(20, 130));
	assert(column[2] == BPoint(20, 100));
	assert(AlignPositions(std::vector<BPoint>(), true, 30).empty());

	std::puts("haichi layout tests passed");
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
