#include "Common.h"

#include <fstream>
#include <iostream>

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
};