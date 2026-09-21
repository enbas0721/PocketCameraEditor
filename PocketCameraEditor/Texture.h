// Texture.h
// A simple wrapper for an OpenGL texture object.
#pragma once

#include <cstdint>
#include <glad/glad.h>
#include <span>

class Texture
{
public:
	Texture() = default;
	~Texture();
	Texture(const Texture&) = delete;               // Disable copy constructor
	Texture& operator=(const Texture&) = delete;    // Disable copy assignment operator
	Texture(Texture&&) noexcept;                    // Move constructor
	Texture& operator=(Texture&&) noexcept;         // Move assignment operator

	void Upload(std::span<const uint8_t> pixels, int width, int height);
	GLuint GetId() const { return m_textureId; }

private:
	GLuint m_textureId = 0;
};