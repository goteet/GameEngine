#include <algorithm>
#include "VulkanGraphicDevice.h"
#include "VulkanGraphicModule.h"

#define ENGINE_VK_VALIDATION_LAYER_NAME  "VK_LAYER_KHRONOS_validation"
namespace GFXI
{
    const int kEnableDebugValidationLayer = true;
    const int32_t kQueueFamilyNotFound = -1;
    const int32_t kQueueNotFound = -1;

    GraphicModuleVulkan::GraphicModuleVulkan()
    {
        VkInstance VulkanInstance = CreateVkInstance(kEnableDebugValidationLayer);
        if (VulkanInstance != nullptr)
        {
            mVulkanInstance = VulkanInstance;
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
        return mVulkanInstance != nullptr;
    }

    struct VulkanQueueFamilyInfo
    {
        uint32_t Index = 0;
        VkQueueFamilyProperties Properties;
        uint32_t NumQueues()   const { return Properties.queueCount; }
        bool SupportGraphic()  const { return (Properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0; }
        bool SupportCompute()  const { return (Properties.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0; }
        bool SupportTransfer() const { return (Properties.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0; }
    };

    struct VulkanPhysicalDeviceInfo
    {
        VkPhysicalDevice            Device = nullptr;
        VkPhysicalDeviceFeatures    Features;
        VkPhysicalDeviceProperties  Properties;

        uint32_t                    EstimateScore = 0;
        uint32_t                    GraphicQueueFamilyIndex  = kQueueFamilyNotFound, GraphicQueueIndex  = kQueueNotFound;
        uint32_t                    ComputeQueueFamilyIndex  = kQueueFamilyNotFound, ComputeQueueIndex  = kQueueNotFound;
        uint32_t                    TransferQueueFamilyIndex = kQueueFamilyNotFound, TransferQueueIndex = kQueueNotFound;
        std::vector<VulkanQueueFamilyInfo> QueueFamilyInfos;
    };

    void EstimatePhysicalDeviceScore(VulkanPhysicalDeviceInfo& InOutPhysicalDeviceInfo)
    {
        InOutPhysicalDeviceInfo.EstimateScore = 0;

        const VkBool32 RequiredProperties[] =
        {
            InOutPhysicalDeviceInfo.Features.multiViewport,
            InOutPhysicalDeviceInfo.Features.geometryShader,
            InOutPhysicalDeviceInfo.Features.tessellationShader,
            InOutPhysicalDeviceInfo.Features.multiDrawIndirect
        };

        for (bool SupportedProperty : RequiredProperties)
        {
            if (!SupportedProperty)
            {
                InOutPhysicalDeviceInfo.EstimateScore = 0;
                return;
            }
        }

        switch (InOutPhysicalDeviceInfo.Properties.deviceType)
        {
        case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            InOutPhysicalDeviceInfo.EstimateScore = 2000;
            break;

        case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            InOutPhysicalDeviceInfo.EstimateScore = 1000;
            break;
        default:
            return;
        }

        bool bDedicatedTransfer = InOutPhysicalDeviceInfo.TransferQueueFamilyIndex != InOutPhysicalDeviceInfo.ComputeQueueFamilyIndex;
        bool bAsyncCompute = InOutPhysicalDeviceInfo.ComputeQueueFamilyIndex != InOutPhysicalDeviceInfo.ComputeQueueFamilyIndex;

        if (bDedicatedTransfer)
        {
            InOutPhysicalDeviceInfo.EstimateScore += 100;
        }
        if (bAsyncCompute)
        {
            InOutPhysicalDeviceInfo.EstimateScore += 100;
        }
    }

    std::vector<VulkanPhysicalDeviceInfo> EnumerateAvailableVulkanPhysicalDevices(VkInstance Instance, bool EnableDebugLayer)
    {
        std::vector<VulkanPhysicalDeviceInfo> DeviceInfos;

        uint32_t PhysicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, nullptr);
        if (PhysicalDeviceCount > 0)
        {
            std::vector<VkPhysicalDevice> PhysicalDevices(PhysicalDeviceCount);
            vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, PhysicalDevices.data());
            for (VkPhysicalDevice& PhysicalDevice : PhysicalDevices)
            {
                uint32_t NumQueueFamily = 0;
                uint32_t NumLayers = 0;
                uint32_t NumExtensions = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &NumQueueFamily, nullptr);
                vkEnumerateDeviceLayerProperties(PhysicalDevice, &NumLayers, nullptr);
                vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &NumExtensions, nullptr);

                if (NumQueueFamily == 0 || NumExtensions == 0 || (NumLayers == 0 && EnableDebugLayer))
                {
                    continue;
                }

                if (EnableDebugLayer && NumLayers > 0)
                {
                    bool bFoundDebugLayer = false;
                    std::vector<VkLayerProperties> DevicesLayerProperties(NumLayers);
                    vkEnumerateDeviceLayerProperties(PhysicalDevice, &NumLayers, DevicesLayerProperties.data());
                    for (const VkLayerProperties& DeviceLayerProperty : DevicesLayerProperties)
                    {
                        if (strcmp(DeviceLayerProperty.layerName, ENGINE_VK_VALIDATION_LAYER_NAME) == 0)
                        {
                            bFoundDebugLayer = true;
                            break;
                        }
                    }
                    if (!bFoundDebugLayer)
                    {
                        continue;
                    }
                }

                {
                    bool bFoundSwapchainExtension = false;
                    std::vector<VkExtensionProperties> Extensions(NumExtensions);
                    vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &NumExtensions, Extensions.data());
                    
                    for (VkExtensionProperties& Extension : Extensions)
                    {
                        if (strcmp(Extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
                        {
                            bFoundSwapchainExtension = true;
                            break;
                        }
                    }
                    if (!bFoundSwapchainExtension)
                    {
                        continue;
                    }
                }


                VulkanPhysicalDeviceInfo PhysicalDeviceInfo;
                PhysicalDeviceInfo.Device = PhysicalDevice;
                vkGetPhysicalDeviceProperties(PhysicalDevice, &PhysicalDeviceInfo.Properties);
                vkGetPhysicalDeviceFeatures(PhysicalDevice, &PhysicalDeviceInfo.Features);

                uint32_t PipelineSupport = 0;
                std::vector<VkQueueFamilyProperties> DeviceQueueFamilyProperties(NumQueueFamily);
                vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &NumQueueFamily, DeviceQueueFamilyProperties.data());

                int32_t QueueFamilyIndex = 0;
                for (VkQueueFamilyProperties& QueueFamilyProperties : DeviceQueueFamilyProperties)
                {
                    VulkanQueueFamilyInfo QueueFamilyInfo;
                    QueueFamilyInfo.Index = QueueFamilyIndex;
                    QueueFamilyInfo.Properties = QueueFamilyProperties;
                    PhysicalDeviceInfo.QueueFamilyInfos.emplace_back(QueueFamilyInfo);

                    uint32_t QueueIndex = 0;
                    if (PhysicalDeviceInfo.GraphicQueueFamilyIndex == kQueueFamilyNotFound && QueueFamilyInfo.SupportGraphic())
                    {
                        PhysicalDeviceInfo.GraphicQueueFamilyIndex = QueueFamilyInfo.Index;
                        PhysicalDeviceInfo.GraphicQueueIndex = QueueIndex;
                        if (QueueFamilyInfo.NumQueues() > QueueIndex + 1)
                        {
                            QueueIndex++;
                        }
                    }

                    if (QueueFamilyInfo.SupportCompute())
                    {
                        bool bComputeQueueFamilyNotAssigned = PhysicalDeviceInfo.ComputeQueueFamilyIndex == kQueueFamilyNotFound;
                        bool bComputeOnlyQueueFamily = !QueueFamilyInfo.SupportGraphic() && PhysicalDeviceInfo.ComputeQueueFamilyIndex == PhysicalDeviceInfo.GraphicQueueFamilyIndex;
                        if (bComputeQueueFamilyNotAssigned || bComputeOnlyQueueFamily)
                        {
                            PhysicalDeviceInfo.ComputeQueueFamilyIndex = QueueFamilyInfo.Index;
                            PhysicalDeviceInfo.ComputeQueueIndex = QueueIndex;
                            if (QueueFamilyInfo.NumQueues() > QueueIndex + 1)
                            {
                                QueueIndex++;
                            }
                        }
                    }

                    if (QueueFamilyInfo.SupportTransfer())
                    {
                        bool bTransferQueueFamilyNotAssigned = PhysicalDeviceInfo.TransferQueueFamilyIndex == kQueueFamilyNotFound;
                        bool bTransferOnlyQueueFamily = !QueueFamilyInfo.SupportGraphic() && !QueueFamilyInfo.SupportCompute() && PhysicalDeviceInfo.TransferQueueFamilyIndex == PhysicalDeviceInfo.ComputeQueueFamilyIndex;

                        if (bTransferQueueFamilyNotAssigned || bTransferOnlyQueueFamily)
                        {
                            PhysicalDeviceInfo.TransferQueueFamilyIndex = QueueFamilyInfo.Index;
                            PhysicalDeviceInfo.TransferQueueIndex = QueueIndex;
                        }
                    }
                    QueueFamilyIndex++;
                }

                if (PhysicalDeviceInfo.QueueFamilyInfos.size() > 0)
                {
                    EstimatePhysicalDeviceScore(PhysicalDeviceInfo);
                    if (PhysicalDeviceInfo.EstimateScore > 0)
                    {
                        DeviceInfos.emplace_back(PhysicalDeviceInfo);
                    }
                }
            }
        }

        return DeviceInfos;
    }

