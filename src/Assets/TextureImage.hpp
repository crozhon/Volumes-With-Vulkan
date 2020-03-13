#pragma once

#include <memory>

namespace Vulkan
{
	class CommandPool;
	class DeviceMemory;
	class Image;
	class ImageView;
	class Sampler;
}

namespace Assets
{
	class Texture;
	
	class TextureImage final
	{
	public:

		TextureImage(const TextureImage&) = delete;
		TextureImage(TextureImage&&) = delete;
		TextureImage& operator = (const TextureImage&) = delete;
		TextureImage& operator = (TextureImage&&) = delete;

		TextureImage(Vulkan::CommandPool& commandPool, const Texture& texture);
		void Reload(Vulkan::CommandPool& commandPool, const unsigned char* pixels);
		~TextureImage();

		const Vulkan::ImageView& ImageView() const { return *imageView_; }
		const Vulkan::Sampler& Sampler() const { return *sampler_; }
		const Vulkan::Image& Image() const { return *image_; }

	private:

		std::unique_ptr<Vulkan::Image> image_;
		std::unique_ptr<Vulkan::DeviceMemory> imageMemory_;
		std::unique_ptr<Vulkan::ImageView> imageView_;
		std::unique_ptr<Vulkan::Sampler> sampler_;
	};

}
