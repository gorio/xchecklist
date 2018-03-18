#include "XPLMDisplay.h"    // for window creation and manipulation
#include "XPLMGraphics.h"   // for window drawing
#include "XPLMDataAccess.h" // for the VR dataref
#include "XPLMPlugin.h"     // for XPLM_MSG_SCENERY_LOADED message
#include "XPLMUtilities.h"  // for Various utilities
#include <stdio.h>
#include <string.h>
#if IBM
    #include <windows.h>
#endif
#if LIN
    #include <GL/gl.h>
#elif __GNUC__
    #include <GL/gl.h>
#else
    #include <GL/gl.h>
#endif

#ifndef XPLM301
    #error This is made to be compiled against the XPLM301 SDK
#endif



#include "utils.h"
#include "interface.h"

static XPLMWindowID	xcvr_g_window;

static float g_vr_button_lbrt[4]; // left, bottom, right, top


static float g_check_box_lbrt[25][4]; // left, bottom, right, top

static float g_previous_button_lbrt[4]; // left, bottom, right, top
static float g_next_button_lbrt[4]; // left, bottom, right, top
static float g_check_item_button_lbrt[4]; // left, bottom, right, top



char scratch_buffer[150];
float col_white[] = {1.0, 1.0, 1.0};
float col_green[] = {0.0, 1.0, 0.0};
float col_red[] = {1.0, 0.0, 0.0};
float col_fuchsia[] = {1.0, 0.0, 1.0};
float col_blue[] = {0.0, 0.0, 1.0};
float col_black[] = {0.0, 0.0, 0.0};

float green[] = {0.0, 1.0, 0.0, 1.0};
float white[] = {1.0, 1.0, 1.0, 1.0};

char * copilot_on = "+";


char * checkmark_off = "    ";
char * checkmark_on = " X ";

int checked [25];

int line_number = 2;

int size = 25;

size_t ii;


// bounds_lbrt  0 left,  1 bottom,  2 right,  3 top
static int	coord_in_rect(float x, float y, float * bounds_lbrt)  { return (((x - 10) >= bounds_lbrt[0]) && ((x - 20) < bounds_lbrt[2]) && (y < bounds_lbrt[3]) && (y >= bounds_lbrt[1])); }



