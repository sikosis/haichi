#include "TrayView.h"

#include "Constants.h"

#include <Application.h>
#include <Archivable.h>
#include <MenuItem.h>
#include <Messenger.h>
#include <PopUpMenu.h>
#include <Roster.h>

namespace Haichi {

TrayView::TrayView(BRect frame)
	:
	BView(frame, kDeskbarItemName, B_FOLLOW_NONE, B_WILL_DRAW) {
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetToolTip("haichi Desktop icon placer");
}
//---------------------------------------------------------------------------------------------------------------------------------//

TrayView::TrayView(BMessage* archive)
	:
	BView(archive) {
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetToolTip("haichi Desktop icon placer");
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
	BRect bounds = Bounds();
	SetHighColor(ViewColor());
	FillRect(updateRect);

	SetHighColor(ui_color(B_CONTROL_TEXT_COLOR));
	float left = bounds.left + 3.0f;
	float right = bounds.right - 3.0f;
	float top = bounds.top + 3.0f;
	float bottom = bounds.bottom - 3.0f;
	StrokeRect(BRect(left, top, right, bottom));
	StrokeLine(BPoint(left, top + (bottom - top) / 2.0f),
		BPoint(right, top + (bottom - top) / 2.0f));
	StrokeLine(BPoint(left + (right - left) / 2.0f, top),
		BPoint(left + (right - left) / 2.0f, bottom));
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