    GraphicDevice* GraphicModuleVulkan::CreateDevice()
    {
        VkPhysicalDevice PhyiscalDevice = nullptr;

        std::vector<VulkanPhysicalDeviceInfo> PhysicalDevices = EnumerateAvailableVulkanPhysicalDevices(mVulkanInstance, mEnableDebugLayer);
        if (PhysicalDevices.size() == 0)
        {
            return nullptr;
        }

        uint32_t MaxScore = PhysicalDevices[0].EstimateScore;
        size_t MaxScoreIndex = 0;
        for (size_t Index = 1; Index < PhysicalDevices.size(); Index++)
        {
            const VulkanPhysicalDeviceInfo& Info = PhysicalDevices[Index];
            if (Info.EstimateScore > MaxScore)
            {
                MaxScoreIndex = Index;
            }
            else if (Info.EstimateScore == MaxScore)
            {
                const VulkanPhysicalDeviceInfo& MaxScoreInfo = PhysicalDevices[MaxScoreIndex];
                const bool MaxDedicatedCompute = (MaxScoreInfo.ComputeQueueFamilyIndex != MaxScoreInfo.GraphicQueueFamilyIndex);
                const bool CurDedicatedCompute = (Info.ComputeQueueFamilyIndex != Info.GraphicQueueFamilyIndex);
                const bool MaxDedicatedTransfer = (MaxScoreInfo.TransferQueueFamilyIndex != MaxScoreInfo.ComputeQueueFamilyIndex);
                const bool CurDedicatedTransfer = (Info.TransferQueueFamilyIndex != Info.ComputeQueueFamilyIndex);

                if (CurDedicatedTransfer && !MaxDedicatedTransfer)
                {
                    MaxScoreIndex = Index;
                }
                else if (CurDedicatedTransfer == MaxDedicatedTransfer && CurDedicatedCompute && !MaxDedicatedCompute)
                {
                    MaxScoreIndex = Index;
                }
            }
        }
        const VulkanPhysicalDeviceInfo& Info = PhysicalDevices[MaxScoreIndex];

        //TODO: 
        std::vector<VkDeviceQueueCreateInfo> DeviceQueueCreateInfos;
        uint32_t FamilyIndices[] = {
            Info.GraphicQueueFamilyIndex,
            Info.ComputeQueueFamilyIndex,
            Info.TransferQueueFamilyIndex
        };
        const float QueuePriorities[] = { 1.0f, 1.0f, 1.0f };
        const float* QueuePrioritiesOffset = QueuePriorities;
        for (uint32_t FamilyIndex : FamilyIndices)
        {
            bool FoundInQueue = false;
            for (VkDeviceQueueCreateInfo& Info : DeviceQueueCreateInfos)
            {
                if (Info.queueFamilyIndex == FamilyIndex)
                {
                    Info.queueCount++;
                    QueuePrioritiesOffset++;
                    FoundInQueue = true;
                    break;
                }
            }
            if (!FoundInQueue)
            {
                VkDeviceQueueCreateInfo DeviceQueueCreateInfo;
                VulkanZeroMemory(DeviceQueueCreateInfo);
                DeviceQueueCreateInfo.queueCount = 1;
                DeviceQueueCreateInfo.queueFamilyIndex = FamilyIndex;
                DeviceQueueCreateInfo.pQueuePriorities = QueuePrioritiesOffset;
                DeviceQueueCreateInfos.emplace_back(DeviceQueueCreateInfo);
            }
        }

        VkDeviceCreateInfo DeviceCreateInfo;
        VulkanZeroMemory(DeviceCreateInfo, VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);
        DeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(DeviceQueueCreateInfos.size());
        DeviceCreateInfo.pQueueCreateInfos = DeviceQueueCreateInfos.data();
        DeviceCreateInfo.pEnabledFeatures = &Info.Features;
        const char* Layers[] = { ENGINE_VK_VALIDATION_LAYER_NAME };
        const char* Extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        if (mEnableDebugLayer)
        {
            DeviceCreateInfo.enabledLayerCount = 1;
            DeviceCreateInfo.ppEnabledLayerNames = Layers;
        }
        DeviceCreateInfo.enabledExtensionCount= 1;
        DeviceCreateInfo.ppEnabledExtensionNames = Extensions;

        VkDevice VulkanDevice;
        VkResult RetCreateDevice = vkCreateDevice(Info.Device, &DeviceCreateInfo, GFX_VK_ALLOCATION_CALLBACK, &VulkanDevice);

        if (RetCreateDevice == VkResult::VK_SUCCESS)
        {
            return new GraphicDeviceVulkan(this, Info.Device, VulkanDevice,
                Info.GraphicQueueFamilyIndex,  Info.GraphicQueueIndex,
                Info.ComputeQueueFamilyIndex,  Info.ComputeQueueIndex,
                Info.TransferQueueFamilyIndex, Info.TransferQueueIndex);
        }

        return nullptr;
    }

