#ifndef SIMPLETEXTLOG_H
#define SIMPLETEXTLOG_H

#include "settings.h"
#include "messagelog.h" // inheriting class's header file
#include <wx/textctrl.h>

/*
 * No description
 */
class DLLIMPORT SimpleTextLog : public MessageLog
{
	public:
		// class constructor
		SimpleTextLog(bool fixedPitchFont = false);
		// class destructor
		~SimpleTextLog();

		virtual void AddLog(const wxString& msg, bool addNewLine = true);
		virtual void ResetLogFont();
		wxTextCtrl* GetTextControl(){ return m_Text; }
    protected:
        wxTextCtrl* m_Text;
        bool m_UseFixedPitchFont;
    private:
};

#endif // SIMPLETEXTLOG_H

