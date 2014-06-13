/********************************************************************************
 Copyright (C) 2014 Append Huang <Append@gmail.com>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
********************************************************************************/


//#include "Main.h"
//#include "OBSApi.h"
#include "NicoCommentPlugin.h"
extern "C" __declspec(dllexport) bool LoadPlugin();
extern "C" __declspec(dllexport) void UnloadPlugin();
extern "C" __declspec(dllexport) CTSTR GetPluginName();
extern "C" __declspec(dllexport) CTSTR GetPluginDescription();
extern "C" __declspec(dllexport) void ConfigPlugin(HWND);

HINSTANCE hinstMain = NULL;

ImageSource* STDCALL CreateTextSource(XElement *data)
{
    if(!data)
        return NULL;

    return new NicoCommentPlugin(data);
}

int CALLBACK FontEnumProcThingy(ENUMLOGFONTEX *logicalData, NEWTEXTMETRICEX *physicalData, DWORD fontType, ConfigTextSourceInfo *configInfo)
{
    if(fontType == TRUETYPE_FONTTYPE) //HomeWorld - GDI+ doesn't like anything other than truetype
    {
        configInfo->fontNames << logicalData->elfFullName;
        configInfo->fontFaces << logicalData->elfLogFont.lfFaceName;
    }

    return 1;
}

void DoCancelStuff(HWND hwnd)
{
    ConfigTextSourceInfo *configInfo = (ConfigTextSourceInfo*)GetWindowLongPtr(hwnd, DWLP_USER);
    ImageSource *source = API->GetSceneImageSource(configInfo->lpName);
    //XElement *data = configInfo->data;

    if(source)
        source->UpdateSettings();
}

UINT FindFontFace(ConfigTextSourceInfo *configInfo, HWND hwndFontList, CTSTR lpFontFace)
{
    UINT id = configInfo->fontFaces.FindValueIndexI(lpFontFace);
    if(id == INVALID)
        return INVALID;
    else
    {
        for(UINT i=0; i<configInfo->fontFaces.Num(); i++)
        {
            UINT targetID = (UINT)SendMessage(hwndFontList, CB_GETITEMDATA, i, 0);
            if(targetID == id)
                return i;
        }
    }

    return INVALID;
}

UINT FindFontName(ConfigTextSourceInfo *configInfo, HWND hwndFontList, CTSTR lpFontFace)
{
    return configInfo->fontNames.FindValueIndexI(lpFontFace);
}

CTSTR GetFontFace(ConfigTextSourceInfo *configInfo, HWND hwndFontList)
{
    UINT id = (UINT)SendMessage(hwndFontList, CB_GETCURSEL, 0, 0);
    if(id == CB_ERR)
        return NULL;

    UINT actualID = (UINT)SendMessage(hwndFontList, CB_GETITEMDATA, id, 0);
    return configInfo->fontFaces[actualID];
}

