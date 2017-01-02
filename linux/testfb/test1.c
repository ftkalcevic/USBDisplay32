#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <stdint.h>


#define DLFB_IOCTL_RETURN_EDID   0xAD
#define DLFB_IOCTL_REPORT_DAMAGE 0xAA
struct dloarea {
        int x, y;
        int w, h;
        int x2, y2;
};

#define RGB(r,g,b) ((uint16_t)(((((r)>>3)&0x1f) << 11) | ((((g)>>2)&0x3f) << 5) | (((b)>>3)&0x1f) ))

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
char *fbp = 0;

void pixel( int x, int y, int c )
{
    int location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                   (y+vinfo.yoffset) * finfo.line_length;

    unsigned short int t = c;
    *((unsigned short int*)(fbp + location)) = t;
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

//finfo.line_length = 800;
    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
        printf("Error reading fixed information.\n");
        //exit(2);
    }

    //vinfo.xres = 480;
    //vinfo.yres = 272;
    //vinfo.bits_per_pixel = 16;
    //vinfo.xoffset = 0;
    //vinfo.yoffset = 0;

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
    if ((int)fbp == -1) {
        printf("Error: failed to map framebuffer device to memory.\n");
        exit(2);
    }
    printf("The framebuffer device was mapped to memory successfully.\n");

#define X 00
#define Y 00
#define W (vinfo.xres)
#define H (vinfo.yres)

    struct dloarea area;
/*
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

            if ( vinfo.bits_per_pixel == 32 ) {
                *(fbp + location) = b;
                *(fbp + location + 1) = g;
                *(fbp + location + 2) = r;
                *(fbp + location + 3) = 0;      // No transparency
            } else  { //assume 16bpp
                unsigned short int t = RGB(r,g,b);
                *((unsigned short int*)(fbp + location)) = t;
            }

        }
    area.x = X;
    area.y = Y;
    area.w = W;
    area.h = H;

    if (ioctl(fbfd, DLFB_IOCTL_REPORT_DAMAGE, &area)) {
        printf("Error: failed to damage framebuffer.\n");
    }
*/
    for ( y = 0; y <= H - 20; y++ )
    for ( x = 0; x <= W - 20; x++ )
    {
        int i,j;
        for ( i = 0; i < 20; i++ )
            for ( j = 0; j < 20; j++ )
            {
                char b = 0;
                char g = 0;
                char r = 255;

                if ( vinfo.bits_per_pixel == 32 ) {
                    *(fbp + location) = b;
                    *(fbp + location + 1) = g;
                    *(fbp + location + 2) = r;
                    *(fbp + location + 3) = 0;      // No transparency
                } else  { //assume 16bpp
                    unsigned short int t = RGB(r,g,b);
                    *((unsigned short int*)(fbp + location)) = t;
                }
                pixel( x+i, y+j, RGB(r,g,b) );
            }
            for ( i = 0; i < 20; i++ )
            {
                int c = RGB(255,0,0);
                pixel( x+i,y,c);
                pixel( x+i,y+20-1,c);
                pixel( x,y+i,c);
                pixel( x+20-1,y+i,c);
                c = RGB(0,0,255);
                pixel( x+i,y+i,c);
                pixel( x+i,y+20-i,c);
            }
        area.x = x;
        area.y = y;
        area.w = 20;
        area.h = 20;

        if (ioctl(fbfd, DLFB_IOCTL_REPORT_DAMAGE, &area)) {
            printf("Error: failed to damage framebuffer.\n");
        }

        getchar();
    }

    munmap(fbp, screensize);
    close(fbfd);
    return 0;
}
