#ifndef PLAYGROUND_H
#define PLAYGROUND_H

#pragma once

#include <QImage>
#include <QColor>
#include <QVector>

#include <qmath.h>
#include "Floatbits.h"

class Playground : public QImage
{
public:
    Playground(unsigned int neurons);
};

#endif // PLAYGROUND_H
