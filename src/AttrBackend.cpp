#include "AttrBackend.h"

#include <AppDefs.h>
#include <ByteOrder.h>
#include <Directory.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <Message.h>
#include <Messenger.h>
#include <Node.h>
#include <NodeMonitor.h>
#include <Path.h>

#include <algorithm>
namespace Haichi {

namespace {

#if B_HOST_IS_LENDIAN
const char* const kPoseInfoAttribute = "_trk/pinfo_le";
const char* const kForeignPoseInfoAttribute = "_trk/pinfo";
const char* const kExtendedPoseInfoAttribute = "_trk/xtpinfo_le";
const char* const kForeignExtendedPoseInfoAttribute = "_trk/xtpinfo";
#else
const char* const kPoseInfoAttribute = "_trk/pinfo";
const char* const kForeignPoseInfoAttribute = "_trk/pinfo_le";
const char* const kExtendedPoseInfoAttribute = "_trk/xtpinfo";
const char* const kForeignExtendedPoseInfoAttribute = "_trk/xtpinfo_le";
#endif

const char* const kTrackerSignature = "application/x-vnd.Be-TRAK";

struct PoseInfo {
	bool invisible;
	ino_t initialisedDirectory;
	BPoint location;
};

bool CompareIcons(const IconItem& left, const IconItem& right) {
	return left.name.ICompare(right.name) < 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//

status_t GetDesktopRef(entry_ref& desktopRef) {
	BPath desktopPath;
	status_t result = find_directory(B_DESKTOP_DIRECTORY, &desktopPath);
	if (result != B_OK)
		return result;

	BEntry desktopEntry(desktopPath.Path());
	result = desktopEntry.InitCheck();
	if (result != B_OK)
		return result;

	return desktopEntry.GetRef(&desktopRef);
}
//---------------------------------------------------------------------------------------------------------------------------------//

status_t RefreshTrackerPose(const entry_ref& ref) {
	BMessenger tracker(kTrackerSignature);
	if (!tracker.IsValid())
		return B_OK;

	entry_ref desktopRef;
	status_t result = GetDesktopRef(desktopRef);
	if (result != B_OK)
		return result;

	BMessage getWindow(B_GET_PROPERTY);
	result = getWindow.AddRef("data", &desktopRef);
	if (result == B_OK)
		result = getWindow.AddSpecifier("Folder");
	if (result != B_OK)
		return result;

	BMessage reply;
	result = tracker.SendMessage(&getWindow, &reply);
	if (result != B_OK)
		return result;
	if (reply.FindInt32("error", &result) == B_OK && result != B_OK)
		return result;

	BMessenger window;
	result = reply.FindMessenger("window", &window);
	if (result != B_OK)
		return result;

	BEntry entry(&ref);
	node_ref nodeRef;
	result = entry.GetNodeRef(&nodeRef);
	if (result != B_OK)
		return result;

	// Tracker does not reload a pose location for B_ATTR_CHANGED. A synthetic
	// remove/create pair makes the Desktop pose view re-read the saved pose
	// without renaming the file or restarting Tracker.
	BMessage removed(B_NODE_MONITOR);
	removed.AddInt32("opcode", B_ENTRY_REMOVED);
	removed.AddInt32("device", nodeRef.device);
	removed.AddInt64("node", nodeRef.node);
	removed.AddInt64("directory", ref.directory);
	removed.AddSpecifier("Poses");
	result = window.SendMessage(&removed);
	if (result != B_OK)
		return result;

	BMessage created(B_NODE_MONITOR);
	created.AddInt32("opcode", B_ENTRY_CREATED);
	created.AddInt32("device", nodeRef.device);
	created.AddInt64("node", nodeRef.node);
	created.AddInt64("directory", ref.directory);
	created.AddString("name", ref.name);
	created.AddSpecifier("Poses");
	return window.SendMessage(&created);
}
//---------------------------------------------------------------------------------------------------------------------------------//

} // namespace

IconPositionBackend::~IconPositionBackend() {
}
//---------------------------------------------------------------------------------------------------------------------------------//

status_t AttrBackend::ListIcons(std::vector<IconItem>& icons) const {
	icons.clear();

	BPath desktopPath;
	status_t result = find_directory(B_DESKTOP_DIRECTORY, &desktopPath);
	if (result != B_OK)
		return result;

	BDirectory desktop(desktopPath.Path());
	result = desktop.InitCheck();
	if (result != B_OK)
		return result;

	entry_ref ref;
	while ((result = desktop.GetNextRef(&ref)) == B_OK) {
		IconItem icon;
		icon.name = ref.name;
		icon.ref = ref;
		icon.location = BPoint(0, 0);
		icon.hasLocation = GetLocation(ref, &icon.location) == B_OK;
		icons.push_back(icon);
	}

	if (result != B_ENTRY_NOT_FOUND)
		return result;

	std::sort(icons.begin(), icons.end(), CompareIcons);
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//

status_t AttrBackend::GetLocation(const entry_ref& ref, BPoint* location) const {
	if (location == NULL)
		return B_BAD_VALUE;

	BNode node(&ref);
	status_t result = node.InitCheck();
	if (result != B_OK)
		return result;

	PoseInfo poseInfo;
	ssize_t bytesRead = node.ReadAttr(kPoseInfoAttribute, B_RAW_TYPE, 0,
		&poseInfo, sizeof(poseInfo));
	if (bytesRead < 0)
		return (status_t)bytesRead;
	if ((size_t)bytesRead != sizeof(poseInfo))
		return B_BAD_DATA;
	if (poseInfo.initialisedDirectory != ref.directory)
		return B_ENTRY_NOT_FOUND;

	*location = poseInfo.location;
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//

status_t AttrBackend::SetLocation(const entry_ref& ref, BPoint location) {
	BNode node(&ref);
	status_t result = node.InitCheck();
	if (result != B_OK)
		return result;

	PoseInfo poseInfo = {};
	poseInfo.invisible = false;
	poseInfo.initialisedDirectory = ref.directory;
	poseInfo.location = location;

	ssize_t bytesWritten = node.WriteAttr(kPoseInfoAttribute, B_RAW_TYPE, 0,
		&poseInfo, sizeof(poseInfo));
	if (bytesWritten < 0)
		return (status_t)bytesWritten;
	if ((size_t)bytesWritten != sizeof(poseInfo))
		return B_IO_ERROR;

	node.RemoveAttr(kForeignPoseInfoAttribute);
	// The extended pose cache contains locations keyed by screen frame. Remove
	// it so a stale cached location cannot undo this move after a resolution
	// change. Tracker recreates it after the next interactive placement.
	node.RemoveAttr(kExtendedPoseInfoAttribute);
	node.RemoveAttr(kForeignExtendedPoseInfoAttribute);
	result = node.Sync();
	if (result != B_OK)
		return result;

	BPoint verifiedLocation;
	result = GetLocation(ref, &verifiedLocation);
	if (result != B_OK)
		return result;
	if (verifiedLocation != location)
		return B_IO_ERROR;

	return RefreshTrackerPose(ref);
}
//---------------------------------------------------------------------------------------------------------------------------------//

} // namespace Haichi
