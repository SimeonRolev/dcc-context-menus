#pragma once
#include <string>;

using namespace std;

const wstring ROOT(L"C:\\Users\\Test User\\");
const wstring DROPBOX_ROOT(L"C:\\Users\\Dropbox\\");

const wstring VWX_1(ROOT + L"first.vwx");
const wstring VWX_2(ROOT + L"second.vwx");

const wstring IMG_1(ROOT + L"image.tiff");
const wstring IMG_2(ROOT + L"image.tif");
const wstring IMG_3(ROOT + L"image.svg");
const wstring IMG_4(ROOT + L"image.png");
const wstring IMG_5(ROOT + L"image.jpg");
const wstring IMG_6(ROOT + L"image.jpeg");
const wstring IMG_7(ROOT + L"image.ico");
const wstring IMG_8(ROOT + L"image.gif");
const wstring IMG_9(ROOT + L"image.bmp");

const wstring FOLDER_1(ROOT + L"folder\\\\");

const wstring EMPTY(L"");
const wstring OUTSIDE_FOLDER(L"C:\\Users\\Not here\\");

const wstring DROPBOX_SYNC_FILE(DROPBOX_ROOT + L"dropboxfile.vwx");
const wstring OUTSIDE_PATH(OUTSIDE_FOLDER + L"nothing.jpg");