INT_PTR CALLBACK ConfigureTextProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool bInitializedDialog = false;

    switch(message)
    {
        case WM_INITDIALOG:
            {
                ConfigTextSourceInfo *configInfo = (ConfigTextSourceInfo*)lParam;
                SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)configInfo);
                LocalizeWindow(hwnd);

                XElement *data = configInfo->data;

                //-----------------------------------------

                HDC hDCtest = GetDC(hwnd);

                LOGFONT lf;
                zero(&lf, sizeof(lf));
                EnumFontFamiliesEx(hDCtest, &lf, (FONTENUMPROC)FontEnumProcThingy, (LPARAM)configInfo, 0);

                HWND hwndFonts = GetDlgItem(hwnd, IDC_FONT);
                for(UINT i=0; i<configInfo->fontNames.Num(); i++)
                {
                    int id = (int)SendMessage(hwndFonts, CB_ADDSTRING, 0, (LPARAM)configInfo->fontNames[i].Array());
                    SendMessage(hwndFonts, CB_SETITEMDATA, id, (LPARAM)i);
                }

                CTSTR lpFont = data->GetString(TEXT("font"));
                UINT id = FindFontFace(configInfo, hwndFonts, lpFont);
                if(id == INVALID)
                    id = (UINT)SendMessage(hwndFonts, CB_FINDSTRINGEXACT, -1, (LPARAM)TEXT("Arial"));

                SendMessage(hwndFonts, CB_SETCURSEL, id, 0);

                //-----------------------------------------

                SendMessage(GetDlgItem(hwnd, IDC_TEXTSIZE), UDM_SETRANGE32, 5, 2048);
                SendMessage(GetDlgItem(hwnd, IDC_TEXTSIZE), UDM_SETPOS32, 0, data->GetInt(TEXT("fontSize"), 48));

                //-----------------------------------------

                CCSetColor(GetDlgItem(hwnd, IDC_COLOR), data->GetInt(TEXT("color"), 0xFFFFFFFF));

                SendMessage(GetDlgItem(hwnd, IDC_TEXTOPACITY), UDM_SETRANGE32, 0, 100);
                SendMessage(GetDlgItem(hwnd, IDC_TEXTOPACITY), UDM_SETPOS32, 0, data->GetInt(TEXT("textOpacity"), 100));

                //SendMessage(GetDlgItem(hwnd, IDC_SCROLLSPEED), UDM_SETRANGE32, -4095, 4095);
				SendMessage(GetDlgItem(hwnd, IDC_SCROLLSPEED), UDM_SETRANGE32, 5, 100); //only positive
                SendMessage(GetDlgItem(hwnd, IDC_SCROLLSPEED), UDM_SETPOS32, 0, data->GetInt(TEXT("scrollSpeed"), 10));

                SendMessage(GetDlgItem(hwnd, IDC_BOLD), BM_SETCHECK, data->GetInt(TEXT("bold"), 0) ? BST_CHECKED : BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hwnd, IDC_ITALIC), BM_SETCHECK, data->GetInt(TEXT("italic"), 0) ? BST_CHECKED : BST_UNCHECKED, 0);
                SendMessage(GetDlgItem(hwnd, IDC_UNDERLINE), BM_SETCHECK, data->GetInt(TEXT("underline"), 0) ? BST_CHECKED : BST_UNCHECKED, 0);

                BOOL bUsePointFilter = data->GetInt(TEXT("pointFiltering"), 0) != 0;
                SendMessage(GetDlgItem(hwnd, IDC_POINTFILTERING), BM_SETCHECK, bUsePointFilter ? BST_CHECKED : BST_UNCHECKED, 0);

                //-----------------------------------------

                CCSetColor(GetDlgItem(hwnd, IDC_BACKGROUNDCOLOR), data->GetInt(TEXT("backgroundColor"), 0xFF000000));

                SendMessage(GetDlgItem(hwnd, IDC_BACKGROUNDOPACITY), UDM_SETRANGE32, 0, 100);
                SendMessage(GetDlgItem(hwnd, IDC_BACKGROUNDOPACITY), UDM_SETPOS32, 0, data->GetInt(TEXT("backgroundOpacity"), 0));

                //-----------------------------------------

                bool bChecked = data->GetInt(TEXT("useOutline"), 0) != 0;
                SendMessage(GetDlgItem(hwnd, IDC_USEOUTLINE), BM_SETCHECK, bChecked ? BST_CHECKED : BST_UNCHECKED, 0);

                EnableWindow(GetDlgItem(hwnd, IDC_OUTLINETHICKNESS_EDIT), bChecked);
                EnableWindow(GetDlgItem(hwnd, IDC_OUTLINETHICKNESS), bChecked);
                EnableWindow(GetDlgItem(hwnd, IDC_OUTLINECOLOR), bChecked);
                EnableWindow(GetDlgItem(hwnd, IDC_OUTLINEOPACITY_EDIT), bChecked);
                EnableWindow(GetDlgItem(hwnd, IDC_OUTLINEOPACITY), bChecked);

                SendMessage(GetDlgItem(hwnd, IDC_OUTLINETHICKNESS), UDM_SETRANGE32, 1, 20);
                SendMessage(GetDlgItem(hwnd, IDC_OUTLINETHICKNESS), UDM_SETPOS32, 0, data->GetInt(TEXT("outlineSize"), 2));

                CCSetColor(GetDlgItem(hwnd, IDC_OUTLINECOLOR), data->GetInt(TEXT("outlineColor"), 0xFF000000));

                SendMessage(GetDlgItem(hwnd, IDC_OUTLINEOPACITY), UDM_SETRANGE32, 0, 100);
                SendMessage(GetDlgItem(hwnd, IDC_OUTLINEOPACITY), UDM_SETPOS32, 0, data->GetInt(TEXT("outlineOpacity"), 100));

                //-----------------------------------------
				UINT basewidth=0;
				UINT baseheight=0;
				OBSGetBaseSize(basewidth, baseheight);
                SendMessage(GetDlgItem(hwnd, IDC_EXTENTWIDTH),  UDM_SETRANGE32, 32, 2048);
                SendMessage(GetDlgItem(hwnd, IDC_EXTENTHEIGHT), UDM_SETRANGE32, 32, 2048);
                SendMessage(GetDlgItem(hwnd, IDC_EXTENTWIDTH),  UDM_SETPOS32, 0, data->GetInt(TEXT("extentWidth"),  basewidth));
                SendMessage(GetDlgItem(hwnd, IDC_EXTENTHEIGHT), UDM_SETPOS32, 0, data->GetInt(TEXT("extentHeight"), baseheight));

                //-----------------------------------------
                SendMessage(GetDlgItem(hwnd, IDC_NUMOFLINES), UDM_SETRANGE32, 3, 10);
                SendMessage(GetDlgItem(hwnd, IDC_NUMOFLINES),  UDM_SETPOS32, 0, data->GetInt(TEXT("NumOfLines"), 5));

				//-----------------------------------------
                HWND hwndIRCServer = GetDlgItem(hwnd, IDC_IRCSERVER);
                SendMessage(hwndIRCServer, CB_ADDSTRING, 0, (LPARAM)L"Twitch (irc.twitch.tv)");
                SendMessage(hwndIRCServer, CB_ADDSTRING, 0, (LPARAM)L"Justin.tv (irc.justin.tv)");
                int iServer = data->GetInt(TEXT("iServer"), 0);
                ClampVal(iServer, 0, 1);
                SendMessage(hwndIRCServer, CB_SETCURSEL, iServer, 0);

                HWND hwndIRCPort = GetDlgItem(hwnd, IDC_PORT);
                SendMessage(hwndIRCPort, CB_ADDSTRING, 0, (LPARAM)L"443");
                SendMessage(hwndIRCPort, CB_ADDSTRING, 0, (LPARAM)L"6667");
				SendMessage(hwndIRCPort, CB_ADDSTRING, 0, (LPARAM)L"80");
                int iPort = data->GetInt(TEXT("iPort"), 0);
                ClampVal(iServer, 0, 2);
                SendMessage(hwndIRCPort, CB_SETCURSEL, iPort, 0);

				SetWindowText(GetDlgItem(hwnd, IDC_NICKNAME), data->GetString(TEXT("Nickname")));
				SetWindowText(GetDlgItem(hwnd, IDC_PASSWORD), data->GetString(TEXT("Password")));
				SetWindowText(GetDlgItem(hwnd, IDC_CHANNEL), data->GetString(TEXT("Channel")));

                bInitializedDialog = true;

                return TRUE;
            }

        case WM_DESTROY:
            bInitializedDialog = false;
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {	//FONT ONLY
                case IDC_FONT:
                    if(bInitializedDialog)
                    {
                        if(HIWORD(wParam) == CBN_SELCHANGE || HIWORD(wParam) == CBN_EDITCHANGE)
                        {
                            ConfigTextSourceInfo *configInfo = (ConfigTextSourceInfo*)GetWindowLongPtr(hwnd, DWLP_USER);
                            if(!configInfo) break;

                            String strFont;
                            if(HIWORD(wParam) == CBN_SELCHANGE)
                                strFont = GetFontFace(configInfo, (HWND)lParam);
                            else
                            {
                                UINT id = FindFontName(configInfo, (HWND)lParam, GetEditText((HWND)lParam));
                                if(id != INVALID)
                                    strFont = configInfo->fontFaces[id];
                            }

                            ImageSource *source = API->GetSceneImageSource(configInfo->lpName);
                            if(source && strFont.IsValid())
                                source->SetString(TEXT("font"), strFont);
                        }
                    }
                    break;
				//COLOR SELECTION
                case IDC_OUTLINECOLOR:
                case IDC_BACKGROUNDCOLOR:
                case IDC_COLOR:
                    if(bInitializedDialog)
                    {
                        DWORD color = CCGetColor((HWND)lParam);

                        ConfigTextSourceInfo *configInfo = (ConfigTextSourceInfo*)GetWindowLongPtr(hwnd, DWLP_USER);
                        if(!configInfo) break;
                        ImageSource *source = API->GetSceneImageSource(configInfo->lpName);
                        if(source)
                        {
                            switch(LOWORD(wParam))
                            {
                                case IDC_OUTLINECOLOR:      source->SetInt(TEXT("outlineColor"), color); break;
                                case IDC_BACKGROUNDCOLOR:   source->SetInt(TEXT("backgroundColor"), color); break;
                                case IDC_COLOR:             source->SetInt(TEXT("color"), color); break;
                            }
                        }
                    }
                    break;
				//TEXT EDIT
                case IDC_TEXTSIZE_EDIT:
//                case IDC_EXTENTWIDTH_EDIT:
//                case IDC_EXTENTHEIGHT_EDIT:
                case IDC_BACKGROUNDOPACITY_EDIT:
                case IDC_TEXTOPACITY_EDIT:
                case IDC_OUTLINEOPACITY_EDIT:
                case IDC_OUTLINETHICKNESS_EDIT:
                case IDC_SCROLLSPEED_EDIT:
				case IDC_NUMOFLINES_EDIT:
                    if(HIWORD(wParam) == EN_CHANGE && bInitializedDialog)
                    {
                        int val = (int)SendMessage(GetWindow((HWND)lParam, GW_HWNDNEXT), UDM_GETPOS32, 0, 0);

                        ConfigTextSourceInfo *configInfo = (ConfigTextSourceInfo*)GetWindowLongPtr(hwnd, DWLP_USER);
                        if(!configInfo) break;

                        ImageSource *source = API->GetSceneImageSource(configInfo->lpName);
                        if(source)
                        {
                            switch(LOWORD(wParam))
                            {
                                case IDC_TEXTSIZE_EDIT:             source->SetInt(TEXT("fontSize"), val); break;
                                case IDC_EXTENTWIDTH_EDIT:          source->SetInt(TEXT("extentWidth"), val); break;
                                case IDC_EXTENTHEIGHT_EDIT:         source->SetInt(TEXT("extentHeight"), val); break;
                                case IDC_TEXTOPACITY_EDIT:          source->SetInt(TEXT("textOpacity"), val); break;
                                case IDC_OUTLINEOPACITY_EDIT:       source->SetInt(TEXT("outlineOpacity"), val); break;
                                case IDC_BACKGROUNDOPACITY_EDIT:    source->SetInt(TEXT("backgroundOpacity"), val); break;
                                case IDC_OUTLINETHICKNESS_EDIT:     source->SetFloat(TEXT("outlineSize"), (float)val); break;
                                case IDC_SCROLLSPEED_EDIT:          source->SetInt(TEXT("scrollSpeed"), val); break;
								case IDC_NUMOFLINES_EDIT:           source->SetInt(TEXT("NumOfLines"), val); break;
                            }
                        }
                    }
                    break;

                case IDC_BOLD:
                case IDC_ITALIC:
                case IDC_UNDERLINE:
                case IDC_USEOUTLINE:
                    if(HIWORD(wParam) == BN_CLICKED && bInitializedDialog)
                    {
                        BOOL bChecked = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;

                        ConfigTextSourceInfo *configInfo = (ConfigTextSourceInfo*)GetWindowLongPtr(hwnd, DWLP_USER);
                        if(!configInfo) break;
                        ImageSource *source = API->GetSceneImageSource(configInfo->lpName);
                        if(source)
                        {
                            switch(LOWORD(wParam))
                            {
                                case IDC_BOLD:              source->SetInt(TEXT("bold"), bChecked); break;
                                case IDC_ITALIC:            source->SetInt(TEXT("italic"), bChecked); break;
                                case IDC_UNDERLINE:         source->SetInt(TEXT("underline"), bChecked); break;
                                case IDC_USEOUTLINE:        source->SetInt(TEXT("useOutline"), bChecked); break;
                            }
                        }

                        else if(LOWORD(wParam) == IDC_USEOUTLINE)
                        {
                            EnableWindow(GetDlgItem(hwnd, IDC_OUTLINETHICKNESS_EDIT), bChecked);
                            EnableWindow(GetDlgItem(hwnd, IDC_OUTLINETHICKNESS), bChecked);
                            EnableWindow(GetDlgItem(hwnd, IDC_OUTLINECOLOR), bChecked);
                            EnableWindow(GetDlgItem(hwnd, IDC_OUTLINEOPACITY_EDIT), bChecked);
                            EnableWindow(GetDlgItem(hwnd, IDC_OUTLINEOPACITY), bChecked);
                        }
                    }
                    break;

				case IDC_IRCSERVER:
				case IDC_PORT:
                    /*if(HIWORD(wParam) == CBN_SELCHANGE && bInitializedDialog)
                    {
                        int val = (int)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
                        if( val == CB_ERR)
                            break;

                        ConfigTextSourceInfo *configInfo = (ConfigTextSourceInfo*)GetWindowLongPtr(hwnd, DWLP_USER);
                        if(!configInfo) break;
                        ImageSource *source = API->GetSceneImageSource(configInfo->lpName);
                        if(source)
						{
							switch(LOWORD(wParam))
							{
								//case IDC_IRCSERVER:		source->SetInt(TEXT("iServer"), val);
								//case IDC_PORT:			source->SetInt(TEXT("iPort"), val);
							}
						}
                    }*/
                    break;
                case IDC_NICKNAME:
                case IDC_PASSWORD:
                case IDC_CHANNEL:
                    /*if(HIWORD(wParam) == EN_CHANGE && bInitializedDialog)
                    {
                        String val = GetEditText((HWND)lParam);

                        ConfigTextSourceInfo *configInfo = (ConfigTextSourceInfo*)GetWindowLongPtr(hwnd, DWLP_USER);
                        if(!configInfo) break;
                        ImageSource *source = API->GetSceneImageSource(configInfo->lpName);
                        if(source)
                        {
                            switch(LOWORD(wParam))
                            {
                                //case IDC_NICKNAME: source->SetString(TEXT("Nickname"), val); break;
                                //case IDC_PASSWORD: source->SetString(TEXT("Password"), val); break;
                                //case IDC_CHANNEL: source->SetString(TEXT("Channel"), val); break;
                            }
                        }
                    }*/
                    break;

                case IDOK:
                    {
                        ConfigTextSourceInfo *configInfo = (ConfigTextSourceInfo*)GetWindowLongPtr(hwnd, DWLP_USER);
                        if(!configInfo) break;
                        XElement *data = configInfo->data;

                        BOOL bUseOutline = SendMessage(GetDlgItem(hwnd, IDC_USEOUTLINE), BM_GETCHECK, 0, 0) == BST_CHECKED;
                        float outlineSize = (float)SendMessage(GetDlgItem(hwnd, IDC_OUTLINETHICKNESS), UDM_GETPOS32, 0, 0);

						String Nickname = GetEditText(GetDlgItem(hwnd, IDC_NICKNAME));
                        String Password = GetEditText(GetDlgItem(hwnd, IDC_PASSWORD));
						String Channel = GetEditText(GetDlgItem(hwnd, IDC_CHANNEL));

                        UINT extentWidth  = (UINT)SendMessage(GetDlgItem(hwnd, IDC_EXTENTWIDTH),  UDM_GETPOS32, 0, 0);
                        UINT extentHeight = (UINT)SendMessage(GetDlgItem(hwnd, IDC_EXTENTHEIGHT), UDM_GETPOS32, 0, 0);
						UINT NumOfLines = (UINT)SendMessage(GetDlgItem(hwnd, IDC_NUMOFLINES), UDM_GETPOS32, 0, 0);

                        String strFont = GetFontFace(configInfo, GetDlgItem(hwnd, IDC_FONT));
                        UINT fontSize = (UINT)SendMessage(GetDlgItem(hwnd, IDC_TEXTSIZE), UDM_GETPOS32, 0, 0);

                        BOOL bBold = SendMessage(GetDlgItem(hwnd, IDC_BOLD), BM_GETCHECK, 0, 0) == BST_CHECKED;
                        BOOL bItalic = SendMessage(GetDlgItem(hwnd, IDC_ITALIC), BM_GETCHECK, 0, 0) == BST_CHECKED;

                        BOOL pointFiltering = SendMessage(GetDlgItem(hwnd, IDC_POINTFILTERING), BM_GETCHECK, 0, 0) == BST_CHECKED;

                        String strFontDisplayName = GetEditText(GetDlgItem(hwnd, IDC_FONT));
                        if(strFont.IsEmpty())
                        {
                            UINT id = FindFontName(configInfo, GetDlgItem(hwnd, IDC_FONT), strFontDisplayName);
                            if(id != INVALID)
                                strFont = configInfo->fontFaces[id];
                        }

                        if(strFont.IsEmpty())
                        {
                            String strError = Str("Sources.TextSource.FontNotFound");
                            strError.FindReplace(TEXT("$1"), strFontDisplayName);
                            MessageBox(hwnd, strError, NULL, 0);
                            break;
                        }

						configInfo->cx = float(extentWidth);
						configInfo->cy = float(extentHeight);
                        data->SetFloat(TEXT("baseSizeCX"), configInfo->cx);
                        data->SetFloat(TEXT("baseSizeCY"), configInfo->cy);

                        data->SetString(TEXT("font"), strFont);
                        data->SetInt(TEXT("color"), CCGetColor(GetDlgItem(hwnd, IDC_COLOR)));
                        data->SetInt(TEXT("fontSize"), fontSize);
                        data->SetInt(TEXT("textOpacity"), (UINT)SendMessage(GetDlgItem(hwnd, IDC_TEXTOPACITY), UDM_GETPOS32, 0, 0));
                        data->SetInt(TEXT("scrollSpeed"), (int)SendMessage(GetDlgItem(hwnd, IDC_SCROLLSPEED), UDM_GETPOS32, 0, 0));
                        data->SetInt(TEXT("bold"), bBold);
                        data->SetInt(TEXT("italic"), bItalic);
                        data->SetInt(TEXT("underline"), SendMessage(GetDlgItem(hwnd, IDC_UNDERLINE), BM_GETCHECK, 0, 0) == BST_CHECKED);
                        data->SetInt(TEXT("pointFiltering"), pointFiltering);

                        data->SetInt(TEXT("backgroundColor"), CCGetColor(GetDlgItem(hwnd, IDC_BACKGROUNDCOLOR)));
                        data->SetInt(TEXT("backgroundOpacity"), (UINT)SendMessage(GetDlgItem(hwnd, IDC_BACKGROUNDOPACITY), UDM_GETPOS32, 0, 0));

                        data->SetInt(TEXT("useOutline"), bUseOutline);
                        data->SetInt(TEXT("outlineColor"), CCGetColor(GetDlgItem(hwnd, IDC_OUTLINECOLOR)));
                        data->SetFloat(TEXT("outlineSize"), outlineSize);
                        data->SetInt(TEXT("outlineOpacity"), (UINT)SendMessage(GetDlgItem(hwnd, IDC_OUTLINEOPACITY), UDM_GETPOS32, 0, 0));

                        data->SetInt(TEXT("extentWidth"), extentWidth);
                        data->SetInt(TEXT("extentHeight"), extentHeight);
						data->SetInt(TEXT("NumOfLines"), NumOfLines);
						data->SetInt(TEXT("iServer"), (int)SendMessage(GetDlgItem(hwnd, IDC_IRCSERVER), CB_GETCURSEL, 0, 0));
						data->SetInt(TEXT("iPort"), (int)SendMessage(GetDlgItem(hwnd, IDC_PORT), CB_GETCURSEL, 0, 0));
						data->SetString(TEXT("Nickname"), Nickname);
                        data->SetString(TEXT("Password"), Password);
                        data->SetString(TEXT("Channel"), Channel);
                    }

                case IDCANCEL:
                    if(LOWORD(wParam) == IDCANCEL)
                        DoCancelStuff(hwnd);

                    EndDialog(hwnd, LOWORD(wParam));
            }
            break;

        case WM_CLOSE:
            DoCancelStuff(hwnd);
            EndDialog(hwnd, IDCANCEL);
    }
    return 0;
}

