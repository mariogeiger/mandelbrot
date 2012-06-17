#include "mandelbrot.h"
#include <cmath>

Mandelbrot::Mandelbrot(QWidget *parent)
    : QGLWidget(parent)
{
    _scale = 1.0;

    _timer.setSingleShot(true);
    connect(&_timer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
}

Mandelbrot::~Mandelbrot()
{
}

void Mandelbrot::initializeGL()
{
    makeCurrent();

    _shader = new QGLShaderProgram(this);
    _shader->addShaderFromSourceFile(QGLShader::Vertex, ":/mandelbrot.vert");
    _shader->addShaderFromSourceFile(QGLShader::Fragment, ":/mandelbrot.frag");
    _shader->bindAttributeLocation("vertex", 0);
    _shader->link();
    _shader->bind();
    _centerLocation = _shader->uniformLocation("center");
    _scaleLocation = _shader->uniformLocation("scale");
    _ratioLocation = _shader->uniformLocation("ratio");
    _accuracyLocation = _shader->uniformLocation("accuracy");

    QVector4D colormap[256];
    for (int i = 0; i < 256; ++i) {
        colormap[i] = rgbFromWaveLength(380.0 + (i * 400.0 / 256.0));
    }
    _shader->setUniformValueArray("colormap", colormap, 256);
}

void Mandelbrot::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    _shader->setUniformValue(_ratioLocation, GLfloat(w) / GLfloat(h ? h : 1));
    _shader->setUniformValue(_accuracyLocation, 0);
    _timer.start(1000);
}

void Mandelbrot::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    static const GLfloat vertices[] = {
        -1.0,  +1.0,
        -1.0,  -1.0,
        +1.0,  -1.0,
        +1.0,  +1.0
    };
    _shader->setAttributeArray(0, vertices, 2);

    _shader->setUniformValue(_centerLocation, _center);
    _shader->setUniformValue(_scaleLocation, _scale);

    _shader->enableAttributeArray(0);
    glDrawArrays(GL_QUADS, 0, 4);
    _shader->disableAttributeArray(0);
}

#include <QMouseEvent>
void Mandelbrot::mousePressEvent(QMouseEvent *e)
{
    QGLWidget::mousePressEvent(e);

    _mouseposition = e->posF();
}

void Mandelbrot::mouseMoveEvent(QMouseEvent *e)
{
    QGLWidget::mouseMoveEvent(e);

    if (e->buttons() & Qt::LeftButton) {
        QPointF d = e->posF() - _mouseposition;
        d.ry() = -d.y();
        d /= qreal(height());
        d *= _scale * 2.0;

        _center -= d;

        _shader->setUniformValue(_accuracyLocation, 0);
        updateGL();
        _timer.start(500);
    }
    _mouseposition = e->posF();
}

#include <QWheelEvent>
void Mandelbrot::wheelEvent(QWheelEvent *e)
{
    QGLWidget::wheelEvent(e);

    GLfloat k = std::pow(0.9999f, e->delta());
    _scale *= k;

    _shader->setUniformValue(_accuracyLocation, 0);
    updateGL();
    _timer.start(300);
}

void Mandelbrot::timerTimeout()
{
    _shader->setUniformValue(_accuracyLocation, 4);
    updateGL();
}

QVector4D Mandelbrot::rgbFromWaveLength(double wave)
{
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;

    if (wave >= 380.0 && wave <= 440.0) {
        r = -1.0 * (wave - 440.0) / (440.0 - 380.0);
        b = 1.0;
    } else if (wave >= 440.0 && wave <= 490.0) {
        g = (wave - 440.0) / (490.0 - 440.0);
        b = 1.0;
    } else if (wave >= 490.0 && wave <= 510.0) {
        g = 1.0;
        b = -1.0 * (wave - 510.0) / (510.0 - 490.0);
    } else if (wave >= 510.0 && wave <= 580.0) {
        r = (wave - 510.0) / (580.0 - 510.0);
        g = 1.0;
    } else if (wave >= 580.0 && wave <= 645.0) {
        r = 1.0;
        g = -1.0 * (wave - 645.0) / (645.0 - 580.0);
    } else if (wave >= 645.0 && wave <= 780.0) {
        r = 1.0;
    }

    double s = 1.0;
    if (wave > 700.0)
        s = 0.3 + 0.7 * (780.0 - wave) / (780.0 - 700.0);
    else if (wave <  420.0)
        s = 0.3 + 0.7 * (wave - 380.0) / (420.0 - 380.0);

    r = std::pow(r * s, 0.8);
    g = std::pow(g * s, 0.8);
    b = std::pow(b * s, 0.8);
    return QVector4D(r, g, b, 1.0);
}