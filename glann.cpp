#include "glann.h"


GLANN::GLANN(unsigned int renderPasses, Scene *renderScene,
             QWidget *parent, QGLWidget *shareWidget)
      : QGLWidget(parent, shareWidget)
{
    QScreen *screen = QApplication::screens().at(0);
    int width = screen->size().width();
    int height = screen->size().height();

    //qDebug() << width << height << "------------------ WIDTH , HEIGHT";

    setFixedWidth(width);
    setFixedHeight(height);
    this->width = width;
    this->height = height;
    this->TexWidth = width;
    this->TexHeight = height;

    qsrand((uint)QTime::currentTime().msec());

    mScene = renderScene;
    SceneImage = renderScene->getSceneImage();
    numObjects = SceneImage->width();
    renderedImage = new Playground(width,height);
}

void GLANN::initializeGL(){
    //setAutoBufferSwap(true);

    makeCurrent();
    initializeGLFunctions();

    //glEnable(GL_CULL_FACE);

    glClearColor(0.0, 0.0, 0.0, 0.0);

    initShader();
    initTextures();

    // Generate 2 VBOs
    glGenBuffers(1, &vboId0);
    glGenBuffers(1, &vboId1);

    VertexData vertices[] = {
        // Vertex data for face 0
                {QVector3D(-1.0, -1.0,  1.0), QVector2D(0.0, 0.0)},  // v0
                {QVector3D( 1.0, -1.0,  1.0), QVector2D(1.0, 0.0)}, // v1
                {QVector3D(-1.0,  1.0,  1.0), QVector2D(0.0, 1.0)},  // v2
                {QVector3D( 1.0,  1.0,  1.0), QVector2D(1.0, 1.0)}, // v3
    };
    // Transfer vertex data to VBO 0
    glBindBuffer(GL_ARRAY_BUFFER, vboId0);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(VertexData), vertices, GL_STATIC_DRAW);

     GLushort indices[] = {
                  0,  1,  2,  3,  3,
    };
    // Transfer index data to VBO 1
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboId1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 5 * sizeof(GLushort), indices, GL_STATIC_DRAW);

    //Init the Framebuffer
    initFbo();

    // Use QBasicTimer because its faster than QTimer
    timer.start(0, this);
}

bool GLANN::initFbo(){
    fbo = new QOpenGLFramebufferObject(TexWidth, TexHeight);
    return true;
}

void GLANN::resizeGL(int w, int h){
    glViewport(0,0,w,h);
}

void GLANN::mouseMoveEvent(QMouseEvent* event){
    if(event->buttons() == Qt::LeftButton){
        LineObject newPoly(xTemp,yTemp, 1.0f*event->pos().x()/width, 1.0f-1.0f*event->pos().y()/height,qRgba(255,0,0,255),0.99,0.0,0.0,0.0);
        mScene->addObject(newPoly);

        delete SceneImage;
        SceneImage = mScene->getSceneImage();
        numObjects = SceneImage->width();

        //Bind WeightmapTexture
        pixelsScene = QGLWidget::bindTexture(*SceneImage);

        qDebug() << xTemp << yTemp << numObjects;

        //Bind last rendered Image
        renderedImage->fill(qRgba(0,0,0,255));
        pixelsRenderedImage = bindTexture(*renderedImage);

        mRenderPasses = 0;

        // Set number of Objects
        program.setUniformValue("numObjects",numObjects);

        xTemp = 1.0f*event->pos().x()/width;
        yTemp = 1.0f-1.0f*event->pos().y()/height;
    }
}

void GLANN::mousePressEvent(QMouseEvent* event){
    if(event->button() == Qt::LeftButton){
            xTemp = 1.0f*event->pos().x()/width;
            yTemp = 1.0f-1.0f*event->pos().y()/height;
        }
}

void GLANN::paintGL(){

    render();

    //getFeedbackTexture();

    //increment number of rendered passes
    mRenderPasses++;
}

void GLANN::render(){

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Set random seed
        program.setUniformValue("seedX", ((float)qrand()/RAND_MAX));
        program.setUniformValue("seedY", ((float)qrand()/RAND_MAX));

        //Set number of alredy rendered passes
        program.setUniformValue("numRenderPass",mRenderPasses);

        //Set program to fbo render mode
        program.setUniformValue("fbo",true);

        //Bind last rendered Image
        //pixelsRenderedImage = bindTexture(*renderedImage);

        //Load Identity
        //glLoadIdentity();

        //Move to rendering point
        //glTranslatef( -1.0, -1.0, 0.0f );

        // Draw geometry
        // Tell OpenGL which VBOs to use

         // Render to our framebuffer
         fbo->bind();
         glViewport(0,0,TexWidth,TexHeight);

         // Tell OpenGL which VBOs to use
         glBindBuffer(GL_ARRAY_BUFFER, vboId0);
         glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboId1);

         // Offset for position
         int offset = 0;

         // Tell OpenGL programmable pipeline how to locate vertex position data
         int vertexLocation = program.attributeLocation("a_position");
         program.enableAttributeArray(vertexLocation);
         glVertexAttribPointer(vertexLocation, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void *)offset);

         // Offset for texture coordinate
         offset += sizeof(QVector3D);

         // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
         int texcoordLocation = program.attributeLocation("a_texcoord");
         program.enableAttributeArray(texcoordLocation);
         glVertexAttribPointer(texcoordLocation, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (const void *)offset);

         //glEnable(GL_TEXTURE_2D);

         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D, pixelsRenderedImage);

         glActiveTexture(GL_TEXTURE1);
         glBindTexture(GL_TEXTURE_2D, pixelsScene);

         glActiveTexture(GL_TEXTURE2);
         glBindTexture(GL_TEXTURE_2D, pixelsRandom);

         // Draw cube geometry using indices from VBO 1
         glDrawElements(GL_TRIANGLE_STRIP, 5, GL_UNSIGNED_SHORT, 0);

//         qDebug() << glGetError() << "Line 183";

         fbo->release();


     pixelsRenderedImage = fbo->texture();
     //Set Program to screen frendering
     program.setUniformValue("fbo",false);
     //Set Viewport back to default
     glViewport(0,0,width,height);
     //Render To Screen
     //glEnable(GL_TEXTURE_2D);
     glActiveTexture(GL_TEXTURE0);
     glBindTexture(GL_TEXTURE_2D, pixelsRenderedImage);

     // Draw quad geometry using indices from VBO 1
     glDrawElements(GL_TRIANGLE_STRIP, 5, GL_UNSIGNED_SHORT, 0);


}

void GLANN::timerEvent(QTimerEvent *)
{
    // Update scene
    update();
}

void GLANN::initTextures(){

    //glEnable(GL_TEXTURE_2D);

    //FEEDBACK Texture
    renderedImageUCHAR = new unsigned char [TexWidth*TexHeight*4];

    //Bind SceneTexture
    pixelsScene = QGLWidget::bindTexture(*SceneImage);

    //Random
    randomImage = new Playground(width,height);
    pixelsRandom = QGLWidget::bindTexture(*randomImage);

    // Poor filtering. Needed !
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
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

    // Use texture unit 2
    program.setUniformValue("random",2);

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
