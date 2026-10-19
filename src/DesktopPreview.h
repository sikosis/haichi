#ifndef HAICHI_DESKTOP_PREVIEW_H
#define HAICHI_DESKTOP_PREVIEW_H

#include <View.h>

#include <vector>

#include "IconPositionBackend.h"

namespace Haichi {

class DesktopPreview : public BView {
public:
	DesktopPreview();

	virtual void Draw(BRect updateRect);
	virtual void MouseDown(BPoint where);
	virtual void MouseMoved(BPoint where, uint32 transit,
		const BMessage* dragMessage);
	virtual void MouseUp(BPoint where);

	void SetIcons(const std::vector<IconItem>& icons, int32 selectedIndex);
	void SetPinPosition(BPoint position, bool visible);
	void SetScreenSize(int32 width, int32 height);

private:
	BRect MonitorFrame() const;
	void PickPosition(BPoint where);

	std::vector<IconItem> fIcons;
	int32 fSelectedIndex;
	int32 fScreenWidth;
	int32 fScreenHeight;
	BPoint fPinPosition;
	bool fPinVisible;
	bool fDragging;
};

} // namespace Haichi

#endif // HAICHI_DESKTOP_PREVIEW_H
