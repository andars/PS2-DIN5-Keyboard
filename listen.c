#include <unistd.h>
#include <stdint.h>
#include <fcntl.h> 
#include <termios.h> 
#include <string.h> 

#include <ApplicationServices/ApplicationServices.h>


//TODO: make this a command line argument?

#define SERIAL_PORT "/dev/tty.usbmodemfa131"

//scancodes to mac keycodes
uint8_t keymap[] = {
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0xc,0x12,0,0,0,0x6,0x1,0x0,0xd,
    0x13,0,0,0x8,0x7,0x2,0xe,0x15,0x14,0,
    0,0x31,0x9,0x3,0x11,0xf,0x17,0,0,0x2d,
    0xb,0x4,0x5,0x10,0x16,0,0,0,0x2e,0x26,
    0x20,0x1a,0x1c,0,0,0,0x28,0x22,0x1f,0x1d,
    0x19,0,0,0,0,0x25,0,0x23,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0x24,0,0,0,0,0,0,0,0,0,
    0,0,0x33,0,0,0,0,0,0,0,
};

int open_serial() {
    int fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_NDELAY);      

    if (fd == -1) {
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
    options.c_cflag |= (IXON | IXOFF | IXANY);
    
    return fd;
 }

CGKeyCode scanToKey(uint8_t scancode) {
    return (CGKeyCode) keymap[scancode];
}
void keyPress(uint8_t keycode) {
    CGEventRef keyDown = CGEventCreateKeyboardEvent(NULL, keycode, true);
    CGEventRef keyUp = CGEventCreateKeyboardEvent(NULL, keycode, false);
    CGEventPost(kCGHIDEventTap, keyDown);
    CGEventPost(kCGHIDEventTap, keyUp);
    
    /* This will be useful later
     *
    CGEventSetFlags(keyDown, CGEventGetFlags(keyDown) | kCGEventFlagMaskShift);
    CGEventSetIntegerValueField(keyDown, kCGKeyboardEventAutorepeat, 1);
    */

    CFRelease(keyDown);
    CFRelease(keyUp);
}

int main() {
    int fd = open_serial();
    
    if (fd < 0) {
        return 1;
    }

    uint8_t buffer[1024];
    int last_release = 0;

    while(1) {
        int byteCount = read(fd, buffer, 32);
        buffer[byteCount] = '\0';
        for (int i = 0; i<byteCount; i++) {
            if (buffer[i] == 0xE0 && buffer[i+1] == 0xF0) {
                last_release = 1;
                i++;
                continue;
            }
            if (buffer[i] == 0xF0) { //this probably won't work
                last_release = 1;
                continue;
            }

            if (!last_release) {
                keyPress(scanToKey(buffer[i]));
            }
            last_release = 0;
        }
    };

    return 0;
}
