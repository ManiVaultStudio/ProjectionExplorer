#pragma once

#include "Explanation/DataMatrix.h"
#include "OffscreenBuffer.h"

#include "graphics/Shader.h"

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLTexture>

class LocalValueComputation : protected QOpenGLFunctions_3_3_Core
{
public:
    void initialize(DataMatrix& projection);

    void splatValues(DataMatrix& dataset, DataMatrix& projection, std::vector<std::vector<float>>& localValues);

private:
    //QOpenGLTexture* _splatTexture;

    OffscreenBuffer* _offscreenBuffer;

    mv::ShaderProgram _splatProgram;

    GLuint _splatFBO;
    GLuint _splatTexture;

    GLuint _vao;
    GLuint _pbo;
    GLuint _vvbo;
};
