#include <sdk.h>
#ifndef CB_PRECOMP
    #include <manager.h>
    #include <messagemanager.h>
    #include <configmanager.h>
    #include <projectmanager.h>
    #include <scriptingmanager.h>
    #include <compilerfactory.h>
    #include <compiler.h>
#endif
#include <sqplus.h>

#include "wizpage.h"
#include "intropanel.h"
#include "projectpathpanel.h"
#include "compilerpanel.h"
#include "buildtargetpanel.h"
#include "languagepanel.h"
#include "filepathpanel.h"
#include "genericselectpath.h"

// utility function to append a path separator to the
// string parameter, if needed.
wxString AppendPathSepIfNeeded(const wxString& path)
{
    if (path.Last() == _T('/') || path.Last() == _T('\\'))
        return path;
    return path + wxFILE_SEP_PATH;
}

////////////////////////////////////////////////////////////////////////////////
// WizPage
////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(WizPageBase, wxWizardPageSimple)
    EVT_WIZARD_PAGE_CHANGING(-1, WizPageBase::OnPageChanging)
    EVT_WIZARD_PAGE_CHANGED(-1, WizPageBase::OnPageChanged)
END_EVENT_TABLE()

WizPageBase::WizPageBase(const wxString& pageName, wxWizard* parent, const wxBitmap& bitmap)
    : wxWizardPageSimple(parent, 0, 0, bitmap),
    m_PageName(pageName)
{
}

//------------------------------------------------------------------------------
WizPageBase::~WizPageBase()
{
}

//------------------------------------------------------------------------------
void WizPageBase::OnPageChanging(wxWizardEvent& event)
{
    try
    {
        wxString sig = _T("OnLeave_") + m_PageName;
        SqPlus::SquirrelFunction<bool> cb(cbU2C(sig));
        if (cb.func.IsNull())
            return;
        bool allow = cb(event.GetDirection() != 0); // !=0 forward, ==0 backward
        if (!allow)
            event.Veto();
    }
    catch (SquirrelError& e)
    {
        Manager::Get()->GetScriptingManager()->DisplayErrors(&e);
    }
}

//------------------------------------------------------------------------------
void WizPageBase::OnPageChanged(wxWizardEvent& event)
{
    try
    {
        wxString sig = _T("OnEnter_") + m_PageName;
        SqPlus::SquirrelFunction<void> cb(cbU2C(sig));
        if (cb.func.IsNull())
            return;
        cb(event.GetDirection() != 0); // !=0 forward, ==0 backward
    }
    catch (SquirrelError& e)
    {
        Manager::Get()->GetScriptingManager()->DisplayErrors(&e);
    }
}

////////////////////////////////////////////////////////////////////////////////
// WizPage
////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(WizPage, WizPageBase)
    EVT_BUTTON(-1, WizPage::OnButton)
END_EVENT_TABLE()

WizPage::WizPage(const wxString& panelName, wxWizard* parent, const wxBitmap& bitmap)
    : WizPageBase(panelName, parent, bitmap)
{
    wxXmlResource::Get()->LoadPanel(this, panelName);
}

//------------------------------------------------------------------------------
WizPage::~WizPage()
{
}

//------------------------------------------------------------------------------
void WizPage::OnButton(wxCommandEvent& event)
{
    wxWindow* win = FindWindowById(event.GetId(), this);
    if (!win)
    {
        LOGSTREAM << _T("Can't locate window with id ") << event.GetId() << _T("!\n");
        return;
    }
    try
    {
        wxString sig = _T("OnClick_") + win->GetName();
        SqPlus::SquirrelFunction<void> cb(cbU2C(sig));
        if (cb.func.IsNull())
            return;
        cb();
    }
    catch (SquirrelError& e)
    {
        Manager::Get()->GetScriptingManager()->DisplayErrors(&e);
    }
}

////////////////////////////////////////////////////////////////////////////////
// WizIntroPanel
////////////////////////////////////////////////////////////////////////////////

