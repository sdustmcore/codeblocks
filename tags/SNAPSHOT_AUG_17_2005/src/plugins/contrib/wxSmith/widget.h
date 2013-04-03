#ifndef __WIDGET_H
#define __WIDGET_H


#include <wx/window.h>
#include <wx/image.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <tinyxml/tinyxml.h>
#include <settings.h>
#include <manager.h>
#include <messagemanager.h>

#include "wxsproperties.h"
#include "wxswindoweditor.h"
#include "wxsstyle.h"

#define DebLog Manager::Get()->GetMessageManager()->DebugLog

class wxsWidgetManager;
class wxsEventDesc;
class wxsWidget;

/** Class representing one event */
class wxsEventDesc
{
    public:
        wxsEventDesc() {};
        virtual ~wxsEventDesc() {};
        
        /** Function which should giveevent's name */
        virtual wxString EventName() = 0;
        
        /** Function which should produce content of ( ) in event handler 
         *  including braces.
         */
        virtual wxString GetFunctionParams() = 0;
        
        /** Function which should produce entry in event array for
         * specified function
         */
        virtual wxString GetETEntry(const wxString& FunctionName) = 0;
};

/** Structure containing info aboul widget */
struct wxsWidgetInfo
{
    wxString Name;                  ///< Widget's name
    wxString License;               ///< Widget's license
    wxString Author;                ///< Widget's author
    wxString AuthorEmail;           ///< Widget's authos's email
    wxString AuthorSite;            ///< Widget's author's site
    wxString WidgetsSite;           ///< Site about this widget
    wxString Category;              ///< Widget's category
    wxString DefaultVarName;        ///< Prefix for default variable name
    bool Container;                 ///< True if this widget can have other widgets inside
    bool Sizer;                     ///< True if this widget is a sizer (Container must also be true)
    unsigned short VerHi;           ///< Lower number of version
    unsigned short VerLo;           ///< Higher number of version
    wxBitmap* Icon;                 ///< Icon used in pallette
    wxsWidgetManager* Manager;      ///< Manager handling this widget
    int Id;                         ///< Identifier used inside manager to handle this widget, must be same as 'Number' in GetWidgetInfo call
    int TreeIconId;
    wxsStyle* Styles;               ///< Set of available styles, ending with NULL-named style
};

/** Structure describing default widget's options */
struct wxsWidgetBaseParams
{
    wxString IdName;                ///< Widget's identifier
    wxString VarName;               ///< Widget's variable used inside main window
    bool IsMember;                  ///< True if widget's variable won't be stored inside main window
    int PosX, PosY;                 ///< Widget's position
    bool DefaultPosition;           ///< Widget has default position
    int SizeX, SizeY;               ///< Widget's size
    bool DefaultSize;               ///< Widget has default size
    
    
    /* Used by sizers */
    
    int Proportion;                 ///< Proportion param (see wxW documentation for details)
    
    enum BorderFlagsValues          ///< Values which can be used in BorderFlags (ored values)
    {
        None   = 0,
        Top    = 1,
        Bottom = 2,
        Left   = 4,
        Right  = 8
    };
    
    int BorderFlags;                ///< Which sides should use additional bordeer ?
    
    enum PlacementType              ///< Type of border
    {
        LeftTop = 0,
        CenterTop,
        RightTop,
        LeftCenter,
        Center,
        RightCenter,
        LeftBottom,
        CenterBottom,
        RightBottom
    };
    
    bool Expand;
    bool Shaped;
    bool FixedMinSize;
    
    int Placement;                  ///< Placement of this element
    int Border;                     ///< Size of additional border (in pixels)
    int Style;                      ///< Current style
    
    wxsWidgetBaseParams():
        IdName(_T("")),
        VarName(_T("")),
        IsMember(true),
        PosX(-1), PosY(-1),
        DefaultPosition(true),
        SizeX(-1), SizeY(-1),
        DefaultSize(true),
        Proportion(0),
        BorderFlags(Top|Bottom|Left|Right),
        Expand(false),
        Shaped(false),
        FixedMinSize(false),
        Placement(Center),
        Border(5),
        Style(0)
        {}
};


