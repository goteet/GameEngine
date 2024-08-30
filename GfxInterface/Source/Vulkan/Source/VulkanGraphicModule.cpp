#include "VulkanGraphicDevice.h"
#include "VulkanGraphicModule.h"

#define kVulkanValidationLayer "VK_LAYER_KHRONOS_validation"
namespace GFXI
{
    const int kEnableDebugValidationLayer = true;
    GraphicModuleVulkan::GraphicModuleVulkan()
    {
        VkInstance VulkanInstance = CreateVkInstance(kEnableDebugValidationLayer);
        if (VulkanInstance != nullptr)
        {
            VkPhysicalDevice PhyiscalDevice = nullptr;
            VkPhysicalDeviceFeatures DeviceFeatures;
            int32_t QueueFamilyIndex = -1;
            
            if (SelectBestPhysicalDevice(VulkanInstance, PhyiscalDevice, QueueFamilyIndex, DeviceFeatures))
            {
                mVulkanInstance     = VulkanInstance;
                mPhyiscalDevice     = PhyiscalDevice;
                mQueueFamilyIndex   = QueueFamilyIndex;
                mDeviceFeatures     = DeviceFeatures;
            }
            else
            {
                vkDestroyInstance(VulkanInstance, GFX_VK_ALLOCATION_CALLBACK);
            }
        }
    }

    GraphicModuleVulkan::~GraphicModuleVulkan()
    {
        if (mVulkanInstance != nullptr)
        {
            vkDestroyInstance(mVulkanInstance, GFX_VK_ALLOCATION_CALLBACK);
        }
    }

    bool GraphicModuleVulkan::IsHardwareSupported()
    {
        return mVulkanInstance != nullptr && mPhyiscalDevice != nullptr && mQueueFamilyIndex != kQueueFamilyNotFound;
    }

    GraphicDevice* GraphicModuleVulkan::CreateDevice()
    {
        const float kDeviceQueuePriority = 1.0;
        VkDeviceQueueCreateInfo DeviceQueueCreateInfo;
        VulkanZeroMemory(DeviceQueueCreateInfo, VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO);
        DeviceQueueCreateInfo.queueFamilyIndex = mQueueFamilyIndex;
        DeviceQueueCreateInfo.queueCount = 1;
        DeviceQueueCreateInfo.pQueuePriorities = &kDeviceQueuePriority;

        VkDeviceCreateInfo DeviceCreateInfo;
        VulkanZeroMemory(DeviceCreateInfo, VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);
        DeviceCreateInfo.queueCreateInfoCount = 1;
        DeviceCreateInfo.pQueueCreateInfos = &DeviceQueueCreateInfo;
        DeviceCreateInfo.pEnabledFeatures = &mDeviceFeatures;
        if (mDebugLayer.size() > 0)
        {
            DeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(mDebugLayer.size());
            DeviceCreateInfo.ppEnabledLayerNames = mDebugLayer.data();
        }

        VkDevice VulkanDevice;
        VkResult RetCreateDevice = vkCreateDevice(mPhyiscalDevice, &DeviceCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &VulkanDevice);

        if (RetCreateDevice == VkResult::VK_SUCCESS)
        {
            return new GraphicDeviceVulkan(this,VulkanDevice);
        }

        return nullptr;
    }

    void GraphicModuleVulkan::Release()
    {
        delete this;
    }

