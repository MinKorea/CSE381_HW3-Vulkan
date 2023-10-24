// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "swapchain.hpp"

// OUR OWN TYPES
#include "core/device_memory/image.hpp"
#include "core/instance.hpp"
#include "device.hpp"
#include "image_resource.hpp"
#include "image_view.hpp"
#include "physical_device.hpp"

namespace W3D
{

Swapchain::Swapchain(Device &device, vk::Extent2D window_extent) :
    device_(device)
{
	build(window_extent);
}

Swapchain::~Swapchain()
{
	cleanup();
}

void Swapchain::cleanup()
{
	p_depth_resource_.reset();
	frame_image_views_.clear();
	device_.get_handle().destroySwapchainKHR(handle_);
}

void Swapchain::rebuild(vk::Extent2D new_window_extent)
{
	cleanup();
	build(new_window_extent);
}

void Swapchain::build(vk::Extent2D window_extent)
{
	const SwapchainSupportDetails &details = device_.get_physical_device().get_swapchain_support_details();
	choose_format(details.formats);
	choose_present_mode(details.present_modes);
	choose_extent(details.capabilities, window_extent);

	vk::SwapchainCreateInfoKHR swapchain_cinfo{
	    .surface          = device_.get_instance().get_surface(),
	    .minImageCount    = calc_min_image_count(details.capabilities.minImageCount, details.capabilities.maxImageCount),
	    .imageFormat      = properties_.surface_format.format,
	    .imageColorSpace  = properties_.surface_format.colorSpace,
	    .imageExtent      = properties_.extent,
	    .imageArrayLayers = 1,
	    .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
	    .preTransform     = details.capabilities.currentTransform,
	    .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
	    .presentMode      = properties_.present_mode,
	    .clipped          = true,
	    .oldSwapchain     = nullptr,
	};
	const QueueFamilyIndices &indices = device_.get_physical_device().get_queue_family_indices();
	if (indices.graphics_index.value() == indices.present_index.value())
	{
		swapchain_cinfo.imageSharingMode = vk::SharingMode::eExclusive;
	}
	else
	{
		swapchain_cinfo.imageSharingMode      = vk::SharingMode::eConcurrent;
		swapchain_cinfo.queueFamilyIndexCount = 2;
	}

	handle_ = device_.get_handle().createSwapchainKHR(swapchain_cinfo);
	create_frame_resources();
}

void Swapchain::create_frame_resources()
{
	frame_images_ = std::move(device_.get_handle().getSwapchainImagesKHR(handle_));

	vk::ImageViewCreateInfo swapchain_image_view_cinfo{
	    .viewType         = vk::ImageViewType::e2D,
	    .format           = properties_.surface_format.format,
	    .subresourceRange = {
	        .aspectMask     = vk::ImageAspectFlagBits::eColor,
	        .baseMipLevel   = 0,
	        .levelCount     = 1,
	        .baseArrayLayer = 0,
	        .layerCount     = 1,
	    },
	};

	for (auto image : frame_images_)
	{
		swapchain_image_view_cinfo.image = image;
		frame_image_views_.emplace_back(ImageView(device_, swapchain_image_view_cinfo));
	}

	vk::ImageCreateInfo depth_image_cinfo{

	    .imageType = vk::ImageType::e2D,
	    .format    = choose_depth_format(),
	    .extent    = vk::Extent3D{
	           .width  = properties_.extent.width,
	           .height = properties_.extent.height,
	           .depth  = 1,
        },
	    .mipLevels     = 1,
	    .arrayLayers   = 1,
	    .samples       = vk::SampleCountFlagBits::e1,
	    .tiling        = vk::ImageTiling::eOptimal,
	    .usage         = vk::ImageUsageFlagBits::eDepthStencilAttachment,
	    .sharingMode   = vk::SharingMode::eExclusive,
	    .initialLayout = vk::ImageLayout::eUndefined,
	};
	Image img = device_.get_device_memory_allocator().allocate_device_only_image(depth_image_cinfo);

	vk::ImageViewCreateInfo depth_image_view_cinfo = ImageView::two_dim_view_cinfo(img.get_handle(), depth_image_cinfo.format, vk::ImageAspectFlagBits::eDepth, 1);
	p_depth_resource_                              = std::make_unique<ImageResource>(std::move(img), ImageView(device_, depth_image_view_cinfo));
}

vk::Format Swapchain::choose_depth_format()
{
	std::array<vk::Format, 3> candidate_formats = {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint};
	for (vk::Format candidate : candidate_formats)
	{
		vk::FormatProperties candidate_properties = device_.get_physical_device().get_handle().getFormatProperties(candidate);
		if (candidate_properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
		{
			return candidate;
		}
	}

	throw std::runtime_error("failed to find supported depth format!");
};

void Swapchain::choose_format(const std::vector<vk::SurfaceFormatKHR> &formats)
{
	for (const auto &avaliable_surface_format : formats)
	{
		if (avaliable_surface_format.format == vk::Format::eB8G8R8Srgb && avaliable_surface_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			properties_.surface_format = avaliable_surface_format;
		}
	}
	properties_.surface_format = formats[0];
}

void Swapchain::choose_present_mode(const std::vector<vk::PresentModeKHR> &present_modes)
{
	for (const auto &avaliable_present_mode : present_modes)
	{
		if (avaliable_present_mode == vk::PresentModeKHR::eMailbox)
		{
			properties_.present_mode = avaliable_present_mode;
			return;
		}
	}
	properties_.present_mode = present_modes[0];
}

void Swapchain::choose_extent(const vk::SurfaceCapabilitiesKHR &capabilities, vk::Extent2D window_extent)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		properties_.extent = capabilities.currentExtent;
		return;
	}