/** Structure containing all data needed while generating code */
struct wxsCodeParams
{
    wxString ParentName;
    bool IsDirectParent;
    int UniqueNumber;
};

/** Class representing one widget */
class wxsWidget
{
    public:
    
        /** Type representing base properties set while creating widget */
        typedef unsigned long BasePropertiesType;
        
        static const BasePropertiesType bptPosition  = 0x0001;  ///< this widget is using position
        static const BasePropertiesType bptSize      = 0x0002;  ///< this widget is using size
        static const BasePropertiesType bptId        = 0x0004;  ///< this widget is using identifier
        static const BasePropertiesType bptVariable  = 0x0008;  ///< this widget is using variable
        static const BasePropertiesType bptStyle     = 0x0010;  ///< this widget is using style
        
        /** BasePropertiesType with no default properties */
        static const BasePropertiesType propNone     = 0;        
        
        /** VasePropertiesTyue usede by common windows */
        static const BasePropertiesType propWindow   = bptStyle;
        
        /** BasePropertiesType used by common widgets */
        static const BasePropertiesType propWidget   = bptPosition | bptSize | bptId | bptVariable | bptStyle;
        
        /** BasePropertiesType used by common sizers */
        static const BasePropertiesType propSizer    = bptVariable;
    
        /** Default constructor */
        wxsWidget(wxsWidgetManager* Man,wxsWindowRes* Res,BasePropertiesType pType = propNone):
            PropertiesObject(this),
            Manager(Man),
            Preview(NULL),
            Resource(Res),
            Properties(NULL),
            Parent(NULL),
            MaxChildren(0),
            XmlElement(NULL),
            ContainerType(NoContainer),
            Updating(false),
            PropertiesCreated(false),
            BPType(pType)
        {
        }
        
        /** Constructor used by containers 
         *
         *  \param Man - widgets manager 
         *  \param ISwxWindow - true if this container is an wxWindow object,
         *                      false otherwise (usually for wxSizer objects
         *  \param MaxChildren - maximal number of children which can be handled by this container
         */
        wxsWidget(wxsWidgetManager* Man, wxsWindowRes* Res, bool ISwxWindow, int MaxChild,BasePropertiesType pType = propNone):
            PropertiesObject(this),
            Manager(Man),
            Preview(NULL),
            Resource(Res),
            Properties(NULL),
            Parent(NULL),
            MaxChildren(MaxChild),
            XmlElement(NULL),
            ContainerType(ISwxWindow ? ContainerWindow : ContainerSizer ),
            Updating(false),
            PropertiesCreated(false),
            BPType(pType)
        {
        }
            
        
        /** Destructor */
        virtual ~wxsWidget(); 
        
        /** Getting widget's info */
        virtual const wxsWidgetInfo& GetInfo() = 0;
        
        /** Getting manager of this widget */
        inline wxsWidgetManager* GetManager() { return Manager; }
        
        /** Getting parent widget of this one */
        inline wxsWidget* GetParent() { return Parent; }
        
        /** Getting resource tree of this widget */
        inline wxTreeItemId GetTreeId() { return TreeId; }
        
        /** Getting BasePropertiesType for this widget */
        inline BasePropertiesType GetBPType() { return BPType; }
        
        /** Getting resource owning this widget */
        inline wxsWindowRes* GetResource() { return Resource; }
        
        /** Getting editor for this widget (or NULL if there's no editor) */
        wxsWindowEditor* GetEditor();

/******************************************************************************/
/* Preview                                                                    */
/******************************************************************************/
    
    public:

        /** This should create new preview object */
        wxWindow* CreatePreview(wxWindow* Parent,wxsWindowEditor* Editor);
        
        /** This should kill preview object */
        void KillPreview();
        
        /** Function returning current pereview window */
        inline wxWindow* GetPreview() { return Preview; }
            
    protected:
    
