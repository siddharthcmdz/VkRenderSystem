
#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include "BoundingBox.h"
#include "ssenums.h"

namespace ss
{
    /**
     * @brief A utility class to control the camera of the scene. This can be constrrained to 2D or to 3D.
     */
    class Camera final
    {
    private:
        constexpr static float DEFAULT_PERSP_FOVY = 45.0f;
        constexpr static float DEFAULT_ORTHO_LEFT = -1.0f;
        constexpr static float DEFAULT_ORTHO_RIGHT = 1.0f;
        constexpr static float DEFAULT_ORTHO_BOTTOM = -1.0f;
        constexpr static float DEFAULT_ORTHO_TOP = 1.0f;
        constexpr static float DEFAULT_ZNEAR = 0.001f;
        constexpr static float DEFAULT_ZFAR = 10000.0f;
        constexpr static float DEFAULT_ZOOM_SCALE = 1.0f;

        struct
        {
            float fov = DEFAULT_PERSP_FOVY;
            float znear = DEFAULT_ZNEAR, zfar = DEFAULT_ZFAR;
        } _perspective;
        
        struct
        {
            float left = DEFAULT_ORTHO_LEFT, right = DEFAULT_ORTHO_RIGHT;
            float bottom = DEFAULT_ORTHO_BOTTOM, top = DEFAULT_ORTHO_TOP;
            float near = DEFAULT_ZNEAR, far = DEFAULT_ZFAR;
            float zoomscale = DEFAULT_ZOOM_SCALE;
        } _orthographic;

        struct
        {
            glm::mat4 projection;
            glm::mat4 view;
        } _matrices;

        bool updated = false;
        bool flipY = true;

        BoundingBox _worldBounds;
        glm::mat4 _defaultOrientation;
        ViewType _viewType = ViewType::vtInvalid;
        ProjectionType _projType = ProjectionType::ptInvalid;
        glm::vec3 _rotation = glm::vec3(0.0f);
        glm::vec3 _position = glm::vec3(0, 0, 0.5);
        glm::vec4 _viewPos = glm::vec4();
        glm::mat4 _obliqueMat = glm::mat4(1.0f);

        /**
         * @brief Utility method to update the camera's view matrix.
         */
        void updateViewMatrix();
        bool isAnatomicalView() const;
        
    public:
        float rotationSpeed = 1.0f;
        
        /**
         * @brief Constructs the default camera.
         */
        Camera() = default;
        
        /**
         * @brief Constructs a camera based on the view type.
         * @param vt the specified view type.
         */
        Camera(ViewType vt);
        
        /**
         * @brief Validates if the view is currently being animated
         * @return true if view is animating, false otherwise
         */
        bool isAnimating() const;
        
        /**
         * @brief Gets the near clip plane of the view frustrum.
         * @return the near clip plane of the view frustrum.
         */
        float getNearClip() const;
        
        /**
         * @brief Gets the far clip plane of the view fustrum.
         * @return the far clip plane of the view frustrum.
         */
        float getFarClip() const;
        
        /**
         * @brief Sets the perspective projection on the camera.
         * @param fov the specified field of view of the camera.
         * @param aspect the specified aspect ratio of the the surface onto which the view is rendered.
         * @param znear the near clip plane of the view frustrum.
         * @param zfar the far clip plane of the view frustrum.
         */
        void setPerspective(float fov, float aspect, float znear, float zfar);
        
        /**
         * @brief Sets the orthographics projection on the camera.
         * @param left the specified left extent of the parallel frustrum
         * @param right the specified right extent of the parallel frustrum
         * @param bottom the specified bottom extent of the parallel frustrum
         * @param top the specified top extent of the parallel frustrum
         * @param znear the specified near clipping plane
         * @param zfar the specified far clipping plane
         */
        void setOrthographic(float left, float right, float bottom, float top, float znear, float zfar);
        
        /**
         * @brief Sets the scale for the view. This is specifically used for orthographic projection
         * @param viewScale the specified view scale
         */
        void setViewScale(float viewScale);
        
        /**
         * @brief a convenient method to update the aspect ratio of the view when resizing or changes in orientation of the surface.
         * @param aspect the specified aspect ratio of the view.
         */
        void updateAspectRatio(float aspect);
        
        /**
         * @brief Sets the world bounds on the camera. This is needed to zoom to fit.
         * @param worldBounds the specified world's bounding box
         */
        void setWorldBounds(BoundingBox& worldBounds);
        
        /**
         * @brief Gets the world bounds that was previously set on the camera.
         * @return the bounding box.
         */
        BoundingBox getWorldBounds() const;
        
        /**
         * @brief Sets the position of the camera that allows it to pan in 3 or 2 space.
         * @param position the location of the camera in world space.
         */
        void setPosition(glm::vec3 position);
        
        /**
         * @brief Sets the rotation of the camera
         * @param rotation the specified angle of rotation in degrees along each axis.
         */
        void setRotation(glm::vec3 rotation);
        
        /**
         * @brief Sets the rotation matrix for an oblique view of an anatomical view. Currently this affects only axial and sagittal views.
         * @param oblique the specified 3x3 rotation matrix
         */
        void setOblique(glm::mat3 oblique);
        
        /**
         * @brief Rotates the camera matrix by a delta amount
         * @param delta the delta angle along each axis.
         */
        void rotate(glm::vec3 delta);
        
        /**
         * @brief Sets the absolute translation value on the camera matrix. This moves the camera position and the target.
         * @param translation the specified translation amount.
         */
        void setTranslation(glm::vec3 translation);
        
        /**
         * @brief Sets the relative translation value on the camera matrix.
         * @param delta the specified delta of translation
         */
        void translate(glm::vec3 delta);
        
        /**
         * @brief Sets a multiplier factor for the rotation speed.
         * @param rotationSpeed the specified multiplier factor.
         */
        void setRotationSpeed(float rotationSpeed);
        
        /**
         * @brief Gets the camera's current view matrix.
         * @return the view matrix.
         */
        glm::mat4 getViewMatrix() const;
        
        /**
         * @brief Gets the view type
         * @return the view type.
         */
        ViewType getViewType() const;
        
        /**
         * @brief Gets the type of projection for this camera which could be either perspective or orthographic.
         * @return the type of projection
         */
        ProjectionType getProjectionType() const;
        
        /**
         * @brief Gets the camera's current projection matrix.
         * @return the projection matrix.
         */
        glm::mat4 getProjectionMatrix() const;
    };
}