WizIntroPanel::WizIntroPanel(const wxString& intro_msg, wxWizard* parent, const wxBitmap& bitmap)
    : WizPageBase(_T("IntroPage"), parent, bitmap)
{
    IntroPanel* pnl = new IntroPanel(this);
    pnl->SetIntroText(intro_msg);
}

//------------------------------------------------------------------------------
WizIntroPanel::~WizIntroPanel()
{
}

////////////////////////////////////////////////////////////////////////////////
// WizFilePathPanel
////////////////////////////////////////////////////////////////////////////////

WizFilePathPanel::WizFilePathPanel(bool showHeaderGuard, wxWizard* parent, const wxBitmap& bitmap)
    : WizPageBase(_T("FilePathPage"), parent, bitmap),
    m_AddToProject(false),
    m_TargetIndex(0)
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("scripts"));
    m_pFilePathPanel = new FilePathPanel(this);

    m_pFilePathPanel->ShowHeaderGuard(showHeaderGuard);
    m_pFilePathPanel->SetAddToProject(cfg->ReadBool(_T("/generic_wizard/add_file_to_project"), true));
}

//------------------------------------------------------------------------------
WizFilePathPanel::~WizFilePathPanel()
{
}

void WizFilePathPanel::SetFilePathSelectionFilter(const wxString& filter)
{
    m_pFilePathPanel->SetFilePathSelectionFilter(filter);
}

void WizFilePathPanel::OnPageChanging(wxWizardEvent& event)
{
    if (event.GetDirection() != 0) // !=0 forward, ==0 backward
    {
        m_Filename = m_pFilePathPanel->GetFilename();
        m_HeaderGuard = m_pFilePathPanel->GetHeaderGuard();
        m_AddToProject = m_pFilePathPanel->GetAddToProject();
        m_TargetIndex = m_pFilePathPanel->GetTargetIndex();

        if (m_Filename.IsEmpty() || !wxDirExists(wxPathOnly(m_Filename)))
        {
            cbMessageBox(_("Please select a filename for your new file..."), _("Error"), wxICON_ERROR);
            event.Veto();
            return;
        }

        ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("scripts"));
        cfg->Write(_T("/generic_wizard/add_file_to_project"), (bool)m_pFilePathPanel->GetAddToProject());
    }
    WizPageBase::OnPageChanging(event); // let the base class handle it too
}

////////////////////////////////////////////////////////////////////////////////
// WizProjectPathPanel
////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(WizProjectPathPanel, WizPageBase)
    EVT_BUTTON(-1, WizProjectPathPanel::OnButton)
END_EVENT_TABLE()

WizProjectPathPanel::WizProjectPathPanel(wxWizard* parent, const wxBitmap& bitmap)
    : WizPageBase(_T("ProjectPathPage"), parent, bitmap)
{
    m_pProjectPathPanel = new ProjectPathPanel(this);
}

//------------------------------------------------------------------------------
WizProjectPathPanel::~WizProjectPathPanel()
{
}

//------------------------------------------------------------------------------
wxString WizProjectPathPanel::GetPath()
{
    return AppendPathSepIfNeeded(m_pProjectPathPanel->GetPath());
}

//------------------------------------------------------------------------------
wxString WizProjectPathPanel::GetName()
{
    return m_pProjectPathPanel->GetName();
}

//------------------------------------------------------------------------------
wxString WizProjectPathPanel::GetFullFileName()
{
    return m_pProjectPathPanel->GetFullFileName();
}

//------------------------------------------------------------------------------
wxString WizProjectPathPanel::GetTitle()
{
    return m_pProjectPathPanel->GetTitle();
}

//------------------------------------------------------------------------------
void WizProjectPathPanel::OnButton(wxCommandEvent& event)
{
    wxString dir = m_pProjectPathPanel->GetPath();
    dir = ChooseDirectory(0, _("Please select the folder to create your project in"), dir, wxEmptyString, false, true);
    if (!dir.IsEmpty() && wxDirExists(dir))
        m_pProjectPathPanel->SetPath(dir);
}

