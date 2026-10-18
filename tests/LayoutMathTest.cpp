#include <Point.h>

#include <cassert>
#include <cstdio>

#include "PositionScaling.h"

using namespace Haichi;

int main() {
	BPoint unchanged = ScalePosition(BPoint(120, 340), 1920, 1080, 1920, 1080);
	assert(unchanged == BPoint(120, 340));

	BPoint doubled = ScalePosition(BPoint(120, 340), 960, 540, 1920, 1080);
	assert(doubled == BPoint(240, 680));

	BPoint noSavedFrame = ScalePosition(BPoint(120, 340), 0, 0, 1920, 1080);
	assert(noSavedFrame == BPoint(120, 340));

	BPoint clampedLow = ClampPosition(BPoint(-20, -30), 1920, 1080);
	assert(clampedLow == BPoint(0, 0));

	BPoint clampedHigh = ClampPosition(BPoint(2200, 1500), 1920, 1080);
	assert(clampedHigh == BPoint(1888, 1048));

	std::puts("haichi layout tests passed");
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
