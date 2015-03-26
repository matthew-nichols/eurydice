#ifndef MTNDINO_SDL_HEADER
#define MTNDINO_SDL_HEADER

#include <SDL2/SDL.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <ostream>
#include <type_traits>

namespace sdl {
	class Texture;

	static_assert(std::is_same<std::uint8_t, Uint8>::value, "");
	static_assert(std::is_same<std::uint32_t, Uint32>::value, "");

	class SDL {
	public:
		SDL(std::uint32_t flags) {
			if (SDL_Init(flags))
				throw SDL_GetError(); // TODO: exception
		}

		~SDL() {
			SDL_Quit();
		}

		SDL(const SDL&) = delete;
		SDL(SDL&&) = delete;
		SDL& operator=(const SDL&) = delete;
		SDL& operator=(SDL&&) = delete;
	};

	class Window {
		SDL_Window* window_;
	public:
		Window(const char* title, int x, int y, int w, int h, std::uint32_t flags = 0) {
			window_ = SDL_CreateWindow(title, x, y, w, h, flags);
			if (!window_) throw SDL_GetError();
		}

		Window(const char* title, int w, int h, std::uint32_t flags = 0)
		: Window{title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, flags} { }

		Window(const Window&) = delete;
		Window(Window&& win): window_{win.window_} { win.window_ = nullptr; }
		Window& operator=(const Window&) = delete;
		Window& operator=(Window&& win) { std::swap(window_, win.window_); return *this; }

		~Window() { if (window_) SDL_DestroyWindow(window_); } // TODO: SDL documentation is remarkably sparse on whether SDL_DestroyWindow allows nullptr
		SDL_Window* get() { return window_; }

		void Raise() { SDL_RaiseWindow(window_); }
	};

	class Renderer {
		SDL_Renderer* renderer_;
	public:
		Renderer(Window& window, int index = -1, std::uint32_t flags = 0) {
			renderer_ = SDL_CreateRenderer(window.get(), index, flags);
			if (!renderer_) throw SDL_GetError();
		}

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&& r): renderer_{r.renderer_} { r.renderer_ = nullptr; }
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&& r) { std::swap(renderer_, r.renderer_); return *this; }

		~Renderer() { if (renderer_) SDL_DestroyRenderer(renderer_); }
		SDL_Renderer* get() { return renderer_; }

		void Clear() {
			if (SDL_RenderClear(renderer_) < 0) throw SDL_GetError();
		}

		void Present() { SDL_RenderPresent(renderer_); }

		void SetDrawColor(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = SDL_ALPHA_OPAQUE) {
			if (SDL_SetRenderDrawColor(renderer_, r, g, b, a) < 0) throw SDL_GetError();
		}

		void Copy(Texture& tex, const SDL_Rect* src, const SDL_Rect* dst);
		void Copy(Texture& tex, int x, int y);
	};

	class Surface {
		SDL_Surface* surface_;
	public:
		explicit Surface(SDL_Surface* sur): surface_{sur} { }

		Surface(const Surface&) = delete;
		Surface(Surface&& sur): surface_{sur.surface_} { sur.surface_ = nullptr; }
		Surface& operator=(const Surface&) = delete;
		Surface& operator=(Surface&& sur) { std::swap(surface_, sur.surface_); return *this; }

		~Surface() { if (surface_) SDL_FreeSurface(surface_); }
		SDL_Surface* get() const { return surface_; }
	};

	class Texture {
		SDL_Texture* texture_;
	public:
		Texture(Renderer& renderer, std::uint32_t format, int access, int w, int h) {
			texture_ = SDL_CreateTexture(renderer.get(), format, access, w, h);
			if (!texture_) throw SDL_GetError();
		}
		Texture(Renderer& ren, const Surface& sur) {
			texture_ = SDL_CreateTextureFromSurface(ren.get(), sur.get());
			if (!texture_) throw SDL_GetError();
		}

		Texture(const Texture&) = delete;
		Texture(Texture&& tex): texture_{tex.texture_} { tex.texture_ = nullptr; }
		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&& tex) { std::swap(texture_, tex.texture_); return *this; }

		~Texture() { if (texture_) SDL_DestroyTexture(texture_); }
		SDL_Texture* get() { return texture_; }

		void Query(std::uint32_t* format, int* access, int* w, int* h) {
			if (SDL_QueryTexture(texture_, format, access, w, h) < 0)
				throw SDL_GetError();
		}
	};

	inline void Renderer::Copy(Texture& tex, const SDL_Rect* src, const SDL_Rect* dst) {
		if (SDL_RenderCopy(renderer_, tex.get(), src, dst) < 0)
			throw SDL_GetError();
	}

	inline void Renderer::Copy(Texture& tex, int x, int y) {
		SDL_Rect dst = {x, y, 0, 0};
		tex.Query(nullptr, nullptr, &dst.w, &dst.h);
		Copy(tex, nullptr, &dst);
	}
}

	#if !defined(__has_include) || (defined(__has_include) && __has_include(<SDL2/SDL_image.h>))
	#include <SDL2/SDL_image.h>
namespace sdl {
	namespace img {
		class IMG {
		public:
			IMG(int flags) {
				int ret = IMG_Init(flags);
				if ((ret & flags) != flags)
					throw IMG_GetError();
			}
			~IMG() { IMG_Quit(); }
		};

		inline Surface Load(const char* file) {
			SDL_Surface* sur = IMG_Load(file);
			if (!sur) throw IMG_GetError();
			return Surface{sur};
		}
	}
}
	#endif

inline std::ostream& operator<<(std::ostream& out, SDL_version ver) {
	out << static_cast<int>(ver.major) << "." << static_cast<int>(ver.minor) << "." << static_cast<int>(ver.patch);
	return out;
}

template<class Rep, class Period>
void SDL_Delay(std::chrono::duration<Rep, Period> duration) {
	SDL_Delay(static_cast<std::uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()));
}

#endif
