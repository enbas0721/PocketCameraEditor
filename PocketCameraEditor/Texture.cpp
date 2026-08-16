// Texture.cpp
#include "Texture.h"
#include <cassert>

Texture::~Texture()
{
    glDeleteTextures(1, &m_textureId);
}

Texture::Texture(Texture&& other) noexcept
    : m_textureId(other.m_textureId)
{
    other.m_textureId = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other)
    {
        glDeleteTextures(1, &m_textureId);
        m_textureId = other.m_textureId;
        other.m_textureId = 0;
    }
    return *this;
}

void Texture::Upload(std::span<const uint8_t> pixels, int width, int height)
{
    assert(pixels.size() == static_cast<size_t>(width * height * 4)); // Ensure the pixel data matches the expected size for RGBA.

    if (m_textureId == 0)
    {
        glGenTextures(1, &m_textureId);
    }

    // Bind the texture with the given texture ID. This makes the texture active for subsequent operations.
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}