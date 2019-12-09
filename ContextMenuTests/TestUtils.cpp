#include "pch.h"
#include "Fixtures.h"

#include <algorithm>
#include <cctype>
#include <map>;

#include "../OpenWithExt/Utils.cpp";

using namespace std;


TEST(TestUtils, processPathByName) {
	wstring res;
	EXPECT_EQ(S_OK, Utils::processPathByName(L"explorer.exe", res));
	EXPECT_EQ(Utils::toLower(res), L"c:\\windows\\explorer.exe");

	wstring fail;
	EXPECT_EQ(E_INVALIDARG, Utils::processPathByName(L"Nosuchprocessatall.exe", fail));
	EXPECT_EQ(true, fail.empty());
}

TEST(TestUtils, getExtension) {
	wstring caseSens(L"C:\\Users\\s r\\example.pDF");
	wstring noExt(L"C:\\Users\\s r\\example");

	EXPECT_EQ(L"vwx", Utils::getExtension(VWX_1));
	EXPECT_EQ(L"pdf", Utils::getExtension(caseSens));
	EXPECT_EQ(L"", Utils::getExtension(FOLDER_1));
	EXPECT_EQ(L"", Utils::getExtension(EMPTY));
}

TEST(TestUtils, isFolder) {
	wstring folder(L"C:\\Users");
	wstring file(L"C:\\Windows\\explorer.exe");
	wstring notExistent(L"bang bang");

	ASSERT_TRUE(Utils::isFolder(folder));
	ASSERT_FALSE(Utils::isFolder(file));
	ASSERT_FALSE(Utils::isFolder(notExistent));
	ASSERT_FALSE(Utils::isFolder(EMPTY));
}

TEST(TestUtils, childNodeOf) {
	wstring shorter(ROOT.substr(0, ROOT.length() - 3));

	ASSERT_FALSE(Utils::childNodeOf(ROOT, ROOT));
	ASSERT_FALSE(Utils::childNodeOf(EMPTY, ROOT));
	ASSERT_FALSE(Utils::childNodeOf(ROOT, EMPTY));
	ASSERT_FALSE(Utils::childNodeOf(ROOT, shorter));
	ASSERT_FALSE(Utils::childNodeOf(ROOT, OUTSIDE_FOLDER));

	ASSERT_TRUE(Utils::childNodeOf(ROOT, VWX_1));
	ASSERT_TRUE(Utils::childNodeOf(ROOT, FOLDER_1));
}

TEST(TestUtils, wrapSpacesForCMD) {
	EXPECT_EQ(L"C:\\Users\\\"Test User\"\\", Utils::wrapSpacesForCMD(ROOT, L"\\"));
}

TEST(TestUtils, getActionsVWX) {
	const vector <wstring> singleVWX{ VWX_1 };
	const vector <wstring> multipleVWX{ VWX_1, VWX_2 };

	EXPECT_EQ(L"vwx", Utils::getActions(singleVWX));
	EXPECT_EQ(L"vwx", Utils::getActions(multipleVWX));
}

TEST(TestUtils, getActionsPHOTO) {
	const vector <wstring> singleIMG{ IMG_1 };
	const vector <wstring> multipleIMG{ IMG_1, IMG_2, IMG_3, IMG_4, IMG_5, IMG_6, IMG_7, IMG_8, IMG_9 };
	const vector <wstring> folder{ FOLDER_1 };

	// Correct image
	EXPECT_EQ(L"jpeg", Utils::getActions(singleIMG));
	EXPECT_EQ(L"jpeg", Utils::getActions(multipleIMG));
	EXPECT_EQ(L"jpeg", Utils::getActions(folder));
}

TEST(TestUtils, getActionsOTHERS) {
	const vector <wstring> mix{ IMG_1, VWX_1 };

	EXPECT_EQ(L"NONE", Utils::getActions(mix));
}

TEST(TestUtils, parseFromExe) {
	const wstring exePathBeta = L"C:\\Users\\simeon rolev\\AppData\\Local\\Programs\\vectorworks-cloud-services-beta\\resources\\server\\Vectorworks Cloud Services Background Service.exe";
	map<wstring, wstring> res = Utils::parsePathsFromExecutable(exePathBeta);

	ASSERT_EQ(L"C:\\Users\\simeon rolev\\AppData\\Local\\", res[L"LOCAL_APP"]);
	ASSERT_EQ(L"C:\\Users\\simeon rolev\\AppData\\Local\\Programs\\", res[L"LOCAL_APP_PROGS"]);
	ASSERT_EQ(L"vectorworks-cloud-services-beta", res[L"installedApp_"]);
	ASSERT_EQ(L"C:\\Users\\simeon rolev\\AppData\\Local\\Programs\\vectorworks-cloud-services-beta\\", res[L"BASE"]);
	ASSERT_EQ(L"C:\\Users\\simeon rolev\\AppData\\Local\\Programs\\vectorworks-cloud-services-beta\\resources\\", res[L"RESOURCES"]);
	ASSERT_EQ(L"C:\\Users\\simeon rolev\\AppData\\Local\\Programs\\vectorworks-cloud-services-beta\\resources\\server\\", res[L"SERVER"]);
	ASSERT_EQ(L"C:\\Users\\simeon rolev\\AppData\\Local\\Programs\\vectorworks-cloud-services-beta\\resources\\context_actions\\icons\\", res[L"ICONS"]);
	ASSERT_EQ(L"C:\\Users\\\"simeon rolev\"\\AppData\\Local\\Programs\\vectorworks-cloud-services-beta\\resources\\server\\\"Vectorworks Cloud Services Background Service\".exe", res[L"bgSrvCmd_"]);
}

TEST(TestUtils, envFromAppName) {
	const wstring prod = L"vectorworks-cloud-services";
	const wstring beta = L"vectorworks-cloud-services-beta";
	const wstring qa = L"vectorworks-cloud-services-qa";
	const wstring dev= L"vectorworks-cloud-services-devel";
	const wstring banger= L"vectorworks-cloud-services-bang";

	ASSERT_EQ(0, Utils::envFromAppName(prod));
	ASSERT_EQ(1, Utils::envFromAppName(beta));
	ASSERT_EQ(2, Utils::envFromAppName(qa));
	ASSERT_EQ(3, Utils::envFromAppName(dev));
	
	ASSERT_EQ(-1, Utils::envFromAppName(banger));
}
