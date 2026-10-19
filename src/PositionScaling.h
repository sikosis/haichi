#ifndef HAICHI_POSITION_SCALING_H
#define HAICHI_POSITION_SCALING_H

#include <Point.h>

namespace Haichi {

BPoint ScalePosition(BPoint position, int32 savedWidth, int32 savedHeight,
	int32 currentWidth, int32 currentHeight);
bool IsSanePosition(BPoint position);

} // namespace Haichi

#endif // HAICHI_POSITION_SCALING_H
