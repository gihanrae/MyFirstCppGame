#pragma once
#include <raylib.h>
#include <string>
#include <vector>

Rectangle placeRectangleTopRightCorner(Rectangle r, float w);

Rectangle placeRectangleTopLeftCorner(Rectangle r, float w);

Rectangle placeRectangleBottomRightCorner(Rectangle r, float w, float h);

Rectangle placeRectangleBottomLeftCorner(Rectangle r, float w, float h);

Rectangle placeRectangleCenter(Rectangle r, float w, float h);

Rectangle placeRectangleCenterTop(Rectangle r, float w);

Rectangle placeRectangleCenterBottom(Rectangle r, float w, float h);

Rectangle placeRectangleCenterLeft(Rectangle r, float h);

Rectangle placeRectangleCenterRight(Rectangle r, float w, float h);

Rectangle enlargeRectanglePixels(Rectangle r, float pixelsX, float pixelsY);

Rectangle shrinkRectanglePercentage(Rectangle r, float percentageX, float percentageY);