    VkInstance GraphicModuleVulkan::CreateVkInstance(bool bEnableDebugLayer)
    {
        if (bEnableDebugLayer)
        {
            uint32_t NumLayers = 0;
            vkEnumerateInstanceLayerProperties(&NumLayers, nullptr);

            if (NumLayers > 0)
            {
                std::vector<VkLayerProperties> LayerProperties(NumLayers);
                vkEnumerateInstanceLayerProperties(&NumLayers, LayerProperties.data());

                for (const VkLayerProperties& LayerProperty : LayerProperties)
                {
                    if (strcmp(LayerProperty.layerName, kVulkanValidationLayer) == 0)
                    {
                        mDebugLayer.push_back(kVulkanValidationLayer);
                    }
                }
            }
        }

        VkApplicationInfo ApplicationInfo;
        VulkanZeroMemory(ApplicationInfo, VK_STRUCTURE_TYPE_APPLICATION_INFO);
        ApplicationInfo.pEngineName = "GameEngine";
        ApplicationInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo CreateInfo;
        VulkanZeroMemory(CreateInfo, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
        CreateInfo.pApplicationInfo = &ApplicationInfo;

        if (mDebugLayer.size() > 0)
        {
            CreateInfo.enabledLayerCount = static_cast<uint32_t>(mDebugLayer.size());
            CreateInfo.ppEnabledLayerNames = mDebugLayer.data();
        }

        VkInstance VulkanInstance = nullptr;
        VkResult result = vkCreateInstance(&CreateInfo, GFX_VK_ALLOCATION_CALLBACK, &VulkanInstance);

        if (result == VkResult::VK_SUCCESS)
        {
            return VulkanInstance;
        }
        else
        {
            return nullptr;
        }
    }

    bool GraphicModuleVulkan::SelectBestPhysicalDevice(const VkInstance& VulkanInstance, VkPhysicalDevice& OutPhysicalDevice, int32_t& OutQueueFamilyIndex, VkPhysicalDeviceFeatures& OutDeviceFeatures)
    {
        uint32_t PhysicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(VulkanInstance, &PhysicalDeviceCount, nullptr);

        if (PhysicalDeviceCount > 0)
        {
            std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDeviceCount);
            vkEnumeratePhysicalDevices(VulkanInstance, &PhysicalDeviceCount, PhysicalDevices.data());

            int32_t MaxDeviceScore = 0;
            int32_t BestDeviceQueueIndex = 0;
            VkPhysicalDevice BestPhysicalDevice = nullptr;
            VkPhysicalDeviceFeatures BestPhysicalDeviceFeatures;
            for (VkPhysicalDevice& PhysicalDevice : PhysicalDevices)
            {
                int32_t Score = 0;
                VkPhysicalDeviceProperties  DeviceProperties;
                VkPhysicalDeviceFeatures    DeviceFeatures;
                vkGetPhysicalDeviceProperties(PhysicalDevice, &DeviceProperties);
                vkGetPhysicalDeviceFeatures(PhysicalDevice, &DeviceFeatures);

                if (DeviceProperties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                {
                    Score += 100;
                }

                if (DeviceFeatures.geometryShader)
                {
                    Score += 10;
                }

                if (DeviceFeatures.tessellationShader)
                {
                    Score += 10;
                }

                uint32_t NumQueleFamily = 0;
                uint32_t PipelineSupport = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &NumQueleFamily, nullptr);
                if (NumQueleFamily > 0)
                {
                    std::vector<VkQueueFamilyProperties> DeviceQueueFamilyProperties(NumQueleFamily);
                    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &NumQueleFamily, DeviceQueueFamilyProperties.data());

                    int32_t QueueIndex = 0;
                    for (VkQueueFamilyProperties& QueueFamilyProperties : DeviceQueueFamilyProperties)
                    {
                        const bool SupportGraphicPipeline = (QueueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
                        const bool SupportComputePipeline = (QueueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0;
                        if (SupportGraphicPipeline && SupportComputePipeline)
                        {
                            PipelineSupport = 2;
                            BestDeviceQueueIndex = QueueIndex;
                            break;
                        }
                        else if (SupportGraphicPipeline && PipelineSupport == 0)
                        {
                            PipelineSupport = 1;
                            BestDeviceQueueIndex = QueueIndex;
                        }
                        QueueIndex += 1;
                    }

                    if (PipelineSupport == 0)
                    {
                        Score = 0;
                    }
                    else
                    {
                        Score += PipelineSupport * 10;
                    }
                }
                else
                {
                    Score = 0;
                }

                if (MaxDeviceScore < Score)
                {
                    MaxDeviceScore = Score;
                    BestPhysicalDevice = PhysicalDevice;
                    BestPhysicalDeviceFeatures = DeviceFeatures;
                }
            }

            if (BestPhysicalDevice != nullptr && BestDeviceQueueIndex != -1)
            {
                OutPhysicalDevice = BestPhysicalDevice;
                OutQueueFamilyIndex = BestDeviceQueueIndex;
                OutDeviceFeatures = BestPhysicalDeviceFeatures;
                return true;
            }
        }
        return false;
    }
}

extern "C" GfxInterfaceAPI GFXI::GraphicModule* CreateGfxModule()
{
    return new GFXI::GraphicModuleVulkan();
}
