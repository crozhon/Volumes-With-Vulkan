#pragma once

#include "Vulkan.hpp"
#include "DeviceMemory.hpp"

#include <variant>

namespace Vulkan
{
	class Buffer;
	class CommandPool;
	class Device;

	class Image final
	{
	public:

		Image(const Image&) = delete;
		Image& operator = (const Image&) = delete;
		Image& operator = (Image&&) = delete;

		Image(const Device& device, VkExtent2D extent, VkFormat format);
		Image(const Device& device, VkExtent3D extent, VkFormat format);
		Image(const Device& device, VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);
		Image(const Device& device, VkExtent3D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);
		Image(Image&& other) noexcept;
		~Image();

		const class Device& Device() const { return device_; }
		VkExtent2D Extent() const { return std::get<VkExtent2D>(extent_); }
		VkFormat Format() const { return format_; }

		DeviceMemory AllocateMemory(VkMemoryPropertyFlags properties) const;
		VkMemoryRequirements GetMemoryRequirements() const;

		void TransitionImageLayout(CommandPool& commandPool, VkImageLayout newLayout);
		void CopyFrom(CommandPool& commandPool, const Buffer& buffer);

	private:

		const class Device& device_;
		const std::variant<VkExtent2D, VkExtent3D> extent_;
		const VkFormat format_;
		VkImageLayout imageLayout_;

		VULKAN_HANDLE(VkImage, image_)
	};

}
