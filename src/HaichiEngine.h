#ifndef HAICHI_ENGINE_H
#define HAICHI_ENGINE_H

#include <String.h>

#include <memory>
#include <vector>

#include "IconPositionBackend.h"
#include "LayoutStore.h"

namespace Haichi {

struct RestoreResult {
	int32 moved;
	std::vector<BString> missing;
};

class HaichiEngine {
public:
	HaichiEngine(std::unique_ptr<IconPositionBackend> backend,
		std::unique_ptr<LayoutStore> layoutStore);

	status_t ListIcons(std::vector<IconItem>& icons) const;
	status_t MoveIcon(const char* name, BPoint location);
	status_t SaveLayout(const char* name, int32 screenWidth, int32 screenHeight);
	status_t RestoreLayout(const char* name, int32 screenWidth,
		int32 screenHeight, RestoreResult& restoreResult);

private:
	const IconItem* FindIcon(const std::vector<IconItem>& icons,
		const char* name) const;

	std::unique_ptr<IconPositionBackend> fBackend;
	std::unique_ptr<LayoutStore> fLayoutStore;
};

} // namespace Haichi

#endif // HAICHI_ENGINE_H
