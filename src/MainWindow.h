#ifndef HAICHI_MAIN_WINDOW_H
#define HAICHI_MAIN_WINDOW_H

#include <Window.h>

#include <vector>

#include "IconPositionBackend.h"

class BButton;
class BListView;
class BStringView;
class BTextControl;

namespace Haichi {

class HaichiEngine;
class DesktopPreview;

class MainWindow : public BWindow {
public:
	explicit MainWindow(HaichiEngine* engine);

	virtual void MessageReceived(BMessage* message);
	virtual bool QuitRequested();

private:
	void ApplyPosition();
	bool ParseCoordinate(BTextControl* control, float& coordinate) const;
	void RefreshIcons();
	void SelectPreviewIcon(BMessage* message);
	void SetPickedPosition(BMessage* message);
	void SetControlsEnabled(bool enabled);
	void SetStatus(const char* text);
	void UpdateSelection();

	HaichiEngine* fEngine;
	BListView* fIconList;
	BTextControl* fXControl;
	BTextControl* fYControl;
	BButton* fApplyButton;
	BStringView* fStatusView;
	BStringView* fSelectionView;
	DesktopPreview* fPreview;
	std::vector<IconItem> fIcons;
};

} // namespace Haichi

#endif // HAICHI_MAIN_WINDOW_H
