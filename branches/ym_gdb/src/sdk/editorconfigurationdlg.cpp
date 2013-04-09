/*
* This file is part of Code::Blocks Studio, an open-source cross-platform IDE
* Copyright (C) 2003  Yiannis An. Mandravellos
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Contact e-mail: Yiannis An. Mandravellos <mandrav@codeblocks.org>
* Program URL   : http://www.codeblocks.org
*
* $Id$
* $Date$
*/

#include "sdk_precomp.h"
#include <wx/xrc/xmlres.h>
#include <wx/colordlg.h>
#include <wx/fontdlg.h>
#include <wx/fontutil.h>
#include <wx/fontmap.h>
#include <wx/spinctrl.h>
#include <wx/combobox.h>
#include <wx/radiobox.h>
#include <wx/listbox.h>
#include <wx/textctrl.h>
#include <wx/listbook.h>

#include "editorcolorset.h"
#include "editorconfigurationdlg.h"
#include "editkeywordsdlg.h"
#include "manager.h"
#include "configmanager.h"
#include "pluginmanager.h"
#include "editormanager.h"
#include "cbeditor.h"
#include "globals.h"

// images by order of pages
const wxString base_imgs[] =
{
    _T("editor"),
    _T("folding"),
    _T("gutter-margin"),
    _T("syntax-highlight"),
    _T("abbrev"),
    _T("default-code"),
};
const int IMAGES_COUNT = 6; // keep this in sync!

// map cmbDefCodeFileType indexes to FileType values
// if more entries are added to cmbDefCodeFileType, edit the mapping here
const FileType IdxToFileType[] = { ftSource, ftHeader };

BEGIN_EVENT_TABLE(EditorConfigurationDlg, wxDialog)
	EVT_BUTTON(XRCID("btnChooseEditorFont"), 	EditorConfigurationDlg::OnChooseFont)
	EVT_BUTTON(XRCID("btnKeywords"), 			EditorConfigurationDlg::OnEditKeywords)
	EVT_BUTTON(XRCID("btnFilemasks"), 			EditorConfigurationDlg::OnEditFilemasks)
	EVT_BUTTON(XRCID("btnColorsReset"), 		EditorConfigurationDlg::OnColorsReset)
	EVT_BUTTON(XRCID("btnGutterColor"), 		EditorConfigurationDlg::OnChooseColor)
	EVT_BUTTON(XRCID("btnColorsFore"), 			EditorConfigurationDlg::OnChooseColor)
	EVT_BUTTON(XRCID("btnColorsBack"), 			EditorConfigurationDlg::OnChooseColor)
	EVT_BUTTON(XRCID("btnColorsAddTheme"), 		EditorConfigurationDlg::OnAddColorTheme)
	EVT_BUTTON(XRCID("btnColorsDeleteTheme"), 	EditorConfigurationDlg::OnDeleteColorTheme)
	EVT_BUTTON(XRCID("btnColorsRenameTheme"), 	EditorConfigurationDlg::OnRenameColorTheme)
	EVT_CHECKBOX(XRCID("chkColorsBold"),		EditorConfigurationDlg::OnBoldItalicUline)
	EVT_CHECKBOX(XRCID("chkColorsItalics"),		EditorConfigurationDlg::OnBoldItalicUline)
	EVT_CHECKBOX(XRCID("chkColorsUnderlined"),	EditorConfigurationDlg::OnBoldItalicUline)
	EVT_BUTTON(XRCID("btnOK"), 					EditorConfigurationDlg::OnOK)
	EVT_LISTBOX(XRCID("lstComponents"),			EditorConfigurationDlg::OnColorComponent)
	EVT_COMBOBOX(XRCID("cmbLangs"),				EditorConfigurationDlg::OnChangeLang)
	EVT_COMBOBOX(XRCID("cmbDefCodeFileType"),	EditorConfigurationDlg::OnChangeDefCodeFileType)
	EVT_COMBOBOX(XRCID("cmbThemes"),	        EditorConfigurationDlg::OnColorTheme)
	EVT_LISTBOX(XRCID("lstAutoCompKeyword"),	EditorConfigurationDlg::OnAutoCompKeyword)
	EVT_BUTTON(XRCID("btnAutoCompAdd"),	        EditorConfigurationDlg::OnAutoCompAdd)
	EVT_BUTTON(XRCID("btnAutoCompDelete"),	    EditorConfigurationDlg::OnAutoCompDelete)

    EVT_LISTBOOK_PAGE_CHANGED(XRCID("nbMain"), EditorConfigurationDlg::OnPageChanged)
END_EVENT_TABLE()

