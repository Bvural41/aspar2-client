#import "AppDelegate.h"
#import "GameViewController.h"
#import "IOSMain.h"
#import <AVFoundation/AVFoundation.h>

#import <signal.h>

static void UncaughtExceptionHandler(NSException *exception) {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *docsPath = paths.firstObject;
    if (docsPath) {
        NSString *logPath = [docsPath stringByAppendingPathComponent:@"syserr.txt"];
        NSString *errorLog = [NSString stringWithFormat:@"\n=== CRASH EXCEPTION ===\nName: %@\nReason: %@\nStack:\n%@\n",
                              exception.name, exception.reason, [exception callStackSymbols]];
        FILE *f = fopen([logPath UTF8String], "a");
        if (f) {
            fputs([errorLog UTF8String], f);
            fclose(f);
        }
    }
}

static void SignalHandler(int sig) {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *docsPath = paths.firstObject;
    if (docsPath) {
        NSString *logPath = [docsPath stringByAppendingPathComponent:@"syserr.txt"];
        NSString *errorLog = [NSString stringWithFormat:@"\n=== CRASH SIGNAL %d ===\n", sig];
        FILE *f = fopen([logPath UTF8String], "a");
        if (f) {
            fputs([errorLog UTF8String], f);
            fclose(f);
        }
    }
}

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    NSSetUncaughtExceptionHandler(&UncaughtExceptionHandler);
    signal(SIGABRT, SignalHandler);
    signal(SIGILL, SignalHandler);
    signal(SIGSEGV, SignalHandler);
    signal(SIGFPE, SignalHandler);
    signal(SIGBUS, SignalHandler);
    signal(SIGPIPE, SignalHandler);

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
