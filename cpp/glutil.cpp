#include <iostream>
#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "kosongg/glutil.hpp"

namespace kosongg {

// CheckGLError
void CheckGLError(const char *file, int line)
{
	GLenum err;
	while((err = glGetError()) != GL_NO_ERROR)
	{
		std::string error;

		switch(err) {

			case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
			case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
			case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
			case GL_STACK_OVERFLOW:         error="STACK_OVERFLOW";         break;
			case GL_STACK_UNDERFLOW:        error="STACK_UNDERFLOW";        break;
			case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
			case GL_CONTEXT_LOST:           error="GL_CONTEXT_LOST";        break;
			//case GL_TABLE_TOO_LARGE:        error="GL_TABLE_TOO_LARGE";     break;
		}

		std::cerr << "GL_" << error.c_str() <<" - "<<file<<":"<<line << std::endl;
		assert(err == GL_NO_ERROR);
	}
}

// OnGLMessageCallback
void GLAPIENTRY OnGLMessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	(void)source;
	(void)length;
	(void)userParam;

	std::cout << "---------------------opengl-callback-start------------" << std::endl;
	std::cout << "message: " << message << std::endl;
	std::cout << "type: ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		std::cout << "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		std::cout << "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		std::cout << "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		std::cout << "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		std::cout << "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER:
		std::cout << "OTHER";
		break;
	default:
		std::cout << "0x" << std::hex << type;
	}
	std::cout << std::endl;

	std::cout << "id: 0x" << std::hex << id << std::endl;
	std::cout << "severity: ";
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:
		std::cout << "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		std::cout << "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		std::cout << "HIGH";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		std::cout << "NOTIFICATION";
		break;
	default:
		std::cout << "0x" << std::hex << severity;
	}
	std::cout << std::endl;
	std::cout << "---------------------opengl-callback-end--------------" << std::endl;
}

// LoadTextureFromFile
// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, unsigned int* out_texture, int* out_width, int* out_height)
{
		// Load from file
		int image_width = 0;
		int image_height = 0;
		unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
		if (image_data == NULL) {
				std::cerr << "Error LoadTextureFromFile: " << filename << std::endl;
				return false;
		}

		// Create a OpenGL texture identifier
		GLuint image_texture;
		glGenTextures(1, &image_texture);
		glBindTexture(GL_TEXTURE_2D, image_texture);

		// Setup filtering parameters for display
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

		// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
		stbi_image_free(image_data);

		*out_texture = (unsigned int)image_texture;
		*out_width = image_width;
		*out_height = image_height;

		return true;
}


}