//------------------------------------------------------------------------------
void WizProjectPathPanel::OnPageChanging(wxWizardEvent& event)
{
    if (event.GetDirection() != 0) // !=0 forward, ==0 backward
	{
	    wxString dir = m_pProjectPathPanel->GetPath();
	    wxString name = m_pProjectPathPanel->GetName();
	    wxString fullname = m_pProjectPathPanel->GetFullFileName();
	    wxString title = m_pProjectPathPanel->GetTitle();
//		if (!wxDirExists(dir))
//		{
//            cbMessageBox(_("Please select a valid path to create your project..."), _("Error"), wxICON_ERROR);
//            event.Veto();
//            return;
//		}
		if (title.IsEmpty())
		{
            cbMessageBox(_("Please select a title for your project..."), _("Error"), wxICON_ERROR);
            event.Veto();
            return;
		}
		if (name.IsEmpty())
		{
            cbMessageBox(_("Please select a name for your project..."), _("Error"), wxICON_ERROR);
            event.Veto();
            return;
		}
		if (wxFileExists(fullname))
		{
            if (cbMessageBox(_("A project with the same name already exists in the project folder.\n"
                        "Are you sure you want to use this directory (files may be OVERWRITTEN)?"),
                        _("Confirmation"),
                        wxICON_QUESTION | wxYES_NO) != wxID_YES)
            {
//                cbMessageBox(_("A project with the same name already exists in the project folder.\n"
//                            "Please select a different project name..."), _("Warning"), wxICON_WARNING);
                event.Veto();
                return;
            }
		}
        Manager::Get()->GetProjectManager()->SetDefaultPath(dir);
	}
    WizPageBase::OnPageChanging(event); // let the base class handle it too
}

//------------------------------------------------------------------------------
void WizProjectPathPanel::OnPageChanged(wxWizardEvent& event)
{
    if (event.GetDirection() != 0) // !=0 forward, ==0 backward
    {
        wxString dir = Manager::Get()->GetProjectManager()->GetDefaultPath();
        m_pProjectPathPanel->SetPath(dir);
    }
    WizPageBase::OnPageChanged(event); // let the base class handle it too
}

////////////////////////////////////////////////////////////////////////////////
// WizGenericSelectPathPanel
////////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(WizGenericSelectPathPanel, WizPageBase)
    EVT_BUTTON(-1, WizGenericSelectPathPanel::OnButton)
END_EVENT_TABLE()

WizGenericSelectPathPanel::WizGenericSelectPathPanel(const wxString& pageId, const wxString& descr, const wxString& label,
                                            wxWizard* parent, const wxBitmap& bitmap)
    : WizPageBase(pageId, parent, bitmap)
{
    m_pGenericSelectPath = new GenericSelectPath(this);
    m_pGenericSelectPath->txtFolder->SetValue(Manager::Get()->GetConfigManager(_T("project_wizard"))->Read(_T("/generic_paths/") + pageId));
    m_pGenericSelectPath->SetDescription(descr);
    m_pGenericSelectPath->lblLabel->SetLabel(label);
}

//------------------------------------------------------------------------------
WizGenericSelectPathPanel::~WizGenericSelectPathPanel()
{
}

//------------------------------------------------------------------------------
void WizGenericSelectPathPanel::OnButton(wxCommandEvent& event)
{
    wxString dir = m_pGenericSelectPath->txtFolder->GetValue();
    dir = ChooseDirectory(0, _("Please select location"), dir, wxEmptyString, false, true);
    if (!dir.IsEmpty() && wxDirExists(dir))
        m_pGenericSelectPath->txtFolder->SetValue(dir);
}

