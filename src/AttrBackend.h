#ifndef HAICHI_ATTR_BACKEND_H
#define HAICHI_ATTR_BACKEND_H

#include "IconPositionBackend.h"

namespace Haichi {

class AttrBackend : public IconPositionBackend {
public:
	virtual status_t ListIcons(std::vector<IconItem>& icons) const;
	virtual status_t GetLocation(const entry_ref& ref, BPoint* location) const;
	virtual status_t SetLocation(const entry_ref& ref, BPoint location);
};

} // namespace Haichi

#endif // HAICHI_ATTR_BACKEND_H
