#pragma once
#include "GEPredefinedMacros.h"

namespace GE
{
    constexpr int kNumpadOffset = 0x1000;

    // Message used in Engine.
    // Generally, only the Engine will generates virtual messages.
    struct GameEngineAPI Message
    {
        enum class GameEngineAPI EEvent
        {
            Unknown,                // We dont care.
            LostFocus,              // Switch to other window.
            MouseMove,
            MouseButtonDown,
            MouseButtonUp,
            MouseButtonDoubleClick,
            KeyDown,
            KeyUp,
        };

        enum class GameEngineAPI EEventSource
        {
            Mouse,
            Keyboard,
            None
        };

        enum class GameEngineAPI EMouseButton : int
        {
            Left = 0,
            Middle = 1,
            Right = 2,
            None = 3
        };

        enum class GameEngineAPI EKeyCode : int
        {
            Invalid = 0,

            // Function keys.
            Escape,
            Enter = '\n',
            Space = ' ',
            Tab = '\t',
            Control,
            Shift,
            Alt,
            Capital,
            Backspace,
            Pause,
            PageUp, PageDown, Home, End, Insert, Delete,
            ArrowLeft, ArrowUp, ArrowRight, ArrowDown,

            // F area.
            F1 = 0x1101, F2 = 0x1102, F3 = 0x1103,
            F4 = 0x1104, F5 = 0x1105, F6 = 0x1106,
            F7 = 0x1107, F8 = 0x1108, F9 = 0x1109,
            F10 = 0x110A, F11 = 0x110B, F12 = 0x110C,

            // Numbers, equals to '0' to '9'.
            Num0 = '0', Num1 = '1', Num2 = '2', Num3 = '3', Num4 = '4',
            Num5 = '5', Num6 = '6', Num7 = '7', Num8 = '8', Num9 = '9',

            // Numpad area, equals to 0x100+'0' to 0x100+'9'.
            Numpad0 = '0' + kNumpadOffset, Numpad1 = '1' + kNumpadOffset, Numpad2 = '2' + kNumpadOffset,
            Numpad3 = '3' + kNumpadOffset, Numpad4 = '4' + kNumpadOffset, Numpad5 = '5' + kNumpadOffset,
            Numpad6 = '6' + kNumpadOffset, Numpad7 = '7' + kNumpadOffset, Numpad8 = '8' + kNumpadOffset,
            Numpad9 = '9' + kNumpadOffset, NumpadLock,
            NumpadDecimal = '.' + kNumpadOffset, NumpadEnter = '\n' + kNumpadOffset,
            NumpadAdd = '+' + kNumpadOffset, NumpadSub = '-' + kNumpadOffset,
            NumpadMul = '*' + kNumpadOffset, NumpadDiv = '/' + kNumpadOffset,

            // Characters, equals to 'A' to 'Z'.
            KeyA = 'A', KeyB = 'B', KeyC = 'C', KeyD = 'D', KeyE = 'E', KeyF = 'F', KeyG = 'G',
            KeyH = 'H', KeyI = 'I', KeyJ = 'J', KeyK = 'K', KeyL = 'L', KeyM = 'M', KeyN = 'N',
            KeyO = 'O', KeyP = 'P', KeyQ = 'Q', KeyR = 'R', KeyS = 'S', KeyT = 'T',
            KeyU = 'U', KeyV = 'V', KeyW = 'W', KeyX = 'X', KeyY = 'Y', KeyZ = 'Z',

            // Symbols.
            OMETilde = '`', OEMSub = '-', OEMAdd = '+', OEMLBracket = '[', OEMRBracket = ']', OEMRSlash = '\\',
            OEMColon = ';', OEMQuote = '\'', OEMComma = ',', OEMPeriod = '.', OEMSlash = '/',
        };

        // Indicate what the message is.
        const EEvent Event = EEvent::Unknown;
        const EEventSource Source = EEventSource::None;
        const EMouseButton MouseButton = EMouseButton::None;

        const int CursorPositionX = 0;
        const int CursorPositionY = 0;
        const EKeyCode Key = EKeyCode::Invalid;

        Message() = default;
        Message(const Message& other) = default;
        Message(EEvent ev, EMouseButton btn,
            unsigned int x, unsigned int y);            // Build message of mouse event.
        Message(EEvent ev, EKeyCode key);               // Build message of keyboard event.
        Message(EEvent ev, EEventSource src);           // Build message of lost focus event.

        //for-internal use
        Message(EEvent ev, EMouseButton btn);           // Build mouse of mouse event by another incomplete message.
        Message(const Message& m, int x, int y);
        Message(const Message& m, EKeyCode key);        // Build mouse of keyboard event by another incomplete message.
    };

    // Use this function to translate system messages to engine-used Messages.
    Message GameEngineAPI TranslateMessageFromWin32(unsigned int message, unsigned int wparam, unsigned int lparam);
}