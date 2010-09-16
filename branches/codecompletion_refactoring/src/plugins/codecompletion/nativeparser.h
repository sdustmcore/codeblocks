/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 */

#ifndef NATIVEPARSER_H
#define NATIVEPARSER_H

#include "parser/parser.h"

#include <queue>
#include <map>

#include <wx/event.h>
#include <wx/hashmap.h> // TODO: replace with std::map

extern bool s_DebugSmartSense;
extern const wxString g_StartHereTitle;
extern const int g_EditorActivatedDelay;

// forward decls
class cbEditor;
class EditorBase;
class cbProject;
class ClassBrowser;
class Token;

WX_DECLARE_HASH_MAP(cbProject*, Parser*, wxPointerHash, wxPointerEqual, ParsersMap);
WX_DECLARE_HASH_MAP(cbProject*, wxString, wxPointerHash, wxPointerEqual, ParsersFilenameMap);

typedef std::map<cbProject*, wxArrayString> ProjectSearchDirsMap;

enum ParserTokenType
{
    pttSearchText = 0,
    pttClass,
    pttNamespace,
    pttFunction
};


enum OperatorType
{
    otOperatorUndefine = 0,
    otOperatorSquare,
    otOperatorParentheses,
    otOperatorPointer,
    otOperatorStar
};

enum BrowserViewMode
{
    bvmRaw = 0,
    bvmInheritance
};

struct ParserComponent
{
    wxString component;
    ParserTokenType token_type;
    OperatorType tokenOperatorType;
};

struct ccSearchData
{
    cbStyledTextCtrl* control;
    wxString file;
};

class NativeParser : public wxEvtHandler
{
    public:
        NativeParser();
        ~NativeParser();

        // Return value is *ALWAYS* valid, without checking
        Parser* GetParser() const { return m_pParser; }

        Parser* GetParserByProject(cbProject* project);
        Parser* GetParserByFilename(const wxString& filename);
        cbProject* GetProjectByParser(Parser* parser);
        cbProject* GetProjectByFilename(const wxString& filename);

        bool Done();

        wxImageList* GetImageList() { return m_pImageList; }
        int GetTokenKindImage(Token* token);
        void SetTokenKindImage(int kind, const wxBitmap& bitmap, const wxBitmap& mask = wxNullBitmap);
        void SetTokenKindImage(int kind, const wxBitmap& bitmap, const wxColour& maskColour);
        void SetTokenKindImage(int kind, const wxIcon& icon);

        wxArrayString GetAllPathsByFilename(const wxString& filename);
        void AddPaths(wxArrayString& dirs, const wxString& path, bool hasExt);

        bool CreateParser(cbProject* project);
        bool DeleteParser(cbProject* project);
        bool ReparseFile(cbProject* project, const wxString& filename);
        bool AddFileToParser(cbProject* project, const wxString& filename);
        bool RemoveFileFromParser(cbProject* project, const wxString& filename);

        void RereadParserOptions();
        void ReparseCurrentProject();
        void ReparseSelectedProject();

        size_t MarkItemsByAI(TokenIdxSet& result, bool reallyUseAI = true, bool isPrefix = false,
                             bool caseSensitive = false, int caretPos = -1);
        size_t MarkItemsByAI(ccSearchData* searchData, TokenIdxSet& result, bool reallyUseAI = true,
                             bool isPrefix = false, bool caseSensitive = false, int caretPos = -1);
        void RemoveInvalid(TokenIdxSet& result, const wxString& target);

        const wxString& GetCodeCompletionItems();
        void GetCallTipHighlight(const wxString& calltip, int* start, int* end);
        int CountCommas(const wxString& calltip, int start);
        const wxArrayString& GetCallTips(int chars_per_line);
        int GetCallTipCommas() const { return m_CallTipCommas; }

        int GetEditorStartWord() const { return m_EditorStartWord; }
        int GetEditorEndWord() const { return m_EditorEndWord; }

        wxArrayString& GetProjectSearchDirs(cbProject* project);

        // returns the editor's position where the current function starts
        // optionally, returns the function's namespace (ends in double-colon ::) and name
        int FindCurrentFunctionStart(ccSearchData* searchData, wxString* nameSpace = 0L, wxString* procName = 0L, int caretPos = -1);
        // fills the result argument with all the tokens matching the current function (hopefully, just one)
        size_t FindCurrentFunctionToken(ccSearchData* searchData, TokenIdxSet& result, int caretPos = -1);

        ClassBrowser* GetClassBrowser() const { return m_pClassBrowser; }
        void CreateClassBrowser();
        void RemoveClassBrowser(bool appShutDown = false);
        void UpdateClassBrowser();
        void SetCBViewMode(const BrowserViewMode& mode);

        void OnProjectLoadingHook(cbProject* project, TiXmlElement* elem, bool loading);

    protected:
        bool StartCompleteParsing(cbProject* project, Parser* parser);
        bool SwitchParser(cbProject* project, Parser* parser);
        void SetParser(Parser* parser);
        void ClearParsers();
        void RemoveObsoleteParsers();
        std::pair<cbProject*, Parser*> GetProjectParserByActivedEditor();