EditorConfigurationDlg::EditorConfigurationDlg(wxWindow* parent)
	: m_TextColorControl(0L),
	m_AutoCompTextControl(0L),
	m_Theme(0L),
	m_Lang(-1),
	m_DefCodeFileType(0),
	m_ThemeModified(false),
	m_LastAutoCompKeyword(-1)
{
	wxXmlResource::Get()->LoadDialog(this, parent, _T("dlgConfigureEditor"));

	XRCCTRL(*this, "lblEditorFont", wxStaticText)->SetLabel(_("This is sample text"));
	UpdateSampleFont(false);

    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("editor"));

   	XRCCTRL(*this, "chkAutoIndent", wxCheckBox)->SetValue(cfg->ReadBool(_T("/auto_indent"), true));
   	XRCCTRL(*this, "chkSmartIndent", wxCheckBox)->SetValue(cfg->ReadBool(_T("/smart_indent"), true));
   	XRCCTRL(*this, "chkUseTab", wxCheckBox)->SetValue(cfg->ReadBool(_T("/use_tab"), false));
   	XRCCTRL(*this, "chkShowIndentGuides", wxCheckBox)->SetValue(cfg->ReadBool(_T("/show_indent_guides"), false));
   	XRCCTRL(*this, "chkTabIndents", wxCheckBox)->SetValue(cfg->ReadBool(_T("/tab_indents"), true));
   	XRCCTRL(*this, "chkBackspaceUnindents", wxCheckBox)->SetValue(cfg->ReadBool(_T("/backspace_unindents"), true));
   	XRCCTRL(*this, "chkWordWrap", wxCheckBox)->SetValue(cfg->ReadBool(_T("/word_wrap"), false));
   	XRCCTRL(*this, "chkShowLineNumbers", wxCheckBox)->SetValue(cfg->ReadBool(_T("/show_line_numbers"), true));
   	XRCCTRL(*this, "chkHighlightCaretLine", wxCheckBox)->SetValue(cfg->ReadBool(_T("/highlight_caret_line"), false));
   	XRCCTRL(*this, "spnTabSize", wxSpinCtrl)->SetValue(cfg->ReadInt(_T("/tab_size"), 4));
   	XRCCTRL(*this, "cmbViewWS", wxComboBox)->SetSelection(cfg->ReadInt(_T("/view_whitespace"), 0));
   	XRCCTRL(*this, "rbTabText", wxRadioBox)->SetSelection(cfg->ReadBool(_T("/tab_text_relative"), true) ? 1 : 0);
   	XRCCTRL(*this, "chkAutoWrapSearch", wxCheckBox)->SetValue(cfg->ReadBool(_T("/auto_wrap_search"), true));

   	// end-of-line
   	XRCCTRL(*this, "chkShowEOL", wxCheckBox)->SetValue(cfg->ReadBool(_T("/show_eol"), false));
   	XRCCTRL(*this, "chkStripTrailings", wxCheckBox)->SetValue(cfg->ReadBool(_T("/eol/strip_trailing_spaces"), true));
   	XRCCTRL(*this, "chkEnsureFinalEOL", wxCheckBox)->SetValue(cfg->ReadBool(_T("/eol/ensure_final_line_end"), true));
   	XRCCTRL(*this, "chkEnsureConsistentEOL", wxCheckBox)->SetValue(cfg->ReadBool(_T("/eol/ensure_consistent_line_ends"), false));
    XRCCTRL(*this, "cmbEOLMode", wxComboBox)->SetSelection(cfg->ReadInt(_T("/eol/eolmode"), 0));

	//folding
   	XRCCTRL(*this, "chkEnableFolding", wxCheckBox)->SetValue(cfg->ReadBool(_T("/folding/show_folds"), true));
   	XRCCTRL(*this, "chkFoldOnOpen", wxCheckBox)->SetValue(cfg->ReadBool(_T("/folding/fold_all_on_open"), false));
   	XRCCTRL(*this, "chkFoldPreprocessor", wxCheckBox)->SetValue(cfg->ReadBool(_T("/folding/fold_preprocessor"), false));
   	XRCCTRL(*this, "chkFoldComments", wxCheckBox)->SetValue(cfg->ReadBool(_T("/folding/fold_comments"), true));
   	XRCCTRL(*this, "chkFoldXml", wxCheckBox)->SetValue(cfg->ReadBool(_T("/folding/fold_xml"), true));

	//gutter
    wxColour color = cfg->ReadColour(_T("/gutter/color"), *wxLIGHT_GREY);
    XRCCTRL(*this, "lstGutterMode", wxChoice)->SetSelection(cfg->ReadInt(_T("/gutter/mode"), 0));
    XRCCTRL(*this, "btnGutterColor", wxButton)->SetBackgroundColour(color);
    XRCCTRL(*this, "spnGutterColumn", wxSpinCtrl)->SetValue(cfg->ReadInt(_T("/gutter/column"), 80));

	// color set
	LoadThemes();

    // auto-complete
    CreateAutoCompText();
    wxListBox* lstKeyword = XRCCTRL(*this, "lstAutoCompKeyword", wxListBox);
    lstKeyword->Clear();
    m_AutoCompMap = Manager::Get()->GetEditorManager()->GetAutoCompleteMap();
    AutoCompleteMap::iterator it;
    for (it = m_AutoCompMap.begin(); it != m_AutoCompMap.end(); ++it)
    {
    	lstKeyword->Append(it->first);
    }
    if (m_AutoCompMap.size() != 0)
    {
        lstKeyword->SetSelection(0);
        m_LastAutoCompKeyword = 0;
        it = m_AutoCompMap.begin();
        m_AutoCompTextControl->SetText(it->second);
    }

	// default code
    wxString key;
    key.Printf(_T("/default_code/set%d"), IdxToFileType[m_DefCodeFileType]);
    XRCCTRL(*this, "txtDefCode", wxTextCtrl)->SetValue(cfg->Read(key, wxEmptyString));
    wxFont tmpFont(8, wxMODERN, wxNORMAL, wxNORMAL);
    XRCCTRL(*this, "txtDefCode", wxTextCtrl)->SetFont(tmpFont);

    // load listbook images
    const wxString base = ConfigManager::GetDataFolder() + _T("/images/settings/");

    wxImageList* images = new wxImageList(80, 80);
    wxBitmap bmp;
    for (int i = 0; i < IMAGES_COUNT; ++i)
    {
        bmp.LoadFile(base + base_imgs[i] + _T(".png"), wxBITMAP_TYPE_PNG);
        images->Add(bmp);
        bmp.LoadFile(base + base_imgs[i] + _T("-off.png"), wxBITMAP_TYPE_PNG);
        images->Add(bmp);
    }
    wxListbook* lb = XRCCTRL(*this, "nbMain", wxListbook);
    lb->AssignImageList(images);

    // add all plugins configuration panels
    AddPluginPanels();

    // make sure everything is laid out properly
    GetSizer()->SetSizeHints(this);
}