//------------------------------------------------------------------------------
void WizGenericSelectPathPanel::OnPageChanging(wxWizardEvent& event)
{
    if (event.GetDirection() != 0) // !=0 forward, ==0 backward
	{
	    wxString dir = m_pGenericSelectPath->txtFolder->GetValue();
		if (!wxDirExists(dir))
		{
            cbMessageBox(_("Please select a valid location..."), _("Error"), wxICON_ERROR);
            event.Veto();
            return;
		}
	}
    WizPageBase::OnPageChanging(event); // let the base class handle it too

    if (event.GetDirection() != 0 && event.IsAllowed())
    {
        Manager::Get()->GetConfigManager(_T("project_wizard"))->Write(_T("/generic_paths/") + m_PageName, m_pGenericSelectPath->txtFolder->GetValue());
    }
}

////////////////////////////////////////////////////////////////////////////////
// WizCompilerPanel
////////////////////////////////////////////////////////////////////////////////

WizCompilerPanel::WizCompilerPanel(const wxString& compilerID, const wxString& validCompilerIDs, wxWizard* parent, const wxBitmap& bitmap,
                                    bool allowCompilerChange, bool allowConfigChange)
    : WizPageBase(_T("CompilerPage"), parent, bitmap)
{
    m_pCompilerPanel = new CompilerPanel(this);

    wxArrayString valids = GetArrayFromString(validCompilerIDs, _T(";"), true);
    wxString def = compilerID;
    if (def.IsEmpty())
        def = CompilerFactory::GetDefaultCompiler()->GetName();
    int id = 0;
    wxComboBox* cmb = m_pCompilerPanel->GetCompilerCombo();
    cmb->Clear();
    for (size_t i = 0; i < CompilerFactory::GetCompilersCount(); ++i)
    {
        for (size_t n = 0; n < valids.GetCount(); ++n)
        {
            if (CompilerFactory::GetCompiler(i)->GetID().Matches(valids[n]))
            {
                cmb->Append(CompilerFactory::GetCompiler(i)->GetName());
                if (CompilerFactory::GetCompiler(i)->GetID().IsSameAs(def))
                    id = cmb->GetCount();
                break;
            }
        }
    }
    cmb->SetSelection(id);
    cmb->Enable(allowCompilerChange);
    m_pCompilerPanel->EnableConfigurationTargets(allowConfigChange);

    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("scripts"));

    m_pCompilerPanel->SetWantDebug(cfg->ReadBool(_T("/generic_wizard/want_debug"), true));
    m_pCompilerPanel->SetDebugName(cfg->Read(_T("/generic_wizard/debug_name"), _T("Debug")));
    m_pCompilerPanel->SetDebugOutputDir(cfg->Read(_T("/generic_wizard/debug_output"), _T("bin") + wxString(wxFILE_SEP_PATH) + _T("Debug")));
    m_pCompilerPanel->SetDebugObjectOutputDir(cfg->Read(_T("/generic_wizard/debug_objects_output"), _T("obj") + wxString(wxFILE_SEP_PATH) + _T("Debug")));

    m_pCompilerPanel->SetWantRelease(cfg->ReadBool(_T("/generic_wizard/want_release"), true));
    m_pCompilerPanel->SetReleaseName(cfg->Read(_T("/generic_wizard/release_name"), _T("Release")));
    m_pCompilerPanel->SetReleaseOutputDir(cfg->Read(_T("/generic_wizard/release_output"), _T("bin") + wxString(wxFILE_SEP_PATH) + _T("Release")));
    m_pCompilerPanel->SetReleaseObjectOutputDir(cfg->Read(_T("/generic_wizard/release_objects_output"), _T("obj") + wxString(wxFILE_SEP_PATH) + _T("Release")));
}

//------------------------------------------------------------------------------
WizCompilerPanel::~WizCompilerPanel()
{
}

//------------------------------------------------------------------------------
wxString WizCompilerPanel::GetCompilerID()
{
    Compiler* compiler = CompilerFactory::GetCompilerByName(m_pCompilerPanel->GetCompilerCombo()->GetStringSelection());
    if (compiler)
        return compiler->GetID();
    return wxEmptyString;
}

