#import "IOSPackUpdater.h"
#import <zlib.h>
extern "C" {
#include "lzo1x.h"
}

static NSString * const kUpdateBaseURL = @"https://metin2plus.com/pe3qgb78x/patcher_01/0.0.0.1/";
static NSString * const kCrcListName   = @"mobile_crclist";
static NSString * const kPackDirName   = @"mobile_pack/";

@interface PackCrcItem : NSObject
@property (nonatomic, assign) uint32_t crc;
@property (nonatomic, assign) uint64_t size;
@property (nonatomic, copy) NSString *name;
@end

@implementation PackCrcItem
@end

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

- (instancetype)init {
    self = [super init];
    if (self) {
        lzo_init();
    }
    return self;
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

- (BOOL)downloadUrl:(NSString *)urlStr toFile:(NSString *)destPath statusCode:(int *)outStatus {
    NSURL *url = [NSURL URLWithString:urlStr];
    if (!url) return NO;
    
    NSMutableURLRequest *req = [NSMutableURLRequest requestWithURL:url cachePolicy:NSURLRequestReloadIgnoringLocalCacheData timeoutInterval:30.0];
    [req setValue:@"Mozilla/5.0 (iPhone; CPU iPhone OS 17_0 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Mobile/15E148" forHTTPHeaderField:@"User-Agent"];
    
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);
    __block BOOL success = NO;
    __block int httpCode = 0;
    
    NSURLSessionDownloadTask *task = [[NSURLSession sharedSession] downloadTaskWithRequest:req completionHandler:^(NSURL *location, NSURLResponse *response, NSError *error) {
        NSHTTPURLResponse *httpResp = (NSHTTPURLResponse *)response;
        if (httpResp) httpCode = (int)httpResp.statusCode;
        
        if (!error && location && httpCode == 200) {
            NSFileManager *fm = [NSFileManager defaultManager];
            NSString *parentDir = [destPath stringByDeletingLastPathComponent];
            if (![fm fileExistsAtPath:parentDir]) {
                [fm createDirectoryAtPath:parentDir withIntermediateDirectories:YES attributes:nil error:nil];
            }
            if ([fm fileExistsAtPath:destPath]) [fm removeItemAtPath:destPath error:nil];
            
            NSError *cpErr = nil;
            if ([fm copyItemAtURL:location toURL:[NSURL fileURLWithPath:destPath] error:&cpErr]) {
                success = YES;
            } else {
                NSData *data = [NSData dataWithContentsOfURL:location];
                if (data && data.length > 0) {
                    success = [data writeToFile:destPath atomically:YES];
                }
            }
        }
        dispatch_semaphore_signal(sema);
    }];
    [task resume];
    dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    
    if (outStatus) *outStatus = httpCode;
    return success;
}

