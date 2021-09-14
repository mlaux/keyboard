/* A MIDI keyboard application for Windows. Controlled with the first 2 rows
   of the computer keyboard. Should compile on any Windows version down to
   Windows 3.0 or 3.1. Doesn't use any fancy APIs. */
#include <windows.h>
#include <string.h> // for borland
#include <mmsystem.h> // for win 3.1

/* I think these keys were added in win2k */
#ifndef VK_OEM_COMMA
#define VK_OEM_COMMA 188
#endif

#ifndef VK_OEM_PERIOD
#define VK_OEM_PERIOD 190
#endif

/* The keys used to control the keyboard. */
static const char *keys = "Q2W3ER5T6Y7UI9O0P";
/* Note value assigned to each key. */
static unsigned char values[256];
/* MIDI octave to play in. */
static unsigned char octave = 5;
/* Handle to the MIDI device. */
static HMIDIOUT hMidi;
/* Handle to the window. */
static HWND hWnd;
/* Handle to the drop-down list. */
static HWND hDropDown;
/* for painting background of pressed keys */
static HBRUSH hRedBrush;

/* All the MIDI instruments. */
static char *patches[] = {
    "Acoustic Grand Piano",
    "Bright Acoustic Piano",
    "Electric Grand Piano",
    "Honky-tonk Piano",
    "Rhodes Piano",
    "Chorused Piano",
    "Harpsichord",
    "Clavinet",
    "Celesta",
    "Glockenspiel",
    "Music Box",
    "Vibraphone",
    "Marimba",
    "Xylophone",
    "Tubular Bells",
    "Dulcimer",
    "Hammond Organ",
    "Percussive Organ",
    "Rock Organ",
    "Church Organ",
    "Reed Organ",
    "Accordion",
    "Harmonica",
    "Tango Accordion",
    "Acoustic Guitar (nylon)",
    "Acoustic Guitar (steel)",
    "Electric Guitar (jazz)",
    "Electric Guitar (clean)",
    "Electric Guitar (muted)",
    "Overdriven Guitar",
    "Distortion Guitar",
    "Guitar Harmonics",
    "Acoustic Bass",
    "Electric Bass (finger)",
    "Electric Bass (pick)",
    "Fretless Bass",
    "Slap Bass 1",
    "Slap Bass 2",
    "Synth Bass 1",
    "Synth Bass 2",
    "Violin",
    "Viola",
    "Cello",
    "Contrabass",
    "Tremolo Strings",
    "Pizzicato Strings",
    "Orchestral Harp",
    "Timpani",
    "String Ensemble 1",
    "String Ensemble 2",
    "SynthStrings 1",
    "SynthStrings 2",
    "Choir Aahs",
    "Voice Oohs",
    "Synth Voice",
    "Orchestra Hit",
    "Trumpet",
    "Trombone",
    "Tuba",
    "Muted Trumpet",
    "French Horn",
    "Brass Section",
    "Synth Brass 1",
    "Synth Brass 2",
    "Soprano Sax",
    "Alto Sax",
    "Tenor Sax",
    "Baritone Sax",
    "Oboe",
    "English Horn",
    "Bassoon",
    "Clarinet",
    "Piccolo",
    "Flute",
    "Recorder",
    "Pan Flute",
    "Bottle Blow",
    "Shakuhachi",
    "Whistle",
    "Ocarina",
    "Lead 1 (square)",
    "Lead 2 (sawtooth)",
    "Lead 3 (calliope lead)",
    "Lead 4 (chiff lead)",
    "Lead 5 (charang)",
    "Lead 6 (voice)",
    "Lead 7 (fifths)",
    "Lead 8 (bass + lead)",
    "Pad 1 (new age)",
    "Pad 2 (warm)",
    "Pad 3 (polysynth)",
    "Pad 4 (choir)",
    "Pad 5 (bowed)",
    "Pad 6 (metallic)",
    "Pad 7 (halo)",
    "Pad 8 (sweep)",
    "FX 1 (rain)",
    "FX 2 (soundtrack)",
    "FX 3 (crystal)",
    "FX 4 (atmosphere)",
    "FX 5 (brightness)",
    "FX 6 (goblins)",
    "FX 7 (echoes)",
    "FX 8 (sci-fi)",
    "Sitar",
    "Banjo",
    "Shamisen",
    "Koto",
    "Kalimba",
    "Bagpipe",
    "Fiddle",
    "Shanai",
    "Tinkle Bell",
    "Agogo",
    "Steel Drums",
    "Woodblock",
    "Taiko Drum",
    "Melodic Tom",
    "Synth Drum",
    "Reverse Cymbal",
    "Guitar Fret Noise",
    "Breath Noise",
    "Seashore",
    "Bird Tweet",
    "Telephone Ring",
    "Helicopter",
    "Applause",
    "Gunshot"
};

