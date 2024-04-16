//// =================================================================================
// Copyright (c) 2023, See All Surgical Inc. All rights reserved.
//
// This software is the property of See All Surgical Inc. The software may not be reproduced,
// modified, distributed, or transferred without the express written permission of See All Surgical Inc.
//
// In no event shall See All Surgical Inc. be liable for any claim, damages or other liability,
// whether in an action of contract, tort or otherwise, arising from, out of or in connection with the software
// or the use or other dealings in the software.
// =================================================================================

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