EditorConfigurationDlg::~EditorConfigurationDlg()
{
	if (m_Theme)
		delete m_Theme;

	if (m_TextColorControl)
		delete m_TextColorControl;

    if (m_AutoCompTextControl)
        delete m_AutoCompTextControl;
}

void EditorConfigurationDlg::AddPluginPanels()
{
    const wxString base = ConfigManager::GetDataFolder() + _T("/images/settings/");

    wxListbook* lb = XRCCTRL(*this, "nbMain", wxListbook);
    // get all configuration panels which are about the editor.
    Manager::Get()->GetPluginManager()->GetConfigurationPanels(cgEditor, lb, m_PluginPanels);

    for (size_t i = 0; i < m_PluginPanels.GetCount(); ++i)
    {
        cbConfigurationPanel* panel = m_PluginPanels[i];
        lb->AddPage(panel, panel->GetTitle());

        lb->GetImageList()->Add(LoadPNGWindows2000Hack(base + panel->GetBitmapBaseName() + _T(".png")));
        lb->GetImageList()->Add(LoadPNGWindows2000Hack(base + panel->GetBitmapBaseName() + _T("-off.png")));
        lb->SetPageImage(lb->GetPageCount() - 1, lb->GetImageList()->GetImageCount() - 2);
    }

    UpdateListbookImages();
}

void EditorConfigurationDlg::UpdateListbookImages()
{
    wxListbook* lb = XRCCTRL(*this, "nbMain", wxListbook);
    int sel = lb->GetSelection();
    // set page images according to their on/off status
    for (size_t i = 0; i < IMAGES_COUNT + m_PluginPanels.GetCount(); ++i)
    {
        lb->SetPageImage(i, (i * 2) + (sel == (int)i ? 0 : 1));
    }

    // the selection color is ruining the on/off effect,
    // so make sure no item is selected ;)
    lb->GetListView()->Select(sel, false);

    // update the page title
    wxString label = lb->GetPageText(sel);
    // replace any stray & with && because label makes it an underscore
    while (label.Replace(_T(" & "), _T(" && ")))
        ;
    XRCCTRL(*this, "lblBigTitle", wxStaticText)->SetLabel(label);
}

void EditorConfigurationDlg::OnPageChanged(wxListbookEvent& event)
{
    UpdateListbookImages();
}

void EditorConfigurationDlg::CreateColorsSample()
{
    Freeze();
	if (m_TextColorControl)
		delete m_TextColorControl;
	m_TextColorControl = new cbStyledTextCtrl(this, wxID_ANY);
	m_TextColorControl->SetTabWidth(4);

    int breakLine = -1;
    int debugLine = -1;
    int errorLine = -1;
    wxString code = m_Theme->GetSampleCode(m_Lang, &breakLine, &debugLine, &errorLine);
    if (!code.IsEmpty())
        m_TextColorControl->LoadFile(code);

	m_TextColorControl->SetReadOnly(true);
	m_TextColorControl->SetCaretWidth(0);
    m_TextColorControl->SetMarginType(0, wxSCI_MARGIN_NUMBER);
    m_TextColorControl->SetMarginWidth(0, 32);
	ApplyColors();

    m_TextColorControl->SetMarginWidth(1, 0);
	if (breakLine != -1) m_TextColorControl->MarkerAdd(breakLine, 2); // breakpoint line
	if (debugLine != -1) m_TextColorControl->MarkerAdd(debugLine, 3); // active line
	if (errorLine != -1) m_TextColorControl->MarkerAdd(errorLine, 4); // error line

	FillColorComponents();
    wxXmlResource::Get()->AttachUnknownControl(_T("txtColorsSample"), m_TextColorControl);
    Thaw();
}