//------------------------------------------------------------------------------
bool WizCompilerPanel::GetWantDebug()
{
    return m_pCompilerPanel->GetWantDebug();
}

//------------------------------------------------------------------------------
wxString WizCompilerPanel::GetDebugName()
{
    return m_pCompilerPanel->GetDebugName();
}

//------------------------------------------------------------------------------
wxString WizCompilerPanel::GetDebugOutputDir()
{
    return AppendPathSepIfNeeded(m_pCompilerPanel->GetDebugOutputDir());
}

//------------------------------------------------------------------------------
wxString WizCompilerPanel::GetDebugObjectOutputDir()
{
    return AppendPathSepIfNeeded(m_pCompilerPanel->GetDebugObjectOutputDir());
}

//------------------------------------------------------------------------------
bool WizCompilerPanel::GetWantRelease()
{
    return m_pCompilerPanel->GetWantRelease();
}

//------------------------------------------------------------------------------
wxString WizCompilerPanel::GetReleaseName()
{
    return m_pCompilerPanel->GetReleaseName();
}

//------------------------------------------------------------------------------
wxString WizCompilerPanel::GetReleaseOutputDir()
{
    return AppendPathSepIfNeeded(m_pCompilerPanel->GetReleaseOutputDir());
}

//------------------------------------------------------------------------------
wxString WizCompilerPanel::GetReleaseObjectOutputDir()
{
    return AppendPathSepIfNeeded(m_pCompilerPanel->GetReleaseObjectOutputDir());
}

//------------------------------------------------------------------------------
void WizCompilerPanel::OnPageChanging(wxWizardEvent& event)
{
    if (event.GetDirection() != 0) // !=0 forward, ==0 backward
	{
        if (GetCompilerID().IsEmpty())
        {
            wxMessageBox(_("You must select a compiler for your project..."), _("Error"), wxICON_ERROR);
            event.Veto();
            return;
        }
        if (!GetWantDebug() && !GetWantRelease())
        {
            wxMessageBox(_("You must select at least one configuration..."), _("Error"), wxICON_ERROR);
            event.Veto();
            return;
        }

        ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("scripts"));

        cfg->Write(_T("/generic_wizard/want_debug"), (bool)GetWantDebug());
        cfg->Write(_T("/generic_wizard/debug_name"), GetDebugName());
        cfg->Write(_T("/generic_wizard/debug_output"), GetDebugOutputDir());
        cfg->Write(_T("/generic_wizard/debug_objects_output"), GetDebugObjectOutputDir());

        cfg->Write(_T("/generic_wizard/want_release"), (bool)GetWantRelease());
        cfg->Write(_T("/generic_wizard/release_name"), GetReleaseName());
        cfg->Write(_T("/generic_wizard/release_output"), GetReleaseOutputDir());
        cfg->Write(_T("/generic_wizard/release_objects_output"), GetReleaseObjectOutputDir());
	}
    WizPageBase::OnPageChanging(event); // let the base class handle it too
}

////////////////////////////////////////////////////////////////////////////////
// WizBuildTargetPanel
////////////////////////////////////////////////////////////////////////////////

WizBuildTargetPanel::WizBuildTargetPanel(const wxString& targetName, bool isDebug,
                                    wxWizard* parent, const wxBitmap& bitmap,
                                    bool showCompiler,
                                    const wxString& compilerID, const wxString& validCompilerIDs,
                                    bool allowCompilerChange)
    : WizPageBase(_T("BuildTargetPage"), parent, bitmap)
{
    m_pBuildTargetPanel = new BuildTargetPanel(this);
    m_pBuildTargetPanel->SetTargetName(targetName);
    m_pBuildTargetPanel->SetEnableDebug(isDebug);
    m_pBuildTargetPanel->ShowCompiler(showCompiler);

    if (showCompiler)
    {
        wxArrayString valids = GetArrayFromString(validCompilerIDs, _T(";"), true);
        wxString def = compilerID;
        if (def.IsEmpty())
            def = CompilerFactory::GetDefaultCompiler()->GetName();
        int id = 0;
        wxComboBox* cmb = m_pBuildTargetPanel->GetCompilerCombo();
        cmb->Clear();
        for (size_t i = 0; i < CompilerFactory::GetCompilersCount(); ++i)
        {
            for (size_t n = 0; n < valids.GetCount(); ++n)
            {
                if (CompilerFactory::GetCompiler(i)->GetID().Matches(valids[n]))
                {
                    cmb->Append(CompilerFactory::GetCompiler(i)->GetName());
                    if (CompilerFactory::GetCompiler(i)->GetID().IsSameAs(def))
                        id = cmb->GetCount();
                    break;
                }
            }
        }
        cmb->SetSelection(id);
        cmb->Enable(allowCompilerChange);
    }
}

