#include "compiler.h"
#include "manager.h"
#include "messagemanager.h"
#include "configmanager.h"
#include "globals.h"
#include <wx/intl.h>
#include <wx/filename.h>

wxString Compiler::CommandTypeDescriptions[COMPILER_COMMAND_TYPES_COUNT] =
{
    // These are the strings that describe each CommandType enumerator...
    // No need to say that it must have the same order as the enumerators!
    _("Compile single file to object file"),
    _("Generate dependencies for file"),
    _("Compile Win32 resource file"),
    _("Link object files to executable"),
    _("Link object files to dynamic library"),
    _("Link object files to static library")
};
long Compiler::CompilerIDCounter = 0;

Compiler::Compiler(const wxString& name)
    : m_Name(name),
    m_ID(++CompilerIDCounter),
    m_ParentID(-1)
{
	//ctor
    Manager::Get()->GetMessageManager()->DebugLog("Added compiler \"%s\"", m_Name.c_str());
}

Compiler::Compiler(const Compiler& other)
    : m_ParentID(other.m_ID)
{
    m_ID = ++CompilerIDCounter; // this copy ctor is created for this
    m_Name = "Copy of " + other.m_Name;
    m_MasterPath = other.m_MasterPath;
    m_Programs = other.m_Programs;
    m_Switches = other.m_Switches;
    m_Options = other.m_Options;
    for (int i = 0; i < COMPILER_COMMAND_TYPES_COUNT; ++i)
    {
        m_Commands[i] = "echo \"No compiler command set for this operation!\"";
    }
}

Compiler::~Compiler()
{
	//dtor
}

void Compiler::SaveSettings(const wxString& baseKey)
{
    wxString tmp;
    tmp.Printf("%s/%3.3d", baseKey.c_str(), (int)m_ID);
	ConfigManager::Get()->Write(tmp + "/_name", m_Name);
	ConfigManager::Get()->Write(tmp + "/_parent", m_ParentID);

	wxString key = GetStringFromArray(m_CompilerOptions);
	ConfigManager::Get()->Write(tmp + "/compiler_options", key);
	key = GetStringFromArray(m_LinkerOptions);
	ConfigManager::Get()->Write(tmp + "/linker_options", key);
	key = GetStringFromArray(m_IncludeDirs);
	ConfigManager::Get()->Write(tmp + "/include_dirs", key);
	key = GetStringFromArray(m_LibDirs);
	ConfigManager::Get()->Write(tmp + "/library_dirs", key);
	key = GetStringFromArray(m_CmdsBefore);
	ConfigManager::Get()->Write(tmp + "/commands_before", key);
	key = GetStringFromArray(m_CmdsAfter);
	ConfigManager::Get()->Write(tmp + "/commands_after", key);

    ConfigManager::Get()->Write(tmp + "/master_path", m_MasterPath);
    ConfigManager::Get()->Write(tmp + "/c_compiler", m_Programs.C);
    ConfigManager::Get()->Write(tmp + "/cpp_compiler", m_Programs.CPP);
    ConfigManager::Get()->Write(tmp + "/linker", m_Programs.LD);
    ConfigManager::Get()->Write(tmp + "/res_compiler", m_Programs.WINDRES);
    ConfigManager::Get()->Write(tmp + "/make", m_Programs.MAKE);

    for (int i = 0; i < COMPILER_COMMAND_TYPES_COUNT; ++i)
    {
        ConfigManager::Get()->Write(tmp + "/macros/" + CommandTypeDescriptions[i], m_Commands[i]);
    }

    // switches
    ConfigManager::Get()->Write(tmp + "/switches/includes", m_Switches.includeDirs);
    ConfigManager::Get()->Write(tmp + "/switches/libs", m_Switches.libDirs);
    ConfigManager::Get()->Write(tmp + "/switches/link", m_Switches.linkLibs);
    ConfigManager::Get()->Write(tmp + "/switches/define", m_Switches.defines);
    ConfigManager::Get()->Write(tmp + "/switches/generic", m_Switches.genericSwitch);
    ConfigManager::Get()->Write(tmp + "/switches/objectext", m_Switches.objectExtension);
    ConfigManager::Get()->Write(tmp + "/switches/deps", m_Switches.needDependencies);
}

void Compiler::LoadSettings(const wxString& baseKey)
{
    wxString tmp;
    tmp.Printf("%s/%3.3d", baseKey.c_str(), (int)m_ID);
    wxString sep = wxFileName::GetPathSeparator();

	m_Name = ConfigManager::Get()->Read(tmp + "/_name", m_Name);

    if (ConfigManager::Get()->HasGroup(tmp) || ConfigManager::Get()->HasEntry(tmp))
    {
        m_MasterPath = ConfigManager::Get()->Read(tmp + "/master_path", wxEmptyString);
        m_Programs.C = ConfigManager::Get()->Read(tmp + "/c_compiler", wxEmptyString);
        m_Programs.CPP = ConfigManager::Get()->Read(tmp + "/cpp_compiler", wxEmptyString);
        m_Programs.LD = ConfigManager::Get()->Read(tmp + "/linker", wxEmptyString);
        m_Programs.WINDRES = ConfigManager::Get()->Read(tmp + "/res_compiler", wxEmptyString);
        m_Programs.MAKE = ConfigManager::Get()->Read(tmp + "/make", wxEmptyString);
    }

	m_CompilerOptions = GetArrayFromString(ConfigManager::Get()->Read(tmp + "/compiler_options", wxEmptyString));
	m_LinkerOptions = GetArrayFromString(ConfigManager::Get()->Read(tmp + "/linker_options", wxEmptyString));
	m_IncludeDirs = GetArrayFromString(ConfigManager::Get()->Read(tmp + "/include_dirs", m_MasterPath + sep + "include"));
	m_LibDirs = GetArrayFromString(ConfigManager::Get()->Read(tmp + "/library_dirs", m_MasterPath + sep + "lib"));
	m_CmdsBefore = GetArrayFromString(ConfigManager::Get()->Read(tmp + "/commands_before", wxEmptyString));
	m_CmdsAfter = GetArrayFromString(ConfigManager::Get()->Read(tmp + "/commands_after", wxEmptyString));

    for (int i = 0; i < COMPILER_COMMAND_TYPES_COUNT; ++i)
    {
        m_Commands[i] = ConfigManager::Get()->Read(tmp + "/macros/" + CommandTypeDescriptions[i], m_Commands[i]);
    }

    // switches
    m_Switches.includeDirs = ConfigManager::Get()->Read(tmp + "/switches/includes", m_Switches.includeDirs);
    m_Switches.libDirs = ConfigManager::Get()->Read(tmp + "/switches/libs", m_Switches.libDirs);
    m_Switches.linkLibs = ConfigManager::Get()->Read(tmp + "/switches/link", m_Switches.linkLibs);
    m_Switches.defines = ConfigManager::Get()->Read(tmp + "/switches/define", m_Switches.defines);
    m_Switches.genericSwitch = ConfigManager::Get()->Read(tmp + "/switches/generic", m_Switches.genericSwitch);
    m_Switches.objectExtension = ConfigManager::Get()->Read(tmp + "/switches/objectext", m_Switches.objectExtension);
    m_Switches.needDependencies = ConfigManager::Get()->Read(tmp + "/switches/deps", m_Switches.needDependencies);
}