#ifndef LINEOBJECT_H
#define LINEOBJECT_H

#pragma once

#include "Floatbits.h"
#include <QVector>
#include <qmath.h>
#include <math.h>
#include <QRgb>

class LineObject
{
public:
    LineObject();
    LineObject(float x0, float y0, float x1, float y1, float emission, unsigned int color);
    unsigned int getAttributeAsPixel(int i);

    static int getSize();

private:
    QVector<unsigned char> packFloat(float f);

    QVector<unsigned int> mData;
};

#endif // LINEOBJECT_H
