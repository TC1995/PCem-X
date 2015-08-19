#define BITMAP WINDOWS_BITMAP
#include <windows.h>
#include <windowsx.h>
#undef BITMAP

#include "ibm.h"
#include "ide.h"
#include "resources.h"
#include "win.h"

static int hd_changed = 0;

static char hd_new_name[512];
static int hd_new_spt, hd_new_hpc, hd_new_cyl;

static uint64_t cookie = 0;

#ifndef __MINGW64__
static BOOL CALLBACK hdnew_dlgproc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
#else
static INT_PTR CALLBACK hdnew_dlgproc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
#endif
{
        char s[260];
        HWND h;
        int c;
        PcemHDC hd[4];
        FILE *f;
        uint8_t buf[512];
        switch (message)
        {
                case WM_INITDIALOG:
                h = GetDlgItem(hdlg, IDC_EDIT1);
                sprintf(s, "%i", 63);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                h = GetDlgItem(hdlg, IDC_EDIT2);
                sprintf(s, "%i", 16);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                h = GetDlgItem(hdlg, IDC_EDIT3);
                sprintf(s, "%i", 511);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);

                h = GetDlgItem(hdlg, IDC_EDITC);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)"");
                
                h = GetDlgItem(hdlg, IDC_TEXT1);
                sprintf(s, "Size : %imb", (((511*16*63)*512)/1024)/1024);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);

                return TRUE;
                case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                        case IDOK:
                        h = GetDlgItem(hdlg, IDC_EDITC);
                        SendMessage(h, WM_GETTEXT, 511, (LPARAM)hd_new_name);
                        if (!hd_new_name[0])
                        {
                                MessageBox(ghwnd,"Please enter a valid file name","PCem error",MB_OK);
                                return TRUE;
                        }
                        h = GetDlgItem(hdlg, IDC_EDIT1);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd_new_spt);
                        h = GetDlgItem(hdlg, IDC_EDIT2);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd_new_hpc);
                        h = GetDlgItem(hdlg, IDC_EDIT3);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd_new_cyl);
                        
                        if (hd_new_spt > 63)
                        {
                                MessageBox(ghwnd, "Drive has too many sectors (maximum is 63)", "PCem error", MB_OK);
                                return TRUE;
                        }
                        if (hd_new_hpc > 128)
                        {
                                MessageBox(ghwnd, "Drive has too many heads (maximum is 128)", "PCem error", MB_OK);
                                return TRUE;
                        }
                        if (hd_new_cyl > 16383)
                        {
                                MessageBox(ghwnd, "Drive has too many cylinders (maximum is 16383)", "PCem error", MB_OK);
                                return TRUE;
                        }
                        
                        f = fopen64(hd_new_name, "wb");
                        if (!f)
                        {
                                MessageBox(ghwnd, "Can't open file for write", "PCem error", MB_OK);
                                return TRUE;
                        }
                        memset(buf, 0, 512);
                        for (c = 0; c < (hd_new_cyl * hd_new_hpc * hd_new_spt); c++)
                            fwrite(buf, 512, 1, f);
                        fclose(f);
                        
                        MessageBox(ghwnd, "Remember to partition and format the new drive", "PCem", MB_OK);
                        
                        EndDialog(hdlg, 1);
                        return TRUE;
                        case IDCANCEL:
                        EndDialog(hdlg, 0);
                        return TRUE;

                        case IDC_CFILE:
                        if (!getsfile(hdlg, "Hard disc image (*.IMG)\0*.IMG\0All files (*.*)\0*.*\0", ""))
                        {
                                h = GetDlgItem(hdlg, IDC_EDITC);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)openfilestring);
                        }
                        return TRUE;
                        
                        case IDC_EDIT1: case IDC_EDIT2: case IDC_EDIT3:
                        h = GetDlgItem(hdlg, IDC_EDIT1);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[0].spt);
                        h = GetDlgItem(hdlg, IDC_EDIT2);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[0].hpc);
                        h = GetDlgItem(hdlg, IDC_EDIT3);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[0].tracks);

                        h = GetDlgItem(hdlg, IDC_TEXT1);
                        sprintf(s, "Size : %imb", (((((uint64_t)hd[0].tracks*(uint64_t)hd[0].hpc)*(uint64_t)hd[0].spt)*512)/1024)/1024);
                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                        return TRUE;
                }
                break;

        }
        return FALSE;
}

