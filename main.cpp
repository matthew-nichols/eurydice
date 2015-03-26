#include <mtn/sdl.h>
#include <SDL2/SDL_revision.h>

#include <iostream>

using namespace std;

int main() {
	try {
		{
			SDL_version compiled, linked;
			SDL_VERSION(&compiled);
			SDL_GetVersion(&linked);
			cout << "SDL version: compiled " << compiled << " (" << SDL_REVISION << ") , linked " << linked << " (" << SDL_GetRevision() << ")" << endl;
		}
		{
			SDL_version compiled, linked;
			SDL_IMAGE_VERSION(&compiled);
			linked = *IMG_Linked_Version();
			cout << "SDL_image version: compiled " << compiled << ", linked " << linked << endl;
		}

		sdl::SDL SDL{SDL_INIT_VIDEO};
		sdl::Window window{"Hello, world", 800, 600};
		sdl::Renderer renderer{window};
		sdl::Texture texture{renderer, sdl::img::Load("test.png")};
		window.Raise();
		renderer.SetDrawColor(0, 0, 0);
		renderer.Clear();
		renderer.Copy(texture, 16, 16);
		renderer.Present();

		SDL_Delay(5s);
	}
	catch(const char* str) {
		cout << "Caught: " << str << endl;
	}
}
