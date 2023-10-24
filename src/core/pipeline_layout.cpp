// IN THIS FILE WE'LL BE DECLARING METHODS DECLARED INSIDE THIS HEADER FILE
#include "pipeline_layout.hpp"

// OUR OWN TYPES
#include "device.hpp"

namespace W3D
{

PipelineLayout::PipelineLayout(Device &device, std::nullptr_t nptr) :
    device_(device)
{
}

PipelineLayout::PipelineLayout(Device &device, vk::PipelineLayoutCreateInfo &pipeline_layout_cinfo) :
    device_(device)
{
	handle_ = device_.get_handle().createPipelineLayout(pipeline_layout_cinfo);
}

PipelineLayout::~PipelineLayout()
{
	if (handle_)
	{
		device_.get_handle().destroyPipelineLayout(handle_);
	}
}

}	// namespace W3D