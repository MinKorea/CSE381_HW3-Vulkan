#include "window.hpp"

#include "common/logging.hpp"
#include "common/utils.hpp"
#include "common/vk_common.hpp"

// THIS IS THE WINDOW LIBRARY
#include "GLFW/glfw3.h"

// OUR OWN TYPES
#include "instance.hpp"
#include "renderer.hpp"
#include "scene_graph/event.hpp"

extern const char *APP_NAME;

namespace W3D
{
// INITIAL WINDOW DIMENSIONS
const int Window::DEFAULT_WINDOW_WIDTH  = 800;
const int Window::DEFAULT_WINDOW_HEIGHT = 600;

Window::Window(const char *title, int width, int height)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	// NOTE, handle_ IS AN INSTANCE VARIABLE
	handle_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

Window::~Window()
{
	glfwDestroyWindow(handle_);
	glfwTerminate();
}

vk::Extent2D Window::get_extent() const
{
	int width, height;
	glfwGetFramebufferSize(handle_, &width, &height);
	return vk::Extent2D{to_u32(width), to_u32(height)};
}

GLFWwindow *Window::get_handle()
{
	return handle_;
}

bool Window::should_close()
{
	return glfwWindowShouldClose(handle_);
}

vk::SurfaceKHR Window::create_surface(Instance &instance)
{
	// THIS IS THE SURFACE WE WILL RENDER TO
	VkSurfaceKHR surface;

	// CREATE THE SURFACE USING THE VULKAN instance
	if (glfwCreateWindowSurface(instance.get_handle(), handle_, nullptr, &surface) != VK_SUCCESS)
	{
		LOGE("Unable to create surface!");
		throw std::runtime_error("Unrecoverable error");
	}

	// RETURN THE SURFACE
	return vk::SurfaceKHR(surface);
}
void Window::poll_events()
{
	glfwPollEvents();
}

void Window::wait_events()
{
	glfwWaitEvents();
}

/*
 * resize_callback - called when the frame buffer is resized, which would result
 * in a resize of the rendering surface.
 */
void resize_callback(GLFWwindow *window, int width, int height)
{
	ResizeEvent event;
	Renderer   *p_renderer = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
	p_renderer->process_event(event);
}

// THE FOLLOWING METHODS ARE GLOBAL HELPER METHODS

/*
 * translate_key_code - since we are using GLFW, key presses will be received as
 * GLFW key code constants. This function examines a GLFW key code argument and returns our
 * appropriate KeyCode value.
 */
inline KeyCode translate_key_code(int key)
{
	// THIS HASH TABLE HELPS US QUICKLY LOOK UP THE CORRESPONDING KEY CODE
	static const std::unordered_map<int, KeyCode> key_lookup = {
	    {GLFW_KEY_W, KeyCode::eW},
	    {GLFW_KEY_S, KeyCode::eS},
	    {GLFW_KEY_A, KeyCode::eA},
	    {GLFW_KEY_D, KeyCode::eD},
	    {GLFW_KEY_1, KeyCode::e1},
	    {GLFW_KEY_2, KeyCode::e2},
	    {GLFW_KEY_3, KeyCode::e3},
	};
	// WHAT IS THE CORRESPONDING KeyCode? NOTE, THE find METHOD RETURNS AN
	// ITERATOR, WHICH POINTS TO A NODE WHERE second WOULD BE THE DATA
	auto it = key_lookup.find(key);

	// IF IT RETURNED THE END, IT MEANS IT WAS NOT FOUND
	if (it == key_lookup.end())
	{
		return KeyCode::eUnknown;
	}
	// second IS THE DATA, SO RETURN THAT AS THE CORRESPONDING KeyCode
	else
	{
		return it->second;
	}
}

/*
 * translate_key_action - since we are using GLFW, key interactions will be received as
 * GLFW action constants. This function examines a GLFW action argument and returns our
 * appropriate KeyAction value.
 */
inline KeyAction translate_key_action(int action)
{
	// KEY WAS PRESSED
	if (action == GLFW_PRESS)
	{
		return KeyAction::eDown;
	}
	// KEY WAS RELEASED
	else if (action == GLFW_RELEASE)
	{
		return KeyAction::eUp;
	}
	// KEY WAS DOUBLE PRESSED
	else if (action == GLFW_REPEAT)
	{
		return KeyAction::eRepeat;
	}
	// SOMETHING ELSE, WE DON'T KNOW WHAT
	return KeyAction::eUnknown;
}

/*
 * translate_mouse_action - since we are using GLFW, mouse interactions will be received as
 * GLFW action constants. This function examines a GLFW action argument and returns our
 * appropriate MouseAction value.
 */
inline MouseAction translate_mouse_action(int action)
{
	// MOUSE BUTTON WAS PRESSED
	if (action == GLFW_PRESS)
	{
		return MouseAction::eDown;
	}
	// MOUSE BUTTON WAS RELEASED
	else if (action == GLFW_RELEASE)
	{
		return MouseAction::eUp;
	}
	// SOMETHING ELSE
	return MouseAction::eUnknown;
}

/*
 * translate_mouse_button - since we are using GLFW, mouse interactions will be received as
 * GLFW constants. This function examines a GLFW button argument and returns our
 * appropriate MouseButton value.
 */
inline MouseButton translate_mouse_button(int button)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		return MouseButton::eLeft;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		return MouseButton::eRight;
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
	{
		return MouseButton::eMiddle;
	}
	return MouseButton::eUnknown;
}

