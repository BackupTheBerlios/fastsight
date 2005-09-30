// generated by Fast Light User Interface Designer (fluid) version 1.0106

#ifndef fsvplay_h
#define fsvplay_h
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
extern Fl_Double_Window *mainwnd;
#include <FL/Fl_Box.H>
extern Fl_Box *videoarea;
#include <FL/Fl_Button.H>
#include <FL/Fl_Slider.H>
extern Fl_Slider *position;
Fl_Double_Window* make_main_wnd();
void show_video(void*);
void make_index();
int main(int argc, char **argv);
#endif
