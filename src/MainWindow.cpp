#include "MainWindow.h"

#include "Constants.h"
#include "DesktopPreview.h"
#include "HaichiEngine.h"
#include "PositionScaling.h"

#include <Button.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <ScrollView.h>
#include <Screen.h>
#include <Size.h>
#include <SplitView.h>
#include <StringItem.h>
#include <StringView.h>
#include <TextControl.h>
#include <View.h>

#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace Haichi {

MainWindow::MainWindow(HaichiEngine* engine)
	:
	BWindow(BRect(60, 60, 1160, 780), "haichi", B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS),
	fEngine(engine),
	fIconList(new BListView("icons", B_SINGLE_SELECTION_LIST)),
	fXControl(new BTextControl("x", "X:", "", NULL)),
	fYControl(new BTextControl("y", "Y:", "", NULL)),
	fApplyButton(new BButton("apply", "Apply", new BMessage(kApplyPosition))),
	fStatusView(new BStringView("status", "Ready")),
	fSelectionView(new BStringView("selection", "Selected icon: none")),
	fPreview(new DesktopPreview()) {
	fIconList->SetSelectionMessage(new BMessage(kSelectionChanged));
	fIconList->SetExplicitMinSize(BSize(220, 400));
	BScrollView* iconScroll = new BScrollView("icon scroll", fIconList, 0,
		true, true);
	iconScroll->SetExplicitMinSize(BSize(240, 420));
	BView* iconPane = new BView("icon pane", B_WILL_DRAW);
	iconPane->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	iconPane->SetExplicitMinSize(BSize(240, 420));
	BView* previewPane = new BView("preview pane", B_WILL_DRAW);
	previewPane->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	BButton* refreshButton = new BButton("refresh", "Refresh",
		new BMessage(kRefreshIcons));
	BStringView* previewTitle = new BStringView("preview title",
		"Desktop position preview");
	BStringView* iconTitle = new BStringView("icon title", "Desktop icons");
	BStringView* previewHelp = new BStringView("preview help",
		"Click an icon square to select it, then drag it or click a destination.");
	BLayoutBuilder::Group<>(iconPane, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.Add(iconTitle)
		.Add(iconScroll, 1.0f);
	BLayoutBuilder::Group<>(previewPane, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.Add(previewTitle)
		.Add(fSelectionView)
		.Add(fPreview, 1.0f)
		.Add(previewHelp)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fXControl)
			.Add(fYControl)
			.Add(fApplyButton)
		.End();
	BSplitView* splitView = new BSplitView(B_HORIZONTAL,
		B_USE_DEFAULT_SPACING);
	splitView->AddChild(iconPane, 0.34f);
	splitView->AddChild(previewPane, 0.66f);
	splitView->SetCollapsible(false);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_SPACING)
		.Add(splitView, 1.0f)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fStatusView, 1.0f)
			.Add(refreshButton)
		.End();

	SetControlsEnabled(false);
	PostMessage(kRefreshIcons);
}
//---------------------------------------------------------------------------------------------------------------------------------//

