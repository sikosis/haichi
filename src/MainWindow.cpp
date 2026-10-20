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
	fIconList(new BListView("icons", B_MULTIPLE_SELECTION_LIST)),
	fXControl(new BTextControl("x", "X:", "", NULL)),
	fYControl(new BTextControl("y", "Y:", "", NULL)),
	fSpacingControl(new BTextControl("spacing", "Spacing:", "96", NULL)),
	fApplyButton(new BButton("apply", "Apply", new BMessage(kApplyPosition))),
	fAlignRowButton(new BButton("align row", "Align row",
		new BMessage(kAlignRow))),
	fAlignColumnButton(new BButton("align column", "Align column",
		new BMessage(kAlignColumn))),
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
		.End()
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fSpacingControl)
			.Add(fAlignRowButton)
			.Add(fAlignColumnButton)
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
	SetAlignmentControlsEnabled(false);
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

		case kAlignRow:
			AlignSelected(true);
			break;

		case kAlignColumn:
			AlignSelected(false);
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

void MainWindow::AlignSelected(bool horizontal) {
	std::vector<int32> selections;
	CollectSelections(selections);
	if (selections.size() < 2) {
		SetStatus("Select at least two icons to align");
		return;
	}

	float spacing;
	if (!ParseCoordinate(fSpacingControl, spacing) || spacing < 0
		|| spacing > 50000) {
		SetStatus("Spacing must be between 0 and 50000");
		return;
	}

	std::vector<BPoint> positions;
	for (size_t index = 0; index < selections.size(); index++) {
		const IconItem& icon = fIcons[selections[index]];
		if (!icon.hasLocation) {
			SetStatus("Every selected icon needs a current position");
			return;
		}
		positions.push_back(DisplayPosition(icon.location));
	}
	std::vector<BPoint> aligned = AlignPositions(positions, horizontal, spacing);
	for (size_t index = 0; index < aligned.size(); index++) {
		if (!IsSanePosition(aligned[index])) {
			SetStatus("Aligned positions exceed Tracker's safe range");
			return;
		}
	}

	for (size_t index = 0; index < selections.size(); index++) {
		status_t result = fEngine->MoveIcon(
			fIcons[selections[index]].name.String(), aligned[index]);
		if (result != B_OK) {
			bool rollbackFailed = false;
			for (size_t moved = 0; moved < index; moved++) {
				if (fEngine->MoveIcon(fIcons[selections[moved]].name.String(),
					positions[moved]) != B_OK) {
					rollbackFailed = true;
				}
			}
			RefreshIcons();
			BString status("Could not align icons: ");
			status << std::strerror(result);
			if (rollbackFailed)
				status << "; some earlier moves could not be restored";
			else if (index > 0)
				status << "; earlier moves were restored";
			SetStatus(status.String());
			return;
		}
	}

	int32 count = (int32)selections.size();
	RefreshIcons();
	BString status;
	status.SetToFormat("Aligned %" B_PRId32 " icons in a %s", count,
		horizontal ? "row" : "column");
	SetStatus(status.String());
}
//---------------------------------------------------------------------------------------------------------------------------------//

void MainWindow::ApplyPosition() {
	int32 selection = fIconList->CurrentSelection();
	if (selection < 0 || (size_t)selection >= fIcons.size())
		return;
	if (fIconList->CurrentSelection(1) >= 0) {
		SetStatus("Use Align row or Align column for multiple icons");
		return;
	}

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

void MainWindow::CollectSelections(std::vector<int32>& selections) const {
	selections.clear();
	for (int32 selectionIndex = 0;; selectionIndex++) {
		int32 selection = fIconList->CurrentSelection(selectionIndex);
		if (selection < 0)
			break;
		selections.push_back(selection);
	}
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
	std::vector<BString> selectedNames;
	std::vector<int32> selections;
	CollectSelections(selections);
	for (size_t index = 0; index < selections.size(); index++) {
		if ((size_t)selections[index] < fIcons.size())
			selectedNames.push_back(fIcons[selections[index]].name);
	}

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
		for (size_t selectedIndex = 0; selectedIndex < selectedNames.size();
			selectedIndex++) {
			if (fIcons[index].name == selectedNames[selectedIndex]) {
				fIconList->Select((int32)index, true);
				break;
			}
		}
	}
	UpdateSelection();

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

void MainWindow::SetAlignmentControlsEnabled(bool enabled) {
	fSpacingControl->SetEnabled(enabled);
	fAlignRowButton->SetEnabled(enabled);
	fAlignColumnButton->SetEnabled(enabled);
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
	std::vector<int32> selections;
	CollectSelections(selections);
	if (selections.empty()) {
		fXControl->SetText("");
		fYControl->SetText("");
		SetControlsEnabled(false);
		SetAlignmentControlsEnabled(false);
		fSelectionView->SetText("Selected icon: none");
		fPreview->SetIcons(fIcons, -1);
		fPreview->SetPinPosition(BPoint(0, 0), false);
		return;
	}
	int32 selection = selections[0];
	if (selections.size() > 1) {
		BString selected;
		selected.SetToFormat("Selected icons: %" B_PRId32,
			(int32)selections.size());
		fSelectionView->SetText(selected.String());
		fXControl->SetText("");
		fYControl->SetText("");
		SetControlsEnabled(false);
		SetAlignmentControlsEnabled(true);
		fPreview->SetIcons(fIcons, selection);
		fPreview->SetPinPosition(BPoint(0, 0), false);
		return;
	}
	SetAlignmentControlsEnabled(false);

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