void initGDIObjects(void)
{
    LOGBRUSH redBrushDesc = { BS_SOLID, RGB(255, 0, 0), 0 };
    hRedBrush = CreateBrushIndirect(&redBrushDesc);
}

void destroyGDIObjects(void)
{
    DeleteObject(hRedBrush);
}

/* Sets the key notes based on the octave */
void setKeyValues(void)
{
    int k, len = strlen(keys);
    for(k = 0; k < len; k++) {
        values[keys[k]] = k + (octave * 12);
    }
    /* Force a redraw of the window */
    InvalidateRect(hWnd, NULL, FALSE);
}

void populateList(void)
{
    int k;

    /* Add all the instruments to the list box. */
    for(k = 0; k < 128; k++)
        SendMessage(hDropDown, CB_ADDSTRING, (WPARAM) 0, (LPARAM) (LPCSTR) patches[k]);
    /* Set the highlighted item to the first one. */
    SendMessage(hDropDown, CB_SETCURSEL, (WPARAM) 0, (LPARAM) 0);
}

/* Sends a message to the MIDI output. */
void sendMIDIMessage(int status, int channel, int data1, int data2)
{
    DWORD message = ((DWORD) data2) << 16 | (data1 << 8) | status | channel;
    midiOutShortMsg(hMidi, message);
}

/* Called when a key is pressed down */
void onKeyDown(unsigned char key)
{
    if (key == VK_ESCAPE) { /* exit */
        PostQuitMessage(0);
    } else if (key == VK_OEM_COMMA) { /* octave down */
        if (octave > 1) octave--;
        setKeyValues();
    } else if (key == VK_OEM_PERIOD) { /* octave up */
        if (octave < 10) octave++;
        setKeyValues();
    } else if (values[key] != 0) { /* some note key */
        sendMIDIMessage(0x90, 0, values[key], 127);
        /* Force a redraw of the window */
        InvalidateRect(hWnd, NULL, FALSE);
    }
}

/* Called when a key is released */
void onKeyUp(unsigned char key)
{
    if (values[key] != 0) { /* end the note by sending a velocity of 0 */
        sendMIDIMessage(0x90, 0, values[key], 0);
        /* Force a redraw of the window */
        InvalidateRect(hWnd, NULL, FALSE);
    }
}

/* Returns 1 if the key (0 to 11) is an accidental */
int isAccidental(int key)
{
    int pos = key % 12;
    return pos == 1 || pos == 3 || pos == 6 || pos == 8 || pos == 10;
}

/* Draws the keyboard on the window. */
void drawKeyboard(HDC hdc)
{
    static const char *octaves = "0123456789";
    int k, x = 0, y, len = strlen(keys);

    /* Draw the octave */
    TextOut(hdc, 200, 165, "Octave: ", 8);
    TextOut(hdc, 260, 165, &octaves[octave - 1], 1);
    TextOut(hdc, 280, 165, "(< and > to change)", 19);

    for (k = 0; k < len; k++) {
        if (GetAsyncKeyState(keys[k]) & 0x80000000) {
            /* Set the background of the note to red */
            SelectObject(hdc, hRedBrush);
            /* Set the text foreground color to white */
            SetTextColor(hdc, RGB(255, 255, 255));
            /* Set the text background color to red */
            SetBkColor(hdc, RGB(255, 0, 0));
        } else if (isAccidental(k)) {
            /* Set the background of the note to black */
            SelectObject(hdc, GetStockObject(BLACK_BRUSH));
            /* Set the text foreground color to white */
            SetTextColor(hdc, RGB(255, 255, 255));
            /* Set the text background color to black */
            SetBkColor(hdc, RGB(0, 0, 0));
        } else {
            /* Set the background of the note to white */
            SelectObject(hdc, GetStockObject(WHITE_BRUSH));
            /* Set the text foreground color to black */
            SetTextColor(hdc, RGB(0, 0, 0));
            /* Set the text background color to white */
            SetBkColor(hdc, RGB(255, 255, 255));
        }

        /* If the note is a white key, draw it lower */
        if (!isAccidental(k))
            y = 80;
        else y = 0;

        /* Draw the actual key */
        Rectangle(hdc, x, y, x + 59, y + 79);
        /* Set the text font to something less ugly than SYSTEM_FONT */
        /* only works on win95+ */
#ifdef DEFAULT_GUI_FONT
        SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
#else
        SelectObject(hdc, GetStockObject(SYSTEM_FONT));
#endif
        /* Draw the letter which controls this key */
        TextOut(hdc, x + 10, y + 10, &keys[k], 1);

        /* Increment the position for the next key. If there are 2 white
           keys in a row (E to F or B to C), make the gap bigger so they
           don't run into each other, otherwise only offset a bit so the
           keys overlap. */
        if (!isAccidental(k) && !isAccidental(k + 1))
            x += 60;
        else x += 30;
    }
}

