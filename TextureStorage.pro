TEMPLATE = app

HEADERS = GLWidget.h \
          Window.h
SOURCES = GLWidget.cpp \
          Window.cpp

RESOURCES     = textures.qrc
QT           += opengl widgets


app {
  TARGET = TextureStorage
  SOURCES += main.cpp
}

test {
  TARGET = Test
  SOURCES += test.cpp
}
