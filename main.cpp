#define VOLK_IMPLEMENTATION
#include <volk.h>


#include <cassert>
#include <vector>
#include <cstring>
#include <iostream>

int main()
{
    VkResult res{volkInitialize()};
    assert(res == VK_SUCCESS);

    VkApplicationInfo application_info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = VK_MAKE_API_VERSION(0,1,3,0)
    };

    VkInstanceCreateInfo instance_create_info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &application_info,
        .enabledExtensionCount = 0,
    };

    VkInstance instance;
    res = vkCreateInstance(&instance_create_info, nullptr, &instance);
    assert(res == VK_SUCCESS);

    volkLoadInstanceOnly(instance);

    uint32_t device_count{};
    res = vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    assert(res == VK_SUCCESS);

    std::vector<VkPhysicalDevice> physical_devices{device_count};
    res = vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data());

    VkPhysicalDevice selected_physical_device;
    for (VkPhysicalDevice const& physical_device : physical_devices)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_device, &properties);

        if (strstr(properties.deviceName, "NVIDIA"))
        {
            selected_physical_device = physical_device;
            std::cout << "Selected physical device: " << properties.deviceName << '\n';
            break;
        }
    }

    {
        uint32_t extension_count{};
        res = vkEnumerateDeviceExtensionProperties(selected_physical_device,
            VK_NULL_HANDLE,
            &extension_count,
            nullptr);
        assert(res == VK_SUCCESS);

        std::vector<VkExtensionProperties> supported_extensions{extension_count};
        res = vkEnumerateDeviceExtensionProperties(selected_physical_device,
            VK_NULL_HANDLE,
            &extension_count,
            supported_extensions.data());
        assert(res == VK_SUCCESS);

        bool deferred_supported{};
        bool acceleration_supported{};
        for (VkExtensionProperties const& properties : supported_extensions)
        {
            if (strstr(properties.extensionName, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME))
            {
                deferred_supported = true;
                std::cout << VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME << " version " << properties.specVersion << '\n';
            }
            else if (strstr(properties.extensionName, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME))
            {
                acceleration_supported = true;
                std::cout << VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME << " version " << properties.specVersion << '\n';
            }
        }

        assert(deferred_supported == acceleration_supported);
    }

    {
        VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
        };

        VkPhysicalDeviceVulkan12Features device_12_features {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &acceleration_structure_features,
        };

        VkPhysicalDeviceFeatures2 device_features
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &device_12_features
        };

        vkGetPhysicalDeviceFeatures2(selected_physical_device, &device_features);
        std::cout << "descriptorIndexing: " << device_12_features.descriptorIndexing << '\n';
        std::cout << "bufferDeviceAddress: " << device_12_features.bufferDeviceAddress << '\n';
        std::cout << "accelerationStructure: " << acceleration_structure_features.accelerationStructure << '\n';

        assert(device_12_features.descriptorIndexing && device_12_features.bufferDeviceAddress && acceleration_structure_features.accelerationStructure);
    }

    float priority{1.0f};
    VkDeviceQueueCreateInfo queue_create_info {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .queueFamilyIndex = 0,
        .queueCount = 1,
        .pQueuePriorities = &priority
    };


    char const* extensions[] = { VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME };

    VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
        .accelerationStructure = VK_TRUE,
    };

    VkPhysicalDeviceVulkan12Features device_12_features {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &acceleration_structure_features,
        .descriptorIndexing = VK_TRUE,
        .bufferDeviceAddress = VK_TRUE,
    };

    VkPhysicalDeviceFeatures2 device_features
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &device_12_features
    };

    VkDeviceCreateInfo device_create_info {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &device_features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_create_info,
        .enabledExtensionCount = 2,
        .ppEnabledExtensionNames = extensions,
    };

    VkDevice device;
    res = vkCreateDevice(selected_physical_device, &device_create_info, nullptr, &device);
    assert(res == VK_SUCCESS);

    volkFinalize();
}
