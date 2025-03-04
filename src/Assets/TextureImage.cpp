#include "TextureImage.hpp"
#include "Texture.hpp"
#include "Vulkan/Buffer.hpp"
#include "Vulkan/CommandPool.hpp"
#include "Vulkan/ImageView.hpp"
#include "Vulkan/Image.hpp"
#include "Vulkan/Sampler.hpp"
#include <cstring>

namespace Assets {

TextureImage::TextureImage(Vulkan::CommandPool& commandPool, const Texture& texture)
{
	if (texture.Is_Volume() == false) {
		// Create a host staging buffer and copy the image into it.
		const VkDeviceSize imageSize = texture.Width() * texture.Height() * 4;
		const auto& device = commandPool.Device();

		auto stagingBuffer = std::make_unique<Vulkan::Buffer>(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		auto stagingBufferMemory = stagingBuffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		const auto data = stagingBufferMemory.Map(0, imageSize);
		std::memcpy(data, texture.Pixels(), imageSize);
		stagingBufferMemory.Unmap();

		// Create the device side image, memory, view and sampler.
		image_.reset(new Vulkan::Image(device, VkExtent2D{ static_cast<uint32_t>(texture.Width()), static_cast<uint32_t>(texture.Height()) }, VK_FORMAT_R8G8B8A8_UNORM));
		imageMemory_.reset(new Vulkan::DeviceMemory(image_->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
		imageView_.reset(new Vulkan::ImageView(device, image_->Handle(), image_->Format(), VK_IMAGE_ASPECT_COLOR_BIT));
		sampler_.reset(new Vulkan::Sampler(device, Vulkan::SamplerConfig()));

		// Transfer the data to device side.
		image_->TransitionImageLayout(commandPool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		image_->CopyFrom(commandPool, *stagingBuffer);
		image_->TransitionImageLayout(commandPool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// Delete the buffer before the memory
		stagingBuffer.reset();
	} else {
		const VkDeviceSize imageSize = texture.Width() * texture.Height() * texture.Depth();
		const auto& device = commandPool.Device();

		auto stagingBuffer = std::make_unique<Vulkan::Buffer>(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		auto stagingBufferMemory = stagingBuffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		const auto data = stagingBufferMemory.Map(0, imageSize);
		std::memcpy(data, texture.Pixels(), imageSize);
		stagingBufferMemory.Unmap();

		image_.reset(new Vulkan::Image(device,
									   VkExtent3D{ static_cast<uint32_t>(texture.Width()), static_cast<uint32_t>(texture.Height()), static_cast<uint32_t>(texture.Depth()) },
									   VK_FORMAT_R8_UNORM));
		imageMemory_.reset(new Vulkan::DeviceMemory(image_->AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)));
		imageView_.reset(new Vulkan::ImageView(device, image_->Handle(), image_->Format(), VK_IMAGE_ASPECT_COLOR_BIT, true));

		auto sampler_config = Vulkan::SamplerConfig();
		sampler_config.MaxAnisotropy = 1.0f;
		sampler_config.BorderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		sampler_config.AddressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_config.AddressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_config.AddressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler_.reset(new Vulkan::Sampler(device, sampler_config));

		image_->TransitionImageLayout(commandPool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		image_->CopyFrom(commandPool, *stagingBuffer);
		image_->TransitionImageLayout(commandPool, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// Delete the buffer before the memory
		stagingBuffer.reset();
	}
}

void TextureImage::Reload(Vulkan::CommandPool& commandPool, const unsigned char* pixels)
{
	const VkDeviceSize imageSize = 128 * 128 * 128 * 1;
	const auto& device = commandPool.Device();

	auto stagingBuffer = std::make_unique<Vulkan::Buffer>(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	auto stagingBufferMemory = stagingBuffer->AllocateMemory(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	const auto data = stagingBufferMemory.Map(0, imageSize);
	std::memcpy(data, pixels, imageSize);
	stagingBufferMemory.Unmap();

	image_->CopyFrom(commandPool, *stagingBuffer);

	stagingBuffer.reset();
}

TextureImage::~TextureImage()
{
	sampler_.reset();
	imageView_.reset();
	image_.reset();
	imageMemory_.reset();
}

}
