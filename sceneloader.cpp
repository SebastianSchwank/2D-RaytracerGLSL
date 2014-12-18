#include "sceneloader.h"

SceneLoader::SceneLoader(QString filename)
{
    if(filename == "demoScene"){
        mScene = new Scene();

        LineObject l0(0.7,0.3,0.3,0.7,1.0,qRgba(255,255,255,255));
        mScene->addObject(l0);
    }
}


Scene* SceneLoader::getScene(){
    if(mScene != NULL) return mScene;
    else qDebug("Error no Scene loaded !");
}
