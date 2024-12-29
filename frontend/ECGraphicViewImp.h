#ifndef ECGraphicViewImp_h
#define ECGraphicViewImp_h

#include <vector>
#include <map>
#include "ECObserver.h"
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

//***********************************************************
// Supported event codes

enum ECGVEventType {
    ECGV_EV_NULL = -1,
    ECGV_EV_CLOSE = 0,
    ECGV_EV_KEY_UP_UP = 1,
    ECGV_EV_KEY_UP_DOWN = 2,
    ECGV_EV_KEY_UP_LEFT = 3,
    ECGV_EV_KEY_UP_RIGHT = 4,
    ECGV_EV_KEY_UP_ESCAPE = 5,
    ECGV_EV_KEY_DOWN_UP = 6,
    ECGV_EV_KEY_DOWN_DOWN = 7,
    ECGV_EV_KEY_DOWN_LEFT = 8,
    ECGV_EV_KEY_DOWN_RIGHT = 9,
    ECGV_EV_KEY_DOWN_ESCAPE = 10,
    ECGV_EV_TIMER = 11,
    ECGV_EV_MOUSE_BUTTON_DOWN = 12,
    ECGV_EV_MOUSE_BUTTON_UP = 13,
    ECGV_EV_MOUSE_MOVING = 14,
    // more keys
    ECGV_EV_KEY_UP_Z = 15,
    ECGV_EV_KEY_DOWN_Z = 16,
    ECGV_EV_KEY_UP_Y = 17,
    ECGV_EV_KEY_DOWN_Y = 18,
    ECGV_EV_KEY_UP_D = 19,
    ECGV_EV_KEY_DOWN_D = 20,
    ECGV_EV_KEY_UP_SPACE = 21,
    ECGV_EV_KEY_DOWN_SPACE = 22,
    ECGV_EV_KEY_DOWN_G = 23,
    ECGV_EV_KEY_UP_G = 24
};

//***********************************************************
// Pre-defined color

enum ECGVColor {
    ECGV_BLACK = 0,
    ECGV_WHITE = 1,
    ECGV_RED = 2,
    ECGV_GREEN = 3,
    ECGV_BLUE = 4,
    ECGV_YELLOW = 5,
    ECGV_PURPLE = 6,
    ECGV_CYAN = 7,
    ECGV_NONE = 8,
    ECGV_NUM_COLORS
};

// Allegro color
extern ALLEGRO_COLOR arrayAllegroColors[ECGV_NUM_COLORS];

class ECGraphicViewImp : public ECObserverSubject {
public:
    // Create a view with size (width, height)
    ECGraphicViewImp(int width, int height);
    virtual ~ECGraphicViewImp();
    
    // Show the view. This would enter a forever loop, until quit is set. 
    void Show();
    
    // Set flag to redraw (or not)
    void SetRedraw(bool f) { fRedraw = f; }
    
    // Access view properties
    int GetWith() const { return widthView; }
    int GetWidth() const { return widthView; }
    int GetHeight() const { return heightView; }
    
    // Get cursor position (cx, cy)
    void GetCursorPosition(int &cx, int &cy) const;

    // The current event
    ECGVEventType GetCurrEvent() const { return evtCurrent; }

    // Drawing functions
    void DrawLine(int x1, int y1, int x2, int y2, int thickness=3, ECGVColor color=ECGV_BLACK);
    void DrawRectangle(int x1, int y1, int x2, int y2, int thickness=3, ECGVColor color=ECGV_BLACK);
    void DrawFilledRectangle(int x1, int y1, int x2, int y2, ECGVColor color=ECGV_BLACK);
    void DrawCircle(int xcenter, int ycenter, double radius, int thickness=3, ECGVColor color=ECGV_BLACK);
    void DrawFilledCircle(int xcenter, int ycenter, double radius, ECGVColor color=ECGV_BLACK);
    void DrawEllipse(int xcenter, int ycenter, double radiusx, double radiusy, int thickness=3, ECGVColor color=ECGV_BLACK);
    void DrawFilledEllipse(int xcenter, int ycenter, double radiusx, double radiusy, ECGVColor color=ECGV_BLACK);
    void DrawText(int xcenter, int ycenter, const char *ptext, ECGVColor color = ECGV_BLACK);
    void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int thickness=3, ECGVColor color=ECGV_BLACK);
    void DrawFilledTriangle(int x1, int y1, int x2, int y2, int x3, int y3, ECGVColor color=ECGV_BLACK);

protected:
    // Render functions for derived classes to access
    void RenderStart();
    void RenderEnd();

private:
    // Internal functions
    void Init();
    void Shutdown();
    ECGVEventType  WaitForEvent();

    // Data members
    int widthView;
    int heightView;
    bool fRedraw;
    ECGVEventType evtCurrent;

    // Allegro related data
    ALLEGRO_DISPLAY *display;
    ALLEGRO_EVENT_QUEUE *event_queue;
    ALLEGRO_TIMER *timer;
    ALLEGRO_FONT *fontDef;
};

#endif /* ECGraphicViewImp_h */