#ifndef HAICHI_LAYOUT_STORE_H
#define HAICHI_LAYOUT_STORE_H

#include <Message.h>
#include <String.h>
#include <SupportDefs.h>

#include <vector>

#include "IconPositionBackend.h"

namespace Haichi {

struct LayoutEntry {
	BString fileName;
	BPoint location;
};

struct Layout {
	BString name;
	int32 screenWidth;
	int32 screenHeight;
	std::vector<LayoutEntry> entries;
};

class LayoutStore {
public:
	status_t Save(const Layout& layout) const;
	status_t Load(const char* name, Layout& layout) const;

private:
	status_t GetLayoutPath(const char* name, BString& path,
		bool createDirectory) const;
	bool IsValidName(const char* name) const;
};

} // namespace Haichi

#endif // HAICHI_LAYOUT_STORE_H
