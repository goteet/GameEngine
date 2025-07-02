#pragma once
#include <vector>
#include "VulkanInclude.h"
#include "GfxInterface.h"



namespace GFXI
{
    struct GraphicDeviceVulkan;

    struct ShaderBinaryVulkan : public ShaderBinary
    {
        ShaderBinaryVulkan(EShaderType, std::vector<unsigned char>&&, const std::string& name, const std::string& entryPoint);
        virtual ~ShaderBinaryVulkan() = default;
        virtual void Release() override;

        virtual EShaderType     GetShaderType() override;
        virtual const char*     GetShaderName() override;
        virtual const char*     GetEntryPointName() override;
        virtual void*           GetBytecode() override;
        virtual unsigned int    GetBytecodeLength() override;

    private:
        EShaderType mShaderType;
        std::vector<unsigned char> mSpirVBinary;
        std::string mName;
        std::string mEntryPoint;
    };

    struct ShaderVulkan : public Shader
    {
        ShaderVulkan(GraphicDeviceVulkan* belongsTo, EShaderType, VkShaderModule, std::vector<unsigned char>&&, const std::string& name, const std::string& entryPoint);
        virtual ~ShaderVulkan();
        virtual void Release() override;
        virtual EShaderType     GetShaderType() override;
        virtual const char*     GetShaderName() override;
        virtual const char*     GetEntryPointName() override;
        virtual unsigned int    GetBytecodeLength()override;
        virtual void*           GetBytecode() override;

        VkShaderModule          GetVulkanShaderModule();
    private:
        GraphicDeviceVulkan* mBelongsTo;
        EShaderType mShaderType;
        VkShaderModule mVulkanShaderModule;
        std::vector<unsigned char> mSpirVBinary;
        std::string mName;
        std::string mEntryPoint;
    };

}
