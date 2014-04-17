/*Copyright Sebastien Tricoire (29/05/2011)

3dsman@free.fr

This software is a computer program whose purpose is to slice 3d models in realtime and control hardware of a dlp 3d printer.
You can find more information here: http://forum.allinbox.com/aspectgeek/AutresElectronique/3dprint-logiciel-slicer-sujet_8661_1.htm

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software.  You can  use,
modify and/or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info".

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability.

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or
data to be ensured and,  more generally, to use and operate it in the
same conditions as regards security.

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.
*/

#include <cstdlib>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>//#include <windows.h>
#include <tinythread.h>

using namespace std;
using namespace tthread;

int main(int argc, char *argv[])
{
    int waitTime = 10;

    for(int i=1; i < argc; i++) {
        if ((strcmp(argv[i],"--help")==0)||(strcmp(argv[i],"-help")==0)||(strcmp(argv[i],"--h")==0)||(strcmp(argv[i],"-h")==0))
        {
           printf("exemple: slicer -nextSlice\n\n");
           //cout<<"exemple: slicer -nextSlice\n\n";
           return 0;
        }

        if ((strcmp(argv[i],"-sleep")==0)&&(argc>i+1))
        {
            waitTime = atoi(argv[++i]);
        }

        if (((strcmp(argv[i],"-init")==0)||(strcmp(argv[i],"-i")==0)))
        {
	        printf( "init\n");
	        //cout<<"init\n";
        }
        if (((strcmp(argv[i],"-start")==0)||(strcmp(argv[i],"-s")==0)))
        {
            printf( "start\n");
	        //cout<<"start\n";
        }
        if (((strcmp(argv[i],"-end")==0)||(strcmp(argv[i],"-e")==0)))
        {
	        printf( "end\n");
	        //cout<<"end\n";
        }
        if (((strcmp(argv[i],"-nextSlice")==0)||(strcmp(argv[i],"-n")==0)))
        {
	        //printf( "nextSlice\n");
	        cout<< "nextSlice\n";
            tthread::chrono::milliseconds dura( (int)(waitTime));
            this_thread::sleep_for( dura );
	        //sleep(waitTime);
        }
        if ((strcmp(argv[i],"-infos")==0)&&(argc>i+1))
        {
            int curSlice = atoi(argv[++i]);
            int totalSlice = atoi(argv[++i]);
            int restTime = atoi(argv[++i]);
            printf( "slice %i/%i resting time: %02i:%02i:%02i\n",curSlice,totalSlice,restTime/3600,(restTime/60)%60,restTime%60);
	        //cout<< "slice "<<curSlice<<"/"<<totalSlice<<" resting time: "<< restTime/3600 <<":"<< (restTime/60)%60 <<":"<< restTime%60 <<"\n";
        }
    }

    return EXIT_SUCCESS;
}
