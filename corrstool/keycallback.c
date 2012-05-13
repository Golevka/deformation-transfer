#include <string.h>
#include <errno.h>

#include "interact.h"
#include "render.h"
#include "pickup_list.h"



/* Registered GLUT callback function */
void __callback_OnKeyDown(unsigned char key, int sx, int sy)
{
    switch (key) {
        case 'p':  __callback_OnPickup(sx, sy);      break;
        case 'a':  __callback_AppendCurrent();       break;
        case 'm':  __callback_SwitchRenderMode();    break;
        case 'c':  __callback_SwitchVertex(sx, sy);  break;
        case 'w':  __callback_WriteToFile();         break;
        case 'u':  __callback_Undo();                break;

    default: break;
    }
}


/* pick up an object (triangular unit) */
void __callback_OnPickup(int sx, int sy)
{
    const dtMeshModel *model;
    __gl_UIPanel panel = __gl_SelectPanelbyPoint(sx, sy);
    int i_obj;

    switch (panel) {
        case __gl_PANEL_LEFTVIEW:   model = &modelL; break;
        case __gl_PANEL_RIGHTVIEW:  model = &modelR; break;

    default: return;
    }

    __gl_SetCurrentView(panel);
    __gl_RenderPanel(panel);

    if ((i_obj = __gl_PickNearestObject(model, sx, sy)) != -1)
    {
        __dt_SetCurrentPickupItem(panel, (dt_index_type)i_obj);
        __gl_SetStatusInformation("Triangular unit selected: %d", i_obj);
    }
    else {
        __gl_SetStatusInformation("No object is picked up");
    }

    glutPostRedisplay();
}


/* append current correspondency selection */
void __callback_AppendCurrent(void)
{
    __dt_AppendCurrentPickupItem();
    __gl_SetStatusInformation(
        "Current selections appended to correspondence list successfully");
    glutPostRedisplay();
}


/* delete newly appended pickup item */
void __callback_Undo(void)
{
    if (!__dt_IsPickupListEmpty())
    {
        __dt_PopPickupItem();
        __gl_SetStatusInformation("Undo");
    }
    else {
        __gl_SetStatusInformation("No furture undo operations");
    }

    glutPostRedisplay();
}


/* switch render mode: solid or wired */
void __callback_SwitchRenderMode(void)
{
    __gl_SwitchRenderMode();
    __gl_SetStatusInformation("Render mode switched");
    glutPostRedisplay();
}


/* switch vertex in selected triangle unit */
void __callback_SwitchVertex(int sx, int sy)
{
    __gl_UIPanel panel = __gl_SelectPanelbyPoint(sx, sy);
    if (panel == __gl_PANEL_LEFTVIEW || panel == __gl_PANEL_RIGHTVIEW)
    {
        __dt_SwitchVertex(panel);
        __gl_SetStatusInformation(
            "Another vertex on this triangular unit is picked up");
    }
    else {
        __gl_SetStatusInformation(
            "Please move the cursor to the model viewer panel first");
    }

    glutPostRedisplay();
}


/* Write hand-picked vertex correspondeces to a .cons file */
void __callback_WriteToFile(void)
{
    extern const char *cons_filename;

    if (__dt_SaveHandPickedCorrespondences(
            cons_filename, &modelL, &modelR) == 0)
    {
        __gl_SetStatusInformation(
            "Correspondences saved to file \"%s\"",
            cons_filename);
    }
    else {
        __gl_SetStatusInformation(strerror(errno));
    }

    glutPostRedisplay();
}
