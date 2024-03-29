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

#include "GLWidget.h"
#include "Window.h"

Window::Window()
  : previousGlWidget(0),
    rotationSpeed(2)
{
  QGridLayout *mainLayout = new QGridLayout;
  QStringList texturePaths = QStringList() << QString(":/images/side1.png") << QString(":/images/side2.png") << QString(":/images/side3.png") << QString(":/images/side4.png") << QString(":/images/side5.png") << QString(":/images/side6.png");

  int c = 0;
  for (int i = 0; i < NumRows; ++i) {
    for (int j = 0; j < NumColumns; ++j) {
      QColor clearColor;
      clearColor.setHsv(((i * NumColumns) + j) * 255 / (NumRows * NumColumns - 1), 255, 63);

      glWidgets[i][j] = new GLWidget(texturePaths[c]);
      glWidgets[i][j]->setClearColor(clearColor);
      mainLayout->addWidget(glWidgets[i][j], i, j);

      connect(glWidgets[i][j], SIGNAL(clicked()), this, SLOT(setCurrentGlWidget()));
      c++; // YES :)
    }
  }
  setLayout(mainLayout);

  currentGlWidget = glWidgets[0][0];

  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(rotateOneStep()));
  timer->start(20);

  setWindowTitle(tr("TextureStorage"));
}

void Window::setCurrentGlWidget()
{
  previousGlWidget = currentGlWidget;
  currentGlWidget = qobject_cast<GLWidget *>(sender());

  if (currentGlWidget == previousGlWidget) {
    currentGlWidget->toggleRotationIndex();
    rotationSpeed = (rotationSpeed == 2) ? 8 : 2;
  }
  else {
    rotationSpeed = 2;
  }
}

void Window::rotateOneStep()
{
  if (currentGlWidget) {
    currentGlWidget->rotateBy(rotationSpeed * 16, rotationSpeed * 16, -1 * rotationSpeed * 16);
  }
}