//------------------------------------------------------------------------------
WizBuildTargetPanel::~WizBuildTargetPanel()
{
}

//------------------------------------------------------------------------------
wxString WizBuildTargetPanel::GetCompilerID()
{
    if (!m_pBuildTargetPanel->GetCompilerCombo()->IsShown())
        return wxEmptyString;

    Compiler* compiler = CompilerFactory::GetCompilerByName(m_pBuildTargetPanel->GetCompilerCombo()->GetStringSelection());
    if (compiler)
        return compiler->GetID();
    return wxEmptyString;
}

//------------------------------------------------------------------------------
bool WizBuildTargetPanel::GetEnableDebug()
{
    return m_pBuildTargetPanel->GetEnableDebug();
}

//------------------------------------------------------------------------------
wxString WizBuildTargetPanel::GetTargetName()
{
    return m_pBuildTargetPanel->GetTargetName();
}

//------------------------------------------------------------------------------
wxString WizBuildTargetPanel::GetTargetOutputDir()
{
    return AppendPathSepIfNeeded(m_pBuildTargetPanel->GetOutputDir());
}

//------------------------------------------------------------------------------
wxString WizBuildTargetPanel::GetTargetObjectOutputDir()
{
    return AppendPathSepIfNeeded(m_pBuildTargetPanel->GetObjectOutputDir());
}

//------------------------------------------------------------------------------
void WizBuildTargetPanel::OnPageChanging(wxWizardEvent& event)
{
    if (event.GetDirection() != 0) // !=0 forward, ==0 backward
	{
        if (m_pBuildTargetPanel->GetCompilerCombo()->IsShown() && GetCompilerID().IsEmpty())
        {
            wxMessageBox(_("You must select a compiler for your build target..."), _("Error"), wxICON_ERROR);
            event.Veto();
            return;
        }

        cbProject* theproject = Manager::Get()->GetProjectManager()->GetActiveProject(); // can't fail; if no project, the wizard didn't even run
        if (theproject->GetBuildTarget(m_pBuildTargetPanel->GetTargetName()))
        {
            wxMessageBox(_("A build target with that name already exists in the active project..."), _("Error"), wxICON_ERROR);
            event.Veto();
            return;
        }
	}
    WizPageBase::OnPageChanging(event); // let the base class handle it too
}

////////////////////////////////////////////////////////////////////////////////
// WizLanguagePanel
////////////////////////////////////////////////////////////////////////////////

WizLanguagePanel::WizLanguagePanel(const wxArrayString& langs, int defLang, wxWizard* parent, const wxBitmap& bitmap)
    : WizPageBase(_T("LanguagePage"), parent, bitmap)
{
    m_pLanguagePanel = new LanguagePanel(this);
    m_pLanguagePanel->SetChoices(langs, defLang);
}

//------------------------------------------------------------------------------
WizLanguagePanel::~WizLanguagePanel()
{
}

int WizLanguagePanel::GetLanguage()
{
    return m_pLanguagePanel->GetLanguage();
}

void WizLanguagePanel::SetLanguage(int lang)
{
    m_pLanguagePanel->SetLanguage(lang);
}