void	xcvr_draw(XPLMWindowID in_window_id, void * in_refcon)
{

    XPLMSetGraphicsState(
            0 /* no fog */,
            0 /* 0 texture units */,
            0 /* no lighting */,
            0 /* no alpha testing */,
            1 /* do alpha blend */,
            1 /* do depth testing */,
            0 /* no depth writing */
    );

    // We draw our rudimentary button boxes based on the height of the button text
    int char_height;
    XPLMGetFontDimensions(xplmFont_Proportional, NULL, &char_height, NULL);

    int l, t, r, b;
    XPLMGetWindowGeometry(in_window_id, &l, &t, &r, &b);


    // const int vr_is_enabled = XPLMGetDatai(g_vr_dref);
    if(vr_is_enabled)
    {
       XPLMSetWindowPositioningMode(xcvr_g_window, xplm_WindowVR, 0);
    }
    else
    {
        XPLMSetWindowPositioningMode(xcvr_g_window, xplm_WindowPositionFree, 0);

    }

        // Draw the main body of the checklist window.

        line_number = 1;

        // xcDebug("Xchecklist: xcvr_title = %s\n", xcvr_title);
        // xcDebug("Xchecklist: xcvr_width = %d    xcvr_height = %d\n", xcvr_width, xcvr_height);


        for (ii = 0; ii < xcvr_size; ++ii) {



            // If we have a copilot draw the copilot symbol
            if (xcvr_copilot_controlled[ii]) {
                XPLMDrawString(col_white, l, t - (line_number * char_height), copilot_on, NULL, xplmFont_Proportional);
            }



            // If checkable then draw a checkbox
            if (!xcvr_item_void[ii]) {
                // Draw the checkbox box.
                // Position the button in the upper left of the window (sized to fit the button text)
                g_check_box_lbrt[ii][0] = l + 16;
                g_check_box_lbrt[ii][3] = t - (line_number * char_height) + 10;
                g_check_box_lbrt[ii][2] = g_check_box_lbrt[ii][0] + XPLMMeasureString(xplmFont_Proportional, checkmark_off, strlen(checkmark_off)); // *just* wide enough to fit the button text
                g_check_box_lbrt[ii][1] = g_check_box_lbrt[ii][3] - (1.25f * char_height); // a bit taller than the button text

                // Draw the box around our rudimentary button
                glColor4fv(green);
                glBegin(GL_POLYGON);
                {
                    glVertex2i(g_check_box_lbrt[ii][0], g_check_box_lbrt[ii][3]);
                    glVertex2i(g_check_box_lbrt[ii][2], g_check_box_lbrt[ii][3]);
                    glVertex2i(g_check_box_lbrt[ii][2], g_check_box_lbrt[ii][1]);
                    glVertex2i(g_check_box_lbrt[ii][0], g_check_box_lbrt[ii][1]);
                }
                glEnd();

                if (xcvr_item_checked[ii])
                {
                   XPLMDrawString(col_black, g_check_box_lbrt[ii][0], g_check_box_lbrt[ii][1] + 4, (char *)checkmark_on, NULL, xplmFont_Proportional);
                }
                else
                {
                   XPLMDrawString(col_black, g_check_box_lbrt[ii][0], g_check_box_lbrt[ii][1] + 4, (char *)checkmark_off, NULL, xplmFont_Proportional);
                }

                // Draw the checkmard on the checkmark box.
                // XPLMDrawString(col_black, g_check_box_lbrt[i][0], g_check_box_lbrt[i][1] + 4, (char *)Checkmark[i], NULL, xplmFont_Proportional);

            }


            // Draw text for what to be checked.
            XPLMDrawString(col_white, l + 40, t - line_number * char_height, (char *)xcvr_text[ii], NULL, xplmFont_Proportional);

            // Draw text for the result to be checked
            XPLMDrawString(col_white, l + xcvr_right_text_start, t - line_number * char_height, (char *)xcvr_suffix[ii], NULL, xplmFont_Proportional);
            //XPLMDrawString(col_white, l + 350, t - line_number * char_height, (char *)xcvr_suffix[i], NULL, xplmFont_Proportional);


            // xcDebug("Xchecklist: xcvr_item_checked[ii] =  %d  \n", xcvr_item_checked[ii]);

            line_number = line_number + 2;


        }


        // Draw the Previous button
        line_number = line_number + 2;
        const char * previous_btn_label = "     Previous     ";

        // 0 left, 1 bottom, 2 right, 3 top
        // Position the button in the upper left of the window (sized to fit the button text)
        g_previous_button_lbrt[0] = l;
        g_previous_button_lbrt[3] = t - (line_number * char_height);
        g_previous_button_lbrt[2] = g_previous_button_lbrt[0] + XPLMMeasureString(xplmFont_Proportional, previous_btn_label, strlen(previous_btn_label)); // *just* wide enough to fit the button text
        g_previous_button_lbrt[1] = g_previous_button_lbrt[3] - (1.25f * char_height); // a bit taller than the button text

        // Draw the box around our rudimentary button
        glColor4fv(green);
        glBegin(GL_POLYGON);
        {
            glVertex2i(g_previous_button_lbrt[0], g_previous_button_lbrt[3]);
            glVertex2i(g_previous_button_lbrt[2], g_previous_button_lbrt[3]);
            glVertex2i(g_previous_button_lbrt[2], g_previous_button_lbrt[1]);
            glVertex2i(g_previous_button_lbrt[0], g_previous_button_lbrt[1]);
        }
        glEnd();

        // Draw the text on the previous button.
        // 0 left, 1 bottom, 2 right, 3 top
        XPLMDrawString(col_black, g_previous_button_lbrt[0], g_previous_button_lbrt[1] + 4, (char *)previous_btn_label, NULL, xplmFont_Proportional);


        // Draw the Check Item button
        const char * check_item_btn_label = "     Check Item     ";

        // 0 left, 1 bottom, 2 right, 3 top
        // Position the button in the upper left of the window (sized to fit the button text)
        g_check_item_button_lbrt[0] = l + 150;
        g_check_item_button_lbrt[3] = t - (line_number * char_height);
        g_check_item_button_lbrt[2] = g_check_item_button_lbrt[0] + XPLMMeasureString(xplmFont_Proportional, check_item_btn_label, strlen(check_item_btn_label)); // *just* wide enough to fit the button text
        g_check_item_button_lbrt[1] = g_check_item_button_lbrt[3] - (1.25f * char_height); // a bit taller than the button text

        // Draw the box around our rudimentary button
        glColor4fv(green);
        glBegin(GL_POLYGON);
        {
            glVertex2i(g_check_item_button_lbrt[0], g_check_item_button_lbrt[3]);
            glVertex2i(g_check_item_button_lbrt[2], g_check_item_button_lbrt[3]);
            glVertex2i(g_check_item_button_lbrt[2], g_check_item_button_lbrt[1]);
            glVertex2i(g_check_item_button_lbrt[0], g_check_item_button_lbrt[1]);
        }
        glEnd();

        // Draw the text on the check item button.
        // 0 left, 1 bottom, 2 right, 3 top
        XPLMDrawString(col_black, g_check_item_button_lbrt[0], g_check_item_button_lbrt[1] + 4, (char *)check_item_btn_label, NULL, xplmFont_Proportional);


        // Draw the Next button
        const char * next_btn_label = "     Next     ";

        // Position the button in the upper left of the window (sized to fit the button text)
        g_next_button_lbrt[0] = l + 325;
        g_next_button_lbrt[3] = t - (line_number * char_height);
        g_next_button_lbrt[2] = g_next_button_lbrt[0] + XPLMMeasureString(xplmFont_Proportional, next_btn_label, strlen(next_btn_label)); // *just* wide enough to fit the button text
        g_next_button_lbrt[1] = g_next_button_lbrt[3] - (1.25f * char_height); // a bit taller than the button text

        // Draw the box around our rudimentary button
        //float green2[] = {0.0, 1.0, 0.0, 1.0};
        glColor4fv(green);
        glBegin(GL_POLYGON);
        {
            glVertex2i(g_next_button_lbrt[0], g_next_button_lbrt[3]);
            glVertex2i(g_next_button_lbrt[2], g_next_button_lbrt[3]);
            glVertex2i(g_next_button_lbrt[2], g_next_button_lbrt[1]);
            glVertex2i(g_next_button_lbrt[0], g_next_button_lbrt[1]);
        }
        glEnd();

        // Draw the text for the next button.
        XPLMDrawString(col_black, g_next_button_lbrt[0], g_next_button_lbrt[1] + 4, (char *)next_btn_label, NULL, xplmFont_Proportional);





        // Draw a bunch of informative text
            {
                // Set the y position for the first bunch of text we'll draw to a little below the buttons
                int y = g_previous_button_lbrt[1] - 2 * char_height;

                // Display whether we're in front of our our layer
                {
                    sprintf(scratch_buffer, "In front? %s", XPLMIsWindowInFront(in_window_id) ? "Y" : "N");
                    XPLMDrawString(col_white, l, y, scratch_buffer, NULL, xplmFont_Proportional);
                    y -= 1.5 * char_height;
                }

                // Display the total global desktop bounds
                {
                    int global_desktop_lbrt[4];
                    XPLMGetScreenBoundsGlobal(&global_desktop_lbrt[0], &global_desktop_lbrt[3], &global_desktop_lbrt[2], &global_desktop_lbrt[1]);
                    sprintf(scratch_buffer, "Global desktop bounds: (%d, %d) to (%d, %d)", global_desktop_lbrt[0], global_desktop_lbrt[1], global_desktop_lbrt[2], global_desktop_lbrt[3]);
                    XPLMDrawString(col_white, l, y, scratch_buffer, NULL, xplmFont_Proportional);
                    y -= 1.5 * char_height;
                }

                // Display our bounds
                if(XPLMWindowIsPoppedOut(in_window_id)) // we are in our own first-class window, rather than "floating" within X-Plane's own window
                {
                    int window_os_bounds[4];
                    XPLMGetWindowGeometryOS(in_window_id, &window_os_bounds[0], &window_os_bounds[3], &window_os_bounds[2], &window_os_bounds[1]);
                    sprintf(scratch_buffer, "OS Bounds: (%d, %d) to (%d, %d)", window_os_bounds[0], window_os_bounds[1], window_os_bounds[2], window_os_bounds[3]);
                    XPLMDrawString(col_white, l, y, scratch_buffer, NULL, xplmFont_Proportional);
                    y -= 1.5 * char_height;
                }
                else
                {
                    int global_bounds[4];
                    XPLMGetWindowGeometry(in_window_id, &global_bounds[0], &global_bounds[3], &global_bounds[2], &global_bounds[1]);
                    sprintf(scratch_buffer, "Window bounds: %d %d %d %d", global_bounds[0], global_bounds[1], global_bounds[2], global_bounds[3]);
                    XPLMDrawString(col_white, l, y, scratch_buffer, NULL, xplmFont_Proportional);
                    y -= 1.5 * char_height;
                }

                // Display the mouse's position info text
                {
                    int mouse_global_x, mouse_global_y;
                    XPLMGetMouseLocationGlobal(&mouse_global_x, &mouse_global_y);
                    sprintf(scratch_buffer, "Draw mouse (global): %d %d\n", mouse_global_x, mouse_global_y);
                    XPLMDrawString(col_white, l, y, scratch_buffer, NULL, xplmFont_Proportional);
                    y -= 1.5 * char_height;
                }

            }
}


