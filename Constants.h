#pragma once

static const std::wstring EXT_VWX(L"vwx");

// PHOTOGRAM extensions
static const std::wstring EXT_TIFF(L"tiff");
static const std::wstring EXT_TIF(L"tif");
static const std::wstring EXT_SVG(L"svg");
static const std::wstring EXT_PNG(L"png");
static const std::wstring EXT_JPG(L"jpg");
static const std::wstring EXT_JPEG(L"jpeg");
static const std::wstring EXT_ICO(L"ico");
static const std::wstring EXT_GIF(L"gif");
static const std::wstring EXT_BMP(L"bmp");

static const std::wstring ENV_PROD(L"prod");
static const std::wstring ENV_BETA(L"beta");
static const std::wstring ENV_QA(L"qa");
static const std::wstring ENV_DEVEL(L"devel");
static const std::wstring ENV_ARRAY[] = { ENV_PROD, ENV_BETA, ENV_QA, ENV_DEVEL };

static const std::wstring PROD_CONFIG(L"dcc.main.prod_settings");
static const std::wstring BETA_CONFIG(L"dcc.main.beta_settings");
static const std::wstring QA_CONFIG(L"dcc.main.beta_settings");
static const std::wstring DEV_CONFIG(L"dcc.main.test_settings");
static const std::wstring ENV_CONFIG_ARRAY[] = { PROD_CONFIG, BETA_CONFIG, QA_CONFIG, DEV_CONFIG };

static const std::wstring ACTIVE_SESSION_FN(L"active_session.json");
