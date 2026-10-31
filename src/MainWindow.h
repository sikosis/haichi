#ifndef HAICHI_MAIN_WINDOW_H
#define HAICHI_MAIN_WINDOW_H

#include <Window.h>

#include <vector>

#include "IconPositionBackend.h"

class BButton;
class BListView;
class BMenuField;
class BPopUpMenu;
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
	void AlignSelected(bool horizontal);
	void ApplyPosition();
	void CollectSelections(std::vector<int32>& selections) const;
	bool GetScreenSize(int32& width, int32& height) const;
	bool ParseCoordinate(BTextControl* control, float& coordinate) const;
	void PlaceRelative(int32 horizontalDirection, int32 verticalDirection);
	void RebuildReferenceMenu(const char* previousName);
	void RefreshIcons();
	void RestoreSnapshot();
	void SaveSnapshot();
	void SetReferenceIcon(BMessage* message);
	void SelectPreviewIcon(BMessage* message);
	void SetPickedPosition(BMessage* message);
	void SetAlignmentControlsEnabled(bool enabled);
	void SetControlsEnabled(bool enabled);
	void SetStatus(const char* text);
	void UpdateReferenceControls();
	void UpdateSelection();

	HaichiEngine* fEngine;
	BListView* fIconList;
	BTextControl* fXControl;
	BTextControl* fYControl;
	BTextControl* fSpacingControl;
	BPopUpMenu* fReferenceMenu;
	BMenuField* fReferenceField;
	BButton* fApplyButton;
	BButton* fAlignRowButton;
	BButton* fAlignColumnButton;
	BButton* fPlaceLeftButton;
	BButton* fPlaceRightButton;
	BButton* fPlaceAboveButton;
	BButton* fPlaceBelowButton;
	BStringView* fStatusView;
	BStringView* fSelectionView;
	DesktopPreview* fPreview;
	std::vector<IconItem> fIcons;
	int32 fReferenceIndex;
};

} // namespace Haichi

#endif // HAICHI_MAIN_WINDOW_H
