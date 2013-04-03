#ifndef WXSDIALOGRES_H
#define WXSDIALOGRES_H

#include "../wxsresource.h"
#include "../defwidgets/wxsdialog.h"
#include <wx/string.h>

class wxsDialogRes : public wxsResource
{
	public:
	
        /** Ctor */
		wxsDialogRes(wxsProject* Project,const wxString& Class, const wxString& Xrc, const wxString& Src,const wxString& Head);
		
		/** Dctor */
		virtual ~wxsDialogRes();
		
		/** Getting name of class implementing this dialog */
		inline const wxString& GetClassName() { return ClassName; }
		
		/** Getting name of xrc file containing structire of dialog */
		inline const wxString& GetXrcFile() { return XrcFile; }
		
		/** Getting name of source file implementing dialog */
		inline const wxString& GetSourceFile() { return SrcFile; }
		
		/** Getting name of header file declaring dialog */
		inline const wxString& GetHeaderFile() { return HFile; }
		
		/** Getting main dialog widget */
		inline wxsDialog& GetDialog() { return *Dialog; }
		
		/** Saving current dialog to xml file */
		void Save();
		
        /** Ckecking if this resource ecan be Previewed */
        virtual bool CanPreview() { return true; }
        
        /** This function should show preview in modal */
        virtual void ShowPreview();
        
        /** Getting resource name */
        virtual const wxString& GetResourceName();
        
        /** Generating empty source and header file.
          *
          * WARNING: Not checking if files exist
          */
        bool GenerateEmptySources();
        
        /** This will be used to shedule code rebuilds */
        virtual void NotifyChange();
        
    protected:
    
        /** Creating editor object */
        virtual wxsEditor* CreateEditor();
		
	private:
	
        /** Creating xml tree for current widget */
        TiXmlDocument* GenerateXml();
        
        /** Adding declaration codes for locally stored widgets */
        void AddDeclarationsReq(wxsWidget* Widget,wxString& LocalCode,wxString& GlobalCode,int LocalTabSize,int GlobalTabSize,bool& WasLocal);
	
        wxString   ClassName;
        wxString   XrcFile;
        wxString   SrcFile;
        wxString   HFile;
        wxsDialog* Dialog;
};

#endif // WXSDIALOGRES_H