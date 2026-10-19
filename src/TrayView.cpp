#include "TrayView.h"

#include "Constants.h"

#include <Application.h>
#include <Archivable.h>
#include <Bitmap.h>
#include <IconUtils.h>
#include <MenuItem.h>
#include <Messenger.h>
#include <Mime.h>
#include <PopUpMenu.h>
#include <Resources.h>
#include <Roster.h>
#include <Window.h>

#include <algorithm>

static const int32 kResourceAnchor = 0;

namespace Haichi {

TrayView::TrayView(BRect frame)
	:
	BView(frame, kDeskbarItemName, B_FOLLOW_NONE, B_WILL_DRAW),
	fIcon(NULL) {
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetToolTip("haichi Desktop icon placer");
	LoadIcon();
}
//---------------------------------------------------------------------------------------------------------------------------------//

TrayView::TrayView(BMessage* archive)
	:
	BView(archive),
	fIcon(NULL) {
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetToolTip("haichi Desktop icon placer");
	LoadIcon();
}
//---------------------------------------------------------------------------------------------------------------------------------//

TrayView::~TrayView() {
	delete fIcon;
}
//---------------------------------------------------------------------------------------------------------------------------------//

BArchivable* TrayView::Instantiate(BMessage* archive) {
	if (!validate_instantiation(archive, "Haichi::TrayView"))
		return NULL;
	return new TrayView(archive);
}
//---------------------------------------------------------------------------------------------------------------------------------//

status_t TrayView::Archive(BMessage* archive, bool deep) const {
	status_t result = BView::Archive(archive, deep);
	if (result == B_OK)
		result = archive->AddString("add_on", kApplicationSignature);
	if (result == B_OK)
		result = archive->AddString("class", "Haichi::TrayView");
	return result;
}
//---------------------------------------------------------------------------------------------------------------------------------//

void TrayView::AttachedToWindow() {
	BView::AttachedToWindow();
	SetLowColor(ViewColor());
}
//---------------------------------------------------------------------------------------------------------------------------------//

void TrayView::Draw(BRect updateRect) {
	SetHighColor(ViewColor());
	FillRect(updateRect);

	if (fIcon == NULL)
		return;

	float size = std::min(Bounds().Width(), Bounds().Height());
	BRect iconFrame(0, 0, size, size);
	iconFrame.OffsetTo(Bounds().left + (Bounds().Width() - size) / 2.0f,
		Bounds().top + (Bounds().Height() - size) / 2.0f);
	SetDrawingMode(B_OP_ALPHA);
	SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
	DrawBitmap(fIcon, fIcon->Bounds(), iconFrame);
	SetDrawingMode(B_OP_COPY);
}
//---------------------------------------------------------------------------------------------------------------------------------//

void TrayView::LoadIcon() {
	BResources resources;
	if (resources.SetToImage(&kResourceAnchor) != B_OK)
		return;

	size_t dataSize = 0;
	const uint8* data = static_cast<const uint8*>(
		resources.LoadResource(B_VECTOR_ICON_TYPE, 101, &dataSize));
	if (data == NULL || dataSize == 0)
		return;

	float viewSize = std::min(Bounds().Width(), Bounds().Height());
	int32 iconSize = std::max((int32)viewSize + 1, (int32)16);
	BBitmap* icon = new BBitmap(BRect(0, 0, iconSize - 1, iconSize - 1),
		B_RGBA32);
	if (icon->InitCheck() == B_OK
		&& BIconUtils::GetVectorIcon(data, dataSize, icon) == B_OK) {
		fIcon = icon;
		return;
	}
	delete icon;
}
//---------------------------------------------------------------------------------------------------------------------------------//

void TrayView::MouseDown(BPoint where) {
	int32 buttons = B_PRIMARY_MOUSE_BUTTON;
	if (Window() != NULL && Window()->CurrentMessage() != NULL)
		Window()->CurrentMessage()->FindInt32("buttons", &buttons);

	BMessenger application(kApplicationSignature);
	if ((buttons & B_SECONDARY_MOUSE_BUTTON) != 0) {
		BPopUpMenu menu("haichi", false, false);
		menu.AddItem(new BMenuItem("Open haichi", new BMessage(kOpenWindow)));
		menu.AddSeparatorItem();
		menu.AddItem(new BMenuItem("About", new BMessage(B_ABOUT_REQUESTED)));
		menu.AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED)));
		BMenuItem* selected = menu.Go(ConvertToScreen(where), false, true);
		if (selected != NULL)
			application.SendMessage(selected->Message());
		return;
	}

	if (!application.IsValid()) {
		be_roster->Launch(kApplicationSignature);
		return;
	}
	application.SendMessage(kOpenWindow);
}
//---------------------------------------------------------------------------------------------------------------------------------//

} // namespace Haichi

extern "C" _EXPORT BView* instantiate_deskbar_item(float maxWidth,
	float maxHeight) {
	float size = maxHeight > 0.0f ? maxHeight : 16.0f;
	if (maxWidth > 0.0f && maxWidth < size)
		size = maxWidth;
	return new Haichi::TrayView(BRect(0, 0, size - 1.0f, size - 1.0f));
}
//---------------------------------------------------------------------------------------------------------------------------------//
