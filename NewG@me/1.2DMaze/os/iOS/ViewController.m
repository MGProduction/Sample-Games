//
//  ViewController.m
//  P@cM@n
//
//  Created by Marco Giorgini on 21/04/13.
//  Copyright (c) 2013 Marco Giorgini. All rights reserved.
//

#import "ViewController.h"

#include "g_main.h"

@interface ViewController () {
}
@property (strong, nonatomic) EAGLContext *context;
@property (strong, nonatomic) GLKBaseEffect *effect;

- (void)setupGL;
- (void)tearDownGL;

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

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    if (interfaceOrientation == UIDeviceOrientationLandscapeLeft )
    {
        if((os_touch_flip!=1)||(os_portrait!=0))
        {
            /*if(os_portrait==1)
            {float t;t=os_video_w;os_video_w=os_video_h;os_video_h=t;}
            os_portrait=0;*/
            os_touch_flip=1;
            //[self changeOrientation];
            //SYSTEM_worldrotate();
        }
    }
    else
        if (interfaceOrientation == UIDeviceOrientationLandscapeRight)
        {
            if((os_touch_flip!=0)||(os_portrait!=0))
            {
                /*if(os_portrait==1)
                {float t;t=os_video_w;os_video_w=os_video_h;os_video_h=t;}
                os_portrait=0;*/
                os_touch_flip=0;
                //[self changeOrientation];
                //SYSTEM_worldrotate();
            }
        }
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
    } else {
        return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
    }
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];
    
    NSString *homeDir = [[NSBundle mainBundle] resourcePath];
    float width=[[UIScreen mainScreen] bounds].size.width;
	float height=[[UIScreen mainScreen] bounds].size.height;
    GLKView *view = (GLKView *)self.view;
    if ([view respondsToSelector:@selector(contentScaleFactor)])
    {
        float scale=[[UIScreen mainScreen] scale];
        width*=scale;
        height*=scale;
        os_scale=(int)scale;
    }
    
    os_portrait=1;
    os_flip=os_touch_flip=0;
    
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
int   b_os_x[10],b_os_y[10];

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
        if(os_portrait==1)
        {
            px =(int)(lvideo_w-floor(touchLocation.y));
            py= (int)floor(touchLocation.x);
            opx =(int)(lvideo_w-floor(ptouchLocation.y));
            opy= (int)floor(ptouchLocation.x);
        }
        else
        {
            px =(int)(floor(touchLocation.x));
            py= (int)floor(touchLocation.y);
            opx =(int)(floor(ptouchLocation.x));
            opy= (int)floor(ptouchLocation.y);
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
	//GameTouches( touchCount, points );
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

@end
