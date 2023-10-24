#pragma once

#include <stdint.h>
#include <vector>

const int DEFAULT_WIDTH  = 800;
const int DEFAULT_HEIGHT = 600;

struct GLFWwindow;

namespace vk
{
	class SurfaceKHR;
	struct Extent2D;
} // namespace vk

namespace W3D
{
class Renderer;
class Instance;

/*
* Window - this class serves as a wrapper class for our GLFW window, which
* will provide the surface where we'll render. It is also responsible for
* polling the window for events, and when they do occur, marshalling relevant
* event data and sending that event information onto the renderer's process_event
* function for a programmed response.
*/
class Window
{
  private:
	// HANDLE FOR THE GLFW LIBRARY WINDOW, WHICH IS WHERE WE'LL
	// RENDER OUR SCENE
	GLFWwindow *handle_;

  public:
	// OUR INITIAL WINDOW SIZE
	static const int DEFAULT_WINDOW_WIDTH;
	static const int DEFAULT_WINDOW_HEIGHT;
	static void      push_required_extensions(std::vector<const char *> &extensions);

	// CONSTRUCTOR/DESTRUCTOR
	/*
	 * Constructor that initializes a GLFW window and gets its handle. For
	 * more on this API see https://www.glfw.org/
	 */
	Window(const char *title, int width = DEFAULT_WIDTH,
	       int height = DEFAULT_HEIGHT);
	/*
	 * Destructor cleans up the window now that we're not using it anymore.
	 */
	~Window();

	// ACCESSOR METHODS
	/*
	 * Accessor method for getting the size of the window.
	 */
	vk::Extent2D get_extent() const;

	/*
	 * Accessor method for getting the GLFW window.
	 */
	GLFWwindow    *get_handle();

	/*
	 * Proxy method for a GLFW function, it gets a flag from the GLFW window
	 * that specifies if this window is closing or not.
	 */
	bool           should_close();

	/*
	 * This function creates and returns a GLFW surface that we can render
	 * to using Vulkan.
	 */
	vk::SurfaceKHR create_surface(Instance &instance);

	/*
	 * A proxy for a glfw function, it retrieves events from the window that have
	 * happened since the last time we polled it.
	 */
	void poll_events();

	/*
	 * A proxy for a glfw function, it waits until events are queued and processes them.
	 */
	void wait_events();

	/*
	 * Specifies which of our functions in this class will respond
	 * when the user interacts with the window.
	 */
	void register_callbacks(Renderer &renderer);

	/*
	 * Tests the framebuffer size for this window and if it
	 * is not set, will wait until it is set. Once set, it will return the window surface
	 * dimensions, i.e. the size of its framebuffer.
	 */
	vk::Extent2D wait_for_non_zero_extent();

};	// class Window

}	// namespace W3D