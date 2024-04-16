
#pragma once

namespace ss {
    
    /**
     * @brief Describes a volume slice's orientation
     */
    enum VolumeSliceOrientation
    {
        vsoAxial,
        vsoSagittal,
        vsoCoronal,
        vsoInvalid
    };

    /**
     * @brief Describes orientation of an orthogonal axis
     */
    enum Axis
    {
        X,
        Y,
        Z,
        Invalid
    };

    /**
     * @brief Describes the view type
     */
    enum ViewType
    {
        vtAxial,
        vtSaggital,
        vtCoronal,
        vt3D,
        vtScrewGuide,
        vtLandingGuide,
        vtInvalid
    };

    /**
     * @brief Describes the gesture type.
     */
    enum GestureType
    {
        gtOneFinger,
        gtTwoFinger,
        gtPinch
    };

    /**
     * @brief Describes the navigation type.
     */
    enum NavigationType
    {
        ntRotate,
        ntZoom,
        ntPan,
        ntUnknown
    };

    /**
     * @brief Describes the drawable type.
     */
    enum DrawableType 
    {
        dtQuadrics,
        dtMultiQuadrics,
        dtGLTFmodel,
        dtBenchmark,
        dtGizmo2d,
        dtVolumeSlice,
        dtMain,
        snMax
    };

    /**
     * @brief Describes the type of projection matrix
     */
    enum ProjectionType
    {
        ptPerspective,
        ptOrthographic,
        ptInvalid
    };

}
