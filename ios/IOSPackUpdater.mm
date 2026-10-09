#import "IOSPackUpdater.h"
#import <zlib.h>
#include "../lzo/lzo1x.h"

static NSString * const kUpdateBaseURL = @"https://metin2plus.com/pe3qgb78x/patcher_01/0.0.0.1/";
static NSString * const kCrcListName   = @"mobile_crclist";
static NSString * const kPackDirName   = @"mobile_pack/";

struct PackCrcEntry {
    uint32_t crc;
    uint64_t size;
    NSString *name;
};

@interface IOSPackUpdater () {
    BOOL _isCancelled;
    NSString *_packDir;
}
@end

@implementation IOSPackUpdater

+ (instancetype)sharedInstance {
    static IOSPackUpdater *s_instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        s_instance = [[IOSPackUpdater alloc] init];
    });
    return s_instance;
}

- (void)cancelUpdate {
    _isCancelled = YES;
}

- (uint32_t)calculateCrc32ForFile:(NSString *)filePath {
    NSInputStream *stream = [NSInputStream inputStreamWithFileAtPath:filePath];
    if (!stream) return 0;
    [stream open];
    
    uLong crc = crc32(0L, Z_NULL, 0);
    uint8_t buffer[65536];
    
    while ([stream hasBytesAvailable]) {
        NSInteger readBytes = [stream read:buffer maxLength:sizeof(buffer)];
        if (readBytes <= 0) break;
        crc = crc32(crc, buffer, (uInt)readBytes);
    }
    [stream close];
    return (uint32_t)crc;
}

- (NSData *)fetchUrlData:(NSURLRequest *)request response:(NSURLResponse **)outResponse error:(NSError **)outError {
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);
    __block NSData *resultData = nil;
    __block NSURLResponse *resultResp = nil;
    __block NSError *resultErr = nil;
    
    NSURLSessionDataTask *task = [[NSURLSession sharedSession] dataTaskWithRequest:request completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
        resultData = data;
        resultResp = response;
        resultErr = error;
        dispatch_semaphore_signal(sema);
    }];
    [task resume];
    dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    
    if (outResponse) *outResponse = resultResp;
    if (outError) *outError = resultErr;
    return resultData;
}

