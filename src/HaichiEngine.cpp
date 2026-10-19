#include "HaichiEngine.h"

#include "PositionScaling.h"

#include <utility>

namespace Haichi {

HaichiEngine::HaichiEngine(std::unique_ptr<IconPositionBackend> backend,
	std::unique_ptr<LayoutStore> layoutStore)
	:
	fBackend(std::move(backend)),
	fLayoutStore(std::move(layoutStore)) {
}
//---------------------------------------------------------------------------------------------------------------------------------//

status_t HaichiEngine::ListIcons(std::vector<IconItem>& icons) const {
	return fBackend->ListIcons(icons);
}
//---------------------------------------------------------------------------------------------------------------------------------//

status_t HaichiEngine::MoveIcon(const char* name, BPoint location) {
	std::vector<IconItem> icons;
	status_t result = fBackend->ListIcons(icons);
	if (result != B_OK)
		return result;

	const IconItem* icon = FindIcon(icons, name);
	if (icon == NULL)
		return B_ENTRY_NOT_FOUND;

	return fBackend->SetLocation(icon->ref, location);
}
//---------------------------------------------------------------------------------------------------------------------------------//

status_t HaichiEngine::SaveLayout(const char* name, int32 screenWidth,
	int32 screenHeight) {
	std::vector<IconItem> icons;
	status_t result = fBackend->ListIcons(icons);
	if (result != B_OK)
		return result;

	Layout layout;
	layout.name = name;
	layout.screenWidth = screenWidth;
	layout.screenHeight = screenHeight;
	for (size_t index = 0; index < icons.size(); index++) {
		if (!icons[index].hasLocation)
			continue;
		LayoutEntry entry;
		entry.fileName = icons[index].name;
		entry.location = icons[index].location;
		layout.entries.push_back(entry);
	}

	return fLayoutStore->Save(layout);
}
//---------------------------------------------------------------------------------------------------------------------------------//

status_t HaichiEngine::RestoreLayout(const char* name, int32 screenWidth,
	int32 screenHeight, RestoreResult& restoreResult) {
	restoreResult.moved = 0;
	restoreResult.missing.clear();

	Layout layout;
	status_t result = fLayoutStore->Load(name, layout);
	if (result != B_OK)
		return result;

	std::vector<IconItem> icons;
	result = fBackend->ListIcons(icons);
	if (result != B_OK)
		return result;

	for (size_t index = 0; index < layout.entries.size(); index++) {
		const LayoutEntry& entry = layout.entries[index];
		const IconItem* icon = FindIcon(icons, entry.fileName.String());
		if (icon == NULL) {
			restoreResult.missing.push_back(entry.fileName);
			continue;
		}

		BPoint location = ScalePosition(entry.location, layout.screenWidth,
			layout.screenHeight, screenWidth, screenHeight);
		result = fBackend->SetLocation(icon->ref, location);
		if (result != B_OK)
			return result;
		restoreResult.moved++;
	}

	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//

const IconItem* HaichiEngine::FindIcon(const std::vector<IconItem>& icons,
	const char* name) const {
	if (name == NULL)
		return NULL;

	for (size_t index = 0; index < icons.size(); index++) {
		if (icons[index].name == name)
			return &icons[index];
	}

	return NULL;
}
//---------------------------------------------------------------------------------------------------------------------------------//

} // namespace Haichi
