#import "GameViewController.h"
#import "IOSMain.h"
#import "IOSPackUpdater.h"
#import <WebKit/WebKit.h>
#import <AVFoundation/AVFoundation.h>

extern GLuint g_nDefaultFramebuffer;

@interface EAGLView : UIView
@end

@implementation EAGLView
+ (Class)layerClass {
    return [CAEAGLLayer class];
}
@end

@interface GameViewController () <UITextFieldDelegate, WKNavigationDelegate> {
    EAGLContext *_context;
    CADisplayLink *_displayLink;
    GLuint _defaultFramebuffer;
    GLuint _colorRenderbuffer;
    GLuint _depthRenderbuffer;
    GLint _framebufferWidth;
    GLint _framebufferHeight;

    int _targetWidth;
    int _targetHeight;

    BOOL _initialized;

    // Otopatch Downloader UI
    UIView *_otopatchContainer;
    UIView *_otopatchPanel;
    UIProgressView *_otopatchProgress;
    UILabel *_otopatchStatusLabel;
    UILabel *_otopatchPercentLabel;
    UILabel *_otopatchDetailLabel;
    UIButton *_otopatchRetryBtn;
    UIButton *_otopatchContinueBtn;
    BOOL _otopatchDone;

    // Multi-touch tracking
    UITouch *_joystickTouch;
    UITouch *_actionTouch;
    UITouch *_secondaryTouch;

    float _actionStartX;
    float _actionStartY;
    float _actionLastX;
    float _actionLastY;
    BOOL _actionDidDrag;

    float _secondaryLastX;
    float _secondaryLastY;

    // Joystick zone
    float _joystickX;
    float _joystickY;
    float _joystickW;
    float _joystickH;

    // Keyboard support
    UITextField *_hiddenTextField;
    BOOL _ignoreTextChange;

    // Web popup (Nesne Market)
    UIView *_webContainerView;
    WKWebView *_webView;
    UIProgressView *_progressBar;
    BOOL _isWebShowing;
}
@end

static GameViewController *s_sharedInstance = nil;

@implementation GameViewController

+ (GameViewController *)sharedInstance {
    return s_sharedInstance;
}

- (void)loadView {
    EAGLView *glView = [[EAGLView alloc] initWithFrame:[UIScreen mainScreen].bounds];
    glView.multipleTouchEnabled = YES;
    self.view = glView;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    s_sharedInstance = self;

    _targetHeight = 600;
    _targetWidth = 1067;
    _joystickX = 0.0f;
    _joystickY = -1.0f;
    _joystickW = 194.0f;
    _joystickH = 194.0f;

    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    if (!_context) {
        NSLog(@"[Aspar2 iOS] Failed to create OpenGLES 3.0 context, falling back to 2.0");
        _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    }
    [EAGLContext setCurrentContext:_context];

    CGFloat screenScale = [UIScreen mainScreen].nativeScale;
    if (screenScale <= 0.0) screenScale = [UIScreen mainScreen].scale;
    if (screenScale <= 0.0) screenScale = 2.0;

    self.view.contentScaleFactor = screenScale;
    CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.view.layer;
    eaglLayer.contentsScale = screenScale;
    eaglLayer.opaque = YES;
    eaglLayer.drawableProperties = @{
        kEAGLDrawablePropertyRetainedBacking: @(NO),
        kEAGLDrawablePropertyColorFormat: kEAGLColorFormatRGBA8
    };

    [self setupBuffers];

    // Setup hidden text field for keyboard
    _hiddenTextField = [[UITextField alloc] initWithFrame:CGRectMake(0, 0, 10, 10)];
    _hiddenTextField.alpha = 0.001;
    _hiddenTextField.autocorrectionType = UITextAutocorrectionTypeNo;
    _hiddenTextField.autocapitalizationType = UITextAutocapitalizationTypeNone;
    _hiddenTextField.delegate = self;
    [self.view addSubview:_hiddenTextField];

    [_hiddenTextField addTarget:self action:@selector(textFieldDidChange:) forControlEvents:UIControlEventEditingChanged];

    // Calculate aspect ratio resolution
    CGSize screenSize = [UIScreen mainScreen].bounds.size;
    CGFloat maxDim = MAX(screenSize.width, screenSize.height);
    CGFloat minDim = MIN(screenSize.width, screenSize.height);
    if (minDim > 0) {
        float aspect = (float)maxDim / (float)minDim;
        _targetWidth = (int)roundf(_targetHeight * aspect);
        if (_targetWidth % 2 != 0) _targetWidth++;
    }

    // Setup Otopatch Downloader UI
    [self setupOtopatchUI];

    // Start display link
    _displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(renderFrame:)];
    if (@available(iOS 15.0, *)) {
        _displayLink.preferredFrameRateRange = CAFrameRateRangeMake(60, 60, 60);
    } else {
        _displayLink.preferredFramesPerSecond = 60;
    }
    [_displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
}

