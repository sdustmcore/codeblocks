#ifndef NATIVEPARSER_H
#define NATIVEPARSER_H

#include <queue>

#include <wx/event.h>
#include <wx/hashmap.h>
#include "parser/parser.h"

#define DEBUG_CC_AI
extern bool s_DebugSmartSense;

// forward decls
class cbEditor;
class EditorBase;
class cbProject;
class ClassBrowser;
//class Parser;
class Token;

WX_DECLARE_HASH_MAP(cbProject*, Parser*, wxPointerHash, wxPointerEqual, ParsersMap);
WX_DECLARE_HASH_MAP(cbProject*, wxString, wxPointerHash, wxPointerEqual, ParsersFilenameMap);

enum ParserTokenType
{
	pttSearchText = 0,
	pttClass,
	pttNamespace,
	pttFunction
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
};

class NativeParser : public wxEvtHandler
{
	public:
		NativeParser();
		~NativeParser();

		void AddParser(cbProject* project, bool useCache = true);
		void RemoveParser(cbProject* project, bool useCache = true);
		void ClearParsers();
		void RereadParserOptions();
		void AddFileToParser(cbProject* project, const wxString& filename);
		void RemoveFileFromParser(cbProject* project, const wxString& filename);
		void ForceReparseActiveProject();

		size_t MarkItemsByAI(TokenIdxSet& result, bool reallyUseAI = true);

		const wxString& GetCodeCompletionItems();
		const wxArrayString& GetCallTips(int chars_per_line);
		int GetCallTipCommas(){ return m_CallTipCommas; }
        int CountCommas(const wxString& calltip, int start);
		void GetCallTipHighlight(const wxString& calltip, int* start, int* end);

		int GetEditorStartWord() const { return m_EditorStartWord; }
		int GetEditorEndWord() const { return m_EditorEndWord; }

		Parser* FindParserFromActiveEditor();
		Parser* FindParserFromEditor(cbEditor* editor);
		Parser* FindParserFromActiveProject();
		Parser* FindParserFromProject(cbProject* project);

        // returns the editor's position where the current function starts
        // optionally, returns the function's namespace (ends in double-colon ::) and name
		int FindCurrentFunctionStart(cbEditor* editor, wxString* nameSpace = 0L, wxString* procName = 0L);
		// fills the result argument with all the tokens matching the current function (hopefully, just one)
		size_t FindCurrentFunctionToken(cbEditor* editor, TokenIdxSet& result);

        ClassBrowser* GetClassBrowser() const { return m_pClassBrowser; }
		void CreateClassBrowser();
		void RemoveClassBrowser(bool appShutDown = false);
		void UpdateClassBrowser();
		void SetClassBrowserProject(cbProject* project);
		void SetCBViewMode(const BrowserViewMode& mode);
	protected:
	private:
        friend class CodeCompletion;
		size_t AI(TokenIdxSet& result, cbEditor* editor, Parser* parser, const wxString& lineText = wxEmptyString, bool noPartialMatch = false, bool caseSensitive = false, TokenIdxSet* search_scope = 0);

		size_t FindAIMatches(Parser* parser, std::queue<ParserComponent> components, TokenIdxSet& result, int parentTokenIdx = -1, bool noPartialMatch = false, bool caseSensitive = false, bool use_inheritance = true, short int kindMask = 0xFFFF, TokenIdxSet* search_scope = 0);
        size_t BreakUpComponents(Parser* parser, const wxString& actual, std::queue<ParserComponent>& components);
        bool BelongsToParentOrItsAncestors(TokensTree* tree, Token* token, int parentIdx, bool use_inheritance = true);
        size_t GenerateResultSet(TokensTree* tree, const wxString& search, int parentIdx, TokenIdxSet& result, bool caseSens = true, bool isPrefix = false, short int kindMask = 0xFFFF);

        bool LastAISearchWasGlobal() const { return m_LastAISearchWasGlobal; }
        const wxString& LastAIGlobalSearch() const { return m_LastAIGlobalSearch; }

		bool ParseUsingNamespace(cbEditor* ed, TokenIdxSet& search_scope);
		bool ParseFunctionArguments(cbEditor* ed);
		bool ParseLocalBlock(cbEditor* ed); // parses from the start of function up to the cursor

		unsigned int FindCCTokenStart(const wxString& line);
		wxString GetNextCCToken(const wxString& line, unsigned int& startAt, bool& is_function);
		wxString GetCCToken(wxString& line, ParserTokenType& tokenType);
		void BreakUpInLines(wxString& str, const wxString& original_str, int chars_per_line = -1);
		void AddCompilerDirs(Parser* parser, cbProject* project);
		bool LoadCachedData(Parser* parser, cbProject* project);
		bool SaveCachedData(Parser* parser, const wxString& projectFilename);
		void DisplayStatus(Parser* parser);
		void OnThreadStart(wxCommandEvent& event);
		void OnThreadEnd(wxCommandEvent& event);
		void OnParserEnd(wxCommandEvent& event);
		void OnEditorActivated(EditorBase* editor);

		bool SkipWhitespaceForward(cbEditor* editor, int& pos);
		bool SkipWhitespaceBackward(cbEditor* editor, int& pos);

        Parser m_Parser;
		int m_EditorStartWord;
		int m_EditorEndWord;
		wxString m_CCItems;
		wxArrayString m_CallTips;
		int m_CallTipCommas;
    	ClassBrowser* m_pClassBrowser;
    	bool m_GettingCalltips; // flag while getting calltips
    	bool m_ClassBrowserIsFloating;

    	bool m_LastAISearchWasGlobal; // true if the phrase for code-completion is empty or partial text (i.e. no . -> or :: operators)
    	wxString m_LastAIGlobalSearch; // same case like above, it holds the search string

        DECLARE_EVENT_TABLE()
};

#endif // NATIVEPARSER_H
