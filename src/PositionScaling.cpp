#include "PositionScaling.h"

#include <algorithm>
#include <cmath>

namespace Haichi {

namespace {

const float kMaximumTrackerCoordinate = 50000.0f;

BPoint MinimumPosition(const std::vector<BPoint>& positions) {
	if (positions.empty())
		return BPoint(0, 0);

	BPoint minimum = positions[0];
	for (size_t index = 1; index < positions.size(); index++) {
		minimum.x = std::min(minimum.x, positions[index].x);
		minimum.y = std::min(minimum.y, positions[index].y);
	}
	return minimum;
}
//---------------------------------------------------------------------------------------------------------------------------------//

} // namespace

BPoint DisplayPosition(BPoint trackerPosition) {
	return trackerPosition;
}
//---------------------------------------------------------------------------------------------------------------------------------//

std::vector<BPoint> AlignPositions(const std::vector<BPoint>& positions,
	bool horizontal, float spacing) {
	std::vector<BPoint> aligned(positions);
	if (positions.empty())
		return aligned;

	std::vector<size_t> order;
	for (size_t index = 0; index < positions.size(); index++)
		order.push_back(index);
	std::sort(order.begin(), order.end(), [&positions, horizontal](size_t left,
		size_t right) {
		if (horizontal && positions[left].x != positions[right].x)
			return positions[left].x < positions[right].x;
		if (!horizontal && positions[left].y != positions[right].y)
			return positions[left].y < positions[right].y;
		if (horizontal)
			return positions[left].y < positions[right].y;
		return positions[left].x < positions[right].x;
	});

	BPoint minimum = MinimumPosition(positions);
	for (size_t rank = 0; rank < order.size(); rank++) {
		if (horizontal)
			aligned[order[rank]] = BPoint(minimum.x + rank * spacing, minimum.y);
		else
			aligned[order[rank]] = BPoint(minimum.x, minimum.y + rank * spacing);
	}
	return aligned;
}
//---------------------------------------------------------------------------------------------------------------------------------//

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
