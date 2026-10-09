#import "IOSPackUpdater.h"
#import <zlib.h>
extern "C" {
#include "../source/lzo/lzo1x.h"
}

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
    [req setValue:@"Aspar2iOS" forHTTPHeaderField:@"User-Agent"];
    
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);
    __block BOOL success = NO;
    __block int httpCode = 0;
    
    NSURLSessionDownloadTask *task = [[NSURLSession sharedSession] downloadTaskWithRequest:req completionHandler:^(NSURL *location, NSURLResponse *response, NSError *error) {
        NSHTTPURLResponse *httpResp = (NSHTTPURLResponse *)response;
        if (httpResp) httpCode = (int)httpResp.statusCode;
        
        if (!error && location && httpCode == 200) {
            NSFileManager *fm = [NSFileManager defaultManager];
            if ([fm fileExistsAtPath:destPath]) [fm removeItemAtPath:destPath error:nil];
            NSError *mvErr = nil;
            if ([fm moveItemAtURL:location toURL:[NSURL fileURLWithPath:destPath] error:&mvErr]) {
                success = YES;
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

        // Check if local pack/aspar2 directory already exists
        NSString *aspar2PackDir = [[docsPath stringByAppendingPathComponent:@"aspar2"] stringByAppendingPathComponent:@"pack"];
        BOOL hasLocalDocsPack = [fm fileExistsAtPath:[self->_packDir stringByAppendingPathComponent:@"root.index"]] &&
                                [fm fileExistsAtPath:[self->_packDir stringByAppendingPathComponent:@"root.data"]];
        BOOL hasLocalAspar2Pack = [fm fileExistsAtPath:[aspar2PackDir stringByAppendingPathComponent:@"root.index"]] &&
                                  [fm fileExistsAtPath:[aspar2PackDir stringByAppendingPathComponent:@"root.data"]];
        
        if (hasLocalDocsPack || hasLocalAspar2Pack) {
            reportStatus(@"Yerel paketler bulundu! Oyuna giriliyor...");
            dispatch_async(dispatch_get_main_queue(), ^{
                if (completionBlock) completionBlock(YES, nil);
            });
            return;
        }

        reportStatus(@"Sunucuya bağlanılıyor...");
        
        // 1. Fetch mobile_crclist
        NSString *crcUrlStr = [NSString stringWithFormat:@"%@%@?t=%ld", kUpdateBaseURL, kCrcListName, (long)[[NSDate date] timeIntervalSince1970]];
        NSString *tempCrcFile = [NSTemporaryDirectory() stringByAppendingPathComponent:@"mobile_crclist.tmp"];
        int httpCode = 0;
        
        if (![self downloadUrl:crcUrlStr toFile:tempCrcFile statusCode:&httpCode]) {
            if (hasLocalDocsPack || hasLocalAspar2Pack) {
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
            
            NSString *lzPartPath = [self->_packDir stringByAppendingPathComponent:[entry.name stringByAppendingString:@".lz.part"]];
            NSString *rawPartPath = [self->_packDir stringByAppendingPathComponent:[entry.name stringByAppendingString:@".part"]];
            NSString *finalPath = [self->_packDir stringByAppendingPathComponent:entry.name];
            
            BOOL isLz = YES;
            NSString *lzUrlStr = [NSString stringWithFormat:@"%@%@%@.lz", kUpdateBaseURL, kPackDirName, entry.name];
            int dlCode = 0;
            
            if (![self downloadUrl:lzUrlStr toFile:lzPartPath statusCode:&dlCode]) {
                isLz = NO;
                NSString *rawUrlStr = [NSString stringWithFormat:@"%@%@%@", kUpdateBaseURL, kPackDirName, entry.name];
                if (![self downloadUrl:rawUrlStr toFile:rawPartPath statusCode:&dlCode]) {
                    dispatch_async(dispatch_get_main_queue(), ^{
                        if (completionBlock) completionBlock(NO, [NSString stringWithFormat:@"İndirme başarısız: %@", entry.name]);
                    });
                    return;
                }
            }
            
            if (isLz) {
                reportStatus([NSString stringWithFormat:@"Açılıyor: %@ (%d/%d)", entry.name, i + 1, (int)todoList.count]);
                NSString *decErr = nil;
                if (![self decompressLzFile:lzPartPath toFile:rawPartPath error:&decErr]) {
                    [fm removeItemAtPath:lzPartPath error:nil];
                    [fm removeItemAtPath:rawPartPath error:nil];
                    dispatch_async(dispatch_get_main_queue(), ^{
                        if (completionBlock) completionBlock(NO, decErr ?: [NSString stringWithFormat:@"LZ açma hatası: %@", entry.name]);
                    });
                    return;
                }
                [fm removeItemAtPath:lzPartPath error:nil];
            }
            
            // Verify size & CRC32
            NSDictionary *rawAttrs = [fm attributesOfItemAtPath:rawPartPath error:nil];
            if ([rawAttrs fileSize] != entry.size) {
                [fm removeItemAtPath:rawPartPath error:nil];
                dispatch_async(dispatch_get_main_queue(), ^{
                    if (completionBlock) completionBlock(NO, [NSString stringWithFormat:@"Boyut uyuşmuyor: %@", entry.name]);
                });
                return;
            }
            
            reportStatus([NSString stringWithFormat:@"Doğrulanıyor: %@ (%d/%d)", entry.name, i + 1, (int)todoList.count]);
            uint32_t fileCrc = [self calculateCrc32ForFile:rawPartPath];
            if (fileCrc != entry.crc) {
                [fm removeItemAtPath:rawPartPath error:nil];
                dispatch_async(dispatch_get_main_queue(), ^{
                    if (completionBlock) completionBlock(NO, [NSString stringWithFormat:@"CRC32 uyuşmuyor: %@", entry.name]);
                });
                return;
            }
            
            if ([fm fileExistsAtPath:finalPath]) [fm removeItemAtPath:finalPath error:nil];
            [fm moveItemAtPath:rawPartPath toPath:finalPath error:nil];
            
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