void EditorConfigurationDlg::CreateAutoCompText()
{
	if (m_AutoCompTextControl)
		delete m_AutoCompTextControl;
	m_AutoCompTextControl = new cbStyledTextCtrl(this, wxID_ANY);
	m_AutoCompTextControl->SetTabWidth(4);
    m_AutoCompTextControl->SetMarginType(0, wxSCI_MARGIN_NUMBER);
    m_AutoCompTextControl->SetMarginWidth(0, 32);
    m_AutoCompTextControl->SetViewWhiteSpace(1);
	ApplyColors();
    wxXmlResource::Get()->AttachUnknownControl(_T("txtAutoCompCode"), m_AutoCompTextControl);
}

void EditorConfigurationDlg::FillColorComponents()
{
	wxListBox* colors = XRCCTRL(*this, "lstComponents", wxListBox);
	colors->Clear();
	for (int i = 0; i < m_Theme->GetOptionCount(m_Lang); ++i)
	{
		OptionColor* opt = m_Theme->GetOptionByIndex(m_Lang, i);
		if (colors->FindString(opt->name) == -1)
            colors->Append(opt->name);
	}
	colors->SetSelection(0);
	ReadColors();
}

void EditorConfigurationDlg::ApplyColors()
{
	if (m_TextColorControl && m_Theme)
	{
		wxFont fnt = XRCCTRL(*this, "lblEditorFont", wxStaticText)->GetFont();
		if (m_TextColorControl)
		{
            m_TextColorControl->StyleSetFont(wxSCI_STYLE_DEFAULT,fnt);
            m_Theme->Apply(m_Lang, m_TextColorControl);
        }
		if (m_AutoCompTextControl)
		{
            m_AutoCompTextControl->StyleSetFont(wxSCI_STYLE_DEFAULT,fnt);
            m_Theme->Apply(m_Theme->GetHighlightLanguage(_T("C/C++")), m_AutoCompTextControl);
        }
	}
}

void EditorConfigurationDlg::ReadColors()
{
	if (m_Theme)
	{
		wxListBox* colors = XRCCTRL(*this, "lstComponents", wxListBox);
/* TODO (mandrav#1#): FIXME!!! */
		OptionColor* opt = m_Theme->GetOptionByName(m_Lang, colors->GetStringSelection());
		if (opt)
		{
			wxColour c = opt->fore;
			if (c == wxNullColour)
			{
                XRCCTRL(*this, "btnColorsFore", wxButton)->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
                XRCCTRL(*this, "btnColorsFore", wxButton)->SetLabel(_("\"Default\""));
            }
            else
            {
                XRCCTRL(*this, "btnColorsFore", wxButton)->SetBackgroundColour(c);
                XRCCTRL(*this, "btnColorsFore", wxButton)->SetLabel(_T(""));
            }

			c = opt->back;
			if (c == wxNullColour)
			{
                XRCCTRL(*this, "btnColorsBack", wxButton)->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
                XRCCTRL(*this, "btnColorsBack", wxButton)->SetLabel(_("\"Default\""));
            }
            else
            {
                XRCCTRL(*this, "btnColorsBack", wxButton)->SetBackgroundColour(c);
                XRCCTRL(*this, "btnColorsBack", wxButton)->SetLabel(_T(""));
            }

			XRCCTRL(*this, "chkColorsBold", wxCheckBox)->SetValue(opt->bold);
			XRCCTRL(*this, "chkColorsItalics", wxCheckBox)->SetValue(opt->italics);
			XRCCTRL(*this, "chkColorsUnderlined", wxCheckBox)->SetValue(opt->underlined);

			XRCCTRL(*this, "btnColorsFore", wxButton)->Enable(opt->isStyle);
			XRCCTRL(*this, "chkColorsBold", wxCheckBox)->Enable(opt->isStyle);
			XRCCTRL(*this, "chkColorsItalics", wxCheckBox)->Enable(opt->isStyle);
			XRCCTRL(*this, "chkColorsUnderlined", wxCheckBox)->Enable(opt->isStyle);
		}
	}
}

void EditorConfigurationDlg::WriteColors()
{
	if (m_Theme)
	{
		wxListBox* colors = XRCCTRL(*this, "lstComponents", wxListBox);
/* TODO (mandrav#1#): FIXME!!! */
		OptionColor* opt = m_Theme->GetOptionByName(m_Lang, colors->GetStringSelection());
		if (opt)
		{
            wxColour c = XRCCTRL(*this, "btnColorsFore", wxButton)->GetBackgroundColour();
            if (c != wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE))
                opt->fore = c;
			c = XRCCTRL(*this, "btnColorsBack", wxButton)->GetBackgroundColour();
            if (c != wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE))
                opt->back = c;
			opt->bold = XRCCTRL(*this, "chkColorsBold", wxCheckBox)->GetValue();
			opt->italics = XRCCTRL(*this, "chkColorsItalics", wxCheckBox)->GetValue();
			opt->underlined = XRCCTRL(*this, "chkColorsUnderlined", wxCheckBox)->GetValue();
			m_Theme->UpdateOptionsWithSameName(m_Lang, opt);
		}
	}
	ApplyColors();
	m_ThemeModified = true;
}

