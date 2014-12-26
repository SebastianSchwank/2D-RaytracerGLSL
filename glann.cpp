#include "glann.h"


GLANN::GLANN(unsigned int width, unsigned int height, unsigned int renderPasses, Scene *renderScene,
             QWidget *parent, QGLWidget *shareWidget)
      : QGLWidget(parent, shareWidget)
{
    setFixedSize(width,height);
    this->width = width;
    this->height = height;

    qsrand((uint)QTime::currentTime().msec());

    mScene = renderScene;
    SceneImage = renderScene->getSceneImage();
    numObjects = SceneImage->width();
    renderedImage = new Playground(width);
}

void GLANN::initializeGL(){
    setAutoBufferSwap(true);

    this->makeCurrent();
    initializeGLFunctions();

    // Generate 2 VBOs
    glGenBuffers(1, &vboId0);
    glGenBuffers(1, &vboId1);

    initShader();
    initTextures();

    VertexData vertices[] = {
                 // Vertex data for face 0
                 {QVector3D(-1.0, -1.0,  1.0), QVector2D(0.0, 0.0)},  // v0
                 {QVector3D( 1.0, -1.0,  1.0), QVector2D(1.0, 0.0)}, // v1
                 {QVector3D(-1.0,  1.0,  1.0), QVector2D(1.0, 1.0)},  // v2
                 {QVector3D( 1.0,  1.0,  1.0), QVector2D(0.0, 1.0)}, // v3
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

void GLANN::mouseMoveEvent(QMouseEvent* event){
    if(event->buttons() == Qt::LeftButton){
        LineObject newPoly(xTemp,yTemp, 1.0f*event->pos().x()/width, 1.0f-1.0f*event->pos().y()/width,qRgba(255,255,255,255),0.4,0.5,0.3,0.0);
        mScene->addObject(newPoly);

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

    // Set random seed
    program.setUniformValue("seed", ((float)qrand()/RAND_MAX));

    //Set number of alredy rendered passes
    program.setUniformValue("numRenderPass",mRenderPasses);

    //Bind last rendered Image
    //pixelsRenderedImage = bindTexture(*renderedImage);

    //Load Identity
    //glLoadIdentity();

    //Move to rendering point
    //glTranslatef( -1.0, -1.0, 0.0f );

    // Draw geometry
    // Tell OpenGL which VBOs to use

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

         glEnable(GL_TEXTURE_2D);

         glActiveTexture(GL_TEXTURE1);
         glBindTexture(GL_TEXTURE_2D, pixelsScene);

         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D, pixelsRenderedImage);

         // Draw cube geometry using indices from VBO 1
         glDrawElements(GL_TRIANGLE_STRIP, 5, GL_UNSIGNED_SHORT, 0);


     //Get the rendered Image as Texure
     //glReadBuffer(GL_BACK);
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
