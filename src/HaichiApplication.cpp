#include "HaichiApplication.h"

#include "AboutWindow.h"
#include "AttrBackend.h"
#include "Constants.h"
#include "LayoutStore.h"
#include "MainWindow.h"

#include <Deskbar.h>
#include <Roster.h>

namespace Haichi {

HaichiApplication::HaichiApplication()
	:
	BApplication(kApplicationSignature),
	fEngine(new HaichiEngine(std::unique_ptr<IconPositionBackend>(
		new AttrBackend()), std::unique_ptr<LayoutStore>(new LayoutStore()))),
	fWindow(NULL) {
}
//---------------------------------------------------------------------------------------------------------------------------------//

HaichiApplication::~HaichiApplication() {
}
//---------------------------------------------------------------------------------------------------------------------------------//

void HaichiApplication::AboutRequested() {
	(new AboutWindow())->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//

void HaichiApplication::MessageReceived(BMessage* message) {
	switch (message->what) {
		case kOpenWindow:
			OpenWindow();
			break;

		default:
			BApplication::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//

bool HaichiApplication::QuitRequested() {
	BDeskbar deskbar;
	if (deskbar.HasItem(kDeskbarItemName))
		deskbar.RemoveItem(kDeskbarItemName);
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//

void HaichiApplication::ReadyToRun() {
	InstallDeskbarItem();
	OpenWindow();
}
//---------------------------------------------------------------------------------------------------------------------------------//

void HaichiApplication::InstallDeskbarItem() {
	BDeskbar deskbar;
	if (deskbar.HasItem(kDeskbarItemName))
		return;

	app_info info;
	if (GetAppInfo(&info) != B_OK)
		return;

	int32 id;
	deskbar.AddItem(&info.ref, &id);
}
//---------------------------------------------------------------------------------------------------------------------------------//

void HaichiApplication::OpenWindow() {
	if (fWindow == NULL)
		fWindow = new MainWindow(fEngine.get());

	if (fWindow->IsHidden())
		fWindow->Show();
	fWindow->Activate(true);
	fWindow->PostMessage(kRefreshIcons);
}
//---------------------------------------------------------------------------------------------------------------------------------//

} // namespace Haichi