void EditorConfigurationDlg::UpdateSampleFont(bool askForNewFont)
{
    wxFont tmpFont(8, wxMODERN, wxNORMAL, wxNORMAL);
    wxString fontstring = Manager::Get()->GetConfigManager(_T("editor"))->Read(_T("/font"), wxEmptyString);

    if (!fontstring.IsEmpty())
    {
        wxNativeFontInfo nfi;
        nfi.FromString(fontstring);
        tmpFont.SetNativeFontInfo(nfi);
    }

	XRCCTRL(*this, "lblEditorFont", wxStaticText)->SetFont(tmpFont);
	if (!askForNewFont)
		return;

	wxFontData data;
    data.SetInitialFont(tmpFont);

	wxFontDialog dlg(this, &data);
    if (dlg.ShowModal() == wxID_OK)
    {
    	wxFont font = dlg.GetFontData().GetChosenFont();
		XRCCTRL(*this, "lblEditorFont", wxStaticText)->SetFont(font);
		ApplyColors();
    }
}

void EditorConfigurationDlg::LoadThemes()
{
	wxComboBox* cmbThemes = XRCCTRL(*this, "cmbThemes", wxComboBox);
	cmbThemes->Clear();
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("editor"));
	wxArrayString list = cfg->EnumerateSubPaths(_T("/color_sets"));
	for (unsigned int i = 0; i < list.GetCount(); ++i)
        cmbThemes->Append(list[i]);
    if (cmbThemes->GetCount() == 0)
        cmbThemes->Append(COLORSET_DEFAULT);
    wxString group = cfg->Read(_T("/color_sets/active_color_set"), COLORSET_DEFAULT);
    long int cookie = cmbThemes->FindString(group);
    if (cookie == wxNOT_FOUND)
        cookie = 0;
    cmbThemes->SetSelection(cookie);
    ChangeTheme();
}

bool EditorConfigurationDlg::AskToSaveTheme()
{
    wxComboBox* cmbThemes = XRCCTRL(*this, "cmbThemes", wxComboBox);
    if (m_Theme && m_ThemeModified)
    {
        wxString msg;
        msg.Printf(_("The color theme \"%s\" is modified.\nDo you want to save the changes?"), m_Theme->GetName().c_str());
        int ret = wxMessageBox(msg, _("Save"), wxYES_NO | wxCANCEL);
        switch (ret)
        {
            case wxYES: m_Theme->Save(); break;
            case wxCANCEL:
            {
                int idx = cmbThemes->FindString(m_Theme->GetName());
                cmbThemes->SetSelection(idx);
                return false;
            }
            default: break;
        }
    }
    return true;
}

void EditorConfigurationDlg::ChangeTheme()
{
    wxComboBox* cmbThemes = XRCCTRL(*this, "cmbThemes", wxComboBox);
    if (cmbThemes->GetSelection() == wxNOT_FOUND)
        cmbThemes->SetSelection(0);
    wxString key = cmbThemes->GetStringSelection();
    XRCCTRL(*this, "btnColorsRenameTheme", wxButton)->Enable(key != COLORSET_DEFAULT);
    XRCCTRL(*this, "btnColorsDeleteTheme", wxButton)->Enable(key != COLORSET_DEFAULT);

    if (m_Theme)
        delete m_Theme;
    m_Theme = new EditorColorSet(key);

   	XRCCTRL(*this, "btnKeywords", wxButton)->Enable(m_Theme);
   	XRCCTRL(*this, "btnFilemasks", wxButton)->Enable(m_Theme);

	wxComboBox* cmbLangs = XRCCTRL(*this, "cmbLangs", wxComboBox);
	int sel = cmbLangs->GetSelection();
    cmbLangs->Clear();
    wxArrayString langs = m_Theme->GetAllHighlightLanguages();
    for (unsigned int i = 0; i < langs.GetCount(); ++i)
    {
    	cmbLangs->Append(langs[i]);
    }
	if (sel == -1)
	{
	    wxString lang = Manager::Get()->GetConfigManager(_T("editor"))->Read(_T("/color_sets/active_lang"), _T("C/C++"));
        sel = cmbLangs->FindString(lang);
	}
    cmbLangs->SetSelection(sel != -1 ? sel : 0);
    cmbLangs->Enable(langs.GetCount() != 0);
	if (m_Theme)
	{
		wxString sel = cmbLangs->GetStringSelection();
		m_Lang = m_Theme->GetHighlightLanguage(sel);
	}

	CreateColorsSample();
	m_ThemeModified = false;
}

// events

