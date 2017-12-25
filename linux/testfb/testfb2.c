#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <stdint.h>
#include <time.h>
#include <sys/ioctl.h>
#include <string.h>


#define DLFB_IOCTL_RETURN_EDID   0xAD
#define DLFB_IOCTL_REPORT_DAMAGE 0xAA
struct dloarea {
        int x, y;
        int w, h;
        int x2, y2;
};


static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;
static char *fbp = 0;


static void refresh(int fd, int x, int y, int w, int h)
{
    struct dloarea area;
    area.x = x;
    area.y = y;
    area.w = w;
    area.h = h;

    if ( w < 0 )
    {
        area.x += w;
        area.w = -w;
    }

    if ( h < 0 )
    {
        area.y += h;
        area.h = -h;
    }

    if ( w == 0 )
        area.w = 1;
    if ( h == 0 )
        area.h = 1;

    if (ioctl(fd, DLFB_IOCTL_REPORT_DAMAGE, &area)) {
        printf("Error: failed to damage framebuffer.\n");
    }
 }

static void plotc(int x, int y, unsigned short c)
{
    if ( x < 0 || x >= vinfo.xres ) return;
    if ( y < 0 || y >= vinfo.yres ) return;
        
    long int location =    (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                           (y+vinfo.yoffset) * finfo.line_length;

    *((unsigned short int*)(fbp + location)) = c;

    printf("%d,%d\n", x, y );
}


static void plot(int x, int y, double c)
{

    uint16_t r = 255*c;
    uint16_t g = 255*c;
    uint16_t b = 255*c;

    //assume 16bpp
    unsigned short int t = ((r>>3)<<11) | ((g>>2) << 5) | (b>>3);
    plotc(x,y,t);
}

int  ipart(double x)
{
    return (int)floor(x);
}

double iround(double x)
{
    return ipart(x + 0.5);
}

double fpart(double x)
{
    return x - floor(x);
}

double rfpart(double x)
{
    return 1.0 - fpart(x);
}

#define swap(x,y) {double _n = x; x = y; y = _n;}
double drawline(double x0,double y0,double x1,double y1)
{
    int  steep = abs(y1 - y0) > abs(x1 - x0);
        
    if (steep)
    {
        swap(x0, y0);
        swap(x1, y1);
    }
    if ( x0 > x1 )
    {
        swap(x0, x1);
        swap(y0, y1);
    }

    double dx = x1 - x0;
    double dy = y1 - y0;
    double gradient;
    if ( dx == 0.0 )
        gradient = 1.0;
    else
        gradient = dy / dx;

    // handle first endpoint
    double xend = iround(x0);
    double yend = y0 + gradient * (xend - x0);
    double xgap = rfpart(x0 + 0.5);
    int xpxl1 = xend; // this will be used in the main loop
    int ypxl1 = ipart(yend);
    if (steep)
	{
        plot(ypxl1,   xpxl1, rfpart(yend) * xgap);
        plot(ypxl1+1, xpxl1,  fpart(yend) * xgap);
	}
    else
	{
        plot(xpxl1, ypxl1  , rfpart(yend) * xgap);
        plot(xpxl1, ypxl1+1,  fpart(yend) * xgap);
    }
    double intery = yend + gradient; // first y-intersection for the main loop

    // handle second endpoint
    xend = iround(x1);
    yend = y1 + gradient * (xend - x1);
    xgap = fpart(x1 + 0.5);
    int xpxl2 = xend; //this will be used in the main loop
    int ypxl2 = ipart(yend);
    if (steep)
	{
        plot(ypxl2  , xpxl2, rfpart(yend) * xgap);
        plot(ypxl2+1, xpxl2,  fpart(yend) * xgap);
	}
    else
	{
        plot(xpxl2, ypxl2,  rfpart(yend) * xgap);
        plot(xpxl2, ypxl2+1, fpart(yend) * xgap);
	}

        printf("xpxl1=%d,xpxl2=%d\n", xpxl1, xpxl2 );
    // main loop
     /*
    if (steep)
	{
        for (int x = xpxl1 + 1; x <= xpxl2 - 1; x++ )
		{
            printf("%d,%d %f:%f\n", (int)ipart(intery), x, intery, gradient );
            plot(ipart(intery)  , x, rfpart(intery));
            plot(ipart(intery)+1, x,  fpart(intery));
            intery = intery + gradient;
        }	
    }
    else
    {
        for ( int x = xpxl1 + 1; x <= xpxl2 - 1; x++ )
        {
            plot(x, ipart(intery),  rfpart(intery));
            plot(x, ipart(intery)+1, fpart(intery));
            intery = intery + gradient;
         }
    }
    */
    for ( int x = xpxl1 + 1; x <= xpxl2 - 1; x++ )
    {
        if ( steep )
        {
            plot(ipart(intery),   x, rfpart(intery));
            plot(ipart(intery)+1, x, fpart(intery));
        }
        else
        {
            plot(x, ipart(intery),  rfpart(intery));
            plot(x, ipart(intery)+1, fpart(intery));
        }
        intery = intery + gradient;
     }
 }


int main(int argc, char *argv[] )
{
    int fbfd = 0;
    long int screensize = 0;
    int x = 0, y = 0;
    long int location = 0;

    // Open the file for reading and writing
    fbfd = open(argv[1], O_RDWR);
    if (!fbfd) {
        printf("Error: cannot open framebuffer device: %s.\n", argv[1]);
        exit(1);
    }
    printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
        printf("Error reading fixed information.\n");
        //exit(2);
    }

     // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
        printf("Error reading variable information.\n");
        //exit(3);
    }

   printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel );
   printf("+%d+%d\n", vinfo.xoffset, vinfo.yoffset );

    // Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
                       fbfd, 0);
    if ((int)(void *)fbp == -1) {
        printf("Error: failed to map framebuffer device to memory.\n");
        exit(4);
    }
    printf("The framebuffer device was mapped to memory successfully.\n");

