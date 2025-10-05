#include "resources.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

SDL_Texture *Resources::loadTexture(SDL_Renderer *renderer, const std::string &filepath)
{
	// get pixel data and image info
	int width = 0;
	int height = 0;
	int channels = 0;
	stbi_uc *pixData = stbi_load(filepath.c_str(), &width, &height, &channels, 4);

	// create surface, copy copy into texture
	SDL_Surface *surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA32, pixData, width * 4);
	SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
	textures.push_back(tex);
	SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);

	// free STB pixel data first, then destroy surface
	stbi_image_free(pixData);
	SDL_DestroySurface(surface);

	return tex;
}