- (void)setupOtopatchUI {
    CGRect bounds = [UIScreen mainScreen].bounds;
    CGFloat width = bounds.size.width;
    CGFloat height = bounds.size.height;
    
    _otopatchContainer = [[UIView alloc] initWithFrame:bounds];
    _otopatchContainer.backgroundColor = [UIColor colorWithRed:0.04 green:0.04 blue:0.06 alpha:1.0];
    _otopatchContainer.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self.view addSubview:_otopatchContainer];
    
    // 1. Background Image (bg_loading.jpg)
    UIImage *bgImg = [UIImage imageNamed:@"bg_loading.jpg"];
    if (bgImg) {
        UIImageView *bgView = [[UIImageView alloc] initWithFrame:bounds];
        bgView.image = bgImg;
        bgView.contentMode = UIViewContentModeScaleAspectFill;
        bgView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        [_otopatchContainer addSubview:bgView];
    }
    
    // 2. Light Dimming Overlay (#33000000 -> 20% dark overlay)
    UIView *dimOverlay = [[UIView alloc] initWithFrame:bounds];
    dimOverlay.backgroundColor = [UIColor colorWithRed:0.0 green:0.0 blue:0.0 alpha:0.20];
    dimOverlay.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [_otopatchContainer addSubview:dimOverlay];
    
    // 3. Top-Left Logo (logo.png)
    UIImage *logoImg = [UIImage imageNamed:@"logo.png"];
    if (logoImg && logoImg.size.width > 0) {
        UIImageView *logoView = [[UIImageView alloc] initWithFrame:CGRectMake(24, 16, 180, 65)];
        logoView.image = logoImg;
        logoView.contentMode = UIViewContentModeScaleAspectFit;
        logoView.autoresizingMask = UIViewAutoresizingFlexibleRightMargin | UIViewAutoresizingFlexibleBottomMargin;
        [_otopatchContainer addSubview:logoView];
    } else {
        UILabel *titleLabel = [[UILabel alloc] initWithFrame:CGRectMake(24, 20, 200, 36)];
        titleLabel.text = @"ASPAR2";
        titleLabel.textColor = [UIColor colorWithRed:0.95 green:0.77 blue:0.40 alpha:1.0];
        titleLabel.font = [UIFont boldSystemFontOfSize:26.0];
        titleLabel.autoresizingMask = UIViewAutoresizingFlexibleRightMargin | UIViewAutoresizingFlexibleBottomMargin;
        [_otopatchContainer addSubview:titleLabel];
    }
    
    // 4. Top-Right Version Label ("v1.0.4 • iOS")
    UILabel *verLabel = [[UILabel alloc] initWithFrame:CGRectMake(width - 200 - 24, 20, 200, 24)];
    verLabel.text = @"v1.0.4 • iOS";
    verLabel.textColor = [UIColor colorWithRed:0.69 green:0.78 blue:0.63 alpha:0.7];
    verLabel.font = [UIFont systemFontOfSize:12.0 weight:UIFontWeightMedium];
    verLabel.textAlignment = NSTextAlignmentRight;
    verLabel.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleBottomMargin;
    [_otopatchContainer addSubview:verLabel];
    
    // 5. Bottom MMORPG Obsidian Panel with Gold Border
    CGFloat panelMarginH = 32.0f;
    CGFloat panelWidth = width - (panelMarginH * 2.0f);
    CGFloat panelHeight = 110.0f;
    CGFloat panelY = height - panelHeight - 18.0f;
    
    _otopatchPanel = [[UIView alloc] initWithFrame:CGRectMake(panelMarginH, panelY, panelWidth, panelHeight)];
    _otopatchPanel.backgroundColor = [UIColor colorWithRed:0.06 green:0.05 blue:0.04 alpha:0.88]; // #E00F0C09
    _otopatchPanel.layer.cornerRadius = 12.0f;
    _otopatchPanel.layer.borderWidth = 2.0f;
    _otopatchPanel.layer.borderColor = [UIColor colorWithRed:0.63 green:0.51 blue:0.31 alpha:1.0].CGColor; // #A0824E Gold
    _otopatchPanel.clipsToBounds = YES;
    _otopatchPanel.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleTopMargin;
    [_otopatchContainer addSubview:_otopatchPanel];
    
    CGFloat contentW = panelWidth - 40.0f;
    
    // Top Row: Status (Left) & Percent (Right)
    _otopatchStatusLabel = [[UILabel alloc] initWithFrame:CGRectMake(20, 12, contentW - 70.0f, 22)];
    _otopatchStatusLabel.text = @"Hazırlanıyor...";
    _otopatchStatusLabel.textColor = [UIColor colorWithRed:0.94 green:0.90 blue:0.85 alpha:1.0]; // #EFE6D8
    _otopatchStatusLabel.font = [UIFont boldSystemFontOfSize:14.0];
    _otopatchStatusLabel.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    [_otopatchPanel addSubview:_otopatchStatusLabel];
    
    _otopatchPercentLabel = [[UILabel alloc] initWithFrame:CGRectMake(panelWidth - 20 - 60, 12, 60, 22)];
    _otopatchPercentLabel.text = @"%0";
    _otopatchPercentLabel.textColor = [UIColor colorWithRed:0.95 green:0.77 blue:0.40 alpha:1.0]; // #F2C465 GOLD
    _otopatchPercentLabel.font = [UIFont boldSystemFontOfSize:16.0];
    _otopatchPercentLabel.textAlignment = NSTextAlignmentRight;
    _otopatchPercentLabel.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin;
    [_otopatchPanel addSubview:_otopatchPercentLabel];
    
    // Progress Bar
    _otopatchProgress = [[UIProgressView alloc] initWithProgressViewStyle:UIProgressViewStyleBar];
    _otopatchProgress.frame = CGRectMake(20, 42, contentW, 14);
    _otopatchProgress.progressTintColor = [UIColor colorWithRed:0.95 green:0.77 blue:0.40 alpha:1.0]; // Gold fill
    _otopatchProgress.trackTintColor = [UIColor colorWithRed:0.12 green:0.10 blue:0.07 alpha:1.0]; // Dark track
    _otopatchProgress.layer.cornerRadius = 7.0f;
    _otopatchProgress.layer.borderWidth = 1.0f;
    _otopatchProgress.layer.borderColor = [UIColor colorWithRed:0.35 green:0.27 blue:0.16 alpha:1.0].CGColor;
    _otopatchProgress.clipsToBounds = YES;
    _otopatchProgress.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    [_otopatchPanel addSubview:_otopatchProgress];
    
    // Bottom Detail Label
    _otopatchDetailLabel = [[UILabel alloc] initWithFrame:CGRectMake(20, 64, contentW, 20)];
    _otopatchDetailLabel.text = @"";
    _otopatchDetailLabel.textColor = [UIColor colorWithRed:0.69 green:0.65 blue:0.58 alpha:1.0]; // #AFA595 SUBTEXT
    _otopatchDetailLabel.font = [UIFont systemFontOfSize:12.0];
    _otopatchDetailLabel.autoresizingMask = UIViewAutoresizingFlexibleWidth;
    [_otopatchPanel addSubview:_otopatchDetailLabel];
    
    // Buttons (Retry / Continue - Initially Hidden)
    _otopatchRetryBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    _otopatchRetryBtn.frame = CGRectMake(20, 62, 130, 34);
    [_otopatchRetryBtn setTitle:@"Tekrar Dene" forState:UIControlStateNormal];
    [_otopatchRetryBtn setTitleColor:[UIColor colorWithRed:0.10 green:0.07 blue:0.03 alpha:1.0] forState:UIControlStateNormal];
    _otopatchRetryBtn.backgroundColor = [UIColor colorWithRed:0.95 green:0.77 blue:0.40 alpha:1.0];
    _otopatchRetryBtn.layer.cornerRadius = 8.0f;
    _otopatchRetryBtn.titleLabel.font = [UIFont boldSystemFontOfSize:13.0];
    _otopatchRetryBtn.hidden = YES;
    [_otopatchRetryBtn addTarget:self action:@selector(onOtopatchRetryClicked) forControlEvents:UIControlEventTouchUpInside];
    [_otopatchPanel addSubview:_otopatchRetryBtn];
    
    _otopatchContinueBtn = [UIButton buttonWithType:UIButtonTypeCustom];
    _otopatchContinueBtn.frame = CGRectMake(160, 62, 130, 34);
    [_otopatchContinueBtn setTitle:@"Oyuna Gir" forState:UIControlStateNormal];
    [_otopatchContinueBtn setTitleColor:[UIColor colorWithRed:0.10 green:0.07 blue:0.03 alpha:1.0] forState:UIControlStateNormal];
    _otopatchContinueBtn.backgroundColor = [UIColor colorWithRed:0.95 green:0.77 blue:0.40 alpha:1.0];
    _otopatchContinueBtn.layer.cornerRadius = 8.0f;
    _otopatchContinueBtn.titleLabel.font = [UIFont boldSystemFontOfSize:13.0];
    _otopatchContinueBtn.hidden = YES;
    [_otopatchContinueBtn addTarget:self action:@selector(onOtopatchContinueClicked) forControlEvents:UIControlEventTouchUpInside];
    [_otopatchPanel addSubview:_otopatchContinueBtn];

    // Start Otopatch Downloader
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *docsPath = paths.firstObject;
    [self startOtopatchUpdateWithDocsPath:docsPath];
}