#define X 00
#define Y 00
#define W (vinfo.xres)
#define H (vinfo.yres)

    // Clear screen
    {
        printf("Clearscreen.\n");
        memset( fbp, 0, W*H*vinfo.bits_per_pixel / 8);
        refresh(fbfd, X,Y,W,H);
    }
if (0)
    {
        struct dloarea area[] = {   
                    { 200,50, 81,70 },
                    { 200,48, 80,72 },
                    { 200,47, 78,73 },
                    { 200,45, 77,75 },
                    { 200,44, 76,76 } };

            usleep(500000);
        for ( int i = 0; i < 5; i++ )
        {
            unsigned short int r,g,b;
            if ( i & 1 )
            {
                r = 255;
                g = 255;
                b = 0;
            }
            else
            {
                r = 0 ;
                g = 255;
                b = 255;
            }
            unsigned short int t = ((r>>3)<<11) | ((g>>2) << 5) | (b>>3);
            printf("t=%04X\n",t);
            memset( fbp, 0, W*H*vinfo.bits_per_pixel / 8);
            for ( int x = 0; x < 90; x++ )
            {
                plotc(W/2+x,H/2-x,t);
            }

            if (ioctl(fbfd, DLFB_IOCTL_REPORT_DAMAGE, area+i)) {
                printf("Error: failed to damage framebuffer.\n");
            }
            usleep(5000000);
        }
 }
if(0)
{
    double hz = 10;
    double angle = 0;
    double x0 = W/2;
    double y0 = H/2;
    double l = H/2*0.9;
    for (;;)
    {
        static int n = 0;
        n++;
        uint16_t *ptr = fbp;
        for ( int y = 0; y < H; y++ )
            for ( int x = 0; x < W; x++ )
                *ptr++ = (((y>>3)|(x>>3))&1)?(n&1?0xFFFF:0x11AA):(n&1?0x11AA:0xFFFF);

        double x1 = x0 + l*cos( angle / 180.0 * M_PI );
        double y1 = y0 - l*sin( angle / 180.0 * M_PI );
        printf("drawline %f %f,%f-%f,%f.\n",angle, x0,y0,x1,y1);

        //refresh(fbfd, 0,0,W,H);
        refresh(fbfd, x0,y0, x1-x0, y1-y0 );
        usleep(1000000/hz);

        angle += 1.0;
    }
}

if (1)
{
    double hz = 10;
    double angle = 44;
    double x0 = W/2;
    double y0 = H/2;
    double l = H/2*0.9;
    for (;;)
    {
        //if ( angle > 47 )
            //break;
        memset( fbp, 0, W*H*vinfo.bits_per_pixel / 8);

        double x1 = x0 + l*cos( angle / 180.0 * M_PI );
        double y1 = y0 - l*sin( angle / 180.0 * M_PI );
        printf("drawline %f %f,%f-%f,%f.\n",angle, x0,y0,x1,y1);
        drawline( x0, y0, x1, y1 );

        //refresh(fbfd, 0,0,W,H);
        refresh(fbfd, x0,y0, x1-x0, y1-y0 );
        usleep(1000000/hz);

        angle += 1.0;
    }
}

if (0)
{

    // Figure out where in memory to put the pixel
    for ( y = Y; y < Y+H; y++ )
        for ( x = X; x < X+W; x++ ) {

            char b = 100;        // Some blue
            char g = 15+(x-100)/2;     // A little green
            char r = 200-(y-100)/5;    // A lot of red

	    if ( x != y && x != 0 &&y != 0 && x != W-1 && y != H-1)
		//r = g = b = 0;
            location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                       (y+vinfo.yoffset) * finfo.line_length;

            { //assume 16bpp
                unsigned short int t = ((r>>3)<<11) | ((g>>2) << 5) | (b>>3);
                *((unsigned short int*)(fbp + location)) = t;
            }

            struct dloarea area;
            area.x = x;
            area.y = y;
            area.w = 2;
            area.h = 2;

            if (ioctl(fbfd, DLFB_IOCTL_REPORT_DAMAGE, &area)) {
                printf("Error: failed to damage framebuffer.\n");
            }

            usleep(10000);

        }
    struct dloarea area;
    area.x = X;
    area.y = Y;
    area.w = W;
    area.h = H;

    if (ioctl(fbfd, DLFB_IOCTL_REPORT_DAMAGE, &area)) {
        printf("Error: failed to damage framebuffer.\n");
    }
}
    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}