void EditorConfigurationDlg::OnColorTheme(wxCommandEvent& event)
{
    // theme has changed
    wxComboBox* cmbThemes = XRCCTRL(*this, "cmbThemes", wxComboBox);
    if (m_Theme && m_Theme->GetName() != cmbThemes->GetStringSelection())
    {
        if (AskToSaveTheme())
            ChangeTheme();
    }
}

void EditorConfigurationDlg::OnAddColorTheme(wxCommandEvent& event)
{
    wxTextEntryDialog dlg(this, _("Please enter the name of the new color theme:"), _("New theme name"));
    if (dlg.ShowModal() != wxID_OK)
        return;

    wxString name = dlg.GetValue();
    wxComboBox* cmbThemes = XRCCTRL(*this, "cmbThemes", wxComboBox);
    cmbThemes->Append(name);
    cmbThemes->SetSelection(cmbThemes->GetCount() - 1);
    ChangeTheme();
}

void EditorConfigurationDlg::OnDeleteColorTheme(wxCommandEvent& event)
{
    if (wxMessageBox(_("Are you sure you want to delete this theme?"), _("Confirmation"), wxYES_NO) == wxYES)
    {
        Manager::Get()->GetConfigManager(_T("editor"))->DeleteSubPath(_T("/color_sets/") + m_Theme->GetName());
        wxComboBox* cmbThemes = XRCCTRL(*this, "cmbThemes", wxComboBox);
        int idx = cmbThemes->FindString(m_Theme->GetName());
        if (idx != wxNOT_FOUND)
            cmbThemes->Delete(idx);
        cmbThemes->SetSelection(wxNOT_FOUND);
        ChangeTheme();
    }
}

void EditorConfigurationDlg::OnRenameColorTheme(wxCommandEvent& event)
{
#ifdef __WXMSW__
    wxTextEntryDialog dlg(this, _("Please enter the new name of the new color theme:"), _("New theme name"), m_Theme->GetName());
    if (dlg.ShowModal() != wxID_OK)
        return;

    wxString name = dlg.GetValue();
    wxComboBox* cmbThemes = XRCCTRL(*this, "cmbThemes", wxComboBox);
    int idx = cmbThemes->FindString(m_Theme->GetName());
    if (idx != wxNOT_FOUND)
        cmbThemes->SetString(idx, name);
    Manager::Get()->GetConfigManager(_T("editor"))->DeleteSubPath(_T("/color_sets/") + m_Theme->GetName());
    m_Theme->SetName(name);
#else
	#warning "wxComboBox::SetString() doesn't work under non-win32 platforms"
#endif
}

void EditorConfigurationDlg::OnEditKeywords(wxCommandEvent& event)
{
	if (m_Theme && m_Lang != HL_NONE)
	{
	    EditKeywordsDlg dlg(0, m_Theme, m_Lang);
	    if (dlg.ShowModal() == wxID_OK)
	    {
			m_Theme->SetKeywords(m_Lang, 0, dlg.GetLangKeywords());
			m_Theme->SetKeywords(m_Lang, 1, dlg.GetDocKeywords());
			m_Theme->SetKeywords(m_Lang, 2, dlg.GetUserKeywords());
	    }
	}
}

void EditorConfigurationDlg::OnEditFilemasks(wxCommandEvent& event)
{
	if (m_Theme && m_Lang != HL_NONE)
	{
		wxString masks = wxGetTextFromUser(_("Edit filemasks (use commas to spearate them - case insensitive):"),
										m_Theme->GetLanguageName(m_Lang),
										GetStringFromArray(m_Theme->GetFileMasks(m_Lang), _T(",")));
		if (!masks.IsEmpty())
			m_Theme->SetFileMasks(m_Lang, masks);
	}
}

void EditorConfigurationDlg::OnColorsReset(wxCommandEvent& event)
{
	if (m_Theme && m_Lang != HL_NONE)
	{
	    wxString tmp;
	    tmp.Printf(_("Are you sure you want to reset all settings to defaults for \"%s\"?"),
                    m_Theme->GetLanguageName(m_Lang).c_str());
        if (wxMessageBox(tmp, _("Confirmation"), wxICON_QUESTION | wxYES_NO) == wxYES)
        {
            m_Theme->Reset(m_Lang);
            ApplyColors();
            m_ThemeModified = true;
        }
	}
}

void EditorConfigurationDlg::OnChangeLang(wxCommandEvent& event)
{
	if (m_Theme)
	{
		wxString sel = XRCCTRL(*this, "cmbLangs", wxComboBox)->GetStringSelection();
		m_Lang = m_Theme->GetHighlightLanguage(sel);
	}
	FillColorComponents();
	CreateColorsSample();
}