- (void)startOtopatchUpdateWithDocsPath:(NSString *)docsPath {
    _otopatchRetryBtn.hidden = YES;
    _otopatchContinueBtn.hidden = YES;
    _otopatchDetailLabel.hidden = NO;
    
    [[IOSPackUpdater sharedInstance] startUpdateWithDocsPath:docsPath
        onStatus:^(NSString *statusText) {
            self->_otopatchStatusLabel.textColor = [UIColor colorWithRed:0.94 green:0.90 blue:0.85 alpha:1.0];
            self->_otopatchStatusLabel.text = statusText;
        }
        onProgress:^(NSString *fileName, int fileIndex, int fileCount, int percent, NSString *speedText) {
            self->_otopatchStatusLabel.textColor = [UIColor colorWithRed:0.94 green:0.90 blue:0.85 alpha:1.0];
            self->_otopatchStatusLabel.text = [NSString stringWithFormat:@"İndiriliyor: %@ (%d/%d)", fileName, fileIndex, fileCount];
            self->_otopatchProgress.progress = (float)percent / 100.0f;
            self->_otopatchPercentLabel.text = [NSString stringWithFormat:@"%%%d", percent];
            self->_otopatchDetailLabel.text = [NSString stringWithFormat:@"Hız: %@", speedText];
        }
        onCompletion:^(BOOL success, NSString *errorMsg) {
            if (success) {
                self->_otopatchProgress.progress = 1.0f;
                self->_otopatchPercentLabel.text = @"%100";
                self->_otopatchStatusLabel.textColor = [UIColor colorWithRed:0.94 green:0.90 blue:0.85 alpha:1.0];
                self->_otopatchStatusLabel.text = @"Oyun başlatılıyor...";
                self->_otopatchDetailLabel.text = @"";
                
                // Allow UIKit to render "%100" and "Oyun başlatılıyor..." before executing engine init
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.15 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                    [EAGLContext setCurrentContext:self->_context];
                    glBindFramebuffer(GL_FRAMEBUFFER, self->_defaultFramebuffer);
                    glViewport(0, 0, self->_framebufferWidth, self->_framebufferHeight);

                    NSString *bundlePath = [[NSBundle mainBundle] bundlePath];
                    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
                    NSString *docsPath = paths.firstObject;

                    BOOL initOk = IOS_Init([bundlePath UTF8String], [docsPath UTF8String], self->_targetWidth, self->_targetHeight);
                    if (initOk) {
                        self->_initialized = YES;
                        [UIView animateWithDuration:0.5 animations:^{
                            self->_otopatchContainer.alpha = 0.0f;
                        } completion:^(BOOL finished) {
                            [self->_otopatchContainer removeFromSuperview];
                            self->_otopatchContainer = nil;
                            self->_otopatchDone = YES;
                        }];
                    } else {
                        self->_otopatchStatusLabel.textColor = [UIColor colorWithRed:1.00 green:0.48 blue:0.41 alpha:1.0];
                        self->_otopatchStatusLabel.text = @"Oyun motoru başlatılamadı!";
                        self->_otopatchDetailLabel.hidden = YES;
                        self->_otopatchRetryBtn.hidden = NO;
                        self->_otopatchContinueBtn.hidden = NO;
                        
                        UIAlertController *alert = [UIAlertController alertControllerWithTitle:@"Motor Hatası"
                                                                                       message:@"Oyun motoru (system.py) başlatılamadı.\nLütfen 3uTools -> Documents/syserr.txt dosyasını kontrol edin."
                                                                                preferredStyle:UIAlertControllerStyleAlert];
                        [alert addAction:[UIAlertAction actionWithTitle:@"Tamam" style:UIAlertActionStyleDefault handler:nil]];
                        [self presentViewController:alert animated:YES completion:nil];
                    }
                });
            } else {
                self->_otopatchStatusLabel.textColor = [UIColor colorWithRed:1.00 green:0.48 blue:0.41 alpha:1.0];
                self->_otopatchStatusLabel.text = errorMsg ?: @"Güncelleme hatası!";
                self->_otopatchDetailLabel.hidden = YES;
                self->_otopatchRetryBtn.hidden = NO;
                self->_otopatchContinueBtn.hidden = NO;
            }
        }];
}

