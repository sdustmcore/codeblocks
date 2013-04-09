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
* $Revision$
* $Id$
* $HeadURL$
*/

#include <sdk.h>
#include "app.h"
#include <wx/fs_zip.h>
#include <wx/xrc/xmlres.h>
#include <wx/cmdline.h>
#include <wx/regex.h>
#include <wx/filefn.h>
#include <wx/msgdlg.h>
#include <wx/choicdlg.h>
#include <wx/notebook.h>
#include <wx/clipbrd.h>
#include <wx/taskbar.h>
#include <cbexception.h>
#if wxCHECK_VERSION(2,6,0)
    #include <wx/debugrpt.h>
#endif
#include <configmanager.h>
#include <editormanager.h>
#include <projectmanager.h>
#include <personalitymanager.h>
#include <pluginmanager.h>
#include <sdk_events.h>
#include <manager.h>
#include <scriptingmanager.h>
#include <wxFlatNotebook.h>
#include <globals.h>
#include "splashscreen.h"
#include <wx/arrstr.h>
#include "crashhandler.h"

#include <sqplus.h>

#ifndef __WXMSW__
    #include "prefix.h" // binreloc
#else
    #include "associations.h"
#endif

#include "appglobals.h"

namespace
{
#if wxUSE_CMDLINE_PARSER
static const wxCmdLineEntryDesc cmdLineDesc[] =
{
    { wxCMD_LINE_SWITCH, _T("h"), _T("help"), _T("show this help message"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
#ifdef __WXMSW__
    { wxCMD_LINE_SWITCH, _T("na"), _T("no-check-associations"), _T("don't perform any association checks"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
    { wxCMD_LINE_SWITCH, _T("nd"), _T("no-dde"), _T("don't start a DDE server"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
#endif
    { wxCMD_LINE_SWITCH, _T("ns"), _T("no-splash-screen"), _T("don't display a splash screen while loading"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
    { wxCMD_LINE_SWITCH, _T("d"), _T("debug-log"), _T("display application's debug log"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
    { wxCMD_LINE_OPTION, _T(""), _T("prefix"),  _T("the shared data dir prefix"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_NEEDS_SEPARATOR },
    { wxCMD_LINE_OPTION, _T("p"), _T("personality"),  _T("the personality to use: \"ask\" or <personality-name>"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_NEEDS_SEPARATOR },
    { wxCMD_LINE_OPTION, _T(""), _T("profile"),  _T("synonym to personality"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_NEEDS_SEPARATOR },
    { wxCMD_LINE_SWITCH, _T(""), _T("rebuild"), _T("clean and then build the project/workspace"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
    { wxCMD_LINE_SWITCH, _T(""), _T("build"), _T("just build the project/workspace"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
    { wxCMD_LINE_OPTION, _T(""), _T("target"),  _T("the target for the batch build"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_NEEDS_SEPARATOR },
    { wxCMD_LINE_SWITCH, _T(""), _T("no-batch-window-close"),  _T("do not auto-close log window when batch build is done"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
    { wxCMD_LINE_SWITCH, _T(""), _T("batch-build-notify"),  _T("show message when batch build is done"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
    { wxCMD_LINE_PARAM, _T(""), _T(""),  _T("filename(s)"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },
    { wxCMD_LINE_NONE }
};
#endif // wxUSE_CMDLINE_PARSER

#ifdef __WXMSW__
DDEServer* g_DDEServer = 0L;
#endif

// this list will be filled with DDE files to load after the app has started up
wxArrayString s_DdeFilesToOpen;
bool s_Loading = false;

class Splash
{
    public:
        Splash(const bool show) : m_pSplash(0)
        {
            if (show)
            {
                wxBitmap bmp = LoadPNGWindows2000Hack(ConfigManager::ReadDataPath() + _T("/images/splash_new.png"));
                m_pSplash = new cbSplashScreen(bmp, -1, 0, -1, wxNO_BORDER | wxFRAME_NO_TASKBAR);
                Manager::Yield();
            }
        }
        ~Splash() { Hide(); }
        void Hide()
        {
            if (m_pSplash)
            {
                m_pSplash->Destroy();
                m_pSplash = 0;
            }
        }
    private:
        cbSplashScreen* m_pSplash;
};
}; // namespace

IMPLEMENT_APP(CodeBlocksApp)

BEGIN_EVENT_TABLE(CodeBlocksApp, wxApp)
	EVT_ACTIVATE_APP(CodeBlocksApp::OnAppActivate)
	EVT_COMPILER_FINISHED(CodeBlocksApp::OnBatchBuildDone)
END_EVENT_TABLE()

bool CodeBlocksApp::LoadConfig()
{
    if (ParseCmdLine(0L) != 0)

        return false;

    ConfigManager *cfg = Manager::Get()->GetConfigManager(_T("app"));
    cfg->Write(_T("app_path"), GetAppPath());

    // find out about data path
#ifdef __WXMSW__
    wxString data = GetAppPath(); // under windows it is under the exe dir
#else
    #ifdef APP_PREFIX
        wxString data = wxT(APP_PREFIX); // under linux, get the preprocessor value
    #else
        wxString data = GetAppPath();
    #endif
#endif
    wxString actualData = _T("/share/codeblocks");
    data << actualData;
    // check if the user has passed --prefix in the command line
    if (!m_Prefix.IsEmpty())
        data = m_Prefix + actualData;
    else
    {
        // if no --prefix passed, check for the environment variable
        wxString env;
        wxGetEnv(_T("CODEBLOCKS_DATA_DIR"), &env);
        if (!env.IsEmpty())
            data = env + actualData;
    }

    cfg->Write(_T("data_path"), data);
    m_HasDebugLog = Manager::Get()->GetConfigManager(_T("message_manager"))->ReadBool(_T("/has_debug_log"), false) || m_HasDebugLog;
    Manager::Get()->GetConfigManager(_T("message_manager"))->Write(_T("/has_debug_log"), m_HasDebugLog);
    return true;
}

void CodeBlocksApp::InitAssociations(MainFrame* frame)
{
#ifdef __WXMSW__
	ConfigManager *cfg = Manager::Get()->GetConfigManager(_T("app"));
	if (!m_NoAssocs && cfg->ReadBool(_T("/environment/check_associations"), true))
	{
		if (!Associations::Check())
		{
            AskAssocDialog dlg(Manager::Get()->GetAppWindow());
            PlaceWindow(&dlg);

            switch(dlg.ShowModal())
            {
            case 0:
                Manager::Get()->GetConfigManager(_T("app"))->Write(_T("/environment/check_associations"), false);
                break;
            case 1:
                break;
            case 2:
                Associations::SetCore();
                break;
            case 3:
                Associations::SetAll();
                break;
            };

        }
    }
#endif
}

void CodeBlocksApp::InitDebugConsole()
{
#ifdef __WXMSW__
	#ifdef __CBDEBUG__
    // Remember to compile as a console application!
    AllocConsole();
    HANDLE myhandle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD co = {80,2000};
	SetConsoleScreenBufferSize(myhandle, co);
	fprintf(stdout,"CONSOLE DEBUG ACTIVATED\n");
	// wxLogWindow *myerr = new wxLogWindow(NULL,"debug");
	#endif
#endif
}

void CodeBlocksApp::InitExceptionHandler()
{
#ifdef __WXMSW__
    m_ExceptionHandlerLib = LoadLibrary(_T("exchndl.dll"));
#endif
}

void CodeBlocksApp::InitImageHandlers()
{
	wxInitAllImageHandlers();
}


bool CodeBlocksApp::InitXRCStuff()
{
    // load all application-resources
    wxFileSystem::AddHandler(new wxZipFSHandler);
    wxXmlResource::Get()->InitAllHandlers();

    wxString res = ConfigManager::ReadDataPath() + _T("/resources.zip");

    if (!CheckResource(res))
    	return false;
    /// @todo Checkout why it doesn't work with VC++ unless "#zip:*.xrc" appended
	wxXmlResource::Get()->Load(res+_T("#zip:*.xrc"));
	return true;
}

MainFrame* CodeBlocksApp::InitFrame()
{
    MainFrame *frame = new MainFrame();
    SetTopWindow(0);
    //frame->Hide(); // shouldn't need this explicitely
#ifdef __WXMSW__
    if (!m_NoDDE)
    {
        g_DDEServer = new DDEServer(frame);
        g_DDEServer->Create(DDE_SERVICE);
        g_DDEServer->SetFrame(frame);
    }
#endif
    if (ParseCmdLine(frame) == 0)
    {
        if (Manager::Get()->GetConfigManager(_T("app"))->ReadBool(_T("/environment/blank_workspace"), true) == false)
            Manager::Get()->GetProjectManager()->LoadWorkspace(DEFAULT_WORKSPACE);
    }

    if (!m_Batch)
    {
#ifdef wxUSE_BINDERAPP
        SetGlobalHandler(frame);
        SetGlobalBinder(frame->m_KeyProfiles.Item(0));
#endif
    }
    return frame;
}

void CodeBlocksApp::CheckVersion()
{
#ifdef __WXMSW__
    // for windows users only, display a message that no compiler is provided
    ConfigManager *cfg = Manager::Get()->GetConfigManager(_T("app"));

    if (cfg->Read(_T("version")) != g_AppActualVersion)
    {
        // this is a (probably) newer version; show a message box with
        // important notes

        // NOTE:
        // the info box, has moved to the installer, because now there are
        // setup files including a compiler...

        // update the version
        cfg->Write(_T("version"), g_AppActualVersion);
    }
#endif
}

void CodeBlocksApp::InitLocale()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("app"));


    bool i18n=cfg->ReadBool(_T("/environment/I18N"),LOCALIZE);
	int lng =-1;	// -1 = Don't use locale; the default is 0 = Use locale
	if(i18n)
		lng = cfg->ReadInt(_T("/locale/language"),(int)0);

	int catalogNum = cfg->ReadInt(_T("/locale/catalogNum"), (int)0);
	if (catalogNum == 0)
	{
		catalogNum = 1;
		cfg->Write(_T("/locale/Domain1"), "codeblocks");
	}

    if (lng>=0)
    {
        m_locale.Init(locales[lng]);
        wxLocale::AddCatalogLookupPathPrefix(ConfigManager::GetDataFolder() + _T("/locale"));
        wxLocale::AddCatalogLookupPathPrefix(wxT("."));
        wxLocale::AddCatalogLookupPathPrefix(wxT(".."));
		for (int i = 1; i <= catalogNum; ++i)
		{
			wxString tempStr = wxString::Format(_T("/locale/Domain%d"), i);
			wxString catalogName = cfg->Read(tempStr, wxEmptyString);
			if (catalogName.IsEmpty())
			{
				cfg->Write(tempStr,
                        cfg->Read(wxString::Format(_T("/locale/Domain%d"), catalogNum)));
				catalogNum--;
			}
			else if (cfg->Read(_T("/plugins/") + catalogName))
				m_locale.AddCatalog(catalogName);
		}

    }
	cfg->Write(_T("/locale/catalogNum"), (int)catalogNum);
	cfg->Write(_T("/environment/I18N"),  (bool)i18n);
    cfg->Write(_T("/locale/language"), (int)lng);
}

bool CodeBlocksApp::OnInit()
{
    s_Loading = true;
    m_Batch = false;
    m_BatchNotify = false;
    m_Build = false;
    m_ReBuild = false;
    m_BatchExitCode = 0;
    m_BatchWindowAutoClose = true;
    m_pBatchBuildDialog = 0;

	wxTheClipboard->Flush();

    static CrashHandler crash_handler;

    try
    {
    #if (wxUSE_ON_FATAL_EXCEPTION == 1)
        wxHandleFatalExceptions(true);
    #endif
        InitExceptionHandler();


        if(!LoadConfig())
            return false;

        InitImageHandlers();

        if(!InitXRCStuff())
        {
           // wsSafeShowMessage(_T("Fatal error"), _T("Initialisation of resources failed."));
            return false;
        }

        Splash splash(!m_Batch &&
                      !m_NoSplash &&
                      Manager::Get()->GetConfigManager(_T("app"))->ReadBool(_T("/environment/show_splash"), true));

        InitLocale();
        m_pSingleInstance = 0;
        if (Manager::Get()->GetConfigManager(_T("app"))->ReadBool(_T("/environment/single_instance"), true))
        {
            const wxString name = wxString::Format(_T("Code::Blocks-%s"), wxGetUserId().c_str());

			// this is a really Smart way of getting to the temp folder...
			// providing a function wxGetTempFolder would be far too easy
			wxString tempFolder;
			wxFile f;
			wxString wxIsJustAnnoying = wxFileName::CreateTempFileName(wxEmptyString, &f);

			if(!wxIsJustAnnoying.IsEmpty())
			{
			::wxRemoveFile(wxIsJustAnnoying); // hmph...!
			tempFolder = wxFileName(wxIsJustAnnoying).GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
			}

            m_pSingleInstance = new wxSingleInstanceChecker(name, tempFolder);
            if (m_pSingleInstance->IsAnotherRunning())
            {
                splash.Hide();
                wxLogError(_("Another program instance is already running.\nCode::Blocks is currently configured to only allow one running instance.\n\nYou can access this Setting under the menu item 'Environment'."));
                return false;
            }
        }

        InitDebugConsole();

        Manager::SetBatchBuild(m_Batch);
        MainFrame* frame = InitFrame();

        if (m_Batch)
        {
            s_Loading = false;
            DelayLoadDdeFiles(frame);
            BatchJob();
            frame->Close();
            return true;
        }

        CheckVersion();
        Manager::Get()->GetMessageManager()->DebugLog(_T("Initializing plugins..."));

        CodeBlocksEvent event(cbEVT_APP_STARTUP_DONE);
        Manager::Get()->ProcessEvent(event);

        // run startup script
        try
        {
            Manager::Get()->GetScriptingManager()->LoadScript(_T("startup.script"));
            SqPlus::SquirrelFunction<void>("main")();
        }
        catch (SquirrelError& exception)
        {
            Manager::Get()->GetScriptingManager()->DisplayErrors(&exception);
        }
        Manager::ProcessPendingEvents();

        // finally, show the app
        splash.Hide();
        SetTopWindow(frame);
        frame->Show();

        frame->ShowTips(); // this func checks if the user wants tips, so no need to check here
        InitAssociations(frame);

        s_Loading = false;
        DelayLoadDdeFiles(frame);

        return true;
    }
    catch (cbException& exception)
    {
        exception.ShowErrorMessage();
    }
    catch (SquirrelError& exception)
    {
        Manager::Get()->GetScriptingManager()->DisplayErrors(&exception);
    }
    catch (const char* message)
    {
        wxSafeShowMessage(_T("Exception"), cbC2U(message));
    }
    catch (...)
    {
        wxSafeShowMessage(_T("Exception"), _T("Unknown exception was raised. The application will terminate immediately..."));
    }
    // if we reached here, return error
    return false;
}

int CodeBlocksApp::OnExit()
{
	wxTheClipboard->Flush();

#ifdef __WXMSW__
	if (g_DDEServer)
		delete g_DDEServer;
    if (m_ExceptionHandlerLib)
        FreeLibrary(m_ExceptionHandlerLib);
#endif
    if (m_pSingleInstance)
        delete m_pSingleInstance;
    // WX docs say that this function's return value is ignored,
    // but we return our value anyway. It might not be ignored at some point...
    return m_Batch ? m_BatchExitCode : 0;
}

int CodeBlocksApp::OnRun()
{
    try
    {
        return wxApp::OnRun();
    }
    catch (cbException& exception)
    {
        exception.ShowErrorMessage();
    }
    catch (SquirrelError& exception)
    {
        Manager::Get()->GetScriptingManager()->DisplayErrors(&exception);
    }
    catch (const char* message)
    {
        wxSafeShowMessage(_("Exception"), cbC2U(message));
    }
    catch (...)
    {
        wxSafeShowMessage(_("Exception"), _("Unknown exception was raised. The application will terminate immediately..."));
    }
    // if we reached here, return error
    return -1;
}

bool CodeBlocksApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    return wxApp::OnCmdLineParsed(parser);
}

void CodeBlocksApp::OnFatalException()
{
#if wxCHECK_VERSION(2,6,0) && wxUSE_DEBUGREPORT && wxUSE_XML && wxUSE_ON_FATAL_EXCEPTION
    wxDebugReport report;
    wxDebugReportPreviewStd preview;

    report.AddAll();
    if ( preview.Show(report) )
        report.Process();
#else
    cbMessageBox(wxString::Format(_("Something has gone wrong inside %s and it "
                                    "will terminate immediately.\n"
                                    "We are sorry for the inconvenience..."), g_AppName.c_str()));
#endif
}

int CodeBlocksApp::BatchJob()
{
    if (!m_Batch)
        return -1;

    // find compiler plugin
    PluginsArray arr = Manager::Get()->GetPluginManager()->GetCompilerOffers();
    if (arr.GetCount() == 0)
        return -2;

    cbCompilerPlugin* compiler = static_cast<cbCompilerPlugin*>(arr[0]);
    if (!compiler)
        return -3;

    wxTaskBarIcon* tbIcon = new wxTaskBarIcon();
    tbIcon->SetIcon(
            #ifdef __WXMSW__
                wxICON(A_MAIN_ICON),
            #else
                wxIcon(app),
            #endif // __WXMSW__
                _("Building ") + wxFileNameFromPath(wxString(argv[argc-1])));

    m_pBatchBuildDialog = Manager::Get()->GetMessageManager()->GetBatchBuildDialog();
    PlaceWindow(m_pBatchBuildDialog);
    m_pBatchBuildDialog->Show();

    if (m_ReBuild)
        compiler->RebuildWorkspace(m_BatchTarget);
    else if (m_Build)
        compiler->BuildWorkspace(m_BatchTarget);

    // the batch build log might have been deleted in
    // CodeBlocksApp::OnBatchBuildDone().
    // if it hasn't, it's still compiling
    if (m_pBatchBuildDialog)
        m_pBatchBuildDialog->ShowModal();

    tbIcon->RemoveIcon();
    delete tbIcon;
    if (m_pBatchBuildDialog)
        m_pBatchBuildDialog->Destroy();
    m_pBatchBuildDialog = 0;

    return 0;
}

void CodeBlocksApp::OnBatchBuildDone(CodeBlocksEvent& event)
{
    event.Skip();
    // the event comes more than once. deal with it...
    static bool one_time_only = false;
    if (!m_Batch || one_time_only)
        return;
    one_time_only = true;

    cbCompilerPlugin* compiler = static_cast<cbCompilerPlugin*>(event.GetPlugin());
    m_BatchExitCode = compiler->GetExitCode();

    if (m_BatchNotify)
    {
        wxString msg;
        if (m_BatchExitCode == 0)
            msg << _("Batch build ended.\n");
        else
            msg << _("Batch build stopped with errors.\n");
        msg << wxString::Format(_("Process exited with status code %d."), m_BatchExitCode);
        cbMessageBox(msg, g_AppName, m_BatchExitCode == 0 ? wxICON_INFORMATION : wxICON_WARNING);
    }
    else
        wxBell();

    if (m_pBatchBuildDialog && m_BatchWindowAutoClose && (m_BatchExitCode == 0))
    {
        m_pBatchBuildDialog->EndModal(wxID_OK);
        m_pBatchBuildDialog->Destroy();
        m_pBatchBuildDialog = 0;
    }
}

bool CodeBlocksApp::CheckResource(const wxString& res)
{
    if (!wxFileExists(res))
    {
    	wxString msg;
    	msg.Printf(_T("Cannot find %s...\n"
    		"%s was configured to be installed in '%s'.\n"
    		"Please use the command-line switch '--prefix' or "
            "set the CODEBLOCKS_DATA_DIR environment variable "
    		"to point where %s is installed,\n"
    		"or try re-installing the application..."),
    		res.c_str(),
    		g_AppName.c_str(),
    		ConfigManager::ReadDataPath().c_str(),
    		g_AppName.c_str());
    	cbMessageBox(msg);
    	return false;
    }

    return true;
}

wxString CodeBlocksApp::GetAppPath() const
{
    wxString base;
#ifdef __WXMSW__
    wxChar name[MAX_PATH] = {0};
    GetModuleFileName(0L, name, MAX_PATH);
    wxFileName fname(name);
    base = fname.GetPath(wxPATH_GET_VOLUME);
#elif defined(__WXMAC__)
	// TODO: get the path
	base = _T(".");
#else
    if (!m_Prefix.IsEmpty())
        return m_Prefix;

    // SELFPATH is a macro from prefix.h (binreloc)
    // it returns the absolute filename of us
    // similar to win32 GetModuleFileName()...
    base = wxString(SELFPATH,wxConvUTF8);
    base = wxFileName(base).GetPath();
	if (base.IsEmpty())
		base = _T(".");
#endif
    return base;
}

int CodeBlocksApp::ParseCmdLine(MainFrame* handlerFrame)
{
    // code shamelessely taken from the console wxWindows sample :)
	bool filesInCmdLine = false;

#if wxUSE_CMDLINE_PARSER
    wxCmdLineParser parser(cmdLineDesc, argc, argv);
    // wxApp::argc is a wxChar**

    switch ( parser.Parse() )
    {
        case -1:
            return -1;

        case 0:
            {
                if (handlerFrame)
                {
                    int count = parser.GetParamCount();
					filesInCmdLine = count != 0;
					bool hasProj = false;
					bool hasWksp = false;
                    for ( int param = 0; param < count; ++param )
                    {
                        // is it a project/workspace?
                        FileType ft = FileTypeOf(parser.GetParam(param));
                        if (ft == ftCodeBlocksProject)
                        {
                            hasProj = true;
                            s_DdeFilesToOpen.Add(parser.GetParam(param));
                        }
                        else if (ft == ftCodeBlocksWorkspace)
                        {
                            // only one workspace can be opened
                            hasWksp = true;
                            s_DdeFilesToOpen.Clear(); // remove all other files
                            s_DdeFilesToOpen.Add(parser.GetParam(param)); // and add only the workspace
                            break; // and stop processing any more files
                        }
                    }

                    // batch jobs
                    m_Batch = hasProj || hasWksp;
                    m_Batch = m_Batch && (m_Build || m_ReBuild);
                }
                else
                {
                    wxString val;
                    parser.Found(_T("prefix"), &m_Prefix);
#ifdef __WXMSW__
					m_NoDDE = parser.Found(_T("no-dde"));
					m_NoAssocs = parser.Found(_T("no-check-associations"));
#else
                    m_NoDDE = false;
                    m_NoAssocs = false;
#endif
					m_NoSplash = parser.Found(_T("no-splash-screen"));
					m_HasDebugLog = parser.Found(_T("debug-log"));
					if (parser.Found(_T("personality"), &val) ||
                        parser.Found(_T("profile"), &val))
                    {
                        SetupPersonality(val);
                    }

                    // batch jobs
                    m_BatchNotify = parser.Found(_T("batch-build-notify"));
                    m_BatchWindowAutoClose = !parser.Found(_T("no-batch-window-close"));
                    m_Build = parser.Found(_T("build"));
                    m_ReBuild = parser.Found(_T("rebuild"));
                    parser.Found(_T("target"), &m_BatchTarget);

                    // initial setting for batch flag (will be reset when ParseCmdLine() is called again).
                    m_Batch = m_Build || m_ReBuild;
                }
            }
            break;

        default:
            return 1; // syntax error / unknown option
    }
#endif // wxUSE_CMDLINE_PARSER
    return filesInCmdLine ? 1 : 0;
}

void CodeBlocksApp::SetupPersonality(const wxString& personality)
{
    if (personality.CmpNoCase(_T("ask")) == 0)
    {
#if (wxMAJOR_VERSION == 2) && (wxMINOR_VERSION < 5)
        // wx < 2.5.x single choice dialog wants wxString*
        const wxArrayString& list = Manager::Get()->GetPersonalityManager()->GetPersonalitiesList();
        wxString* strings = new wxString[list.GetCount()];
        for (unsigned int i = 0; i < list.GetCount(); ++i)
            strings[i] = list[i];
#endif
        // display personality selection dialog
        wxSingleChoiceDialog dlg(0,
                                _("Please choose which personality (profile) to load:"),
                                _("Personalities (profiles)"),
#if (wxMAJOR_VERSION == 2) && (wxMINOR_VERSION < 5)
                                list.GetCount(), strings);
#else
                                Manager::Get()->GetPersonalityManager()->GetPersonalitiesList());
#endif
        PlaceWindow(&dlg);
        if (dlg.ShowModal() == wxID_OK)
            Manager::Get()->GetPersonalityManager()->SetPersonality(dlg.GetStringSelection());
#if (wxMAJOR_VERSION == 2) && (wxMINOR_VERSION < 5)
        delete[] strings;
#endif
    }
    else
        Manager::Get()->GetPersonalityManager()->SetPersonality(personality, true);
}

void CodeBlocksApp::DelayLoadDdeFiles(MainFrame* frame)
{
    for (size_t i = 0; i < s_DdeFilesToOpen.GetCount(); ++i)
    {
        frame->Open(s_DdeFilesToOpen[i], true);
    }
    s_DdeFilesToOpen.Clear();
}

#ifdef __WXMSW__
//// DDE

wxConnectionBase* DDEServer::OnAcceptConnection(const wxString& topic)
{
    if (topic == DDE_TOPIC)
        return new DDEConnection(m_Frame);

    // unknown topic
    return 0L;
}

bool DDEConnection::OnExecute(const wxString& topic, wxChar *data, int size, wxIPCFormat format)
{
	wxString strData = data;
	if (!strData.StartsWith(_T("[Open(\"")))
		return false;

	wxRegEx reCmd(_T("\"(.*)\""));
	if (reCmd.Matches(strData))
	{
		wxString file = reCmd.GetMatch(strData, 1);
		if (s_Loading)
            s_DdeFilesToOpen.Add(file);
        else
        {
            if(m_Frame)
                m_Frame->Open(file, true); // add to history, files that open through DDE
        }
	}
    return true;
}
#endif

// event handlers

void CodeBlocksApp::OnAppActivate(wxActivateEvent& event)
{
	if (!event.GetActive())
		return;
    if (!Manager::Get())
        return;

    if (Manager::Get()->GetEditorManager() && Manager::Get()->GetConfigManager(_T("app"))->ReadBool(_T("/environment/check_modified_files"), true))
    {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, idEditorManagerCheckFiles);
        wxPostEvent(Manager::Get()->GetEditorManager(), evt);
//        Manager::Get()->GetEditorManager()->CheckForExternallyModifiedFiles();
    }
    cbEditor* ed = Manager::Get()->GetEditorManager()
                    ? Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor()
                    : 0;
	if (ed)
	{
		// hack for linux: without it, the editor loses the caret every second activate o.O
		Manager::Get()->GetEditorManager()->GetNotebook()->SetFocus();

		ed->GetControl()->SetFocus();
	}
}