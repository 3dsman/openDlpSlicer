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


#include <stdlib.h>    // For malloc() etc.
#include <stdio.h>     // For printf(), fopen() etc.
#include "tinyxml/tinyxml2.h"
#include "prefs.h"
using namespace tinyxml2;

//----------------------------------------------------------------------
// Prefs() - Constructor
//----------------------------------------------------------------------

Prefs::Prefs()
{ }

//----------------------------------------------------------------------
// ~Prefs() - Destructor
//----------------------------------------------------------------------

Prefs::~Prefs()
{ }

//----------------------------------------------------------------------
// Load() - Load preferences from prefs.xml file
//----------------------------------------------------------------------

bool Prefs::Load()
{
    XMLDocument doc;
    string filename = "prefs.xml";
	bool loadError = doc.LoadFile(filename.c_str());

    // init some variables
    Thickness = 0.1;
    Exposure = 2;
    Volume.X=100;
    Volume.Y=150;
    Volume.Z=75;

    //if file doesn't exist then create it
	if ( loadError )
	{
		cout<< "Could not load params file 'prefs.xml'. create a new one.\n";
    	const char* paramsInit =
    	//"<?xml version=\"1.0\"  standalone='no' >\n"
    	"<!-- prefs for slicer -->"
    	"<prefs>\n"
    	"<!-- temps d'exposition d'une tranche en s -->"
    	"<exposure value=\"2\" />"
    	"<!-- epaisseur des tranches en mm -->"
    	"<thickness value=\"0.1\" />"
        "<!-- nombre de pas du moteur pour 1mm -->"
        "<steps value=\"48\" />"
    	"<!-- volume utile en mm (plan image: xz) -->"
    	"<volume x=\"100\" y=\"150\" z=\"75\" />"
    	"<!-- options de commande de l'exe de com -->"
    	"<command com=\"slicer_test -sleep 1000\" />"
    	"</prefs>";
    	//XMLError error;
    	doc.Parse( paramsInit );

    	if ( doc.Error() )
    	{
            doc.PrintError ();
    		//printf( "Error in %s: %s\n", doc.Value(), doc.ErrorDesc() );
    		exit( 1 );
    	}
    	doc.SaveFile(filename.c_str());
	}
	else
	{
        //load every params
        //XMLHandle hdl(&doc);
        XMLElement *elem = doc.FirstChildElement("prefs")->FirstChildElement("exposure");

        if(elem){
            elem->QueryFloatAttribute( "value", &Exposure );
	        cout<< "pref->exposure = "<< Exposure <<"\n" ;
        }else{
		    cout<< "pas de noeud exposure dans le fichier de prefs\n";
        }

        elem = doc.FirstChildElement("prefs")->FirstChildElement("thickness");
        if(elem){
            elem->QueryFloatAttribute( "value", &Thickness );
	        cout<< "pref->thickness = "<< Thickness<<"\n";
        }else{
	       cout<< "pas de noeud thickness dans le fichier de prefs\n";
        }
        elem = doc.FirstChildElement("prefs")->FirstChildElement("volume");
        if(elem){
            elem->QueryFloatAttribute( "x", &Volume.X );
            elem->QueryFloatAttribute( "y", &Volume.Y );
            elem->QueryFloatAttribute( "z", &Volume.Z );
	       cout<< "pref->volume = "<<Volume.X<<" "<< Volume.Y<<" "<< Volume.Z<<"\n";
        }else{
	       cout<< "pas de noeud volume dans le fichier de prefs\n";
        }
        elem = doc.FirstChildElement("prefs")->FirstChildElement("command");
        if(elem){
            Command = string(elem->Attribute("com"));
            //strcpy(&Command,elem->Attribute("com") );
	        cout<< "pref->command = "<<Command<<"\n";
        }else{
	       cout<< "pas de noeud command dans le fichier de prefs\n";
        }
        elem = doc.FirstChildElement("prefs")->FirstChildElement("steps");
        if(elem){
            elem->QueryIntAttribute( "value", &Steps );
	        cout<< "pref->steps = "<< Steps <<"\n";
        }else{
	       cout<< "pas de noeud steps dans le fichier de prefs\n" ;
        }
    }

	return 0;
}

//----------------------------------------------------------------------
// Save() - Save preferences to prefs.xml file (to write)
//----------------------------------------------------------------------

bool Prefs::Save()
{
   return 0;
}

//----------------------------------------------------------------------
// GetThickness() - Return Thickness
//----------------------------------------------------------------------

float Prefs::GetThickness()
{
    return Thickness;
}

//----------------------------------------------------------------------
// GetExposure() - Return Exposure
//----------------------------------------------------------------------

float Prefs::GetExposure()
{
    return Exposure;
}

//----------------------------------------------------------------------
// GetVolume() - Return printable volume
//----------------------------------------------------------------------

Coord Prefs::GetVolume()
{
    return Volume;
}

//----------------------------------------------------------------------
// GetCommand() - Return hardware dependent executable commandline
//----------------------------------------------------------------------

string Prefs::GetCommand()
{
    return Command;
}

//----------------------------------------------------------------------
// GetSteps() - Return number of steps/mm for the Z axis motor
//----------------------------------------------------------------------

int Prefs::GetSteps()
{
    return Steps;
}