void EditorConfigurationDlg::OnChangeDefCodeFileType(wxCommandEvent& event)
{
    wxString key;
	int sel = XRCCTRL(*this, "cmbDefCodeFileType", wxComboBox)->GetSelection();
//	if (sel != m_DefCodeFileType)
	{
        key.Printf(_T("/default_code/set%d"), IdxToFileType[m_DefCodeFileType]);
        Manager::Get()->GetConfigManager(_T("editor"))->Write(key, XRCCTRL(*this, "txtDefCode", wxTextCtrl)->GetValue());
	}
	m_DefCodeFileType = sel;
    key.Printf(_T("/default_code/set%d"), IdxToFileType[m_DefCodeFileType]);
    XRCCTRL(*this, "txtDefCode", wxTextCtrl)->SetValue(Manager::Get()->GetConfigManager(_T("editor"))->Read(key, wxEmptyString));
}

void EditorConfigurationDlg::OnChooseColor(wxCommandEvent& event)
{
	wxColourData data;
	wxWindow* sender = FindWindowById(event.GetId());
    data.SetColour(sender->GetBackgroundColour());

	wxColourDialog dlg(this, &data);
    if (dlg.ShowModal() == wxID_OK)
    {
    	wxColour color = dlg.GetColourData().GetColour();
	    sender->SetBackgroundColour(color);
    }

	if (event.GetId() == XRCID("btnColorsFore") ||
		event.GetId() == XRCID("btnColorsBack"))
		WriteColors();
}

void EditorConfigurationDlg::OnChooseFont(wxCommandEvent& event)
{
	UpdateSampleFont(true);
}

void EditorConfigurationDlg::OnColorComponent(wxCommandEvent& event)
{
	ReadColors();
}

void EditorConfigurationDlg::OnBoldItalicUline(wxCommandEvent& event)
{
	WriteColors();
}

void EditorConfigurationDlg::AutoCompUpdate(int index)
{
    if (index != -1)
    {
        wxListBox* lstKeyword = XRCCTRL(*this, "lstAutoCompKeyword", wxListBox);
        wxString lastSel = lstKeyword->GetString(index);
        if (m_AutoCompTextControl->GetText() != m_AutoCompMap[lastSel])
            m_AutoCompMap[lastSel] = m_AutoCompTextControl->GetText();
    }
}

void EditorConfigurationDlg::OnAutoCompAdd(wxCommandEvent& event)
{
    wxString key = wxGetTextFromUser(_("Please enter the new keyword"), _("Add keyword"));
    if (!key.IsEmpty())
    {
        AutoCompleteMap::iterator it = m_AutoCompMap.find(key);
        if (it != m_AutoCompMap.end())
        {
            wxMessageBox(_("This keyword already exists!"), _("Error"), wxICON_ERROR);
            return;
        }
        m_AutoCompMap[key] = _T("");
        wxListBox* lstKeyword = XRCCTRL(*this, "lstAutoCompKeyword", wxListBox);
        lstKeyword->Append(key);
        AutoCompUpdate(lstKeyword->GetSelection());
        m_AutoCompTextControl->SetText(_T(""));
        m_LastAutoCompKeyword = lstKeyword->GetCount() - 1;
        lstKeyword->SetSelection(m_LastAutoCompKeyword);
    }
}

void EditorConfigurationDlg::OnAutoCompDelete(wxCommandEvent& event)
{
    wxListBox* lstKeyword = XRCCTRL(*this, "lstAutoCompKeyword", wxListBox);
    if (lstKeyword->GetSelection() == -1)
        return;

    if (wxMessageBox(_("Are you sure you want to delete this keyword?"), _("Confirmation"), wxICON_QUESTION | wxYES_NO) == wxNO)
        return;

    int sel = lstKeyword->GetSelection();
    AutoCompleteMap::iterator it = m_AutoCompMap.find(lstKeyword->GetString(sel));
    if (it != m_AutoCompMap.end())
    {
        m_AutoCompMap.erase(it);
        lstKeyword->Delete(sel);
        if (sel >= lstKeyword->GetCount())
            sel = lstKeyword->GetCount() - 1;
        lstKeyword->SetSelection(sel);
        if (sel != -1)
        {
            m_AutoCompTextControl->SetText(m_AutoCompMap[lstKeyword->GetString(sel)]);
            m_LastAutoCompKeyword = sel;
        }
        else
            m_AutoCompTextControl->SetText(_T(""));
    }
}

void EditorConfigurationDlg::OnAutoCompKeyword(wxCommandEvent& event)
{
    wxListBox* lstKeyword = XRCCTRL(*this, "lstAutoCompKeyword", wxListBox);
    if (lstKeyword->GetSelection() == m_LastAutoCompKeyword)
        return;

    AutoCompUpdate(m_LastAutoCompKeyword);
    // list new keyword's code
    m_AutoCompTextControl->SetText(m_AutoCompMap[lstKeyword->GetString(lstKeyword->GetSelection())]);
    m_LastAutoCompKeyword = lstKeyword->GetSelection();
}

