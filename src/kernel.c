// by using this you admit to not being a bum which includes:

// not being homophobic
// not being transphobic
// not being racist
// not being sexist
// not being anti-furry (grow up its 2026 not 2022)
// not being a vibe coder (excludes ai for help or tiny code)
// not being xenophobic

// =====================================================================
// Includes
// =====================================================================

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <limine.h>
#include "gdt.h"
#include <flanterm.h>
#include <flanterm_backends/fb.h>
#include "interrupts.h"

//exceptional sturcture because flanterm is a needy bastard
struct flanterm_context *terminal;

// =====================================================================
// Macros / Defines
// =====================================================================

#define LMRQ   __attribute__((used, section(".limine_requests")))
#define LMRQ_S __attribute__((used, section(".limine_requests_start")))
#define LMRQ_E __attribute__((used, section(".limine_requests_end")))
#define STVL   static volatile

// =====================================================================
// Limine bootloader requests
// =====================================================================

LMRQ_S
STVL uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

LMRQ
STVL uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

LMRQ
STVL struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

LMRQ_E
STVL uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// =====================================================================
// Standard library-like helpers
// =====================================================================

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = dest;
    const uint8_t *restrict psrc = src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = dest;
    const uint8_t *psrc = src;

    if ((uintptr_t)src > (uintptr_t)dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if ((uintptr_t)src < (uintptr_t)dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = s1;
    const uint8_t *p2 = s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// =====================================================================
// Kernel utility functions
// =====================================================================

void hcf(void) {
    __asm__ ("cli");
    for (;;) {
        __asm__ ("hlt");
    }
}


int getlenght(char* str) {
    int i = 0;
    while (str[i] != '\0') {
        i++;
    }
    return i;
}

void strcomb(char a[], char b[]) {
    int combinedlenght = getlenght(a) + getlenght(b);
    char temp[combinedlenght + 1];
    int ia = getlenght(a);
    int ib = getlenght(b);
    for (int i = 0; i < ia; i++) {
        temp[i] = a[i];
    }
    for (int i = 0; i < ib; i++) {
        temp[ia + i] = b[i];
    }
    temp[ia + ib] = '\0';
    memcpy(a, temp, combinedlenght + 1);
}

void itoa(unsigned int num, char* str) {
    int i = 0;
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    while (num > 0) {
        str[i] = (num % 10) + '0';
        num = num / 10;
        i++;
    }
    str[i] = '\0';
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void u64_to_hex(uint64_t value, char *text) {
    const char hex[] = "0123456789ABCDEF";

    text[0] = '0';
    text[1] = 'x';

    for (int i = 0; i < 16; i++) {
        text[i + 2] = hex[(value >> (60 - i * 4)) & 0xF];
    }

    text[18] = '\0';
}

void sleep(uint64_t cycles) {
    for (volatile uint64_t i = 0; i < cycles; i++) {
        __asm__ volatile("pause");
    }
}

// =====================================================================
// Framebuffer drawing
// =====================================================================

void putpx(uint32_t x, uint32_t y, uint32_t r, uint32_t g, uint32_t b) {
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    uint32_t *fb = framebuffer->address;
    uint32_t pitch = framebuffer->pitch / sizeof(uint32_t);
    fb[y * pitch + x] = (r << 16) | (g << 8) | b;
}

void rect(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t r, uint32_t g, uint32_t b) {
    for (uint32_t i = y1; i < y2; i++) {
        for (uint32_t j = x1; j < x2; j++) {
            putpx(j, i, r, g, b);
        }
    }
}

// =====================================================================
// Terminal
// =====================================================================

bool terminalavailable = false;

void nwrite(char* text, int color, bool light) {
    if (terminalavailable) {
        char formattedtext[256] = "\033[";
        char colorcode[4];

        if (light) {
            colorcode[0] = '9';
            if (color == 7) {
                colorcode[1] = '0';
            } else if (color == 1) {
                colorcode[1] = '1';
            } else if (color == 3) {
                colorcode[1] = '2';
            } else if (color == 2) {
                colorcode[1] = '3';
            } else if (color == 5) {
                colorcode[1] = '4';
            } else if (color == 6) {
                colorcode[1] = '5';
            } else if (color == 4) {
                colorcode[1] = '6';
            } else {
                colorcode[1] = '7';
            }
        } else {
            colorcode[0] = '3';
            if (color == 7) {
                colorcode[1] = '0';
            } else if (color == 1) {
                colorcode[1] = '1';
            } else if (color == 3) {
                colorcode[1] = '2';
            } else if (color == 2) {
                colorcode[1] = '3';
            } else if (color == 5) {
                colorcode[1] = '4';
            } else if (color == 6) {
                colorcode[1] = '5';
            } else if (color == 4) {
                colorcode[1] = '6';
            } else {
                colorcode[1] = '7';
            }
        }
        colorcode[2] = 'm';
        colorcode[3] = '\0';

        strcomb(formattedtext, colorcode);
        strcomb(formattedtext, text);
        strcomb(formattedtext, "\033[0m\r\n");

        flanterm_write(terminal, formattedtext, getlenght(formattedtext));
    }
}

void write(char* text, int color, bool light) {
    if (terminalavailable) {
        char formattedtext[256] = "\033[";
        char colorcode[4];

        if (light) {
            colorcode[0] = '9';
            if (color == 7) {
                colorcode[1] = '0';
            } else if (color == 1) {
                colorcode[1] = '1';
            } else if (color == 3) {
                colorcode[1] = '2';
            } else if (color == 2) {
                colorcode[1] = '3';
            } else if (color == 5) {
                colorcode[1] = '4';
            } else if (color == 6) {
                colorcode[1] = '5';
            } else if (color == 4) {
                colorcode[1] = '6';
            } else {
                colorcode[1] = '7';
            }
        } else {
            colorcode[0] = '3';
            if (color == 7) {
                colorcode[1] = '0';
            } else if (color == 1) {
                colorcode[1] = '1';
            } else if (color == 3) {
                colorcode[1] = '2';
            } else if (color == 2) {
                colorcode[1] = '3';
            } else if (color == 5) {
                colorcode[1] = '4';
            } else if (color == 6) {
                colorcode[1] = '5';
            } else if (color == 4) {
                colorcode[1] = '6';
            } else {
                colorcode[1] = '7';
            }
        }
        colorcode[2] = 'm';
        colorcode[3] = '\0';

        strcomb(formattedtext, colorcode);
        strcomb(formattedtext, text);
        strcomb(formattedtext, "\033[0m");

        flanterm_write(terminal, formattedtext, getlenght(formattedtext));
    }
}

//so far color pallete!

// white - Informative
// red - Fatal Exception
// yellow - Warning, Light Exception, Error code
// green - Loaded, Kernel mention
// cyan - Interrupt
// blue - Panic
// purple - empty
// grey - empty
// light red - empty
// light yellow - empty
// light green - empty
// light cyan - empty
// light blue - empty
// light purple - empty
 
// =====================================================================
// Others i guess
// =====================================================================

void killcores(void) {
    
}

void panic(int vector, int cr2, int errorcode) {
    write("\033[2J\033[H", 7, true);

    for (int i = 0; i < 160; i++) {
        write("█", 1, true);
    }
    nwrite("", 1, true);

    write("█", 1, true);
    write("█", 1, true);
    for (int j = 0; j < 156; j++) {
        write(" ", 1, false);
    }
    write("█", 1, true);
    nwrite("█", 1, true);

    write("█", 1, true);
    write("█", 1, true);

    write("  K   K  EEEEE  RRRR   N   N  EEEEE  L        PPPP    AAA   N   N  IIIII   CCCC", 1, true);

    for (int j = 0; j < 77; j++) {
        write(" ", 1, false);
    }
    
    write("█", 1, true);
    nwrite("█", 1, true);

    write("█", 1, true);
    write("█", 1, true);

    write("  K  K   E      R   R  NN  N  E      L        P   P  A   A  NN  N    I    C    ", 1, true);

    for (int j = 0; j < 77; j++) {
        write(" ", 1, false);
    }

    write("█", 1, true);
    nwrite("█", 1, true);

    write("█", 1, true);
    write("█", 1, true);

    write("  KKK    EEEEE  RRRR   N N N  EEEEE  L        PPPP   AAAAA  N N N    I    C    ", 1, true);

    for (int j = 0; j < 77; j++) {
        write(" ", 1, false);
    }

    write("█", 1, true);
    nwrite("█", 1, true);

    write("█", 1, true);
    write("█", 1, true);

    write("  K K    E      R  R   N  NN  E      L        P      A   A  N  NN    I    C    ", 1, true);

    for (int j = 0; j < 77; j++) {
        write(" ", 1, false);
    }

    write("█", 1, true);
    nwrite("█", 1, true);

    write("█", 1, true);
    write("█", 1, true);

    write("  K   K  EEEEE  R   R  N   N  EEEEE  LLLLL    P      A   A  N   N  IIIII   CCCC", 1, true);

    for (int j = 0; j < 77; j++) {
        write(" ", 1, false);
    }

    write("█", 1, true);
    nwrite("█", 1, true);

    write("█", 1, true);
    write("█", 1, true);
    for (int j = 0; j < 156; j++) {
        write(" ", 1, false);
    }
    write("█", 1, true);
    nwrite("█", 1, true);

    for (int j = 0; j < 160; j++) {
        write("█", 1, true);
    }
    nwrite("", 0, false);

    write("█", 1, true);
    write("█", 1, true);
    for (int j = 0; j < 156; j++) {
        write(" ", 1, false);
    }
    write("█", 1, true);
    nwrite("█", 1, true);

    write("█", 1, true);
    write("█", 1, true);

    write("  THE KERNEL RAN INTO AN ERROR THAT EITHER COULD NOT BE RECOVERED FROM, OR REQUIRED A NECESSARY STOP OF THE SYSTEM", 1, true);
    
    for (int j = 0; j < 42; j++) {
        write(" ", 1, false);
    }
    write("█", 1, true);
    nwrite("█", 1, true);

    write("█", 1, true);
    write("█", 1, true);
    for (int j = 0; j < 156; j++) {
        write(" ", 1, false);
    }
    write("█", 1, true);
    write("█", 1, true);
    nwrite("", 3, true);

    write("█", 1, true);
    write("█", 1, true);
    write("  - WHAT DO I DO?", 1, true);
    for (int j = 0; j < 139; j++) {
        write(" ", 1, false);
    }
    write("█", 1, true);
    write("█", 1, true);
    nwrite("", 3, true);
    
    write("█", 1, true);
    write("█", 1, true);
    write("    - - O P T I O N S - -", 1, true);
    for (int j = 0; j < 131; j++) {
        write(" ", 1, false);
    }
    write("█", 1, true);
    write("█", 1, true);
    nwrite("", 3, true);

    write("█", 1, true);
    write("█", 1, true);
    write("    1) GO TO THE OFFICIAL WEBSITE AND FIND A SOLUTION TO THE ERROR", 1, true);
    for (int j = 0; j < 90; j++) {
        write(" ", 1, false);
    }
    write("█", 1, true);
    write("█", 1, true);
    nwrite("", 3, true);

    write("█", 1, true);
    write("█", 1, true);
    write("    2) FLASH A NEW PHOSPHOROS IMAGE", 1, true);
    for (int j = 0; j < 121; j++) {
        write(" ", 1, false);
    }
    write("█", 1, true);
    write("█", 1, true);
    nwrite("", 3, true);

    write("█", 1, true);
    write("█", 1, true);
    write("    3) REBOOT THE SYSTEM IF THIS IS A ONE TIME BUG", 1, true);
    for (int j = 0; j < 106; j++) {
        write(" ", 1, false);
    }
    write("█", 1, true);
    write("█", 1, true);
    nwrite("", 3, true);

    
    for (int i = 0; i < 20; i++) {
        write("█", 1, true);
        write("█", 1, true);
        for (int j = 0; j < 156; j++) {
            write(" ", 1, false);
        }
        write("█", 1, true);
        write("█", 1, true);
        nwrite("", 3, true);
    }

    write("█", 1, true);
    write("█", 1, true);

    switch (vector) {
        case 8:
            write("  ERROR_HANDLE_FAIL (#DF)");
        case 10:
            
        case 13:
            write("  GENERAL_PROTECTION_FAULT");
        case 14:
            write("  PAGE_FAULT");
        case 18:
            write("  HARDWARE_RETURN_LOSS (#MC)")
    }
    write("█", 1, true);
    nwrite("█", 1, true);
    
    for (int i = 0; i < 160; i++) {
        write("█", 1, true);
    }
}

// =====================================================================
// Kernel entry point
// =====================================================================

void _start(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    init_gdt();

    if (framebuffer_request.response == NULL
         || framebuffer_request.response->framebuffer_count < 1) {
            hcf();
        }

        struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

        terminal = flanterm_fb_init(
            NULL, NULL,
            (uint32_t *)fb->address,
            fb->width, fb->height, fb->pitch,
            fb->red_mask_size, fb->red_mask_shift,
            fb->green_mask_size, fb->green_mask_shift,
            fb->blue_mask_size, fb->blue_mask_shift,
            NULL, NULL, NULL, NULL, NULL, NULL, NULL,
            NULL, 0, 0, 0, 0, 0, 0,
            FLANTERM_FB_ROTATE_0
        );

        if (terminal == NULL) {
            terminalavailable = false;
        } else {
            terminalavailable = true;
        }
    
    write("Greetings from the ", 8, true);
    write("K15² ", 3, true);
    nwrite(" Kernel!", 8, true);
    nwrite(" ", 8, true);
    
    InitiateIDT();

    panic(0, 0, 1);
    
    hcf();

}