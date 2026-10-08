#import <UIKit/UIKit.h>
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>

@interface GameViewController : UIViewController

+ (GameViewController *)sharedInstance;

- (void)showKeyboard:(const char *)initialText;
- (void)hideKeyboard;

- (void)showWebPage:(const char *)url;
- (void)hideWebPage;
- (BOOL)isWebShowing;

- (void)setJoystickZoneWithX:(float)x y:(float)y width:(float)width height:(float)height;

@end
