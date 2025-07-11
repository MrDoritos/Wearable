#define STB_IMAGE_IMPLEMENTATION

#include "third_party/stb_image.h"
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <errno.h>
#include <iterator>

typedef unsigned char color;

struct Image {
    Image(int valuebits, int alphabits)
    :valuebits(valuebits),alphabits(alphabits) {}

    ~Image() {
        if (raw)
            stbi_image_free(raw);
        if (converted)
            delete [] converted;
    }

    int valuebits, alphabits;
    int width = 0;
    int height = 0;
    int bpp = 0;
    
    color *raw = nullptr;
    color *converted = nullptr;

    int pixel_count() {
        return width * height;
    }

    int total_size() {
        return width * height * bpp;
    }

    int converted_size() {
        return (pixel_count() / (sizeof(color) * 8)) * (valuebits + alphabits);
    }

    int converted_px_per_byte() {
        return 8 / (valuebits + alphabits);
    }

    int convert_value() {
        if (!width || !height || !bpp || !raw)
            return 0;

        const int count = pixel_count();
        const int total = total_size();

        const int destcount = converted_size();

        const int colorbits = bpp - 1;

        const int valuediv = (colorbits * int(color(-1))) / ((1 << valuebits)-1);
        const int alphadiv = (alphabits * int(color(-1))) / ((1 << alphabits)-1);
        const int destbits = valuebits + alphabits;

        const int px_per_byte = converted_px_per_byte();

        converted = new color[destcount];

        if (!converted) {
            int err = errno;
            printf("Failed to allocate %i bytes. %i %s\n", destcount, err, strerror(err));
            return 0;
        }

        memset(converted, 0, destcount);

        for (int i = 0, k = 0; i < total; i+=bpp, k++) {
            int sum = 0;
            for (int j = 0; j < colorbits; j++)
                sum += raw[i+j];
            int value = sum / valuediv;
            int alpha = 255;
            if (bpp > colorbits)
                alpha = raw[i+(bpp-1)];
            alpha /= alphadiv;
            const int converted_offset = k / px_per_byte;
            const int converted_bitshift = (k % px_per_byte) * destbits;
            const int converted_px = (value << valuebits) | alpha;
            const int bitmask = (1 << destbits) - 1;
            const int destpx = (converted_px & bitmask) << converted_bitshift;
            converted[converted_offset] |= destpx;
        }

        return 1;
    }

    int load(const char *filename) {
        raw = stbi_load(filename, &width, &height, &bpp, 4);

        if (!raw)
            return 0;

        return 1;
    }

    int save(const std::string &filename) {
        std::string save_name = filename;
        int buflen = 256;
        char buf[buflen];
        const auto it = std::find_if(filename.crbegin(), filename.crend(), [](auto c){return c=='/'||c=='\\';});
        if (it != filename.crend()) {
            auto fit = it.base();
            save_name.clear();
            std::copy(fit, filename.cend(), std::back_inserter(save_name));
        }
        snprintf(buf, buflen, "%s.%ibv%iba%iw%ih.bin", save_name.c_str(), valuebits, alphabits, width, height);
        printf("Saving %i bytes, %s -> %s\n", converted_size(), filename.c_str(), buf);

        FILE *fp = fopen(buf, "w+");

        if (!fp) {
            int err = errno;
    
            printf("Failed to create or open. %i %s\n", err, strerror(err));
    
            return 0;
        }

        int count = fwrite(converted, 1, converted_size(), fp);

        if (count < 0) {
            int err = errno;

            printf("Failed to write data. %i %s\n", err, strerror(err));

            return 0;
        }

        printf("Saved %i bytes\n", count);

        return 1;        
    }
};

int main(int argc, char ** argv) {
    if (argc < 4) {
        puts("Provide (value bits) (alpha bits) (image filepath) [image filepaths...]");
        exit(1);
    }

    int value_bits, alpha_bits;
    if (sscanf(argv[1], "%i", &value_bits) != 1) {
        puts("Argument value bits");
        exit(1);
    }

    if (sscanf(argv[2], "%i", &alpha_bits) != 1) {
        puts("Argument alpha bits");
        exit(1);
    }

    printf("value bits: %i, alpha bits: %i\n", value_bits, alpha_bits);

    for (int i = 3; i < argc; i++) {
        const char *filename = argv[i];
        Image image(value_bits, alpha_bits);
        if (!image.load(filename)) {
            printf("Failed to load %s\n", filename);
            continue;
        }
        printf("Loaded image %s\n", filename);
        printf("width: %i, height: %i, bpp: %i\n", image.width, image.height, image.bpp);
        if (!image.convert_value()) {
            printf("Failed to convert %s\n", filename);
            continue;
        }
        
        if (!image.save(filename)) {
            printf("Failed to save %s\n", filename);
            continue;
        }
    }
}