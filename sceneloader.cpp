#include "sceneloader.h"

SceneLoader::SceneLoader(QString filename)
{
    if(filename == "demoScene"){
        mScene = new Scene();

        for(int i = 0; i < 500; i++){
        LineObject l0(1.0f*qrand()/RAND_MAX,1.0f*qrand()/RAND_MAX,
                      1.0f*qrand()/RAND_MAX,1.0f*qrand()/RAND_MAX,
                      (1.0f*qrand()/RAND_MAX)/2.0,
                      qRgba((1.0f*qrand()/RAND_MAX)*255.0,
                            (1.0f*qrand()/RAND_MAX)*255.0,
                            (1.0f*qrand()/RAND_MAX)*255.0,255));
        mScene->addObject(l0);
        }

    }
}


Scene* SceneLoader::getScene(){
    if(mScene != NULL) return mScene;
    else qDebug("Error no Scene loaded !");
}
