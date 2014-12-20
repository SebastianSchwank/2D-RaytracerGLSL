#include "glann.h"

GLANN::GLANN(unsigned int width, unsigned int height, unsigned int renderPasses, Scene *renderScene)
{
    setFixedSize(width,height);
    this->width = width;
    this->height = height;

    qsrand((uint)QTime::currentTime().msec());

    SceneImage = renderScene->getSceneImage();
    numObjects = SceneImage->width();
    renderedImage = new Playground(width);
}

void GLANN::initializeGL(){
    initializeGLFunctions();
    initShader();
    initTextures();
    //initFbo();
    // Use QBasicTimer because its faster than QTimer
    timer.start(0, this);
}

void GLANN::initFbo(){
    //glGenFramebuffers(1, &fboId);
}

void GLANN::resizeGL(int w, int h){
    glViewport(0,0,w,h);
}

void GLANN::paintGL(){

    render();

    //getFeedbackTexture();

    //increment number of rendered passes
    mRenderPasses++;
}

void GLANN::render(){

    // Set random seed
    program.setUniformValue("seed", ((float)qrand()/RAND_MAX));

    //Set number of alredy rendered passes
    program.setUniformValue("numRenderPass",mRenderPasses);

    //Bind last rendered Image
    //pixelsRenderedImage = bindTexture(*renderedImage);

    //Load Identity
    glLoadIdentity();

    //Move to rendering point
    glTranslatef( -1.0, -1.0, 0.0f );

    glEnable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pixelsScene);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pixelsRenderedImage);

    // Draw geometry
    //Render textured quad
    glBegin( GL_QUADS );
        glTexCoord2f( 0.f, 0.f ); glVertex2f( 0, 0);
        glTexCoord2f( 1.f, 0.f ); glVertex2f( 2.0, 0);
        glTexCoord2f( 1.f, 1.f ); glVertex2f( 2.0, 2.0);
        glTexCoord2f( 0.f, 1.f ); glVertex2f( 0, 2.0);
     glEnd();

     // Getting the pixels from the upper left 512x512 part of the screen:

     //Get the rendered Image as Texure
     glReadBuffer(GL_BACK);
     glEnable(GL_TEXTURE_2D);

     glBindTexture(GL_TEXTURE_2D,pixelsRenderedImage);

     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

     glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,0,0,width,height,0);

}

void GLANN::timerEvent(QTimerEvent *)
{
    // Update scene
    update();
}

void GLANN::initTextures(){
    // Load cube.png image
    glEnable(GL_TEXTURE_2D);

    //FEEDBACK Texture
    renderedImageUCHAR = new unsigned char [width*height*4];

    //Bind WeightmapTexture
    pixelsScene = QGLWidget::bindTexture(*SceneImage);

    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void GLANN::initShader(){

    // Compile vertex shader
    if (!program.addShaderFromSourceFile(QGLShader::Vertex, ":/vshader.glsl"))
        close();

    // Compile fragment shader
    if (!program.addShaderFromSourceFile(QGLShader::Fragment, ":/fshader.glsl"))
        close();

    // Link shader pipeline
    if (!program.link())
        close();

    // Bind shader pipeline for use
    if (!program.bind())
        close();

    // Use texture unit 0
    program.setUniformValue("CalculatedImage",0);

    // Use texture unit 1
    program.setUniformValue("Objects",1);

    // Set number of Objects
    program.setUniformValue("numObjects",numObjects);

    //width
    program.setUniformValue("width", width);

    //height
    program.setUniformValue("height", height);

    float period = 0.0;
    program.setUniformValue("period",period);
}

void GLANN::getFeedbackTexture(){

    //Playground TexImage(thisSize.width(), thisSize.height());
    glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,renderedImageUCHAR);

    //qDebug("%i , %i" ,TexImage->size().width(),TexImage->size().height());

    for(int i = 0; i < width*height; i++){
        renderedImage->setPixel(i%width,(int)(width*height-i-1)/height,
                               qRgba(renderedImageUCHAR[i*4],
                                     renderedImageUCHAR[i*4+1],
                                     renderedImageUCHAR[i*4+2],
                                     renderedImageUCHAR[i*4+3]));
    }
}
