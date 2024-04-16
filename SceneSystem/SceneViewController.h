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

#import <UIKit/UIKit.h>
#import "Image3dModel.h"
#import "MetalViewController.h"
#import "PoseModel.h"

NS_ASSUME_NONNULL_BEGIN

@interface SceneViewController : UIViewController

@property(nonatomic) Boolean gameloopOn;
@property(nonatomic) Boolean enableDebugViz;
@property(nonatomic) Boolean loadWithoutData;
@property(atomic, strong) NSMutableDictionary *trackables;
@property(atomic, strong) NSMutableArray *metalControllers;
@property(atomic, strong) NSData *seriesFileData;
@property(atomic) Image3d seriesData;
@property(atomic, strong) NSMutableArray *swiftUIMetalControllers;
+ (SceneViewController *)sharedInstance;
- (void)addMetalController:(MetalViewController *)controller;
- (void)removeAllMetalControllers;
- (void)removeMetalController:(MetalViewController *)controller;
- (void)appWillTerminate;
- (void)viewsResized;
- (void)renderViewController:(MetalViewController *)controller;
- (void)handleTap:(UITapGestureRecognizer *)sender withController:(MetalViewController *)controller;
- (void)handleDrag:(UIPanGestureRecognizer *)sender withController:(MetalViewController *)controller;
- (void)twoFingerDrag:(UIPanGestureRecognizer *)sender withController:(MetalViewController *)controller;
- (void)pinchZoom:(UIPinchGestureRecognizer *)sender withController:(MetalViewController *)controller;
- (void)longpress:(UILongPressGestureRecognizer *)sender withController:(MetalViewController *)controller;
- (void)updateCamera:(NSString *)cameraView withController:(MetalViewController *)controller;
- (void)turnOnGameloop;
- (void)turnOffGameloop;
- (void)disableObliqueSlices;
- (void)enableObliqueSlices;
- (void)rotateClockwise:(MetalViewController *)controller;
- (void)rotateCounterClockwise:(MetalViewController *)controller;
- (void)swapDrawable:(int)drawable;
- (void)keyBoardInput:(NSString *)key;
- (void)seriesRecieved:(Image3d)seriesData withFileData:(NSData *)data;
- (void)updateWindow:(double)window;
- (void)updateLevel:(double)level;
- (void)updatedPose:(Pose)newPose;
- (id)getTrackalbeByID:(int)id;
- (void)setTrackableById:(NSData *)trackable withID:(int)trackableID;
- (void)clearPose;
- (void)setDebugViz:(Boolean)enableDebugViz;
- (void)updateAxialSlice:(CGFloat)value;
- (void)updateSagitalSlice:(CGFloat)value;
- (void)updateCoronalSlice:(CGFloat)value;
- (void)flipVertical:(MetalViewController *)controller;
- (void)flipHorizontal:(MetalViewController *)controller;
@end

NS_ASSUME_NONNULL_END
