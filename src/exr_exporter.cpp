#include <zlib.h> // CRITICAL: Exposes uLong, Bytef, and Z_OK to tinyexr
#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"
#include "exr_exporter.h"
#include <vector>
#include <string>
#include <iostream>
#include <cstring>

bool ExportSimulationToEXR(const std::string& filename, const float* gridData, int width, int height) {
    EXRHeader header;
    InitEXRHeader(&header);

    EXRImage image;
    InitEXRImage(&image);

    // 1. Configure the image dimensions
    image.num_channels = 1;
    image.width = width;
    image.height = height;

    // 2. Set up the data channel (Using 'Y' for luminance/single-channel raw data)
    std::vector<const float*> images(1);
    images[0] = gridData; 
    image.images = (unsigned char**)images.data();

    // 3. Define the channel metadata
    std::vector<EXRChannelInfo> channels(1);
    std::strncpy(channels[0].name, "Y", 255); 
    channels[0].name[255] = '\0';
    
    header.num_channels = 1;
    header.channels = channels.data();

    // 4. Set pixel data types to 32-bit Full Float
    std::vector<int> pixel_types(1, TINYEXR_PIXELTYPE_FLOAT);
    std::vector<int> requested_pixel_types(1, TINYEXR_PIXELTYPE_FLOAT);
    
    header.pixel_types = pixel_types.data();
    header.requested_pixel_types = requested_pixel_types.data();

    // 5. Specify Compression Type (ZIP compression via native system zlib)
    header.compression_type = TINYEXR_COMPRESSIONTYPE_ZIP;

    // 6. Save data to disk
    const char* err = nullptr;
    int ret = SaveEXRImageToFile(&image, &header, filename.c_str(), &err);
    
    if (ret != TINYEXR_SUCCESS) {
        std::cerr << "[EXR Export Error] Failed to save EXR file: " << (err ? err : "Unknown Error") << std::endl;
        if (err) {
            FreeEXRErrorMessage(err); // Free error message allocated by tinyexr
        }
        return false;
    }

    return true;
}
