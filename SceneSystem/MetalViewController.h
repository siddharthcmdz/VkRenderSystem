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
NS_ASSUME_NONNULL_BEGIN

@interface MetalViewController : UIViewController
@property(nonatomic, weak) IBOutlet UILabel *northLabel;
@property(nonatomic, weak) IBOutlet UILabel *eastLabel;
@property(nonatomic, weak) IBOutlet UILabel *southLabel;
@property(nonatomic, weak) IBOutlet UILabel *westLabel;

@property(nonatomic) int ctxID;
@property(nonatomic, strong) NSString *cameraView;
+ (MetalViewController *)loadFromNib:(NSString *)CameraView;
- (void)redraw;
- (int)getContextID;
- (void)setContextID:(int)id;
- (void)render;
- (void)updateCamera:(NSString *)cameraView;
- (void)rotateClockwise;
- (void)rotateCounterClockwise;
- (void)updateNorthLabel:(NSString *)newValue;
- (void)updateEastLabel:(NSString *)newValue;
- (void)updateSouthLabel:(NSString *)newValue;
- (void)updateWestLabel:(NSString *)newValue;
- (void)flipVertical;
- (void)flipHorizontal;
@end

NS_ASSUME_NONNULL_END
/** The Metal-compatibile view for the demo Storyboard. */
@interface MetalView : UIView
@end