void EditorConfigurationDlg::OnOK(wxCommandEvent& event)
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("editor"));

    cfg->Write(_T("/font"), XRCCTRL(*this, "lblEditorFont", wxStaticText)->GetFont().GetNativeFontInfoDesc());

    cfg->Write(_T("/auto_indent"),			XRCCTRL(*this, "chkAutoIndent", wxCheckBox)->GetValue());
    cfg->Write(_T("/smart_indent"),			XRCCTRL(*this, "chkSmartIndent", wxCheckBox)->GetValue());
    cfg->Write(_T("/use_tab"), 				XRCCTRL(*this, "chkUseTab", wxCheckBox)->GetValue());
    cfg->Write(_T("/show_indent_guides"), 	XRCCTRL(*this, "chkShowIndentGuides", wxCheckBox)->GetValue());
   	cfg->Write(_T("/tab_indents"), 			XRCCTRL(*this, "chkTabIndents", wxCheckBox)->GetValue());
   	cfg->Write(_T("/backspace_unindents"), 	XRCCTRL(*this, "chkBackspaceUnindents", wxCheckBox)->GetValue());
   	cfg->Write(_T("/word_wrap"), 			XRCCTRL(*this, "chkWordWrap", wxCheckBox)->GetValue());
   	cfg->Write(_T("/show_line_numbers"), 	XRCCTRL(*this, "chkShowLineNumbers", wxCheckBox)->GetValue());
   	cfg->Write(_T("/highlight_caret_line"), XRCCTRL(*this, "chkHighlightCaretLine", wxCheckBox)->GetValue());
   	cfg->Write(_T("/tab_size"),             XRCCTRL(*this, "spnTabSize", wxSpinCtrl)->GetValue());
   	cfg->Write(_T("/view_whitespace"),      XRCCTRL(*this, "cmbViewWS", wxComboBox)->GetSelection());
   	cfg->Write(_T("/tab_text_relative"),    XRCCTRL(*this, "rbTabText", wxRadioBox)->GetSelection() ? true : false);
   	cfg->Write(_T("/auto_wrap_search"),     XRCCTRL(*this, "chkAutoWrapSearch", wxCheckBox)->GetValue());
	//folding
   	cfg->Write(_T("/folding/show_folds"), 			XRCCTRL(*this, "chkEnableFolding", wxCheckBox)->GetValue());
   	cfg->Write(_T("/folding/fold_all_on_open"), 	XRCCTRL(*this, "chkFoldOnOpen", wxCheckBox)->GetValue());
   	cfg->Write(_T("/folding/fold_preprocessor"), 	XRCCTRL(*this, "chkFoldPreprocessor", wxCheckBox)->GetValue());
   	cfg->Write(_T("/folding/fold_comments"), 		XRCCTRL(*this, "chkFoldComments", wxCheckBox)->GetValue());
   	cfg->Write(_T("/folding/fold_xml"), 		    XRCCTRL(*this, "chkFoldXml", wxCheckBox)->GetValue());

	//eol
   	cfg->Write(_T("/show_eol"), 			        XRCCTRL(*this, "chkShowEOL", wxCheckBox)->GetValue());
   	cfg->Write(_T("/eol/strip_trailing_spaces"),    XRCCTRL(*this, "chkStripTrailings", wxCheckBox)->GetValue());
   	cfg->Write(_T("/eol/ensure_final_line_end"),    XRCCTRL(*this, "chkEnsureFinalEOL", wxCheckBox)->GetValue());
   	cfg->Write(_T("/eol/ensure_consistent_line_ends"), XRCCTRL(*this, "chkEnsureConsistentEOL", wxCheckBox)->GetValue());
    cfg->Write(_T("/eol/eolmode"),                  XRCCTRL(*this, "cmbEOLMode", wxComboBox)->GetSelection());

	//gutter
    cfg->Write(_T("/gutter/mode"), 			XRCCTRL(*this, "lstGutterMode", wxChoice)->GetSelection());
    cfg->Write(_T("/gutter/color"),		    XRCCTRL(*this, "btnGutterColor", wxButton)->GetBackgroundColour());
    cfg->Write(_T("/gutter/column"), 		XRCCTRL(*this, "spnGutterColumn", wxSpinCtrl)->GetValue());

	int sel = XRCCTRL(*this, "cmbDefCodeFileType", wxComboBox)->GetSelection();
    wxString key;
    key.Printf(_T("/default_code/set%d"), IdxToFileType[sel]);
    cfg->Write(key, XRCCTRL(*this, "txtDefCode", wxTextCtrl)->GetValue());

	if (m_Theme)
	{
		m_Theme->Save();
		Manager::Get()->GetEditorManager()->SetColorSet(m_Theme);
        cfg->Write(_T("/color_sets/active_color_set"), m_Theme->GetName());
	}
    cfg->Write(_T("/color_sets/active_lang"), XRCCTRL(*this, "cmbLangs", wxComboBox)->GetStringSelection());

    // save any changes in auto-completion
    wxListBox* lstKeyword = XRCCTRL(*this, "lstAutoCompKeyword", wxListBox);
    AutoCompUpdate(lstKeyword->GetSelection());
    AutoCompleteMap& map = Manager::Get()->GetEditorManager()->GetAutoCompleteMap();
    map = m_AutoCompMap;

    EndModal(wxID_OK);
}