- (void)startUpdateWithDocsPath:(NSString *)docsPath
                        onStatus:(IOSPackUpdaterStatusBlock)statusBlock
                      onProgress:(IOSPackUpdaterProgressBlock)progressBlock
                    onCompletion:(IOSPackUpdaterCompletionBlock)completionBlock {
    _isCancelled = NO;
    _packDir = [docsPath stringByAppendingPathComponent:@"pack"];
    
    NSFileManager *fm = [NSFileManager defaultManager];
    if (![fm fileExistsAtPath:_packDir]) {
        [fm createDirectoryAtPath:_packDir withIntermediateDirectories:YES attributes:nil error:nil];
    }
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        auto reportStatus = ^(NSString *msg) {
            dispatch_async(dispatch_get_main_queue(), ^{
                if (statusBlock) statusBlock(msg);
            });
        };
        
        auto reportProgress = ^(NSString *fileName, int index, int count, int percent, NSString *speed) {
            dispatch_async(dispatch_get_main_queue(), ^{
                if (progressBlock) progressBlock(fileName, index, count, percent, speed);
            });
        };

        reportStatus(@"Sunucuya bağlanılıyor...");
        
        // 1. Fetch mobile_crclist
        NSString *crcUrlStr = [NSString stringWithFormat:@"%@%@?t=%ld", kUpdateBaseURL, kCrcListName, (long)[[NSDate date] timeIntervalSince1970]];
        NSURL *crcUrl = [NSURL URLWithString:crcUrlStr];
        
        NSMutableURLRequest *req = [NSMutableURLRequest requestWithURL:crcUrl cachePolicy:NSURLRequestReloadIgnoringLocalCacheData timeoutInterval:15.0];
        [req setValue:@"Aspar2iOS" forHTTPHeaderField:@"User-Agent"];
        
        NSError *error = nil;
        NSURLResponse *response = nil;
        NSData *crcData = [self fetchUrlData:req response:&response error:&error];
        
        if (error || !crcData || crcData.length == 0) {
            BOOL hasLocal = [fm fileExistsAtPath:[self->_packDir stringByAppendingPathComponent:@"root.index"]] &&
                            [fm fileExistsAtPath:[self->_packDir stringByAppendingPathComponent:@"root.data"]];
            if (hasLocal) {
                reportStatus(@"Sunucuya bağlanılamadı, yerel paketlerle başlatılıyor...");
                dispatch_async(dispatch_get_main_queue(), ^{
                    if (completionBlock) completionBlock(YES, nil);
                });
            } else {
                dispatch_async(dispatch_get_main_queue(), ^{
                    if (completionBlock) completionBlock(NO, @"Sunucuya bağlanılamadı. İnternet bağlantınızı kontrol edin.");
                });
            }
            return;
        }
        
        NSString *crcContent = [[NSString alloc] initWithData:crcData encoding:NSUTF8StringEncoding];
        NSArray *lines = [crcContent componentsSeparatedByCharactersInSet:[NSCharacterSet newlineCharacterSet]];
        
        NSMutableArray<NSValue *> *entries = [NSMutableArray array];
        
        for (NSString *rawLine in lines) {
            NSString *line = [rawLine stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
            if (line.length == 0 || [line hasPrefix:@"#"] || [line hasPrefix:@";"]) continue;
            
            NSArray *tokens = [line componentsSeparatedByCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
            NSMutableArray *filteredTokens = [NSMutableArray array];
            for (NSString *t in tokens) {
                if (t.length > 0) [filteredTokens addObject:t];
            }
            if (filteredTokens.count < 3) continue;
            
            PackCrcEntry entry;
            NSScanner *scanner = [NSScanner scannerWithString:filteredTokens[0]];
            unsigned int hexCrc = 0;
            [scanner scanHexInt:&hexCrc];
            entry.crc = (uint32_t)hexCrc;
            entry.size = [filteredTokens[1] longLongValue];
            
            int fromIdx = (filteredTokens.count >= 5) ? 4 : 2;
            NSMutableString *nm = [NSMutableString string];
            for (int i = fromIdx; i < (int)filteredTokens.count; i++) {
                if (nm.length > 0) [nm appendString:@" "];
                [nm appendString:filteredTokens[i]];
            }
            NSString *name = [nm stringByReplacingOccurrencesOfString:@"\\" withString:@"/"];
            NSInteger lastSlash = [name rangeOfString:@"/" options:NSBackwardsSearch].location;
            if (lastSlash != NSNotFound) {
                name = [name substringFromIndex:lastSlash + 1];
            }
            if (name.length == 0 || [name containsString:@".."]) continue;
            entry.name = name;
            
            NSValue *val = [NSValue value:&entry withObjCType:@encode(PackCrcEntry)];
            [entries addObject:val];
        }
        
        // 2. Identify missing or outdated files
        NSMutableArray<NSValue *> *todoList = [NSMutableArray array];
        uint64_t totalBytesToDownload = 0;
        
        for (NSValue *val in entries) {
            if (self->_isCancelled) return;
            PackCrcEntry entry;
            [val getValue:&entry];
            
            NSString *localPath = [self->_packDir stringByAppendingPathComponent:entry.name];
            BOOL needDownload = YES;
            
            if ([fm fileExistsAtPath:localPath]) {
                NSDictionary *attrs = [fm attributesOfItemAtPath:localPath error:nil];
                if ([attrs fileSize] == entry.size) {
                    uint32_t localCrc = [self calculateCrc32ForFile:localPath];
                    if (localCrc == entry.crc) {
                        needDownload = NO;
                    }
                }
            }
            
            if (needDownload) {
                [todoList addObject:val];
                totalBytesToDownload += entry.size;
            }
        }
        
        if (todoList.count == 0) {
            reportStatus(@"Tüm paketler güncel!");
            dispatch_async(dispatch_get_main_queue(), ^{
                if (completionBlock) completionBlock(YES, nil);
            });
            return;
        }
        
        // 3. Download files in todoList
        uint64_t downloadedBytesAll = 0;
        NSDate *startTime = [NSDate date];
        
        for (int i = 0; i < (int)todoList.count; i++) {
            if (self->_isCancelled) return;
            
            PackCrcEntry entry;
            [todoList[i] getValue:&entry];
            
            reportStatus([NSString stringWithFormat:@"İndiriliyor: %@ (%d/%d)", entry.name, i + 1, (int)todoList.count]);
            
            // Try .lz first
            NSString *fileUrlStr = [NSString stringWithFormat:@"%@%@%@.lz", kUpdateBaseURL, kPackDirName, entry.name];
            NSURL *fileUrl = [NSURL URLWithString:fileUrlStr];
            
            NSMutableURLRequest *fileReq = [NSMutableURLRequest requestWithURL:fileUrl cachePolicy:NSURLRequestReloadIgnoringLocalCacheData timeoutInterval:30.0];
            [fileReq setValue:@"Aspar2iOS" forHTTPHeaderField:@"User-Agent"];
            
            NSURLResponse *httpResp = nil;
            NSError *dlErr = nil;
            NSData *downloadedData = [self fetchUrlData:fileReq response:&httpResp error:&dlErr];
            NSHTTPURLResponse *httpUrlResp = (NSHTTPURLResponse *)httpResp;
            
            BOOL isLz = YES;
            if ((httpUrlResp && httpUrlResp.statusCode == 404) || !downloadedData || downloadedData.length == 0) {
                isLz = NO;
                NSString *rawUrlStr = [NSString stringWithFormat:@"%@%@%@", kUpdateBaseURL, kPackDirName, entry.name];
                fileReq = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:rawUrlStr] cachePolicy:NSURLRequestReloadIgnoringLocalCacheData timeoutInterval:30.0];
                downloadedData = [self fetchUrlData:fileReq response:&httpResp error:&dlErr];
                httpUrlResp = (NSHTTPURLResponse *)httpResp;
            }
            
            if (!downloadedData || downloadedData.length == 0 || (httpUrlResp && httpUrlResp.statusCode != 200)) {
                dispatch_async(dispatch_get_main_queue(), ^{
                    if (completionBlock) completionBlock(NO, [NSString stringWithFormat:@"İndirme başarısız: %@", entry.name]);
                });
                return;
            }
            
            NSData *finalData = nil;
            if (isLz) {
                if (downloadedData.length < 4) {
                    dispatch_async(dispatch_get_main_queue(), ^{
                        if (completionBlock) completionBlock(NO, [NSString stringWithFormat:@"Bozuk LZ paket: %@", entry.name]);
                    });
                    return;
                }
                
                const uint8_t *bytes = (const uint8_t *)downloadedData.bytes;
                uint32_t uncompSize = (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) | ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
                
                NSMutableData *decompressed = [NSMutableData dataWithLength:uncompSize];
                lzo_uint outLen = uncompSize;
                
                int lzoResult = lzo1x_decompress_safe(bytes + 4, (lzo_uint)(downloadedData.length - 4), (lzo_bytep)decompressed.mutableBytes, &outLen, NULL);
                if (lzoResult != 0 || outLen != uncompSize) {
                    dispatch_async(dispatch_get_main_queue(), ^{
                        if (completionBlock) completionBlock(NO, [NSString stringWithFormat:@"LZ açma hatası (%d): %@", lzoResult, entry.name]);
                    });
                    return;
                }
                finalData = decompressed;
            } else {
                finalData = downloadedData;
            }
            
            if (finalData.length != entry.size) {
                dispatch_async(dispatch_get_main_queue(), ^{
                    if (completionBlock) completionBlock(NO, [NSString stringWithFormat:@"Boyut uyuşmuyor: %@", entry.name]);
                });
                return;
            }
            
            uLong fileCrc = crc32(0L, (const Bytef *)finalData.bytes, (uInt)finalData.length);
            if ((uint32_t)fileCrc != entry.crc) {
                dispatch_async(dispatch_get_main_queue(), ^{
                    if (completionBlock) completionBlock(NO, [NSString stringWithFormat:@"CRC32 uyuşmuyor: %@", entry.name]);
                });
                return;
            }
            
            NSString *targetPath = [self->_packDir stringByAppendingPathComponent:entry.name];
            [finalData writeToFile:targetPath atomically:YES];
            
            downloadedBytesAll += entry.size;
            NSTimeInterval elapsed = [[NSDate date] timeIntervalSinceDate:startTime];
            double speedMB = (elapsed > 0) ? ((double)downloadedBytesAll / (1024.0 * 1024.0)) / elapsed : 0.0;
            NSString *speedStr = [NSString stringWithFormat:@"%.1f MB/s", speedMB];
            
            int pct = (int)((double)downloadedBytesAll / (double)totalBytesToDownload * 100.0);
            if (pct > 100) pct = 100;
            
            reportProgress(entry.name, i + 1, (int)todoList.count, pct, speedStr);
        }
        
        reportStatus(@"Tüm güncellemeler yüklendi! Oyuna giriliyor...");
        dispatch_async(dispatch_get_main_queue(), ^{
            if (completionBlock) completionBlock(YES, nil);
        });
    });
}

@end
