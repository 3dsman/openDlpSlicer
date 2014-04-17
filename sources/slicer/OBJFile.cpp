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

#include "OBJFile.h"
#include <stdlib.h>     /* strtoul */

using namespace std;
inline void Split(const std::string& String, std::vector<std::string>& Tokens, const std::string& Delim = " \t\n");

//----------------------------------------------------------------------
// OBJFile() - Constructor
//----------------------------------------------------------------------

OBJFile::OBJFile(std::vector<Face>& faces,std::vector<Coord>& vertex): m_faces(faces), m_vertex(vertex)
{
}

//----------------------------------------------------------------------
// ~OBJFile() - Destructor
//----------------------------------------------------------------------

OBJFile::~OBJFile()
{
    /**
    delete m_position;
    delete m_texture;*/
}

//----------------------------------------------------------------------
// Load() - Load obj model from file
//----------------------------------------------------------------------

bool OBJFile::Load(const string& fileName){

    std::ifstream file(fileName.c_str(), ios::in);
    if(!file){
        return false;
    }
    string buf, key;
    while(getline(file, buf)){
            istringstream line(buf);
            line >> key;
            if(key == "v")
                ReadVertex(line);
            else if(key == "f")
                ReadFace(line);
    }

    return true;
}

//----------------------------------------------------------------------
// ReadFace() - Read a face of the obj file
//----------------------------------------------------------------------

bool OBJFile::ReadFace(istringstream& line){
    Face face;
    string vertex;

    while(!line.eof()){
        line >> vertex;
            vector<string> index;
            Split(vertex, index, "/");
            size_t current = strtoul(index[0].c_str(), NULL, 10)-1;
            face.Vertex.push_back(current);
    }
    m_faces.push_back(face);
    return true;

}

//----------------------------------------------------------------------
// ReadVertex() - Read a vertex of the obj file
//----------------------------------------------------------------------

bool OBJFile::ReadVertex(istringstream& line){
    float x = 0, y = 0, z = 0;

    line >> x >> y >> z;
    m_vertex.push_back(
            Coord(x, y, z)
            );
    return true;
}

//----------------------------------------------------------------------
// Coord() - Coord class
//----------------------------------------------------------------------
Coord::Coord(float x, float y, float z): X(x), Y(y), Z(z){
}

//----------------------------------------------------------------------
// Split() - Split the string by delim and put result in Tokens
//----------------------------------------------------------------------
inline void Split(const std::string& String, std::vector<std::string>& Tokens, const std::string& Delim)
{
        // clear the token vector
        Tokens.clear();

        // split the string using delim
        for (std::string::size_type p1 = 0, p2 = 0; p1 != std::string::npos; )
        {
                p1 = String.find_first_not_of(Delim, p1);
                if (p1 != std::string::npos)
                {
                        p2 = String.find_first_of(Delim , p1);
                        Tokens.push_back(String.substr(p1, p2 - p1));
                        p1 = p2;
                }
        }
}