        /** This function should create preview window for widget.
         *  It is granted that this funcntion will be called when there's no
         *  preview yet.
         */
        virtual wxWindow* MyCreatePreview(wxWindow* Parent) = 0;
        
        /** This function should delete preview object. Usually should be
         *  left as default. Put here to aviod conflicts between different
         *  memory heaps.
         */
        virtual void MyDeletePreview(wxWindow* Preview) { delete Preview; }
        
        /** This fuunction can be used to update all properties for preview
         *  after creating it's children.
         */
        virtual void MyFinalUpdatePreview(wxWindow* Preview) {}
        
/******************************************************************************/
/* Properties                                                                 */
/******************************************************************************/

    public:
    
        /** Function returning base configuration params for this widget */
        inline wxsWidgetBaseParams& GetBaseParams() { return BaseParams; }

        /** Getting properties window for this widget */
        inline wxWindow* CreatePropertiesWindow(wxWindow* Parent)
        {
            if ( !PropertiesCreated )
            {
                CreateObjectProperties();
                PropertiesCreated = true;
            }
            if ( !Properties ) Properties = MyCreatePropertiesWindow(Parent);
            return Properties;
        }
        
        /** getting properties which are currently used */
        inline wxWindow* GetProperties() { return Properties; }
        
        /** This should kill properties window */
        inline void KillProperties()
        {
            if ( Properties != NULL )
            {
                DeleteProperties(Properties);
                Properties = NULL;
            }
        }
        
        /** Function which should update content of current properties window */
        virtual void UpdateProperties()
        {
            if ( Updating ) return;
            Updating = true;
            MyUpdateProperties();
            Updating = false;
        }
        
        /** Function notifying that properties were changed inside properties editor
         *  \param Validate - if true, changed properties should be validated
         *  \param Correct  - if true, invalid properties should be automatically corrected
         *  \return true - properties valid, false - properties invalid (before correction)
         *          always returns true if Validate == false
         */
        bool PropertiesUpdated(bool Validate,bool Correct);

    protected:
    
        /** This function should create properties view for widget. It is granted
         *  that there are no properties created yet.
         */
        virtual wxWindow* MyCreatePropertiesWindow(wxWindow* Parent)
        { 
            return GenBaseParamsConfig(Parent);
        }
        
        /** This function should delete properties window. Usually should be
         *  left as default. Put here to aviod conflicts between different
         *  memory heaps.
         */
        virtual void DeleteProperties(wxWindow* Properties) { delete Properties; }
        
        /** Function which should update content of current properties window */
        virtual void MyUpdateProperties()
        {
            if ( GetProperties() ) PropertiesObject.UpdateProperties();
        }
        
        /** Function initializing properties for this widget.
         *  This should add all properties.
         *  Call to this function is made when properties window is created for the
         *  first time
         */
        virtual void CreateObjectProperties()
        {
            AddDefaultProperties(BPType);
        }

        /** Getting wingow's style */
        inline int GetStyle() { return BaseParams.Style; }
        
        /** Getting window's position */
        inline wxPoint GetPosition()
        {
            return BaseParams.DefaultPosition ?
                wxDefaultPosition :
                wxPoint(BaseParams.PosX,BaseParams.PosY);
        }
        
        /** Getting window's size */
        inline wxSize GetSize()
        {
            return BaseParams.DefaultSize ?
                wxDefaultSize :
                wxSize(BaseParams.SizeX,BaseParams.SizeY);
        }

/******************************************************************************/
/* Code generation                                                            */
/******************************************************************************/

    public:
    
        /** Function generating code which should produce widget
         *  It's called BEFORE widget's children are created and
         *  must set up BaseParams.VarName variable inside code.
         */
        virtual wxString GetProducingCode(wxsCodeParams& Params) { return _T(""); }
            
        /** Function generating code which finishes production process of this
         *  widget, it will be called AFTER child widgets are created
         *
         * It can be used f.ex. by sizers to bind to parent item.
         * UniqueNumber is same as in GetProducingCode
         */
        virtual wxString GetFinalizingCode(wxsCodeParams& Params) { return _T(""); }
        
