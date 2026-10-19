#include <Point.h>

#include <cassert>
#include <cstdio>
#include <limits>

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

	std::puts("haichi layout tests passed");
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
