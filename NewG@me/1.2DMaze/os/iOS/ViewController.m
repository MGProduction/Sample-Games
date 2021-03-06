//
//  ViewController.m
//  P@cM@n
//
//  Created by Marco Giorgini on 21/04/13.
//  Copyright (c) 2013 Marco Giorgini. All rights reserved.
//

#import "ViewController.h"

#include "g_main.h"

int os_touch_portrait;

@interface ViewController () {
}
@property (strong, nonatomic) EAGLContext *context;
@property (strong, nonatomic) GLKBaseEffect *effect;

- (void)setupGL;
- (void)tearDownGL;
- (CGRect)currentScreenBoundsDependOnOrientation;

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];

    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat16;
    
    self.preferredFramesPerSecond = 30;
    
    [self setupGL];
}

- (void)viewDidUnload
{    
    [super viewDidUnload];
    
    [self tearDownGL];
    
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
	self.context = nil;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc. that aren't in use.
}



#define IsIOS8 (NSFoundationVersionNumber > NSFoundationVersionNumber_iOS_7_1)
int             bOLDIOS=0;
#define IOS_INTERFACEORIENTATION UIInterfaceOrientationMaskLandscape

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
    if(IOS_INTERFACEORIENTATION==UIInterfaceOrientationMaskLandscape)
    {
        if(IsIOS8)
            ;
        else
            if(fromInterfaceOrientation==UIInterfaceOrientationLandscapeLeft)
            {os_touch_flip=0;}
            else
                if(fromInterfaceOrientation==UIInterfaceOrientationLandscapeRight)
                {os_touch_flip=1;}        
    }
    else
    {
        if(fromInterfaceOrientation==UIInterfaceOrientationPortrait)
        {os_touch_flip=1;}
        else
            if(fromInterfaceOrientation==UIInterfaceOrientationPortraitUpsideDown)
            {os_touch_flip=0;}
    }
}

-(UIInterfaceOrientation) preferredInterfaceOrientationForPresentation{
    UIDeviceOrientation deviceOrientation = [[UIDevice currentDevice] orientation];
    if(IOS_INTERFACEORIENTATION==UIInterfaceOrientationMaskLandscape)
    {
        if(deviceOrientation==UIDeviceOrientationFaceUp)
        {
            UIDeviceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;   
            if((orientation==UIDeviceOrientationLandscapeLeft)||(orientation==UIDeviceOrientationLandscapeRight))
                return orientation;
            else
                return UIDeviceOrientationLandscapeLeft; 
        }
        else
            if((deviceOrientation==UIDeviceOrientationLandscapeLeft)||(deviceOrientation==UIDeviceOrientationLandscapeRight))
                return deviceOrientation;
            else
                return UIDeviceOrientationLandscapeLeft;        
    }
    else
    {
        if(deviceOrientation==UIDeviceOrientationFaceUp)
        {
            UIDeviceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;   
            if((orientation==UIDeviceOrientationPortrait)||(orientation==UIDeviceOrientationPortraitUpsideDown))
                return orientation;
            else
                return UIDeviceOrientationPortrait; 
        }
        else
            if((deviceOrientation==UIDeviceOrientationPortrait)||(deviceOrientation==UIDeviceOrientationPortraitUpsideDown))
                return deviceOrientation;
            else
                return UIDeviceOrientationPortrait;
    }
}

- (BOOL)shouldAutorotate {
    return YES;
}

