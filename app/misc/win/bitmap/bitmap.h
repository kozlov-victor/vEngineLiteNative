#ifndef VENGINE_BITMAP_H
#define VENGINE_BITMAP_H

#include <vector>
#include <unordered_map>
#include <wrl/client.h>
#include <filesystem>
#include "../../fun.hpp"

using Microsoft::WRL::ComPtr;
namespace fs = std::filesystem;

struct Bitmap {
    int id;
    std::vector<uint8_t> pixels;
    int width;
    int height;
};

class BitmapCache {
public:
    Bitmap* get(int id);
    void put(const Bitmap& bitmap);
    static FileContent<Bitmap> loadBitmap(const fs::path& path);
private:
    std::unordered_map<int, Bitmap> bitmaps;
};

#endif