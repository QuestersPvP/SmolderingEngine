#include "Common.h"

namespace SmolderingEngine
{
    bool IsExtensionSupported(std::vector<VkExtensionProperties> const& _availableExtensions, char const* const _extension)
    {
        for (auto& availableExtension : _availableExtensions)
        {
            if (strstr(availableExtension.extensionName, _extension))
                return true;
        }
        return false;
    }
};