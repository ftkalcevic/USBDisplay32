#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>


#define DLFB_IOCTL_RETURN_EDID   0xAD
#define DLFB_IOCTL_REPORT_DAMAGE 0xAA
struct dloarea {
        int x, y;
        int w, h;
        int x2, y2;
};



int main(int argc, char *argv[] )
{
    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;
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

#define X 00
#define Y 00
#define W (vinfo.xres)
#define H (vinfo.yres)

    struct dloarea area;
    area.x = X;
    area.y = Y;
    area.w = W;
    area.h = H;

    if (ioctl(fbfd, DLFB_IOCTL_REPORT_DAMAGE, &area)) {
        printf("Error: failed to damage framebuffer.\n");
    }

    close(fbfd);
    return 0;
}