    void GraphicModuleVulkan::Release()
    {
        delete this;
    }

    VkInstance GraphicModuleVulkan::CreateVkInstance(bool bEnableDebugLayer)
    {
        uint32_t NumLayers = 0;
        uint32_t NumExtensions = 0;
        vkEnumerateInstanceLayerProperties(&NumLayers, nullptr);
        vkEnumerateInstanceExtensionProperties(nullptr, &NumExtensions, nullptr);

        std::vector<const char*> RequiredLayers;
        std::vector<const char*> RequiredExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
        std::vector<const char*> FoundDebugLayerNames;
        std::vector<const char*> FoundRequiredExtensions;

        if (bEnableDebugLayer)
        {
            mEnableDebugLayer = bEnableDebugLayer;
            RequiredLayers.emplace_back(ENGINE_VK_VALIDATION_LAYER_NAME);
        }

        if (NumLayers > 0)
        {
            std::vector<VkLayerProperties> LayerProperties(NumLayers);
            vkEnumerateInstanceLayerProperties(&NumLayers, LayerProperties.data());

            for (const VkLayerProperties& LayerProperty : LayerProperties)
            {
                for (const char* RequiredLayerName : RequiredLayers)
                {
                    if (strcmp(LayerProperty.layerName, RequiredLayerName) == 0)
                    {
                        FoundDebugLayerNames.push_back(RequiredLayerName);
                    }
                }
            }

            if (FoundDebugLayerNames.size() != RequiredLayers.size())
                return nullptr;
        }

        if (NumExtensions > 0)
        {
            std::vector<VkExtensionProperties> ExtensionProperties(NumExtensions);
            vkEnumerateInstanceExtensionProperties(nullptr, &NumLayers, ExtensionProperties.data());

            for (const VkExtensionProperties& ExtensionProperty : ExtensionProperties)
            {
                for (const char* RequiredExtensinName : RequiredExtensions)
                {
                    if (strcmp(ExtensionProperty.extensionName, RequiredExtensinName) == 0)
                    {
                        FoundRequiredExtensions.emplace_back(RequiredExtensinName);
                    }
                }
            }

            if (FoundRequiredExtensions.size() != RequiredExtensions.size())
                return nullptr;
        }

        VkApplicationInfo ApplicationInfo;
        VulkanZeroMemory(ApplicationInfo);
        ApplicationInfo.pEngineName = "GameEngine";
        ApplicationInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo CreateInfo;
        VulkanZeroMemory(CreateInfo);
        CreateInfo.pApplicationInfo = &ApplicationInfo;

        if (FoundDebugLayerNames.size() > 0)
        {
            CreateInfo.enabledLayerCount    = static_cast<uint32_t>(FoundDebugLayerNames.size());
            CreateInfo.ppEnabledLayerNames  = FoundDebugLayerNames.data();
        }

        if (FoundRequiredExtensions.size() > 0)
        {
            CreateInfo.enabledExtensionCount    = static_cast<uint32_t>(FoundRequiredExtensions.size());
            CreateInfo.ppEnabledExtensionNames  = FoundRequiredExtensions.data();
        }

        VkInstance VulkanInstance = nullptr;
        VkResult RetCreateVulkanInstance = vkCreateInstance(&CreateInfo, GFX_VK_ALLOCATION_CALLBACK, &VulkanInstance);

        if (RetCreateVulkanInstance == VkResult::VK_SUCCESS)
        {
            return VulkanInstance;
        }
        else
        {
            return nullptr;
        }
    }
}

extern "C" GfxInterfaceAPI GFXI::GraphicModule* CreateGfxModule()
{
    return new GFXI::GraphicModuleVulkan();
}