- (NSUInteger)supportedInterfaceOrientations {
    return IOS_INTERFACEORIENTATION;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    if(bOLDIOS)
    {
        
        if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
            return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown)&&(interfaceOrientation != UIInterfaceOrientationPortrait);
        } else {
            return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown)&&(interfaceOrientation != UIInterfaceOrientationPortrait);
        }    
        
    }
    
    //return ((interfaceOrientation == UIInterfaceOrientationPortrait));//||(interfaceOrientation == UIDeviceOrientationPortraitUpsideDown));
    if (interfaceOrientation == UIDeviceOrientationLandscapeLeft )
    {
        if((os_touch_flip!=1)||(os_portrait!=0))
        {
            os_touch_flip=1;
        }
    }
    else
        if (interfaceOrientation == UIDeviceOrientationLandscapeRight)
        {
            if((os_touch_flip!=0)||(os_portrait!=0))
            {                
                os_touch_flip=0;
            }
        }  
    return YES;
}

-(CGRect)currentScreenBoundsDependOnOrientation
{
    NSString *reqSysVer = @"8.0";
    NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
    if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending)
        return [UIScreen mainScreen].bounds;
    if(IsIOS8){
        return [UIScreen mainScreen].bounds ;
    }
    
    CGRect screenBounds = [UIScreen mainScreen].bounds ;
    CGFloat width = CGRectGetWidth(screenBounds)  ;
    CGFloat height = CGRectGetHeight(screenBounds) ;
    UIInterfaceOrientation interfaceOrientation = [UIApplication sharedApplication].statusBarOrientation;
    
    if(UIInterfaceOrientationIsPortrait(interfaceOrientation)){
        screenBounds.size = CGSizeMake(width, height);
        NSLog(@"Portrait Height: %f", screenBounds.size.height);
    }else if(UIInterfaceOrientationIsLandscape(interfaceOrientation)){
        screenBounds.size = CGSizeMake(height, width);
        NSLog(@"Landscape Height: %f", screenBounds.size.height);
    }
    
    return screenBounds ;
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];
    
    NSString *homeDir = [[NSBundle mainBundle] resourcePath];
    CGRect rect=[self currentScreenBoundsDependOnOrientation];
    float width=rect.size.width;
    float height=rect.size.height;
    GLKView *view = (GLKView *)self.view;
    if ([view respondsToSelector:@selector(contentScaleFactor)])
    {
        float scale=[[UIScreen mainScreen] scale];
        if([[UIScreen mainScreen] respondsToSelector:@selector(nativeScale)])
            scale=[[UIScreen mainScreen] nativeScale];
        width*=scale;
        height*=scale;
        os_scale=scale;
    }    
    
    UIDeviceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;
    os_flip=os_touch_flip=0;
    if(IsIOS8)
        os_touch_portrait=0;
    else
    {
        if(IOS_INTERFACEORIENTATION==UIInterfaceOrientationMaskLandscape)
            os_touch_portrait=1;   
        else
            if((orientation==UIDeviceOrientationPortrait)||(orientation==UIDeviceOrientationPortraitUpsideDown))
                os_touch_portrait=0;
            else
                os_touch_portrait=1;
        
    }
    
    [view setMultipleTouchEnabled:YES];
    
    NSString *reqSysVer = @"6.0";
    NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
    BOOL osVersionSupported = ([currSysVer compare:reqSysVer 
                                           options:NSNumericSearch] != NSOrderedAscending);
    if(osVersionSupported)
    {
        float t=width;
        width=height;
        height=t;
        bOLDIOS=1;
    }
    if(IsIOS8)
        ;
    else
        if((orientation==UIDeviceOrientationLandscapeRight))//||(orientation==UIDeviceOrientationPortrait))
            os_touch_flip=1;        
    
    GAME_init(&g_W,[[homeDir stringByAppendingFormat:@"/"] cStringUsingEncoding: NSASCIIStringEncoding],height,width);
}