        /** Function generating code which generates variable containing this 
         *  widget. If there's no variable (f.ex. space inside sizers), it should
         *  return empty string
         */
        virtual wxString GetDeclarationCode(wxsCodeParams& Params) { return _T(""); }

        /** Structure deeclaring some code-defines which could be usefull while
         *  creating widget's code
         */
        struct CodeDefines
        {
            wxString FColour;   ///< Foreground colour, in form 'wxColour(...)'
            wxString BColour;   ///< Background colour, in form 'wxColour(...)'
            wxString Font;      ///< Font, in form 'wxFont(...)'
            wxString Style;     ///< Widget's style in form 'wxSTYLE1|wxSTYLE2'
            wxString Pos;       ///< Widget's position
            wxString Size;      ///< Widget's size
        };
        
        /** Function creating current coded defines */
        virtual const CodeDefines& GetCodeDefines();
        
        /** Util function - changing given string to it's representation in C++.
         *
         *  Could be used when need to throw some string to generated code
         */
        static const wxString& GetCString(const wxString& Source);
        
/**********************************************************************/
/* Used for extended widgets                                          */
/**********************************************************************/
        
        /** Types of extended widgets */
        enum ExType
        {
            exNone = 0,     ///< This is widget from standard set
            exCode,         ///< This widget provides it's source code
            exLibrary       ///< This widget is provided in additional library, currently not supported
        };
        
        /** Checking type of extendet widget */
        virtual ExType GetExtended() { return exNone; }
        
        /** Takign declaration of class using widget's implementation,
         *  this is valid for wxCode widgets only 
         */
        virtual const wxString& GetWidgetCodeDefinition() { static wxString Str(wxT("")); return Str; }
        
        /** Taking defition of widget's members
         *  this is valid for wxCode widgets only 
         */
        virtual const wxString& GetWidgetCodeDeclaration() { static wxString Str(wxT("")); return Str; }

        /** Taking name of library in which this widget is defined
         *  this is valid for wxLibrary widgets only, currently not supported
         */
        virtual const wxString& GetWidgetLibrary() { static wxString Str(wxT("")); return Str; }
        
        /** Taking name of header file which should be included in order to use this widget
         *  valid for exLibrary or exNone widgets only, currently exNone widgets are only supported
         */
        virtual const wxString& GetWidgetHeader() { static wxString Str(wxT("")); return Str; }
        
/**********************************************************************/
/* Support for containers                                             */
/**********************************************************************/
        
        /** Checking if this widget is an container */
        inline bool IsContainer() { return ContainerType != NoContainer; }
        
        /** Getting max number of children */
        inline int GetMaxChildren() { return IsContainer() ? MaxChildren : 0; }
        
        /** Checking if this container is a window (children are connected
         *  directly to it)
         */
        inline bool IsContWindow() { return ContainerType == ContainerWindow; }
        
        /** Getting number of internal widgets */
        virtual int GetChildCount() { return 0; }
        
        /** Getting pointer to given child */
        virtual wxsWidget* GetChild(int Id) { return NULL; }
        
        /** Binding child
         *
         * \param InsertAfterThis - position where to add new widget, if -1,
         *        adding at the end
         */
        virtual int AddChild(wxsWidget* NewWidget,int InsertBeforeThis = -1) { return -1; }
        
        /** Unbinding child
         *
         * This structure must inbing child window from this one, must NOT delete
         * child
         */
        virtual bool DelChildId(int Id) { return false; }

        /** Unbinding child
         *
         * This structure must inbing child window from this one, must NOT delete
         * child
         */
        virtual bool DelChild(wxsWidget* Widget) { return false; }
        
        /** Searching for specified widget 
         * \returns >= 0 if found (this value is Id of widget, 0<=Id<ChildCount),
         *          -1 if there's no such child
         */
        virtual int FindChild(wxsWidget* Widget) { return -1; }
        
        /** Changing position of widget in child list */
        virtual bool ChangeChildPos(int PrevPos, int NewPos) { return false; }
        
