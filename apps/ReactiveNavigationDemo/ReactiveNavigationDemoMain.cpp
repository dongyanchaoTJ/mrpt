/* +---------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)               |
   |                          http://www.mrpt.org/                             |
   |                                                                           |
   | Copyright (c) 2005-2015, Individual contributors, see AUTHORS file        |
   | See: http://www.mrpt.org/Authors - All rights reserved.                   |
   | Released under BSD License. See details in http://www.mrpt.org/License    |
   +---------------------------------------------------------------------------+ */

#include "ReactiveNavigationDemoMain.h"
#include "CIniEditor.h"
#include <wx/msgdlg.h>
#include <wx/filename.h>

// In milliseconds:
#define SIMULATION_TIME_STEPS   100

//(*InternalHeaders(ReactiveNavigationDemoFrame)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/icon.h>
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

#include <mrpt/gui/WxUtils.h>

// The default configuration strings:
std::string EDIT_internalCfgReactive;
std::string EDIT_internalCfgRobot;

CIniEditor    *iniEditoreactivenav=NULL;

#include "imgs/main_icon.xpm"
#include "../wx-common/mrpt_logo.xpm"

#include "DEFAULT_GRIDMAP_DATA.h"

// A custom Art provider for customizing the icons:
class MyArtProvider : public wxArtProvider
{
protected:
    virtual wxBitmap CreateBitmap(const wxArtID& id,
                                  const wxArtClient& client,
                                  const wxSize& size);
};

// CreateBitmap function
wxBitmap MyArtProvider::CreateBitmap(const wxArtID& id,
                                     const wxArtClient& client,
                                     const wxSize& size)
{
    if (id == wxART_MAKE_ART_ID(MAIN_ICON))   return wxBitmap(main_icon_xpm);
    if (id == wxART_MAKE_ART_ID(IMG_MRPT_LOGO))  return wxBitmap(mrpt_logo_xpm);

    // Any wxWidgets icons not implemented here
    // will be provided by the default art provider.
    return wxNullBitmap;
}

// General global variables:
#include <mrpt/maps/COccupancyGridMap2D.h>
#include <mrpt/utils/CRobotSimulator.h>
#include <mrpt/utils/CFileGZInputStream.h>
#include <mrpt/utils/CConfigFile.h>
#include <mrpt/utils/CMemoryStream.h>
#include <mrpt/system/filesystem.h>

using namespace mrpt;
using namespace mrpt::obs;
using namespace mrpt::maps;
using namespace mrpt::opengl;
using namespace mrpt::math;
using namespace mrpt::poses;
using namespace mrpt::utils;
using namespace std;

//wxImage * auxMRPTImage2wxImage( const CImage &img );

#include <mrpt/nav/reactive/CReactiveNavigationSystem.h>
using namespace mrpt::nav;

CReactiveNavigationSystem		*reacNavObj=NULL;

// The obstacles map:
COccupancyGridMap2D		gridMap;
CRobotSimulator			robotSim(1e-9f,0);
TPoint2D				curCursorPos;

class CMyReactInterface : public CReactiveInterfaceImplementation
{
public:
	bool getCurrentPoseAndSpeeds( mrpt::poses::CPose2D &curPose, float &curV, float &curW)
	{
		robotSim.getRealPose( curPose );
		curV = robotSim.getV();
		curW = robotSim.getW();
		return true;
	}

	bool changeSpeeds( float v, float w )
	{
		robotSim.movementCommand(v,w);
		return true;
	}

	bool senseObstacles( mrpt::maps::CSimplePointsMap 		&obstacles )
	{
		CPose2D  robotPose;

		robotSim.getRealPose(robotPose);

		CObservation2DRangeScan    laserScan;
		laserScan.aperture = M_2PIf;
		laserScan.rightToLeft = true;
		laserScan.maxRange  = 7.0f;
		laserScan.stdError  = 0.003f;


		gridMap.laserScanSimulator(
			laserScan,
			robotPose,
			0.5f, // grid cell threshold
			361,  // Number of rays
			0.001     // Noise std
			);

		// Build the points map:
		obstacles.insertionOptions.minDistBetweenLaserPoints = 0.005f;
		obstacles.insertionOptions.also_interpolate = false;

		obstacles.clear();
		obstacles.insertObservation( &laserScan );

		// Draw points:
		vector<float> xs,ys,zs;
		obstacles.getAllPoints(xs,ys,zs);

		the_frame->lyLaserPoints->setPoints(xs,ys);
		the_frame->lyLaserPoints->SetCoordinateBase(
			robotSim.getX(),
			robotSim.getY(),
			robotSim.getPHI() );

		return true;
	}

