#include "LayoutStore.h"

#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>

#include <cctype>

namespace Haichi {

namespace {

const uint32 kLayoutMessage = 'LAYT';

} // namespace

status_t LayoutStore::Save(const Layout& layout) const {
	BString path;
	status_t result = GetLayoutPath(layout.name.String(), path, true);
	if (result != B_OK)
		return result;

	BMessage archive(kLayoutMessage);
	result = archive.AddString("name", layout.name);
	if (result == B_OK)
		result = archive.AddInt32("screen_w", layout.screenWidth);
	if (result == B_OK)
		result = archive.AddInt32("screen_h", layout.screenHeight);

	for (size_t index = 0; result == B_OK && index < layout.entries.size();
		index++) {
		BMessage entry;
		result = entry.AddString("file", layout.entries[index].fileName);
		if (result == B_OK)
			result = entry.AddPoint("loc", layout.entries[index].location);
		if (result == B_OK)
			result = archive.AddMessage("icon", &entry);
	}
	if (result != B_OK)
		return result;

	BFile file(path.String(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	result = file.InitCheck();
	if (result != B_OK)
		return result;

	return archive.Flatten(&file);
}
//---------------------------------------------------------------------------------------------------------------------------------//

status_t LayoutStore::Load(const char* name, Layout& layout) const {
	BString path;
	status_t result = GetLayoutPath(name, path, false);
	if (result != B_OK)
		return result;

	BFile file(path.String(), B_READ_ONLY);
	result = file.InitCheck();
	if (result != B_OK)
		return result;

	BMessage archive;
	result = archive.Unflatten(&file);
	if (result != B_OK)
		return result;
	if (archive.what != kLayoutMessage)
		return B_BAD_DATA;

	layout.entries.clear();
	result = archive.FindString("name", &layout.name);
	if (result != B_OK)
		return result;
	result = archive.FindInt32("screen_w", &layout.screenWidth);
	if (result != B_OK)
		return result;
	result = archive.FindInt32("screen_h", &layout.screenHeight);
	if (result != B_OK)
		return result;

	BMessage entryArchive;
	for (int32 index = 0;
		archive.FindMessage("icon", index, &entryArchive) == B_OK; index++) {
		LayoutEntry entry;
		result = entryArchive.FindString("file", &entry.fileName);
		if (result == B_OK)
			result = entryArchive.FindPoint("loc", &entry.location);
		if (result != B_OK)
			return B_BAD_DATA;
		layout.entries.push_back(entry);
	}

	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//

status_t LayoutStore::GetLayoutPath(const char* name, BString& path,
	bool createDirectory) const {
	if (!IsValidName(name))
		return B_BAD_VALUE;

	BPath settingsPath;
	status_t result = find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath,
		createDirectory);
	if (result != B_OK)
		return result;

	result = settingsPath.Append("haichi");
	if (result != B_OK)
		return result;
	if (createDirectory) {
		result = create_directory(settingsPath.Path(), 0755);
		if (result != B_OK && result != B_FILE_EXISTS)
			return result;
	}

	BString fileName(name);
	fileName << ".layout";
	result = settingsPath.Append(fileName.String());
	if (result != B_OK)
		return result;

	path = settingsPath.Path();
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//

bool LayoutStore::IsValidName(const char* name) const {
	if (name == NULL || name[0] == '\0')
		return false;

	for (const unsigned char* character = (const unsigned char*)name;
		*character != '\0'; character++) {
		if (!std::isalnum(*character) && *character != ' '
			&& *character != '-' && *character != '_') {
			return false;
		}
	}

	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//

} // namespace Haichi
