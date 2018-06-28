#include <gtkmm.h>
#include <gtkglmm.h>

#include "IQWin.h"

int main(int argc, char** argv) {
  Gtk::Main kit(argc, argv);

  Glib::thread_init();

  //
  // Init gtkglextmm.
  //

  Gtk::GL::init(argc, argv);

  //
  // Parse arguments.
  //

  char sIcoFile[512];
  char sDataFile[512];
  *sIcoFile  = '\0';
  *sDataFile = '\0';
  bool bPreSel = true;
  if (argc > 1) {
      if (argc > 2) {
          strcpy(sDataFile, argv[2]);
      }
      strcpy(sIcoFile, argv[1]);
  }

  //
  // Instantiate and run the application.
  //
  
  IQWin iqwin(sIcoFile, sDataFile, bPreSel);

  kit.run(iqwin);
  printf("bye\n");
  return 0;
}
