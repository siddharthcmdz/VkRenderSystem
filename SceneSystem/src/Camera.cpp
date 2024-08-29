
#include "Camera.h"
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <iostream>
#include <algorithm>

namespace ss 
{
#define DEMO_ORIENTATION
    Camera::Camera(ViewType vt)
    {
        _viewType = vt;
        switch (vt)
        {
            case ViewType::vtFromFront:
#ifdef DEMO_ORIENTATION
            {
                glm::mat4 flipY = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));
                glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                _defaultOrientation = rotateY * flipY;
            }
#else
                _defaultOrientation = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));
#endif
                break;
                
            case ViewType::vtFromRight:
            {
#ifdef DEMO_ORIENTATION
                glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                _defaultOrientation = /*rotZ * rotX **/ rotY;

#else
                glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                _defaultOrientation = rotX * rotY;
#endif
                break;
            }
                
            case ViewType::vtFromTop:
                _defaultOrientation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                break;
                
            default:
                _defaultOrientation = glm::mat4(1.0f);
                break;
        }
        
        _projType = isAnatomicalView() ? ProjectionType::ptOrthographic : ProjectionType::ptPerspective;
        updateViewMatrix();
    }

    float Camera::getNearClip() const 
    {
        return _perspective.znear;
    }

    bool Camera::isAnatomicalView() const
    {
        return _viewType == ViewType::vtFromFront || _viewType == ViewType::vtFromTop || _viewType == ViewType::vtFromRight;
    }

    float Camera::getFarClip() const 
    {
        return _perspective.zfar;
    }

    void Camera::setViewScale(float viewScale)
    {
        const static float SCALE_HI_THRESHOLD = 5.0f;
        const static float SCALE_LOW_THRESHOLD = 0.15f;
        
        _orthographic.zoomscale = std::clamp(_orthographic.zoomscale + viewScale, SCALE_LOW_THRESHOLD, SCALE_HI_THRESHOLD);
        updateViewMatrix();
    }

    void Camera::setOrthographic(float left, float right, float bottom, float top, float znear, float zfar)
    {
        _orthographic.left = left;
        _orthographic.right = right;
        _orthographic.bottom = bottom;
        _orthographic.top = top;
        _orthographic.znear = znear;
        _orthographic.zfar = zfar;
        if(_projType == ProjectionType::ptOrthographic)
        {
            _matrices.projection = glm::ortho(left, right, bottom, top, znear, zfar);
//            std::cout<<"ortho: "<<glm::to_string(_matrices.projection)<<std::endl;
            if (flipY)
            {
                _matrices.projection[1][1] *= -1.0f;
            }
        }
    }

    void Camera::setPerspective(float fov, float aspect, float znear, float zfar) 
    {
        _perspective.fov = fov;
        _perspective.znear = znear;
        _perspective.zfar = zfar;
        if(_projType == ProjectionType::ptPerspective)
        {
            _matrices.projection = glm::perspective(glm::radians(fov), aspect, znear, zfar);
            if (flipY) 
            {
                _matrices.projection[1][1] *= -1.0f;
            }
        }
    }

    void Camera::updateAspectRatio(float aspect) 
    {
        _matrices.projection = glm::perspective(glm::radians(_perspective.fov), aspect, _perspective.znear, _perspective.zfar);
        if (flipY)
        {
            _matrices.projection[1][1] *= -1.0f;
        }
    }

    void Camera::setPosition(glm::vec3 position) 
    {
        _position = position;
        updateViewMatrix();
    }

    void Camera::setRotation(glm::vec3 rotation) 
    {
        _rotation = rotation;
        updateViewMatrix();
    }

    void Camera::rotate(glm::vec3 delta) 
    {
        _rotation += (delta * rotationSpeed);
        updateViewMatrix();
    }

    void Camera::setTranslation(glm::vec3 translation) 
    {
        _position = translation;
        updateViewMatrix();
    }

    void Camera::translate(glm::vec3 delta) {
        _position += delta;
        updateViewMatrix();
    }

    void Camera::setOblique(glm::mat3 oblique)
    {
        glm::mat4 oblique4 = glm::mat4(oblique);
        
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(oblique4, scale, rotation, translation, skew, perspective);
        float pitch = 0.0f, yaw = 0.0f, roll = 0.0f;
        glm::extractEulerAngleXYZ(oblique4, pitch, yaw, roll);
        
        if(_viewType == ViewType::vtFromFront)
        {
            _obliqueMat = glm::rotate(glm::mat4(1.0f), pitch, glm::vec3(1.0f, 0.0f, 0.0f));
        }
        else if(_viewType == ViewType::vtFromRight)
        {
//            _obliqueMat = glm::rotate(glm::mat4(1.0f), roll, glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 rot90 = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 oblique = glm::rotate(glm::mat4(1.0f), roll, glm::vec3(1.0f, 0.0f, 0.0f));
//            _obliqueMat = rot90 * axialolique;
//            _obliqueMat = glm::mat4(1.0f);
            _obliqueMat = oblique;
            
        }
        else
        {
            _obliqueMat = glm::mat4(1.0f);
        }
        
        updateViewMatrix();
    }

    void Camera::setRotationSpeed(float rotationSpeed) 
    {
        this->rotationSpeed = rotationSpeed;
    }

    glm::mat4 Camera::getViewMatrix() const 
    {
        return _matrices.view;
    }

    glm::mat4 Camera::getProjectionMatrix() const 
    {
        return _matrices.projection;
    }

    void Camera::setWorldBounds(BoundingBox& worldBounds)
    {
        _worldBounds = worldBounds;
        updateViewMatrix();
    }

    BoundingBox Camera::getWorldBounds() const
    {
        return _worldBounds;
    }

    ViewType Camera::getViewType() const
    {
        return _viewType;
    }

    ProjectionType Camera::getProjectionType() const
    {
        return _projType;
    }

    void Camera::updateViewMatrix()
    {
        glm::mat4 rotM = glm::mat4(1.0f);
        
        rotM = glm::rotate(rotM, glm::radians(_rotation.x * (flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
//        rotM = glm::rotate(rotM, glm::radians(_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        rotM = glm::rotate(rotM, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::vec3 translation = _position;
        if (flipY) 
        {
            translation.y *= -1.0f;
        }
        
        glm::mat4 transM = glm::translate(glm::mat4(1.0f), translation);
        glm::vec3 center = _worldBounds.getCenter();
        glm::mat4 originTransM = glm::translate(glm::mat4(1.0f), -center);
        glm::mat4 viewScaleMat(1.0f);
        if(_projType == ProjectionType::ptOrthographic)
        {
            viewScaleMat = glm::scale(glm::mat4(1.0f), glm::vec3(_orthographic.zoomscale));
        }
        glm::mat4 lookatMat = glm::mat4(1.0f);
        if (_viewType == ViewType::vtFromFront)
        {
            lookatMat = glm::mat4(_obliqueMat);
        }
        else if(_viewType == ViewType::vtFromRight)
        {
            lookatMat = glm::mat4(_obliqueMat);

        }
        _matrices.view = transM * rotM * viewScaleMat * lookatMat * _defaultOrientation * originTransM;
//        _matrices.view = transM * viewScaleMat * /*lookatMat **/ rotM * _defaultOrientation * originTransM;

        _viewPos = glm::vec4(_position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

        updated = true;
    }
}
