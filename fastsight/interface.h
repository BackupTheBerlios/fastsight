// generated by Fast Light User Interface Designer (fluid) version 1.0106

#ifndef interface_h
#define interface_h
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
extern Fl_Double_Window *mainwnd;
#include <FL/Fl_Box.H>
extern Fl_Box *videoarea;
#include <FL/Fl_Button.H>
extern Fl_Box *statusbar;
Fl_Double_Window* make_main_wnd();
extern Fl_Double_Window *configwnd;
#include <FL/Fl_Input.H>
extern Fl_Input *listenport;
#include <FL/Fl_File_Input.H>
extern Fl_File_Input *shotdir;
extern Fl_Input *j2krate;
#include <FL/Fl_Return_Button.H>
Fl_Double_Window* make_config_wnd();
extern Fl_Double_Window *connectwnd;
extern Fl_Input *txtcode;
#include <FL/Fl_Group.H>
#include <FL/Fl_Round_Button.H>
extern Fl_Round_Button *chkview;
extern Fl_Round_Button *chksend;
Fl_Double_Window* make_connect_wnd();
extern Fl_Double_Window *aboutwnd;
Fl_Double_Window* make_about_wnd();
void show_config_wnd();
#endif