int	xcvr_handle_mouse(XPLMWindowID xcvr_in_window_id, int xcvr_x, int xcvr_y, XPLMMouseStatus xcvr_mouse_status, void * in_refcon)
{

    if(xcvr_mouse_status == xplm_MouseDown)
    {
        if(!XPLMIsWindowInFront(xcvr_in_window_id))
        {
            XPLMBringWindowToFront(xcvr_in_window_id);
        }
        else
        {

            if (coord_in_rect(xcvr_x, xcvr_y, g_check_item_button_lbrt)) {
                sprintf(scratch_buffer, "Check Item button has been clicked\n");
                XPLMDebugString(scratch_buffer);
                if(item_checked(checkable)){
                  check_item(checkable);
                }
            }


            else if (coord_in_rect(xcvr_x, xcvr_y, g_previous_button_lbrt))
            {
                sprintf(scratch_buffer, "Previous button has been clicked\n");
                XPLMDebugString(scratch_buffer);
                prev_checklist();
            }


            else if (coord_in_rect(xcvr_x, xcvr_y, g_next_button_lbrt))
            {
                sprintf(scratch_buffer, "Next button has been clicked\n");
                XPLMDebugString(scratch_buffer);
                next_checklist(true);
            }

            for (int iii = 0; iii < xcvr_size; ++iii) {

                if(coord_in_rect(xcvr_x, xcvr_y, g_check_box_lbrt[iii])) // user clicked the pop-in/pop-out button
                {
                    sprintf(scratch_buffer, "Clickbox %d clicked \n", iii);
                    XPLMDebugString(scratch_buffer);
                    if (checked[iii] == 0)
                    {
                        checked[iii] = 1;
                    }
                    else
                    {
                       checked[iii] = 0;
                    }
                }
            }

        }

    }
    return 1;
}