#include "DesktopPreview.h"

#include "Constants.h"
#include "PositionScaling.h"

#include <InterfaceDefs.h>
#include <Message.h>
#include <Window.h>

#include <algorithm>

namespace Haichi {

DesktopPreview::DesktopPreview()
	:
	BView("desktop preview", B_WILL_DRAW | B_FRAME_EVENTS),
	fSelectedIndex(-1),
	fScreenWidth(1),
	fScreenHeight(1),
	fPinPosition(0, 0),
	fPinVisible(false),
	fDragging(false) {
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetExplicitMinSize(BSize(420, 280));
}
//---------------------------------------------------------------------------------------------------------------------------------//

void DesktopPreview::Draw(BRect updateRect) {
	BRect monitor = MonitorFrame();
	SetHighColor(ViewColor());
	FillRect(updateRect);

	SetHighColor(ui_color(B_DOCUMENT_BACKGROUND_COLOR));
	FillRect(monitor);
	SetHighColor(ui_color(B_CONTROL_TEXT_COLOR));
	StrokeRect(monitor);

	rgb_color gridColour = tint_color(ui_color(B_CONTROL_TEXT_COLOR),
		B_LIGHTEN_2_TINT);
	SetHighColor(gridColour);
	for (int32 column = 1; column < 10; column++) {
		float x = monitor.left + monitor.Width() * column / 10.0f;
		StrokeLine(BPoint(x, monitor.top), BPoint(x, monitor.bottom));
	}
	for (int32 row = 1; row < 6; row++) {
		float y = monitor.top + monitor.Height() * row / 6.0f;
		StrokeLine(BPoint(monitor.left, y), BPoint(monitor.right, y));
	}

	for (size_t index = 0; index < fIcons.size(); index++) {
		if (!fIcons[index].hasLocation)
			continue;

		BPoint position = DisplayPosition(fIcons[index].location);
		if (position.x < 0 || position.x >= fScreenWidth || position.y < 0
			|| position.y >= fScreenHeight) {
			continue;
		}

		BPoint point(monitor.left + position.x * monitor.Width() / fScreenWidth,
			monitor.top + position.y * monitor.Height() / fScreenHeight);
		if ((int32)index == fSelectedIndex)
			SetHighColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
		else
			SetHighColor(ui_color(B_CONTROL_TEXT_COLOR));
		FillRect(BRect(point.x - 4, point.y - 4, point.x + 4, point.y + 4));
	}

	if (fPinVisible) {
		BPoint point(monitor.left
				+ fPinPosition.x * monitor.Width() / fScreenWidth,
			monitor.top + fPinPosition.y * monitor.Height() / fScreenHeight);
		SetHighColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
		SetPenSize(2.0f);
		StrokeLine(BPoint(point.x - 11, point.y), BPoint(point.x + 11, point.y));
		StrokeLine(BPoint(point.x, point.y - 11), BPoint(point.x, point.y + 11));
		SetPenSize(1.0f);
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//

void DesktopPreview::MouseDown(BPoint where) {
	if (fSelectedIndex < 0)
		return;

	fDragging = true;
	SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
	PickPosition(where);
}
//---------------------------------------------------------------------------------------------------------------------------------//

void DesktopPreview::MouseMoved(BPoint where, uint32 transit,
	const BMessage* dragMessage) {
	if (fDragging)
		PickPosition(where);
}
//---------------------------------------------------------------------------------------------------------------------------------//

void DesktopPreview::MouseUp(BPoint where) {
	if (fDragging)
		PickPosition(where);
	fDragging = false;
}
//---------------------------------------------------------------------------------------------------------------------------------//

void DesktopPreview::SetIcons(const std::vector<IconItem>& icons,
	int32 selectedIndex) {
	fIcons = icons;
	fSelectedIndex = selectedIndex;
	Invalidate();
}
//---------------------------------------------------------------------------------------------------------------------------------//

void DesktopPreview::SetPinPosition(BPoint position, bool visible) {
	fPinPosition = position;
	fPinVisible = visible;
	Invalidate();
}
//---------------------------------------------------------------------------------------------------------------------------------//

void DesktopPreview::SetScreenSize(int32 width, int32 height) {
	fScreenWidth = std::max(width, (int32)1);
	fScreenHeight = std::max(height, (int32)1);
	Invalidate();
}
//---------------------------------------------------------------------------------------------------------------------------------//

BRect DesktopPreview::MonitorFrame() const {
	BRect available = Bounds();
	available.InsetBy(18, 18);
	float aspect = (float)fScreenWidth / (float)fScreenHeight;
	float width = available.Width();
	float height = width / aspect;
	if (height > available.Height()) {
		height = available.Height();
		width = height * aspect;
	}
	float left = available.left + (available.Width() - width) / 2.0f;
	float top = available.top + (available.Height() - height) / 2.0f;
	return BRect(left, top, left + width, top + height);
}
//---------------------------------------------------------------------------------------------------------------------------------//

void DesktopPreview::PickPosition(BPoint where) {
	BRect monitor = MonitorFrame();
	where.x = std::max(monitor.left, std::min(where.x, monitor.right));
	where.y = std::max(monitor.top, std::min(where.y, monitor.bottom));

	BPoint position((where.x - monitor.left) * fScreenWidth / monitor.Width(),
		(where.y - monitor.top) * fScreenHeight / monitor.Height());
	position.x = std::min(position.x, (float)fScreenWidth - 1.0f);
	position.y = std::min(position.y, (float)fScreenHeight - 1.0f);
	SetPinPosition(position, true);

	BMessage message(kPreviewPosition);
	message.AddFloat("x", position.x);
	message.AddFloat("y", position.y);
	Window()->PostMessage(&message);
}
//---------------------------------------------------------------------------------------------------------------------------------//

} // namespace Haichi