- (void)tearDownGL
{
    GAME_reset(&g_W);
    
    [EAGLContext setCurrentContext:self.context];
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{     
    GAME_loop(&g_W);
}

extern int   os_x[10],os_y[10],os_status[10];
extern int   os_np,os_flip,os_dblclick;
int          b_os_x[10],b_os_y[10];

- (void) handleTouches:(NSSet*)touches withEvent:(UIEvent*)event
{
    int    i,err=0;
    float  lvideo_w=os_video_w/(float)os_scale;
    float  lvideo_h=os_video_h/(float)os_scale;
    
    NSSet *t = [event allTouches];
    while(os_np&&(os_status[os_np-1]==0)) os_np--;
    for (UITouch *myTouch in t)
    {
        CGPoint touchLocation = [myTouch locationInView:nil];
        CGPoint ptouchLocation = [myTouch previousLocationInView:nil];
        int px,py,opx,opy;
        if(IsIOS8)
        {
            opx=px = touchLocation.x;
            opy=py = touchLocation.y;
        }
        else
            if ( os_touch_portrait ) {
                opx=px = touchLocation.y;
                opy=py = ( lvideo_h - 1 ) - touchLocation.x;
            } else {
                opx=px = touchLocation.x;
                opy=py = touchLocation.y;
            }

        if (myTouch.phase == UITouchPhaseBegan)
        {
            // new touch handler
            for(i=0;i<10;i++)
                if(os_status[i]==0)
                {
                    os_status[ i ] = 1|256;
                    b_os_x[i]=px;b_os_y[i]=py;
                    if(i>=os_np)
                        os_np++;
                    break;
                }
            if(i==10)
            {
                err=1;
            }
            
        }
        else
            if ((myTouch.phase == UITouchPhaseMoved)||(myTouch.phase==UITouchPhaseStationary)) {
                // touch moved handler
                int i,bdist=100000,best=-1;
                for(i=0;i<os_np;i++)
                    if(os_status[i])
                    {
                        int dist=((b_os_x[i]-opx)*(b_os_x[i]-opx))+((b_os_y[i]-opy)*(b_os_y[i]-opy));
                        if(dist<bdist)
                        {
                            bdist=dist;
                            best=i;
                        }
                    }
                if(best!=-1)
                {
                    i=best;
                    os_status[i]=2|256;
                    b_os_x[i]=px;b_os_y[i]=py;
                }
                else
                {
                    err=1;
                }
            }
            else
                if ((myTouch.phase == UITouchPhaseEnded)||(myTouch.phase == UITouchPhaseCancelled)) {
                    int i,bdist=100000,best=-1;
                    for(i=0;i<os_np;i++)
                        if(os_status[i])
                        {
                            int dist=((b_os_x[i]-opx)*(b_os_x[i]-opx))+((b_os_y[i]-opy)*(b_os_y[i]-opy));
                            if(dist<bdist)
                            {
                                bdist=dist;
                                best=i;
                            }
                        }
                    if(best!=-1)
                    {
                        i=best;
                        os_status[i]=3|256;
                        b_os_x[i]=px;b_os_y[i]=py;
                    }
                    else
                    {
                        err=1;
                    }
                }
                else
                {
                    err=1;
                }
        
    }
    
    for(i=0;i<10;i++)
        if(os_status[i]&256)
            os_status[i]-=256;
        else
            if(os_status[i]!=0)
                os_status[i]=3;
    
    if(os_flip||os_touch_flip)
    {
        for(i=0;i<10;i++)
        {
            os_x[i]=(lvideo_w-b_os_x[i])*os_scale;
            os_y[i]=(lvideo_h-b_os_y[i])*os_scale;
        }
    }
    else
    {
        for(i=0;i<10;i++)
        {
            os_x[i]=(b_os_x[i])*os_scale;
            os_y[i]=(b_os_y[i])*os_scale;
        }
    }
	
}

- (void) touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event {
	[self handleTouches:touches withEvent:event];
}

- (void) touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {
	[self handleTouches:touches withEvent:event];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	[self handleTouches:touches withEvent:event];
}


- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	[self handleTouches:touches withEvent:event];
}

- (void)setframerate:(int)fps
{
    self.preferredFramesPerSecond = fps;
}

- (BOOL)prefersStatusBarHidden {return YES;}

@end