        /** Generating code which will bind children to this window */
        virtual const wxString& GetBindingCode(const wxString* Children, int Count) { static wxString Str(wxT("")); return Str; }
        
        
/**********************************************************************/
/* Support for base widget's parameters                               */
/**********************************************************************/
        
   protected:
   
        /** Function creating wxPanel object which contains panel with
         * configuration of base widget's properties
         */
        inline wxWindow* GenBaseParamsConfig(wxWindow* Parent)
        {
            return PropertiesObject.GenerateWindow(Parent);
        }
        
        /** This function updates content of given base properties panel,
         *  should be called when any operation on preview changes any of 
         *  base properties.
         */
        inline void UpdateBaseParamsConfig()
        {
            PropertiesObject.UpdateProperties();
        }
        
        /** Base parameters describing this widget */
        wxsWidgetBaseParams BaseParams;
        
        /** Base object used to handle properties */
        wxsProperties PropertiesObject;
        
    
/******************************************************************************/
/* XML Operations                                                             */
/******************************************************************************/
     
    public:
        /** Loading widget from xml source */
        inline bool XmlLoad(TiXmlElement* Element)
        {
            XmlAssignElement(Element);
            bool Result = MyXmlLoad();
            if ( !XmlLoadDefaults() ) Result = false;
            XmlAssignElement(NULL);
            return Result;
        }
        
        inline bool XmlSave(TiXmlElement* Element)
        {
            XmlAssignElement(Element);
            bool Result = MyXmlSave();
            if ( !XmlSaveDefaults() ) Result = false;
            XmlAssignElement(NULL);
            return Result;
        }
        
    protected:

        /** Internal function loading data from xml tree
         *
         * This functino can be overriden inside derivedd classes
         * to allow loading additional data. This data should be loadedd
         * from node returned by XmlElem function (all XmlGet... functions
         * are also using this one). See bptxxx and propxxx to find out which
         * settings are loaded automatically.
         */
        virtual bool MyXmlLoad() { return true; }
        
        /** Internal function saving xml tree.
         *
         * This function can be overriden inside derived classes
         * to allow saving additional data. This data should be saved
         * from node returned by XmlElem function (all XmlSet... functions
         * are also using this one). See bptxxx and propxxx to find out which
         * settings are saved automatically.
         */
        virtual bool MyXmlSave() { return true; }

        /** Getting currenlty associated xml element */
        inline TiXmlElement* XmlElem() { return XmlElement; }
        
/**********************************************************************/
/* Helpful functions which could be used while operating on resources */
/**********************************************************************/
        
        /** Getting value from given name */
        virtual wxString XmlGetVariable(const wxString& Name);
        
        /** Getting integer from given name
         *
         *  \param Name - name of xml tag
         *  \param IsInvalid - redeference to boolean variable which will get
         *                     information about read success
         *  \param DefaultValue - value which will be returned in case of any error
         *                        (IsInvalid=true)
         *  \returns value of variable
         */
        virtual int XmlGetInteger(const wxString &Name,bool& IsInvalid,int DefaultValue=0);
        
        /** Getting integer from given name without returning error */
        inline int XmlGetInteger(const wxString &Name,int DefaultValue=0)
        {
            bool Temp;
            return XmlGetInteger(Name,Temp,DefaultValue);
        }
        
        /** Getting size/position from given name */
        virtual bool XmlGetIntPair(const wxString& Name,int& P1,int& P2,int DefP1=-1,int DefP2=-1);
        
        // Added by cyberkoa
        /** Getting a series of string with given parent element and child element name */
        virtual bool wxsWidget::XmlGetStringArray(const wxString &ParentName,const wxString& ChildName, wxArrayString& stringArray);
        //End Add
        
        /** Setting string value */
        virtual bool XmlSetVariable(const wxString &Name,const wxString& Value);
        
        /** Setting integer value */
        virtual bool XmlSetInteger(const wxString &Name,int Value);
        
        /** Setting 2 integers */
        virtual bool XmlSetIntPair(const wxString &Name,int Val1,int Val2);
        
