#import "AppDelegate.h"
#import "GameViewController.h"
#import "IOSMain.h"
#import <AVFoundation/AVFoundation.h>

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.window.rootViewController = [[GameViewController alloc] init];
    [self.window makeKeyAndVisible];

    [UIApplication sharedApplication].idleTimerDisabled = YES;

    // Ambient audio session (allows background/game sound properly)
    @try {
        AVAudioSession *audioSession = [AVAudioSession sharedInstance];
        [audioSession setCategory:AVAudioSessionCategoryAmbient error:nil];
        [audioSession setActive:YES error:nil];
    } @catch (NSException *e) {
        NSLog(@"AudioSession setup exception: %@", e);
    }

    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    IOS_SaveConfig();
    IOS_PauseAudio();
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    IOS_ResumeAudio();
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    IOS_SaveConfig();
}

- (void)applicationWillTerminate:(UIApplication *)application {
    IOS_SaveConfig();
}

@end
