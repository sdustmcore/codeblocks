#include "tinywxuni.h"
#include "tinyxml.h"

#include <wx/string.h>
#include <wx/file.h>


bool TinyXML::LoadDocument(const wxString& filename, TiXmlDocument *doc)
{

    if(!doc || !wxFile::Access(filename, wxFile::read))
        return false;

    wxFile file(filename);
    size_t len = file.Length();

    char *input = new char[len+1];
    input[len] = '\0';
    file.Read(input, len);

    doc->Parse(input);
    delete[] input;
    return true;
}

TiXmlDocument* TinyXML::LoadDocument(const wxString& filename)
{
    TiXmlDocument* doc = new TiXmlDocument();

    if(TinyXML::LoadDocument(filename, doc))
    {
        return doc;
    }
    else
    {
        delete doc;
        return 0;
    }
}

bool TinyXML::SaveDocument(const wxString& filename, TiXmlDocument* doc)
{
    if (!doc)
        return false;

	TiXmlPrinter Printer;
	Printer.SetIndent("\t");
	doc->Accept(&Printer);

    wxTempFile file(filename);
    if(file.IsOpened())
        if(file.Write(Printer.CStr(), Printer.Size()) && file.Commit())
            return true;

    return false;
}