#ifndef __MINGW64__
BOOL CALLBACK hdsize_dlgproc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
#else
INT_PTR CALLBACK hdsize_dlgproc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
#endif
{
        char s[260];
        HWND h;
        PcemHDC hd[4];
        switch (message)
        {
                case WM_INITDIALOG:
                h = GetDlgItem(hdlg, IDC_EDIT1);
                sprintf(s, "%i", hd_new_spt);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                h = GetDlgItem(hdlg, IDC_EDIT2);
                sprintf(s, "%i", hd_new_hpc);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                h = GetDlgItem(hdlg, IDC_EDIT3);
                sprintf(s, "%i", hd_new_cyl);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);

                h = GetDlgItem(hdlg, IDC_TEXT1);
                sprintf(s, "Size : %imb", ((((uint64_t)hd_new_spt*(uint64_t)hd_new_hpc*(uint64_t)hd_new_cyl)*512)/1024)/1024);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);

                return TRUE;
                case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                        case IDOK:
                        h = GetDlgItem(hdlg, IDC_EDIT1);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd_new_spt);
                        h = GetDlgItem(hdlg, IDC_EDIT2);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd_new_hpc);
                        h = GetDlgItem(hdlg, IDC_EDIT3);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd_new_cyl);
                        
                        if (hd_new_spt > 63)
                        {
                                MessageBox(ghwnd,"Drive has too many sectors (maximum is 63)","PCem error",MB_OK);
                                return TRUE;
                        }
                        if (hd_new_hpc > 128)
                        {
                                MessageBox(ghwnd,"Drive has too many heads (maximum is 128)","PCem error",MB_OK);
                                return TRUE;
                        }
                        if (hd_new_cyl > 16383)
                        {
                                MessageBox(ghwnd,"Drive has too many cylinders (maximum is 16383)","PCem error",MB_OK);
                                return TRUE;
                        }
                        
                        EndDialog(hdlg,1);
                        return TRUE;
                        case IDCANCEL:
                        EndDialog(hdlg,0);
                        return TRUE;

                        case IDC_EDIT1: case IDC_EDIT2: case IDC_EDIT3:
                        h = GetDlgItem(hdlg, IDC_EDIT1);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[0].spt);
                        h = GetDlgItem(hdlg, IDC_EDIT2);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[0].hpc);
                        h = GetDlgItem(hdlg, IDC_EDIT3);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[0].tracks);

                        h = GetDlgItem(hdlg, IDC_TEXT1);
                        sprintf(s, "Size : %imb", (((((uint64_t)hd[0].tracks*(uint64_t)hd[0].hpc)*(uint64_t)hd[0].spt)*512)/1024)/1024);
                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                        return TRUE;
                }
                break;

        }
        return FALSE;
}