bool STDCALL ConfigureTextSource(XElement *element, bool bCreating)
{
    if(!element)
    {
        AppWarning(TEXT("ConfigureTextSource: NULL element"));
        return false;
    }

    XElement *data = element->GetElement(TEXT("data"));
    if(!data)
        data = element->CreateElement(TEXT("data"));

    ConfigTextSourceInfo configInfo;
    configInfo.lpName = element->GetName();
    configInfo.data = data;

    if(DialogBoxParam(hinstMain, MAKEINTRESOURCE(IDD_CONFIG), API->GetMainWindow(), ConfigureTextProc, (LPARAM)&configInfo) == IDOK)
    {
        element->SetFloat(TEXT("cx"), configInfo.cx);
        element->SetFloat(TEXT("cy"), configInfo.cy);

        return true;
    }

    return false;
}


bool LoadPlugin()
{
	InitColorControl(hinstMain);
    API->RegisterImageSourceClass(TEXT("NicoCommentPlugin"), TEXT("Nico Comment Plugin"), (OBSCREATEPROC)CreateTextSource, (OBSCONFIGPROC)ConfigureTextSource);
	return true;
}

void UnloadPlugin()
{
	onDebugMsg(L"PLUGIN UNLOAD \n");
}

CTSTR GetPluginName()
{
	return TEXT("OBS NicoComment_Plugin");
}

CTSTR GetPluginDescription()
{
	return TEXT("Nico Comment Plugin (based on original TextOutput Source). Alpha Test Version 0.1");
}

//void ConfigPlugin(HWND hWnd)
//{
//	DialogBoxParam(hinstMain, MAKEINTRESOURCE(IDD_CONFIG), OBSGetMainWindow(), ConfigDialogProc, (LPARAM)hWnd);
//}

BOOL CALLBACK DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpBla)
{
	if(dwReason == DLL_PROCESS_ATTACH)
		hinstMain = hInst;

	return TRUE;
}

