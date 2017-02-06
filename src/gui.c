#include<xcb/xcb.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include"gui.h"
#include"main.h"

int windowHeight = 260;
int windowWidth = 400;

int numberOfLinesToPrint;
char** linesToPrint = NULL;

xcb_connection_t* connection = NULL;
xcb_screen_t* screen = NULL;
xcb_window_t window = 0;

static xcb_gc_t getFontGC (xcb_connection_t* c, xcb_screen_t* screen, xcb_window_t winndow, const char* font_name );
static void drawText (xcb_connection_t *c, xcb_screen_t *screen, xcb_window_t window, int16_t x1, int16_t y1, const char* label );

static void testCookie (xcb_void_cookie_t cookie,
                        xcb_connection_t *connection,
                        char *errMessage ) {
    xcb_generic_error_t *error = xcb_request_check (connection, cookie);
    if (error) {
        fprintf (stderr, "ERROR: %s : %i\n", errMessage , error->error_code);
        xcb_disconnect (connection);
        exit (-1);
    }
}

static void drawText(xcb_connection_t  *connection,
                     xcb_screen_t      *screen,
                     xcb_window_t       window,
                     int16_t            x1,
                     int16_t            y1,
                     const char        *label ) {
    xcb_gcontext_t gc = getFontGC(connection,screen,window,"fixed");
    xcb_void_cookie_t textCookie = xcb_image_text_8_checked(connection,strlen(label),window,gc,x1,y1,label);
    testCookie(textCookie,connection,"can't paste text");
    xcb_void_cookie_t gcCookie = xcb_free_gc(connection,gc);
    testCookie(gcCookie,connection,"can't free gc");
}

static xcb_gc_t getFontGC (xcb_connection_t  *connection,
                           xcb_screen_t      *screen,
                           xcb_window_t       window,
                           const char        *font_name ) {
    //get font
    xcb_font_t font = xcb_generate_id (connection);
    xcb_void_cookie_t fontCookie = xcb_open_font_checked(connection,font,strlen(font_name),font_name);
    testCookie(fontCookie,connection,"can't open font");

    //get graphics content
    xcb_gcontext_t gc = xcb_generate_id (connection);
    uint32_t mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
    uint32_t value_list[3] = { screen->white_pixel, screen->black_pixel, font };
    xcb_void_cookie_t gcCookie = xcb_create_gc_checked (connection,
                                                        gc,
                                                        window,
                                                        mask,
                                                        value_list );
    testCookie(gcCookie, connection, "can't create gc");

    //close font
    fontCookie = xcb_close_font_checked (connection, font);
    testCookie(fontCookie, connection, "can't close font");

    //finish
    return gc;
}

void updateData(){
    if (linesToPrint == NULL) {
        linesToPrint = allocForDirToStrings();
    }
    dirToStrings(linesToPrint,&numberOfLinesToPrint);
}

void guiStart() {
    int screenNum;
    
    //get connection
    connection = xcb_connect(NULL,&screenNum);
    
    //get screen
    const xcb_setup_t* setup = xcb_get_setup(connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    for(int i = 0; i < screenNum; ++i) {
        xcb_screen_next(&iter);
    }
    screen = iter.data;

    //get window
    window = xcb_generate_id(connection);
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[2];
    values[0] = screen->black_pixel;
    values[1] = XCB_EVENT_MASK_KEY_RELEASE |
                XCB_EVENT_MASK_EXPOSURE;
    xcb_void_cookie_t windowCookie = xcb_create_window_checked(connection,screen->root_depth,window,screen->root,500,500,windowWidth,windowHeight,0,XCB_WINDOW_CLASS_INPUT_OUTPUT,screen->root_visual,mask,values);
    testCookie(windowCookie,connection,"can't create window");

    xcb_void_cookie_t mapCookie = xcb_map_window_checked (connection, window);
    testCookie(mapCookie,connection,"can't map window");

    xcb_flush(connection);

    updateData();
}

void drawAllText() {
    for (int i = 0; i < numberOfLinesToPrint; i++)
    {
        drawText (connection,screen,window,10, windowHeight - (10*i), linesToPrint[i]);
    }
}

void guiEventLoop() {
    xcb_generic_event_t* event;    
    while(1) {
        //whenever we get a new event, do...
        if((event = xcb_poll_for_event(connection))) {
            switch(event->response_type & ~0x80) { //what the actual fuck
                case XCB_EXPOSE: {
                    drawAllText();
                    break;
                }
                case XCB_KEY_RELEASE: {
                    xcb_key_release_event_t *kr = (xcb_key_release_event_t *)event;
                    switch (kr->detail) {
                        case 9: { //ESC key
                            free (event);
                            xcb_disconnect(connection);
                            return;
                        }
                        default:
                            printf("%i pressed... no handling\n",kr->detail);
                            fflush(stdout);
                            break;
                    }
                }
            }
        }
    }
}