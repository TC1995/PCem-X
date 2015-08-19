#define BITMAP WINDOWS_BITMAP
#include <windows.h>
#include <windowsx.h>
#undef BITMAP

#include "ibm.h"
#include "config.h"
#include "win-deviceconfig.h"
#include "resources.h"
#include "win.h"

static device_t *config_device;

#ifndef __MINGW64__
static BOOL CALLBACK deviceconfig_dlgproc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
#else
static INT_PTR CALLBACK deviceconfig_dlgproc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
#endif
{
        switch (message)
        {
                case WM_INITDIALOG:
                {
                        int id = IDC_CONFIG_BASE;
                        device_config_t *config = config_device->config;
                        int c;

                        while (config->type != -1)
                        {
                                device_config_selection_t *selection = config->selection;
                                HWND h = GetDlgItem(hdlg, id);
                                int val_int;
                                char *val_string;
                                
                                switch (config->type)
                                {
                                        case CONFIG_BINARY:
                                        val_int = config_get_int(config_device->name, config->name, config->default_int);
                                        
                                        SendMessage(h, BM_SETCHECK, val_int, 0);
                                                
                                        id++;
                                        break;
                                     
                                        case CONFIG_SELECTION:
                                        val_int = config_get_int(config_device->name, config->name, config->default_int);
                                        
                                        c = 0;
                                        while (selection->description[0])
                                        {
                                                SendMessage(h, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)selection->description);
                                                if (val_int == selection->value)
                                                        SendMessage(h, CB_SETCURSEL, c, 0);
                                                selection++;
                                                c++;
                                        }
                                        
                                        id += 2;
                                        break;
                                }
                                config++;
                        }
                }
                return TRUE;
                
                case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                        case IDOK:
                        {
                                int id = IDC_CONFIG_BASE;
                                device_config_t *config = config_device->config;
                                int c;
                                int changed = 0;
                                
                                while (config->type != -1)
                                {
                                        device_config_selection_t *selection = config->selection;
                                        HWND h = GetDlgItem(hdlg, id);
                                        int val_int;
                                        char *val_string;
                                
                                        switch (config->type)
                                        {
                                                case CONFIG_BINARY:
                                                val_int = config_get_int(config_device->name, config->name, config->default_int);

                                                if (val_int != SendMessage(h, BM_GETCHECK, 0, 0))
                                                        changed = 1;
                                                
                                                id++;
                                                break;
                                     
                                                case CONFIG_SELECTION:
                                                val_int = config_get_int(config_device->name, config->name, config->default_int);

                                                c = SendMessage(h, CB_GETCURSEL, 0, 0);

                                                for (; c > 0; c--)
                                                        selection++;

                                                if (val_int != selection->value)
                                                        changed = 1;
                                        
                                                id += 2;
                                                break;
                                        }
                                        config++;
                                }
                                
                                if (!changed)
                                {
                                        EndDialog(hdlg, 0);
                                        return TRUE;
                                }
                                        
                                if (MessageBox(NULL, "This will reset PCem!\nOkay to continue?", "PCem", MB_OKCANCEL) != IDOK)
                                {
                                        EndDialog(hdlg, 0);
                                        return TRUE;
                                }

                                id = IDC_CONFIG_BASE;
                                config = config_device->config;
                                
                                while (config->type != -1)
                                {
                                        device_config_selection_t *selection = config->selection;
                                        HWND h = GetDlgItem(hdlg, id);
                                        int val_int;
                                        char *val_string;
                                
                                        switch (config->type)
                                        {
                                                case CONFIG_BINARY:
                                                config_set_int(config_device->name, config->name, SendMessage(h, BM_GETCHECK, 0, 0));
                                                
                                                id++;
                                                break;
                                     
                                                case CONFIG_SELECTION:
                                                c = SendMessage(h, CB_GETCURSEL, 0, 0);
                                                for (; c > 0; c--)
                                                        selection++;
                                                config_set_int(config_device->name, config->name, selection->value);
                                        
                                                id += 2;
                                                break;
                                        }
                                        config++;
                                }

                                saveconfig();
                        
                                resetpchard();

                                EndDialog(hdlg, 0);
                                return TRUE;
                        }
                        case IDCANCEL:
                        EndDialog(hdlg, 0);
                        return TRUE;
                }
                break;
        }
        return FALSE;
}

