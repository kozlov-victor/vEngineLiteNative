
#include "keyboard.h"

#include <Windows.h>
#include <cstdint>
#include <string_view>

std::string_view getKeyboardCode(LPARAM lParam)
{
    const auto bits = static_cast<std::uint32_t>(lParam);
    const auto scanCode = (bits >> 16) & 0xFF;
    const bool extended = (bits & (1u << 24)) != 0;

    const auto key = scanCode | (extended ? 0x100u : 0u);

    switch (key)
    {
        // Основний ряд клавіш
        case 0x029: return "Backquote";
        case 0x002: return "Digit1";
        case 0x003: return "Digit2";
        case 0x004: return "Digit3";
        case 0x005: return "Digit4";
        case 0x006: return "Digit5";
        case 0x007: return "Digit6";
        case 0x008: return "Digit7";
        case 0x009: return "Digit8";
        case 0x00A: return "Digit9";
        case 0x00B: return "Digit0";
        case 0x00C: return "Minus";
        case 0x00D: return "Equal";
        case 0x00E: return "Backspace";

        // Літери
        case 0x01E: return "KeyA";
        case 0x030: return "KeyB";
        case 0x02E: return "KeyC";
        case 0x020: return "KeyD";
        case 0x012: return "KeyE";
        case 0x021: return "KeyF";
        case 0x022: return "KeyG";
        case 0x023: return "KeyH";
        case 0x017: return "KeyI";
        case 0x024: return "KeyJ";
        case 0x025: return "KeyK";
        case 0x026: return "KeyL";
        case 0x032: return "KeyM";
        case 0x031: return "KeyN";
        case 0x018: return "KeyO";
        case 0x019: return "KeyP";
        case 0x010: return "KeyQ";
        case 0x013: return "KeyR";
        case 0x01F: return "KeyS";
        case 0x014: return "KeyT";
        case 0x016: return "KeyU";
        case 0x02F: return "KeyV";
        case 0x011: return "KeyW";
        case 0x02D: return "KeyX";
        case 0x015: return "KeyY";
        case 0x02C: return "KeyZ";

        // Пробіл та службові клавіші
        case 0x039: return "Space";
        case 0x01C: return "Enter";
        case 0x11C: return "NumpadEnter";
        case 0x00F: return "Tab";
        case 0x001: return "Escape";
        case 0x00E + 0x100: return "Unidentified"; // не використовувати для Backspace

        // Модифікатори
        case 0x02A: return "ShiftLeft";
        case 0x036: return "ShiftRight";
        case 0x01D: return "ControlLeft";
        case 0x11D: return "ControlRight";
        case 0x038: return "AltLeft";
        case 0x138: return "AltRight";

        // Стрілки
        case 0x14B: return "ArrowLeft";
        case 0x14D: return "ArrowRight";
        case 0x148: return "ArrowUp";
        case 0x150: return "ArrowDown";

        // Цифровий блок
        case 0x052: return "Numpad0";
        case 0x04F: return "Numpad1";
        case 0x050: return "Numpad2";
        case 0x051: return "Numpad3";
        case 0x04B: return "Numpad4";
        case 0x04C: return "Numpad5";
        case 0x04D: return "Numpad6";
        case 0x047: return "Numpad7";
        case 0x048: return "Numpad8";
        case 0x049: return "Numpad9";
        case 0x053: return "NumpadDecimal";
        case 0x135: return "NumpadDivide";
        case 0x037: return "NumpadMultiply";
        case 0x04A: return "NumpadSubtract";
        case 0x04E: return "NumpadAdd";

        // Навігаційні клавіші
        case 0x147: return "Home";
        case 0x14F: return "End";
        case 0x149: return "PageUp";
        case 0x151: return "PageDown";
        case 0x152: return "Insert";
        case 0x153: return "Delete";

        // Windows та контекстне меню
        case 0x15B: return "MetaLeft";
        case 0x15C: return "MetaRight";
        case 0x15D: return "ContextMenu";
    }

    // F1-F10: scan code 0x3B-0x44
    if (scanCode >= 0x3B && scanCode <= 0x44 && !extended)
    {
        static constexpr std::string_view functionKeys[] = {
            "F1", "F2", "F3", "F4", "F5",
            "F6", "F7", "F8", "F9", "F10"
        };
        return functionKeys[scanCode - 0x3B];
    }

    // F11 та F12
    if (!extended && scanCode == 0x57) return "F11";
    if (!extended && scanCode == 0x58) return "F12";

    return "undefined";
}