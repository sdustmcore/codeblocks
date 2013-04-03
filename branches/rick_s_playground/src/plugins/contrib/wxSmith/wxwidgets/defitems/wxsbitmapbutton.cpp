/*
* This file is part of wxSmith plugin for Code::Blocks Studio
* Copyright (C) 2006  Bartlomiej Swiecki
*
* wxSmith is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* wxSmith is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with wxSmith; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*
* $Revision$
* $Id$
* $HeadURL$
*/

#include "wxsbitmapbutton.h"

namespace
{
    wxsRegisterItem<wxsBitmapButton> Reg(_T("BitmapButton"),wxsTWidget,_T("Standard"),50);

    WXS_ST_BEGIN(wxsBitmapButtonStyles,_T("wxBU_AUTODRAW"))
        WXS_ST_CATEGORY("wxBitmapButton")
        WXS_ST(wxBU_LEFT)
        WXS_ST(wxBU_TOP)
        WXS_ST(wxBU_RIGHT)
        WXS_ST(wxBU_BOTTOM)
        WXS_ST(wxBU_AUTODRAW)
        // cyberkoa: "The help mentions that wxBU_EXACTFIX is not used but the XRC code yes
        //  WXS_ST(wxBU_EXACTFIX)
        WXS_ST_DEFAULTS()
    WXS_ST_END()


    WXS_EV_BEGIN(wxsBitmapButtonEvents)
        WXS_EVI(EVT_BUTTON,wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEvent,Click)
    WXS_EV_END()
}

wxsBitmapButton::wxsBitmapButton(wxsItemResData* Data):
    wxsWidget(
        Data,
        &Reg.Info,
        wxsBitmapButtonEvents,
        wxsBitmapButtonStyles)
{}

void wxsBitmapButton::OnBuildCreatingCode(wxString& Code,const wxString& WindowParent,wxsCodingLang Language)
{
    switch ( Language )
    {
        case wxsCPP:
        {
            Codef(_T("%C(%W,%I,%i,%P,%S,%T,%V,%N);\n"),&BitmapLabel,wxART_BUTTON);
            if ( !BitmapDisabled.IsEmpty() )
            {
                Codef(_T("%ASetBitmapDisabled(%i);\n"),&BitmapDisabled,wxART_BUTTON);
            }
            if ( !BitmapSelected.IsEmpty() )
            {
                Codef(_T("%ASetBitmapSelected(%i);\n"),&BitmapSelected,wxART_BUTTON);
            }
            if ( !BitmapFocus.IsEmpty() )
            {
                Codef(_T("%ASetBitmapFocus(%i);\n"),&BitmapFocus,wxART_BUTTON);
            }

            if ( IsDefault )
            {
                Codef(_T("%ASetDefault();\n"));
            }
            SetupWindowCode(Code,WindowParent,Language);
            return;
        }

        default:
        {
            wxsCodeMarks::Unknown(_T("wxsBitmapButton::OnBuildCreatingCode"),Language);
        }
    }
}


wxObject* wxsBitmapButton::OnBuildPreview(wxWindow* Parent,long Flags)
{
    wxBitmapButton* Preview = new wxBitmapButton(Parent,GetId(),BitmapLabel.GetPreview(wxDefaultSize),Pos(Parent),Size(Parent),Style());

    if ( !BitmapDisabled.IsEmpty() )
    {
        Preview->SetBitmapDisabled(BitmapDisabled.GetPreview(wxDefaultSize));
    }

    if ( !BitmapSelected.IsEmpty() )
    {
        Preview->SetBitmapSelected(BitmapSelected.GetPreview(wxDefaultSize));
    }

    if ( !BitmapFocus.IsEmpty() )
    {
        Preview->SetBitmapFocus(BitmapFocus.GetPreview(wxDefaultSize));
    }

    if ( IsDefault )
    {
        Preview->SetDefault();
    }
    return SetupWindow(Preview,Flags);
}


void wxsBitmapButton::OnEnumWidgetProperties(long Flags)
{
    WXS_BITMAP(wxsBitmapButton,BitmapLabel,_("Bitmap"),_T("bitmap"),_T("wxART_OTHER"))
    WXS_BITMAP(wxsBitmapButton,BitmapDisabled,_("Disabled bmp."),_T("disabled"),_T("wxART_OTHER"))
    WXS_BITMAP(wxsBitmapButton,BitmapSelected,_("Pressed bmp."),_T("selected"),_T("wxART_OTHER"))
    WXS_BITMAP(wxsBitmapButton,BitmapFocus,_("Focused bmp."),_T("focus"),_T("wxART_OTHER"))
    WXS_BOOL  (wxsBitmapButton,IsDefault,_("Is default"),_T("default"),false)
}

void wxsBitmapButton::OnEnumDeclFiles(wxArrayString& Decl,wxArrayString& Def,wxsCodingLang Language)
{
    switch ( Language )
    {
        case wxsCPP:
        {
            Decl.Add(_T("<wx/bmpbuttn.h>"));
            Def.Add(_T("<wx/bitmap.h>"));
            Def.Add(_T("<wx/image.h>"));
            Def.Add(_T("<wx/artprov.h>"));
            return;
        }

        default:
        {
            wxsCodeMarks::Unknown(_T("wxsBitmapButtonn::OnEnumDeclFiles"),Language);
        }
    }
}