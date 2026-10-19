#include "PositionScaling.h"

#include <cmath>

namespace Haichi {

namespace {

const float kMaximumTrackerCoordinate = 50000.0f;

} // namespace

BPoint ScalePosition(BPoint position, int32 savedWidth, int32 savedHeight,
	int32 currentWidth, int32 currentHeight) {
	if (savedWidth <= 0 || savedHeight <= 0 || currentWidth <= 0
		|| currentHeight <= 0) {
		return position;
	}

	position.x *= (float)currentWidth / (float)savedWidth;
	position.y *= (float)currentHeight / (float)savedHeight;
	return position;
}
//---------------------------------------------------------------------------------------------------------------------------------//

bool IsSanePosition(BPoint position) {
	return std::isfinite(position.x) && std::isfinite(position.y)
		&& position.x >= -kMaximumTrackerCoordinate
		&& position.x <= kMaximumTrackerCoordinate
		&& position.y >= -kMaximumTrackerCoordinate
		&& position.y <= kMaximumTrackerCoordinate;
}
//---------------------------------------------------------------------------------------------------------------------------------//

} // namespace Haichi
