#ifndef HAICHI_APPLICATION_H
#define HAICHI_APPLICATION_H

#include <Application.h>

#include <memory>

#include "HaichiEngine.h"

namespace Haichi {

class MainWindow;

class HaichiApplication : public BApplication {
public:
	HaichiApplication();
	virtual ~HaichiApplication();

	virtual void AboutRequested();
	virtual void MessageReceived(BMessage* message);
	virtual bool QuitRequested();
	virtual void ReadyToRun();

private:
	void InstallDeskbarItem();
	void OpenWindow();

	std::unique_ptr<HaichiEngine> fEngine;
	MainWindow* fWindow;
};

} // namespace Haichi

#endif // HAICHI_APPLICATION_H
