#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#pragma pack(push, 1)

typedef struct {
    uint16_t type;             
    uint32_t size;             
    uint16_t reserved1;        
    uint16_t reserved2;        
    uint32_t offset;           
} BMPFileHeader;

typedef struct {
    uint32_t size;             
    int32_t  width;            
    int32_t  height;           
    uint16_t planes;           
    uint16_t bits_per_pixel;   
    uint32_t compression;      
    uint32_t image_size;       
    int32_t  x_pixels_per_m;   
    int32_t  y_pixels_per_m;   
    uint32_t colors_used;      
    uint32_t colors_important; 
} BMPInfoHeader;

#pragma pack(pop)


void filter_grayscale(int width, int height, int stride, int bytesPerPixel, uint8_t *pixels) {
    for (int row = 0; row < height; row++){
        for (int col = 0; col < width; col++){
            int byte_idx = (row * stride) + (col * bytesPerPixel);

            uint8_t blue = pixels[byte_idx + 0];
            uint8_t green = pixels[byte_idx + 1];
            uint8_t red = pixels[byte_idx + 2];
            
            int avg = round((blue + green + red) / 3.0);
            pixels[byte_idx + 0] = avg;
            pixels[byte_idx + 1] = avg;
            pixels[byte_idx + 2] = avg;
        }
    }

}

void filter_invert(int width, int height, int stride, int bytesPerPixel, uint8_t *pixels) {
    for (int row = 0; row < height; row++){
        for (int col = 0; col < width; col++){
            int byte_idx = (row * stride) + (col * bytesPerPixel);
            pixels[byte_idx + 0] = 255 - pixels[byte_idx + 0];
            pixels[byte_idx + 1] = 255 - pixels[byte_idx + 1];
            pixels[byte_idx + 2] = 255 - pixels[byte_idx + 2];
        }
    }
}

void filter_reflect(int width, int height, int stride, int bytesPerPixel, uint8_t *pixels) {
        for (int row = 0; row < height; row++){
            for (int col = 0; col < width / 2; col++){
                int left_idx = (row * stride) + (col * bytesPerPixel);
                int right_idx = (row * stride) + ((width - 1 - col) * bytesPerPixel);

                uint8_t temp_blue = pixels[left_idx + 0];
                pixels[left_idx + 0] = pixels[right_idx + 0];
                pixels[right_idx + 0] = temp_blue;

                uint8_t temp_green = pixels[left_idx + 1];
                pixels[left_idx + 1] = pixels[right_idx + 1];
                pixels[right_idx + 1] = temp_green;

                uint8_t temp_red = pixels[left_idx + 2];
                pixels[left_idx + 2] = pixels[right_idx + 2];
                pixels[right_idx + 2] = temp_red;
            }
        }                   
}



float edge_kernel[3][3] = {
    {-1, -1, -1}, // Top row weights
    {-1,  8, -1}, // Middle row weights (center is 8)
    {-1, -1, -1}  // Bottom row weights
};
//

void apply_kernel(int width, int height, int stride, int bytesPerPixel, uint8_t *pixels, float kernel[3][3]) {
    uint8_t *temp = malloc(height * stride);
    
    for (int row = 0; row < height; row++){
        for (int col = 0; col < width; col++){
            float sum_blue = 0, sum_green = 0, sum_red = 0;
            
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int neighbor_row = row + dy;
                    int neighbor_col = col + dx;
                    
                    if (neighbor_row < 0 || neighbor_row >= height || 
                        neighbor_col < 0 || neighbor_col >= width) {
                        continue; 
                    }
                    
                    int neighbor_index = (neighbor_row * stride) + (neighbor_col * bytesPerPixel);
                    float weight = kernel[dy + 1][dx + 1];

                    sum_blue += pixels[neighbor_index + 0] * weight;
                    sum_green += pixels[neighbor_index + 1] * weight;
                    sum_red += pixels[neighbor_index + 2] * weight;
                }
            }
            
            int final_b = sum_blue > 255 ? 255 : (sum_blue < 0 ? 0 : round(sum_blue));
            int final_g = sum_green > 255 ? 255 : (sum_green < 0 ? 0 : round(sum_green));
            int final_r = sum_red > 255 ? 255 : (sum_red < 0 ? 0 : round(sum_red));

            int current_index = (row * stride) + (col * bytesPerPixel);

            temp[current_index + 0] = final_b;
            temp[current_index + 1] = final_g;
            temp[current_index + 2] = final_r;
        }
    }
    
    memcpy(pixels, temp, height * stride);
    free(temp);
}


