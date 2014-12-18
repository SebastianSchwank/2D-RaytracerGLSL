#ifndef GLANN_H
#define GLANN_H

#pragma once

#include <QGLShaderProgram>
#include <QGLFunctions>
#include <QOpenGLFramebufferObject>
#include <QGLPixelBuffer>

#include <QBasicTimer>
#include <QTime>
#include <QImage>
#include <QVector>

#include "scene.h"
#include "playground.h"
#include "Floatbits.h"

class GLANN : public QGLWidget,protected QGLFunctions
{
    Q_OBJECT

public:
    GLANN(unsigned int width, unsigned int height, unsigned int renderPasses, Scene *renderScene = 0);

    QImage getRenderedImage();

protected:

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void timerEvent(QTimerEvent *);

private:
    void render();

    void getFeedbackTexture();

    void initShader();
    void initTextures();

    unsigned int width;
    unsigned int height;

    QBasicTimer timer;

    QImage *SceneImage;
    QImage *renderedImage;

    QGLShaderProgram program;

    GLuint pixelsRenderedImage;
    GLuint pixelsScene;
    unsigned char *renderedImageUCHAR;

    unsigned int numObjects;
    unsigned int mRenderPasses = 0;
};

#endif // GLANN_H
