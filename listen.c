#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h> 
#include <termios.h> 
#include <string.h> 

#include <ApplicationServices/ApplicationServices.h>


//TODO: make this a command line argument?

#define SERIAL_PORT "/dev/tty.usbmodemfa131"

//scancodes to mac keycodes
uint8_t keymap[] = {
    39,39,39,39,39,39,39,39,39,39,
    39,39,39,0x30,0x32,0,0,0x3a,0x38,39,
    0x3b,0xc,0x12,0,0,0,0x6,0x1,0x0,0xd,
    0x13,0,0,0x8,0x7,0x2,0xe,0x15,0x14,0,
    0,0x31,0x9,0x3,0x11,0xf,0x17,0,0,0x2d,
    0xb,0x4,0x5,0x10,0x16,0,0,0,0x2e,0x26,
    0x20,0x1a,0x1c,0,0,0x2b,0x28,0x22,0x1f,0x1d,
    0x19,0,0,0x2f,0x2c,0x25,0x29,0x23,0x1b,0,
    0,0,0x27,0,0x21,0x18,0,0,0x39,0x38,
    0x24,0x1e,0,0x2a,0,0,0,0,0,0,
    0,0,0x33,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0x35,0,
};

uint8_t special_keymap[] = {
/*0*/0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
/*5*/0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
/*10*/0,0,0,0,0,0,0,0x7b,0,0,
    0,0,0,0,0x7d,0,0x7c,0x7e,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
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

CGKeyCode scanToKey(uint8_t scancode, bool special) {
    if (special) {
        return (CGKeyCode) special_keymap[scancode];
    }
    return (CGKeyCode) keymap[scancode];
}

void keyPress(uint8_t keycode) {
    CGEventRef keyDown = CGEventCreateKeyboardEvent(NULL, keycode, true);
    CGEventPost(kCGHIDEventTap, keyDown);
    CFRelease(keyDown);
}

void keyRelease(uint8_t keycode) {
    CGEventRef keyUp = CGEventCreateKeyboardEvent(NULL, keycode, false);
    CGEventPost(kCGHIDEventTap, keyUp);
    CFRelease(keyUp);
}

void keyType(uint8_t keycode) {
    keyPress(keycode);
    keyRelease(keycode);
    
    /* This will be useful later
     *
    CGEventSetFlags(keyDown, CGEventGetFlags(keyDown) | kCGEventFlagMaskShift);
    CGEventSetIntegerValueField(keyDown, kCGKeyboardEventAutorepeat, 1);
    */
}

int main() {
    int fd = open_serial();
    
    if (fd < 0) {
        return 1;
    }

    uint8_t buffer[1024];
    bool last_release = false;
    bool special = false;
    while(1) {
        int byteCount = read(fd, buffer, 32);
        buffer[byteCount] = '\0';
        for (int i = 0; i<byteCount; i++) {
            fprintf(stderr, "%#x\n", buffer[i]);
            if (buffer[i] == 0x0) {
                last_release = false;
                special = false;
                continue;
            }
            if (buffer[i] == 0xE0) {
                special = true;
                continue;
            }
            if (buffer[i] == 0xF0 || buffer[i] == 0x80) {
                last_release = true;
                continue;
            }

            if (!last_release) {
                keyPress(scanToKey(buffer[i], special));
            } else {
                keyRelease(scanToKey(buffer[i], special));
            }
            last_release = false;
            special = false;
        }
    };

    return 0;
}
