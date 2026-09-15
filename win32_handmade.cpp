/*
    $File: $
    $Date: $
    $Revision: $
    $Creator: Paul Maverick Pastor $
    $Notice: This is used as a learning material
*/

#include <windows.h>
#include <stdint.h>
#include <ctime>
#include <cstdlib>
#include <xinput.h>

typedef uint8_t uint8;
typedef uint32_t uint32;

#define internal static
#define local_persist static
#define global_variable static

struct Win32_Back_Buffer {
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int BytesPerPixel;
};

struct Win32_Window_Dimension {
    int Width;
    int Height;
};

global_variable bool GlobalRunning; 
global_variable Win32_Back_Buffer Buffer;

internal int RandomizeColorValues();

internal void WeirdGraphicsRenderer(Win32_Back_Buffer Buffer, int XOffset, int YOffset) {
    int Width = Buffer.Width;
    int Height = Buffer.Height;

    uint8 *Row = (uint8 *)Buffer.Memory;
    int Pitch = Width*Buffer.BytesPerPixel;
    for(int Y = 0; Y < Buffer.Height; ++Y) {

        uint32 *Pixel = (uint32 *)Row;
        for(int X = 0; X < Buffer.Width; ++X) {
            uint8 Blue = (X+XOffset);
            uint8 Green = (Y+YOffset);  //std::rand() % 256;
            
            *Pixel++ = ((Green << 8) | Blue);
        }

        Row += Pitch;
    }
}

internal Win32_Window_Dimension Win32GetWindowDimensions(HWND Window) {
    Win32_Window_Dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return(Result);  
}

internal int RandomizeColorValues() {
    return std::rand() % 256;
}

internal void Win32CopyBackBufferToWindow(
    Win32_Back_Buffer *Buffer,
    int Width,
    int Height 
)   {

    if(Buffer->Memory) {  
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE); 
    }

    Buffer->Width = Width;
    Buffer->Height = Height;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    Buffer->BytesPerPixel = 4;
    int BitmapMemorySize = (Buffer->Width*Buffer->Height)*Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void Win32UpdateWindows(
    HDC DeviceContext,
    Win32_Back_Buffer Buffer,
    int WindowWidth,
    int WindowHeight,
    int X,
    int Y,
    int Width,
    int Height
) {
    StretchDIBits(DeviceContext,
        /*X, Y, Width, Height,
        X, Y, Width, Height, */
        0, 0, WindowWidth, WindowHeight,       
        0, 0, Buffer.Width, Buffer.Height,     
        Buffer.Memory,
        &Buffer.Info,
        DIB_RGB_COLORS,
        SRCCOPY
    );
}

LRESULT CALLBACK Win32MainWindowCallBack(
    HWND Window,
    UINT Message,
    WPARAM WParam,
    LPARAM LParam
) 
{
    LRESULT Result = 0;

    switch (Message)
    {
        case WM_SIZE:
        {
            // Win32_Window_Dimension Dimension = Win32GetWindowDimensions(Window);
            // Win32CopyBackBufferToWindow(&Buffer, Dimension.Width, Dimension.Height);
            
        } break;

        case WM_DESTROY:
        {
            GlobalRunning = false;
        } break;

        case WM_CLOSE:
        {
            GlobalRunning = false;
        }break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n"); 
        } break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            int Width = Paint.rcPaint.right - Paint.rcPaint.left;

            Win32_Window_Dimension Dimension = Win32GetWindowDimensions(Window);

            Win32UpdateWindows(DeviceContext, Buffer, Dimension.Width, Dimension.Height, X, Y, Width, Height);
            EndPaint(Window, &Paint);


        } break;
    
        default:
        {
            //OutputDebugStringA("default\n");
            Result = DefWindowProc(
                Window,
                Message,
                WParam,
                LParam
            );
        } break;
    }

    return(Result);
}   

int CALLBACK WinMain(
    HINSTANCE Instannce,
    HINSTANCE PrevInstance,
    LPSTR CommandLine,
    int CodeShow
) {
    WNDCLASSA WindowClass = {};

    Win32CopyBackBufferToWindow(&Buffer, 1280, 720);

    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallBack;
    WindowClass.hInstance = Instannce;
//  WindowClass.hIcon
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClassA(&WindowClass)) {
        HWND Window = CreateWindowExA(
            0,
            WindowClass.lpszClassName,
            "Handmade Hero",
            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT, 
            CW_USEDEFAULT, 
            CW_USEDEFAULT, 
            0,
            0,
            Instannce,
            0
        );

        if (Window) {  
            std::srand(std::time(0));
            int XOffset = 0;
            int YOffset = 0;
            GlobalRunning = true;
            while (GlobalRunning){
                

                MSG Message;
                while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)) {
                    if(Message.message == WM_QUIT) {
                        GlobalRunning = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }

                for(DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ControllerIndex++) {
                    XINPUT_STATE XInputState;
                    
                    if(XInputGetState(ControllerIndex, &XInputState) == ERROR_SUCCESS) {
                        XINPUT_GAMEPAD *Pad = &XInputState.Gamepad;

                        
                    } else {

                    }
                }

                WeirdGraphicsRenderer(Buffer, XOffset, YOffset);
                HDC DeviceContext = GetDC(Window);
                Win32_Window_Dimension Dimension = Win32GetWindowDimensions(Window);

                Win32UpdateWindows(DeviceContext, Buffer, 
                    Dimension.Width, Dimension.Height, 0, 0, 
                    Dimension.Width, Dimension.Height);
                ReleaseDC(Window, DeviceContext);

                ++XOffset;
            }
        }
    } else {
        //TODO
    }

    return (0);
}