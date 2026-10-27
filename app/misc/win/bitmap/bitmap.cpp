
#include "bitmap.h"
#include <wincodec.h>
#include "../../fun.hpp"
#include  "../../../app/gl//GlFunctions.h"

Bitmap* BitmapCache::get(int id) {
    const auto it = bitmaps.find(id);
    return it != bitmaps.end() ? &it->second : nullptr;
}

void BitmapCache::put(const Bitmap &bitmap) {
    bitmaps.insert_or_assign(bitmap.id, bitmap);
}


FileContent<Bitmap> BitmapCache::loadBitmap(const fs::path &path) {
     // WIC очікує Unicode-шлях.
     const std::wstring widePath = path.wstring();

     // Створюємо WIC factory.
     ComPtr<IWICImagingFactory> factory;

     HRESULT hr = CoCreateInstance(
         CLSID_WICImagingFactory,
         nullptr,
         CLSCTX_INPROC_SERVER,
         IID_PPV_ARGS(&factory)
     );

     if (FAILED(hr)) {
         return {false,"CoCreateInstance error"};
     }

     // Декодуємо файл.
     ComPtr<IWICBitmapDecoder> decoder;

     hr = factory->CreateDecoderFromFilename(
         widePath.c_str(),
         nullptr,
         GENERIC_READ,
         WICDecodeMetadataCacheOnDemand,
         &decoder
     );

     if (FAILED(hr)) {
         return {false,"CreateDecoderFromFilename error"};
     }

     // Беремо перший кадр (звичайні PNG/JPEG).
     ComPtr<IWICBitmapFrameDecode> frame;

     hr = decoder->GetFrame(0, &frame);

     if (FAILED(hr)) {
         return {false,"first frame decoding error"};
     }

     UINT width = 0;
     UINT height = 0;

     hr = frame->GetSize(&width, &height);

     if (FAILED(hr) || width == 0 || height == 0) {
         return {false,"size getting error"};
     }

     // Перевіряємо розмір буфера, щоб уникнути переповнення.
     const uint64_t bufferSize =
         static_cast<uint64_t>(width) * height * 4;

     if (bufferSize > std::numeric_limits<size_t>::max() ||
         bufferSize > std::numeric_limits<UINT>::max()) {
         return {false,"bufferSize error"};
     }

     // Конвертуємо будь-який підтримуваний формат у RGBA8.
     ComPtr<IWICFormatConverter> converter;

     hr = factory->CreateFormatConverter(&converter);

     if (FAILED(hr)) {
         return {false,"CreateFormatConverter error"};
     }

     hr = converter->Initialize(
         frame.Get(),
         GUID_WICPixelFormat32bppRGBA,
         WICBitmapDitherTypeNone,
         nullptr,
         0.0,
         WICBitmapPaletteTypeCustom
     );

     if (FAILED(hr)) {
         return {false,"converter initialize error"};
     }

     Bitmap bitmap{};
     static int id = 0;
     id++;
     bitmap.id = id; // ID призначить кеш.
     bitmap.width = static_cast<int>(width);
     bitmap.height = static_cast<int>(height);
     bitmap.pixels.resize(bufferSize);

     const UINT stride = width * 4;

     hr = converter->CopyPixels(
         nullptr,
         stride,
         static_cast<UINT>(bufferSize),
         bitmap.pixels.data()
     );

     if (FAILED(hr)) {
         return {false,{}};
     }
     if (GlFunctions::premultiply_alpha_webgl) {
         premultiplyAlphaRGBA(bitmap);
     }
     return {true, "", bitmap};
}

void BitmapCache::premultiplyAlphaRGBA(Bitmap &bitmap) {
    // RGBA: 4 bytes per pixel.
    for (std::size_t i = 0; i + 3 < bitmap.pixels.size(); i += 4)
    {
        const unsigned int alpha = bitmap.pixels[i + 3];

        bitmap.pixels[i]     = static_cast<std::uint8_t>(
            (bitmap.pixels[i   ]  * alpha + 127) / 255
        );
        bitmap.pixels[i + 1] = static_cast<std::uint8_t>(
            (bitmap.pixels[i + 1] * alpha + 127) / 255
        );
        bitmap.pixels[i + 2] = static_cast<std::uint8_t>(
            (bitmap.pixels[i + 2] * alpha + 127) / 255
        );
    }
}
