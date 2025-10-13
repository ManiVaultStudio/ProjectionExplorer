#include "LocalValueComputation.h"

#include "graphics/Matrix3f.h"
#include "graphics/Vector2f.h"
#include "graphics/Bounds.h"

#include "util/Timer.h"

#include <QImage>

void LocalValueComputation::initialize()
{
    qDebug() << "LocalValueComputation::initialize()";
    _offscreenBuffer = new OffscreenBuffer();
    _offscreenBuffer->initialize();

    initializeOpenGLFunctions();

    //_splatTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    //_splatTexture->bind();

    bool loaded = true;
    loaded &= _splatProgram.loadShaderFromFile(":projection_explorer/shaders/Splat.vert", ":projection_explorer/shaders/Splat.frag");

    if (!loaded) {
        qCritical() << "Failed to load one of the Projection Explorer shaders";
    }

    glGenFramebuffers(1, &_splatFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, _splatFBO);

    glGenTextures(1, &_splatTexture);
    glBindTexture(GL_TEXTURE_2D, _splatTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1024, 1024, 0, GL_RGBA, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _splatTexture, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    std::vector<float> vertices(
        {
            -1, -1,
            1, -1,
            -1, 1,
            -1, 1,
            1, -1,
            1, 1
        }
    );

    // Vertex buffer
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    
    glGenBuffers(1, &_pbo);
    glBindBuffer(GL_ARRAY_BUFFER, _pbo);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(1);

    // Value buffer
    glGenBuffers(1, &_vvbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vvbo);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(2);

    glClearColor(0, 0, 0, 1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    _offscreenBuffer->releaseContext();
}

/**
* Builds an orthographic projection matrix that transforms the given bounds
* to the range [-1, 1] in both directions.
*/
mv::Matrix3f createProjectionMatrix(const mv::Bounds& bounds)
{
    mv::Matrix3f m;
    m.setIdentity();
    m[0] = 2 / bounds.getWidth();
    m[4] = 2 / bounds.getHeight();
    m[6] = -((bounds.getRight() + bounds.getLeft()) / bounds.getWidth());
    m[7] = -((bounds.getTop() + bounds.getBottom()) / bounds.getHeight());
    return m;
}

/** Find the rectangular bounds of the 2D dataset provided */
mv::Bounds getDataBounds(DataMatrix& projection)
{
    mv::Bounds bounds = mv::Bounds::Max;

    for (int i = 0; i < projection.getNumRows(); i++)
    {
        bounds.setLeft(std::min(projection(i, 0), bounds.getLeft()));
        bounds.setRight(std::max(projection(i, 0), bounds.getRight()));
        bounds.setBottom(std::min(projection(i, 1), bounds.getBottom()));
        bounds.setTop(std::max(projection(i, 1), bounds.getTop()));
    }

    return bounds;
}

void LocalValueComputation::splatValues(DataMatrix& dataset, DataMatrix& projection, std::vector<std::vector<float>>& localValues, float splatSize)
{
    qDebug() << "LocalValueComputation::splatValues()";
    _offscreenBuffer->bindContext();
    //glViewport();

    //glActiveTexture()?

    mv::Bounds bounds = getDataBounds(projection);
    bounds.ensureMinimumSize(0.01, 0.01);
    bounds.makeSquare();
    bounds.expand(0.1);

    mv::Matrix3f orthoM = createProjectionMatrix(bounds);
    mv::Matrix3f clipToTextureSpace;
    clipToTextureSpace[0] = 1024 * 0.5;
    clipToTextureSpace[4] = 1024 * 0.5;
    clipToTextureSpace[6] = 1024 * 0.5;
    clipToTextureSpace[7] = 1024 * 0.5;

    glBindFramebuffer(GL_FRAMEBUFFER, _splatFBO);

    _splatProgram.bind();
    _splatProgram.uniformMatrix3f("projMatrix", orthoM);
    _splatProgram.uniform1f("splatSize", splatSize);

    glViewport(0, 0, 1024, 1024);

    glBindVertexArray(_vao);

    // Fill GPU data buffers
    std::vector<float> positions(projection.getNumRows() * 2);
    for (int i = 0; i < projection.getNumRows(); i++)
    {
        positions[i * 2 + 0] = projection(i, 0);
        positions[i * 2 + 1] = projection(i, 1);
    }
    glBindBuffer(GL_ARRAY_BUFFER, _pbo);
    glBufferData(GL_ARRAY_BUFFER, projection.getNumRows() * 2 * sizeof(float), positions.data(), GL_STATIC_DRAW);

    // Value buffer
    glBindBuffer(GL_ARRAY_BUFFER, _vvbo);
    glBufferData(GL_ARRAY_BUFFER, projection.getNumRows() * 3 * sizeof(float), nullptr, GL_STATIC_DRAW);

    for (int col = 0; col < dataset.getNumCols()-3; col += 3)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        {
            Timer t("Value Splat");

            // Set value buffer
            std::vector<float> values(projection.getNumRows() * 3);
            for (int i = 0; i < projection.getNumRows(); i++)
            {
                values[i * 3 + 0] = dataset(i, col + 0);
                values[i * 3 + 1] = dataset(i, col + 1);
                values[i * 3 + 2] = dataset(i, col + 2);
            }
            //qDebug() << values[0] << values[1] << values[2] << values[3] << values[4] << values[5] << values[6];
            glBindBuffer(GL_ARRAY_BUFFER, _vvbo);
            glBufferData(GL_ARRAY_BUFFER, projection.getNumRows() * 3 * sizeof(float), values.data(), GL_STATIC_DRAW);

            Timer vt("Value Splat Draw");

            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)projection.getNumRows());
            glFinish();
        }

        Timer rt("Rest timer");

        // Get back fbo texture and save it
        std::vector<float> pixels(1024 * 1024 * 4);
        glBindTexture(GL_TEXTURE_2D, _splatTexture);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels.data());

        qDebug() << "Pixels: " << pixels[0] << pixels[1] << pixels[2] << pixels[3] << pixels[4] << pixels[5];

        // Get back value at point location
        for (int i = 0; i < projection.getNumRows(); i++)
        {
            mv::Vector2f point(projection(i, 0), projection(i, 1));
            mv::Vector2f pos = (clipToTextureSpace * orthoM * point);
            //qDebug() << "Pos: " << pos.x << pos.y;
            //pixels[(int) pos.y * 1024 * 4 + (int) pos.x * 4 + 1] = 1;

            int index = (int)pos.y * 1024 * 4 + (int)pos.x * 4;
            float mean0 = pixels[index + 0];
            float mean1 = pixels[index + 1];
            float mean2 = pixels[index + 2];

            float numNeighbours = (int)pixels[index + 3];

            localValues[i][col + 0] = mean0 / numNeighbours;
            localValues[i][col + 1] = mean1 / numNeighbours;
            localValues[i][col + 2] = mean2 / numNeighbours;
        }

        //QImage image(1024, 1024, QImage::Format_RGBA8888);
        //for (int y = 0; y < 1024; y++)
        //{
        //    for (int x = 0; x < 1024; x++)
        //    {
        //        QColor c;
        //        c.setRedF(pixels[y * 1024 * 4 + x * 4 + 0] * 1.0 / 100);
        //        c.setGreenF(pixels[y * 1024 * 4 + x * 4 + 1] * 1.0 / 100);
        //        c.setBlueF(pixels[y * 1024 * 4 + x * 4 + 2] * 1.0 / 100);
        //        c.setAlphaF(1);
        //        image.setPixel(x, y, c.rgba());
        //    }
        //}
        //QTransform myTransform;
        //myTransform.rotate(180);
        //image = image.transformed(myTransform);
        //image.save(QString("fbo_out%1.png").arg(col));
    }

    _offscreenBuffer->releaseContext();
    qDebug() << "Finisssshhh";
}
