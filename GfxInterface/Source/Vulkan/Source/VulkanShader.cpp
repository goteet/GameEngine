#pragma once
#include "VulkanShader.h"
#include "VulkanGraphicDevice.h"



namespace GFXI
{
    ShaderBinaryVulkan::ShaderBinaryVulkan(EShaderType type, std::vector<unsigned char>&& binary, const std::string& name, const std::string& entryPoint)
        : mShaderType(type)
        , mSpirVBinary(std::move(binary))
        , mName(name)
        , mEntryPoint(entryPoint)
    {

    }

    void ShaderBinaryVulkan::Release()
    {
        delete this;
    }

    EShaderType ShaderBinaryVulkan::GetShaderType()
    {
        return mShaderType;
    }

    const char* ShaderBinaryVulkan::GetShaderName()
    {
        return mName.c_str();
    }

    const char* ShaderBinaryVulkan::GetEntryPointName()
    {
        return mEntryPoint.c_str();
    }

    void* ShaderBinaryVulkan::GetBytecode()
    {
        return mSpirVBinary.data();
    }

    unsigned int ShaderBinaryVulkan::GetBytecodeLength()
    {
        return static_cast<unsigned int>(mSpirVBinary.size());
    }

    ShaderVulkan::ShaderVulkan(GraphicDeviceVulkan* belongsTo, EShaderType type, VkShaderModule shaderModule, std::vector<unsigned char>&& binary, const std::string& name, const std::string& entryPoint)
        : mBelongsTo(belongsTo)
        , mShaderType(type)
        , mVulkanShaderModule(shaderModule)
        , mSpirVBinary(std::move(binary))
        , mName(name)
        , mEntryPoint(entryPoint)
    {
    }

    ShaderVulkan::~ShaderVulkan()
    {
        vkDestroyShaderModule(mBelongsTo->GetVulkanDevice(), mVulkanShaderModule, GFX_VK_ALLOCATION_CALLBACK);
    }

    void ShaderVulkan::Release()
    {
    }

    EShaderType ShaderVulkan::GetShaderType()
    {
        return mShaderType;
    }

    const char* ShaderVulkan::GetShaderName()
    {
        return mName.c_str();
    }

    const char* ShaderVulkan::GetEntryPointName()
    {
        return mEntryPoint.c_str();
    }

    unsigned int ShaderVulkan::GetBytecodeLength()
    {
        return static_cast<unsigned int>(mSpirVBinary.size());
    }

    void* ShaderVulkan::GetBytecode()
    {
        return mSpirVBinary.data();
    }

    VkShaderModule ShaderVulkan::GetVulkanShaderModule()
    {
        return mVulkanShaderModule;
    }
}