/* Called when the window receives a message */
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    int notifCode;

    switch (msg) {
        case WM_COMMAND:
#ifdef WIN32
            notifCode = HIWORD(wParam);
#else
            notifCode = HIWORD(lParam);
#endif
            if (notifCode == CBN_SELCHANGE) { /* New item selected from the listbox */
                /* Send the MIDI message to change the instrument. The 'patch number'
                   that the instrument is changed to is queried from the dropdown list. */
                int patch = (int) SendMessage(hDropDown, CB_GETCURSEL, 0, 0);
                sendMIDIMessage(0xC0, 0, patch, 0);
                /* Set the focus back to the main window so we can still get key events */
                SetFocus(hWnd);
            } else if (notifCode == CBN_SELENDCANCEL) {
                /* The user clicked out of the combo box without selecting anything.
                   Set the focus back to the main window so we can still get key events */
                SetFocus(hWnd);
            }
            break;
        case WM_DESTROY: /* Window is being closed */
            PostQuitMessage(0);
            break;
        case WM_KEYDOWN: /* Key press */
            /* Only call the function if it's not a 'held down' key event.
               Otherwise, the notes would repeat and it'd sound bad. */
            if (((lParam >> 30) & 0x01) == 0)
                onKeyDown((unsigned char) wParam);
            break;
        case WM_KEYUP: /* Key release */
            onKeyUp((unsigned char) wParam);
            break;
        case WM_PAINT: /* Window is being redrawn */
            BeginPaint(hWnd, &ps);
            /* draw the keyboard */
            drawKeyboard(ps.hdc);
            EndPaint(hWnd, &ps);
            break;
        default:
            /* Make Windows handle all messages that we don't. */
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}

/* Main entry point. */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc;
    MSG msg;
    RECT rct;

    initGDIObjects();

    /* Create and register the window type */
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "MidiKeyboard";

    RegisterClass(&wc);

    /* Calculate the width/height of the window based on the
       width/height of the canvas area */
    rct.left = 0;
    rct.top = 0;
    rct.right = 600;
    rct.bottom = 185;
    AdjustWindowRect(&rct, WS_OVERLAPPEDWINDOW, FALSE);

    /* Actually create/show the window */
    hWnd = CreateWindow("MidiKeyboard", "Keyboard", WS_OVERLAPPED | WS_CAPTION
        | WS_MINIMIZEBOX | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT,
        rct.right - rct.left, rct.bottom - rct.top, 0, 0, hInstance, 0);
    hDropDown = CreateWindow("COMBOBOX", NULL, CBS_DROPDOWNLIST
        | WS_VISIBLE | WS_VSCROLL | WS_CHILD, 0, 160, 200, 200, hWnd, 0, hInstance, 0);
    populateList();
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    /* Set up the MIDI stuff */
    midiOutOpen(&hMidi, (UINT) MIDIMAPPER, 0, 0, 0);
    /* switch to first instrument */
    sendMIDIMessage(0xC0, 0, 0, 0);
    setKeyValues();

    /* Message loop */
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    /* Release the MIDI resources. */
    midiOutClose(hMidi);
    destroyGDIObjects();

    return msg.wParam;
}

