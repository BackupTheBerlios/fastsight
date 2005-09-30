#include "cfgmanager.h"
#include "interface.h"
#include "filters.h"
#include "filtercfg.h"

int main(int argc, char **argv)
{
  if(!cfgmanager_init())
    return 1;
  if(!filters_init())
    return 1;
  make_config_wnd();
  make_about_wnd();
  make_filters_wnd();
  make_main_wnd();
  make_connect_wnd()->show();
  return Fl::run();
}

