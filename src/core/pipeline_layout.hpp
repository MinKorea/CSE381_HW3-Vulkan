#pragma once

#include "common/vk_common.hpp"
#include "core/vulkan_object.hpp"

namespace W3D
{
class Device;

/*
* Serves as a wrapper class for a Vulkan API PipelineLayout object, which is
* what the handle for this function will be.
*/
class PipelineLayout : public VulkanObject<vk::PipelineLayout>
{
  private:
	Device &device_;	// LOGICAL DEVICE FOR THIS PIPELINE LAYOUT

  public:
	/*
	* Constructor sets the device but does not create the layout.
	*/
	PipelineLayout(Device &device, std::nullptr_t nptr);

	/*
	* Constructor sets the device and also uses the info object to create the
	* Vulkan pipeline layout object, which is stored as the handle.
	*/
	PipelineLayout(Device &device, vk::PipelineLayoutCreateInfo &pipeline_layout_cinfo);

	/*
	* Since the constructor creates the handle, this destructor has to destroy the
	* Vulkan API object.
	*/
	~PipelineLayout() override;

};	// class PipelineLayout

}	// namespace W3D