- (void)onOtopatchContinueClicked {
    [EAGLContext setCurrentContext:_context];
    glBindFramebuffer(GL_FRAMEBUFFER, _defaultFramebuffer);
    glViewport(0, 0, _framebufferWidth, _framebufferHeight);

    NSString *bundlePath = [[NSBundle mainBundle] bundlePath];
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *docsPath = paths.firstObject;

    BOOL initOk = IOS_Init([bundlePath UTF8String], [docsPath UTF8String], self->_targetWidth, self->_targetHeight);
    if (initOk) {
        self->_initialized = YES;
        [UIView animateWithDuration:0.5 animations:^{
            self->_otopatchContainer.alpha = 0.0f;
        } completion:^(BOOL finished) {
            [self->_otopatchContainer removeFromSuperview];
            self->_otopatchContainer = nil;
            self->_otopatchDone = YES;
        }];
    }
}

- (void)onOtopatchRetryClicked {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *docsPath = paths.firstObject;
    [self startOtopatchUpdateWithDocsPath:docsPath];
}

- (void)setupBuffers {
    [EAGLContext setCurrentContext:_context];

    if (_defaultFramebuffer) {
        glDeleteFramebuffers(1, &_defaultFramebuffer);
        _defaultFramebuffer = 0;
    }
    if (_colorRenderbuffer) {
        glDeleteRenderbuffers(1, &_colorRenderbuffer);
        _colorRenderbuffer = 0;
    }
    if (_depthRenderbuffer) {
        glDeleteRenderbuffers(1, &_depthRenderbuffer);
        _depthRenderbuffer = 0;
    }

    glGenFramebuffers(1, &_defaultFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _defaultFramebuffer);
    g_nDefaultFramebuffer = _defaultFramebuffer;

    CGFloat screenScale = [UIScreen mainScreen].nativeScale;
    if (screenScale <= 0.0) screenScale = [UIScreen mainScreen].scale;
    if (screenScale <= 0.0) screenScale = 2.0;
    self.view.contentScaleFactor = screenScale;
    ((CAEAGLLayer *)self.view.layer).contentsScale = screenScale;

    glGenRenderbuffers(1, &_colorRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
    [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)self.view.layer];
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorRenderbuffer);

    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_framebufferWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_framebufferHeight);

    if (_framebufferWidth <= 0 || _framebufferHeight <= 0) {
        CGSize sz = self.view.bounds.size;
        _framebufferWidth = (GLint)(sz.width * screenScale);
        _framebufferHeight = (GLint)(sz.height * screenScale);
    }

    glGenRenderbuffers(1, &_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _framebufferWidth, _framebufferHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    NSLog(@"[Aspar2 iOS] Framebuffer dimensions: %dx%d, status: 0x%x", _framebufferWidth, _framebufferHeight, status);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        NSLog(@"[Aspar2 iOS] Failed to make complete framebuffer object 0x%x", status);
    }
}