#ifndef __MINGW64__
static BOOL CALLBACK hdconf_dlgproc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
#else
static INT_PTR CALLBACK hdconf_dlgproc(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
#endif
{
        char s[260];
        HWND h;
        PcemHDC hd[4];
        FILE *f;
        off64_t sz;
        switch (message)
        {
                case WM_INITDIALOG:
                pause = 1;
                hd[0] = hdc[0];
                hd[1] = hdc[1];
                hd[2] = hdc[2];
                hd_changed = 0;
                
                h = GetDlgItem(hdlg, IDC_EDIT_C_SPT);
                sprintf(s, "%i", hdc[0].spt);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                h = GetDlgItem(hdlg, IDC_EDIT_C_HPC);
                sprintf(s, "%i", hdc[0].hpc);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                h = GetDlgItem(hdlg, IDC_EDIT_C_CYL);
                sprintf(s, "%i", hdc[0].tracks);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                h = GetDlgItem(hdlg, IDC_EDIT_C_FN);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)ide_fn[0]);

                h = GetDlgItem(hdlg, IDC_EDIT_D_SPT);
                sprintf(s, "%i", hdc[1].spt);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                h = GetDlgItem(hdlg, IDC_EDIT_D_HPC);
                sprintf(s, "%i", hdc[1].hpc);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                h = GetDlgItem(hdlg, IDC_EDIT_D_CYL);
                sprintf(s, "%i", hdc[1].tracks);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                h=  GetDlgItem(hdlg, IDC_EDIT_D_FN);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)ide_fn[1]);

                h = GetDlgItem(hdlg, IDC_EDIT_E_SPT);
                sprintf(s, "%i", hdc[2].spt);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                h = GetDlgItem(hdlg, IDC_EDIT_E_HPC);
                sprintf(s, "%i", hdc[2].hpc);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                h = GetDlgItem(hdlg, IDC_EDIT_E_CYL);
                sprintf(s, "%i", hdc[2].tracks);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                h=  GetDlgItem(hdlg, IDC_EDIT_E_FN);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)ide_fn[2]);

                h = GetDlgItem(hdlg, IDC_EDIT_F_SPT);
                sprintf(s, "%i", hdc[3].spt);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                h = GetDlgItem(hdlg, IDC_EDIT_F_HPC);
                sprintf(s, "%i", hdc[3].hpc);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                h = GetDlgItem(hdlg, IDC_EDIT_F_CYL);
                sprintf(s, "%i", hdc[3].tracks);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                h=  GetDlgItem(hdlg, IDC_EDIT_F_FN);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)ide_fn[3]);
                
                h = GetDlgItem(hdlg, IDC_TEXT_C_SIZE);
                sprintf(s, "Size : %imb", (((((uint64_t)hd[0].tracks*(uint64_t)hd[0].hpc)*(uint64_t)hd[0].spt)*512)/1024)/1024);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);

                h = GetDlgItem(hdlg, IDC_TEXT_D_SIZE);
                sprintf(s, "Size : %imb", (((((uint64_t)hd[1].tracks*(uint64_t)hd[1].hpc)*(uint64_t)hd[1].spt)*512)/1024)/1024);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                return TRUE;

                h = GetDlgItem(hdlg, IDC_TEXT_E_SIZE);
                sprintf(s, "Size : %imb", (((((uint64_t)hd[2].tracks*(uint64_t)hd[2].hpc)*(uint64_t)hd[2].spt)*512)/1024)/1024);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                return TRUE;

                h = GetDlgItem(hdlg, IDC_TEXT_F_SIZE);
                sprintf(s, "Size : %imb", (((((uint64_t)hd[3].tracks*(uint64_t)hd[3].hpc)*(uint64_t)hd[3].spt)*512)/1024)/1024);
                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                return TRUE;
                
                case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                        case IDOK:
                        if (hd_changed)
                        {                     
                                if (MessageBox(NULL, "This will reset PCem!\nOkay to continue?", "PCem", MB_OKCANCEL) == IDOK)
                                {
                                        h = GetDlgItem(hdlg, IDC_EDIT_C_SPT);
                                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                                        sscanf(s, "%i", &hd[0].spt);
                                        h = GetDlgItem(hdlg, IDC_EDIT_C_HPC);
                                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                                        sscanf(s, "%i", &hd[0].hpc);
                                        h = GetDlgItem(hdlg, IDC_EDIT_C_CYL);
                                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                                        sscanf(s, "%i", &hd[0].tracks);
                                        h = GetDlgItem(hdlg, IDC_EDIT_C_FN);
                                        SendMessage(h, WM_GETTEXT, 511, (LPARAM)ide_fn[0]);

                                        h = GetDlgItem(hdlg, IDC_EDIT_D_SPT);
                                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                                        sscanf(s, "%i", &hd[1].spt);
                                        h = GetDlgItem(hdlg, IDC_EDIT_D_HPC);
                                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                                        sscanf(s, "%i", &hd[1].hpc);
                                        h = GetDlgItem(hdlg, IDC_EDIT_D_CYL);
                                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                                        sscanf(s, "%i", &hd[1].tracks);
                                        h = GetDlgItem(hdlg, IDC_EDIT_D_FN);
                                        SendMessage(h, WM_GETTEXT, 511, (LPARAM)ide_fn[1]);

                                        h = GetDlgItem(hdlg, IDC_EDIT_E_SPT);
                                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                                        sscanf(s, "%i", &hd[2].spt);
                                        h = GetDlgItem(hdlg, IDC_EDIT_E_HPC);
                                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                                        sscanf(s, "%i", &hd[2].hpc);
                                        h = GetDlgItem(hdlg, IDC_EDIT_E_CYL);
                                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                                        sscanf(s, "%i", &hd[2].tracks);
                                        h = GetDlgItem(hdlg, IDC_EDIT_E_FN);
                                        SendMessage(h, WM_GETTEXT, 511, (LPARAM)ide_fn[2]);

                                        h = GetDlgItem(hdlg, IDC_EDIT_F_SPT);
                                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                                        sscanf(s, "%i", &hd[3].spt);
                                        h = GetDlgItem(hdlg, IDC_EDIT_F_HPC);
                                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                                        sscanf(s, "%i", &hd[3].hpc);
                                        h = GetDlgItem(hdlg, IDC_EDIT_F_CYL);
                                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                                        sscanf(s, "%i", &hd[3].tracks);
                                        h = GetDlgItem(hdlg, IDC_EDIT_F_FN);
                                        SendMessage(h, WM_GETTEXT, 511, (LPARAM)ide_fn[3]);
                                        
                                        hdc[0] = hd[0];
                                        hdc[1] = hd[1];
                                        hdc[2] = hd[2];
                                        hdc[3] = hd[3];

                                        saveconfig();
                                                                                
                                        resetpchard();
                                }                                
                        }
                        case IDCANCEL:
                        EndDialog(hdlg, 0);
                        pause = 0;
                        return TRUE;

                        case IDC_EJECTC:
                        hd[0].spt = 0;
                        hd[0].hpc = 0;
                        hd[0].tracks = 0;
                        ide_fn[0][0] = 0;
                        SetDlgItemText(hdlg, IDC_EDIT_C_SPT, "0");
                        SetDlgItemText(hdlg, IDC_EDIT_C_HPC, "0");
                        SetDlgItemText(hdlg, IDC_EDIT_C_CYL, "0");
                        SetDlgItemText(hdlg, IDC_EDIT_C_FN, "");
                        hd_changed = 1;
                        return TRUE;

                        case IDC_EJECTD:
                        hd[1].spt = 0;
                        hd[1].hpc = 0;
                        hd[1].tracks = 0;
                        ide_fn[1][0] = 0;
                        SetDlgItemText(hdlg, IDC_EDIT_D_SPT, "0");
                        SetDlgItemText(hdlg, IDC_EDIT_D_HPC, "0");
                        SetDlgItemText(hdlg, IDC_EDIT_D_CYL, "0");
                        SetDlgItemText(hdlg, IDC_EDIT_D_FN, "");
                        hd_changed = 1;
                        return TRUE;

                        case IDC_EJECTE:
                        hd[2].spt = 0;
                        hd[2].hpc = 0;
                        hd[2].tracks = 0;
                        ide_fn[2][0] = 0;
                        SetDlgItemText(hdlg, IDC_EDIT_E_SPT, "0");
                        SetDlgItemText(hdlg, IDC_EDIT_E_HPC, "0");
                        SetDlgItemText(hdlg, IDC_EDIT_E_CYL, "0");
                        SetDlgItemText(hdlg, IDC_EDIT_E_FN, "");
                        hd_changed = 1;
                        return TRUE;

                        case IDC_EJECTF:
                        hd[3].spt = 0;
                        hd[3].hpc = 0;
                        hd[3].tracks = 0;
                        ide_fn[3][0] = 0;
                        SetDlgItemText(hdlg, IDC_EDIT_F_SPT, "0");
                        SetDlgItemText(hdlg, IDC_EDIT_F_HPC, "0");
                        SetDlgItemText(hdlg, IDC_EDIT_F_CYL, "0");
                        SetDlgItemText(hdlg, IDC_EDIT_F_FN, "");
                        hd_changed = 1;
                        return TRUE;
                        
                        case IDC_CNEW:
                        if (DialogBox(hinstance, TEXT("HdNewDlg"), hdlg, hdnew_dlgproc) == 1)
                        {
                                h = GetDlgItem(hdlg, IDC_EDIT_C_SPT);
                                sprintf(s, "%i", hd_new_spt);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                h = GetDlgItem(hdlg, IDC_EDIT_C_HPC);
                                sprintf(s, "%i", hd_new_hpc);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                h = GetDlgItem(hdlg, IDC_EDIT_C_CYL);
                                sprintf(s, "%i", hd_new_cyl);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                h = GetDlgItem(hdlg, IDC_EDIT_C_FN);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)hd_new_name);

                                h = GetDlgItem(hdlg, IDC_TEXT_C_SIZE);
                                sprintf(s, "Size : %imb", (((((uint64_t)hd_new_cyl*(uint64_t)hd_new_hpc)*(uint64_t)hd_new_spt)*512)/1024)/1024);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);

                                hd_changed = 1;
                        }                              
                        return TRUE;
                        
                        case IDC_CFILE:
                        if (!getfile(hdlg, "Hard disk image (*.IMG;*.IMA;*.VHD)\0*.IMG;*.IMA;*.VHD\0All files (*.*)\0*.*\0", ""))
                        {
                                f = fopen64(openfilestring, "rb");
                                if (!f)
                                {
                                        MessageBox(ghwnd,"Can't open file for read","PCem error",MB_OK);
                                        return TRUE;
                                }
                                fseeko64(f, -1, SEEK_END);
                                sz = ftello64(f) + 1;
				fseeko64(f, -512, SEEK_END);
				fread(&cookie, 1, 8, f);
				// VPC 2004+ VHD
				if (cookie == 0x78697463656E6F63)  sz -= 512;
				// Connectix VPC VHD
				if ((cookie & 0xFFFFFFFFFFFFFF00) == 0x697463656E6F6300)  sz -= 511;
                                fclose(f);
                                hd_new_spt = 63;
                                hd_new_hpc = 16;
                                hd_new_cyl = ((sz / 512) / 16) / 63;
                                
                                if (DialogBox(hinstance, TEXT("HdSizeDlg"), hdlg, hdsize_dlgproc) == 1)
                                {
                                        h = GetDlgItem(hdlg, IDC_EDIT_C_SPT);
                                        sprintf(s, "%i", hd_new_spt);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                        h = GetDlgItem(hdlg, IDC_EDIT_C_HPC);
                                        sprintf(s, "%i", hd_new_hpc);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                        h = GetDlgItem(hdlg, IDC_EDIT_C_CYL);
                                        sprintf(s, "%i", hd_new_cyl);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                        h = GetDlgItem(hdlg, IDC_EDIT_C_FN);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)openfilestring);

                                        h=  GetDlgItem(hdlg, IDC_TEXT_C_SIZE);
                                        sprintf(s, "Size : %imb", (((((uint64_t)hd_new_cyl*(uint64_t)hd_new_hpc)*(uint64_t)hd_new_spt)*512)/1024)/1024);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
        
                                        hd_changed = 1;
                                }
                        }
                        return TRUE;
                                
                        case IDC_DNEW:
                        if (DialogBox(hinstance, TEXT("HdNewDlg"), hdlg, hdnew_dlgproc) == 1)
                        {
                                h = GetDlgItem(hdlg, IDC_EDIT_D_SPT);
                                sprintf(s, "%i", hd_new_spt);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                h = GetDlgItem(hdlg, IDC_EDIT_D_HPC);
                                sprintf(s, "%i", hd_new_hpc);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                h = GetDlgItem(hdlg, IDC_EDIT_D_CYL);
                                sprintf(s, "%i", hd_new_cyl);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                h = GetDlgItem(hdlg, IDC_EDIT_D_FN);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)hd_new_name);

                                h=  GetDlgItem(hdlg, IDC_TEXT_D_SIZE);
                                sprintf(s, "Size : %imb", (((((uint64_t)hd_new_cyl*(uint64_t)hd_new_hpc)*(uint64_t)hd_new_spt)*512)/1024)/1024);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);

                                hd_changed = 1;
                        }                              
                        return TRUE;
                        
                        case IDC_DFILE:
                        if (!getfile(hdlg, "Hard disk image (*.IMG;*.IMA;*.VHD)\0*.IMG;*.IMA;*.VHD\0All files (*.*)\0*.*\0", ""))
                        {
                                f = fopen64(openfilestring, "rb");
                                if (!f)
                                {
                                        MessageBox(ghwnd,"Can't open file for read","PCem error",MB_OK);
                                        return TRUE;
                                }
                                fseeko64(f, -1, SEEK_END);
                                sz = ftello64(f) + 1;
				fseeko64(f, -512, SEEK_END);
				fread(&cookie, 1, 8, f);
				// VPC 2004+ VHD
				if (cookie == 0x78697463656E6F63)  sz -= 512;
				// Connectix VPC VHD
				if ((cookie & 0xFFFFFFFFFFFFFF00) == 0x697463656E6F6300)  sz -= 511;
                                fclose(f);
                                hd_new_spt = 63;
                                hd_new_hpc = 16;
                                hd_new_cyl = ((sz / 512) / 16) / 63;
                                
                                if (DialogBox(hinstance, TEXT("HdSizeDlg"), hdlg, hdsize_dlgproc) == 1)
                                {
                                        h = GetDlgItem(hdlg, IDC_EDIT_D_SPT);
                                        sprintf(s, "%i", hd_new_spt);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                        h = GetDlgItem(hdlg, IDC_EDIT_D_HPC);
                                        sprintf(s, "%i", hd_new_hpc);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                        h = GetDlgItem(hdlg, IDC_EDIT_D_CYL);
                                        sprintf(s, "%i", hd_new_cyl);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                        h = GetDlgItem(hdlg, IDC_EDIT_D_FN);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)openfilestring);

                                        h = GetDlgItem(hdlg, IDC_TEXT_D_SIZE);
                                        sprintf(s, "Size : %imb", (((((uint64_t)hd_new_cyl*(uint64_t)hd_new_hpc)*(uint64_t)hd_new_spt)*512)/1024)/1024);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
        
                                        hd_changed = 1;
                                }
                        }
                        return TRUE;

                        case IDC_ENEW:
                        if (DialogBox(hinstance, TEXT("HdNewDlg"), hdlg, hdnew_dlgproc) == 1)
                        {
                                h = GetDlgItem(hdlg, IDC_EDIT_E_SPT);
                                sprintf(s, "%i", hd_new_spt);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                h = GetDlgItem(hdlg, IDC_EDIT_E_HPC);
                                sprintf(s, "%i", hd_new_hpc);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                h = GetDlgItem(hdlg, IDC_EDIT_E_CYL);
                                sprintf(s, "%i", hd_new_cyl);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                h = GetDlgItem(hdlg, IDC_EDIT_E_FN);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)hd_new_name);

                                h=  GetDlgItem(hdlg, IDC_TEXT_E_SIZE);
                                sprintf(s, "Size : %imb", (((((uint64_t)hd_new_cyl*(uint64_t)hd_new_hpc)*(uint64_t)hd_new_spt)*512)/1024)/1024);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);

                                hd_changed = 1;
                        }                              
                        return TRUE;
                        
                        case IDC_EFILE:
                        if (!getfile(hdlg, "Hard disk image (*.IMG;*.IMA;*.VHD)\0*.IMG;*.IMA;*.VHD\0All files (*.*)\0*.*\0", ""))
                        {
                                f = fopen64(openfilestring, "rb");
                                if (!f)
                                {
                                        MessageBox(ghwnd,"Can't open file for read","PCem error",MB_OK);
                                        return TRUE;
                                }
                                fseeko64(f, -1, SEEK_END);
                                sz = ftello64(f) + 1;
				fseeko64(f, -512, SEEK_END);
				fread(&cookie, 1, 8, f);
				// VPC 2004+ VHD
				if (cookie == 0x78697463656E6F63)  sz -= 512;
				// Connectix VPC VHD
				if ((cookie & 0xFFFFFFFFFFFFFF00) == 0x697463656E6F6300)  sz -= 511;
                                fclose(f);
                                hd_new_spt = 63;
                                hd_new_hpc = 16;
                                hd_new_cyl = ((sz / 512) / 16) / 63;
                                
                                if (DialogBox(hinstance, TEXT("HdSizeDlg"), hdlg, hdsize_dlgproc) == 1)
                                {
                                        h = GetDlgItem(hdlg, IDC_EDIT_E_SPT);
                                        sprintf(s, "%i", hd_new_spt);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                        h = GetDlgItem(hdlg, IDC_EDIT_E_HPC);
                                        sprintf(s, "%i", hd_new_hpc);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                        h = GetDlgItem(hdlg, IDC_EDIT_E_CYL);
                                        sprintf(s, "%i", hd_new_cyl);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                        h = GetDlgItem(hdlg, IDC_EDIT_E_FN);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)openfilestring);

                                        h = GetDlgItem(hdlg, IDC_TEXT_E_SIZE);
                                        sprintf(s, "Size : %imb", (((((uint64_t)hd_new_cyl*(uint64_t)hd_new_hpc)*(uint64_t)hd_new_spt)*512)/1024)/1024);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
        
                                        hd_changed = 1;
                                }
                        }
                        return TRUE;

                        case IDC_FNEW:
                        if (DialogBox(hinstance, TEXT("HdNewDlg"), hdlg, hdnew_dlgproc) == 1)
                        {
                                h = GetDlgItem(hdlg, IDC_EDIT_F_SPT);
                                sprintf(s, "%i", hd_new_spt);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                h = GetDlgItem(hdlg, IDC_EDIT_F_HPC);
                                sprintf(s, "%i", hd_new_hpc);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                h = GetDlgItem(hdlg, IDC_EDIT_F_CYL);
                                sprintf(s, "%i", hd_new_cyl);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                h = GetDlgItem(hdlg, IDC_EDIT_F_FN);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)hd_new_name);

                                h=  GetDlgItem(hdlg, IDC_TEXT_F_SIZE);
                                sprintf(s, "Size : %imb", (((((uint64_t)hd_new_cyl*(uint64_t)hd_new_hpc)*(uint64_t)hd_new_spt)*512)/1024)/1024);
                                SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);

                                hd_changed = 1;
                        }                              
                        return TRUE;
                        
                        case IDC_FFILE:
                        if (!getfile(hdlg, "Hard disk image (*.IMG;*.IMA;*.VHD)\0*.IMG;*.IMA;*.VHD\0All files (*.*)\0*.*\0", ""))
                        {
                                f = fopen64(openfilestring, "rb");
                                if (!f)
                                {
                                        MessageBox(ghwnd,"Can't open file for read","PCem error",MB_OK);
                                        return TRUE;
                                }
                                fseeko64(f, -1, SEEK_END);
                                sz = ftello64(f) + 1;
				fseeko64(f, -512, SEEK_END);
				fread(&cookie, 1, 8, f);
				// VPC 2004+ VHD
				if (cookie == 0x78697463656E6F63)  sz -= 512;
				// Connectix VPC VHD
				if ((cookie & 0xFFFFFFFFFFFFFF00) == 0x697463656E6F6300)  sz -= 511;
                                fclose(f);
                                hd_new_spt = 63;
                                hd_new_hpc = 16;
                                hd_new_cyl = ((sz / 512) / 16) / 63;
                                
                                if (DialogBox(hinstance, TEXT("HdSizeDlg"), hdlg, hdsize_dlgproc) == 1)
                                {
                                        h = GetDlgItem(hdlg, IDC_EDIT_F_SPT);
                                        sprintf(s, "%i", hd_new_spt);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                        h = GetDlgItem(hdlg, IDC_EDIT_F_HPC);
                                        sprintf(s, "%i", hd_new_hpc);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                        h = GetDlgItem(hdlg, IDC_EDIT_F_CYL);
                                        sprintf(s, "%i", hd_new_cyl);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                                        h = GetDlgItem(hdlg, IDC_EDIT_F_FN);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)openfilestring);

                                        h = GetDlgItem(hdlg, IDC_TEXT_F_SIZE);
                                        sprintf(s, "Size : %imb", (((((uint64_t)hd_new_cyl*(uint64_t)hd_new_hpc)*(uint64_t)hd_new_spt)*512)/1024)/1024);
                                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
        
                                        hd_changed = 1;
                                }
                        }
                        return TRUE;

                        case IDC_EDIT_C_SPT: case IDC_EDIT_C_HPC: case IDC_EDIT_C_CYL:
                        h = GetDlgItem(hdlg, IDC_EDIT_C_SPT);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[0].spt);
                        h = GetDlgItem(hdlg, IDC_EDIT_C_HPC);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[0].hpc);
                        h = GetDlgItem(hdlg, IDC_EDIT_C_CYL);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[0].tracks);

                        h = GetDlgItem(hdlg, IDC_TEXT_C_SIZE);
                        sprintf(s, "Size : %imb", ((((hd[0].tracks*hd[0].hpc)*hd[0].spt)*512)/1024)/1024);
                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                        return TRUE;

                        case IDC_EDIT_D_SPT: case IDC_EDIT_D_HPC: case IDC_EDIT_D_CYL:
                        h = GetDlgItem(hdlg, IDC_EDIT_D_SPT);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[1].spt);
                        h = GetDlgItem(hdlg, IDC_EDIT_D_HPC);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[1].hpc);
                        h = GetDlgItem(hdlg, IDC_EDIT_D_CYL);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[1].tracks);

                        h = GetDlgItem(hdlg, IDC_TEXT_D_SIZE);
                        sprintf(s, "Size : %imb", ((((hd[1].tracks*hd[1].hpc)*hd[1].spt)*512)/1024)/1024);
                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                        return TRUE;

                        case IDC_EDIT_E_SPT: case IDC_EDIT_E_HPC: case IDC_EDIT_E_CYL:
                        h = GetDlgItem(hdlg, IDC_EDIT_E_SPT);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[2].spt);
                        h = GetDlgItem(hdlg, IDC_EDIT_E_HPC);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[2].hpc);
                        h = GetDlgItem(hdlg, IDC_EDIT_E_CYL);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[2].tracks);

                        h = GetDlgItem(hdlg, IDC_TEXT_E_SIZE);
                        sprintf(s, "Size : %imb", ((((hd[2].tracks*hd[2].hpc)*hd[2].spt)*512)/1024)/1024);
                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                        return TRUE;

                        case IDC_EDIT_F_SPT: case IDC_EDIT_F_HPC: case IDC_EDIT_F_CYL:
                        h = GetDlgItem(hdlg, IDC_EDIT_F_SPT);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[3].spt);
                        h = GetDlgItem(hdlg, IDC_EDIT_F_HPC);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[3].hpc);
                        h = GetDlgItem(hdlg, IDC_EDIT_F_CYL);
                        SendMessage(h, WM_GETTEXT, 255, (LPARAM)s);
                        sscanf(s, "%i", &hd[3].tracks);

                        h = GetDlgItem(hdlg, IDC_TEXT_F_SIZE);
                        sprintf(s, "Size : %imb", ((((hd[3].tracks*hd[3].hpc)*hd[3].spt)*512)/1024)/1024);
                        SendMessage(h, WM_SETTEXT, 0, (LPARAM)s);
                        return TRUE;
                }
                break;

        }
        return FALSE;
}

void hdconf_open(HWND hwnd)
{
        DialogBox(hinstance, TEXT("HdConfDlg"), hwnd, hdconf_dlgproc);
}        