	void notifyHeadingDirection(const double heading_dir_angle)
	{

	}

	ReactiveNavigationDemoFrame *the_frame;
};

CMyReactInterface  myReactiveInterface;

//(*IdInit(ReactiveNavigationDemoFrame)
const long ReactiveNavigationDemoFrame::ID_BUTTON1 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_BUTTON2 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_CHECKBOX3 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_BUTTON3 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_CHECKBOX1 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_STATICTEXT1 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_TEXTCTRL2 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_BUTTON7 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_CHECKBOX2 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_STATICTEXT6 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_TEXTCTRL6 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_STATICTEXT2 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_TEXTCTRL3 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_STATICTEXT4 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_STATICTEXT3 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_TEXTCTRL4 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_BUTTON4 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_PANEL1 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_CUSTOM1 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_TEXTCTRL1 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_STATUSBAR1 = wxNewId();
const long ReactiveNavigationDemoFrame::ID_TIMER1 = wxNewId();
//*)

const long ReactiveNavigationDemoFrame::ID_MENUITEM_SET_reactivenav_TARGET = wxNewId();


BEGIN_EVENT_TABLE(ReactiveNavigationDemoFrame,wxFrame)
    //(*EventTable(ReactiveNavigationDemoFrame)
    //*)
END_EVENT_TABLE()


void emul_printf(const char* s)
{
    cout << s;
}