	properties_.extent = vk::Extent2D{
	    .width  = std::clamp(window_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
	    .height = std::clamp(window_extent.height, capabilities.minImageExtent.height, capabilities.minImageExtent.height),
	};
}

uint32_t Swapchain::calc_min_image_count(uint32_t min_image_count, uint32_t max_image_count)
{
	uint32_t image_count = min_image_count + 1;
	// max_image_count == 0 means there is no limit
	if (max_image_count > 0 && image_count > max_image_count)
	{
		image_count = max_image_count;
	}
	return image_count;
}

const SwapchainProperties &Swapchain::get_swapchain_properties() const
{
	return properties_;
}

const std::vector<ImageView> &Swapchain::get_frame_image_views() const
{
	return frame_image_views_;
}

const ImageResource &Swapchain::get_depth_resource() const
{
	return *p_depth_resource_;
}

// Swapchain::Swapchain(Instance* pInstance, Device* pDevice, Window* pWindow,
//                      vk::SampleCountFlagBits mssaSamples)
//     : pInstance_(pInstance), pDevice_(pDevice), pWindow_(pWindow), mssaSamples_(mssaSamples) {
//     createSwapchain();
//     framebuffers_.reserve(imageViews_.size());
// };

// void Swapchain::recreate() {
//     cleanup();
//     createSwapchain();
// }

// void Swapchain::cleanup() {
//     depthResource_.view.clear();
//     depthResource_.pImage.reset();
//     framebuffers_.clear();
//     imageViews_.clear();
//     swapchain_.reset();
// }

// void Swapchain::createSwapchain() {
//     auto details = pInstance_->swapchainSupportDetails();
//     chooseSurfaceFormat(details.formats);
//     choosePresentMode(details.presentModes);
//     chooseExtent(details.capabilities);
//     uint32_t imageCount = details.capabilities.minImageCount + 1;

//     if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
//         imageCount = details.capabilities.maxImageCount;
//     }

//     vk::SwapchainCreateInfoKHR createInfo{};
//     createInfo.surface = *(pInstance_->surface());
//     createInfo.minImageCount = imageCount;
//     createInfo.imageFormat = surfaceFormat_.format;
//     createInfo.imageColorSpace = surfaceFormat_.colorSpace;
//     createInfo.imageExtent = extent_;
//     createInfo.imageArrayLayers = 1;
//     createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

//     auto indices = pInstance_->queueFamilyIndices();
//     auto uniqueQueueFamilies = pInstance_->uniqueQueueFamilies();
//     auto uniqueFamilyCount = uniqueQueueFamilies.size();
//     std::vector<uint32_t> uniqueIndices;
//     for (auto index : uniqueQueueFamilies) {
//         uniqueIndices.push_back(index);
//     }

//     if (uniqueFamilyCount == 1) {
//         createInfo.imageSharingMode = vk::SharingMode::eExclusive;
//     } else {
//         createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
//         createInfo.queueFamilyIndexCount = uniqueFamilyCount;
//         createInfo.pQueueFamilyIndices = uniqueIndices.data();
//     }
//     createInfo.preTransform = details.capabilities.currentTransform;
//     createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
//     createInfo.presentMode = presentMode_;
//     createInfo.clipped = VK_TRUE;
//     createInfo.oldSwapchain = VK_NULL_HANDLE;

//     swapchain_ = std::make_unique<vk::raii::SwapchainKHR>(pDevice_->handle(), createInfo);
//     images_ = swapchain_->getImages();
//     createImageViews();
//     createDepthResource();
// }

// void Swapchain::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
//     for (const auto& avaliableFormat : formats) {
//         if (avaliableFormat.format == vk::Format::eB8G8R8Srgb &&
//             avaliableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
//             surfaceFormat_ = avaliableFormat;
//             return;
//         }
//     }
//     surfaceFormat_ = formats[0];
// }

// void Swapchain::choosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes) {
//     for (const auto& avaliablePresentMode : presentModes) {
//         if (avaliablePresentMode == vk::PresentModeKHR::eMailbox) {
//             presentMode_ = avaliablePresentMode;
//             return;
//         }
//     }
//     presentMode_ = presentModes[0];
// }

// void Swapchain::chooseExtent(onst vk::SurfaceCapabilitiesKHR& capabilities) {
//     if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
//         extent_ = capabilities.currentExtent;
//         return;
//     }

//     int width, height;
//     pWindow_->getFramebufferSize(&width, &height);
//     vk::Extent2D actualExtent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
//     actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
//                                     capabilities.maxImageExtent.width);
//     actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
//                                      capabilities.maxImageExtent.height);
//     extent_ = actualExtent;
// }

// void Swapchain::createImageViews() {
//     imageViews_.reserve(images_.size());
//     for (auto image : images_) {
//         imageViews_.emplace_back(pDevice_->createImageView(image, surfaceFormat_.format,
//                                                            vk::ImageAspectFlagBits::eColor,
//                                                            vk::ImageViewType::e2D, 1));
//     }
//     // for (size_t i = 0; i < images_.size(); i++) {
//     //     imageViews_[i] = pDevice_->createImageView(images_[i], surfaceFormat_.format,
//     //                                                vk::ImageAspectFlagBits::eColor, 1);
//     // }
// }

// void Swapchain::createDepthResource() {
//     vk::Format depthFormat = findDepthFormat();
//     vk::ImageCreateInfo imageInfo;
//     imageInfo.imageType = vk::ImageType::e2D;
//     imageInfo.extent = vk::Extent3D{extent_.width, extent_.height, 1};
//     imageInfo.mipLevels = 1;
//     imageInfo.arrayLayers = 1;
//     imageInfo.format = depthFormat;
//     imageInfo.tiling = vk::ImageTiling::eOptimal;
//     imageInfo.initialLayout = vk::ImageLayout::eUndefined;
//     imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
//     imageInfo.sharingMode = vk::SharingMode::eExclusive;
//     imageInfo.samples = mssaSamples_;
//     depthResource_.pImage = pDevice_->get_allocator().allocateAttachmentImage(imageInfo);
//     depthResource_.view =
//         pDevice_->createImageView(depthResource_.pImage->handle(), depthFormat,
//                                   vk::ImageAspectFlagBits::eDepth, vk::ImageViewType::e2D, 1);
// }

// vk::Format Swapchain::findDepthFormat() {
//     std::array<vk::Format, 3> candidates = {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
//                                             vk::Format::eD24UnormS8Uint};
//     for (auto format : candidates) {
//         vk::FormatProperties properties = pInstance_->physicalDevice().getFormatProperties(format);
//         if (properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
//             return format;
//         }
//     }

//     throw std::runtime_error("failed to find supported depth format!");
// }

// void Swapchain::createFrameBuffers(vk::raii::RenderPass& renderPass) {
//     std::array<vk::ImageView, 2> attachments;
//     for (auto const& imageView : imageViews_) {
//         attachments[0] = *imageView;
//         attachments[1] = *depthResource_.view;
//         vk::FramebufferCreateInfo framebufferCreateInfo({}, *renderPass, attachments, extent_.width,
//                                                         extent_.height, 1);
//         framebuffers_.emplace_back(pDevice_->handle().createFramebuffer(framebufferCreateInfo));
//     }
// }

// // A Wrapper to avoid exceptions. Instead, we return proper error codes.
// // For discussion, see https://github.com/KhronosGroup/Vulkan-Hpp/issues/599
// std::pair<vk::Result, uint32_t> Swapchain::acquireNextImage(uint64_t timeout,
//                                                             vk::Semaphore semaphore,
//                                                             vk::Fence fence) {
//     uint32_t imageIndex;
//     vk::Result result = static_cast<vk::Result>(swapchain_->getDispatcher()->vkAcquireNextImageKHR(
//         static_cast<VkDevice>(*pDevice_->handle()), static_cast<VkSwapchainKHR>(**swapchain_),
//         timeout, static_cast<VkSemaphore>(semaphore), static_cast<VkFence>(fence), &imageIndex));
//     return std::make_pair(result, imageIndex);
// }

}        // namespace W3D