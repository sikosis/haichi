#include "AboutWindow.h"

#include "Constants.h"

#include <Application.h>
#include <Bitmap.h>
#include <Button.h>
#include <Font.h>
#include <IconUtils.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <Mime.h>
#include <Resources.h>
#include <Roster.h>
#include <SeparatorView.h>
#include <Size.h>
#include <String.h>
#include <StringView.h>
#include <TabView.h>
#include <View.h>

namespace Haichi {

namespace {

class IconView : public BView {
public:
	explicit IconView(BBitmap* icon);
	virtual ~IconView();

	virtual void Draw(BRect updateRect);

private:
	BBitmap* fIcon;
};


class LinkView : public BStringView {
public:
	LinkView(const char* name, const char* label, const char* url);

	virtual void AttachedToWindow();
	virtual void MouseDown(BPoint where);

private:
	BString fUrl;
};


BBitmap* LoadAppIcon() {
	const int32 iconSize = 96;
	BBitmap* icon = new BBitmap(BRect(0, 0, iconSize - 1, iconSize - 1),
		B_RGBA32);
	if (icon->InitCheck() != B_OK) {
		delete icon;
		return NULL;
	}

	BResources* resources = BApplication::AppResources();
	if (resources != NULL) {
		size_t dataSize = 0;
		const uint8* data = static_cast<const uint8*>(
			resources->LoadResource(B_VECTOR_ICON_TYPE, "BEOS:ICON", &dataSize));
		if (data != NULL && dataSize > 0
			&& BIconUtils::GetVectorIcon(data, dataSize, icon) == B_OK) {
			return icon;
		}
	}

	delete icon;
	return NULL;
}
//---------------------------------------------------------------------------------------------------------------------------------//

BStringView* Heading(const char* name, const char* text) {
	BStringView* heading = new BStringView(name, text);
	heading->SetFont(be_bold_font);
	return heading;
}
//---------------------------------------------------------------------------------------------------------------------------------//

BView* CreateCreditsView() {
	BView* view = new BView("Credits", B_WILL_DRAW);
	view->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	BLayoutBuilder::Group<>(view, B_VERTICAL, 5.0f)
		.SetInsets(12.0f)
		.Add(Heading("project heading", "Project"))
		.Add(new BStringView("designer", "Designed and developed by Sikosis"))
		.Add(new BStringView("created", "Created: 19 October 2026"))
		.Add(new BStringView("updated", "Updated: 20 October 2026"))
		.Add(new BStringView("platform", "A native Deskbar application for Haiku"))
		.AddStrut(8.0f)
		.Add(Heading("thanks heading", "Special Thanks"))
		.Add(new BStringView("haiku thanks", "The Haiku and Tracker contributors"))
		.Add(new BStringView("community thanks", "The Haiku community"))
		.AddGlue();

	return view;
}
//---------------------------------------------------------------------------------------------------------------------------------//

BView* CreateLicenceView() {
	BView* view = new BView("Licence", B_WILL_DRAW);
	view->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	BLayoutBuilder::Group<>(view, B_VERTICAL, 5.0f)
		.SetInsets(12.0f)
		.Add(Heading("haichi licence heading", "haichi"))
		.Add(new BStringView("copyright", "Copyright 2026 Sikosis"))
		.Add(new BStringView("licence", "All rights reserved."))
		.AddStrut(8.0f)
		.Add(Heading("haiku heading", "Haiku"))
		.Add(new BStringView("haiku acknowledgement",
			"Built with the Haiku application and interface kits."))
		.Add(new LinkView("haiku link", "haiku-os.org", "https://www.haiku-os.org/"))
		.AddGlue();

	return view;
}
//---------------------------------------------------------------------------------------------------------------------------------//

IconView::IconView(BBitmap* icon)
	:
	BView("application icon", B_WILL_DRAW),
	fIcon(icon) {
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetExplicitMinSize(BSize(96, 96));
	SetExplicitMaxSize(BSize(96, 96));
}
//---------------------------------------------------------------------------------------------------------------------------------//

IconView::~IconView() {
	delete fIcon;
}
//---------------------------------------------------------------------------------------------------------------------------------//

void IconView::Draw(BRect updateRect) {
	(void)updateRect;
	if (fIcon == NULL)
		return;

	SetDrawingMode(B_OP_ALPHA);
	SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
	DrawBitmap(fIcon, fIcon->Bounds(), Bounds(), B_FILTER_BITMAP_BILINEAR);
}
//---------------------------------------------------------------------------------------------------------------------------------//

LinkView::LinkView(const char* name, const char* label, const char* url)
	:
	BStringView(name, label),
	fUrl(url) {
	SetToolTip(url);
	BFont font;
	GetFont(&font);
	font.SetFace(font.Face() | B_UNDERSCORE_FACE);
	SetFont(&font);
}
//---------------------------------------------------------------------------------------------------------------------------------//

void LinkView::AttachedToWindow() {
	BStringView::AttachedToWindow();
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	rgb_color background = ui_color(B_PANEL_BACKGROUND_COLOR);
	int32 luminance = (299 * background.red + 587 * background.green
		+ 114 * background.blue) / 1000;
	if (luminance < 128)
		SetHighColor(105, 190, 255);
	else
		SetHighColor(0, 76, 168);
}
//---------------------------------------------------------------------------------------------------------------------------------//

void LinkView::MouseDown(BPoint where) {
	(void)where;
	const char* arguments[] = { fUrl.String() };
	be_roster->Launch("text/html", 1, arguments);
}
//---------------------------------------------------------------------------------------------------------------------------------//

} // namespace


AboutWindow::AboutWindow()
	:
	BWindow(BRect(0, 0, 600, 500), "About haichi", B_TITLED_WINDOW,
		B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS
			| B_CLOSE_ON_ESCAPE) {
	BStringView* title = new BStringView("title", "haichi");
	BFont titleFont(*be_bold_font);
	titleFont.SetSize(be_plain_font->Size() * 2.0f);
	title->SetFont(&titleFont);

	BString versionText("Version v");
	versionText << kApplicationVersion;
	BStringView* version = new BStringView("version", versionText.String());
	BStringView* description = new BStringView("description",
		"Pinpoint placement for Haiku Desktop icons.");

	BTabView* tabs = new BTabView("about tabs", B_WIDTH_FROM_LABEL);
	tabs->AddTab(CreateCreditsView());
	tabs->AddTab(CreateLicenceView());

	BButton* closeButton = new BButton("close", "Close",
		new BMessage(B_QUIT_REQUESTED));
	closeButton->MakeDefault(true);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 10.0f)
		.SetInsets(18.0f)
		.AddGroup(B_HORIZONTAL, 16.0f)
			.Add(new IconView(LoadAppIcon()))
			.AddGroup(B_VERTICAL, 4.0f)
				.Add(title)
				.Add(version)
				.AddStrut(4.0f)
				.Add(description)
				.Add(new LinkView("source link", "github.com/sikosis/haichi",
					"https://github.com/sikosis/haichi"))
				.AddGlue()
			.End()
		.End()
		.Add(new BSeparatorView(B_HORIZONTAL))
		.Add(tabs, 1.0f)
		.AddGroup(B_HORIZONTAL, 0.0f)
			.AddGlue()
			.Add(closeButton)
		.End();

	CenterOnScreen();
}
//---------------------------------------------------------------------------------------------------------------------------------//

} // namespace Haichi
