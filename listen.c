#include <ApplicationServices/ApplicationServices.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <fcntl.h>   /* File control definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <string.h> 

//scancodes to mac keycodes
uint8_t keymap[] = {
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0xc,0,0,0,0,0x6,0x1,0x0,0xd,
    0,0,0,0x8,0x7,0x2,0xe,0,0,0,
    0,0x31,0x9,0x3,0x11,0xf,0,0,0,0x2d,
    0xb,0x4,0x5,0x10,0,0,0,0,0x2e,0x26,
    0x20,0,0,0,0,0,0x28,0x22,0x1f,0,
    0,0,0,0,0,0x25,0,0x23,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0x33,0,0,0,0,0,0,0,
};
int open_port(void) {
    int fd = open("/dev/tty.usbmodemfa131", O_RDWR | O_NOCTTY | O_NDELAY);      

    if (fd == -1)     {
        perror("cannot open serial port");
    } else { 
        fcntl(fd, F_SETFL, 0);
    }

    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    options.c_cflag |= (CLOCAL | CREAD);
    tcsetattr(fd, TCSANOW, &options);
    options.c_cflag &= ~CSIZE; 
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cflag |= (IXON | IXOFF | IXANY); // xon & xoff on
    
    return (fd);
 }

CGKeyCode scanToKey(uint8_t scancode) {
    return (CGKeyCode) keymap[scancode];
}
void keyPress(uint8_t keycode) {
    CGEventRef keyDown = CGEventCreateKeyboardEvent(NULL, keycode, true);
    CGEventRef keyUp = CGEventCreateKeyboardEvent(NULL, keycode, false);
    CGEventPost(kCGHIDEventTap, keyDown);
    CGEventPost(kCGHIDEventTap, keyUp);

    //CGEventSetFlags(keyDown, CGEventGetFlags(keyDown) | kCGEventFlagMaskShift);
    //CGEventSetIntegerValueField(keyDown, kCGKeyboardEventAutorepeat, 1);
    
    CFRelease(keyDown);
    CFRelease(keyUp);
}

int main() {
    int fd = open_port();
    
    if (fd < 0) {
        return 1;
    }

    uint8_t buffer[4096];
    int last_release = 0;

    while(1) {
        int byteCount = read(fd, buffer, 32);
        buffer[byteCount] = '\0';
    //    printf("read %d bytes: ", byteCount);
        for (int i = 0; i<byteCount; i++) {
  //          printf("%#x ", buffer[i]);
            if (buffer[i] == 0xF0) {
                last_release = 1;
                continue;
            }

            if (!last_release) {
                keyPress(scanToKey(buffer[i]));
            }
            last_release = 0;
        }
//        printf("\n");
        
        //keyPress(scanToKey(0x1c));
    };

    return 0;
}
