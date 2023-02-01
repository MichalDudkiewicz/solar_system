#include "Renderer.h"
#include <iostream>

void Renderer::Clear() const
{
        GLCall(glEnable(GL_DEPTH_TEST));
        GLCall(glDepthFunc(GL_LESS));
        GLCall( glClearColor( 0.0, 0.0, 0.0, 1.0 ) );
        GLCall( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) );
}

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const
{
        shader.Bind();
        va.Bind();
        ib.Bind();
        GLCall( glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr) );
}

