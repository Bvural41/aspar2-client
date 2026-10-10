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

static void SignalHandler(int sig, siginfo_t *info, void *ucontext) {
    signal(sig, SIG_DFL);
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *docsPath = paths.firstObject;
    if (docsPath) {
        NSString *logPath = [docsPath stringByAppendingPathComponent:@"syserr.txt"];
        NSArray *stackSymbols = [NSThread callStackSymbols];
        NSString *stackStr = [stackSymbols componentsJoinedByString:@"\n"];
        void *faultAddr = info ? info->si_addr : NULL;
        int code = info ? info->si_code : 0;
        NSString *errorLog = [NSString stringWithFormat:@"\n=== CRASH SIGNAL %d (code=%d, addr=%p) ===\nStack Trace:\n%@\n",
                              sig, code, faultAddr, stackStr];
        FILE *f = fopen([logPath UTF8String], "a");
        if (f) {
            fputs([errorLog UTF8String], f);
            fflush(f);
            fclose(f);
        }
    }
}

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    NSSetUncaughtExceptionHandler(&UncaughtExceptionHandler);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = SignalHandler;
    sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    signal(SIGPIPE, SIG_IGN);

    self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.window.rootViewController = [[GameViewController alloc] init];
    [self.window makeKeyAndVisible];

    [UIApplication sharedApplication].idleTimerDisabled = YES;

    // Ambient audio session (allows background/game sound properly)
    @try {
        AVAudioSession *audioSession = [AVAudioSession sharedInstance];
        [audioSession setCategory:AVAudioSessionCategoryAmbient withOptions:AVAudioSessionCategoryOptionMixWithOthers error:nil];
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