- (void)makeCurrentGLContext {
    if (_context) {
        [EAGLContext setCurrentContext:_context];
        glBindFramebuffer(GL_FRAMEBUFFER, _defaultFramebuffer);
        glViewport(0, 0, _framebufferWidth, _framebufferHeight);
    }
}

- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    if (_otopatchContainer) {
        _otopatchContainer.frame = self.view.bounds;
    }
    [self setupBuffers];
}

- (BOOL)prefersStatusBarHidden {
    return YES;
}

- (BOOL)prefersHomeIndicatorAutoHidden {
    return YES;
}

- (UIInterfaceOrientationMask)supportedInterfaceOrientations {
    return UIInterfaceOrientationMaskLandscape;
}

- (void)renderFrame:(CADisplayLink *)displayLink {
    if (!_otopatchDone) {
        return;
    }

    [self makeCurrentGLContext];

    IOS_Render();

    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
    [_context presentRenderbuffer:GL_RENDERBUFFER];
}

#pragma mark - Touch Handling

- (BOOL)isPointInJoystickZoneX:(float)x y:(float)y {
    if (!IOS_IsGamePhase()) {
        return NO;
    }
    float jx = _joystickX;
    float jy = (_joystickY >= 0.0f) ? _joystickY : (_targetHeight - _joystickH);
    return (x >= jx && x <= jx + _joystickW && y >= jy && y <= jy + _joystickH);
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    CGSize viewSize = self.view.bounds.size;
    float scaleX = (viewSize.width > 0) ? ((float)_targetWidth / viewSize.width) : 1.0f;
    float scaleY = (viewSize.height > 0) ? ((float)_targetHeight / viewSize.height) : 1.0f;

    for (UITouch *touch in touches) {
        CGPoint pt = [touch locationInView:self.view];
        float x = pt.x * scaleX;
        float y = pt.y * scaleY;

        if ([self isPointInJoystickZoneX:x y:y] && _joystickTouch == nil) {
            _joystickTouch = touch;
            IOS_JoystickTouch(0, x, y); // ACTION_DOWN = 0
        } else if (_actionTouch == nil) {
            _actionTouch = touch;
            _actionStartX = x;
            _actionStartY = y;
            _actionLastX = x;
            _actionLastY = y;
            _actionDidDrag = NO;
            IOS_ActionTouch(0, x, y, 0.0f, 0.0f, false);
        } else if (_secondaryTouch == nil && IOS_IsGamePhase() && ![self isPointInJoystickZoneX:x y:y]) {
            _secondaryTouch = touch;
            _secondaryLastX = x;
            _secondaryLastY = y;
            IOS_SecondaryTouch(0, x, y, 0.0f, 0.0f);
        }
    }
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    CGSize viewSize = self.view.bounds.size;
    float scaleX = (viewSize.width > 0) ? ((float)_targetWidth / viewSize.width) : 1.0f;
    float scaleY = (viewSize.height > 0) ? ((float)_targetHeight / viewSize.height) : 1.0f;

    for (UITouch *touch in touches) {
        CGPoint pt = [touch locationInView:self.view];
        float x = pt.x * scaleX;
        float y = pt.y * scaleY;

        if (touch == _joystickTouch) {
            IOS_JoystickTouch(2, x, y); // ACTION_MOVE = 2
        } else if (touch == _actionTouch) {
            float dx = x - _actionLastX;
            float dy = y - _actionLastY;
            _actionLastX = x;
            _actionLastY = y;

            float distSq = (x - _actionStartX) * (x - _actionStartX) + (y - _actionStartY) * (y - _actionStartY);
            if (distSq > 100.0f) {
                _actionDidDrag = YES;
            }
            IOS_ActionTouch(2, x, y, dx, dy, _actionDidDrag);
        } else if (touch == _secondaryTouch) {
            float sdx = x - _secondaryLastX;
            float sdy = y - _secondaryLastY;
            _secondaryLastX = x;
            _secondaryLastY = y;
            IOS_SecondaryTouch(2, x, y, sdx, sdy);
        }
    }
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    CGSize viewSize = self.view.bounds.size;
    float scaleX = (viewSize.width > 0) ? ((float)_targetWidth / viewSize.width) : 1.0f;
    float scaleY = (viewSize.height > 0) ? ((float)_targetHeight / viewSize.height) : 1.0f;

    for (UITouch *touch in touches) {
        CGPoint pt = [touch locationInView:self.view];
        float x = pt.x * scaleX;
        float y = pt.y * scaleY;

        if (touch == _joystickTouch) {
            IOS_JoystickTouch(1, x, y); // ACTION_UP = 1
            _joystickTouch = nil;
        } else if (touch == _actionTouch) {
            IOS_ActionTouch(1, x, y, 0.0f, 0.0f, _actionDidDrag);
            _actionTouch = nil;
        } else if (touch == _secondaryTouch) {
            IOS_SecondaryTouch(1, x, y, 0.0f, 0.0f);
            _secondaryTouch = nil;
        }
    }
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [self touchesEnded:touches withEvent:event];
}