int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <path_to_image.bmp> <path_to_output_image.bmp> \n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    if (fread(&fileHeader, sizeof(BMPFileHeader), 1, file) != 1) {
        printf("Error reading file header.\n");
        fclose(file);
        return 1;
    }

    if (fileHeader.type != 0x4D42) { // "BM"
        printf("Error: Not a valid BMP file.\n");
        fclose(file);
        return 1;
    }

    if (fread(&infoHeader, sizeof(BMPInfoHeader), 1, file) != 1) {
        printf("Error reading info header.\n");
        fclose(file);
        return 1;
    }

    FILE *out_file = fopen(argv[2], "wb");
    if (!out_file) {
        perror("Error opening output file");
        fclose(file);
        return 1;
    }

    if (fwrite(&fileHeader, sizeof(BMPFileHeader), 1, out_file) != 1) {
        printf("Error writing file header.\n");
        fclose(out_file);
        fclose(file);
        return 1;
    }

    if (fwrite(&infoHeader, sizeof(BMPInfoHeader), 1, out_file) != 1) {
        printf("Error writing info header.\n");
        fclose(out_file);
        fclose(file);
        return 1;
    }

    // Move pointers to start of pixel data
    fseek(file, fileHeader.offset, SEEK_SET);
    fseek(out_file, fileHeader.offset, SEEK_SET);

    int bytesPerPixel = infoHeader.bits_per_pixel / 8;
    int rowSize = infoHeader.width * bytesPerPixel;
    int padding = (4 - (rowSize % 4)) % 4;
    int stride = rowSize + padding;
    int rows = abs(infoHeader.height); 
    int total_bytes = rows * stride;

    uint8_t *pixels = (uint8_t *)malloc(total_bytes);
    if (!pixels) {
        printf("Memory allocation failed for pixel copy.\n");
        fclose(out_file);
        fclose(file);
        return 1;
    }

    size_t bytes_read = fread(pixels, 1, total_bytes, file);
    if (bytes_read != total_bytes) {
        printf("Error: total bytes are not right\n");
        free(pixels);
        fclose(out_file);
        fclose(file);
        return 1;
    }

    
    
    // -- Edge Detection Kernel --
    float edge_kernel[3][3] = { {-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1} };

    
    // -- Blur Kernel (averages out the pixels) --
    float blur_kernel[3][3] = { 
        {1.0/9.0, 1.0/9.0, 1.0/9.0}, 
        {1.0/9.0, 1.0/9.0, 1.0/9.0}, 
        {1.0/9.0, 1.0/9.0, 1.0/9.0} 
    };

    // -- Sharpen Kernel --
    float sharpen_kernel[3][3] = { 
        { 0, -1,  0}, 
        {-1,  5, -1}, 
        { 0, -1,  0} 
    };

    // -- Emboss Kernel (gives a 3D shadow effect) --
    float emboss_kernel[3][3] = { 
        {-2, -1,  0}, 
        {-1,  1,  1}, 
        { 0,  1,  2} 
    };

    if (strcmp(argv[3], "blur") == 0) {
        for (int i = 0; i < 15; i++) {
        apply_kernel(infoHeader.width, rows, stride, bytesPerPixel, pixels, blur_kernel);
        }
    }
    
    if (strcmp(argv[3], "grayscale") == 0){
        filter_grayscale(infoHeader.width, rows, stride, bytesPerPixel, pixels);
    }

    if (strcmp(argv[3], "invert") == 0){
        filter_invert(infoHeader.width, rows, stride, bytesPerPixel, pixels);
    }

    if (strcmp(argv[3], "reflect") == 0){
            filter_reflect(infoHeader.width, rows, stride, bytesPerPixel, pixels);
    }

    if(strcmp(argv[3], "edge") == 0){
        apply_kernel(infoHeader.width, rows, stride, bytesPerPixel, pixels, edge_kernel);
    }

    if(strcmp(argv[3], "sharp") == 0){
        apply_kernel(infoHeader.width, rows, stride, bytesPerPixel, pixels, sharpen_kernel);
    }


    // Write modified pixels to output
    fseek(out_file, fileHeader.offset, SEEK_SET);
    size_t written = fwrite(pixels, 1, total_bytes, out_file);
    if (written != total_bytes) { 
        printf("Error writing pixels\n"); 
    }

    // Cleanup
    free(pixels);
    fclose(out_file);
    fclose(file);

    return 0;
}