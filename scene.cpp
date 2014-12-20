#include "scene.h"

Scene::Scene()
{
}

void Scene::addObject(LineObject newObject){
    mScene.append(newObject);
}

QImage* Scene::getSceneImage(){
    //Probably a memory leak here !
    SceneImage = new QImage(mScene.size()+4,LineObject::getSize(),QImage::Format_ARGB32);

    //Set the Border
    LineObject b0(0,0,1.0,0,qRgba(0,0,0,255),0.0,0,0,0);
    for(int i = 0; i < LineObject::getSize(); i++)
        SceneImage->setPixel(0,i,b0.getAttributeAsPixel(i));

    LineObject b1(1.0,0,1.0,1.0,qRgba(0,0,0,255),0.0,0,0,0);
    for(int i = 0; i < LineObject::getSize(); i++)
        SceneImage->setPixel(1,i,b1.getAttributeAsPixel(i));

    LineObject b2(1.0,1.0,0,1.0,qRgba(0,0,0,255),0.0,0,0,0);
    for(int i = 0; i < LineObject::getSize(); i++)
        SceneImage->setPixel(2,i,b2.getAttributeAsPixel(i));

    LineObject b3(0,1.0,0,0,qRgba(0,0,0,255),0.0,0,0,0);
    for(int i = 0; i < LineObject::getSize(); i++)
        SceneImage->setPixel(3,i,b3.getAttributeAsPixel(i));

    //Set the Scene
    for(int j = 0; j < mScene.size(); j++){
        for(int i = 0; i < LineObject::getSize(); i++)
            SceneImage->setPixel(j+4,i,mScene[j].getAttributeAsPixel(i));
    }

    return SceneImage;
}