#pragma mark - Virtual Keyboard

- (void)showKeyboard:(const char *)initialText {
    NSString *initStr = @"";
    if (initialText && strlen(initialText) > 0) {
        initStr = [NSString stringWithUTF8String:initialText];
        if (!initStr) {
            initStr = [NSString stringWithCString:initialText encoding:NSWindowsCP1254StringEncoding];
        }
        if (!initStr) {
            initStr = [NSString stringWithCString:initialText encoding:NSISOLatin1StringEncoding];
        }
        if (!initStr) initStr = @"";
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        self->_ignoreTextChange = YES;
        self->_hiddenTextField.text = initStr;
        self->_ignoreTextChange = NO;
        [self->_hiddenTextField becomeFirstResponder];
    });
}

- (void)hideKeyboard {
    dispatch_async(dispatch_get_main_queue(), ^{
        [self->_hiddenTextField resignFirstResponder];
    });
}

- (void)textFieldDidChange:(UITextField *)textField {
    if (_ignoreTextChange) return;
    NSString *text = textField.text ?: @"";
    IOS_OnKeyboardText([text UTF8String]);
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
    IOS_OnKeyboardEnter();
    [self hideKeyboard];
    return YES;
}

#pragma mark - Web View (Nesne Market)

- (void)showWebPage:(const char *)url {
    NSString *urlStr = [NSString stringWithUTF8String:url];
    dispatch_async(dispatch_get_main_queue(), ^{
        if (self->_webContainerView) {
            [self hideWebPage];
        }

        self->_isWebShowing = YES;

        CGRect bounds = self.view.bounds;
        self->_webContainerView = [[UIView alloc] initWithFrame:bounds];
        self->_webContainerView.backgroundColor = [UIColor colorWithRed:0.04 green:0.04 blue:0.06 alpha:0.98];

        WKWebViewConfiguration *config = [[WKWebViewConfiguration alloc] init];
        config.allowsInlineMediaPlayback = YES;
        config.websiteDataStore = [WKWebsiteDataStore defaultDataStore];

        self->_webView = [[WKWebView alloc] initWithFrame:bounds configuration:config];
        self->_webView.navigationDelegate = self;
        self->_webView.backgroundColor = [UIColor colorWithRed:0.07 green:0.07 blue:0.09 alpha:1.0];
        self->_webView.opaque = NO;
        self->_webView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        [self->_webContainerView addSubview:self->_webView];

        // Progress bar
        self->_progressBar = [[UIProgressView alloc] initWithProgressViewStyle:UIProgressViewStyleBar];
        self->_progressBar.frame = CGRectMake(0, 0, bounds.size.width, 3);
        self->_progressBar.progressTintColor = [UIColor colorWithRed:0.95 green:0.35 blue:0.15 alpha:1.0];
        [self->_webContainerView addSubview:self->_progressBar];

        // Close button (Top-Right "✕ KAPAT")
        UIButton *closeBtn = [UIButton buttonWithType:UIButtonTypeCustom];
        closeBtn.frame = CGRectMake(bounds.size.width - 120, 16, 104, 36);
        closeBtn.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleBottomMargin;
        [closeBtn setTitle:@"✕ KAPAT" forState:UIControlStateNormal];
        closeBtn.titleLabel.font = [UIFont boldSystemFontOfSize:13.0];
        closeBtn.backgroundColor = [UIColor colorWithRed:0.78 green:0.16 blue:0.16 alpha:1.0];
        closeBtn.layer.cornerRadius = 18.0;
        closeBtn.layer.borderColor = [UIColor colorWithRed:0.94 green:0.33 blue:0.33 alpha:1.0].CGColor;
        closeBtn.layer.borderWidth = 1.0;
        [closeBtn addTarget:self action:@selector(hideWebPage) forControlEvents:UIControlEventTouchUpInside];
        [self->_webContainerView addSubview:closeBtn];

        [self.view addSubview:self->_webContainerView];

        NSURL *nsUrl = [NSURL URLWithString:urlStr];
        if (nsUrl) {
            NSURLRequest *req = [NSURLRequest requestWithURL:nsUrl];
            [self->_webView loadRequest:req];
        }
    });
}

