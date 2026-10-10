#import <Foundation/Foundation.h>

typedef void (^IOSPackUpdaterStatusBlock)(NSString *statusText);
typedef void (^IOSPackUpdaterProgressBlock)(NSString *fileName, int fileIndex, int fileCount, int percent, NSString *speedText);
typedef void (^IOSPackUpdaterCompletionBlock)(BOOL success, NSString *errorMsg);

@interface IOSPackUpdater : NSObject

+ (instancetype)sharedInstance;

- (void)startUpdateWithDocsPath:(NSString *)docsPath
                        onStatus:(IOSPackUpdaterStatusBlock)statusBlock
                      onProgress:(IOSPackUpdaterProgressBlock)progressBlock
                    onCompletion:(IOSPackUpdaterCompletionBlock)completionBlock;

- (void)cancelUpdate;

@end
