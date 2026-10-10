#import <Foundation/Foundation.h>

typedef void (^IOSPackUpdaterStatusBlock)(NSString *statusText);
typedef void (^IOSPackUpdaterProgressBlock)(NSString *fileName, int fileIndex, int fileCount,
                                           uint64_t allDone, uint64_t allTotal,
                                           uint64_t netSpeedBps, int64_t etaSec);
typedef void (^IOSPackUpdaterCompletionBlock)(BOOL success, NSString *errorMsg);

@interface IOSPackUpdater : NSObject

+ (instancetype)sharedInstance;
+ (NSString *)formatBytes:(uint64_t)bytes;

- (void)startUpdateWithDocsPath:(NSString *)docsPath
                        onStatus:(IOSPackUpdaterStatusBlock)statusBlock
                      onProgress:(IOSPackUpdaterProgressBlock)progressBlock
                    onCompletion:(IOSPackUpdaterCompletionBlock)completionBlock;

- (void)cancelUpdate;

@end