    private:
        friend class CodeCompletion;
        size_t AI(TokenIdxSet& result, ccSearchData* searchData, const wxString& lineText = wxEmptyString,
                  bool isPrefix = false, bool caseSensitive = false, TokenIdxSet* search_scope = 0,
                  int caretPos = -1);

        size_t FindAIMatches(std::queue<ParserComponent> components, TokenIdxSet& result, int parentTokenIdx = -1,
                             bool isPrefix = false, bool caseSensitive = false, bool use_inheritance = true,
                             short int kindMask = 0xFFFF, TokenIdxSet* search_scope = 0);
        size_t BreakUpComponents(const wxString& actual, std::queue<ParserComponent>& components);
        bool BelongsToParentOrItsAncestors(TokensTree* tree, Token* token, int parentIdx, bool use_inheritance = true);
        size_t GenerateResultSet(TokensTree* tree, const wxString& search, int parentIdx, TokenIdxSet& result,
                                 bool caseSens = true, bool isPrefix = false, short int kindMask = 0xFFFF);
        size_t GenerateResultSet(wxString search, const TokenIdxSet& ptrParentID, TokenIdxSet& result,
                                 bool caseSens = true, bool isPrefix = false, short int kindMask = 0xFFFF);
        bool LastAISearchWasGlobal() const { return m_LastAISearchWasGlobal; }
        const wxString& LastAIGlobalSearch() const { return m_LastAIGlobalSearch; }

        bool ParseUsingNamespace(ccSearchData* searchData, TokenIdxSet& search_scope, int caretPos = -1);
        bool ParseFunctionArguments(ccSearchData* searchData, int caretPos = -1);
        bool ParseLocalBlock(ccSearchData* searchData, int caretPos = -1); // parses from the start of function up to the cursor

        unsigned int FindCCTokenStart(const wxString& line);
        wxString GetNextCCToken(const wxString& line, unsigned int& startAt, OperatorType& tokenOperatroType);
        wxString GetCCToken(wxString& line, ParserTokenType& tokenType, OperatorType& tokenOperatroType);
        void BreakUpInLines(wxString& str, const wxString& original_str, int chars_per_line = -1);
        bool AddCompilerDirs(cbProject* project, Parser* parser);
        bool AddCompilerPredefinedMacros(cbProject* project, Parser* parser);
        bool AddProjectDefinedMacros(cbProject* project, Parser* parser);
        const wxArrayString& GetGCCCompilerDirs(const wxString &cpp_compiler);
        bool LoadCachedData(cbProject* project);
        bool SaveCachedData(const wxString& projectFilename);

        void OnThreadStart(wxCommandEvent& event);
        void OnThreadEnd(wxCommandEvent& event);
        void OnParserStart(wxCommandEvent& event);
        void OnParserEnd(wxCommandEvent& event);

        void OnEditorActivatedTimer(wxTimerEvent& event);
        void OnEditorActivated(EditorBase* editor);
        void OnEditorClosed(EditorBase* editor);

        bool SkipWhitespaceForward(cbEditor* editor, int& pos);
        bool SkipWhitespaceBackward(cbEditor* editor, int& pos);

        size_t ResolveActualType(wxString searchText, const TokenIdxSet& searchScope, TokenIdxSet& result);
        size_t ResolveExpression(std::queue<ParserComponent> components, const TokenIdxSet& searchScope,
                                 TokenIdxSet& result, bool caseSense = true, bool isPrefix = false);

        void CollectSS(const TokenIdxSet& searchScope, TokenIdxSet& actualTypeScope, TokensTree* tree);
        void AddTemplateAlias(const int& actualTypeResult, const TokenIdxSet& actualTypeScope, TokenIdxSet& initialScope, TokensTree* tree);
        void ResolveTemplateMap(const wxString& searchStr, const TokenIdxSet& actualTypeScope, TokenIdxSet& initialScope);
        void ResolveOpeartor(const OperatorType& tokenOperatorType, const TokenIdxSet& tokens, TokensTree* tree, const TokenIdxSet& searchScope, TokenIdxSet& result);
        typedef std::pair<cbProject*, Parser*> ParserPair;
        typedef std::list<ParserPair> ParserList;
        ParserList           m_ParserList;

        Parser               m_TempParser;
        Parser*              m_pParser;
        int                  m_EditorStartWord;
        int                  m_EditorEndWord;
        wxTimer              m_TimerEditorActivated;
        wxString             m_CCItems;
        wxArrayString        m_CallTips;
        int                  m_CallTipCommas;
        ClassBrowser*        m_pClassBrowser;
        bool                 m_GettingCalltips; // flag while getting calltips
        bool                 m_ClassBrowserIsFloating;

        bool                 m_LastAISearchWasGlobal; // true if the phrase for code-completion is empty or partial text (i.e. no . -> or :: operators)
        wxString             m_LastAIGlobalSearch; // same case like above, it holds the search string

        ProjectSearchDirsMap m_ProjectSearchDirsMap;
        int                  m_HookId; // project loader hook ID
        wxArrayString        m_StandaloneFiles;

        wxImageList*         m_pImageList;
        wxString             m_LastActivatedFile;

        map<wxString, wxString> m_TemplateMap;
        DECLARE_EVENT_TABLE()
};

#endif // NATIVEPARSER_H

