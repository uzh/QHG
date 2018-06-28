#include <gtkmm.h>
#include <gtkglmm.h>

#include "GridWin.h"

int main(int argc, char** argv) {
  Gtk::Main kit(argc, argv);

  //
  // Init gtkglextmm.
  //
  
  Gtk::GL::init(argc, argv);

  printf("\e[5;01;31m");
  printf("TODTODOTODOTDOTODO\n");
  printf("- Tiling: from LoadBalancer, but neeed different weight array (not int**)\n");
  printf("          need something which can be used for ico as well\n");
  printf("          (maybe nodelist?)\n");
  printf("\e[0m");



  //
  // Parse arguments.
  //

  //  bool is_sync = true;
  char sQMap[512];
  strcpy(sQMap, "/mnt/data1/neander/data/wwwf8A_2i.qmap");
  for (int i = 1; i < argc; ++i) {
      if (strcmp(argv[i], "--async") == 0) {
          //is_sync = false;
      } else if (strcmp(argv[i], "-q") == 0) {
          strcpy(sQMap, argv[i+1]);
      }
  }

  //
  // Instantiate and run the application.
  //
  printf("passing file [%s]\n", sQMap);
  GridWin gridwin(sQMap);

  kit.run(gridwin);
  printf("bye\n");
  return 0;
}