void MainWindow::MessageReceived(BMessage* message) {
	switch (message->what) {
		case kApplyPosition:
			ApplyPosition();
			break;

		case kRefreshIcons:
			RefreshIcons();
			break;

		case kSelectionChanged:
			UpdateSelection();
			break;

		case kPreviewPosition:
			SetPickedPosition(message);
			break;

		case kPreviewIconSelected:
			SelectPreviewIcon(message);
			break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//

bool MainWindow::QuitRequested() {
	Hide();
	return false;
}
//---------------------------------------------------------------------------------------------------------------------------------//

void MainWindow::ApplyPosition() {
	int32 selection = fIconList->CurrentSelection();
	if (selection < 0 || (size_t)selection >= fIcons.size())
		return;

	float x;
	float y;
	if (!ParseCoordinate(fXControl, x) || !ParseCoordinate(fYControl, y)) {
		SetStatus("X and Y must be numbers");
		return;
	}

	BPoint location(x, y);
	if (!IsSanePosition(location)) {
		SetStatus("X and Y must be between -50000 and 50000");
		return;
	}

	status_t result = fEngine->MoveIcon(fIcons[selection].name.String(), location);
	if (result != B_OK) {
		BString status("Could not move icon: ");
		status << std::strerror(result);
		SetStatus(status.String());
		return;
	}

	BString movedName(fIcons[selection].name);
	RefreshIcons();
	fIconList->Select(selection);
	BString status("Moved ");
	status << movedName;
	SetStatus(status.String());
}
//---------------------------------------------------------------------------------------------------------------------------------//

bool MainWindow::ParseCoordinate(BTextControl* control, float& coordinate) const {
	const char* text = control->Text();
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

void MainWindow::RefreshIcons() {
	BString selectedName;
	int32 selection = fIconList->CurrentSelection();
	if (selection >= 0 && (size_t)selection < fIcons.size())
		selectedName = fIcons[selection].name;

	std::vector<IconItem> icons;
	status_t result = fEngine->ListIcons(icons);
	if (result != B_OK) {
		BString status("Could not list Desktop icons: ");
		status << std::strerror(result);
		SetStatus(status.String());
		return;
	}

	while (fIconList->CountItems() > 0)
		delete fIconList->RemoveItem((int32)0);
	fIcons.swap(icons);
	BScreen screen(B_MAIN_SCREEN_ID);
	if (screen.IsValid()) {
		BRect frame = screen.Frame();
		fPreview->SetScreenSize(frame.IntegerWidth() + 1,
			frame.IntegerHeight() + 1);
	}

	int32 restoredSelection = -1;
	for (size_t index = 0; index < fIcons.size(); index++) {
		BString label(fIcons[index].name);
		if (fIcons[index].hasLocation) {
			BPoint displayed = DisplayPosition(fIcons[index].location);
			char coordinates[80];
			std::snprintf(coordinates, sizeof(coordinates), "    X %g    Y %g",
				displayed.x, displayed.y);
			label << coordinates;
		}
		fIconList->AddItem(new BStringItem(label.String()));
		if (fIcons[index].name == selectedName)
			restoredSelection = (int32)index;
	}

	if (restoredSelection >= 0)
		fIconList->Select(restoredSelection);
	else
		UpdateSelection();
	fPreview->SetIcons(fIcons, fIconList->CurrentSelection());

	BString status;
	status.SetToFormat("Listed %" B_PRId32 " Desktop icons",
		(int32)fIcons.size());
	SetStatus(status.String());
}
//---------------------------------------------------------------------------------------------------------------------------------//

void MainWindow::SelectPreviewIcon(BMessage* message) {
	int32 index;
	if (message->FindInt32("index", &index) != B_OK || index < 0
		|| (size_t)index >= fIcons.size()) {
		return;
	}
	fIconList->Select(index);
	UpdateSelection();
}
//---------------------------------------------------------------------------------------------------------------------------------//

void MainWindow::SetPickedPosition(BMessage* message) {
	float x;
	float y;
	if (message->FindFloat("x", &x) != B_OK
		|| message->FindFloat("y", &y) != B_OK) {
		return;
	}

	char coordinate[40];
	std::snprintf(coordinate, sizeof(coordinate), "%.0f", x);
	fXControl->SetText(coordinate);
	std::snprintf(coordinate, sizeof(coordinate), "%.0f", y);
	fYControl->SetText(coordinate);
	SetStatus("Position selected; choose Apply to move the icon");
}
//---------------------------------------------------------------------------------------------------------------------------------//

void MainWindow::SetControlsEnabled(bool enabled) {
	fXControl->SetEnabled(enabled);
	fYControl->SetEnabled(enabled);
	fApplyButton->SetEnabled(enabled);
}
//---------------------------------------------------------------------------------------------------------------------------------//

void MainWindow::SetStatus(const char* text) {
	fStatusView->SetText(text);
}
//---------------------------------------------------------------------------------------------------------------------------------//

void MainWindow::UpdateSelection() {
	int32 selection = fIconList->CurrentSelection();
	if (selection < 0 || (size_t)selection >= fIcons.size()) {
		fXControl->SetText("");
		fYControl->SetText("");
		SetControlsEnabled(false);
		fSelectionView->SetText("Selected icon: none");
		fPreview->SetIcons(fIcons, -1);
		fPreview->SetPinPosition(BPoint(0, 0), false);
		return;
	}

	if (!fIcons[selection].hasLocation) {
		BString selected("Selected icon: ");
		selected << fIcons[selection].name;
		fSelectionView->SetText(selected.String());
		fXControl->SetText("0");
		fYControl->SetText("0");
		SetControlsEnabled(true);
		fPreview->SetIcons(fIcons, selection);
		fPreview->SetPinPosition(BPoint(0, 0), false);
		return;
	}

	BString selected("Selected icon: ");
	selected << fIcons[selection].name;
	fSelectionView->SetText(selected.String());
	BPoint displayed = DisplayPosition(fIcons[selection].location);
	char coordinate[40];
	std::snprintf(coordinate, sizeof(coordinate), "%g", displayed.x);
	fXControl->SetText(coordinate);
	std::snprintf(coordinate, sizeof(coordinate), "%g", displayed.y);
	fYControl->SetText(coordinate);
	SetControlsEnabled(true);
	fPreview->SetIcons(fIcons, selection);
	fPreview->SetPinPosition(displayed, true);
}
//---------------------------------------------------------------------------------------------------------------------------------//

} // namespace Haichi
