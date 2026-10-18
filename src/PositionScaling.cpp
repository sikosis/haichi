#include "PositionScaling.h"

#include <algorithm>

namespace Haichi {

namespace {

const int32 kDefaultIconSize = 32;

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

BPoint ClampPosition(BPoint position, int32 screenWidth, int32 screenHeight) {
	if (screenWidth <= 0 || screenHeight <= 0)
		return position;

	float maximumX = (float)std::max(0, screenWidth - kDefaultIconSize);
	float maximumY = (float)std::max(0, screenHeight - kDefaultIconSize);
	position.x = std::max(0.0f, std::min(position.x, maximumX));
	position.y = std::max(0.0f, std::min(position.y, maximumY));
	return position;
}
//---------------------------------------------------------------------------------------------------------------------------------//

} // namespace Haichi
