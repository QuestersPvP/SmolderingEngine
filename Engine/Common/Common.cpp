#include "Common.h"

#include <fstream>
#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include "../../Dependencies/tiny_obj_loader.h"


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

    bool Load3DModelFromObjFile(char const* _filename, bool _loadNormals, bool _loadTexcoords, bool _generateTangentSpaceVectors, bool _unify, Mesh& _mesh, uint32_t* _vertexStride)
    {
        // Load model
        tinyobj::attrib_t                   attribs;
        std::vector<tinyobj::shape_t>       shapes;
        std::vector<tinyobj::material_t>    materials;
        std::string                         warning;
        std::string                         error;

        bool result = tinyobj::LoadObj(&attribs, &shapes, &materials, &warning, &error, _filename);
        if (!result) 
        {
            std::cout << "Could not open the '" << _filename << "' file.";
            if (0 < error.size()) 
            {
                std::cout << " " << error;
            }
            std::cout << std::endl;
            return false;
        }

        // Normal vectors and texture coordinates are required to generate tangent and bitangent vectors
        if (!_loadNormals || !_loadTexcoords) 
        {
            _generateTangentSpaceVectors = false;
        }

        // Load model data and unify (normalize) its size and position
        float minX = attribs.vertices[0];
        float maxX = attribs.vertices[0];
        float minY = attribs.vertices[1];
        float maxY = attribs.vertices[1];
        float minZ = attribs.vertices[2];
        float maxZ = attribs.vertices[2];

        _mesh = {};
        uint32_t offset = 0;
        for (auto& shape : shapes) 
        {
            uint32_t partOffset = offset;

            for (auto& index : shape.mesh.indices) 
            {
                _mesh.data.emplace_back(attribs.vertices[3 * index.vertex_index + 1]);
                _mesh.data.emplace_back(attribs.vertices[3 * index.vertex_index + 2]);
                _mesh.data.emplace_back(attribs.vertices[3 * index.vertex_index + 0]);
                ++offset;

                if (_loadNormals) 
                {
                    if (attribs.normals.size() == 0)
                    {
                        std::cout << "Could not load normal vectors data in the '" << _filename << "' file.";
                        return false;
                    }
                    else 
                    {
                        _mesh.data.emplace_back(attribs.normals[3 * index.normal_index + 0]);
                        _mesh.data.emplace_back(attribs.normals[3 * index.normal_index + 1]);
                        _mesh.data.emplace_back(attribs.normals[3 * index.normal_index + 2]);
                    }
                }

                if (_loadTexcoords) 
                {
                    if (attribs.texcoords.size() == 0) 
                    {
                        std::cout << "Could not load texture coordinates data in the '" << _filename << "' file.";
                        return false;
                    }
                    else 
                    {
                        _mesh.data.emplace_back(attribs.texcoords[2 * index.texcoord_index + 0]);
                        _mesh.data.emplace_back(attribs.texcoords[2 * index.texcoord_index + 1]);
                    }
                }

                if (_generateTangentSpaceVectors) 
                {
                    // Insert temporary tangent space vectors data
                    for (int i = 0; i < 6; ++i) 
                    {
                        _mesh.data.emplace_back(0.0f);
                    }
                }

                if (_unify) 
                {
                    if (attribs.vertices[3 * index.vertex_index + 0] < minX) 
                    {
                        minX = attribs.vertices[3 * index.vertex_index + 0];
                    }
                    if (attribs.vertices[3 * index.vertex_index + 0] > maxX) 
                    {
                        maxX = attribs.vertices[3 * index.vertex_index + 0];
                    }
                    if (attribs.vertices[3 * index.vertex_index + 1] < minY) 
                    {
                        minY = attribs.vertices[3 * index.vertex_index + 1];
                    }
                    if (attribs.vertices[3 * index.vertex_index + 1] > maxY) 
                    {
                        maxY = attribs.vertices[3 * index.vertex_index + 1];
                    }
                    if (attribs.vertices[3 * index.vertex_index + 2] < minZ) 
                    {
                        minZ = attribs.vertices[3 * index.vertex_index + 2];
                    }
                    if (attribs.vertices[3 * index.vertex_index + 2] > maxZ) 
                    {
                        maxZ = attribs.vertices[3 * index.vertex_index + 2];
                    }
                }
            }

            uint32_t partVertexCount = offset - partOffset;
            if (0 < partVertexCount)
            {
                _mesh.parts.push_back({ partOffset, partVertexCount });
            }
        }

        uint32_t stride = 3 + (_loadNormals ? 3 : 0) + (_loadTexcoords ? 2 : 0) + (_generateTangentSpaceVectors ? 6 : 0);
        if (_vertexStride) 
        {
            *_vertexStride = stride * sizeof(float);
        }

        //if (_generateTangentSpaceVectors)
        //{
        //    GenerateTangentSpaceVectors(_mesh);
        //}

        if (_unify) 
        {
            float offsetX = 0.5f * (minX + maxX);
            float offsetY = 0.5f * (minY + maxY);
            float offsetZ = 0.5f * (minZ + maxZ);
            float scaleX = abs(minX - offsetX) > abs(maxX - offsetX) ? abs(minX - offsetX) : abs(maxX - offsetX);
            float scaleY = abs(minY - offsetY) > abs(maxY - offsetY) ? abs(minY - offsetY) : abs(maxY - offsetY);
            float scaleZ = abs(minZ - offsetZ) > abs(maxZ - offsetZ) ? abs(minZ - offsetZ) : abs(maxZ - offsetZ);
            float scale = scaleX > scaleY ? scaleX : scaleY;
            scale = scaleZ > scale ? 1.0f / scaleZ : 1.0f / scale;

            for (size_t i = 0; i < _mesh.data.size() - 2; i += stride) 
            {
                _mesh.data[i + 0] = scale * (_mesh.data[i + 0] - offsetX);
                _mesh.data[i + 1] = scale * (_mesh.data[i + 1] - offsetY);
                _mesh.data[i + 2] = scale * (_mesh.data[i + 2] - offsetZ);
            }
        }

        return true;
    }
};