/*
 * key_callback - retrieves suitable data about the key event and then sends it
 * to the renderer's process_event function for a programmed response.
 */
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	// GET THE KEY CODE
	KeyCode key_code = translate_key_code(key);

	// GET THE ACTION
	KeyAction key_action = translate_key_action(action);

	// SEND IT TO THE RENDERER'S process_event METHOD FOR A PROGRAMMED RESPONSE
	Renderer *p_renderer = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
	p_renderer->process_event(KeyInputEvent(key_code, key_action));
}

/*
 * mouse_button_callback - retrieves suitable data about the mouse event and then sends it
 * to the renderer's process_event function for a programmed response.
 */
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	MouseAction mouse_action = translate_mouse_action(action);
	MouseButton mouse_button = translate_mouse_button(button);

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// SEND IT TO THE RENDERER'S process_event METHOD FOR A PROGRAMMED RESPONSE
	auto pRenderer = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
	pRenderer->process_event(MouseButtonInputEvent{
	    mouse_button,
	    mouse_action,
	    static_cast<float>(xpos),
	    static_cast<float>(ypos)});
}

/*
 * cursor_position_callback - retrieves suitable data about the mouse movement event and then sends it
 * to the renderer's process_event function for a programmed response.
 */
void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
	// SEND IT TO THE RENDERER'S process_event METHOD FOR A PROGRAMMED RESPONSE
	auto pRenderer = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window));
	pRenderer->process_event(MouseButtonInputEvent{MouseButton::eUnknown, MouseAction::eMove, static_cast<float>(xpos), static_cast<float>(ypos)});
}

void Window::register_callbacks(Renderer &renderer)
{
	// GIVE THE GLFW WINDOW OUR RENDERER BECAUSE WE MAY NEED IT WHEN
	// RESPONDING TO WINDOW EVENTS LATER
	glfwSetWindowUserPointer(handle_, &renderer);

	// SPECIFY WHICH FUNCTIONS WILL PROVIDE RESPONSES TO WHICH EVENTS
	glfwSetFramebufferSizeCallback(handle_, resize_callback);
	glfwSetKeyCallback(handle_, key_callback);
	glfwSetMouseButtonCallback(handle_, mouse_button_callback);
	glfwSetCursorPosCallback(handle_, cursor_position_callback);
}

vk::Extent2D Window::wait_for_non_zero_extent()
{
	int width, height;
	glfwGetFramebufferSize(handle_, &width, &height);
	while (!width || !height)
	{
		glfwGetFramebufferSize(handle_, &width, &height);
		glfwWaitEvents();
	}
	return vk::Extent2D{
	    .width  = to_u32(width),
	    .height = to_u32(height),
	};
}

/*
* push_required_extensions - goes through all the library extensions available for
* our window and pushes them onto the extensions vector.
*/
void Window::push_required_extensions(std::vector<const char *> &extensions)
{
	uint32_t     glfwExtensionCount = 0;
	const char **glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	for (uint32_t i = 0; i < glfwExtensionCount; i++)
	{
		extensions.push_back(*(glfwExtensions + i));
	}
};

}        // namespace W3D