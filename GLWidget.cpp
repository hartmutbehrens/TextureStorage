/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include <QtOpenGL>
#include <QOpenGLContext>

#include "GLWidget.h"

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_TEXCOORD_ATTRIBUTE 1

GLWidget::GLWidget(const QString& texturePath, QWidget *parent)
  : QOpenGLWidget(parent),
    clearColor(Qt::black),
    xRot(0),
    yRot(0),
    zRot(0),
    rotIndex(0),
    program(0),
    imageTexture(0),
    storageTexture(0),
    texPath(texturePath)
{
}

GLWidget::~GLWidget()
{
  makeCurrent();
  texCoordBuffer.destroy();
  vertexBuffer.destroy();
  vao.destroy();
  delete imageTexture;
  delete storageTexture;
  delete program;
  doneCurrent();
}

QSize GLWidget::minimumSizeHint() const
{
  return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
  return QSize(200, 200);
}

void GLWidget::rotateBy(int xAngle, int yAngle, int zAngle)
{
  xRot += xAngle;
  yRot += yAngle;
  zRot += zAngle;
  update();
}

void GLWidget::setClearColor(const QColor &color)
{
  clearColor = color;
  update();
}

void GLWidget::initializeGL()
{
  initializeOpenGLFunctions();

  makeObject();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_STENCIL_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glViewport(0, 0, width(), height());

  QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
  QString vsrc =
      "in vec4 vertex;\n"
      "in vec2 texCoord;\n"
      "out vec2 texc;\n"
      "uniform int rotIndex;\n"
      "uniform sampler2D storage;\n"
      "mediump mat4 getRotationMatrix(int index);\n"
      "\n"
      "mediump mat4 getRotationMatrix(int index)  { return mat4(texelFetch(storage, ivec2(0,index), 0), texelFetch(storage, ivec2(1,index), 0), texelFetch(storage, ivec2(2,index), 0), texelFetch(storage, ivec2(3,index), 0)); }\n"
      "\n"
      "void main(void)\n"
      "{\n"
      "    gl_Position = getRotationMatrix(rotIndex) * vertex;\n"
      "    texc = texCoord;\n"
      "}\n";

  QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
  QString fsrc =
      "#ifdef GL_ES\n"
      "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
      "precision highp float;\n"
      "#else\n"
      "precision mediump float;\n"
      "#endif\n"
      "#endif\n"
      "in vec2 texc;\n"
      "out vec4 fragColor;\n"
      "uniform sampler2D tex;\n"
      "void main(void)\n"
      "{\n"
      "    fragColor = texture(tex, texc);\n"
      "}\n";

  if (QOpenGLContext::currentContext()->isOpenGLES()) {
    vsrc.prepend(QByteArrayLiteral("#version 300 es\n"));
    fsrc.prepend(QByteArrayLiteral("#version 300 es\n"));
  }
  else {
    vsrc.prepend(QByteArrayLiteral("#version 150\n"));
    fsrc.prepend(QByteArrayLiteral("#version 150\n"));

  }
  vshader->compileSourceCode(vsrc);
  fshader->compileSourceCode(fsrc);

  program = new QOpenGLShaderProgram(this);
  program->addShader(vshader);
  program->addShader(fshader);
  program->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
  program->bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);
  program->link();

  program->bind();

  //create buffers
  vertexBuffer.create();
  vertexBuffer.bind();
  vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
  vertexBuffer.write(0, vertices.constData(), vertices.size() * sizeof(QVector3D));

  texCoordBuffer.create();
  texCoordBuffer.bind();
  texCoordBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
  texCoordBuffer.write(0, texCoords.constData(), texCoords.size() * sizeof(QVector3D));

  //create VAO
  vao.create();
  vao.bind();

  program->enableAttributeArray(program->attributeLocation("vertex"));
  program->enableAttributeArray(program->attributeLocation("texCoord"));
  program->setAttributeArray(program->attributeLocation("vertex"), vertices.constData());
  program->setAttributeArray(program->attributeLocation("texCoord"), texCoords.constData());

  //bind the uniform samplers to texture units
  program->setUniformValue("tex", 0);
  program->setUniformValue("storage", 1);

  //create texture used for storage
  storageTexture->bind();
  //create the storage for the texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 2, 0, GL_RGBA, GL_FLOAT, NULL);
  storageTexture->release();
}

