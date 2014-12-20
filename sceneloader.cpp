#include "sceneloader.h"

SceneLoader::SceneLoader(QString filename)
{

    qsrand((uint)QTime::currentTime().msec());

    if(filename == "demoScene"){
        mScene = new Scene();

        for(int i = 0; i < 1; i++){
            LineObject l0(1.0f*qrand()/RAND_MAX,1.0f*qrand()/RAND_MAX,
                          1.0f*qrand()/RAND_MAX,1.0f*qrand()/RAND_MAX,
                          qRgba((1.0f*qrand()/RAND_MAX)*255.0,
                                (1.0f*qrand()/RAND_MAX)*255.0,
                                (1.0f*qrand()/RAND_MAX)*255.0,255),
                          0.7,
                          1.0f*qrand()/RAND_MAX,1.0f*qrand()/RAND_MAX);
            mScene->addObject(l0);
         }

        for(int i = 0; i < 1; i++){
            LineObject l0(1.0f*qrand()/RAND_MAX,1.0f*qrand()/RAND_MAX,
                          1.0f*qrand()/RAND_MAX,1.0f*qrand()/RAND_MAX,
                          qRgba((1.0f*qrand()/RAND_MAX)*255.0,
                                (1.0f*qrand()/RAND_MAX)*255.0,
                                (1.0f*qrand()/RAND_MAX)*255.0,255),
                          0.0,
                          1.0f*qrand()/RAND_MAX,1.0f*qrand()/RAND_MAX);
            mScene->addObject(l0);
         }
    }
}


Scene* SceneLoader::getScene(){
    if(mScene != NULL) return mScene;
    else qDebug("Error no Scene loaded !");
}
