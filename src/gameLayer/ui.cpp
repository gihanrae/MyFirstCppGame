#include "ui.h"


Rectangle placeRectangleTopRightCorner(Rectangle r, float w)
{
	r.x = w - r.width;
	r.y = 0;
	return r;
}

Rectangle placeRectangleTopLeftCorner(Rectangle r, float w)
{
	r.x = 0;
	r.y = 0;
	return r;
}

Rectangle placeRectangleBottomRightCorner(Rectangle r, float w, float h)
{
	r.x = w - r.width;
	r.y = h - r.height;
	return r;
}

Rectangle placeRectangleBottomLeftCorner(Rectangle r, float w, float h)
{
	r.x = 0;
	r.y = h - r.height;
	return r;
}

Rectangle placeRectangleCenter(Rectangle r, float w, float h)
{
	r.x = (w - r.width) / 2.0f;
	r.y = (h - r.height) / 2.0f;
	return r;
}

Rectangle placeRectangleCenterTop(Rectangle r, float w)
{
	r.x = (w - r.width) / 2.0f;
	r.y = 0;
	return r;
}

Rectangle placeRectangleCenterBottom(Rectangle r, float w, float h)
{
	r.x = (w - r.width) / 2.0f;
	r.y = h - r.height;
	return r;
}

Rectangle placeRectangleCenterLeft(Rectangle r, float h)
{
	r.x = 0;
	r.y = (h - r.height) / 2.0f;
	return r;
}

Rectangle placeRectangleCenterRight(Rectangle r, float w, float h)
{
	r.x = w - r.width;
	r.y = (h - r.height) / 2.0f;
	return r;
}

Rectangle enlargeRectanglePixels(Rectangle r, float pixelsX, float pixelsY)
{
	r.width += pixelsX;
	r.height += pixelsY;

	r.x -= pixelsX / 2.f;
	r.y -= pixelsY / 2.f;

	return r;
}

Rectangle shrinkRectanglePercentage(Rectangle r, float percentageX, float percentageY)
{
	float shrinkX = r.width * percentageX;
	float shrinkY = r.height * percentageY;

	r.width -= shrinkX;
	r.height -= shrinkY;

	r.x += shrinkX / 2.f;
	r.y += shrinkY / 2.f;

	return r;
}