ReactiveNavigationDemoFrame::ReactiveNavigationDemoFrame(wxWindow* parent,wxWindowID id)
{
    // Load my custom icons:
#if wxCHECK_VERSION(2, 8, 0)
    wxArtProvider::Push(new MyArtProvider);
#else
    wxArtProvider::PushProvider(new MyArtProvider);
#endif


    //(*Initialize(ReactiveNavigationDemoFrame)
    wxStaticBoxSizer* StaticBoxSizer2;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer2;
    wxStaticText* StaticText1;
    wxFlexGridSizer* FlexGridSizer7;
    wxStaticBoxSizer* StaticBoxSizer3;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer6;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;

    Create(parent, id, _("Reactive Navigation Demo - Part of the MRPT project"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("id"));
    {
    	wxIcon FrameIcon;
    	FrameIcon.CopyFromBitmap(wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("MAIN_ICON")),wxART_FRAME_ICON));
    	SetIcon(FrameIcon);
    }
    FlexGridSizer1 = new wxFlexGridSizer(3, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(1);
    Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer2 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    FlexGridSizer3 = new wxFlexGridSizer(0, 5, 0, 0);
    FlexGridSizer3->AddGrowableCol(3);
    btnStart = new wxButton(Panel1, ID_BUTTON1, _("Simulate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer3->Add(btnStart, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    btnPause = new wxButton(Panel1, ID_BUTTON2, _("Pause"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    btnPause->Disable();
    FlexGridSizer3->Add(btnPause, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    cbLog = new wxCheckBox(Panel1, ID_CHECKBOX3, _("Generate navigation log file"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
    cbLog->SetValue(false);
    FlexGridSizer3->Add(cbLog, 1, wxALL|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
    FlexGridSizer3->Add(-1,-1,1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    btnExit = new wxButton(Panel1, ID_BUTTON3, _("EXIT"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer3->Add(btnExit, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer4->AddGrowableCol(0);
    FlexGridSizer4->AddGrowableCol(1);
    StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Obstacle grid map "));
    FlexGridSizer5 = new wxFlexGridSizer(3, 1, 0, 0);
    cbExtMap = new wxCheckBox(Panel1, ID_CHECKBOX1, _("Use internal default map"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    cbExtMap->SetValue(true);
    FlexGridSizer5->Add(cbExtMap, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, _("External map file:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer5->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    edMapFile = new wxTextCtrl(Panel1, ID_TEXTCTRL2, _("./obstacles_map.gridmap"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    edMapFile->Disable();
    FlexGridSizer5->Add(edMapFile, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer1->Add(FlexGridSizer5, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    FlexGridSizer4->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer3 = new wxStaticBoxSizer(wxVERTICAL, Panel1, _("Navigation parameters"));
    FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
    btnEditNavParams = new wxButton(Panel1, ID_BUTTON7, _("Edit navig. parameters..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
    FlexGridSizer7->Add(btnEditNavParams, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    cbInternalParams = new wxCheckBox(Panel1, ID_CHECKBOX2, _("Use external config files:"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    cbInternalParams->SetValue(false);
    FlexGridSizer7->Add(cbInternalParams, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7->Add(-1,-1,1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer3->Add(FlexGridSizer7, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    FlexGridSizer8 = new wxFlexGridSizer(1, 2, 0, 0);
    FlexGridSizer8->AddGrowableCol(1);
    StaticText6 = new wxStaticText(Panel1, ID_STATICTEXT6, _("Navigation parameters:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer8->Add(StaticText6, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    edNavCfgFile = new wxTextCtrl(Panel1, ID_TEXTCTRL6, _("./CONFIG_ReactiveNavigator.ini"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL6"));
    edNavCfgFile->Disable();
    FlexGridSizer8->Add(edNavCfgFile, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer3->Add(FlexGridSizer8, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    FlexGridSizer4->Add(StaticBoxSizer3, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Navigation target:"));
    FlexGridSizer6 = new wxFlexGridSizer(2, 3, 0, 0);
    StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("x="), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer6->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    edX = new wxTextCtrl(Panel1, ID_TEXTCTRL3, _("5"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
    FlexGridSizer6->Add(edX, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText4 = new wxStaticText(Panel1, ID_STATICTEXT4, _("(Right click on map for an\n easier way of entering commands)"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE, _T("ID_STATICTEXT4"));
    FlexGridSizer6->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(Panel1, ID_STATICTEXT3, _("y="), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer6->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    edY = new wxTextCtrl(Panel1, ID_TEXTCTRL4, _("5"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL4"));
    FlexGridSizer6->Add(edY, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    btnNavigate = new wxButton(Panel1, ID_BUTTON4, _("SET TARGET"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    btnNavigate->SetDefault();
    FlexGridSizer6->Add(btnNavigate, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2->Add(FlexGridSizer6, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    FlexGridSizer4->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer4, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    Panel1->SetSizer(FlexGridSizer2);
    FlexGridSizer2->Fit(Panel1);
    FlexGridSizer2->SetSizeHints(Panel1);
    FlexGridSizer1->Add(Panel1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    plot = new mpWindow(this,ID_CUSTOM1,wxPoint(192,240),wxSize(496,346),0);
    FlexGridSizer1->Add(plot, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    edLog = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(496,166), wxTE_MULTILINE|wxTE_READONLY|wxVSCROLL, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    wxFont edLogFont(7,wxTELETYPE,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    edLog->SetFont(edLogFont);
    FlexGridSizer1->Add(edLog, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    SetSizer(FlexGridSizer1);
    StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[2] = { -10, -30 };
    int __wxStatusBarStyles_1[2] = { wxSB_NORMAL, wxSB_NORMAL };
    StatusBar1->SetFieldsCount(2,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(2,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);
    timSimulate.SetOwner(this, ID_TIMER1);
    timSimulate.Start(20, false);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
    Center();

    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ReactiveNavigationDemoFrame::OnbtnStartClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ReactiveNavigationDemoFrame::OnbtnPauseClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ReactiveNavigationDemoFrame::OnbtnExitClick);
    Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ReactiveNavigationDemoFrame::OnrbExtMapSelect);
    Connect(ID_BUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ReactiveNavigationDemoFrame::OnbtnEditNavParamsClick);
    Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ReactiveNavigationDemoFrame::OncbInternalParamsClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ReactiveNavigationDemoFrame::OnbtnNavigateClick);
    plot->Connect(wxEVT_MOTION,(wxObjectEventFunction)&ReactiveNavigationDemoFrame::OnplotMouseMove,0,this);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&ReactiveNavigationDemoFrame::OntimSimulateTrigger);
    //*)

	Connect( ID_MENUITEM_SET_reactivenav_TARGET, wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction) &ReactiveNavigationDemoFrame::OnreactivenavTargetMenu );


    lyGridmap = new mpBitmapLayer();
    plot->AddLayer(lyGridmap);

    plot->AddLayer( new mpScaleX());
    plot->AddLayer( new mpScaleY());

    lyVehicle = new mpPolygon();
    lyTarget  = new mpPolygon();
    lyLaserPoints = new mpPolygon();

	plot->AddLayer( lyLaserPoints );
	plot->AddLayer( lyVehicle );
	plot->AddLayer( lyTarget );

	{
		vector<float> xs(5),ys(5);
		float CS = 0.4f;  // Cross size
		xs[0] = -CS;   ys[0] = -CS;
		xs[1] =  CS;   ys[1] =  CS;
		xs[2] =   0;   ys[2] =   0;
		xs[3] = -CS;   ys[3] =  CS;
		xs[4] =  CS;   ys[4] = -CS;

		lyTarget->setPoints(xs,ys,false);
	}

    lyVehicle->SetPen( wxPen(wxColour(255,0,0),2) );
    lyTarget->SetPen( wxPen(wxColour(0,0,255),2) );

    lyLaserPoints->SetPen( wxPen(wxColour(255,0,0),5) );
    lyLaserPoints->SetContinuity(false);

    plot->LockAspect(true);
    plot->EnableDoubleBuffer(true);

    Maximize();



	wxMenu *popupMnu = plot->GetPopupMenu();

	popupMnu->InsertSeparator(0);

	wxMenuItem *mnuTarget = new wxMenuItem(popupMnu, ID_MENUITEM_SET_reactivenav_TARGET, _("Navigate to this point"), wxEmptyString, wxITEM_NORMAL);
	popupMnu->Insert(0,mnuTarget);


	// Redirect all output to control:
	myRedirector = new CMyRedirector( edLog, true, 10 );

	// Create dialogs:
	iniEditoreactivenav = new CIniEditor(this);

	wxString	auxStr = 
"# ------------------------------------------------------------------------\n"
"# Example configuration file for MRPT's Reactive Navigation engine.\n"
"# See C++ documentation: http://reference.mrpt.org/svn/classmrpt_1_1nav_1_1_c_reactive_navigation_system.html\n"
"# See ROS node documentation: http://wiki.ros.org/mrpt_reactivenav2d\n"
"# ------------------------------------------------------------------------\n"
"\n"
"[GLOBAL_CONFIG]\n"
"# 0: Virtual Force Field\n"
"# 1: Nearness Diagram (ND)\n"
"HOLONOMIC_METHOD=1\n"
"\n"
"ALARM_SEEMS_NOT_APPROACHING_TARGET_TIMEOUT=100    # (ms)\n"
"\n"
"#    Parameters for the \"Nearness diagram\" Holonomic method\n"
"# ----------------------------------------------------\n"
"[ND_CONFIG]\n"
"factorWeights=1.0 0.5 2.0 0.5\n"
"# 1: Free space\n"
"# 2: Dist. in sectors\n"
"# 3: Closer to target (euclidean)\n"
"# 4: Hysteresis\n"
"WIDE_GAP_SIZE_PERCENT            = 0.25\n"
"MAX_SECTOR_DIST_FOR_D2_PERCENT   = 0.25\n"
"RISK_EVALUATION_SECTORS_PERCENT  = 0.25\n"
"RISK_EVALUATION_DISTANCE         = 0.5  # In normalized ps-meters [0,1]\n"
"TARGET_SLOW_APPROACHING_DISTANCE = 0.8    # For stop gradually\n"
"TOO_CLOSE_OBSTACLE               = 0.03 # In normalized ps-meters\n"
"\n"
"#    Parameters for the \"VFF\" Holonomic method\n"
"# ----------------------------------------------------\n"
"[VFF_CONFIG]\n"
"# Used to decrease speed gradually when the target is going to be reached\n"
"TARGET_SLOW_APPROACHING_DISTANCE = 0.8    \n"
"# Use it to control the relative weight of the target respect to the obstacles\n"
"TARGET_ATTRACTIVE_FORCE = 7.5\n"
"\n"
"# ----------------------------------------------------\n"
"#    Parameters for navigation\n"
"# ----------------------------------------------------\n"
"[ReactiveParams]\n"
"weights=0.5 0.05 0.5 2.0 0.2 0.1\n"
"# 1: Free space\n"
"# 2: Dist. in sectors            \n"
"# 3: Heading toward target\n"
"# 4: Closer to target (euclidean)\n"
"# 5: Hysteresis\n"
"# 6: Security Distance\n"
"\n"
"DIST_TO_TARGET_FOR_SENDING_EVENT=0.6    # Minimum. distance to target for sending the end event. Set to 0 to send it just on navigation end\n"
"\n"
"MinObstaclesHeight=0.0         # Minimum coordinate in the \"z\" axis for an obstacle to be taken into account.\n"
"MaxObstaclesHeight=1.40     # Maximum coordinate in the \"z\" axis for an obstacle to be taken into account.\n"
"\n"
"robotMax_V_mps   = 1.0        # Speed limits\n"
"robotMax_W_degps = 60\n"
"\n"
"MAX_REFERENCE_DISTANCE  = 8.0       # Maximum distance to build PTGs (in meters), i.e. the visibility \"range\" of tentative paths\n"
"LUT_CELL_SIZE           = 0.05      # Look-up-table cell size, or resolution (in meters)\n"
"\n"
"# The constant time of a first-order low-pass filter of outgoing speed commands, \n"
"# i.e. can be used to impose a maximum acceleration.\n"
"SPEEDFILTER_TAU         = 0         \n"
"\n"
"# PTGs: See classes derived from mrpt::nav::CParameterizedTrajectoryGenerator ( http://reference.mrpt.org/svn/classmrpt_1_1nav_1_1_c_parameterized_trajectory_generator.html)# refer to papers for details.\n"
"#------------------------------------------------------------------------------\n"
"# Types:\n"
"# 1 - Circular arcs \n\"\n"
"# 2 - alpha - A, Trajectories with asymptotical heading\n"
"# 3 - C|C,S, R = vmax/wmax, Trajectories to move backward and then forward\n"
"# 4 - C|C,s, like PTG 3, but if t > threshold -> v = w = 0\n"
"# 5 - CS, Trajectories with a minimum turning radius\n"
"# 6 - alpha - SP, Trajectories built upon a spiral segment\n"
"\n"
"PTG_COUNT      = 3\n"
"\n"
"PTG0_Type      = 1\n"
"PTG0_nAlfas    = 121\n"
"PTG0_v_max_mps = 1.0\n"
"PTG0_w_max_gps = 60\n"
"PTG0_K         = 1.0\n"
"\n"
"PTG1_Type        = 2\n"
"PTG1_nAlfas      = 121\n"
"PTG1_v_max_mps   = 1.0\n"
"PTG1_w_max_gps   = 60\n"
"PTG1_cte_a0v_deg = 57\n"
"PTG1_cte_a0w_deg = 57\n"
"\n"
"PTG2_Type      = 1\n"
"PTG2_nAlfas    = 121\n"
"PTG2_v_max_mps = 1.0\n"
"PTG2_w_max_gps = 60\n"
"PTG2_K         = -1.0\n"
"\n"
"\n"
"# Default 2D robot shape for collision checks: (ignored in ROS, superseded by node parameters)\n"
"RobotModel_shape2D_xs=-0.2 0.5 0.5 -0.2\n"
"RobotModel_shape2D_ys=0.3 0.3 -0.3 -0.3\n"
"\n"
"ROBOTMODEL_DELAY=0  # (un-used param, must be present for compat. with old mrpt versions)\n"
"ROBOTMODEL_TAU=0 # (un-used param, must be present for compat. with old mrpt versions)\n";

	iniEditoreactivenav->edText->SetValue( auxStr );

	EDIT_internalCfgReactive = string( iniEditoreactivenav->edText->GetValue().mb_str() );

	// Try to load the map:
	reloadMap();
	reloadRobotShape();

	// Set simulator params:
	plot->Fit();
}

ReactiveNavigationDemoFrame::~ReactiveNavigationDemoFrame()
{
    //(*Destroy(ReactiveNavigationDemoFrame)
    //*)

    delete reacNavObj; reacNavObj = NULL;
	delete myRedirector; myRedirector = NULL;

	delete iniEditoreactivenav;
}

void ReactiveNavigationDemoFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void ReactiveNavigationDemoFrame::OnAbout(wxCommandEvent& event)
{
}

void ReactiveNavigationDemoFrame::OnbtnStartClick(wxCommandEvent& event)
{
	if (!reloadMap()) return;

	tryConstructReactiveNavigator();

	if (!reacNavObj)
	{
		wxMessageBox( _("Could not create the reactive navigation object! Check the log output for error messages."), wxT("Error"), wxOK, this);
		return;
	}


	// Enable buttons:
	btnPause->Enable(true);
	btnStart->Enable(false);


	// Set timer to start simulation:
    timSimulate.Start(SIMULATION_TIME_STEPS, false); // No One-shot
}

void ReactiveNavigationDemoFrame::OnbtnPauseClick(wxCommandEvent& event)
{
	// Enable buttons:
	btnPause->Enable(false);
	btnStart->Enable(true);

}

void ReactiveNavigationDemoFrame::OnbtnExitClick(wxCommandEvent& event)
{
    Close();
}

// objects must be DELETED by the caller!
void createConfigSources(
	ReactiveNavigationDemoFrame *frame,
	CConfigFileBase    *& iniReactive)
{
	// Set config files/internal data:
	if ( frame->cbInternalParams->GetValue() )
	{
		// Use external files:
		string filReac( frame->edNavCfgFile->GetValue().mb_str()  );

		if ( !mrpt::system::fileExists( filReac ))
		{
			wxMessageBox( _U( format("Cannot open file : '%s'",filReac.c_str()).c_str()), wxT("Error"), wxOK, frame);
			return;
		}
		iniReactive = new CConfigFile( filReac );
	}
	else
	{
		// Use internal strings:
		iniReactive = new CConfigFileMemory( EDIT_internalCfgReactive );
	}
}

void ReactiveNavigationDemoFrame::tryConstructReactiveNavigator()
{
	try
	{
		myReactiveInterface.the_frame = this;

		CConfigFileBase    *iniReactive=NULL;

		// Get ini-data:
		createConfigSources(this, iniReactive);
		if (!iniReactive) return;

		if (!reacNavObj)
		{
			const bool ENABLE_reactivenav_LOG_FILES = cbLog->GetValue();

			// Create reactive nav. object:
			reacNavObj = new CReactiveNavigationSystem(
				myReactiveInterface,
				true,
				ENABLE_reactivenav_LOG_FILES );

			reacNavObj->enableTimeLog();
		}

		// Reload config:
		reacNavObj->loadConfigFile(*iniReactive);

		delete iniReactive;

		// load robot shape:
		reloadRobotShape();

		reacNavObj->initialize();

	}
	catch(std::exception &e)
	{
		cout << e.what(); // Redirected to the log control
		cerr << e.what();
	}
}

void ReactiveNavigationDemoFrame::OnbtnNavigateClick(wxCommandEvent& event)
{

	if (!reacNavObj)
	{
		// Initialized ok?
		tryConstructReactiveNavigator();

		if (!reacNavObj)
		{
			wxMessageBox( _("Could not create the reactive navigation object! Check the log output for error messages."), wxT("Error"), wxOK, this);
			return;
		}
	}


	// Send navigation command to navigator:
	wxString  strX = edX->GetValue();
	wxString  strY = edY->GetValue();
	double   x, y;
	if (!strX.ToDouble( &x )) {  wxMessageBox( _("'x' is not a valid number"), wxT("Error"), wxOK, this); return; }
	if (!strY.ToDouble( &y )) {  wxMessageBox( _("'y' is not a valid number"), wxT("Error"), wxOK, this); return; }

	lyTarget->SetCoordinateBase( x,y );
	plot->Refresh();

	if (reacNavObj)
	{
		//CAbstractReactiveNavigationSystem::TNavigationParams   navParams;
		CAbstractPTGBasedReactive::TNavigationParamsPTG   navParams;
		navParams.target.x = x ;
		navParams.target.y = y ;
		navParams.targetAllowedDistance = 0.40f;
		navParams.targetIsRelative = false;

		// Optional: restrict the PTGs to use
		//navParams.restrict_PTG_indices.push_back(1);

		reacNavObj->navigate( &navParams );
	}
}


bool ReactiveNavigationDemoFrame::reloadMap()
{
    try
	{
		// Load internal or external map?
		if (cbExtMap->GetValue())
		{
			// Internal:
			CMemoryStream  s( DEFAULT_GRIDMAP_DATA, sizeof(DEFAULT_GRIDMAP_DATA) );
			s >> gridMap;
		}
		else
		{
			// External
			string filName = string(edMapFile->GetValue().mb_str());
			if ( !mrpt::system::fileExists( filName ) )
			{
				wxMessageBox( _U( format("Grid map file '%s'  cannot be found.\nSimulation cannot start until a valid map is provided",filName.c_str()).c_str() ) , wxT("Error"), wxOK, this);
				return false;
			}

			CFileGZInputStream  f( filName  );
			f >> gridMap;
		}

		// Set the map image:
		CImage imgGrid;
		gridMap.getAsImage(imgGrid);
		wxImage *newBmp = mrpt::gui::MRPTImage2wxImage( imgGrid );
		double lx = gridMap.getXMax()-gridMap.getXMin();
		double ly = gridMap.getYMax()-gridMap.getYMin();
		lyGridmap->SetBitmap(
			*newBmp,
			gridMap.getXMin(),
			gridMap.getYMin(),
			lx,
			ly );
		delete newBmp;


		// Refresh display:
		plot->Refresh();

		return true;
    }
	catch(std::exception &e)
    {
        wxMessageBox( _U( e.what() ), _("Exception"), wxOK, this);
		return false;
    }
    catch(...)
    {
        wxMessageBox( _("Untyped exception!"), _("Exception"), wxOK, this);
		return false;
    }
}

void ReactiveNavigationDemoFrame::OnplotMouseMove(wxMouseEvent& event)
{
	int X, Y;
	event.GetPosition(&X,&Y);
    curCursorPos.x = plot->p2x(X);
    curCursorPos.y = plot->p2y(Y);

	StatusBar1->SetStatusText(_U(format("X=%.03f Y=%.04f",curCursorPos.x,curCursorPos.y).c_str()), 0);

    event.Skip();
}


void ReactiveNavigationDemoFrame::OnreactivenavTargetMenu(wxCommandEvent& event)
{
	edX->SetValue( _U( format("%.03f", curCursorPos.x ).c_str() ) );
	edY->SetValue( _U( format("%.03f", curCursorPos.y ).c_str() ) );

	OnbtnNavigateClick( event );
}

void ReactiveNavigationDemoFrame::OntimSimulateTrigger(wxTimerEvent& event)
{
	static bool IamIN = false;
	if (IamIN) return;
	IamIN=true;

	WX_START_TRY

	if ( btnStart->IsEnabled() )
	{
		wxCommandEvent dummy;
		OnbtnPauseClick( dummy );
		IamIN=false;

		return; // We are paused!
	}

	if (reacNavObj==NULL)
	{
		IamIN=false;
		return;
	}

	// Enable/disable log files on-the-fly:
	reacNavObj->enableLogFile( cbLog->GetValue() );

	// Navigation end?
	if (reacNavObj->getCurrentState() != CAbstractReactiveNavigationSystem::NAVIGATING )
	{
		reacNavObj->getTimeLogger().dumpAllStats();

		cout << endl << "NAVIGATION FINISHED - Select a new target and press 'Start' again." << endl;
		wxCommandEvent dummy;
		OnbtnPauseClick( dummy );
		IamIN=false;
		return;
	}

	// Empty log?
	if (edLog->GetNumberOfLines()>300)
	{
		edLog->Clear();
	}

	// Go on, simulate one time step:
	// Robot sim:
	robotSim.simulateInterval( 0.001 * SIMULATION_TIME_STEPS );

	reacNavObj->navigationStep();

	// Update the target & robot pose & redraw:

	lyVehicle->SetCoordinateBase(
		robotSim.getX(),
		robotSim.getY(),
		robotSim.getPHI() );


	plot->Refresh();

	StatusBar1->SetStatusText(
		_U(format("Pose=(%.03f,%.03f,%.02fdeg) (v,w)=(%.03f,%.02f)",
		robotSim.getX(),
		robotSim.getY(),
		RAD2DEG(robotSim.getPHI()),
		robotSim.getV(),
		RAD2DEG(robotSim.getW()) ).c_str() ), 1 );


	// Set timer to continue simulation:
    //timSimulate.Start(SIMULATION_TIME_STEPS, true); // One-shot

	WX_END_TRY
	IamIN=false;
}

void ReactiveNavigationDemoFrame::reloadRobotShape()
{
	try
	{
		// Get ini-data:
		CConfigFileBase    *iniReactive=NULL;
		createConfigSources(this, iniReactive);
		if (!iniReactive) return;

		const string sectName = "ReactiveParams";
		vector<float> xs,ys;

		iniReactive->read_vector(sectName,"RobotModel_shape2D_xs",vector<float>(0), xs, true );
		iniReactive->read_vector(sectName,"RobotModel_shape2D_ys",vector<float>(0), ys, true );

		delete iniReactive; iniReactive=NULL;

		lyVehicle->setPoints(xs,ys,true);
	}
	catch( std::exception & e)
	{
		cout << e.what() << endl;
		cerr << e.what() << endl;
	}
}

void ReactiveNavigationDemoFrame::OnbtnResetClick(wxCommandEvent& event)
{
}

void ReactiveNavigationDemoFrame::OnbtnEditNavParamsClick(wxCommandEvent& event)
{
	iniEditoreactivenav->Center();
    if (iniEditoreactivenav->ShowModal())
		EDIT_internalCfgReactive = string( iniEditoreactivenav->edText->GetValue().mb_str() );
}

void ReactiveNavigationDemoFrame::OnrbExtMapSelect(wxCommandEvent& event)
{
    edMapFile->Enable( ! cbExtMap->GetValue() );
}

void ReactiveNavigationDemoFrame::OncbInternalParamsClick(wxCommandEvent& event)
{
    btnEditNavParams->Enable( ! cbInternalParams->GetValue() );
    edNavCfgFile->Enable( cbInternalParams->GetValue() );
}



// Debug windows:
#if 0
	if (m_debugWindows)
	{
		if (!m_debugWin_WS.present())
			m_debugWin_WS = mrpt::gui::CDisplayWindowPlotsPtr( new mrpt::gui::CDisplayWindowPlots("[ReactiveNav] Workspace") );

		// Plot obstacles:
		{
			vector<float> xs,ys;
			CSimplePointsMap	pointsTrans = WS_Obstacles;
			pointsTrans.changeCoordinatesReference(curPose);

			pointsTrans.getAllPoints(xs,ys);
			m_debugWin_WS->plot(xs,ys,"b.3","obstacles");
		}

		// Plot robot shape:
		{
			vector<double> xs,ys;
			robotShape.getAllVertices(xs,ys);
			if (!xs.empty()) { xs.push_back(xs[0]); ys.push_back(ys[0]); }
			for (size_t i=0;i<xs.size();i++)
			{
				const CPoint2D  p = curPose + CPoint2D(xs[i],ys[i]);
				xs[i] = p.x();
				ys[i] = p.y();
			}
			m_debugWin_WS->plot(xs,ys,"r-2","shape");
		}

		// Plot current dir:
		{

			vector<double> xs(2),ys(2);
			xs[0] = curPose.x();
			ys[0] = curPose.y();

			xs[1] = curPose.x() + cos(cur_approx_heading_dir+curPose.phi()) * 1.5;
			ys[1] = curPose.y() + sin(cur_approx_heading_dir+curPose.phi()) * 1.5;

			m_debugWin_WS->plot(xs,ys,"b-","cur_dir");
		}

		// Plot target point:
		{
			vector<double> xs,ys;
			xs.push_back( m_navigationParams.target.x );
			ys.push_back( m_navigationParams.target.y );
			m_debugWin_WS->plot(xs,ys,"k.7","target");
		}

		m_debugWin_WS->axis_fit(true);
	}
#endif
