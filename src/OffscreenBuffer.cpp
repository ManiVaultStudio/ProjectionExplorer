#include "OffscreenBuffer.h"

OffscreenBuffer::OffscreenBuffer() :
    _context(nullptr)
{
    setSurfaceType(QWindow::OpenGLSurface);

    create();
}

void OffscreenBuffer::initialize()
{
    QOpenGLContext* globalContext = QOpenGLContext::globalShareContext();
    _context = new QOpenGLContext(this);
    _context->setFormat(globalContext->format());

    if (!_context->create())
        qFatal("Cannot create requested OpenGL context.");

    _context->makeCurrent(this);
}

void OffscreenBuffer::bindContext()
{
    _context->makeCurrent(this);
}

void OffscreenBuffer::releaseContext()
{
    _context->doneCurrent();
}
