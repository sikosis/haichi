#ifndef HAICHI_ICON_POSITION_BACKEND_H
#define HAICHI_ICON_POSITION_BACKEND_H

#include <Entry.h>
#include <Point.h>
#include <String.h>
#include <SupportDefs.h>

#include <vector>

namespace Haichi {

struct IconItem {
	BString name;
	entry_ref ref;
	BPoint location;
	bool hasLocation;
};

class IconPositionBackend {
public:
	virtual ~IconPositionBackend();

	virtual status_t ListIcons(std::vector<IconItem>& icons) const = 0;
	virtual status_t GetLocation(const entry_ref& ref, BPoint* location) const = 0;
	virtual status_t SetLocation(const entry_ref& ref, BPoint location) = 0;
};

} // namespace Haichi

#endif // HAICHI_ICON_POSITION_BACKEND_H