void GLWidget::paintGL()
{
  glClearColor(clearColor.red(), clearColor.green(), clearColor.blue(), clearColor.alpha());
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  QMatrix4x4 m;
  m.ortho(-0.5f, +0.5f, +0.5f, -0.5f, 4.0f, 15.0f);
  m.translate(0.0f, 0.0f, -10.0f);
  m.rotate(xRot / 16.0f, 1.0f, 0.0f, 0.0f);
  m.rotate(yRot / 16.0f, 0.0f, 1.0f, 0.0f);
  m.rotate(zRot / 16.0f, 0.0f, 0.0f, 1.0f);

  QMatrix4x4 n = m;
  n.scale(0.5, 0.5, 0.5);

  glActiveTexture(GL_TEXTURE1); // Texture unit 1
  storageTexture->bind(GL_TEXTURE1);

  //update storage texture
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 1, GL_RGBA, GL_FLOAT, m.constData());
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 1, 4, 1, GL_RGBA, GL_FLOAT, n.constData());

  program->setUniformValue("rotIndex", rotIndex);

  glActiveTexture(GL_TEXTURE0); // Texture unit 0
  imageTexture->bind(GL_TEXTURE0);
  for (int i = 0; i < 6; ++i) {
    glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
  }
}

void GLWidget::toggleRotationIndex()
{
  rotIndex = (rotIndex == 0) ? 1 : 0;
  update();
}

void GLWidget::resizeGL(int width, int height)
{
  int side = qMin(width, height);
  glViewport((width - side) / 2, (height - side) / 2, side, side);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
  lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
  int dx = event->x() - lastPos.x();
  int dy = event->y() - lastPos.y();

  if (event->buttons() & Qt::LeftButton) {
    rotateBy(8 * dy, 8 * dx, 0);
  } else if (event->buttons() & Qt::RightButton) {
    rotateBy(8 * dy, 0, 8 * dx);
  }
  lastPos = event->pos();
}

void GLWidget::mouseReleaseEvent(QMouseEvent * /* event */)
{
  emit clicked();
}

void GLWidget::makeObject()
{
  static const int coords[6][4][3] = {
    { { +1, -1, -1 }, { -1, -1, -1 }, { -1, +1, -1 }, { +1, +1, -1 } },
    { { +1, +1, -1 }, { -1, +1, -1 }, { -1, +1, +1 }, { +1, +1, +1 } },
    { { +1, -1, +1 }, { +1, -1, -1 }, { +1, +1, -1 }, { +1, +1, +1 } },
    { { -1, -1, -1 }, { -1, -1, +1 }, { -1, +1, +1 }, { -1, +1, -1 } },
    { { +1, -1, +1 }, { -1, -1, +1 }, { -1, -1, -1 }, { +1, -1, -1 } },
    { { -1, -1, +1 }, { +1, -1, +1 }, { +1, +1, +1 }, { -1, +1, +1 } }
  };

  imageTexture = new QOpenGLTexture(QImage(texPath).mirrored());
  imageTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
  imageTexture->setMagnificationFilter(QOpenGLTexture::Linear);

  storageTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
  //we're using this texture as storage, so do not want mipmapping
  storageTexture->setMinificationFilter(QOpenGLTexture::Nearest);
  storageTexture->setMagnificationFilter(QOpenGLTexture::Nearest);

  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 4; ++j) {
      texCoords.append(QVector2D(j == 0 || j == 3, j == 0 || j == 1));
      vertices.append(QVector3D(0.2 * coords[i][j][0], 0.2 * coords[i][j][1], 0.2 * coords[i][j][2]));
    }
  }
}