- (BOOL)decompressLzFile:(NSString *)lzPath toFile:(NSString *)dstPath error:(NSString **)outErr {
    FILE *inFp = fopen([lzPath UTF8String], "rb");
    if (!inFp) {
        if (outErr) *outErr = @"LZ dosyası açılamadı";
        return NO;
    }
    
    fseek(inFp, 0, SEEK_END);
    long compSize = ftell(inFp);
    fseek(inFp, 0, SEEK_SET);
    
    if (compSize < 4) {
        fclose(inFp);
        if (outErr) *outErr = @"LZ dosyası çok küçük";
        return NO;
    }
    
    uint32_t uncompSize = 0;
    if (fread(&uncompSize, 1, 4, inFp) != 4) {
        fclose(inFp);
        if (outErr) *outErr = @"LZ başlığı okunamadı";
        return NO;
    }
    
    long rawCompSize = compSize - 4;
    uint8_t *compBuf = (uint8_t *)malloc(rawCompSize);
    if (!compBuf) {
        fclose(inFp);
        if (outErr) *outErr = @"Bellek ayırma hatası (compressed)";
        return NO;
    }
    
    if (fread(compBuf, 1, rawCompSize, inFp) != rawCompSize) {
        free(compBuf);
        fclose(inFp);
        if (outErr) *outErr = @"LZ verisi okunamadı";
        return NO;
    }
    fclose(inFp);
    
    uint8_t *uncompBuf = (uint8_t *)malloc(uncompSize);
    if (!uncompBuf) {
        free(compBuf);
        if (outErr) *outErr = @"Bellek ayırma hatası (uncompressed)";
        return NO;
    }
    
    lzo_uint outLen = uncompSize;
    int r = lzo1x_decompress_safe(compBuf, (lzo_uint)rawCompSize, uncompBuf, &outLen, NULL);
    free(compBuf);
    
    if (r != LZO_E_OK || outLen != uncompSize) {
        free(uncompBuf);
        if (outErr) *outErr = [NSString stringWithFormat:@"LZ decompress hatası (%d)", r];
        return NO;
    }
    
    FILE *outFp = fopen([dstPath UTF8String], "wb");
    if (!outFp) {
        free(uncompBuf);
        if (outErr) *outErr = @"Hedef dosya oluşturulamadı";
        return NO;
    }
    
    fwrite(uncompBuf, 1, uncompSize, outFp);
    fclose(outFp);
    free(uncompBuf);
    
    return YES;
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

        // Check if local pack directory already exists (.index and .data)
        NSString *aspar2PackDir = [[docsPath stringByAppendingPathComponent:@"aspar2"] stringByAppendingPathComponent:@"pack"];
        
        BOOL hasLocalDocsPack = [fm fileExistsAtPath:[self->_packDir stringByAppendingPathComponent:@"root.index"]] &&
                                [fm fileExistsAtPath:[self->_packDir stringByAppendingPathComponent:@"root.data"]];
        BOOL hasLocalAspar2Pack = [fm fileExistsAtPath:[aspar2PackDir stringByAppendingPathComponent:@"root.index"]] &&
                                  [fm fileExistsAtPath:[aspar2PackDir stringByAppendingPathComponent:@"root.data"]];

        NSArray *packFiles = [fm contentsOfDirectoryAtPath:self->_packDir error:nil];
        BOOL hasGenericPacks = NO;
        for (NSString *fName in packFiles) {
            if ([fName hasSuffix:@".index"] || [fName hasSuffix:@".data"]) {
                hasGenericPacks = YES;
                break;
            }
        }
        
        reportStatus(@"Sunucuya bağlanılıyor...");
        
        // 1. Fetch mobile_crclist
        NSString *crcUrlStr = [NSString stringWithFormat:@"%@%@?t=%ld", kUpdateBaseURL, kCrcListName, (long)[[NSDate date] timeIntervalSince1970]];
        NSString *tempCrcFile = [NSTemporaryDirectory() stringByAppendingPathComponent:@"mobile_crclist.tmp"];
        int httpCode = 0;
        
        if (![self downloadUrl:crcUrlStr toFile:tempCrcFile statusCode:&httpCode]) {
            if (hasLocalDocsPack || hasLocalAspar2Pack || hasGenericPacks) {
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
        
        NSString *crcContent = [NSString stringWithContentsOfFile:tempCrcFile encoding:NSUTF8StringEncoding error:nil];
        [fm removeItemAtPath:tempCrcFile error:nil];
        
        if (!crcContent || crcContent.length == 0) {
            dispatch_async(dispatch_get_main_queue(), ^{
                if (completionBlock) completionBlock(NO, @"Güncelleme listesi okunamadı.");
            });
            return;
        }
        
        NSArray *lines = [crcContent componentsSeparatedByCharactersInSet:[NSCharacterSet newlineCharacterSet]];
        NSMutableArray<PackCrcItem *> *entries = [NSMutableArray array];
        
        for (NSString *rawLine in lines) {
            NSString *line = [rawLine stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
            if (line.length == 0 || [line hasPrefix:@"#"] || [line hasPrefix:@";"]) continue;
            
            NSArray *tokens = [line componentsSeparatedByCharactersInSet:[NSCharacterSet whitespaceCharacterSet]];
            NSMutableArray *filteredTokens = [NSMutableArray array];
            for (NSString *t in tokens) {
                if (t.length > 0) [filteredTokens addObject:t];
            }
            if (filteredTokens.count < 3) continue;
            
            PackCrcItem *item = [[PackCrcItem alloc] init];
            NSScanner *scanner = [NSScanner scannerWithString:filteredTokens[0]];
            unsigned int hexCrc = 0;
            [scanner scanHexInt:&hexCrc];
            item.crc = (uint32_t)hexCrc;
            item.size = [filteredTokens[1] longLongValue];
            
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
            item.name = name;
            
            [entries addObject:item];
        }
        
        // 2. Identify missing or outdated files
        NSMutableArray<PackCrcItem *> *todoList = [NSMutableArray array];
        uint64_t totalBytesToDownload = 0;
        
        for (PackCrcItem *item in entries) {
            if (self->_isCancelled) return;
            
            NSString *localPath = [self->_packDir stringByAppendingPathComponent:item.name];
            BOOL needDownload = YES;
            
            if ([fm fileExistsAtPath:localPath]) {
                NSDictionary *attrs = [fm attributesOfItemAtPath:localPath error:nil];
                if ([attrs fileSize] == item.size) {
                    uint32_t localCrc = [self calculateCrc32ForFile:localPath];
                    if (localCrc == item.crc) {
                        needDownload = NO;
                    }
                }
            }
            
            if (needDownload) {
                [todoList addObject:item];
                totalBytesToDownload += item.size;
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
            
            PackCrcItem *item = todoList[i];
            
            reportStatus([NSString stringWithFormat:@"İndiriliyor: %@ (%d/%d)", item.name, i + 1, (int)todoList.count]);
            
            NSString *lzPartPath = [self->_packDir stringByAppendingPathComponent:[item.name stringByAppendingString:@".lz.part"]];
            NSString *rawPartPath = [self->_packDir stringByAppendingPathComponent:[item.name stringByAppendingString:@".part"]];
            NSString *finalPath = [self->_packDir stringByAppendingPathComponent:item.name];
            
            BOOL isLz = YES;
            NSString *lzUrlStr = [NSString stringWithFormat:@"%@%@%@.lz", kUpdateBaseURL, kPackDirName, item.name];
            int dlCode = 0;
            
            if (![self downloadUrl:lzUrlStr toFile:lzPartPath statusCode:&dlCode]) {
                isLz = NO;
                NSString *rawUrlStr = [NSString stringWithFormat:@"%@%@%@", kUpdateBaseURL, kPackDirName, item.name];
                if (![self downloadUrl:rawUrlStr toFile:rawPartPath statusCode:&dlCode]) {
                    dispatch_async(dispatch_get_main_queue(), ^{
                        if (completionBlock) completionBlock(NO, [NSString stringWithFormat:@"İndirme başarısız: %@", item.name]);
                    });
                    return;
                }
            }
            
            if (isLz) {
                reportStatus([NSString stringWithFormat:@"Açılıyor: %@ (%d/%d)", item.name, i + 1, (int)todoList.count]);
                NSString *decErr = nil;
                if (![self decompressLzFile:lzPartPath toFile:rawPartPath error:&decErr]) {
                    [fm removeItemAtPath:lzPartPath error:nil];
                    [fm removeItemAtPath:rawPartPath error:nil];
                    dispatch_async(dispatch_get_main_queue(), ^{
                        if (completionBlock) completionBlock(NO, decErr ?: [NSString stringWithFormat:@"LZ açma hatası: %@", item.name]);
                    });
                    return;
                }
                [fm removeItemAtPath:lzPartPath error:nil];
            }
            
            // Verify size & CRC32
            NSDictionary *rawAttrs = [fm attributesOfItemAtPath:rawPartPath error:nil];
            if ([rawAttrs fileSize] != item.size) {
                [fm removeItemAtPath:rawPartPath error:nil];
                dispatch_async(dispatch_get_main_queue(), ^{
                    if (completionBlock) completionBlock(NO, [NSString stringWithFormat:@"Boyut uyuşmuyor: %@", item.name]);
                });
                return;
            }
            
            reportStatus([NSString stringWithFormat:@"Doğrulanıyor: %@ (%d/%d)", item.name, i + 1, (int)todoList.count]);
            uint32_t fileCrc = [self calculateCrc32ForFile:rawPartPath];
            if (fileCrc != item.crc) {
                [fm removeItemAtPath:rawPartPath error:nil];
                dispatch_async(dispatch_get_main_queue(), ^{
                    if (completionBlock) completionBlock(NO, [NSString stringWithFormat:@"CRC32 uyuşmuyor: %@", item.name]);
                });
                return;
            }
            
            if ([fm fileExistsAtPath:finalPath]) [fm removeItemAtPath:finalPath error:nil];
            [fm moveItemAtPath:rawPartPath toPath:finalPath error:nil];
            
            downloadedBytesAll += item.size;
            NSTimeInterval elapsed = [[NSDate date] timeIntervalSinceDate:startTime];
            double speedMB = (elapsed > 0) ? ((double)downloadedBytesAll / (1024.0 * 1024.0)) / elapsed : 0.0;
            NSString *speedStr = [NSString stringWithFormat:@"%.1f MB/s", speedMB];
            
            int pct = (int)((double)downloadedBytesAll / (double)totalBytesToDownload * 100.0);
            if (pct > 100) pct = 100;
            
            reportProgress(item.name, i + 1, (int)todoList.count, pct, speedStr);
        }
        
        reportStatus(@"Tüm güncellemeler yüklendi! Oyuna giriliyor...");
        dispatch_async(dispatch_get_main_queue(), ^{
            if (completionBlock) completionBlock(YES, nil);
        });
    });
}

@end
