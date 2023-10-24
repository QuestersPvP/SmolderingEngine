#include "Common.h"

#include <fstream>
#include <iostream>


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
    bool GetBinaryFileContents(std::string const& _filename, std::vector<unsigned char>& _contents)
    {
        _contents.clear();

        std::ifstream file(_filename, std::ios::binary);
        
        if (file.fail()) 
        {
            std::cout << "Could not open '" << _filename << "' file." << std::endl;
            return false;
        }

        std::streampos begin;
        std::streampos end;
        begin = file.tellg();
        file.seekg(0, std::ios::end);
        end = file.tellg();

        if ((end - begin) == 0) 
        {
            std::cout << "The '" << _filename << "' file is empty." << std::endl;
            return false;
        }
        _contents.resize(static_cast<size_t>(end - begin));
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(_contents.data()), end - begin);
        file.close();

        return true;
    }
};