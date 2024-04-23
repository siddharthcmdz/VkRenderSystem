
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
