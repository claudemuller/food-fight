#ifndef GFX_H_
#define GFX_H_

#include "gtk/gtk.h"

#define PALEBLUE ((Color){0xd0, 0xdf, 0xff, 0xff})
#define PALEBLUE_D ((Color){0x63, 0x75, 0x9e, 0xff})
#define PALEBLUE_DES ((Color){0xb7, 0xc2, 0xd7, 0xff})

static inline void message_box(const char* title, const char* msg)
{
#ifdef __WIN32
    MessageBoxA(NULL, "File saved successfully", "Success!", MB_OK | MB_ICONINFORMATION);
#else
    GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);

    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    while (g_main_context_iteration(NULL, FALSE))
        ;
#endif
}

#endif // !GFX_H_
