#pragma once
#include <string>

using namespace std;

static const wstring EXT_VWX(L"vwx");

// PHOTOGRAM extensions
static const wstring EXT_TIFF(L"tiff");
static const wstring EXT_TIF(L"tif");
static const wstring EXT_SVG(L"svg");
static const wstring EXT_PNG(L"png");
static const wstring EXT_JPG(L"jpg");
static const wstring EXT_JPEG(L"jpeg");
static const wstring EXT_ICO(L"ico");
static const wstring EXT_GIF(L"gif");
static const wstring EXT_BMP(L"bmp"); 

static const wstring ENV_PROD(L"prod");
static const wstring ENV_BETA(L"beta");
static const wstring ENV_QA(L"qa");
static const wstring ENV_DEVEL(L"devel");
static const wstring ENV_ARRAY[] = { ENV_PROD, ENV_BETA, ENV_QA, ENV_DEVEL };

static const wstring PROD_CONFIG(L"dcc.main.prod_settings");
static const wstring BETA_CONFIG(L"dcc.main.beta_settings");
static const wstring QA_CONFIG(L"dcc.main.beta_settings");
static const wstring DEV_CONFIG(L"dcc.main.test_settings");
static const wstring ENV_CONFIG_ARRAY[] = { PROD_CONFIG, BETA_CONFIG, QA_CONFIG, DEV_CONFIG };

static const wstring ACTIVE_SESSION_FN(L"active_session.json");