void deviceconfig_open(HWND hwnd, device_t *device)
{
        device_config_t *config = device->config;
        uint16_t *data_block = malloc(16384);
        uint16_t *data;
        DLGTEMPLATE *dlg = (DLGTEMPLATE *)data_block;
        DLGITEMTEMPLATE *item;
        int y = 10;
        int id = IDC_CONFIG_BASE;

        memset(data_block, 0, 4096);
        
        dlg->style = DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS | WS_POPUP | WS_CAPTION | WS_SYSMENU;
        dlg->x  = 10;
        dlg->y  = 10;
        dlg->cx = 150;
        dlg->cy = 70;
        
        data = (uint16_t *)(dlg + 1);
        
        *data++ = 0; /*no menu*/
        *data++ = 0; /*predefined dialog box class*/
        data += MultiByteToWideChar(CP_ACP, 0, "Device Configuration", -1, data, 50);

        *data++ = 8; /*Point*/
        data += MultiByteToWideChar(CP_ACP, 0, "MS Sans Serif", -1, data, 50);
        
#ifndef __MINGW64__
        if (((unsigned long)data) & 2)
#else
        if (((unsigned long long)data) & 2)
#endif
                data++;

        while (config->type != -1)
        {
                switch (config->type)
                {
                        case CONFIG_BINARY:
                        item = (DLGITEMTEMPLATE *)data;
                        item->x = 10;
                        item->y = y;
                        item->id = id++;
                
                        item->cx = 80;
                        item->cy = 15;

                        item->style = WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX;

                        data = (uint16_t *)(item + 1);
                        *data++ = 0xFFFF;
                        *data++ = 0x0080;    // button class

                        data += MultiByteToWideChar(CP_ACP, 0, config->description, -1, data, 256);
                        *data++ = 0;              // no creation data
                        
                        y += 20;
                        break;

                        case CONFIG_SELECTION:
                        /*Combo box*/
                        item = (DLGITEMTEMPLATE *)data;
                        item->x = 70;
                        item->y = y;
                        item->id = id++;
                
                        item->cx = 70;
                        item->cy = 150;

                        item->style = WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | WS_VSCROLL;

                        data = (uint16_t *)(item + 1);
                        *data++ = 0xFFFF;
                        *data++ = 0x0085;    // combo box class

                        data += MultiByteToWideChar(CP_ACP, 0, config->description, -1, data, 256);
                        *data++ = 0;              // no creation data
                        
#ifndef __MINGW64__
                        if (((unsigned long)data) & 2)
#else
                        if (((unsigned long long)data) & 2)
#endif
                                data++;

                        /*Static text*/
                        item = (DLGITEMTEMPLATE *)data;
                        item->x = 10;
                        item->y = y;
                        item->id = id++;
                
                        item->cx = 60;
                        item->cy = 15;

                        item->style = WS_CHILD | WS_VISIBLE;

                        data = (uint16_t *)(item + 1);
                        *data++ = 0xFFFF;
                        *data++ = 0x0082;    // static class

                        data += MultiByteToWideChar(CP_ACP, 0, config->description, -1, data, 256);
                        *data++ = 0;              // no creation data
                        
#ifndef __MINGW64__
                        if (((unsigned long)data) & 2)
#else
                        if (((unsigned long long)data) & 2)
#endif
                                data++;

                        y += 20;
                        break;
                }

#ifndef __MINGW64__
                if (((unsigned long)data) & 2)
#else
                if (((unsigned long long)data) & 2)
#endif
                        data++;

                config++;
        }

        dlg->cdit = (id - IDC_CONFIG_BASE) + 2;

//    DEFPUSHBUTTON   "OK",IDOK,64,232,50,14, WS_TABSTOP
//    PUSHBUTTON      "Cancel",IDCANCEL,128,232,50,14, WS_TABSTOP

        item = (DLGITEMTEMPLATE *)data;
        item->x = 20;
        item->y = y;
        item->cx = 50;
        item->cy = 14;
        item->id = IDOK;  // OK button identifier
        item->style = WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON;

        data = (uint16_t *)(item + 1);
        *data++ = 0xFFFF;
        *data++ = 0x0080;    // button class

        data += MultiByteToWideChar(CP_ACP, 0, "OK", -1, data, 50);
        *data++ = 0;              // no creation data

#ifndef __MINGW64__
        if (((unsigned long)data) & 2)
#else
        if (((unsigned long long)data) & 2)
#endif
		data++;
                
        item = (DLGITEMTEMPLATE *)data;
        item->x = 80;
        item->y = y;
        item->cx = 50;
        item->cy = 14;
        item->id = IDCANCEL;  // OK button identifier
        item->style = WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON;

        data = (uint16_t *)(item + 1);
        *data++ = 0xFFFF;
        *data++ = 0x0080;    // button class

        data += MultiByteToWideChar(CP_ACP, 0, "Cancel", -1, data, 50);
        *data++ = 0;              // no creation data

        dlg->cy = y + 20;
        
        config_device = device;
        
        DialogBoxIndirect(hinstance, dlg, hwnd, deviceconfig_dlgproc);

        free(data_block);
}