- (void)hideWebPage {
    dispatch_async(dispatch_get_main_queue(), ^{
        self->_isWebShowing = NO;
        if (self->_webView) {
            [self->_webView stopLoading];
            [self->_webView removeFromSuperview];
            self->_webView = nil;
        }
        if (self->_webContainerView) {
            [self->_webContainerView removeFromSuperview];
            self->_webContainerView = nil;
        }
    });
}

- (BOOL)isWebShowing {
    return _isWebShowing;
}

- (void)webView:(WKWebView *)webView decidePolicyForNavigationAction:(WKNavigationAction *)navigationAction decisionHandler:(void (^)(WKNavigationActionPolicy))decisionHandler {
    NSURL *url = navigationAction.request.URL;
    NSString *scheme = url.scheme.lowercaseString;

    if ([scheme isEqualToString:@"http"] || [scheme isEqualToString:@"https"] || [scheme isEqualToString:@"about"]) {
        decisionHandler(WKNavigationActionPolicyAllow);
    } else {
        // Deep link (banking, payment apps, papara, ininal, etc.)
        [[UIApplication sharedApplication] openURL:url options:@{} completionHandler:nil];
        decisionHandler(WKNavigationActionPolicyCancel);
    }
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation {
    if (_progressBar) {
        [_progressBar setHidden:YES];
    }
}

- (void)setJoystickZoneWithX:(float)x y:(float)y width:(float)width height:(float)height {
    _joystickX = x;
    _joystickY = y;
    _joystickW = width;
    _joystickH = height;
}

- (void)dealloc {
    if (_defaultFramebuffer) glDeleteFramebuffers(1, &_defaultFramebuffer);
    if (_colorRenderbuffer) glDeleteRenderbuffers(1, &_colorRenderbuffer);
    if (_depthRenderbuffer) glDeleteRenderbuffers(1, &_depthRenderbuffer);
}

@end