        /** Function assigning element which will be used while processing
         *  xml resources. Usually this function is calleed automatically
         *  from outside so there's no need to care about currently
         *  associateed Xml element. This functino should be called when
         *  there's need to parse external xml elements using functions
         *  inside wxsWidget class.
         */
        void XmlAssignElement(TiXmlElement* Element);
        
        // Added by cyberkoa
        /** Set a series of string with the same given element name */
        virtual bool wxsWidget::XmlSetStringArray(const wxString &ParentName,const wxString& ChildName, wxArrayString& stringArray);
        //End Add
        
        /** Reading all default values for widget */
        inline bool XmlLoadDefaults() { return XmlLoadDefaultsT(BPType); }
        
        /** Reading default values for widget using given set of base properties */
        virtual bool XmlLoadDefaultsT(BasePropertiesType pType);
        
        /** Saving all default values for widget */
        inline bool XmlSaveDefaults() { return XmlSaveDefaultsT(BPType); }
        
        /** Saving default values for widget using given set of base properties */
        virtual bool XmlSaveDefaultsT(BasePropertiesType pType);
        
        /** Loading all children
         *
         *  Valid for compound objects only
         */
        virtual bool XmlLoadChildren();
        
        /** Saving all children
         *
         * Valid for compouund objects only, if current widget is a sizer,
         * additional "sizeritem" object will be created
         */
        virtual bool XmlSaveChildren();
        
    private:
    
        /** Loading sizer elements from given node */
        virtual void XmlLoadSizerStuff(TiXmlElement* Elem);
        
        /** Saving sizer element to given node */
        virtual void XmlSaveSizerStuff(TiXmlElement* Elem);
         
        /** Adding default properties to properties manager */
        virtual void AddDefaultProperties(BasePropertiesType Props);
        
        /** Function building tree for this widget */
        void BuildTree(wxTreeCtrl* Tree,wxTreeItemId WhereToAdd,int InsertIndex=-1);
        
        wxsWidgetManager* Manager;  ///< Widget's manager
        wxWindow* Preview;          ///< Currently opened preview window (NULL if there's no one)
        wxsWindowRes* Resource;     ///< Resource owning this widget
        wxWindow* Properties;       ///< Currently opened properties window (NULL if there's no one)
        wxsWidget* Parent;          ///< Parent widget of this one
        int MaxChildren;            ///< Num of max. Childs, -1 if no limit, valid for containers only
        
        TiXmlElement* XmlElement;   ///< Currently selected xml element

        enum CType { NoContainer, ContainerSizer, ContainerWindow };

        CType ContainerType;        ///< Type of container (or mark that it's no container)
        bool Updating;              ///< Set to true while any update is made (to avoid infinite loops and data loos)
        
        bool PropertiesCreated;
        
        BasePropertiesType BPType;  ///< Set of base properties used inside constructor
        
        CodeDefines CDefines;       ///< Will be filled and returned inside GetCodedeDefines
        
        wxTreeItemId TreeId;        ///< Id of item in resource tree
        
        friend class wxBaseParamsPanel;
        friend class wxsWindowEditor;
        friend class wxsContainer;
        friend class wxsProject;
        friend class wxsPalette;
        friend class wxsWidgetFactory;

};


/** Class managing widget */
class wxsWidgetManager
{
    public:
        wxsWidgetManager() {}
        virtual ~wxsWidgetManager() {}
        
        /** Funcntion initializing manager */
        virtual bool Initialize() { return true; }
        
        /** Returns number of handled widgets */
        virtual int GetCount() = 0;
        
        /** Getting widget's info */
        virtual const wxsWidgetInfo* GetWidgetInfo(int Number) = 0;
        
        /** Getting new widget */
        virtual wxsWidget* ProduceWidget(int Id,wxsWindowRes* Res) = 0;
        
        /** Killing widget */
        virtual void KillWidget(wxsWidget* Widget) = 0;
        
        /** Fuunction registering this manager into main widget's factory */
        virtual bool RegisterInFactory();
};

#endif