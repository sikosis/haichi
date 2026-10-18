#include <Application.h>
#include <Screen.h>
#include <String.h>

#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

#include "AttrBackend.h"
#include "Constants.h"
#include "HaichiEngine.h"
#include "LayoutStore.h"
#include "PositionScaling.h"

namespace Haichi {

const char* const kApplicationSignature = "application/x-vnd.Sikosis-haichi";
const char* const kApplicationVersion = "0.01";

namespace {

enum ExitCode {
	kSuccess = 0,
	kBadArguments = 1,
	kIconNotFound = 2,
	kOperationFailed = 3
};

void PrintUsage(FILE* stream) {
	std::fprintf(stream,
		"haichi v%s\n"
		"Usage:\n"
		"  haichi list\n"
		"  haichi move <icon name> <x> <y>\n"
		"  haichi save <layout name>\n"
		"  haichi restore <layout name>\n"
		"  haichi help\n",
		kApplicationVersion);
}
//---------------------------------------------------------------------------------------------------------------------------------//

bool ParseCoordinate(const char* text, float& coordinate) {
	if (text == NULL || text[0] == '\0')
		return false;

	errno = 0;
	char* end = NULL;
	float value = std::strtof(text, &end);
	if (errno != 0 || end == text || *end != '\0' || !std::isfinite(value))
		return false;

	coordinate = value;
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//

status_t ScreenSize(int32& width, int32& height) {
	BScreen screen(B_MAIN_SCREEN_ID);
	if (!screen.IsValid())
		return B_ERROR;

	BRect frame = screen.Frame();
	width = frame.IntegerWidth() + 1;
	height = frame.IntegerHeight() + 1;
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//

int ReportError(const char* action, status_t result,
	int notFoundExit = kOperationFailed) {
	std::fprintf(stderr, "haichi: %s: %s\n", action, std::strerror(result));
	return result == B_ENTRY_NOT_FOUND ? notFoundExit : kOperationFailed;
}
//---------------------------------------------------------------------------------------------------------------------------------//

int RunList(HaichiEngine& engine) {
	std::vector<IconItem> icons;
	status_t result = engine.ListIcons(icons);
	if (result != B_OK)
		return ReportError("could not list Desktop icons", result);

	for (size_t index = 0; index < icons.size(); index++) {
		if (icons[index].hasLocation) {
			std::printf("%g\t%g\t%s\n", icons[index].location.x,
				icons[index].location.y, icons[index].name.String());
		} else {
			std::printf("-\t-\t%s\n", icons[index].name.String());
		}
	}

	return kSuccess;
}
//---------------------------------------------------------------------------------------------------------------------------------//

int RunMove(HaichiEngine& engine, const char* name, const char* xText,
	const char* yText) {
	float x;
	float y;
	if (!ParseCoordinate(xText, x) || !ParseCoordinate(yText, y)) {
		std::fprintf(stderr, "haichi: x and y must be numbers\n");
		return kBadArguments;
	}

	int32 screenWidth;
	int32 screenHeight;
	status_t result = ScreenSize(screenWidth, screenHeight);
	if (result != B_OK)
		return ReportError("could not read the screen size", result);
	BPoint requestedLocation(x, y);
	BPoint location = ClampPosition(requestedLocation, screenWidth, screenHeight);
	if (location != requestedLocation) {
		std::fprintf(stderr,
			"haichi: clamped position to %g, %g for the %" B_PRId32
			" x %" B_PRId32 " Desktop\n", location.x, location.y,
			screenWidth, screenHeight);
	}

	result = engine.MoveIcon(name, location);
	if (result != B_OK)
		return ReportError("could not move icon", result, kIconNotFound);

	std::printf("Moved %s to %g, %g\n", name, location.x, location.y);
	return kSuccess;
}
//---------------------------------------------------------------------------------------------------------------------------------//

int RunSave(HaichiEngine& engine, const char* name) {
	int32 screenWidth;
	int32 screenHeight;
	status_t result = ScreenSize(screenWidth, screenHeight);
	if (result != B_OK)
		return ReportError("could not read the screen size", result);

	result = engine.SaveLayout(name, screenWidth, screenHeight);
	if (result != B_OK)
		return ReportError("could not save layout", result);

	std::printf("Saved layout %s\n", name);
	return kSuccess;
}
//---------------------------------------------------------------------------------------------------------------------------------//

int RunRestore(HaichiEngine& engine, const char* name) {
	int32 screenWidth;
	int32 screenHeight;
	status_t result = ScreenSize(screenWidth, screenHeight);
	if (result != B_OK)
		return ReportError("could not read the screen size", result);

	RestoreResult restoreResult;
	result = engine.RestoreLayout(name, screenWidth, screenHeight, restoreResult);
	if (result != B_OK)
		return ReportError("could not restore layout", result);

	std::printf("Restored %" B_PRId32 " icons from %s\n",
		restoreResult.moved, name);
	for (size_t index = 0; index < restoreResult.missing.size(); index++) {
		std::fprintf(stderr, "haichi: skipped missing icon: %s\n",
			restoreResult.missing[index].String());
	}
	return kSuccess;
}
//---------------------------------------------------------------------------------------------------------------------------------//

} // namespace

} // namespace Haichi

int main(int argumentCount, char** arguments) {
	using namespace Haichi;

	if (argumentCount < 2) {
		PrintUsage(stderr);
		return kBadArguments;
	}

	BApplication application(kApplicationSignature);
	if (application.InitCheck() != B_OK)
		return ReportError("could not connect to the application server",
			application.InitCheck());

	std::unique_ptr<IconPositionBackend> backend(new AttrBackend());
	std::unique_ptr<LayoutStore> layoutStore(new LayoutStore());
	HaichiEngine engine(std::move(backend), std::move(layoutStore));

	BString command(arguments[1]);
	if (command == "list" && argumentCount == 2)
		return RunList(engine);
	if (command == "move" && argumentCount == 5)
		return RunMove(engine, arguments[2], arguments[3], arguments[4]);
	if (command == "save" && argumentCount == 3)
		return RunSave(engine, arguments[2]);
	if (command == "restore" && argumentCount == 3)
		return RunRestore(engine, arguments[2]);
	if ((command == "help" || command == "--help" || command == "-h")
		&& argumentCount == 2) {
		PrintUsage(stdout);
		return kSuccess;
	}

	PrintUsage(stderr);
	return kBadArguments;
}
//---------------------------------------------------------------------------------------------------------------------------------//
