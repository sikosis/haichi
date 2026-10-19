#include "HaichiApplication.h"

#include "AttrBackend.h"
#include "Constants.h"
#include "LayoutStore.h"
#include "MainWindow.h"

#include <Alert.h>
#include <Bitmap.h>
#include <Deskbar.h>
#include <IconUtils.h>
#include <Mime.h>
#include <Resources.h>
#include <Roster.h>
#include <String.h>

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
	BString text("haichi v");
	text << kApplicationVersion
		<< "\n\nPinpoint placement for Haiku Desktop icons.";
	BAlert* alert = new BAlert("About haichi", text.String(), "OK");
	BResources* resources = AppResources();
	if (resources != NULL) {
		size_t dataSize = 0;
		const uint8* data = static_cast<const uint8*>(
			resources->LoadResource(B_VECTOR_ICON_TYPE, 101, &dataSize));
		if (data != NULL && dataSize > 0) {
			BBitmap* icon = new BBitmap(BRect(0, 0, 63, 63), B_RGBA32);
			if (icon->InitCheck() == B_OK
				&& BIconUtils::GetVectorIcon(data, dataSize, icon) == B_OK) {
				alert->SetIcon(icon);
			} else {
				delete icon;
			}
		}
	}
	alert->Go(NULL);
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
