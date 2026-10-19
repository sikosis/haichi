#ifndef HAICHI_TRAY_VIEW_H
#define HAICHI_TRAY_VIEW_H

#include <View.h>

namespace Haichi {

class TrayView : public BView {
public:
	explicit TrayView(BRect frame);
	explicit TrayView(BMessage* archive);

	static BArchivable* Instantiate(BMessage* archive);

	virtual status_t Archive(BMessage* archive, bool deep = true) const;
	virtual void AttachedToWindow();
	virtual void Draw(BRect updateRect);
	virtual void MouseDown(BPoint where);
};

} // namespace Haichi

extern "C" _EXPORT BView* instantiate_deskbar_item(float maxWidth,
	float maxHeight);

#endif // HAICHI_TRAY_VIEW_H
