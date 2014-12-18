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
    // Use QBasicTimer because its faster than QTimer
    timer.start(0, this);
}

void GLANN::resizeGL(int w, int h){
    glViewport(0,0,w,h);
}

void GLANN::paintGL(){
    render();
    //getFeedbackTexture();
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

    // Set nearest filtering mode for texture minification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Set bilinear filtering mode for texture magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
    program.setUniformValue("Objects",0);

    // Use texture unit 1
    program.setUniformValue("CalculatedImage",1);

    // Set number of Objects
    program.setUniformValue("numObjects",numObjects);

    // Use texture unit 0 which contains cube.png
    program.setUniformValue("width", width);

    // Use texture unit 0 which contains cube.png
    program.setUniformValue("height", height);
}

void GLANN::render(){
    // Set random seed
    program.setUniformValue("seed", ((float)qrand()/RAND_MAX));

    //Set number of alredy rendered passes
    program.setUniformValue("numRenderPass",mRenderPasses);

    //Bind last rendered Image
    pixelsRenderedImage = bindTexture(*renderedImage);

    //Load Identity
    glLoadIdentity();

    //Move to rendering point
    glTranslatef( -1.0, -1.0, 0.0f );

    glEnable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pixelsScene);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pixelsRenderedImage);

    // Draw geometry
    //Render textured quad
    glBegin( GL_QUADS );
        glTexCoord2f( 0.f, 0.f ); glVertex2f( 0, 0);
        glTexCoord2f( 1.f, 0.f ); glVertex2f( 2.0, 0);
        glTexCoord2f( 1.f, 1.f ); glVertex2f( 2.0, 2.0);
        glTexCoord2f( 0.f, 1.f ); glVertex2f( 0, 2.0);
     glEnd();

     //increment number of rendered passes
     mRenderPasses++;
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
