#pragma once

#include "common/vk_common.hpp"
#include "core/image_view.hpp"
#include "device_memory/image.hpp"

#include <memory>

namespace W3D
{

struct ImageLoadResult;

/*
* Meta information that describes the image being loaded.
*/
struct ImageMetaInfo
{
	vk::Extent3D extent;
	vk::Format   format;
	uint32_t     levels;
};

/*
* Stores information about an image being loaded divided into the
* image data (binary) and information about the image (meta).
*/
struct ImageTransferInfo
{
	std::vector<uint8_t> binary;
	ImageMetaInfo        meta;
};

ImageTransferInfo stb_load(const std::string &path);
ImageTransferInfo gli_load(const std::string &path);

class ImageView;

/*
* This class combines a Vulkan API ImageView with a device memory object image so
* as to manage the loading and unloading of images on the GPU for our application.
*/
class ImageResource
{
  private:
	Image     image_;	// DEVICE MEMORY OBJECT Image
	ImageView view_;	// Vulkan API ImageView wrapper

  public:
	/*
	* Static helper function get getting the number of bytes required for storing
	* data of a particular Vulkan format (i.e. vk::Format).
	*/
	static uint8_t           format_to_bytes_per_pixel(vk::Format format);

	/*
	* Static function for loading a two dimensional image.
	*/
	static ImageTransferInfo load_two_dim_image(const std::string &path);

	/*
	* Static function for loading a cubic image for cubic mapping.
	*/
	static ImageTransferInfo load_cubic_image(const std::string &path);

	/*
	* Static function for creating an empty two dimensional image without data.
	*/
	static ImageResource create_empty_two_dim_img_resrc(const Device &device, const ImageMetaInfo &meta);

	/*
	 * Static function for creating an empty cubic image without data.
	 */
	static ImageResource     create_empty_cubic_img_resrc(const Device &device, const ImageMetaInfo &meta);

	/*
	* This constructor initializes this object using a null image.
	*/
	ImageResource(const Device &device, std::nullptr_t nptr);

	/*
	 * This constructor initializes this object using the provided ImageView.
	 */
	ImageResource(Image &&image, ImageView &&view);

	/*
	 * This constructor initializes this object using the provided image resource.
	 */
	ImageResource(ImageResource &&rhs);

	/*
	 * Overloaded assignment operator.
	 */
	ImageResource &operator=(ImageResource &&rhs);

	/*
	* Nothing for this destructor to destroy.
	*/
	~ImageResource();

	/*
	* Accessor method for getting the device memory object image.
	*/
	Image           &get_image();

	/*
	* Accessor method for getting the Vulkan API ImageView.
	*/
	const ImageView &get_view() const;

};	// class ImageResource

/*
* Used for storing information about a image loading.
*/
struct ImageLoadResult
{
	ImageResource     resource;
	ImageTransferInfo image_tifno;
};

};	// namespace W3D