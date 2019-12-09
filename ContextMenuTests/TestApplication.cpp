#include "pch.h"
#include "Fixtures.h"
#include <gtest/gtest.h>

#include <vector>

#include "../OpenWithExt/Application.cpp";

using namespace std;


struct AppCorrect : testing::Test {
	Application* app;

	AppCorrect() {
		app = new Application();
		const wstring appPath = L"C:\\Users\\simeon rolev\\AppData\\Local\\Programs\\vectorworks-cloud-services-devel\\resources\\server\\Vectorworks Cloud Services Background Service.exe";
		
		// Mimic the setUp() method
		app->setPaths(Utils::parsePathsFromExecutable(appPath));
		app->setEnv(Utils::envFromAppName(app->installedApp()));
		app->setSyncDir(ROOT);
		app->setDropboxDir(DROPBOX_ROOT);
	}

	~AppCorrect() {
		delete app;
	}
};

TEST_F(AppCorrect, parseFile) {
	EXPECT_EQ(S_OK, app->parseFile(VWX_1));
	EXPECT_EQ(S_OK, app->parseFile(DROPBOX_SYNC_FILE));
	EXPECT_EQ(E_INVALIDARG, app->parseFile(OUTSIDE_PATH));
	EXPECT_EQ(E_INVALIDARG, app->parseFile(EMPTY));

	vector<wstring> files = { VWX_1, DROPBOX_SYNC_FILE };
	EXPECT_EQ(app->filesArray().size(), 2);
	EXPECT_EQ(app->filesArray(), files);
}
