#pragma once

#include "Vulkan/Sampler.hpp"
#include <memory>
#include <string>

namespace Assets
{
	class Texture final
	{
	public:

		static Texture LoadTexture(const std::string& filename, const Vulkan::SamplerConfig& samplerConfig);

		Texture& operator = (const Texture&) = delete;
		Texture& operator = (Texture&&) = delete;

		Texture() = default;
		Texture(const Texture&) = default;
		Texture(Texture&&) = default;
		~Texture() = default;

		const unsigned char* Pixels() const { return pixels_.get(); }
		int Width() const { return width_; }
		int Height() const { return height_; }

		int Depth() const { return depth_; }

		bool Is_Volume() const { return is_volume_; }

		Texture(int width, int height, int channels, unsigned char* pixels, int depth=0);
	private:

		Vulkan::SamplerConfig samplerConfig_;
		int width_;
		int height_;

		int depth_; // Optional
		bool is_volume_;

		int channels_;
		std::unique_ptr<unsigned char, void (*) (void*)> pixels_;
	};

}
