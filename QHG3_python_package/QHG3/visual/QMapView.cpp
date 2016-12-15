#include <string.h>
#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>

#include "xbv.h"




int main(int argc, char *argv[])
{
    Gtk::Main kit(argc, argv);
    printf("Guguseligug\n");
    char *pCommands = NULL;
    bool bInteractive = true;
    xbv m_xbv;
    bool bExtract=false;
    int iResult = 0;
    if (argc > 1) {
        printf("Have more than 1 arg\n");
        if (argc > 2) {
            printf("Have more than 2 args (2=[%s])\n", argv[2]);
            if (strstr(argv[2], "--command=") == argv[2]) {
                pCommands = strchr(argv[2], '=');
                pCommands++;
                bInteractive = false;
                printf("AHA! Command:[%s]\n", pCommands);
            } else {
                if (strcmp(argv[2], "-v") == 0) {
                    printf("Loading vecs\n");
                    m_xbv.loadVectors(argv[3]);
                } else if (strcmp(argv[2], "-a") == 0) {
                    m_xbv.loadArcs(argv[3]);
                } else if (strcmp(argv[2], "-x") == 0) {
                    bExtract=true;
                }


            }
        }
        if (bExtract) {
            iResult = m_xbv.setReader(argv[1], argv[3]);
        } else {
            iResult = m_xbv.setReader(argv[1], NULL);
        }
        if  ((argc > 2) &&(strcmp(argv[2], "-v") == 0)) {
            printf("setting to thresh\n");
            m_xbv.setLookUp("Thresh", "0:0");
        }

        if (iResult != 0) {
            char s[256];
            sprintf(s, "Can't show %s\n", argv[1]);
            Gtk::MessageDialog dialog(m_xbv, s, false, Gtk::MESSAGE_ERROR);
            switch (iResult) {
            case -1:
                sprintf(s, "Couldn't create reader for %s", argv[1]);
                dialog.set_secondary_text(s);
                break;
            case -2:
                sprintf(s, "File [%s] doesn't exist", argv[1]);
                dialog.set_secondary_text(s);
                break;
            case -3:
                dialog.set_secondary_text("Unsupported file type");
                break;
            default:
                sprintf(s, "Unknown error (%d)", iResult);
                dialog.set_secondary_text(s);
                break;
            }
            dialog.run();
        }
        
    }
    
    if (bInteractive) {
        //  m_xbv.setImage("xxx.png");
        //Shows the window and returns when it is closed.
        if (iResult == 0) {
            Gtk::Main::run(m_xbv);
        }
        //    fprintf(stderr, "\n");
        
    } else {
        printf("Calling for command:[%s]\n", pCommands);

        m_xbv.doCommands(pCommands);
    }       
 
    return 0;
}
