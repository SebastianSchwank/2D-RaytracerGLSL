#ifndef SCENE_H
#define SCENE_H

#pragma once

#include <QVector>
#include <QImage>
#include "lineobject.h"


class Scene
{
public:
    Scene();
    void addObject(LineObject newObject);
    QImage *getSceneImage();

private:
    QVector<LineObject> mScene;
    QImage *SceneImage;
};

#endif // SCENE_H
