#include "Common.h"

#include <fstream>
#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include "../../Dependencies/tiny_obj_loader.h"


namespace SmolderingEngine
{
    Matrix4x4 operator*(Matrix4x4 const& _left, Matrix4x4 const& _right)
    {
        return 
        {
            _left[0] * _right[0] + _left[4] * _right[1] + _left[8] * _right[2] + _left[12] * _right[3],
            _left[1] * _right[0] + _left[5] * _right[1] + _left[9] * _right[2] + _left[13] * _right[3],
            _left[2] * _right[0] + _left[6] * _right[1] + _left[10] * _right[2] + _left[14] * _right[3],
            _left[3] * _right[0] + _left[7] * _right[1] + _left[11] * _right[2] + _left[15] * _right[3],
                                                          
            _left[0] * _right[4] + _left[4] * _right[5] + _left[8] * _right[6] + _left[12] * _right[7],
            _left[1] * _right[4] + _left[5] * _right[5] + _left[9] * _right[6] + _left[13] * _right[7],
            _left[2] * _right[4] + _left[6] * _right[5] + _left[10] * _right[6] + _left[14] * _right[7],
            _left[3] * _right[4] + _left[7] * _right[5] + _left[11] * _right[6] + _left[15] * _right[7],
                                                          
            _left[0] * _right[8] + _left[4] * _right[9] + _left[8] * _right[10] + _left[12] * _right[11],
            _left[1] * _right[8] + _left[5] * _right[9] + _left[9] * _right[10] + _left[13] * _right[11],
            _left[2] * _right[8] + _left[6] * _right[9] + _left[10] * _right[10] + _left[14] * _right[11],
            _left[3] * _right[8] + _left[7] * _right[9] + _left[11] * _right[10] + _left[15] * _right[11],

            _left[0] * _right[12] + _left[4] * _right[13] + _left[8] * _right[14] + _left[12] * _right[15],
            _left[1] * _right[12] + _left[5] * _right[13] + _left[9] * _right[14] + _left[13] * _right[15],
            _left[2] * _right[12] + _left[6] * _right[13] + _left[10] * _right[14] + _left[14] * _right[15],
            _left[3] * _right[12] + _left[7] * _right[13] + _left[11] * _right[14] + _left[15] * _right[15]
        };
    }

    float Deg2Rad(float _value)
    {
        return _value * 0.01745329251994329576923690768489f;
    }

    Vector3 Normalize(Vector3 const& _vector)
    {
        float length = std::sqrt(_vector[0] * _vector[0] + _vector[1] * _vector[1] + _vector[2] * _vector[2]);
        return { _vector[0] / length, _vector[1] / length, _vector[2] / length};
    }

    Matrix4x4 PrepareRotationMatrix(float _angle, Vector3 const& _axis, float _normalizeAxis)
    {
        float x;
        float y;
        float z;

        if (_normalizeAxis)
        {
            Vector3 normalized = Normalize(_axis);
            x = normalized[0];
            y = normalized[1];
            z = normalized[2];
        }
        else {
            x = _axis[0];
            y = _axis[1];
            z = _axis[2];
        }

        const float c = cos(Deg2Rad(_angle));
        const float _1_c = 1.0f - c;
        const float s = sin(Deg2Rad(_angle));

        Matrix4x4 rotationMatrix = 
        {
          x * x * _1_c + c,
          y * x * _1_c - z * s,
          z * x * _1_c + y * s,
          0.0f,

          x * y * _1_c + z * s,
          y * y * _1_c + c,
          z * y * _1_c - x * s,
          0.0f,

          x * z * _1_c - y * s,
          y * z * _1_c + x * s,
          z * z * _1_c + c,
          0.0f,

          0.0f,
          0.0f,
          0.0f,
          1.0f
        };

        return rotationMatrix;
    }

    Matrix4x4 PrepareTranslationMatrix(float _x, float _y, float _z)
    {
        return    
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
              _x,   _y,   _z, 1.0f
        };
    }

    Matrix4x4 PreparePerspectiveProjectionMatrix(float _aspectRatio, float _fieldOfView, float _nearPlane, float _farPlane)
    {
        float f = 1.0f / tan(Deg2Rad(0.5f * _fieldOfView));

        return 
        {
            f / _aspectRatio,
            0.0f,
            0.0f,
            0.0f,

            0.0f,
            -f,
            0.0f,
            0.0f,

            0.0f,
            0.0f,
            _farPlane / (_nearPlane - _farPlane),
            -1.0f,

            0.0f,
            0.0f,
            (_nearPlane * _farPlane) / (_nearPlane - _farPlane),
            0.0f
        };
    }

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
                _mesh.data.emplace_back(attribs.vertices[3 * index.vertex_index + 0]);
                _mesh.data.emplace_back(attribs.vertices[3 * index.vertex_index + 1]);
                _mesh.data.emplace_back(attribs.vertices[3 * index.vertex_index + 2]);
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