
#ifndef _OPTICS_H
#define _OPTICS_H


struct position
{
	int x, y;
};

struct dimension
{
	int w, h;
};

struct rect
{
	position pos;
	dimension dim;
};

static dimension	MainWinDim = { 64, 64 };
static dimension	InfoWinDim = { 164, MainWinDim.h };
static rect			StatusPixmapRect = { { 5, 15 }, { 55, 30 } };
static rect			DirectionPixmapRect = { { 5, 33 }, { 55, 12 } };
static dimension	LedDim = { 6, 8 };
static rect			LampsRect[3] = { { {7,6}, {10,10} }, { {27,6}, {10,10} }, { {47,6}, {10,10} } };
static rect			InfoSWRect = { { 8, 48 }, { 10, 8 } };
static rect			LampsSWRect = { { 18, 48 }, { 10, 8 } };
static rect			DeviceRect = { { 30, 48}, { MainWinDim.w-30-2,LedDim.h } };

static char			WindowBackgroundColor[] = "#202020";
static char			DeviceColorHigh[] = "#3ddeff";
static char			DeviceColorLow[] = "#007bff";
static char			InfoTextColor[] = "#167ce2";

#endif
